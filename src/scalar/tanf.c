//! scalar/tanf.c
//!
//! scalar implementation of `tanf(x)`.


#include "vex.h"
#include "helpers.h"
#include "trig_polys.h"

#include <float.h>
#include <math.h>
#include <stdint.h>


static inline float copy_sign(float value, uint32_t sign_mask) {
    return asfloat(asuint(value) ^ sign_mask);
}


float vex_tanf(float x) {
    const float RANGE_LIMIT = 0x1p20f;

    // INFINITY and NaN
    if (!isfinite(x)) {
        return x - x;
    }

    uint32_t bits = asuint(x);
    float ax = asfloat(bits & 0x7fffffffu);
    uint32_t sign_mask = bits & 0x80000000u;

    // subnormals tan(x) ~ x
    if (ax < FLT_MIN) {
        return x;
    }

    if (ax > RANGE_LIMIT) {
        return tanf(x);
    }

    double xd = (double) ax;
    double nd = nearbyint(xd * INV_PIO2);
    int32_t n = (int32_t) nd;

    // [-PI/4, PI/4]
    double r = (xd - nd * PIO2_HI) - nd * PIO2_LO;

    float sin_r = sin_poly(r);
    float cos_r = cos_poly(r);

    float result = ((uint32_t) n & 1u)
        ? -cos_r / sin_r
        :  sin_r / cos_r;

    return copy_sign(result, sign_mask);
}
