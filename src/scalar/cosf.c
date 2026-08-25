//! scalar/cosf.c
//!
//! scalar implementation of `cosf(x)`.


#include "vex.h"
#include "helpers.h"
#include "trig_polys.h"

#include <math.h>
#include <stdint.h>


float vex_cosf(float x) {
    const float RANGE_LIMIT = 0x1p20f;

    // INFINITY and NaN
    if (!isfinite(x)) {
        return x - x;
    }

    float ax = asfloat(asuint(x) & 0x7fffffffu);

    if (ax > RANGE_LIMIT) {
        return cosf(x);
    }

    double xd = (double) ax;
    double nd = nearbyint(xd * INV_PIO2);
    int32_t n = (int32_t) nd;
    uint32_t quadrant = (uint32_t) n & 3u;

    // [-PI/4, PI/4]
    double r = (xd - nd * PIO2_HI) - nd * PIO2_LO;

    switch (quadrant) {
        case 0:
            return cos_poly(r);
        case 1:
            return -sin_poly(r);
        case 2:
            return -cos_poly(r);
        default:
            return sin_poly(r);
    }
}
