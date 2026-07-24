//! helpers.h 


#ifndef VEX_HELPERS_H
#define VEX_HELPERS_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>


/// Returns a `uint32_t` with the same bits as the given `float`. 
static inline uint32_t asuint(float f) { 
    union { 
        float f; 
        uint32_t i; 
    } u = {.f = f}; 

    return u.i; 
}

/// Returns a `float` with the same bits as the given `uint32_t`. 
static inline float asfloat(uint32_t i) { 
    union { 
        uint32_t i;
        float f; 
    } u = {.i = i}; 

    return u.f; 
}

/// Benchmarks a function involving `float`s and returns the elapsed time. 
static inline double benchmark_float( 
    float x, 
    float (*function)(float), 
    size_t iterations
) { 
    volatile float input = x; 
    volatile float result; 

    // warm-up 
    for (size_t i = 0; i < 10000000; i++) {
        result = function(input);
    }

    clock_t beg = clock(); 
    for (size_t i = 0; i < iterations; i++) { 
        result = function(input); 
    }
    clock_t end = clock(); 

    (void) result; 

    return ((double)(end - beg) / CLOCKS_PER_SEC)/(double) iterations; 
}


#endif 
