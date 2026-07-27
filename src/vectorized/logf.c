//! vectorized/logf.c 
//! 
//! vectorized implementation of `logf(c)` 
//! evaluates several in parallel 


#include "vex.h" 
#include <arm_neon.h>
#include <assert.h> 
#include <Accelerate/Accelerate.h> 

#define LANES 4


float32x4_t vector_set_exponent_to(float32x4_t xv, uint32_t exponent) { 
    assert(exponent < 256 && "exponent must be less than 256"); 
    uint32x4_t bits = vreinterpretq_u32_f32(xv); 

    bits = vorrq_u32( 
        vandq_u32( 
            bits, 
            vdupq_n_u32(0x807fffffu)
        ), 
        vdupq_n_u32(exponent << 23)
    ); 

    return vreinterpretq_f32_u32(bits); 
}


float32x4_t vector_negative_poly(float32x4_t r) { 
    const float32x4_t c1 = vdupq_n_f32(0.99999985242729561f);
    const float32x4_t c2 = vdupq_n_f32(-0.50003654322887447f);
    const float32x4_t c3 = vdupq_n_f32(0.33186261984516702f);
    const float32x4_t c4 = vdupq_n_f32(-0.27225053856081005f);
    const float32x4_t c5 = vdupq_n_f32(0.038018061198611182f);
    const float32x4_t c6 = vdupq_n_f32(-0.78543102733181391f);
    const float32x4_t c7 = vdupq_n_f32(-1.0815401875157433f);
    const float32x4_t c8 = vdupq_n_f32(-1.1849314839059311f);  

    float32x4_t q = vfmaq_f32(c7, r, c8); 
    q = vfmaq_f32(c6, r, q); 
    q = vfmaq_f32(c5, r, q);
    q = vfmaq_f32(c4, r, q);
    q = vfmaq_f32(c3, r, q);
    q = vfmaq_f32(c2, r, q);
    q = vfmaq_f32(c1, r, q);
    float32x4_t lnf = vmulq_f32(r, q); 

    return lnf; 
}


void vex_vlogf(
    FloatVectorRef v,
    FloatVectorMut out
) { 
    float LN2 = 0x1.62e43p-1f;
    const float32x4_t OVERFLOW_BOUND = vdupq_n_f32(FLT_MAX); 
    const float32x4_t UNDERFLOW_BOUND = vdupq_n_f32(FLT_MIN); 

    size_t chunk_count = v.len / LANES; 
    size_t tail_count  = v.len % LANES; 

    const float* chunks = v.data; 
    const float* tail   = v.data + chunk_count * LANES; 

    float* chunks_out = out.data; 
    float* tail_out = out.data + chunk_count * LANES; 

    // fixed size chunks; SIMD-optimized
    for (size_t chunk = 0; chunk < chunk_count; chunk++) { 
        float32x4_t xv = vld1q_f32(chunks + chunk * LANES); 

        uint32x4_t too_hi = vcgtq_f32(xv, OVERFLOW_BOUND); 
        uint32x4_t too_lo = vcltq_f32(xv, UNDERFLOW_BOUND); 
        uint32x4_t special = vorrq_u32(too_hi, too_lo); 
        if (vmaxvq_u32(special) != 0) { 

            for (size_t i = 0; i < LANES; i++) {
                size_t idx = chunk * LANES + i; 
                chunks_out[idx] = vex_logf(chunks[idx]);
            }

            continue; 
        }

        uint32x4_t bits = vreinterpretq_u32_f32(xv);
        uint32x4_t biased_e = vandq_u32( 
            vshrq_n_u32(bits, 23), 
            vdupq_n_u32(0xffu) 
        );

        // true exponents 
        int32x4_t e = vsubq_s32( 
            vreinterpretq_s32_u32(biased_e), 
            vdupq_n_s32(127)
        ); 

        float32x4_t f; 
        float32x4_t k; 
        float32x4_t r; 
        float32x4_t lnf; 
        
        // register has an exponent of 0 
        // use scalar path for all inputs in [1, 2)
        int32x4_t zero = vdupq_n_s32(0); 
        uint32x4_t mask = vceqq_s32(e, zero);
        if (vmaxvq_u32(mask) != 0) { 

            for (size_t i = 0; i < LANES; i++) { 
                size_t idx = chunk * LANES + i; 
                chunks_out[idx] = vex_logf(chunks[idx]);
            }

            continue;

        // vectorized path otherwise
        } else { 
            f = vector_set_exponent_to(xv, 126u); 
            k = vmulq_n_f32( 
                vcvtq_f32_s32(
                    vaddq_s32(
                        e, 
                        vdupq_n_s32(1)
                    )
                ), 
                LN2 
            );
            r = vsubq_f32(f, vdupq_n_f32(1.0f)); 
            lnf = vector_negative_poly(r); 

            // store result 
            vst1q_f32(
                chunks_out + chunk * LANES, 
                vaddq_f32(k, lnf)
            );
        }
    }

    // leftover tail 
    for (size_t i = 0; i < tail_count; i++) { 
        tail_out[i] = vex_logf(tail[i]); 
    }
}
