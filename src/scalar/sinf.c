//! scalar/sinf.c 
//! 
//! scalar implementation of `sinf(x)`. 


#include "vex.h"
#include "helpers.h" 
#include "trig_polys.h"
#include <float.h>
#include <math.h> 
#include <stdint.h> 

float vex_sinf(float x) {   
    const float UNDERFLOW_BOUND = FLT_MIN;
    const float OVERFLOW_BOUND = 0x1p20f; 

    uint32_t bits = asuint(x); 
    uint32_t ax = bits & 0x7fffffff; 
    float pos_x = asfloat(ax); 

    // +/- 0 and very small rounds to x 
    if (pos_x < FLT_MIN) { 
        return x; 
    }

    // INFINITY and NaN 
    if (!isfinite(x)) { 
        return x - x; 
    } 

    // above 2^20, my n rounding fails 
    if (pos_x > OVERFLOW_BOUND) { 
        return NAN; 
    }

    uint32_t sign = asuint(x) >> 31; 
    // TODO: better n rounding  
    int64_t n = (int32_t) round((double) pos_x * INV_PIO2);

    // [-PI/4, PI/4] 
    double r = ((double)pos_x - (double)n * PIO2_HI) - (double)n * PIO2_LO;

    uint32_t quadrant = n & 3; 

    float result; 
    switch (quadrant) { 
        case 0: 
            result = sin_poly(r); 
            break; 
        case 1: 
            result = cos_poly(r); 
            break; 
        case 2: 
            result = -sin_poly(r); 
            break; 
        default: 
            result = -cos_poly(r); 
            break; 
    }

    return sign ? -result : result; 
}
