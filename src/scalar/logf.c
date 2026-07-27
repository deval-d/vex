//! scalar/logf.c 
//! 
//! scalar implementation of `logf(c)`. 


#include "vex.h" 
#include "helpers.h" 
#include <assert.h>
#include <float.h>
#include <math.h> 
#include <stdint.h>


float set_exponent_to(float x, uint32_t exponent) {
    assert(exponent < 256 && "exponent must be less than 256"); 
    uint32_t bits = asuint(x); 

    bits = (bits & 0x807fffffu) | (exponent << 23);

    return asfloat(bits);
}


/// Minimax evaluation for f when x's true exponent is not 0. 
float negative_poly(float r) { 
    // minimax remez coefficients for ln(1 + r) on [-0.5, 0) 
    const double c1 =  0.99999985242729561;
    const double c2 = -0.50003654322887447;
    const double c3 =  0.33186261984516702;
    const double c4 = -0.27225053856081005;
    const double c5 =  0.038018061198611182;
    const double c6 = -0.78543102733181391;
    const double c7 = -1.0815401875157433;
    const double c8 = -1.1849314839059311; 

    float q = fmaf(r, c8, c7); 
    q = fmaf(r, q, c6); 
    q = fmaf(r, q, c5); 
    q = fmaf(r, q, c4); 
    q = fmaf(r, q, c3); 
    q = fmaf(r, q, c2); 
    q = fmaf(r, q, c1); 
    float lnf = r * q; 

    return lnf; 
}

/// Minimax evaluation for f when x's true exponent is 0. 
float positive_poly(float r) { 
    // minimax remez coefficients for ln(1 + r) on (0, 1]
    const double c1  =  0.99999999528721917;
    const double c2  = -0.49999902422775899;
    const double c3  =  0.33329962416588654;
    const double c4  = -0.24954133655537836;
    const double c5  =  0.19675939255501274;
    const double c6  = -0.15305592566389509;
    const double c7  =  0.10603654877058005;
    const double c8  = -0.056959534201723228;
    const double c9  =  0.019851462335085494;
    const double c10 = -0.0032440251717337546;

    float q = fmaf(r, c10, c9); 
    q = fmaf(r, q, c8); 
    q = fmaf(r, q, c7); 
    q = fmaf(r, q, c6); 
    q = fmaf(r, q, c5); 
    q = fmaf(r, q, c4); 
    q = fmaf(r, q, c3); 
    q = fmaf(r, q, c2); 
    q = fmaf(r, q, c1); 
    float lnf = r * q; 

    return lnf; 
}


float vex_logf(float x) { 
    const double LN2 = 0.693147180559945309417232121458176568;

    const float OVERFLOW_BOUND = FLT_MAX; 
    const float UNDERFLOW_BOUND = FLT_MIN; 

    // checks 
    if (isnan(x)) return x; 
    if (x < 0.0f) return NAN; 

    // no subnormals 
    if (x < UNDERFLOW_BOUND) return -INFINITY; 
    if (x > OVERFLOW_BOUND) return INFINITY; 

    uint32_t bits = asuint(x); 
    uint32_t biased_e = (bits >> 23) & 0xffu; 

    // true exponent 
    int32_t e = (int32_t) biased_e - 127; 

    float f; 
    double k;
    float r; 
    float lnf; 
    if (e == 0) { 
        // f = 1.M_2 
        f = set_exponent_to(x, 127u); 
        k = (double) e * LN2; 
        r = f - 1.0;
        lnf = positive_poly(r); 

    } else { 
        // f = 1.M_2 / 2
        f = set_exponent_to(x, 126u); 
        k = (double) (e + 1) * LN2; 
        r = f - 1.0; 
        lnf = negative_poly(r); 
    }

    return k + lnf; 
}
