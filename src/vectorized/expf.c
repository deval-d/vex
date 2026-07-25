//! vectorized/expf.c 
//! 
//! vectorized implementation of `expf`; 
//! evaluates several in parallel. 


#include "vex.h" 
#include <assert.h>
#include <stdint.h>
#include <arm_neon.h>


#define LANES 4


void vex_vexpf( 
    FloatVectorRef v, 
    FloatVectorMut out 
) { 
    assert(v.len == out.len && "input and output vector lengths must match.");

    // log_2(e) 
    const float LOG2E = 1.4426950408889634f;

    const float OVERFLOW_BOUND = 88.37626f; 
    const float UNDERFLOW_BOUND = -87.33654f; 

    // minimax remez coefficients for 2^x on [-0.5, 0.5]
    // constrain c0 = 1.0 so e^0 = 1.0 is exact. 
    float32x4_t c0v = vdupq_n_f32(1.0f);
    float32x4_t c1v = vdupq_n_f32(0.69314724206924438f);
    float32x4_t c2v = vdupq_n_f32(0.24022234976291656f);
    float32x4_t c3v = vdupq_n_f32(0.055503085255622864f);
    float32x4_t c4v = vdupq_n_f32(0.0096718752756714821f);
    float32x4_t c5v = vdupq_n_f32(0.0013407259248197079f);

    size_t chunk_count = v.len / LANES; 
    size_t tail_count  = v.len % LANES; 

    // input 
    const float* chunks = v.data; 
    const float* tail   = v.data + chunk_count * LANES; 

    // output 
    float* chunks_out = out.data; 
    float* tail_out   = out.data + chunk_count * LANES; 

    // fixed-size chunks; SIMD optimized 
    float32x4_t log2ev     = vdupq_n_f32(LOG2E); 
    float32x4_t overflowv  = vdupq_n_f32(OVERFLOW_BOUND); 
    float32x4_t underflowv = vdupq_n_f32(UNDERFLOW_BOUND); 
    for (size_t i = 0; i < chunk_count; i++) { 
        float32x4_t xv = vld1q_f32(chunks + i * LANES);

        // if any element in xv is invalid 
        // uses scalar path instead. 
        uint32x4_t valid_high = vcleq_f32(xv, overflowv);
        uint32x4_t valid_low  = vcgeq_f32(xv, underflowv);
        uint32x4_t valid = vandq_u32(valid_high, valid_low); 
        if (vminvq_u32(valid) == 0) { 
            for (size_t lane = 0; lane < LANES; lane++) { 
                size_t idx = i * LANES + lane; 
                chunks_out[idx] = vex_expf(chunks[idx]); 
            }
            continue;
        }

        float32x4_t xlog2ev = vmulq_f32(xv, log2ev); 

        // integer and fractional parts 
        float32x4_t kfv = vrndnq_f32(xlog2ev); 
        float32x4_t rv  = vsubq_f32(xlog2ev, kfv); 

        // evaluating 2^kv 
        int32x4_t kv = vcvtq_s32_f32(kfv); 
        int32x4_t biased_kv = vaddq_s32(kv, vdupq_n_s32(127)); 
        uint32x4_t bits = vshlq_n_u32(vreinterpretq_u32_s32(biased_kv), 23); 
        float32x4_t two_kv  = vreinterpretq_f32_u32(bits); 

        // evaluating minimax polynomial 
        float32x4_t two_rv; 
        two_rv = vfmaq_f32(c4v, c5v, rv); 
        two_rv = vfmaq_f32(c3v, two_rv, rv); 
        two_rv = vfmaq_f32(c2v, two_rv, rv); 
        two_rv = vfmaq_f32(c1v, two_rv, rv); 
        two_rv = vfmaq_f32(c0v, two_rv, rv); 

        float32x4_t result = vmulq_f32(two_kv, two_rv); 
        vst1q_f32(chunks_out + i * LANES, result); 
    }

    // leftover tail 
    for (size_t i = 0; i < tail_count; i++) { 
        tail_out[i] = vex_expf(tail[i]);
    }
}
