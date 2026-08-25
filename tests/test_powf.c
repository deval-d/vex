//! tests/powf.c


#include "vex.h"
#include "test_helpers.h"

#include <Accelerate/Accelerate.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


static bool equal_result(float actual, float expected) {
    if (expected == 0.0f) {
        return asuint(actual) == asuint(expected);
    }

    return nearly_equal(actual, expected);
}


int scalar(float* bases, float* exponents, size_t n) {
    size_t num_failures = 0;
    for (size_t i = 0; i < n; i++) {
        const float result = vex_powf(bases[i], exponents[i]);
        const float expected = powf(bases[i], exponents[i]);

        if (!equal_result(result, expected)) {
            fprintf(
                stderr,
                "failed powf(%g, %g): expected %g, got %g\n",
                bases[i],
                exponents[i],
                expected,
                result
            );
            num_failures++;
        }
    }

    if (num_failures != 0) {
        fprintf(stderr, "%zu test(s) failed\n", num_failures);
        return 1;
    }

    printf("all scalar vex_powf tests passed\n");
    return 0;
}


int vectorized(float* bases, float* exponents, size_t n) {
    float* vex_out = malloc(n * sizeof(float));
    float* acc_out = malloc(n * sizeof(float));

    if (vex_out == NULL || acc_out == NULL) {
        free(vex_out);
        free(acc_out);
        return 1;
    }

    FloatVectorRef base_input = {
        .data = bases,
        .len = n
    };
    FloatVectorRef exponent_input = {
        .data = exponents,
        .len = n
    };
    FloatVectorMut output = {
        .data = vex_out,
        .len = n
    };

    vex_vpowf(base_input, exponent_input, output);

    int32_t count = (int32_t) n;
    vvpowf(acc_out, exponents, bases, &count);

    size_t num_failures = 0;
    for (size_t i = 0; i < n; i++) {
        if (!equal_result(vex_out[i], acc_out[i])) {
            fprintf(
                stderr,
                "failed powf(%g, %g): expected %g, got %g\n",
                bases[i],
                exponents[i],
                acc_out[i],
                vex_out[i]
            );
            num_failures++;
        }
    }

    free(vex_out);
    free(acc_out);

    if (num_failures != 0) {
        fprintf(stderr, "%zu test(s) failed\n", num_failures);
        return 1;
    }

    printf("all vectorized vex_vpowf tests passed\n");
    return 0;
}


int main(void) {
    float bases[] = {
        NAN, NAN, 1.0f, -1.0f, -2.0f, -2.0f, -2.0f,
        -0.0f, -0.0f, 0.0f, 0.0f, 0.5f, 2.0f, 2.0f,
        INFINITY, -INFINITY, -INFINITY, 0x1p-149f
    };
    float exponents[] = {
        0.0f, 2.0f, NAN, INFINITY, 3.0f, 4.0f, 0.5f,
        3.0f, -3.0f, -2.0f, INFINITY, INFINITY, -INFINITY, 10.0f,
        2.0f, 3.0f, -3.0f, 1.0f
    };

    size_t n = sizeof(bases) / sizeof(bases[0]);
    int scalar_failed = scalar(bases, exponents, n);
    int vector_failed = vectorized(bases, exponents, n);

    return scalar_failed | vector_failed;
}
