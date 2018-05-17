/* Copyright 2/6/18 Washington University.  All Rights Reserved.
   logreg.c  $Revision: 1.1 $ */
#include <stdlib.h> 
#include <stdio.h> 
#include "logreg.h"
#include "map_disk.h"
#include "constants.h"

LRy *create_LRy(int num_regions,int ncov){
    int i;
    LRy *lry;
    if(!(lry=(LRy*)malloc(sizeof*lry))) {
        printf("fidlError: Unable to malloc lry\n");
        return NULL;
        }
    if(!(lry->n0=(int*)malloc(sizeof*lry->n0*num_regions))) {
        printf("fidlError: Unable to malloc lry->n0\n");
        return NULL;
        }
    if(!(lry->n1=(int*)malloc(sizeof*lry->n1*num_regions))) {
        printf("fidlError: Unable to malloc lry->n1\n");
        return NULL;
        }
    if(!(lry->starti=(int*)malloc(sizeof*lry->starti*num_regions))) {
        printf("fidlError: Unable to malloc lry->index\n");
        return NULL;
        }
    if(!(lry->nindex=(int*)malloc(sizeof*lry->nindex*num_regions))) {
        printf("fidlError: Unable to malloc lry->nindex\n");
        return NULL;
        }
    if(!(lry->index=(int*)malloc(sizeof*lry->index*num_regions*ncov))) {
        printf("fidlError: Unable to malloc lry->index\n");
        return NULL;
        }
    if(!(lry->y=(double*)malloc(sizeof*lry->y*num_regions*ncov))) {
        printf("fidlError: Unable to malloc lry->y\n");
        return NULL;
        }
    for(i=0;i<num_regions;i++) lry->n0[i]=lry->n1[i]=lry->nindex[i]=0;
    return lry;
    }
int assign_LRy(Data *data,int *num_regions_by_file,int num_regions,LRy *lry){
    int j,jj,m;
    double temp;
    Memory_Map *mm;
    for(jj=j=0;j<num_regions;j++,jj+=data->nsubjects)lry->starti[j]=jj;
    for(m=0;m<data->nsubjects;m++){
        if(!(mm=map_disk(data->subjects[m],num_regions*num_regions_by_file[m],0,sizeof(float))))return 0;
        for(j=0;j<num_regions;j++){
            if((temp=mm->ptr[j])!=(float)UNSAMPLED_VOXEL){
                jj=lry->starti[j]+lry->nindex[j];
                if(temp>0.5) {
                    lry->y[jj]=1.;
                    lry->n1[j]++;
                    }
                else {
                    lry->y[jj]=-1;
                    lry->n0[j]++;
                    }
                lry->index[jj]=m;
                lry->nindex[j]++;
                }
            }
        if(!unmap_disk(mm))return 0;
        }
    return 1;
    }
