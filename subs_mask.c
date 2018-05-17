/* Copyright 11/09/05 Washington University.  All Rights Reserved.
   subs_mask.c  $Revision: 1.28 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fidl.h"
#include "subs_mask.h"
#include "shouldiswap.h"
#include "subs_util.h"
#include "checkOS.h"
Mask_Struct *get_mask_struct(char *mask_file,int vol,int *indices,int SunOS_Linux,LinearModel *glm_in,int mslenvol) {
    int i;
    Mask_Struct *ms;
    if(mask_file){
        ms=read_mask(mask_file,SunOS_Linux,glm_in);

        //START170227
        //if(vol&&(ms->lenvol!=vol)){
        //    printf("fidlError: get_mask_struct ms->lenvol=%d vol=%d Must be equal.\n",ms->lenvol,vol);return NULL;
        //    }
        //START170303
        if(mslenvol&&(ms->vol!=mslenvol)){ //changed from vol to mslenvol for compressed files
            printf("fidlError: get_mask_struct ms->vol=%d vol=%d Must be equal.\n",ms->vol,vol);return NULL;
            }

        }
    else {
        if(!(ms=malloc(sizeof*ms))) {
            printf("Error: Unable to malloc ms in get_mask_struct\n");
            return NULL;
            }
        if(!indices) {
            ms->vol = ms->lenbrain = vol;
            if(!(ms->brnidx=malloc(sizeof*ms->brnidx*ms->lenbrain))) {
                printf("Error: Unable to malloc ms->brnidx in get_mask_struct\n");
                return NULL;
                }
            for(i=0;i<ms->lenbrain;i++) ms->brnidx[i] = i;
            ms->maskidx=ms->brnidx;
            }
        else {
            ms->lenbrain = vol;
            ms->brnidx = indices;
            ms->vol = mslenvol;
            if(!(ms->maskidx=malloc(sizeof*ms->maskidx*ms->vol))) {
                printf("Error: Unable to malloc ms->maskidx in get_mask_struct\n");
                return NULL;
                }
            for(i=0;i<ms->vol;i++) ms->maskidx[i]=-1;
            for(i=0;i<ms->lenbrain;i++) ms->maskidx[ms->brnidx[i]]=i;
            }

        //START160929
        ms->nuns=0;

        }
    return ms;
    }
Mask_Struct *read_mask(char *mask_file,int SunOS_Linux,LinearModel *glm_in) {
    Mask_Struct *ms=NULL;
    Interfile_header *ifh;
    float *temp_float;
    int i,j,k,start_b,swapbytes; 
    char *strptr;
    FILE *fp;
    LinearModel *glm;
    if(!(ms=malloc(sizeof*ms))) {
        printf("Error: Unable to malloc ms in read_mask\n");
        return NULL;
        }
    strptr=strrchr(mask_file,'.');
    if(strstr(strptr,".img")) {
        if(!(ifh=read_ifh(mask_file,(Interfile_header*)NULL))) return NULL;
        ms->xdim = ifh->dim1;
        ms->ydim = ifh->dim2;
        ms->zdim = ifh->dim3;
        ms->vol = ifh->dim1*ifh->dim2*ifh->dim3;
        ms->voxel_size_1 = ifh->voxel_size_1;
        ms->voxel_size_2 = ifh->voxel_size_2;
        ms->voxel_size_3 = ifh->voxel_size_3;

        //START161104
        for(i=0;i<3;i++){ms->center[i]=ifh->center[i];ms->mmppix[i]=ifh->mmppix[i];}


        if(!(temp_float=malloc(sizeof*temp_float*ms->vol))) {
            printf("Error: Unable to malloc temp_float in read_mask\n");
            return NULL;
            }
        if(!readstack(mask_file,(float*)temp_float,sizeof(float),(size_t)ms->vol,SunOS_Linux,ifh->bigendian)) return NULL;
        free_ifh(ifh,0);
        }
    else if(strstr(strptr,".glm")) {
        if(!glm_in) {
            if(!(glm=read_glm(mask_file,(int)FALSE,SunOS_Linux))) {
                printf("Error: Unable to read %s  Abort!\n",mask_file);
                return NULL;
                }
            }
        else {
            glm = glm_in;
            }
        ms->xdim = glm->ifh->glm_xdim;
        ms->ydim = glm->ifh->glm_ydim;
        ms->zdim = glm->ifh->glm_zdim;
        ms->vol = glm->ifh->glm_xdim*glm->ifh->glm_ydim*glm->ifh->glm_zdim;
        start_b = find_b(glm);
        swapbytes = shouldiswap(SunOS_Linux,glm->ifh->bigendian);
        if(!glm_in) free_glm(glm);
        if(!(temp_float=malloc(sizeof*temp_float*ms->vol))) {
            printf("Error: Unable to malloc temp_float\n");
            return NULL;
            }
        if(!(fp=fopen_sub(mask_file,"r"))) return NULL;
        if(fseek(fp,(long)start_b,(int)SEEK_SET)) {
            printf("Error: occured while seeking to %d in %s.\n",start_b,mask_file);
            return NULL;
            }
        if(!fread_sub(temp_float,sizeof(float),(size_t)ms->vol,fp,swapbytes,mask_file)) {
            printf("Error: reading parameter estimates from %s.\n",mask_file);
            return NULL;
            }
        fclose(fp);
        }
    else {
        printf("Error: mask_file = %s  Must have either .img or .glm extension.\n",mask_file);
        return NULL;
        }
    if(!(ms->maskidx=malloc(sizeof*ms->maskidx*ms->vol))) {
        printf("Error: Unable to malloc ms->maskidx\n");
        return NULL;
        }
    for(i=0;i<ms->vol;i++) ms->maskidx[i] = -1;
    for(ms->lenbrain=i=0;i<ms->vol;i++) { /*Could be binary mask or image.*/
        if(temp_float[i]>(float)UNSAMPLED_VOXEL || temp_float[i]<(-(float)UNSAMPLED_VOXEL)) ms->maskidx[i] = ms->lenbrain++;
        }
    if(!(ms->brnidx=malloc(sizeof*ms->brnidx*ms->lenbrain))) {
        printf("fidlError: Unable to malloc ms->brnidx\n");
        return NULL;
        }

    //for(j=i=0;i<ms->lenvol;i++) if(ms->maskidx[i]>=0) ms->brnidx[j++] = i;
    //START160502
    ms->nuns=ms->vol-ms->lenbrain;
    if(!(ms->unsi=malloc(sizeof*ms->unsi*ms->nuns))) {
        printf("fidlError: Unable to malloc ms->unsi\n");
        return NULL;
        }
    for(k=j=i=0;i<ms->vol;i++)if(ms->maskidx[i]>=0){ms->brnidx[j++]=i;}else{ms->unsi[k++]=i;}


    if(j!=ms->lenbrain) {
        printf("fidlError: j=%d ms->lenbrain=%d Must be equal\n",j,ms->lenbrain);
        return NULL;
        }

    //START160502
    if(k!=ms->nuns) {
        printf("fidlError: k=%d ms->nuns=%d Must be equal\n",k,ms->nuns);
        return NULL;
        }

    free(temp_float);
    return ms;
    }

void free_mask(Mask_Struct *ms){

    //START160502
    free(ms->unsi);

    free(ms->brnidx);
    free(ms->maskidx);
    free(ms);
    }
int _read_mask(int argc,char **argv){
    char *mask = (char *)argv[0];
    int SunOS_Linux,lenbrain;
    Mask_Struct *ms;
    if((SunOS_Linux=checkOS())==-1)return 0;
    if(!(ms=read_mask(mask,SunOS_Linux,(LinearModel*)NULL)))return 0;
    lenbrain=ms->lenbrain;
    free_mask(ms);
    return lenbrain;
    }
