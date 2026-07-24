//! bench_expf.c 


#include "vex.h"
#include "helpers.h" 
#include <stdio.h> 
#include <math.h> 
#include <time.h>


int main(void) {
    const float sentinel = 10.0f; 

    double vex_time; 
    double math_time; 

    vex_time  = benchmark_float(sentinel, vex_expf, 100000000);
    math_time = benchmark_float(sentinel, expf, 100000000); 

    printf("vex expf:  %.2f ns \n", vex_time * 1e9); 
    printf("math expf: %.2f ns\n", math_time * 1e9); 

    return 0; 
}   
