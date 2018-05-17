/* Copyright 8/13/15 Washington University.  All Rights Reserved.
   write1.c  $Revision: 1.12 $ */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "ifh.h"
#include "write1.h"
#include "filetype.h"
#include "minmax.h"
#include "subs_util.h"
#include "subs_cifti.h"
#include "subs_nifti.h"
#include "constants.h"
W1 *write1_init(){
    W1 *w1;
    if(!(w1=malloc(sizeof*w1))){
        printf("fidlError: write1_init Unable to malloc w1\n");
        return NULL;
        }
    w1->file_hdr=NULL;
    w1->ifh=NULL;
    w1->temp_double=NULL;
    w1->swapbytes=0;
    w1->filetype=(int)IMG;
    return w1;
    }
int write1(char *file,W1 *w1){
    char string[MAXNAME];
    if(w1->filetype==(int)IMG){
        if(w1->dontminmax!=1){
            min_and_max_init(&w1->ifh->global_min,&w1->ifh->global_max);
            if(w1->temp_double)min_and_max_doublestack(w1->temp_double,w1->lenbrain,&w1->ifh->global_min,&w1->ifh->global_max);
            else min_and_max_floatstack(w1->temp_float,w1->how_many,&w1->ifh->global_min,&w1->ifh->global_max);
            }
        if(!writestack(file,w1->temp_float,sizeof*w1->temp_float,(size_t)w1->how_many,w1->swapbytes))return 0;
        if(!write_ifh(file,w1->ifh,0))return 0;
        }
    else if(w1->filetype==(int)CIFTI){
        if(!cifti_writedscalar(file,w1->cifti_xmldata,w1->xmlsize,w1->temp_float))return 0;
        }
    else if(w1->filetype==(int)CIFTI_DTSERIES){
        if(!cifti_writedtseries(file,w1->cifti_xmldata,w1->xmlsize,w1->temp_float,w1->tdim,w1->dims[0]*w1->dims[1]*w1->dims[2]))
            return 0;
        }
    else{
        if(!w1->file_hdr){
            if(!nifti_write(file,w1->dims,w1->center,w1->mmppix,w1->temp_float))return 0;
            }
        else{
            if(!nifti_write2(file,w1->file_hdr,w1->temp_float))return 0;
            }

        #if 0
        strcpy(string,file);
        *strstr(string,".nii")=0;
        strcat(string,".4dfp.img");
        if(w1->dontminmax!=1){
            min_and_max_init(&w1->ifh->global_min,&w1->ifh->global_max);
            if(w1->temp_double)min_and_max_doublestack(w1->temp_double,w1->lenbrain,&w1->ifh->global_min,&w1->ifh->global_max);
            else min_and_max_floatstack(w1->temp_float,w1->how_many,&w1->ifh->global_min,&w1->ifh->global_max);
            }
        if(!writestack(string,w1->temp_float,sizeof*w1->temp_float,(size_t)w1->how_many,w1->swapbytes))return 0;
        if(!write_ifh(string,w1->ifh,0))return 0;
        printf("Output written to %s\n",string);
        #endif
        //START170620
        if(w1->ifh){
            strcpy(string,file);
            *strstr(string,".nii")=0;
            strcat(string,".4dfp.img");
            if(w1->dontminmax!=1){
                min_and_max_init(&w1->ifh->global_min,&w1->ifh->global_max);
                if(w1->temp_double)min_and_max_doublestack(w1->temp_double,w1->lenbrain,&w1->ifh->global_min,&w1->ifh->global_max);
                else min_and_max_floatstack(w1->temp_float,w1->how_many,&w1->ifh->global_min,&w1->ifh->global_max);
                }
            if(!writestack(string,w1->temp_float,sizeof*w1->temp_float,(size_t)w1->how_many,w1->swapbytes))return 0;
            if(!write_ifh(string,w1->ifh,0))return 0;
            printf("Output written to %s\n",string);
            }

        }
    return 1;
    }
void* open2(char *file,W1 *w1){
    void* ret=NULL;
    if(w1->filetype==(int)IMG){
        if(!(ret=fopen_sub(file,"w")))return NULL;
        }
    else if(w1->filetype==(int)CIFTI){
        if(!(ret=cifti_setdscalar(file,w1->cifti_xmldata,w1->xmlsize)))return NULL;
        }
    else if(w1->filetype==(int)CIFTI_DTSERIES){
        if(!(ret=cifti_setdtseries(file,w1->cifti_xmldata,w1->xmlsize,(int64_t)w1->ifh->glm_tdim)))return NULL;
        }
    else if(w1->filetype==(int)NIFTI){
        if(!(ret=nifti_writeNew(file,w1->dims,w1->center,w1->mmppix)))return NULL;
        }
    return ret;
    }
