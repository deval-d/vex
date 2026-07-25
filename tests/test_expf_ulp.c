//! test_expf_ulp.c
//!
//! Sweeps over many float inputs and reports ULP errors.

#include "vex.h"
#include "test_helpers.h"

#include <Accelerate/Accelerate.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>


const float OVERFLOW_BOUND = 88.37626f;
const float UNDERFLOW_BOUND = -87.33654f;
#define VEC_LENGTH 4 


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

        float vex_result = vex_expf(x); 
        float lib_result = expf(x); 
        record_ulp_result(&report, x, vex_result , lib_result); 
    }   

    display_ulp_report(&report);
    free_report(&report); 
}

void vectorized(uint64_t n_samples) { 
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

    for (uint64_t idx = 0; idx < n_samples; idx+=VEC_LENGTH) { 

        uint64_t remaining = n_samples - idx; 
        int32_t vec_len = remaining > VEC_LENGTH 
            ? VEC_LENGTH 
            : (int32_t) remaining; 

        float x[VEC_LENGTH];
        float vex_out[VEC_LENGTH]; 
        float lib_out[VEC_LENGTH]; 

        for (int32_t i = 0; i < vec_len; i++) { 
            x[i] = sampled_float(
                UNDERFLOW_BOUND, 
                OVERFLOW_BOUND, 
                n_samples, 
                idx + i
            ); 
        }

        FloatVectorRef vex_input = { 
            .data = x, 
            .len  = (size_t) vec_len 
        }; 

        FloatVectorMut vex_output = { 
            .data = vex_out, 
            .len  = (size_t) vec_len 
        }; 

        vex_vexpf(vex_input, vex_output); 
        vvexpf(lib_out, x, &vec_len); 

        for (int32_t i = 0; i < vec_len; i++) { 
            record_ulp_result(&report, x[i], vex_out[i] , lib_out[i]);
        }
    }   

    display_ulp_report(&report);
    free_report(&report);
}


int main(void) { 
    uint64_t n_samples = 100000000; 

    printf("\nscalar expf\n"); 
    scalar(n_samples); 
    printf("\nvectorized expf\n"); 
    vectorized(n_samples);

    return 0; 
}
