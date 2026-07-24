//! include/helpers.h 


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

#endif 
