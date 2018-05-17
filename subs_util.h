/* Copyright 8/13/15 Washington University.  All Rights Reserved.
   subs_util.h  $Revision: 1.3 $*/
#ifndef __SUBS_UTIL_H__
    #define __SUBS_UTIL_H__ 

    #include <stdio.h>

    //#include "fidl.h"

    #ifdef __cplusplus
        extern "C" {
    #endif

    FILE *fopen_sub(char *filename,const char *mode);
    int fwrite_sub(void *ptr,size_t size,size_t nitems,FILE *fp,int swapbytes);
    int writestack(char *filename,void *x,size_t size,size_t nitems,int swapbytes);
    void swap_bytes(unsigned char *buf,size_t size,size_t nitems);
    void swap_bytes_idx(unsigned char *buf,size_t size,int lenbrain,int *brnidx);
    int fread_sub(void *ptr,size_t size,size_t nitems,FILE *fp,int swapbytes,char *filename);
    int readstack(char *infile,void *x,size_t size,size_t nitems,int SunOS_Linux,int bigendian);

    //START170714
    //int crs(double *temp_double,double *region_stack,Regions_By_File *rbf,char *glmfile);

    #ifdef __cplusplus
        }//extern
    #endif
#endif
