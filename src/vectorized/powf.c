//! vectorized/powf.c
//!
//! vectorized implementation of `powf(x, y)`.
//! evaluates several pairs in parallel


#include "vex.h"

#include <arm_neon.h>
#include <assert.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>


#define LANES 4


typedef struct {
    float64x2_t low;
    float64x2_t high;
} DoubleVectorPair;


static DoubleVectorPair widen(float32x4_t x) {
    return (DoubleVectorPair) {
        .low = vcvt_f64_f32(vget_low_f32(x)),
        .high = vcvt_f64_f32(vget_high_f32(x))
    };
}


static float64x2_t log2_mantissa(float64x2_t mantissa) {
    const double INV_LN2 = 0x1.71547652b82fep+0;

    float64x2_t one = vdupq_n_f64(1.0);
    float64x2_t s = vdivq_f64(
        vsubq_f64(mantissa, one),
        vaddq_f64(mantissa, one)
    );
    float64x2_t z = vmulq_f64(s, s);

    float64x2_t poly = vdupq_n_f64(1.0 / 11.0);
    poly = vfmaq_f64(vdupq_n_f64(1.0 / 9.0), z, poly);
    poly = vfmaq_f64(vdupq_n_f64(1.0 / 7.0), z, poly);
    poly = vfmaq_f64(vdupq_n_f64(1.0 / 5.0), z, poly);
    poly = vfmaq_f64(vdupq_n_f64(1.0 / 3.0), z, poly);
    poly = vfmaq_f64(vdupq_n_f64(1.0), z, poly);

    return vmulq_n_f64(vmulq_f64(s, poly), 2.0 * INV_LN2);
}


static DoubleVectorPair vector_log2(float32x4_t x) {
    const float32x4_t sqrt2 = vdupq_n_f32(0x1.6a09e6p+0f);
    const uint32x4_t mantissa_mask = vdupq_n_u32(0x007fffffu);
    const uint32x4_t one_bits = vdupq_n_u32(0x3f800000u);

    uint32x4_t bits = vreinterpretq_u32_f32(x);
    int32x4_t exponent = vsubq_s32(
        vreinterpretq_s32_u32(vshrq_n_u32(bits, 23)),
        vdupq_n_s32(127)
    );

    float32x4_t mantissa = vreinterpretq_f32_u32(
        vorrq_u32(vandq_u32(bits, mantissa_mask), one_bits)
    );

    uint32x4_t reduce = vcgtq_f32(mantissa, sqrt2);
    mantissa = vbslq_f32(reduce, vmulq_n_f32(mantissa, 0.5f), mantissa);
    exponent = vaddq_s32(
        exponent,
        vreinterpretq_s32_u32(vandq_u32(reduce, vdupq_n_u32(1)))
    );

    DoubleVectorPair m = widen(mantissa);
    DoubleVectorPair e = widen(vcvtq_f32_s32(exponent));

    return (DoubleVectorPair) {
        .low = vaddq_f64(e.low, log2_mantissa(m.low)),
        .high = vaddq_f64(e.high, log2_mantissa(m.high))
    };
}


static float64x2_t vector_exp2(float64x2_t x) {
    const double LN2 = 0x1.62e42fefa39efp-1;

    float64x2_t kd = vrndnq_f64(x);
    int64x2_t k = vcvtnq_s64_f64(kd);
    float64x2_t t = vmulq_n_f64(vsubq_f64(x, kd), LN2);

    float64x2_t poly = vdupq_n_f64(1.0 / 40320.0);
    poly = vfmaq_f64(vdupq_n_f64(1.0 / 5040.0), t, poly);
    poly = vfmaq_f64(vdupq_n_f64(1.0 / 720.0), t, poly);
    poly = vfmaq_f64(vdupq_n_f64(1.0 / 120.0), t, poly);
    poly = vfmaq_f64(vdupq_n_f64(1.0 / 24.0), t, poly);
    poly = vfmaq_f64(vdupq_n_f64(1.0 / 6.0), t, poly);
    poly = vfmaq_f64(vdupq_n_f64(0.5), t, poly);
    poly = vfmaq_f64(vdupq_n_f64(1.0), t, poly);
    poly = vfmaq_f64(vdupq_n_f64(1.0), t, poly);

    int64x2_t biased_k = vaddq_s64(k, vdupq_n_s64(1023));
    float64x2_t two_k = vreinterpretq_f64_s64(
        vshlq_n_s64(biased_k, 52)
    );

    return vmulq_f64(two_k, poly);
}


static bool all_in_range(float64x2_t x) {
    uint64x2_t high = vcltq_f64(x, vdupq_n_f64(128.0));
    uint64x2_t low = vcgeq_f64(x, vdupq_n_f64(-126.0));
    uint64x2_t valid = vandq_u64(high, low);

    return vgetq_lane_u64(valid, 0) && vgetq_lane_u64(valid, 1);
}


void vex_vpowf(
    FloatVectorRef bases,
    FloatVectorRef exponents,
    FloatVectorMut out
) {
    assert(bases.len == exponents.len);
    assert(out.len >= bases.len);

    const float32x4_t zero = vdupq_n_f32(0.0f);
    const float32x4_t min_normal = vdupq_n_f32(FLT_MIN);
    const float32x4_t max_float = vdupq_n_f32(FLT_MAX);

    const size_t vector_count = bases.len / LANES;
    const size_t tail_start = vector_count * LANES;

    for (size_t chunk = 0; chunk < vector_count; ++chunk) {
        const size_t offset = chunk * LANES;

        float32x4_t x = vld1q_f32(bases.data + offset);
        float32x4_t y = vld1q_f32(exponents.data + offset);

        uint32x4_t positive = vcgtq_f32(x, zero);
        uint32x4_t normal = vcgeq_f32(x, min_normal);
        uint32x4_t finite_x = vcleq_f32(x, max_float);
        uint32x4_t finite_y = vcleq_f32(vabsq_f32(y), max_float);
        uint32x4_t valid = vandq_u32(
            vandq_u32(positive, normal),
            vandq_u32(finite_x, finite_y)
        );

        // scalar fallback
        if (vminvq_u32(valid) == 0) {
            for (size_t lane = 0; lane < LANES; ++lane) {
                size_t i = offset + lane;
                out.data[i] = vex_powf(bases.data[i], exponents.data[i]);
            }

            continue;
        }

        DoubleVectorPair log2_x = vector_log2(x);
        DoubleVectorPair yd = widen(y);
        DoubleVectorPair power = {
            .low = vmulq_f64(yd.low, log2_x.low),
            .high = vmulq_f64(yd.high, log2_x.high)
        };

        if (!all_in_range(power.low) || !all_in_range(power.high)) {
            for (size_t lane = 0; lane < LANES; ++lane) {
                size_t i = offset + lane;
                out.data[i] = vex_powf(bases.data[i], exponents.data[i]);
            }

            continue;
        }

        float32x2_t low = vcvt_f32_f64(vector_exp2(power.low));
        float32x2_t high = vcvt_f32_f64(vector_exp2(power.high));
        vst1q_f32(out.data + offset, vcombine_f32(low, high));
    }

    for (size_t i = tail_start; i < bases.len; ++i) {
        out.data[i] = vex_powf(bases.data[i], exponents.data[i]);
    }
}
