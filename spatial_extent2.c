/* Copyright 7/21/14 Washington University.  All Rights Reserved.
   spatial_extent2.c  $Revision: 1.11 $*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef __sun__
    #include <stdint.h>
#endif
#include "fidl.h"
#include "spatial_extent2.h"
Spatial_Extent2 *spatial_extent2init(int xdim,int ydim,int zdim,int nbrnidx,int *brnidx){
    Spatial_Extent2 *se;
    if(!(se=malloc(sizeof*se))) {
        printf("fidlError: Unable to malloc se in spatial_extent2init\n");
        return NULL;
        }
    se->xdim=xdim;se->ydim=ydim;se->zdim=zdim;
    se->n_nghbrs_middle = (int)N_NGHBRS_FACE;
    se->n_nghbrs_end = (int)N_NGHBRS_END_FACE;
    se->plndim = xdim*ydim;
    se->vol = xdim*ydim*zdim;
    se->nbrnidx = nbrnidx;
    se->brnidx = brnidx;
    if(!(se->offsets=malloc(sizeof*se->offsets*6))) {
        printf("fidlError: Unable to malloc se->offsets in spatial_extent2init\n");
        return NULL;
        }
    if(!(se->offsets_pln0=malloc(sizeof*se->offsets_pln0*5))) {
        printf("fidlError: Unable to malloc se->offsets_pln0 in spatial_extent2init\n");
        return NULL;
        }
    if(!(se->offsets_plnN=malloc(sizeof*se->offsets_plnN*5))) {
        printf("fidlError: Unable to malloc se->offsets_plnN in spatial_extent2init\n");
        return NULL;
        }
    if(!(se->voxlist=malloc(sizeof*se->voxlist*se->vol))) {
        printf("fidlError: Unable to malloc se->voxlist in spatial_extent2init\n");
        return NULL;
        }
    if(!(se->reglist=malloc(sizeof*se->reglist*se->vol))) {
        printf("fidlError: Unable to malloc se->reglist in spatial_extent2init\n");
        return NULL;
        }
    if(!(se->regsize=malloc(sizeof*se->regsize*nbrnidx))) {
        printf("fidlError: Unable to malloc se->regsize in spatial_extent2init\n");
        return NULL;
        }
    se->offsets[0]=1;se->offsets[1]=-1;se->offsets[2]=-xdim;se->offsets[3]=xdim;se->offsets[4]=se->plndim;se->offsets[5]=-se->plndim;
    se->offsets_pln0[0]=1;se->offsets_pln0[1]=-1;se->offsets_pln0[2]=-xdim;se->offsets_pln0[3]=xdim;se->offsets_pln0[4]=se->plndim;
    se->offsets_plnN[0]=1;se->offsets_plnN[1]=-1;se->offsets_plnN[2]=-xdim;se->offsets_plnN[3]=xdim;se->offsets_plnN[4]=-se->plndim;
    return se;
    }
void spatial_extent2free(Spatial_Extent2 *se){
    free(se->regsize);
    free(se->reglist);
    free(se->voxlist);
    free(se->offsets_plnN);
    free(se->offsets_pln0);
    free(se->offsets);
    free(se);
    }
int spatial_extent2(double *crushed,float *actmask,double thresh,int n1,int n2,int p_z,Spatial_Extent2 *se){                   
    size_t i,j,k;
    int *off=NULL,n_reg,n_srch,vox,vox1,n_nghbrs;
    if(actmask) for(i=0;i<(size_t)se->vol;i++) actmask[i]=0.;

    //START180329
    if(thresh<=(double)UNSAMPLED_VOXEL)thresh=2.*(double)UNSAMPLED_VOXEL;

    for(se->nreg=se->nvox=j=0;j<(size_t)se->nbrnidx;j++) {
        vox = se->brnidx[j];
        if(vox < se->xdim+1 || vox > se->vol-se->xdim-2) {
            /* Skip first and last row in volume. */
            n_nghbrs = 0;
            }
        else if(vox < se->plndim+se->xdim+1) {
            n_nghbrs = se->n_nghbrs_end;
            off = se->offsets_pln0;
            }
        else if(vox > se->vol-(se->plndim+se->xdim)-2) {
           n_nghbrs = se->n_nghbrs_end;
           off = se->offsets_plnN;
           }
        else {
           n_nghbrs = se->n_nghbrs_middle;
           off = se->offsets;
           }
        n_srch = n_reg = 0;
        if(!p_z) {
            if(crushed[vox] < thresh) {
                crushed[vox] = thresh;
                se->reglist[se->nvox+n_reg++] = vox; 
                for(i=0;i<(size_t)n_nghbrs;i++) {
                    if(crushed[vox+off[i]] < thresh) {
                        crushed[vox+off[i]] = thresh;
                        se->voxlist[n_srch++] = vox+off[i];
                        se->reglist[se->nvox+n_reg++] = vox+off[i];
                        }
                    }
                while(n_srch > 0) {
                    vox1 = se->voxlist[--n_srch];
                    if(vox1 < se->xdim+1 || vox1 > se->vol-se->xdim-2) {
                        /* Skip first and last row in volume. */
                        n_nghbrs = 0;
                        }
                    else if(vox1 < se->plndim+se->xdim+1) {
                        n_nghbrs = se->n_nghbrs_end;
                        off = se->offsets_pln0;
                        }
                    else if(vox1 > se->vol - (se->plndim+se->xdim)-2) {
                        n_nghbrs = se->n_nghbrs_end;
                        off = se->offsets_plnN;
                        }
                    else {
                        n_nghbrs = se->n_nghbrs_middle;
                        off = se->offsets;
                        }
                    for(i=0;i<(size_t)n_nghbrs;i++) {
                        if(crushed[vox1+off[i]] < thresh) {
                            crushed[vox1+off[i]] = thresh;
                            se->voxlist[n_srch++] = vox1+off[i];
                            se->reglist[se->nvox+n_reg++] = vox1+off[i];
                            }
                        }
                    }
                }
            }
        else {

            #if 0
            if(fabs(crushed[vox]) > thresh) {
                crushed[vox] = thresh;
            #endif
            //START180320
            #if 1
            if(fabs(crushed[vox]) >= thresh) {
                //crushed[vox] = crushed[vox]>0.?thresh-(double)UNSAMPLED_VOXEL:thresh+(double)UNSAMPLED_VOXEL;
                //crushed[vox] = thresh-(double)UNSAMPLED_VOXEL;
                crushed[vox] = (double)UNSAMPLED_VOXEL;
                //crushed[vox] = (double)DBL_MIN;
            #endif

	        se->reglist[se->nvox+n_reg++] = vox; 
                for(i=0;i<(size_t)n_nghbrs;i++) {

                    #if 0
                    if(fabs(crushed[vox+off[i]]) > thresh) {
                        crushed[vox+off[i]] = thresh;
                    #endif
                    //START180320
                    #if 1
                    if(fabs(crushed[vox+off[i]]) >= thresh) {
                        //crushed[vox+off[i]] = crushed[vox+off[i]]>0.?thresh-(double)UNSAMPLED_VOXEL:thresh+(double)UNSAMPLED_VOXEL; 
                        //crushed[vox+off[i]] = thresh-(double)UNSAMPLED_VOXEL; 
                        crushed[vox+off[i]] = (double)UNSAMPLED_VOXEL; 
                        //crushed[vox+off[i]] = (double)DBL_MIN; 
                    #endif

                        se->voxlist[n_srch++] = vox+off[i];
                        se->reglist[se->nvox+n_reg++] = vox+off[i];
                        }
                    }
                while(n_srch > 0) {
                    vox1 = se->voxlist[--n_srch];
                    if(vox1 < se->xdim+1 || vox1 > se->vol-se->xdim-2) {
                        /* Skip first and last row in volume. */
                        n_nghbrs = 0;
                        }
                    else if(vox1 < se->plndim+se->xdim+1) {
                        n_nghbrs = se->n_nghbrs_end;
                        off = se->offsets_pln0;
                        }
                    else if(vox1 > se->vol - (se->plndim+se->xdim)-2) {
                        n_nghbrs = se->n_nghbrs_end;
                        off = se->offsets_plnN;
                        }
                    else {
                        n_nghbrs = se->n_nghbrs_middle;
                        off = se->offsets;
                        }
                    for(i=0;i<(size_t)n_nghbrs;i++) {

                        #if 0
                        if(fabs(crushed[vox1+off[i]]) > thresh) {
                            crushed[vox1+off[i]] = thresh;
                        #endif
                        //START180320
                        #if 1
                        if(fabs(crushed[vox1+off[i]]) >= thresh) {
                            //crushed[vox1+off[i]]=crushed[vox1+off[i]]>0.?thresh-(double)UNSAMPLED_VOXEL:thresh+(double)UNSAMPLED_VOXEL; 
                            //crushed[vox1+off[i]] = thresh-(double)UNSAMPLED_VOXEL; 
                            crushed[vox1+off[i]] = (double)UNSAMPLED_VOXEL; 
                            //crushed[vox1+off[i]] = (double)DBL_MIN; 
                        #endif

                            se->voxlist[n_srch++] = vox1+off[i];
                            se->reglist[se->nvox+n_reg++] = vox1+off[i];
                            }
                        }
                    }
                }
            }
        if(n_reg >= n1 && n_reg <= n2) {
            if(actmask) for(k=se->nvox,i=0;i<(size_t)n_reg;i++,k++) actmask[se->reglist[k]] = (float)(se->nreg+2);
            se->nvox += n_reg;
            se->regsize[se->nreg++] = n_reg;
            }
        }
    return se->nreg;
    }
int _spatial_extent2(int argc,char **argv){
    float *z=(float*)argv[0];
    float *actmask=(float*)argv[1];
    float thresh=*(float*)argv[2];
    char *acttype=(char*)argv[3];
    #ifdef __sun__
        int dim1 = (int)argv[4];
        int dim2 = (int)argv[5];
        int dim3 = (int)argv[6];
        int extent = (int)argv[7];
        int *nreg=(int*)argv[8];
        int *nvox=(int*)argv[9];
    #else
        int dim1 = (intptr_t)argv[4];
        int dim2 = (intptr_t)argv[5];
        int dim3 = (intptr_t)argv[6];
        int extent = (intptr_t)argv[7];
        intptr_t *nreg=(intptr_t*)argv[8];
        intptr_t *nvox=(intptr_t*)argv[9];
    #endif
    float *reglist=(float*)argv[10];
    float *regsize=(float*)argv[11];
    size_t i;
    int vol,*brnidx;
    double *temp_double;
    Spatial_Extent2 *se;
    vol=dim1*dim2*dim3;
    if(!(brnidx=malloc(sizeof*brnidx*vol))) {
        printf("fidlError: Unable to malloc brnidx\n");
        return 0; 
        }
    for(i=0;i<(size_t)vol;i++) brnidx[i]=i;
    if(!(se=spatial_extent2init(dim1,dim2,dim3,vol,brnidx))) return 0;
    if(!(temp_double=malloc(sizeof*temp_double*vol))) {
        printf("fidlError: Unable to malloc temp_double\n");
        return 0; 
        }
    if(strstr(acttype,"all")) for(i=0;i<(size_t)vol;i++) temp_double[i]=(double)z[i];
    else if(strstr(acttype,"pos")) for(i=0;i<(size_t)vol;i++) temp_double[i]=z[i]<0.? 0.:(double)z[i];
    else if(strstr(acttype,"neg")) for(i=0;i<(size_t)vol;i++) temp_double[i]=z[i]>0.? 0.:(double)z[i];
    else {printf("fidlError: unknown acttype\n");return 0;}
    spatial_extent2(temp_double,actmask,(double)thresh,extent,vol,1,se);
    *nreg = se->nreg;
    *nvox = se->nvox;
    for(i=0;i<(size_t)se->nvox;i++) reglist[i] = (float)se->reglist[i];
    for(i=0;i<(size_t)se->nreg;i++) regsize[i] = (float)se->regsize[i];
    for(i=0;i<(size_t)vol;i++) z[i]=!actmask[i]&&z[i]!=(float)UNSAMPLED_VOXEL?0.:z[i];
    free(temp_double);
    spatial_extent2free(se); 
    free(brnidx);
    return 1; 
    }
