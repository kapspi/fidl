/* Copyright 7/14/15 Washington University.  All Rights Reserved.
   subs_nifti.h  $Revision: 1.5 $ */
#ifndef __SUBS_NIFTI_H__
    #define __SUBS_NIFTI_H__

    #ifdef __cplusplus
        extern "C" {
    #endif

    void* nifti_openRead(char* file);
    int nifti_getvol(void* niftiPtr,int64_t t,float *stack);
    void nifti_free(void* niftiPtr);
    int nifti_read(char *file,float *stack);
    int nifti_write2(char *file,char *file_hdr,float *stack);
    int nifti_write(char *file,int64_t *dims,float *center,float *mmppix,float *stack);
    void* nifti_writeNew(char *file,int64_t *dims,float *center,float *mmppix);
    int nifti_putvol(void* niftiPtr,int64_t t,float *stack);

    #ifdef __cplusplus
        }//extern
    #endif
#endif
