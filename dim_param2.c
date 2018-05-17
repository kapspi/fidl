/* Copyright 7/13/15 Washington University.  All Rights Reserved.
   dim_param2.c  $Revision: 1.11 $*/
//#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filetype.h"
#include "dim_param2.h"
#include "ifh.h"
#include "shouldiswap.h"
#include "nifti_getDimensions.h"
#include "cifti_getDimensions.h"
#include "filetype.h"

Dim_Param2 *dim_param2(size_t nfiles,char **files,int SunOS_Linux){
    size_t i;
    int64_t xdim,ydim,zdim,tdim;
    int lenvol;
    Dim_Param2 *dp;
    Interfile_header *ifh=NULL;
    if(!(dp=malloc(sizeof*dp))){
        printf("fidlError: dim_param2 Unable to malloc dp\n");
        return NULL;
        }
    if(!(dp->filetype=malloc(sizeof*dp->filetype*nfiles))){
        printf("fidlError: dim_param2 Unable to malloc dp->filetype\n");
        return NULL;
        }
    for(i=0;i<nfiles;i++)if(!(dp->filetype[i]=get_filetype(files[i])))return NULL;
    if(!(dp->vol=malloc(sizeof*dp->vol*nfiles))){
        printf("fidlError: dim_param2 Unable to malloc dp->vol\n");
        return NULL;
        }
    if(!(dp->tdim=malloc(sizeof*dp->tdim*nfiles))){
        printf("fidlError: dim_param2 Unable to malloc dp->tdim\n");
        return NULL;
        }
    for(dp->filetypeall=dp->filetype[0],i=1;i<nfiles;i++)if(dp->filetype[i]!=dp->filetypeall){dp->filetypeall=-1;break;}
    if(dp->filetypeall==(int)IMG){ 
        if(!(dp->swapbytes=malloc(sizeof*dp->swapbytes*nfiles))){
            printf("fidlError: dim_param2 Unable to malloc dp->swapbytes\n");
            return NULL;
            }
        if(!(dp->bigendian=malloc(sizeof*dp->bigendian*nfiles))) {
            printf("fidlError: dim_param2 Unable to malloc dp->bigendian\n");
            return NULL;
            }
        if(!(dp->xdim=malloc(sizeof*dp->xdim*nfiles))){
            printf("fidlError: dim_param2 Unable to malloc dp->xdim\n");
            return NULL;
            }
        if(!(dp->ydim=malloc(sizeof*dp->ydim*nfiles))){
            printf("fidlError: dim_param2 Unable to malloc dp->ydim\n");
            return NULL;
            }
        if(!(dp->zdim=malloc(sizeof*dp->zdim*nfiles))){
            printf("fidlError: dim_param2 Unable to malloc dp->zdim\n");
            return NULL;
            }
        if(!(dp->dxdy=malloc(sizeof*dp->dxdy*nfiles))){
            printf("fidlError: dim_param2 Unable to malloc dp->dxdy\n");
            return NULL;
            }
        if(!(dp->dz=malloc(sizeof*dp->dz*nfiles))){
            printf("fidlError: dim_param2 Unable to malloc dp->dz\n");
            return NULL;
            }
        if(!(dp->orientation=malloc(sizeof*dp->orientation*nfiles))){
            printf("fidlError: dim_param2 Unable to malloc dp->orientation\n");
            return NULL;
            }
        }
    //START160729
    else{
        dp->swapbytes=NULL;
        dp->bigendian=NULL;
        dp->xdim=NULL;
        dp->ydim=NULL;
        dp->zdim=NULL;
        dp->dxdy=NULL;
        dp->dz=NULL;
        dp->orientation=NULL;
        }



    for(dp->tdim_max=dp->lenvol_max=dp->tdim_total=0,i=0;i<nfiles;i++){
        if(dp->filetype[i]==(int)IMG){
            if(!(ifh=read_ifh(files[i],(Interfile_header*)NULL)))return NULL;
            dp->swapbytes[i]=shouldiswap(SunOS_Linux,ifh->bigendian);
            dp->bigendian[i]=ifh->bigendian;
            dp->vol[i]=(dp->xdim[i]=ifh->dim1)*(dp->ydim[i]=ifh->dim2)*(dp->zdim[i]=ifh->dim3); 
            dp->dxdy[i]=ifh->voxel_size_1;dp->dz[i]=ifh->voxel_size_3;dp->orientation[i]=ifh->orientation;
            dp->tdim[i]=ifh->dim4<1?1:ifh->dim4;
            free_ifh(ifh,0);
            }
        else if(dp->filetype[i]==(int)CIFTI||dp->filetype[i]==(int)CIFTI_DTSERIES){
            if(!cifti_getDimensions(files[i],&tdim,&xdim))return 0;
            dp->vol[i]=(int)xdim;
            dp->tdim[i]=(int)tdim;
            }
        else if(dp->filetype[i]==(int)NIFTI){
            if(!nifti_getDimensions(files[i],&xdim,&ydim,&zdim,&tdim))return 0;
            dp->vol[i]=(int)(xdim*ydim*zdim);
            dp->tdim[i]=(int)tdim;
            }
        if((lenvol=dp->vol[i]*dp->tdim[i])>dp->lenvol_max)dp->lenvol_max=lenvol;
        if(dp->tdim[i]>dp->tdim_max)dp->tdim_max=dp->tdim[i];
        dp->tdim_total+=dp->tdim[i];
        }
    for(dp->volall=dp->vol[0],i=1;i<nfiles;i++)if(dp->vol[i]!=dp->volall){dp->volall=-1;break;}
    for(dp->tdimall=dp->tdim[0],i=1;i<nfiles;i++)if(dp->tdim[i]!=dp->tdimall){dp->tdimall=-1;break;}
    if(dp->filetypeall==(int)IMG)
        for(dp->bigendianall=dp->bigendian[0],i=1;i<nfiles;i++)if(dp->bigendian[i]!=dp->bigendianall){dp->bigendianall=-1;break;}
    return dp;
    }
void free_dim_param2(Dim_Param2 *dp){

    //START160729
    if(dp->orientation)free(dp->orientation);
    if(dp->dz)free(dp->dz);
    if(dp->dxdy)free(dp->dxdy);
    if(dp->zdim)free(dp->zdim);
    if(dp->ydim)free(dp->ydim);
    if(dp->xdim)free(dp->xdim);
    if(dp->bigendian)free(dp->bigendian);
    if(dp->swapbytes)free(dp->swapbytes);

    if(dp->tdim)free(dp->tdim);
    if(dp->vol)free(dp->vol);
    if(dp->filetype)free(dp->filetype);
    free(dp);
    }
