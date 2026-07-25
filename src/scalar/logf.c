//! scalar/logf.c 
//! 
//! scalar implementation of `logf(c)`. 


#include "vex.h" 
#include "helpers.h" 
#include <float.h>
#include <math.h> 
#include <stdint.h>


float set_exponent_to_126(float x) {
    uint32_t bits = asuint(x); 

    // overwrite exponent field with 126
    bits = (bits & 0x007fffffu) | (126u << 23); 
    return asfloat(bits); 
}


float vex_logf(float x) { 
    const double LN2 = 0.693147180559945309417232121458176568;

    const float OVERFLOW_BOUND = FLT_MAX; 
    const float UNDERFLOW_BOUND = FLT_MIN; 

    // minimax remez coefficients for ln(1+r) on [-0.5, 0] 
    // constrain c0 = 0.0 so ln(1) = ln(1 + 0) = 0; 
    const double c1 =  0.99999985242729561;
    const double c2 = -0.50003654322887447;
    const double c3 =  0.33186261984516702;
    const double c4 = -0.27225053856081005;
    const double c5 =  0.038018061198611182;
    const double c6 = -0.78543102733181391;
    const double c7 = -1.0815401875157433;
    const double c8 = -1.1849314839059311;

    // checks 
    if (isnan(x)) return x; 
    if (x < 0.0f) return NAN; 
    if (x < UNDERFLOW_BOUND) return -INFINITY; 
    if (x > OVERFLOW_BOUND) return INFINITY; 

    uint32_t bits = asuint(x); 
    uint32_t biased_e = (bits >> 23) & 0xffu; 

    // true exponent 
    int32_t e = (int32_t) biased_e - 127; 

    // f = mantissa / 2 
    float f = set_exponent_to_126(x);

    // r in [-0.5, 0] 
    float r = f - 1.0; 

    // evaluate minimax polynomial of ln(1 + r) 
    float q = fmaf(r, c8, c7); 
    q = fmaf(r, q, c6); 
    q = fmaf(r, q, c5); 
    q = fmaf(r, q, c4); 
    q = fmaf(r, q, c3); 
    q = fmaf(r, q, c2); 
    q = fmaf(r, q, c1); 
    float lnf = r * q; 

    // ln(x) = ln(2^(e + 1)) * ln(f) 
    return (float)((double) (e + 1) * LN2) + lnf; 
}
