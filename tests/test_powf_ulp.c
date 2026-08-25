//! test_powf_ulp.c
//!
//! sweeps over many float input pairs and reports ULP errors.


#include "vex.h"
#include "test_helpers.h"

#include <Accelerate/Accelerate.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define VEC_LENGTH 4

const float BASE_LOW = 0x1p-10f;
const float BASE_HIGH = 0x1p10f;
const float EXPONENT_LOW = -10.0f;
const float EXPONENT_HIGH = 10.0f;


static uint64_t exponent_index(uint64_t idx, uint64_t n_samples) {
    return (idx * 48271u) % n_samples;
}


void scalar(uint64_t n_samples) {
    if (n_samples < 2) {
        printf("not enough samples.\n");
        return;
    }

    BinaryULPReport report = empty_binary_report();
    report.n_samples = n_samples;
    report.counts = calloc(MAX_ULP + 1, sizeof(*report.counts));

    if (report.counts == NULL) {
        printf("calloc failed.\n");
        return;
    }

    for (uint64_t idx = 0; idx < n_samples; idx++) {
        float x = sampled_float(BASE_LOW, BASE_HIGH, n_samples, idx);
        float y = sampled_float(
            EXPONENT_LOW,
            EXPONENT_HIGH,
            n_samples,
            exponent_index(idx, n_samples)
        );

        float vex_result = vex_powf(x, y);
        float lib_result = powf(x, y);
        record_binary_ulp_result(
            &report,
            x,
            y,
            vex_result,
            lib_result
        );
    }

    display_binary_ulp_report(&report);
    free_binary_report(&report);
}


void vectorized(uint64_t n_samples) {
    if (n_samples < 2) {
        printf("not enough samples.\n");
        return;
    }

    BinaryULPReport report = empty_binary_report();
    report.n_samples = n_samples;
    report.counts = calloc(MAX_ULP + 1, sizeof(*report.counts));

    if (report.counts == NULL) {
        printf("calloc failed.\n");
        return;
    }

    for (uint64_t idx = 0; idx < n_samples; idx += VEC_LENGTH) {
        uint64_t remaining = n_samples - idx;
        int32_t vec_len = remaining > VEC_LENGTH
            ? VEC_LENGTH
            : (int32_t) remaining;

        float x[VEC_LENGTH];
        float y[VEC_LENGTH];
        float vex_out[VEC_LENGTH];
        float lib_out[VEC_LENGTH];

        for (int32_t i = 0; i < vec_len; i++) {
            uint64_t sample = idx + (uint64_t) i;
            x[i] = sampled_float(BASE_LOW, BASE_HIGH, n_samples, sample);
            y[i] = sampled_float(
                EXPONENT_LOW,
                EXPONENT_HIGH,
                n_samples,
                exponent_index(sample, n_samples)
            );
        }

        FloatVectorRef base_input = {
            .data = x,
            .len = (size_t) vec_len
        };
        FloatVectorRef exponent_input = {
            .data = y,
            .len = (size_t) vec_len
        };
        FloatVectorMut vex_output = {
            .data = vex_out,
            .len = (size_t) vec_len
        };

        vex_vpowf(base_input, exponent_input, vex_output);
        vvpowf(lib_out, y, x, &vec_len);

        for (int32_t i = 0; i < vec_len; i++) {
            record_binary_ulp_result(
                &report,
                x[i],
                y[i],
                vex_out[i],
                lib_out[i]
            );
        }
    }

    display_binary_ulp_report(&report);
    free_binary_report(&report);
}


int main(void) {
    uint64_t n_samples = 100000000;

    printf("\nscalar powf\n");
    scalar(n_samples);
    printf("\nvectorized powf\n");
    vectorized(n_samples);

    return 0;
}
