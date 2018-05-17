/* Copyright 8/28/14 Washington University.  All Rights Reserved.
   center_of_mass2.c  $Revision: 1.10 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef __sun__
    #include <stdint.h>
#endif
#include "fidl.h"
#include "center_of_mass2.h"
#include "get_atlas_coor.h"

//START180116
#include "get_atlas.h"

center_of_mass2struct *center_of_mass2init(int nvox,int nreg){
    center_of_mass2struct *com;
    if(!(com=malloc(sizeof*com))) {
        printf("fidlError: Unable to malloc com in center_of_mass2init\n");
        return NULL;
        }
    if(!(com->x=malloc(sizeof*com->x*nvox))) {
        printf("fidlError: Unable to malloc com->x in center_of_mass2init\n");
        return NULL;
        }
    if(!(com->y=malloc(sizeof*com->y*nvox))) {
        printf("fidlError: Unable to malloc com->y in center_of_mass2init\n");
        return NULL;
        }
    if(!(com->z=malloc(sizeof*com->z*nvox))) {
        printf("fidlError: Unable to malloc com->z in center_of_mass2init\n");
        return NULL;
        }
    if(!(com->px=malloc(sizeof*com->px*nreg))) {
        printf("fidlError: Unable to malloc com->px in center_of_mass2init\n");
        return NULL;
        }
    if(!(com->py=malloc(sizeof*com->py*nreg))) {
        printf("fidlError: Unable to malloc com->py in center_of_mass2init\n");
        return NULL;
        }
    if(!(com->pz=malloc(sizeof*com->pz*nreg))) {
        printf("fidlError: Unable to malloc com->pz in center_of_mass2init\n");
        return NULL;
        }
    if(!(com->coor=malloc(sizeof*com->coor*nreg*3))) {
        printf("fidlError: Unable to malloc coor in center_of_mass2init\n");
        return NULL;
        }
    if(!(com->peakcoor=malloc(sizeof*com->peakcoor*nreg*3))) {
        printf("fidlError: Unable to malloc peakcoor in center_of_mass2init\n");
        return NULL;
        }
    if(!(com->peakval=malloc(sizeof*com->peakval*nreg))) {
        printf("fidlError: Unable to malloc peakval in center_of_mass2init\n");
        return NULL;
        }

    //START180223
    if(!(com->peaki=malloc(sizeof*com->peaki*nreg))) {
        printf("fidlError: Unable to malloc peaki in center_of_mass2init\n");
        return NULL;
        }


    return com;
    }
void center_of_mass2free(center_of_mass2struct *com){

    //START180223
    free(com->peaki);

    free(com->peakval);
    free(com->peakcoor);
    free(com->coor);
    free(com->pz);
    free(com->py);
    free(com->px);
    free(com->z);
    free(com->y);
    free(com->x);
    free(com);
    }
void center_of_mass2(float *act,center_of_mass2struct *com,int nvox,int *indices,int nreg,int *nvoxreg,Atlas_Param *ap){
    double denominator,num_x,num_y,num_z,max,td;
    int i,j,k;
    col_row_slice(nvox,indices,com->x,com->y,com->z,ap);

    //for(i=0;i<nvox;i++)printf("com->x[%d]=%f com->y[%d]=%f com->z[%d]=%f\n",i,com->x[i],i,com->y[i],i,com->z[i]); 

    for(k=i=0;i<nreg;i++) {
        for(max=denominator=num_x=num_y=num_z=0.,j=0;j<nvoxreg[i];j++,k++) {
            denominator += (double)act[indices[k]];
            num_x += (double)(com->x[k]*act[indices[k]]);
            num_y += (double)(com->y[k]*act[indices[k]]);
            num_z += (double)(com->z[k]*act[indices[k]]);
            if((td=fabs(act[indices[k]])) > max) {
                max = td;
                com->peakval[i] = act[indices[k]];
                com->px[i] = (double)com->x[k];
                com->py[i] = (double)com->y[k];
                com->pz[i] = (double)com->z[k];

                //START180223
                com->peaki[i] = indices[k];

                }
            }
        com->x[i] = num_x/denominator;
        com->y[i] = num_y/denominator;
        com->z[i] = num_z/denominator;
        }
    get_atlas_coor(nreg,com->x,com->y,com->z,(double)ap->zdim,ap->center,ap->mmppix,com->coor);
    get_atlas_coor(nreg,com->px,com->py,com->pz,(double)ap->zdim,ap->center,ap->mmppix,com->peakcoor);
    }
int _center_of_mass2(int argc,char **argv){
    float *zc=(float*)argv[0],*coor=(float*)argv[6],*peakcoor=(float*)argv[7],*peakval=(float*)argv[8];
    #ifdef __sun__
        int vol = (int)argv[1];
        int nvox = (int)argv[2];
        int nreg = (int)argv[3];
    #else
        int vol = (intptr_t)argv[1];
        int nvox = (intptr_t)argv[2];
        int nreg = (intptr_t)argv[3];
    #endif
    float *reglistf=(float*)argv[4],*regsizef=(float*)argv[5];
    size_t i;
    int *reglist,*regsize;
    if(!(reglist=malloc(sizeof*reglist*nvox))) {
        printf("fidlError: Unable to malloc reglist in _center_of_mass2\n");
        return 0;
        }
    for(i=0;i<(size_t)nvox;i++) reglist[i]=(int)reglistf[i];
    if(!(regsize=malloc(sizeof*regsize*nreg))) {
        printf("fidlError: Unable to malloc regsize in _center_of_mass2\n");
        return 0;
        }
    for(i=0;i<(size_t)nreg;i++) regsize[i]=(int)regsizef[i];
    char atlas[7];
    Atlas_Param *ap;
    center_of_mass2struct *com;
    get_atlas(vol,atlas);

    //if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL))) return 0;
    //START170706
    if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL,(char*)NULL))) return 0;

    if(!(com=center_of_mass2init(nvox,nreg))) return 0;
    center_of_mass2(zc,com,nvox,reglist,nreg,regsize,ap);
    for(i=0;i<(size_t)(nreg*3);i++) {coor[i]=(float)com->coor[i];peakcoor[i]=(float)com->peakcoor[i];}
    for(i=0;i<(size_t)nreg;i++) peakval[i]=(float)com->peakval[i];
    center_of_mass2free(com);
    free_atlas_param(ap);
    free(regsize);
    free(reglist);
    return 1;
    }
