//! test_helpers.h 
//! 
//! imports and helpers for running tests. 


#ifndef TEST_HELPERS 
#define TEST_HELPERS 


#include <math.h> 
#include <Accelerate/Accelerate.h>
#include <stddef.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>


static inline bool nearly_equal(float actual, float expected)
{
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


#endif 
