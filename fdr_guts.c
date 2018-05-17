/* Copyright 7/26/16 Washington University.  All Rights Reserved.
   fdr_guts.c  $Revision: 1.2 $ */
#include <stdlib.h>
#include <gsl/gsl_sort_int.h>
#include "spatial_extent2.h"
#include "fdr_guts.h"
//int fdr_guts(double *crushed,float *corrected,float *actmask,double thresh,Spatial_Extent2 *se,double **table,double q,int *regstart,
//START160816
int fdr_guts(double *crushed,float *corrected,float *actmask,double thresh,Spatial_Extent2 *se,double **table,double q,float *regstart,
    int *clussize,float *min,float *max,size_t *p){
                                        //allocate ms->lenbrain
    int i,j,k,l,idx0,idx1;
    if(!spatial_extent2(crushed,actmask,thresh,1,se->nbrnidx,1,se)){
        for(j=0;j<se->nbrnidx;j++)corrected[se->brnidx[j]]=0.; 
        return 0; 
        }
    gsl_sort_int_index(p,se->regsize,(size_t)1,(size_t)se->nreg);
    for(idx0=0;table[idx0][3]!=thresh;idx0++);
    //printf("here0 idx0=%d\n",idx0);
    for(idx1=idx0;table[idx1][3]==thresh;idx1++);
    //printf("here1 idx1=%d table[%d][4]=%g\n",idx1,idx1,table[idx1][4]);


    //START180221 This can be improved by instead of se->nreg, we want se->nreg_uniq
    //find_regions_by_file_cover.c has an uniq sort
    //This is not correct. It is a step-down (to a lower procedure). It needs to be a step up. See fidl_logreg_fdr.cxx

    for(j=se->nreg,i=0;i<se->nreg;j--){
        for(;se->regsize[p[i]]>table[idx0][4]&&idx0<idx1;idx0++);
        //printf("here2 se->regsize[%zd]=%d table[%d][4]=%g idx0=%d\n",p[i],se->regsize[p[i]],idx0,table[idx0][4],idx0);
        //printf("table[%d][5]=%g j=%d se->nreg=%d q=%g (double)j/(double)se->nreg*q=%g\n",idx0,table[idx0][5],j,se->nreg,q,
        //    (double)j/(double)se->nreg*q);
        if(table[idx0][5]<=(double)j/(double)se->nreg*q)break;
        for(k=i;se->regsize[p[++i]]==se->regsize[p[k]];);//skips repeated region sizes
        }
    *clussize=(int)table[idx0][4];
    //printf("table[%d][4]=%f *clussize=%d i=%d\n",idx0,table[idx0][4],*clussize,i);
    for(l=j=0;j<se->nreg;j++){
        if(se->regsize[j]<*clussize){
            for(k=0;k<se->regsize[j];k++,l++){
                actmask[se->reglist[l]]=0.;
                }
            }
        else{
            for(k=0;k<se->regsize[j];k++,l++){
                actmask[se->reglist[l]]=*regstart;
                if(corrected[se->reglist[l]]>*max)*max=corrected[se->reglist[l]];
                if(corrected[se->reglist[l]]<*min)*min=corrected[se->reglist[l]];
                }
            (*regstart)++;
            }
        }
    for(j=0;j<se->nbrnidx;j++)if(!actmask[se->brnidx[j]])corrected[se->brnidx[j]]=0.;
    return se->nreg-i;
    }
