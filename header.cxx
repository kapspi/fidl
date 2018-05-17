/* Copyright 7/13/17 Washington University.  All Rights Reserved.
   header.cxx  $Revision: 1.1 $*/
#include <inttypes.h>
#include <math.h>
#include "subs_nifti.h"
#include "subs_util.h"
#include "nifti_gethdr.h"
#include "ifh.h"
#include "filetype.h"
#include "header.h"
int header::header0(char* file0){
    file=file0;
    if(!(filetype=get_filetype(file)))return 0;
    if(filetype==(int)GLM){
        //do nothing

        //START180319
        dim[0]=dim[1]=dim[2]=0;

        }
    else if(filetype==(int)IMG){ 
        Interfile_header* ifh;
        if(!(ifh=read_ifh(file,(Interfile_header*)NULL)))return 0;
        dim[0]=ifh->dim1;dim[1]=ifh->dim2;dim[2]=ifh->dim3;dim[3]=ifh->dim4;
        voxel_size[0]=ifh->voxel_size_1;voxel_size[1]=ifh->voxel_size_2;voxel_size[2]=ifh->voxel_size_3;
        for(int i=0;i<3;i++){centerf[i]=ifh->center[i];mmppixf[i]=ifh->mmppix[i];}
        bigendian=ifh->bigendian;
        free_ifh(ifh,0);
        }
    else if(filetype==(int)NIFTI){
        int64_t xdim64,ydim64,zdim64,tdim64;
        if(!(nifti_gethdr(file,&xdim64,&ydim64,&zdim64,&tdim64,centerf,mmppixf,c_orient)))return 0;

        //START180223
        dim64[0]=xdim64;dim64[1]=ydim64;dim64[2]=zdim64;dim64[3]=tdim64;

        dim[0]=(int)xdim64;dim[1]=(int)ydim64;dim[2]=(int)zdim64;dim[3]=(int)tdim64;
        for(int i=0;i<3;i++)voxel_size[i]=fabsf(mmppixf[i]); 
        }

    //return 1;
    //START180319
    return vol=dim[0]*dim[1]*dim[2];

    }
void header::assign(int& xdim,int& ydim,int& zdim){
    xdim=dim[0];ydim=dim[1];zdim=dim[2];
    }
void header::assign(int& xdim,int& ydim,int& zdim,float& voxel_size_1,float& voxel_size_2,float& voxel_size_3,float *center,
    float *mmppix){
    xdim=dim[0];ydim=dim[1];zdim=dim[2];
    voxel_size_1=voxel_size[0];voxel_size_2=voxel_size[1];voxel_size_3=voxel_size[2];
    for(int i=0;i<3;i++){center[i]=centerf[i];mmppix[i]=mmppixf[i];}
    }
int header::getvol(float *temp_float,int SunOS_Linux){

    //START180319
    //size_t vol=dim[0]*dim[1]*dim[2];

    if(filetype==(int)GLM){
        //do nothing
        }
    else if(filetype==(int)IMG){ 

        //if(!readstack(file,(float*)temp_float,sizeof*temp_float,vol,SunOS_Linux,bigendian))return 0;
        //START180319
        if(!readstack(file,(float*)temp_float,sizeof*temp_float,(size_t)vol,SunOS_Linux,bigendian))return 0;

        }
    else if(filetype==(int)NIFTI){
        if(!nifti_read(file,temp_float))return 0;
        }
    return 1;
    }

//START180319
#if 0
int header::getvol(){
    return vol;
    }
int* header::get_c_orient(){
    return c_orient;
    }
int64_t* header::get_dim64(){
    return dim64;
    }
#endif
