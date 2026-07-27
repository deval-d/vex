//! benches/bench_helpers.h 
//!
//! helpers for benchmarking 


#ifndef BENCH_HELPERS_H 
#define BENCH_HELPERS_H

#include "vex.h"
#include <time.h> 
#include <stdint.h>
#include <Accelerate/Accelerate.h>


typedef void (*vector_function)(
    const float *input,
    float *output,
    size_t len
);


/// Benchmarks a function involving `float`s and returns the elapsed time. 
static inline double benchmark_float( 
    const float x, 
    float (*function)(float), 
    size_t iterations
) { 
    volatile float input = x; 
    volatile float sink; 

    // warm-up 
    for (size_t i = 0; i < 10000000; i++) {
        sink = function(input);
    }

    clock_t beg = clock(); 
    for (size_t i = 0; i < iterations; i++) { 
        sink = function(input); 
    }
    clock_t end = clock(); 

    (void) sink; 

    return ((double)(end - beg) / CLOCKS_PER_SEC)/(double) iterations; 
}

static inline double benchmark_vector_float( 
    const float* input,
    float* output, 
    size_t len, 
    vector_function function, 
    size_t iterations 
) { 

    volatile float sink; 
    // warm-up
    for (size_t i = 0; i < 1000; i++) {
        function(input, output, len);
    }

    clock_t beg = clock();
    for (size_t i = 0; i < iterations; i++) {
        function(input, output, len);
    }
    clock_t end = clock();

    sink = output[len - 1];
    (void)sink;

    return ((double)(end - beg) / CLOCKS_PER_SEC)/ (double)iterations;
}


static inline void benchmark_vex_vexpf(
    const float *input,
    float *output,
    size_t len
) {
    FloatVectorRef in = {
        .data = input,
        .len = len,
    };

    FloatVectorMut out = {
        .data = output,
        .len = len,
    };

    vex_vexpf(in, out);

}

static inline void benchmark_accelerate_vexpf(
    const float *input,
    float *output,
    size_t len
) {
    int32_t count = (int32_t)len;
    vvexpf(output, input, &count);
}

static inline void benchmark_vex_vlogf(
    const float *input,
    float *output,
    size_t len
) {
    FloatVectorRef in = {
        .data = input,
        .len = len,
    };

    FloatVectorMut out = {
        .data = output,
        .len = len,
    };

    vex_vlogf(in, out);

}

static inline void benchmark_accelerate_vlogf(
    const float *input,
    float *output,
    size_t len
) {
    int32_t count = (int32_t)len;
    vvlogf(output, input, &count);
}



#endif 
