//! vectorized/cosf.c
//!
//! vectorized implementation of `cosf(x)`.
//! evaluates several in parallel


#include "vex.h"
#include "trig_polys.h"

#include <arm_neon.h>
#include <assert.h>
#include <stdint.h>


#define LANES 4


void vex_vcosf(FloatVectorRef v, FloatVectorMut out) {
    assert(out.len >= v.len);

    const float32x4_t range_limit = vdupq_n_f32(0x1p20f);

    const uint32x4_t abs_mask = vdupq_n_u32(0x7fffffffu);
    const uint32x4_t sign_bit = vdupq_n_u32(0x80000000u);
    const uint32x4_t quadrant_mask = vdupq_n_u32(3);

    const size_t vector_count = v.len / LANES;
    const size_t tail_start = vector_count * LANES;

    for (size_t chunk = 0; chunk < vector_count; ++chunk) {
        const size_t offset = chunk * LANES;

        float32x4_t x = vld1q_f32(v.data + offset);
        uint32x4_t x_bits = vreinterpretq_u32_f32(x);
        float32x4_t ax = vreinterpretq_f32_u32(
            vandq_u32(x_bits, abs_mask)
        );

        // scalar fallback
        uint32x4_t not_in_range = vmvnq_u32(
            vcleq_f32(ax, range_limit)
        );

        if (vmaxvq_u32(not_in_range) != 0) {
            for (size_t lane = 0; lane < LANES; ++lane) {
                out.data[offset + lane] = vex_cosf(v.data[offset + lane]);
            }

            continue;
        }

        float32x4_t n_float = vrndnq_f32(
            vmulq_n_f32(ax, INV_PIO2)
        );

        int32x4_t n = vcvtq_s32_f32(n_float);
        uint32x4_t quadrant = vandq_u32(
            vreinterpretq_u32_s32(n),
            quadrant_mask
        );

        // [-PI/4, PI/4]
        float32x4_t r = vfmsq_n_f32(ax, n_float, PIO2_HI_F);
        r = vfmsq_n_f32(r, n_float, PIO2_LO_F);

        float32x4_t sin_r = vsin_poly(r);
        float32x4_t cos_r = vcos_poly(r);

        uint32x4_t use_sine = vtstq_u32(
            quadrant,
            vdupq_n_u32(1)
        );

        float32x4_t result = vbslq_f32(use_sine, sin_r, cos_r);

        uint32x4_t negate = vtstq_u32(
            vaddq_u32(quadrant, vdupq_n_u32(1)),
            vdupq_n_u32(2)
        );

        uint32x4_t result_bits = veorq_u32(
            vreinterpretq_u32_f32(result),
            vandq_u32(negate, sign_bit)
        );

        vst1q_f32(
            out.data + offset,
            vreinterpretq_f32_u32(result_bits)
        );
    }

    for (size_t i = tail_start; i < v.len; ++i) {
        out.data[i] = vex_cosf(v.data[i]);
    }
}
