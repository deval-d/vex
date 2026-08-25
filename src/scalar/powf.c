//! scalar/powf.c
//!
//! scalar implementation of `powf(x, y)`.


#include "vex.h"
#include "helpers.h"

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>


static bool integer_exponent(float y, bool* odd) {
    float ay = fabsf(y);

    // float integers above 2^24 are even
    if (ay >= 0x1p24f) {
        *odd = false;
        return true;
    }

    if (truncf(y) != y) {
        *odd = false;
        return false;
    }

    *odd = ((int32_t) y & 1) != 0;
    return true;
}


static double log2_positive(float x) {
    const double INV_LN2 = 0x1.71547652b82fep+0;
    const float SQRT2 = 0x1.6a09e6p+0f;

    int32_t exponent_offset = 0;

    // normalize subnormals
    if (x < FLT_MIN) {
        x *= 0x1p23f;
        exponent_offset = -23;
    }

    uint32_t bits = asuint(x);
    int32_t exponent = (int32_t) (bits >> 23) - 127 + exponent_offset;
    float mantissa = asfloat((bits & 0x007fffffu) | 0x3f800000u);

    // mantissa in [1/sqrt(2), sqrt(2)]
    if (mantissa > SQRT2) {
        mantissa *= 0.5f;
        exponent++;
    }

    // ln(m) = 2s(1 + s^2/3 + s^4/5 + ...)
    double m = (double) mantissa;
    double s = (m - 1.0) / (m + 1.0);
    double z = s * s;

    double poly = 1.0 / 11.0;
    poly = fma(z, poly, 1.0 / 9.0);
    poly = fma(z, poly, 1.0 / 7.0);
    poly = fma(z, poly, 1.0 / 5.0);
    poly = fma(z, poly, 1.0 / 3.0);
    poly = fma(z, poly, 1.0);

    double log2_mantissa = (2.0 * s * poly) * INV_LN2;
    return (double) exponent + log2_mantissa;
}


static float exp2_float(double x) {
    const double LN2 = 0x1.62e42fefa39efp-1;

    if (x >= 128.0) {
        return INFINITY;
    }

    if (x <= -150.0) {
        return 0.0f;
    }

    double kd = nearbyint(x);
    int32_t k = (int32_t) kd;
    double r = x - kd;

    // 2^r on [-0.5, 0.5]
    double t = r * LN2;
    double poly = 1.0 / 40320.0;
    poly = fma(t, poly, 1.0 / 5040.0);
    poly = fma(t, poly, 1.0 / 720.0);
    poly = fma(t, poly, 1.0 / 120.0);
    poly = fma(t, poly, 1.0 / 24.0);
    poly = fma(t, poly, 1.0 / 6.0);
    poly = fma(t, poly, 0.5);
    poly = fma(t, poly, 1.0);
    poly = fma(t, poly, 1.0);

    return (float) scalbn(poly, k);
}


float vex_powf(float x, float y) {
    if (y == 0.0f || x == 1.0f) {
        return 1.0f;
    }

    uint32_t x_bits = asuint(x);
    uint32_t abs_bits = x_bits & 0x7fffffffu;
    float ax = asfloat(abs_bits);

    if (isnan(x) || isnan(y)) {
        return x + y;
    }

    if (isinf(y)) {
        if (ax == 1.0f) {
            return 1.0f;
        }

        bool grows = (ax > 1.0f) == (y > 0.0f);
        return grows ? INFINITY : 0.0f;
    }

    bool odd = false;
    bool y_is_integer = integer_exponent(y, &odd);
    bool negative_result = (x_bits >> 31) != 0 && odd;

    if (ax == 0.0f) {
        float magnitude = y > 0.0f ? 0.0f : INFINITY;
        return negative_result ? -magnitude : magnitude;
    }

    if (isinf(ax)) {
        float magnitude = y > 0.0f ? INFINITY : 0.0f;
        return negative_result ? -magnitude : magnitude;
    }

    if (x < 0.0f && !y_is_integer) {
        return NAN;
    }

    double power = (double) y * log2_positive(ax);
    float magnitude = exp2_float(power);

    return negative_result ? -magnitude : magnitude;
}
