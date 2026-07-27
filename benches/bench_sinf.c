//! benches/bench_sinf.c 


#include "vex.h"
#include "bench_helpers.h" 
#include <stdio.h> 
#include <math.h>
#include <stdlib.h>


int scalar(void) {
    const float sentinel = 10.0f; 

    double vex_time; 
    double math_time; 

    vex_time  = benchmark_float(sentinel, vex_sinf, 100000000);
    math_time = benchmark_float(sentinel, sinf, 100000000); 

    printf("scalar vex sinf:  %.3f ns\n", vex_time * 1e9); 
    printf("scalar math sinf: %.3f ns\n", math_time * 1e9); 

    return 0; 
} 

int main(void) {
    int status = 0;

    printf("\nscalar\n");
    status |= scalar();

    return status;
}
