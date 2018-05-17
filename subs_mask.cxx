/* Copyright 7/14/17 Washington University.  All Rights Reserved.
   subs_mask.cxx  $Revision: 1.1 $ */
#include <stdlib.h>
#include "checkOS.h"
#include "subs_glm.h"
#include "subs_util.h"
#include "shouldiswap.h"
#include "filetype.h"
#include "header.h"
#include "subs_mask.h"
Mask_Struct *get_mask_struct(char *mask_file,int vol,int *indices,int SunOS_Linux,LinearModel *glm_in,int msvol) {
    int i;
    Mask_Struct *ms;
    if(mask_file){
        ms=read_mask(mask_file,SunOS_Linux,glm_in);
        if(msvol&&(ms->vol!=msvol)){ //changed from vol to msvol for compressed files
            printf("fidlError: get_mask_struct ms->vol=%d vol=%d Must be equal.\n",ms->vol,vol);return NULL;
            }
        }
    else {
        if(!(ms=(Mask_Struct*)malloc(sizeof*ms))) {
            printf("fidlError: Unable to malloc ms in get_mask_struct\n");
            return NULL;
            }
        if(!indices) {
            ms->vol = ms->lenbrain = vol;
            if(!(ms->brnidx=(int*)malloc(sizeof*ms->brnidx*ms->lenbrain))) {
                printf("fidlError: Unable to malloc ms->brnidx in get_mask_struct\n");
                return NULL;
                }
            for(i=0;i<ms->lenbrain;i++) ms->brnidx[i] = i;
            ms->maskidx=ms->brnidx;
            }
        else {
            ms->lenbrain = vol;
            ms->brnidx = indices;
            ms->vol = msvol;
            if(!(ms->maskidx=(int*)malloc(sizeof*ms->maskidx*ms->vol))) {
                printf("fidlError: Unable to malloc ms->maskidx in get_mask_struct\n");
                return NULL;
                }
            for(i=0;i<ms->vol;i++) ms->maskidx[i]=-1;
            for(i=0;i<ms->lenbrain;i++) ms->maskidx[ms->brnidx[i]]=i;
            }
        ms->nuns=0;
        }
    return ms;
    }
Mask_Struct *read_mask(char *mask_file,int SunOS_Linux,LinearModel *glm_in) {
    Mask_Struct *ms=NULL;
    float *temp_float;
    int i,j,k,start_b=0,swapbytes=0; 
    FILE *fp;
    LinearModel *glm;
    header hdr;
    if(!(ms=(Mask_Struct*)malloc(sizeof*ms))) {
        printf("Error: Unable to malloc ms in read_mask\n");
        return NULL;
        }
    hdr.header0(mask_file);

    if(hdr.getfiletype()==(int)GLM){
        if(!glm_in) {
            if(!(glm=read_glm(mask_file,0,SunOS_Linux))) {
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
        start_b = find_b(glm);
        swapbytes = shouldiswap(SunOS_Linux,glm->ifh->bigendian);
        if(!glm_in) free_glm(glm);
        }
    else{
        hdr.assign(ms->xdim,ms->ydim,ms->zdim,ms->voxel_size_1,ms->voxel_size_2,ms->voxel_size_3,ms->center,ms->mmppix);
        }
    ms->vol=ms->xdim*ms->ydim*ms->zdim;
    if(!(temp_float=(float*)malloc(sizeof*temp_float*ms->vol))) {
        printf("Error: Unable to malloc temp_float\n");
        return NULL;
        }
    if(hdr.getfiletype()==(int)GLM){
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
    else{
        hdr.getvol(temp_float,SunOS_Linux);
        }
    if(!(ms->maskidx=(int*)malloc(sizeof*ms->maskidx*ms->vol))) {
        printf("Error: Unable to malloc ms->maskidx\n");
        return NULL;
        }
    for(i=0;i<ms->vol;i++) ms->maskidx[i] = -1;
    for(ms->lenbrain=i=0;i<ms->vol;i++) { /*Could be binary mask or image.*/
        if(temp_float[i]>(float)UNSAMPLED_VOXEL || temp_float[i]<(-(float)UNSAMPLED_VOXEL)) ms->maskidx[i] = ms->lenbrain++;
        }
    if(!(ms->brnidx=(int*)malloc(sizeof*ms->brnidx*ms->lenbrain))) {
        printf("fidlError: Unable to malloc ms->brnidx\n");
        return NULL;
        }
    ms->nuns=ms->vol-ms->lenbrain;
    if(!(ms->unsi=(int*)malloc(sizeof*ms->unsi*ms->nuns))) {
        printf("fidlError: Unable to malloc ms->unsi\n");
        return NULL;
        }
    for(k=j=i=0;i<ms->vol;i++)if(ms->maskidx[i]>=0){ms->brnidx[j++]=i;}else{ms->unsi[k++]=i;}
    if(j!=ms->lenbrain) {
        printf("fidlError: j=%d ms->lenbrain=%d Must be equal\n",j,ms->lenbrain);
        return NULL;
        }
    if(k!=ms->nuns) {
        printf("fidlError: k=%d ms->nuns=%d Must be equal\n",k,ms->nuns);
        return NULL;
        }
    free(temp_float);
    return ms;
    }
void free_mask(Mask_Struct *ms){
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
