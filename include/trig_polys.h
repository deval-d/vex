//! include/trig_polys.h 
//! 
//! minimax polynomials to evaluate sin(x) and cos(x) over [-PI/4, PI/4] 


#include <math.h>


static const double PI = 3.14159265358979323846264338327950288;
static const double PIO2_HI = 0x1.921fb50000000p+0;
static const double PIO2_LO = 0x1.110b4611a6263p-26; 
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
    const double c2 = -0.49999999694475949;
    const double c4 =  0.041666620357131624;
    const double c6 = -0.0013886681648107831;
    const double c8 =  0.000024383567331036469;   

    float z = x * x; 
    float y = fmaf(c8, z, c6); 
    y = fmaf(z, y, c4); 
    y = fmaf(z, y, c2);
    return fmaf(z, y, 1.0f); 
}

