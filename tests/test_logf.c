//! tests/logf.c 


#include "vex.h"
#include "test_helpers.h" 
#include <stdio.h>
#include <stdlib.h>


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

int main(void) {
    float inputs[] = {
        -INFINITY, -10.0f, -1.0f, 0.0f, 0.5f,
        1.0f, 2.0f, 10.0f, 80.0f, INFINITY
    };

    size_t n = sizeof(inputs) / sizeof(inputs[0]);
    int scalar_failed = scalar(inputs, n);

    return scalar_failed;
}

