/* Copyright 2/21/18 Washington University.  All Rights Reserved.
   stack.cxx  $Revision: 1.1 $ */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "checkOS.h"
#include "stack.h"
#include "nifti_flipslice.h"
using namespace std;

float* stack::stack0(char *file,int SunOS_Linux){
    if(!header0(file))return NULL;
    vol=header::getvol();
    try{

        //float* f=new float[vol];
        //START180314
        f=new float[vol];

        header::getvol(f,SunOS_Linux);
        return f;
        }
    catch(bad_alloc& ba){
        cout<<"bad_alloc caught: "<<ba.what()<<endl;
        return NULL;
        }
    }

//START180314
int stack::stack1(char *file,int SunOS_Linux,float *f){
    if(!header0(file))return 0;
    header::getvol(f,SunOS_Linux);
    return 1;
    }


int stack::getvol(){
    return vol;
    }

//START180223
//float* stack::flip(){
//float* stack::flip(float* f0){
//START180226
int stack::flip(float* f0){
    char string[4];
    int cf_flip;
    int* c_orient=get_c_orient();
    sprintf(string,"%d%d%d",c_orient[0],c_orient[1],c_orient[2]);
    if((cf_flip=(int)strtol(string,NULL,10))!=52){
        int64_t* dim64=get_dim64();
        int64_t i64;
        float *fptr; 

        #if 0
        header::getvol(f,1);
        cout<<"here6"<<endl;
        cout<<"here6 f0[5399059]="<<f0[5399059]<<endl;
        cout<<"here6 f[5399059]="<<f[5399059]<<endl;

        try{
            float* f=new float[vol];
            header::getvol(f,1);
            }
        catch(bad_alloc& ba){
            cout<<"bad_alloc caught: "<<ba.what()<<endl;
            return NULL;
            }
        #endif

        #if 0
        //Doens't work. f is not preserved.
        for(fptr=f,i64=0;i64<dim64[2];i64++,fptr+=dim64[0]*dim64[1]){
            if(!nifti_flipslice(dim64[0],dim64[1],cf_flip,fptr))return NULL;
            }
        #endif
        for(fptr=f0,i64=0;i64<dim64[2];i64++,fptr+=dim64[0]*dim64[1]){
            //if(!nifti_flipslice(dim64[0],dim64[1],cf_flip,fptr))return NULL;
            //START180226
            if(!nifti_flipslice(dim64[0],dim64[1],cf_flip,fptr))return -1;
            }
        }
    //return f0;
    //START180226
    return cf_flip;
    }
