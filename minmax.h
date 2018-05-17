/* Copyright 8/13/15 Washington University.  All Rights Reserved.
   minmax.h  $Revision: 1.3 $ */

#ifdef __cplusplus
    extern "C" {
#endif

void min_and_max_init(float *min,float *max);
void min_and_max_intstack(int *stack,int size,float *min,float *max);
void min_and_max_floatstack(float *stack,int size,float *min,float *max);
void min_and_max_doublestack(double *stack,int size,float *min,float *max);
void min_and_max(float *stack,int size,float *min,float *max);
void min_and_max_new(float *stack,int size,float *min,float *max,int initialize);

#ifdef __cplusplus
    }//extern
#endif
