/* Copyright 9/17/02 Washington University.  All Rights Reserved.
   get_atlas_param.c  $Revision: 1.32 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fidl.h"
#include "get_atlas_param.h"

//START170616
#include "nifti_gethdr.h"

int _get_atlas_param(int argc,char **argv){

    Atlas_Param *ap;
    char *atlas=argv[0];
    float *param=(float*)argv[1];

    //if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL))) return 0; 
    //START170616
    if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL,(char*)NULL))) return 0; 

    param[0] = (float)ap->mmppix[0];
    param[1] = (float)ap->mmppix[1];
    param[2] = (float)ap->mmppix[2];
    param[3] = (float)ap->center[0];
    param[4] = (float)ap->center[1];
    param[5] = (float)ap->center[2];
    param[6] = (float)ap->xdim;
    param[7] = (float)ap->ydim;
    param[8] = (float)ap->zdim;
    param[9] = (float)ap->area;
    param[10] = (float)ap->vol;
    param[11] = (float)ap->voxel_size[0];
    param[12] = (float)ap->voxel_size[1];
    param[13] = (float)ap->voxel_size[2];

    #if 0
    printf("atlas=%s\n",atlas);
    printf("param[0]=%f\n",param[0]);
    printf("param[1]=%f\n",param[1]);
    printf("param[2]=%f\n",param[2]);
    printf("param[3]=%f\n",param[3]);
    printf("param[4]=%f\n",param[4]);
    printf("param[5]=%f\n",param[5]);
    printf("param[6]=%f\n",param[6]);
    printf("param[7]=%f\n",param[7]);
    printf("param[8]=%f\n",param[8]);
    printf("param[9]=%f\n",param[9]);
    printf("param[10]=%f\n",param[10]);
    printf("param[11]=%f\n",param[11]);
    printf("param[12]=%f\n",param[12]);
    printf("param[13]=%f\n",param[13]);
    #endif

    free_atlas_param(ap);
    return 1;
    }

//Atlas_Param *get_atlas_param(char* atlas,Interfile_header *ifh){
//START170616
Atlas_Param *get_atlas_param(char* atlas,Interfile_header *ifh,char *filename){
    int64_t xdim64,ydim64,zdim64,tdim64;

    Atlas_Param *ap;
    if(!(ap=malloc(sizeof*ap))){
        printf("Error: Unable to malloc ap\n");
        return NULL;
        }
    if(ifh){
        ap->mmppix[0]=(double)ifh->mmppix[0];ap->mmppix[1]=(double)ifh->mmppix[1];ap->mmppix[2]=(double)ifh->mmppix[2];
        ap->center[0]=(double)ifh->center[0];ap->center[1]=(double)ifh->center[1];ap->center[2]=(double)ifh->center[2];
        ap->mmppixf[0]=ifh->mmppix[0];ap->mmppixf[1]=ifh->mmppix[1];ap->mmppixf[2]=ifh->mmppix[2];
        ap->centerf[0]=ifh->center[0];ap->centerf[1]=ifh->center[1];ap->centerf[2]=ifh->center[2];
        if(!strcmp("222MNI",atlas)){
            ap->xdim=(int)XDIM_222MNI;ap->ydim=(int)YDIM_222MNI;ap->zdim=(int)ZDIM_222MNI;
            ap->area=(int)AREA_222MNI;
            ap->vol=(int)VOL_222MNI;
            ap->voxel_size[0]=ap->voxel_size[1]=ap->voxel_size[2]=2;
            strcpy(ap->str,"_MNI152");
            }
        else{
            ap->xdim=ifh->dim1;ap->ydim=ifh->dim2;ap->zdim=ifh->dim3;
            ap->area=ifh->dim1*ifh->dim2;
            ap->vol=ifh->dim1*ifh->dim2*ifh->dim3;
            ap->voxel_size[0]=ifh->voxel_size_1;ap->voxel_size[1]=ifh->voxel_size_2;ap->voxel_size[2]=ifh->voxel_size_3;
            ap->str[0]=0;
            }

        }
    else if(!atlas[0]) {
        ap->mmppix[0]=ap->mmppix[1]=ap->mmppix[2]=ap->center[0]=ap->center[1]=ap->center[2]=0.;
        ap->mmppixf[0]=ap->mmppixf[1]=ap->mmppixf[2]=ap->centerf[0]=ap->centerf[1]=ap->centerf[2]=0.;
        ap->str[0]=0;
        }
    else if(!strcmp("111",atlas)) {
        ap->mmppix[0]=(double)MMPPIX_X_111;ap->mmppix[1]=(double)MMPPIX_Y_111;ap->mmppix[2]=(double)MMPPIX_Z_111;
        ap->center[0]=(double)CENTER_X_111;ap->center[1]=(double)CENTER_Y_111;ap->center[2]=(double)CENTER_Z_111;
        ap->mmppixf[0]=(float)MMPPIX_X_111;ap->mmppixf[1]=(float)MMPPIX_Y_111;ap->mmppixf[2]=(float)MMPPIX_Z_111;
        ap->centerf[0]=(float)CENTER_X_111;ap->centerf[1]=(float)CENTER_Y_111;ap->centerf[2]=(float)CENTER_Z_111;
        ap->xdim = (int)XDIM_111;
        ap->ydim = (int)YDIM_111;
        ap->zdim = (int)ZDIM_111;
        ap->area = AREA_111;
        ap->vol = VOL_111;
        ap->voxel_size[0] = 1;
        ap->voxel_size[1] = 1;
        ap->voxel_size[2] = 1;
        strcpy(ap->str,"_111_t88");
        }
    else if(!strcmp("222",atlas)) {
        ap->mmppix[0]=(double)MMPPIX_X_222;ap->mmppix[1]=(double)MMPPIX_Y_222;ap->mmppix[2]=(double)MMPPIX_Z_222;
        ap->center[0]=(double)CENTER_X_222;ap->center[1]=(double)CENTER_Y_222;ap->center[2]=(double)CENTER_Z_222;
        ap->mmppixf[0]=(float)MMPPIX_X_222;ap->mmppixf[1]=(float)MMPPIX_Y_222;ap->mmppixf[2]=(float)MMPPIX_Z_222;
        ap->centerf[0]=(float)CENTER_X_222;ap->centerf[1]=(float)CENTER_Y_222;ap->centerf[2]=(float)CENTER_Z_222;
        ap->xdim = (int)XDIM_222;
        ap->ydim = (int)YDIM_222;
        ap->zdim = (int)ZDIM_222;
        ap->area = AREA_222;
        ap->vol = VOL_222;
        ap->voxel_size[0] = 2;
        ap->voxel_size[1] = 2;
        ap->voxel_size[2] = 2;
        strcpy(ap->str,"_222_t88");
        }
    else if(!strcmp("333",atlas)) {
        ap->mmppix[0]=(double)MMPPIX_X_333;ap->mmppix[1]=(double)MMPPIX_Y_333;ap->mmppix[2]=(double)MMPPIX_Z_333;
        ap->center[0]=(double)CENTER_X_333;ap->center[1]=(double)CENTER_Y_333;ap->center[2]=(double)CENTER_Z_333;
        ap->mmppixf[0]=(float)MMPPIX_X_333;ap->mmppixf[1]=(float)MMPPIX_Y_333;ap->mmppixf[2]=(float)MMPPIX_Z_333;
        ap->centerf[0]=(float)CENTER_X_333;ap->centerf[1]=(float)CENTER_Y_333;ap->centerf[2]=(float)CENTER_Z_333;
        ap->xdim = (int)XDIM_333;
        ap->ydim = (int)YDIM_333;
        ap->zdim = (int)ZDIM_333;
        ap->area = AREA_333;
        ap->vol = VOL_333;
        ap->voxel_size[0] = 3;
        ap->voxel_size[1] = 3;
        ap->voxel_size[2] = 3;
        strcpy(ap->str,"_333_t88");
        }

    //START170616
    else if(!strcmp("111MNI",atlas)){
        if(!filename){printf("fidlError: get_atlas_param filename is null\n");return NULL;}
        if(!(nifti_gethdr(filename,&xdim64,&ydim64,&zdim64,&tdim64,ap->centerf,ap->mmppixf,ap->c_orient)))return NULL;
        ap->xdim=(int)xdim64;ap->ydim=(int)ydim64;ap->zdim=(int)zdim64;
        ap->mmppix[0]=(double)ap->mmppixf[0];ap->mmppix[1]=(double)ap->mmppixf[1];ap->mmppix[2]=ap->mmppixf[2];
        ap->center[0]=(double)ap->centerf[0];ap->center[1]=(double)ap->centerf[1];ap->center[2]=ap->centerf[2];
        ap->area=(int)(xdim64*ydim64);
        ap->vol=(int)(xdim64*ydim64*zdim64);
        ap->voxel_size[0]=ap->voxel_size[1]=ap->voxel_size[2]=1;
        strcpy(ap->str,"_MNI152");
        }

    else {
        printf("fidlError: Unknown atlas. atlas=%sEND Abort!\n",atlas);
        free_atlas_param(ap);
        ap = (Atlas_Param *)NULL;
        }
    return ap;
    }
void free_atlas_param(Atlas_Param *ap){
    free(ap);
    }
