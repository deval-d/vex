//! benches/bench_powf.c


#include "vex.h"
#include "bench_helpers.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>


int scalar(void) {
    const float base = 1.25f;
    const float exponent = 3.5f;

    double vex_time = benchmark_binary_float(
        base,
        exponent,
        vex_powf,
        100000000
    );
    double math_time = benchmark_binary_float(
        base,
        exponent,
        powf,
        100000000
    );

    printf("scalar vex powf:  %.3f ns\n", vex_time * 1e9);
    printf("scalar math powf: %.3f ns\n", math_time * 1e9);

    return 0;
}


int vectorized(void) {
    const size_t n = 4096;
    const size_t iterations = 100000;

    float* bases = malloc(n * sizeof(float));
    float* exponents = malloc(n * sizeof(float));
    float* output = malloc(n * sizeof(float));

    if (bases == NULL || exponents == NULL || output == NULL) {
        free(bases);
        free(exponents);
        free(output);
        return 1;
    }

    for (size_t i = 0; i < n; i++) {
        bases[i] = 0.25f + 3.75f * (float)i / (float)(n - 1);
        exponents[i] = -4.0f + 8.0f * (float)i / (float)(n - 1);
    }

    double vex_time = benchmark_binary_vector_float(
        bases,
        exponents,
        output,
        n,
        benchmark_vex_vpowf,
        iterations
    );
    double acc_time = benchmark_binary_vector_float(
        bases,
        exponents,
        output,
        n,
        benchmark_accelerate_vpowf,
        iterations
    );

    printf("vector length:     %zu\n", n);
    printf("vex vpowf:         %.3f us, %.3f ns/element\n", vex_time * 1e6, vex_time * 1e9 / n);
    printf("accelerate vvpowf: %.3f us, %.3f ns/element\n", acc_time * 1e6, acc_time * 1e9 / n);

    free(bases);
    free(exponents);
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
