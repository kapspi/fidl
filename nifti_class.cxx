/* Copyright 6/19/17 Washington University.  All Rights Reserved.
   nifti_class.cxx  $Revision: 1.1 $*/

#include <inttypes.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "nifti_gethdr.h" 
#include "subs_nifti.h" 
#include "nifti_flipslice.h" 
#include "nifti_class.h" 
using namespace std;

int niftiStack::niftiStack0(char* file){
    char string[4];
    if(!(nifti_gethdr(file,&xdim64,&ydim64,&zdim64,&tdim64,center,mmppix,c_orient)))return 0;
    if(!(niftiPtr=nifti_openRead(file)))return 0;
    sprintf(string,"%d%d%d",c_orient[0],c_orient[1],c_orient[2]);
    if((cf_flip=(int)strtol(string,NULL,10))!=52)lcflip=1;
    return 1;
    }
niftiStack::~niftiStack(){
    nifti_free(niftiPtr);
    }

int niftiStack::niftiVol(int64_t volidx,float* temp_float,int flipit){
    if(!nifti_getvol(niftiPtr,tdim64>1?volidx:-1,temp_float))return 0;
    if(flipit&&lcflip){
        //for(float* fptr=temp_float,int64_t i64=0;i64<zdim64;i64++,fptr+=xdim64*ydim64){
        float* fptr;
        int64_t i64;
        for(fptr=temp_float,i64=0;i64<zdim64;i64++,fptr+=xdim64*ydim64){
            if(!nifti_flipslice(xdim64,ydim64,cf_flip,fptr))return 0;
            }
        }
    return 1;
    }
