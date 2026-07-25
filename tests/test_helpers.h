//! tests/test_helpers.h 
//! 
//! imports and helpers for running tests. 


#ifndef TEST_HELPERS 
#define TEST_HELPERS 


#include "helpers.h" 
#include <math.h> 
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#define MAX_ULP 256


typedef struct {
    uint64_t n_samples; 
    size_t* counts;
    uint32_t max_ulp;
    float worst_x;
    float worst_vex;
    float worst_lib;
} ULPReport;


/// Returns true if actual is nearly_equal to expected else false
static inline bool nearly_equal(float actual, float expected) {
    if (isnan(actual) || isnan(expected)) {
        return isnan(actual) && isnan(expected);
    }

    if (isinf(actual) || isinf(expected)) {
        return actual == expected;
    }

    const float abs_tol = 1e-6f;
    const float rel_tol = 1e-6f;
    const float diff = fabsf(actual - expected);

    return diff <= abs_tol + rel_tol * fabsf(expected);
}


static uint32_t ordered_float_bits(float x) { 
    uint32_t bits = asuint(x); 

    if (bits >> 31) { 
        return ~bits; 
    }

    return bits | (1u << 31); 
}


static float float_ordered_bits(uint32_t x) { 
    uint32_t bits; 
    
    // originally positive
    if (x >> 31) { 
        bits = x & 0x7fffffffu; 
    // originally negative 
    } else { 
        bits = ~x; 
    }

    return asfloat(bits); 
}


/// Returns the ULP distance between floats a and b 
uint32_t ulp_distance(float a, float b) { 
    if (a == b) { 
        return 0; 
    }

    uint32_t ia = ordered_float_bits(a); 
    uint32_t ib = ordered_float_bits(b); 

    uint32_t distance = ia > ib ? ia - ib : ib - ia; 

    // signs are different 
    // subtract 1 for +/- 0.0 being counted as separate ULPs
    uint32_t sign_a = asuint(a) >> 31; 
    uint32_t sign_b = asuint(b) >> 31; 
    if (sign_a != sign_b) { 
        distance -= 1; 
    }

    return distance; 
}


/// Returns the idx'th float in a uniform sampling 
/// between lo and hi using n_samples. 
float sampled_float( 
    float lo, 
    float hi, 
    uint64_t n_samples, 
    uint64_t idx 
) { 
    uint32_t lo_ordered = ordered_float_bits(lo); 
    uint32_t hi_ordered = ordered_float_bits(hi); 

    uint32_t span = (uint64_t) hi_ordered - lo_ordered; 
    uint32_t sampled_ordered = lo_ordered + idx * span / (n_samples - 1); 

    return float_ordered_bits(sampled_ordered); 
}


static ULPReport empty_report(void) { 
    return (ULPReport) { 
        .n_samples = 0, 
        .counts = NULL, 
        .max_ulp = 0, 
        .worst_x = 0.0f, 
        .worst_vex = 0.0f, 
        .worst_lib = 0.0f
    };
}


static void free_report(ULPReport* report) { 
    free(report -> counts); 
}       


static inline void display_ulp_report(ULPReport* report) { 
    if (report == NULL) { 
        printf("pointer is null.\n");
    }

    printf("maximum ULP: %" PRIu32 "\n", report -> max_ulp);
    printf("from input: %f\n", report -> worst_x); 
    printf("vex measurement: %f\n", report -> worst_vex); 
    printf("lib measurement: %f\n", report -> worst_lib); 

    uint64_t n_samples = report -> n_samples; 
    printf("ulp distribution\n"); 
    for (size_t ulp = 0; ulp <= MAX_ULP; ulp++) { 
        size_t count = report -> counts[ulp]; 

        if (count == 0) { 
            continue; 
        }

        float percent = 100.0f * (float) count / (float) n_samples; 

        if (ulp == MAX_ULP) { 
            printf("%-10s %12zu %5.5f%%\n", "256+", count, percent); 
        } else { 
            printf("%-10zu %12zu %5.5f%%\n", ulp, count, percent); 
        }
    }
    printf("total samples: %" PRIu64 "\n", n_samples); 
}


static inline void record_ulp_result(
    ULPReport* report, 
    float x, 
    float vex_result, 
    float lib_result 
) {
    uint32_t ulp_diff = ulp_distance(vex_result, lib_result); 

    if (ulp_diff > report -> max_ulp) { 
        report -> max_ulp = ulp_diff; 
        report -> worst_x = x; 
        report -> worst_vex = vex_result; 
        report -> worst_lib = lib_result; 
    } 

    size_t index = ulp_diff >= MAX_ULP 
        ? MAX_ULP 
        : (size_t) ulp_diff; 

    report -> counts[index]++; 
}


#endif 
