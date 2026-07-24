//! scalar/expf.c  
//! 
//! scalar implementation of `expf(x)`. 


#include "vex.h"
#include "helpers.h"
#include <math.h> 
#include <stdint.h>


float vex_expf(float x) {
    // log_2(e) 
    const double LOG2E = 1.44269504088896340735992468100189214;

    const float OVERFLOW_BOUND = 88.37626f; 
    const float UNDERFLOW_BOUND = -87.33654f; 

    // minimax remez coefficients for 2^x on [-0.5, 0.5]
    // constrain c0 = 1.0 so e^0 = 1.0 is exact. 
    const float c0 = 1.0f;
    const float c1 = 0.69314724206924438f;
    const float c2 = 0.24022234976291656f;
    const float c3 = 0.055503085255622864f;
    const float c4 = 0.0096718752756714821f;
    const float c5 = 0.0013407259248197079f;

    // checks 
    if (isnan(x)) return x; 
    if (x == -INFINITY) return 0.0f; 
    if (x > OVERFLOW_BOUND) return INFINITY; 
    if (x < UNDERFLOW_BOUND) return 0.0f; 

    double xlog2e;     
    double kd;         
    int32_t k;        
    float r;          
    float two_k;      
    float two_r;      

    xlog2e = (double) x * LOG2E; 

    // integer part of x log_2(e)
    kd = round(xlog2e); 
    k = (int32_t) kd;

    // fractional part of x log_2(e) in [-0.5, 0.5]
    r = (float) (xlog2e - kd); 
    
    // evaluating 2^k
    two_k = asfloat(((uint32_t)(k + 127) << 23)); 

    // evaluating 2^r 
    // minimax polynomial in Horner's form 
    two_r = fmaf(r, c5, c4); 
    two_r = fmaf(r, two_r, c3);
    two_r = fmaf(r, two_r, c2); 
    two_r = fmaf(r, two_r, c1); 
    two_r = fmaf(r, two_r, c0); 
    return two_k * two_r; 
}
