/* Copyright 5/22/15 Washington University.  All Rights Reserved.
   cond_norm1.h  $Revision: 1.2 $ */
#ifndef __COND_NORM1_H__
    #define __COND_NORM1_H__
    #ifdef __cplusplus
        extern "C" {
    #endif
    int cond_norm1(double *A,int m,double *cond,double *Ainv,gsl_permutation *perm);
    double array_norm1(double *A,int m);
    #ifdef __cplusplus
        }//extern
    #endif
#endif

