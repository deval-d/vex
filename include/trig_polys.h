//! include/trig_polys.h 
//! 
//! minimax polynomials to evaluate sin(x) and cos(x) over [-PI/4, PI/4] 


#include <arm_neon.h>
#include <math.h> 
#include <Accelerate/Accelerate.h> 


static const double PI = 3.14159265358979323846264338327950288;

static const double PIO2_HI = 0x1.921fb50000000p+0;
static const double PIO2_LO = 0x1.110b4611a6263p-26; 
static const float PIO2_HI_F = 0x1.921fb0p+0f; 
static const float PIO2_LO_F = 0x1.5110b4p-22f; 

static const double INV_PIO2 = 0x1.45f306dc9c883p-1;



/// Approximates sin(x) with a degree-9 minimax over [-PI/4, PI/4] 
static inline float sin_poly(float x) { 
    const float c3 = -0.1666666716f;
    const float c5 =  0.0083333291f;
    const float c7 = -0.0001983931f;
    const float c9 =  0.0000027181216f;

    float z = x * x; 
    float y = fmaf(c9, z, c7); 
    y = fmaf(z, y, c5); 
    y = fmaf(z, y, c3); 
    return fmaf(x * z, y, x); 
}


/// Approximates cos(x) with a degree-8 minimax over [-PI/4, PI/4] 
static inline float cos_poly(float x) { 
    const float c2 = -0.49999999694475949f;
    const float c4 =  0.041666620357131624f;
    const float c6 = -0.0013886681648107831f;
    const float c8 =  0.000024383567331036469f;   

    float z = x * x; 
    float y = fmaf(c8, z, c6); 
    y = fmaf(z, y, c4); 
    y = fmaf(z, y, c2);
    return fmaf(z, y, 1.0f); 
}



/// Approximates sin(x) with a degree-9 minimax over [-PI/4, PI/4] 
/// on a float32x4_t register 
static inline float32x4_t vsin_poly(float32x4_t x) { 
    const float32x4_t c3v = vdupq_n_f32(-0.1666666716f);
    const float32x4_t c5v = vdupq_n_f32(0.0083333291f);
    const float32x4_t c7v = vdupq_n_f32(-0.0001983931f);
    const float32x4_t c9v = vdupq_n_f32(0.0000027181216f);

    float32x4_t zv = vmulq_f32(x, x); 
    float32x4_t yv = vfmaq_f32(c7v, zv, c9v); 
    yv = vfmaq_f32(c5v, yv, zv); 
    yv = vfmaq_f32(c3v, yv, zv); 
    return vfmaq_f32(x, yv, vmulq_f32(x, zv));  
}


/// Approximates cos(x) with a degree-8 minimax over [-PI/4, PI/4] 
/// on a float32x4_t register 
static inline float32x4_t vcos_poly(float32x4_t x) { 
    const float32x4_t c2v = vdupq_n_f32(-0.49999999694475949f);
    const float32x4_t c4v = vdupq_n_f32(0.041666620357131624f);
    const float32x4_t c6v = vdupq_n_f32(-0.0013886681648107831f);
    const float32x4_t c8v = vdupq_n_f32(0.000024383567331036469f);   

    float32x4_t zv = vmulq_f32(x, x); 
    float32x4_t yv = vfmaq_f32(c6v, zv, c8v); 
    yv = vfmaq_f32(c4v, yv, zv); 
    yv = vfmaq_f32(c2v, yv, zv);
    return vfmaq_f32(vdupq_n_f32(1.0f), yv, zv); 
}

