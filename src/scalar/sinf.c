//! scalar/sinf.c 
//! 
//! scalar implementation of `sinf(x)`. 


#include "vex.h"
#include "helpers.h" 
#include "trig_polys.h"
#include <float.h>
#include <math.h> 
#include <stdint.h> 

inline float apply_sign(float value, uint32_t sign_mask){
    return asfloat(asuint(value) ^ sign_mask);
}

float vex_sinf(float x) {  
    // asuint(FLT_MAX) 
    const uint32_t OVERFLOW_BOUND = 2138135039; 
    const float UNDERFLOW_BOUND = FLT_MIN; 

    // INFINITY and NaN 
    if (!isfinite(x)) { 
        return x - x; 
    } 

    uint32_t bits = asuint(x); 
    float absx = asfloat(bits & 0x7fffffffu);
    uint32_t sign_mask = bits & 0x80000000u;

    // subnormals 
    if (absx < UNDERFLOW_BOUND) { 
        return x; 
    }

    double xd = (double) absx; 
    double nd = round(xd * INV_PIO2); 
    int32_t n = (int32_t) nd; 
    uint32_t quadrant = n & 3; 


    // [-PI/4, PI/4] 
    double r  = (xd - nd * PIO2_HI) - nd * PIO2_LO; 

    float result; 
    switch (quadrant) { 
        case 0:
            return apply_sign(sin_poly(r), sign_mask);
        case 1:
            return apply_sign(cos_poly(r), sign_mask);
        case 2:
            return apply_sign(-sin_poly(r), sign_mask);
        default:
            return apply_sign(-cos_poly(r), sign_mask);
    }

    return asfloat(asuint(result) ^ sign_mask);  
}
