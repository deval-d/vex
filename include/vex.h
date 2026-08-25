//! include/vex.h 
//! 
//! all functions and types. 


#ifndef VEX_H 
#define VEX_H 


#include <stddef.h> 


/// an immutable vector type 
typedef struct { 
    const float* data; 
    size_t len; 
} FloatVectorRef; 

/// a mutable vector type 
typedef struct { 
    float* data; 
    size_t len; 
} FloatVectorMut;

// scalar functions

/// Computes e^x. 
///
/// Reduces the problem to
///
///     e^x = 2^(x log2(e)) = 2^k * 2^r,
///
/// where k is an integer and r is the remaining fraction.
/// 2^k is calculated exactly from the exponent field. 
/// 2^r is approximated using a degree-5 minimax polynomial. 
float vex_expf(float x);


/// Computes ln(x) 
///
/// Reduces the problem to 
///
///     ln(x) = ln(2^(e + 1)) + ln(f), 
///
/// where e is the true exponent of x, and f is the mantissa divided by two. 
/// ln(2^(e + 1)) is approximated using `double`s. 
/// ln(f) is approximated using a degree-8 minimax polynomial.
float vex_logf(float x); 


/// Computes sin(x) 
float vex_sinf(float x);


/// Computes cos(x)
float vex_cosf(float x);


/// Computes tan(x)
float vex_tanf(float x);


/// Computes x^y.
///
/// Reduces the problem to
///
///     x^y = 2^(y log2(x)).
///
/// log2(x) is evaluated after splitting x into its exponent and mantissa.
/// 2^z is evaluated after splitting z into an integer and a small fraction.
float vex_powf(float x, float y);



// vectorized functions 

/// Computes e^x in parallel for multiple x. 
///
/// Reduces the problem to
///
///     e^x = 2^(x log2(e)) = 2^k * 2^r,
///
/// where k is an integer and r is the remaining fraction.
/// 2^k is calculated exactly from the exponent field. 
/// 2^r is approximated using a degree-5 minimax polynomial. 
///
/// args: 
/// * [FloatVectorRef] v - input vector of x 
/// * [FloatVectorMut] out - will store output vector of e^x 
void vex_vexpf(FloatVectorRef v, FloatVectorMut out); 


/// Computes ln(x) in parallel for multiple x. 
///
/// Reduces the problem to 
///
///     ln(x) = ln(2^(e + 1)) + ln(f), 
///
/// where e is the true exponent of x, and f is the mantissa divided by two. 
/// ln(2^(e + 1)) is approximated using `double`s. 
/// ln(f) is approximated using a degree-8 minimax polynomial.
void vex_vlogf(FloatVectorRef v, FloatVectorMut out);

/// Computes sin(x) in parallel for multiple x. 
void vex_vsinf(FloatVectorRef v, FloatVectorMut out); 

/// Computes cos(x) in parallel for multiple x.
void vex_vcosf(FloatVectorRef v, FloatVectorMut out);

/// Computes tan(x) in parallel for multiple x.
void vex_vtanf(FloatVectorRef v, FloatVectorMut out);

/// Computes x^y in parallel for multiple pairs of x and y.
void vex_vpowf(
    FloatVectorRef bases,
    FloatVectorRef exponents,
    FloatVectorMut out
);


#endif
