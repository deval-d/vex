//! tests/logf.c 


#include "vex.h"
#include "test_helpers.h" 
#include <stdio.h>
#include <stdlib.h>
#include <Accelerate/Accelerate.h>


int scalar(float* inputs, size_t n) { 
    size_t num_failures = 0; 
    for (size_t i = 0; i < n; i++) {
        const float result = vex_logf(inputs[i]); 
        const float expected = logf(inputs[i]);  

        if (!nearly_equal(result, expected)) { 
            fprintf(
                stderr, 
                "failed logf(%g): expected %g, got %g\n", 
                inputs[i], 
                expected, 
                result
            ); 
            num_failures++; 
        }
    }

    if (num_failures != 0) { 
        fprintf(
            stderr, 
            "%zu test(s) failed\n", 
            num_failures 
        );
        return 1; 
    } 

    printf("all scalar vex_logf tests passed\n"); 
    return 0;  
}

int vectorized(float* inputs, size_t n) { 
    float *vex_out = malloc(n * sizeof(float)); 
    float *acc_out = malloc(n * sizeof(float)); 

    if (vex_out == NULL || acc_out == NULL) { 
        free(vex_out); 
        free(acc_out); 
        return 1; 
    }

    FloatVectorRef input = {
        .data = inputs,
        .len = n,
    };

    FloatVectorMut output = {
        .data = vex_out,
        .len = n,
    };

    vex_vlogf(input, output); 

    int32_t count = (int32_t)n;
    vvlogf(acc_out, inputs, &count);

    size_t num_failures = 0; 
    for (size_t i = 0; i < n; i++) { 
       if (!nearly_equal(vex_out[i], acc_out[i])) { 
            fprintf(
                stderr, 
                "failed logf(%g): expected %g, got %g\n", 
                inputs[i], 
                acc_out[i], 
                vex_out[i]
            ); 
            num_failures++; 
       }
    }

    free(vex_out); 
    free(acc_out); 

    if (num_failures != 0) { 
        fprintf(
            stderr, 
            "%zu test(s) failed\n", 
            num_failures 
        );
        return 1; 
    } 

    printf("all vectorized vex_vlogf tests passed\n");
    return 0;
}

int main(void) {
    float inputs[] = {
        0.5f, 0.75f, 2.0f, 10.0f,
        3.0f, 4.0f, 20.0f, 80.0f
    };

    size_t n = sizeof(inputs) / sizeof(inputs[0]);
    int scalar_failed = scalar(inputs, n);
    int vector_failed = vectorized(inputs, n); 

    return scalar_failed | vector_failed;
}

