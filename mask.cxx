/* Copyright 11/1/17 Washington University.  All Rights Reserved.
   mask.cxx  $Revision: 1.1 $ */

//#include <stdlib.h>
//START180207
#include <iostream>

#include "checkOS.h"
#include "subs_util.h"
#include "shouldiswap.h"
#include "filetype.h"
#include "mask.h"

//START180207
using namespace std;

int mask::read_mask(char *mask_file,int SunOS_Linux,LinearModel *glm_in){

    //START180207
    //float *temp_float;

    long start_b=0;
    int i,j,k,swapbytes=0; 
    FILE *fp;
    LinearModel *glm;
    if(!header0(mask_file))return 0;
    if(getfiletype()==(int)GLM){
        if(!glm_in) {
            if(!(glm=read_glm(mask_file,0,SunOS_Linux))) {
                printf("fidlError: read_mask Unable to read %s  Abort!\n",mask_file);
                return 0;
                }
            }
        else {
            glm = glm_in;
            }
        assign(glm->ifh->glm_xdim,glm->ifh->glm_ydim,glm->ifh->glm_zdim,glm->ifh->voxel_size_1,glm->ifh->voxel_size_2,
            glm->ifh->voxel_size_3,glm->ifh->center,glm->ifh->mmppix);
        start_b = find_b(glm);
        swapbytes = shouldiswap(SunOS_Linux,glm->ifh->bigendian);
        if(!glm_in)free_glm(glm);
        }
    vol=getvol();


    #if 0
    if(!(temp_float=(float*)malloc(sizeof*temp_float*vol))) {
        printf("fidlError: read_mask Unable to malloc temp_float\n");
        return 0;
        }
    if(getfiletype()==(int)GLM){
        if(!(fp=fopen_sub(mask_file,"r")))return 0;
        if(fseek(fp,start_b,(int)SEEK_SET)){
            printf("fidlError: read_mask occured while seeking to %d in %s.\n",start_b,mask_file);
            return 0;
            }
        if(!fread_sub(temp_float,sizeof(float),(size_t)vol,fp,swapbytes,mask_file)) {
            printf("fidlError: read_mask reading parameter estimates from %s.\n",mask_file);
            return 0;
            }
        fclose(fp);
        }
    else{
        getvol(temp_float,SunOS_Linux);
        }
    if(!(maskidx=(int*)malloc(sizeof*maskidx*vol))) {
        printf("fidlError: read_mask Unable to malloc maskidx\n");
        return 0;
        }
    for(i=0;i<vol;i++)maskidx[i]=-1;
    for(lenbrain=i=0;i<vol;i++) { /*Could be binary mask or image.*/
        if(temp_float[i]>(float)UNSAMPLED_VOXEL || temp_float[i]<(-(float)UNSAMPLED_VOXEL))maskidx[i]=lenbrain++;
        }
    if(!(brnidx=(int*)malloc(sizeof*brnidx*lenbrain))) {
        printf("fidlError: read_mask Unable to malloc brnidx\n");
        return 0;
        }
    nuns=vol-lenbrain;
    if(!(unsi=(int*)malloc(sizeof*unsi*nuns))) {
        printf("fidlError: read_mask Unable to malloc unsi\n");
        return 0;
        }
    for(k=j=i=0;i<vol;i++)if(maskidx[i]>=0){brnidx[j++]=i;}else{unsi[k++]=i;}
    if(j!=lenbrain) {
        printf("fidlError: read_mask j=%d lenbrain=%d Must be equal\n",j,lenbrain);
        return 0;
        }
    if(k!=nuns){
        printf("fidlError: read_mask k=%d nuns=%d Must be equal\n",k,nuns);
        return 0;
        }
    free(temp_float);
    #endif
    //START180207
    try{
        float* temp_float=new float[vol];
        if(getfiletype()==(int)GLM){
            if(!(fp=fopen_sub(mask_file,"r")))return 0;
            if(fseek(fp,(long)start_b,(int)SEEK_SET)){
                cout<<"fidlError: read_mask occured while seeking to "<<start_b<<" in "<<mask_file<<endl;
                return 0;
                }
            if(!fread_sub(temp_float,sizeof(float),(size_t)vol,fp,swapbytes,mask_file)) {
                cout<<"fidlError: read_mask reading parameter estimates from "<<mask_file<<endl;
                return 0;
                }
            fclose(fp);
            }
        else{
            getvol(temp_float,SunOS_Linux);
            }
        maskidx=new int[vol];
        for(i=0;i<vol;i++)maskidx[i]=-1;
        for(lenbrain=i=0;i<vol;i++)  //Could be binary mask or image.
            if(temp_float[i]>(float)UNSAMPLED_VOXEL || temp_float[i]<(-(float)UNSAMPLED_VOXEL))maskidx[i]=lenbrain++;
        brnidx=new int[lenbrain];
        nuns=vol-lenbrain;
        unsi=new int[nuns];
        for(k=j=i=0;i<vol;i++)if(maskidx[i]>=0){brnidx[j++]=i;}else{unsi[k++]=i;}

        if(j!=lenbrain) {
            cout<<"fidlError: read_mask j="<<j<<" lenbrain="<<lenbrain<<" Must be equal."<<endl;
            return 0;
            }
        if(k!=nuns){
            cout<<"fidlError: read_mask k="<<k<<" nuns="<<nuns<<" Must be equal."<<endl;
            return 0;
            }
        delete[] temp_float;
        }
    catch(bad_alloc& ba){
        cout<<"bad_alloc caught: "<<ba.what()<<endl;
        }



    return 1;
    }

#if 0
mask::~mask(){
    free(unsi);
    free(brnidx);
    free(maskidx);
    }
#endif
//START180207
mask::~mask(){
    delete[] unsi;
    delete[] brnidx;
    delete[] maskidx;
    }


int mask::get_mask(char *mask_file,int vol0,int *indices,int SunOS_Linux,LinearModel *glm_in,int msvol){
    int i;
    if(mask_file){

        #if 0
        if(!(read_mask(mask_file,SunOS_Linux,glm_in))){printf("fidlError: get_mask from read_mask\n");return 0;}
        if(msvol&&(vol0!=msvol)){ //changed from vol0 to msvol for compressed files
            printf("fidlError: get_mask vol=%d vol0=%d Must be equal.\n",vol,vol0);return 0;
            }
        #endif
        //180207
        if(!(read_mask(mask_file,SunOS_Linux,glm_in))){
            cout<<"fidlError: get_mask from read_mask"<<endl;
            return 0;
            }
        if(msvol&&(vol0!=msvol)){ //changed from vol0 to msvol for compressed files
            cout<<"fidlError: get_mask vol="<<vol<<" vol0="<<vol0<<" Must be equal."<<endl;
            return 0;
            }

        }
    else {

        #if 0
        if(!indices) {
            vol=lenbrain=vol0;
            if(!(brnidx=(int*)malloc(sizeof*brnidx*lenbrain))){
                printf("fidlError: get_mask Unable to malloc brnidx\n");
                return 0;
                }
            for(i=0;i<lenbrain;i++)brnidx[i]=i;
            maskidx=brnidx;
            }
        else {
            lenbrain=vol0;
            brnidx=indices;
            vol=msvol;
            if(!(maskidx=(int*)malloc(sizeof*maskidx*vol))){
                printf("fidlError: get_mask Unable to malloc maskidx\n");
                return 0;
                }
            for(i=0;i<vol;i++)maskidx[i]=-1;
            for(i=0;i<lenbrain;i++)maskidx[brnidx[i]]=i;
            }
        #endif
        //START180207
        try{ 
            if(!indices) {
                vol=lenbrain=vol0;
                brnidx=new int[lenbrain];
                for(i=0;i<lenbrain;i++)brnidx[i]=i;
                maskidx=brnidx;
                }
            else {
                lenbrain=vol0;
                brnidx=indices;
                vol=msvol;
                maskidx=new int[vol];
                for(i=0;i<vol;i++)maskidx[i]=-1;
                for(i=0;i<lenbrain;i++)maskidx[brnidx[i]]=i;
                }
            }
        catch(bad_alloc& ba){
            cout<<"bad_alloc caught: "<<ba.what()<<endl;
            }

        nuns=0;
        }
    return 1;
    }

int* mask::get_brnidx(int& lenbrain0){
    lenbrain0=lenbrain;
    return brnidx;
    } 
int* mask::get_brnidx(int& lenbrain0,int &vol0){
    lenbrain0=lenbrain;
    vol0=vol;
    return brnidx;
    } 
int* mask::get_brnidx(int& lenbrain0,int& xdim,int& ydim,int& zdim){
    lenbrain0=lenbrain;
    assign(xdim,ydim,zdim);
    return brnidx;
    } 
int* mask::get_brnidx(int& lenbrain0,int &vol0,int& xdim,int& ydim,int& zdim){
    lenbrain0=lenbrain;
    vol0=vol;
    assign(xdim,ydim,zdim);
    return brnidx;
    }


int _read_mask(int argc,char **argv){
    char *maskf = (char *)argv[0];
    int SunOS_Linux,lenbrain;
    mask ms;
    if((SunOS_Linux=checkOS())==-1)return 0;
    if(!(ms.read_mask(maskf,SunOS_Linux,(LinearModel*)NULL)))return 0;

    //lenbrain=ms.get_lenbrain();
    //START180207
    //ms.get_param(lenbrain);
    ms.get_brnidx(lenbrain);

    ms.~mask();
    return lenbrain;
    }
