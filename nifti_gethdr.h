/* Copyright 7/30/15 Washington University.  All Rights Reserved.
   nifti_gethdr.h  $Revision: 1.2 $*/
#ifndef __NIFTI_GETHDR_H__
    #define __NIFTI_GETHDR_H__
    #ifdef __cplusplus
        extern "C" {
    #endif
    int nifti_gethdr(char *file,int64_t *xdim,int64_t *ydim,int64_t *zdim,int64_t *tdim,float *center,float *mmppix,int *c_orient);
    int _nifti_gethdr(int argc,char **argv);
    #ifdef __cplusplus
        }//extern
    #endif
#endif
