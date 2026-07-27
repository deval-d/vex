//! test_sinf_ulp.c
//!
//! sweeps over many float inputs and reports ULP errors.


#include "vex.h"
#include "test_helpers.h"

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define VEC_LENGTH 4 
const float OVERFLOW_BOUND = 0x1p20f;
const float UNDERFLOW_BOUND = FLT_MIN;


void scalar(uint64_t n_samples) { 
    if (n_samples < 2) { 
        printf("not enough samples.\n");
        return; 
    }

    ULPReport report = empty_report();
    report.n_samples = n_samples; 
    report.counts = calloc(MAX_ULP + 1, sizeof(*report.counts));

    if (report.counts == NULL) { 
        printf("calloc failed.\n"); 
        return; 
    }

    for (uint64_t idx = 0; idx < n_samples; idx++) { 
        float x = sampled_float( 
            UNDERFLOW_BOUND, 
            OVERFLOW_BOUND, 
            n_samples, 
            idx 
        ); 

        float vex_result = vex_sinf(x); 
        float lib_result = sinf(x); 
        record_ulp_result(&report, x, vex_result , lib_result); 
    }   

    display_ulp_report(&report);
    free_report(&report); 
}

int main(void) { 
    uint64_t n_samples = 100000000; 

    printf("\nscalar sinf\n"); 
    scalar(n_samples); 

    return 0; 
}
