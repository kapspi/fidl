/* Copyright 10/5/15 Washington University.  All Rights Reserved.
   gsl_svd.h  $Revision: 1.3 $ */
#ifndef __GSL_SVD_H__
    #define __GSL_SVD_H__
    #ifdef __cplusplus
        extern "C" {
    #endif
    double gsl_svd_golubreinsch(double *A,int m,int n,double tol,double *V,double *S,double *Ainv,double *cond_norm2,int *ndep, 
        double *work);
    double gsl_svd_jacobi(double *A,int m,int n,double tol,double *V,double *S,double *Ainv,double *cond_norm2,int *ndep);
    double gsl_svd_modgolubreinsch(double *A,int m,int n,double tol,double *V,double *S,double *Ainv,double *cond_norm2,int *ndep,
        double *work,double *X);
    #ifdef __cplusplus
        }//extern
    #endif
#endif
