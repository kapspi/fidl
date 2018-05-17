/* Copyright 3/16/18 Washington University.  All Rights Reserved.
   region.cxx  $Revision: 1.1 $ */

//START180319
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "region.h"
#include "subs_util.h"
#include "constants.h"
#include "fidl.h"
#include "d2intvar.h"

//START180319
#include "header.h"
using namespace std;

Regions *get_reg(char *region_file,int fidl_aviseg_gt0,int vol,float *image,int nreg,int SunOS_Linux,char **ifh_region_names)
{
    char *strptr=NULL,string[MAXNAME],**region_names_ptr=NULL;
    int nregions,i,lc_freeimage=0,*regval;
    Regions *reg;
    Interfile_header *ifh=NULL;
    if(region_file) {
        if(!(ifh=read_ifh(region_file,(Interfile_header*)NULL))) return NULL;
        if(fidl_aviseg_gt0==0) {
            nregions = ifh->nregions;
            }
        else if(fidl_aviseg_gt0==1) {
            nregions = 3;
            }
        else if(fidl_aviseg_gt0==2) {
            nregions = 1;
            }
        else {
            printf("fidl_aviseg_gt0=%d Must be 0,1,2. Abort!\n",fidl_aviseg_gt0);
            return NULL;
            }
        region_names_ptr=ifh->region_names;
        }
    else {
        nregions = nreg;
        }
    if(!nregions) {
        printf("Error: No regions detected in %s\n",region_file);
        return NULL;
        }
    if(!image&&region_file) {

        #if 0
        vol = ifh->dim1*ifh->dim2*ifh->dim3;
        if(!(image=(float*)malloc(sizeof*image*vol))) {
            printf("Error: Unable to malloc image\n");
            return NULL;
            }
        lc_freeimage = 1;
        if(!readstack(region_file,(float*)image,sizeof(float),(size_t)vol,SunOS_Linux,ifh->bigendian)) return NULL;
        #endif
        //START180319
        header h;
        if(!(vol=h.header0(region_file)))return NULL;
        if(!(image=(float*)malloc(sizeof*image*vol))){
            cout<<"fidlError: Unable to malloc image"<<endl;
            return NULL;
            }
        lc_freeimage = 1;
        if(!h.getvol(image,SunOS_Linux))return NULL;

        }

    if(!(regval=(int*)malloc(sizeof*regval*nregions))) {
        printf("Error: Unable to malloc regval\n");
        return NULL;
        }
    if(!(reg=get_reg_guts(nregions,vol,image,fidl_aviseg_gt0,regval))) return NULL;
    reg->vol = vol;
    if(!(reg->harolds_num=(int*)malloc(sizeof*reg->harolds_num*nregions))) {
        printf("Error: Unable to malloc reg->harolds_num\n");
        return NULL;
        }
    if(ifh_region_names) region_names_ptr=ifh_region_names;
    if(region_file||ifh_region_names) {
        if(!(reg->length=(size_t*)malloc(sizeof*reg->length*nregions))) {
            printf("Error: Unable to malloc reg->length\n");
            return NULL;
            }
        if(fidl_aviseg_gt0==0) {
            for(i=0;i<nregions;i++) {
                strcpy(string,region_names_ptr[i]); /*gets decimated*/
                if(!(strptr=strtok(string," "))) {
                    printf("Error: No token found - spot 1. Abort!\n");
                    return NULL;
                    }
                reg->harolds_num[i] = atoi(strptr) + 1;
                if(!(strptr = strtok((char*)NULL," "))) {
                    printf("Error: No token found - spot 2. Abort!\n");
                    return NULL;
                    }
                reg->length[i] = strlen(strptr) + 1;
                }
            }
        else if(fidl_aviseg_gt0==1) {
            for(i=0;i<reg->nregions;i++) reg->harolds_num[i] = i+1;
            reg->length[0]=4; /*CSF*/
            reg->length[1]=5; /*gray*/
            reg->length[2]=6; /*white*/
            }
        else {
            strcpy(string,region_file); /*gets decimated*/
            if(!(strptr=get_tail_sans_ext(string))) return NULL;
            reg->harolds_num[0] = 1;
            reg->length[0]=strlen(strptr)+1;
            }
        if(!(reg->region_names=d2charvar(reg->nregions,reg->length))) return NULL;
        if(fidl_aviseg_gt0==0) {
            for(i=0;i<reg->nregions;i++) {
                strcpy(string,region_names_ptr[i]); /*gets decimated*/
                if(!(strptr=strtok(string," "))) {
                    printf("Error: No token found - spot 1. Abort!\n");
                    return NULL;
                    }
                if(!(strptr = strtok((char*)NULL," "))) {
                    printf("Error: No token found - spot 2. Abort!\n");
                    return NULL;
                    }
                strcpy(reg->region_names[i],strptr);
                if(ifh_region_names) if((strptr=strtok((char*)NULL," ")))reg->nvoxels_region[i]=atoi(strptr);
                }
            }
        else if(fidl_aviseg_gt0==1) {
            strcpy(reg->region_names[0],"CSF");
            strcpy(reg->region_names[1],"gray");
            strcpy(reg->region_names[2],"white");
            }
        else if(fidl_aviseg_gt0==2) {
            strcpy(reg->region_names[0],strptr);
            }
        if(ifh) free_ifh(ifh,(int)FALSE);
        }
    else {
        reg->length = (size_t*)NULL;
        for(i=0;i<reg->nregions;i++) reg->harolds_num[i] = i+1;
        reg->region_names = (char**)NULL;
        }
    if(lc_freeimage) free(image);
    free(regval);
    return reg;
}
Regions *get_reg_guts(int nregions,int lenvol,float *image,int fidl_aviseg_gt0,int *reg_val)
{
    int i,j,*count;
    Regions *reg;
    if(!(reg=(Regions*)malloc(sizeof*reg))) {
        printf("Error: Unable to malloc reg\n");
        return NULL;
        }
    reg->voxel_indices = NULL;
    reg->nregions = nregions;
    if(fidl_aviseg_gt0==0) {
        for(i=0;i<reg->nregions;i++) reg_val[i] = i+2;
        }
    else if(fidl_aviseg_gt0==1) {
        reg_val[0] = 1000; /*CSF*/
        reg_val[1] = 2000; /*gray*/
        reg_val[2] = 3000; /*white*/
        }
    if(!(reg->nvoxels_region=(int*)malloc(sizeof*reg->nvoxels_region*reg->nregions))) {
        printf("Error: Unable to malloc reg->nvoxels_region\n");
        return NULL;
        }
    for(i=0;i<reg->nregions;i++) reg->nvoxels_region[i] = 0;
    if(image) {
        if(fidl_aviseg_gt0<2) {
            for(i=0;i<lenvol;i++)
                for(j=0;j<reg->nregions;j++)
                    if((int)image[i] == reg_val[j])
                        reg->nvoxels_region[j]++;
            }
        else {
            for(i=0;i<lenvol;i++)
                for(j=0;j<reg->nregions;j++)
                    if(fabs(image[i]) > (float)UNSAMPLED_VOXEL)
                        reg->nvoxels_region[j]++;
            }
        if(!(reg->voxel_indices=d2intvar(reg->nregions,reg->nvoxels_region))) return NULL;
        if(!(count=(int*)malloc(sizeof*count*reg->nregions))) {
            printf("Error: Unable to malloc count\n");
            return NULL; 
            }
        for(i=0;i<reg->nregions;i++) count[i] = 0;
        if(fidl_aviseg_gt0<2) {
            for(i=0;i<lenvol;i++)
                for(j=0;j<reg->nregions;j++)
                    if((int)image[i] == reg_val[j]) {
                        reg->voxel_indices[j][count[j]] = i;
                        count[j]++;
                        }
            }
        else {
            for(i=0;i<lenvol;i++)
                for(j=0;j<reg->nregions;j++)
                    if(fabs(image[i]) > (float)UNSAMPLED_VOXEL) {
                        reg->voxel_indices[j][count[j]] = i;
                        count[j]++;
                        }
            }
        free(count);
        }
    return reg;
}
void free_reg(Regions *reg)
{
    if(reg->region_names) {
        free(reg->region_names[0]);
        free(reg->region_names);
        }
    if(reg->length) free(reg->length);
    free(reg->harolds_num);
    free(reg->voxel_indices[0]);
    free(reg->voxel_indices);
    free(reg->nvoxels_region);
    free(reg);
}
