//! benches/bench_sinf.c 


#include "vex.h"
#include "bench_helpers.h" 
#include <stdio.h> 
#include <math.h>
#include <stdlib.h>


int scalar(void) {
    const float sentinel = 10.0f; 

    double vex_time; 
    double math_time; 

    vex_time  = benchmark_float(sentinel, vex_sinf, 100000000);
    math_time = benchmark_float(sentinel, sinf, 100000000); 

    printf("scalar vex sinf:  %.3f ns\n", vex_time * 1e9); 
    printf("scalar math sinf: %.3f ns\n", math_time * 1e9); 

    return 0; 
} 

int vectorized(void) {
    const size_t n = 4096;
    const size_t iterations = 100000;

    float *input  = malloc(n * sizeof(float));
    float *output = malloc(n * sizeof(float));

    if (input == NULL || output == NULL) {
        free(input);
        free(output);
        return 1;
    }

    for (size_t i = 0; i < n; i++) {
        input[i] = -10.0f + 20.0f * (float)i / (float)(n - 1);
    }

    double vex_time = benchmark_vector_float(
        input,
        output,
        n,
        benchmark_vex_vsinf,
        iterations
    );

    double acc_time = benchmark_vector_float(
        input,
        output,
        n,
        benchmark_accelerate_vsinf,
        iterations
    );

    printf("vector length:     %zu\n", n);
    printf("vex vsinf:         %.3f us, %.3f ns/element\n", vex_time * 1e6, vex_time * 1e9 / n);
    printf("accelerate vvsinf: %.3f us, %.3f ns/element\n", acc_time * 1e6, acc_time * 1e9 / n); 

    free(input);
    free(output);
    return 0;
}

int main(void) {
    int status = 0;

    printf("\nscalar\n");
    status |= scalar();
    printf("\nvectorized\n"); 
    status |= vectorized(); 

    return status;
}
