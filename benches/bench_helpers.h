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

typedef void (*binary_vector_function)(
    const float *x,
    const float *y,
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


static inline double benchmark_binary_float(
    const float x,
    const float y,
    float (*function)(float, float),
    size_t iterations
) {
    volatile float base = x;
    volatile float exponent = y;
    volatile float sink;

    // warm-up
    for (size_t i = 0; i < 10000000; i++) {
        sink = function(base, exponent);
    }

    clock_t beg = clock();
    for (size_t i = 0; i < iterations; i++) {
        sink = function(base, exponent);
    }
    clock_t end = clock();

    (void) sink;

    return ((double)(end - beg) / CLOCKS_PER_SEC) / (double) iterations;
}


static inline double benchmark_binary_vector_float(
    const float* x,
    const float* y,
    float* output,
    size_t len,
    binary_vector_function function,
    size_t iterations
) {
    volatile float sink;

    // warm-up
    for (size_t i = 0; i < 1000; i++) {
        function(x, y, output, len);
    }

    clock_t beg = clock();
    for (size_t i = 0; i < iterations; i++) {
        function(x, y, output, len);
    }
    clock_t end = clock();

    sink = output[len - 1];
    (void) sink;

    return ((double)(end - beg) / CLOCKS_PER_SEC) / (double) iterations;
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

static inline void benchmark_vex_vsinf(
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

    vex_vsinf(in, out);

}

static inline void benchmark_accelerate_vsinf(
    const float *input,
    float *output,
    size_t len
) {
    int32_t count = (int32_t)len;
    vvsinf(output, input, &count);
}

static inline void benchmark_vex_vcosf(
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

    vex_vcosf(in, out);
}

static inline void benchmark_accelerate_vcosf(
    const float *input,
    float *output,
    size_t len
) {
    int32_t count = (int32_t)len;
    vvcosf(output, input, &count);
}

static inline void benchmark_vex_vtanf(
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

    vex_vtanf(in, out);
}

static inline void benchmark_accelerate_vtanf(
    const float *input,
    float *output,
    size_t len
) {
    int32_t count = (int32_t)len;
    vvtanf(output, input, &count);
}

static inline void benchmark_vex_vpowf(
    const float *bases,
    const float *exponents,
    float *output,
    size_t len
) {
    FloatVectorRef base_input = {
        .data = bases,
        .len = len,
    };
    FloatVectorRef exponent_input = {
        .data = exponents,
        .len = len,
    };
    FloatVectorMut out = {
        .data = output,
        .len = len,
    };

    vex_vpowf(base_input, exponent_input, out);
}

static inline void benchmark_accelerate_vpowf(
    const float *bases,
    const float *exponents,
    float *output,
    size_t len
) {
    int32_t count = (int32_t)len;
    vvpowf(output, exponents, bases, &count);
}

#endif 
