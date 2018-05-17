/* Copyright 3/30/18 Washington University.  All Rights Reserved.
   fidl_logreg_collinear.cxx  $Revision: 1.1 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_permutation.h>

#include "checkOS.h"
#include "read_data.h"
#include "d2double.h"
#include "cond_norm1.h"
#include "gsl_svd.h"
#include "files_struct.h"

int main(int argc,char **argv)
{
char *driverf=NULL;
int i,j,k,l,SunOS_Linux,Mcol,ndep,lcconstant=0,nexclude=0,nidx,*idx,*msk,m,n;
size_t i1;
double *A,**ATA,**ATAm1,*temp_double,*V,*S,*work,cond,cond_norm2,condmax=10000.,td,*ATy,yTy,stat,R2,**AATAm1,*yhat,SSE, 
    **ATAm1AT,traceR,traceRR,devmsq,var;
Data *data=NULL;
Files_Struct *exclude=NULL;

if(argc<3){
    printf("    -driver:   Specifies scratch files and behavioral measures (independent variables).\n");
    printf("    -constant  Include a constant term in the model.\n");

    //START180404
    printf("    -exclude:  Names of variables to be excluded from the model.\n");

    exit(-1);
    }
for(i=1;i<argc;i++){
    if(!strcmp(argv[i],"-driver") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        driverf = argv[++i];
    if(!strcmp(argv[i],"-constant"))
        lcconstant = 1;

    //START180404
    if(!strcmp(argv[i],"-exclude") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++nexclude;
        if(!(exclude=get_files(nexclude,&argv[i+1]))) exit(-1);
        i += nexclude;
        }
    }

if((SunOS_Linux=checkOS())==-1)exit(-1);
if(!(data=read_data(driverf,1,0,0,0)))exit(-1);
printf("data->nsubjects=%d data->npoints_per_line[0]=%d data->total_npoints_per_line=%d data->ncol=%d data->npoints=%d\n",
    data->nsubjects,data->npoints_per_line[0],data->total_npoints_per_line,data->ncol,data->npoints);
printf("data->colptr=");for(i=0;i<data->ncol;i++)printf("%s ",data->colptr[i]);printf("\n");

//START180404
if(nexclude){printf("exclude= ");for(i1=0;i1<exclude->nfiles;i1++)printf("%s ",exclude->files[i1]);printf("\n");fflush(stdout);}


Mcol=!lcconstant?data->npoints-1:data->npoints;


//START180404
if(nexclude)Mcol-=exclude->nfiles;
nidx=nexclude?data->npoints-exclude->nfiles:data->npoints;
if(!(idx=(int*)malloc(sizeof*idx*nidx))){
    printf("fidlError: Unable to idx\n");
    exit(-1);
    }
if(!nexclude){
    for(i=0;i<nidx;i++)idx[i]=i;
    }
else{
    if(!(msk=(int*)malloc(sizeof*msk*data->npoints))){
        printf("fidlError: Unable to msk\n");
        exit(-1);
        }
    for(i=0;i<data->npoints;i++)msk[i]=0;

    //for(nidx=j=i=0;i<data->npoints;i++)if(!strcmp(data->colptr[i+1],exclude->files[j])) j++; else idx[nidx++]=i;
    for(i1=0;i1<exclude->nfiles;i1++){
        for(j=0;j<data->npoints;j++)if(!strcmp(data->colptr[j+1],exclude->files[i1])){msk[j]=1;break;}
        if(j==data->npoints){printf("fidlError: %s not found\n",exclude->files[i1]);exit(-1);}
        }
    for(nidx=i=0;i<data->npoints;i++)if(!msk[i])idx[nidx++]=i; 
    if((data->npoints-nidx)!=(int)exclude->nfiles)
        {printf("fidlError: data->npoints-nidx=%d exclude->nfiles=%zd Must be equal.\n",data->npoints-nidx,exclude->nfiles);exit(-1);}
    }
printf("Mcol= %d lcconstant= %d nidx= %d idx= ",Mcol,lcconstant,nidx);for(i=0;i<nidx;i++)printf("%d ",idx[i]);printf("\n");



if(!(A=(double*)malloc(sizeof*A*data->nsubjects*Mcol))){
    printf("fidlError: Unable to A\n");
    exit(-1);
    }
gsl_matrix_view gA = gsl_matrix_view_array(A,data->nsubjects,Mcol);
if(!(ATA=d2double(Mcol,Mcol))) exit(-1);
gsl_matrix_view gATA = gsl_matrix_view_array(ATA[0],Mcol,Mcol);
if(!(temp_double=(double*)malloc(sizeof*temp_double*data->nsubjects*Mcol))){
    printf("fidlError: Unable to temp_double\n");
    exit(-1);
    }
if(!(ATAm1=d2double(Mcol,Mcol)))exit(-1);;
gsl_matrix_view gATAm1 = gsl_matrix_view_array(ATAm1[0],Mcol,Mcol);
if(!(V=(double*)malloc(sizeof*V*Mcol*Mcol))) {
    printf("fidlError: Unable to malloc V\n");
    exit(-1);
    }
if(!(S=(double*)malloc(sizeof*S*Mcol))) {
    printf("fidlError: Unable to malloc S\n");
    exit(-1);
    }
if(!(work=(double*)malloc(sizeof*work*Mcol))) {
    printf("fidlError: Unable to malloc work\n");
    exit(-1);
    }
gsl_permutation *perm = gsl_permutation_alloc(Mcol);
if(!(ATy=(double*)malloc(sizeof*ATy*Mcol))){
    printf("fidlError: Unable to malloc ATy\n");
    exit(-1);
    }

if(!(ATAm1AT=d2double(Mcol,data->nsubjects)))exit(-1);
gsl_matrix_view gATAm1AT = gsl_matrix_view_array(ATAm1AT[0],Mcol,data->nsubjects);
//START180404
if(!(AATAm1=d2double(data->nsubjects,Mcol)))exit(-1);
gsl_matrix_view gAATAm1 = gsl_matrix_view_array(AATAm1[0],data->nsubjects,Mcol);
if(!(yhat=(double*)malloc(sizeof*yhat*data->nsubjects))){
    printf("fidlError: Unable to malloc yhat\n");
    exit(-1);
    }


#if 0
for(i=0;i<data->npoints;i++){
    printf("\ni=%d dependent variable = %s\n",i,data->colptr[i+1]);

    if(!lcconstant)
        {for(l=j=0;j<data->nsubjects;j++)for(k=0;k<data->npoints;k++)if(k!=i)A[l++]=data->x[j][k];}
    else
        {for(l=j=0;j<data->nsubjects;j++){A[l++]=1.;for(k=0;k<data->npoints;k++)if(k!=i)A[l++]=data->x[j][k];}}


    //printf("i=%d A\n",i);for(l=j=0;j<data->nsubjects;j++){for(k=0;k<Mcol;k++,l++)printf("%g ",A[l]);printf("\n");}

    gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,&gA.matrix,&gA.matrix,0.0,&gATA.matrix);
    //printf("ATA\n");for(j=0;j<Mcol;j++){for(k=0;k<Mcol;k++)printf("%g ",ATA[j][k]);printf("\n");}

    //for(l=j=0;j<Mcol;j++)for(k=0;k<Mcol;k++,l++) temp_double[l] = ATA[j][k];
    memcpy(temp_double,ATA[0],Mcol*Mcol*sizeof*temp_double); 

    if(cond_norm1(temp_double,Mcol,&cond,ATAm1[0],perm))cond=0.;
    printf("Condition number cond_norm1: %f\n",cond);
    if(cond>condmax||cond==0.) {
        if(cond>condmax) printf("Condition number greater than %f\n",condmax);
        else printf("Design matrix not invertible.\n");
        printf("Computing singular value decomposition.\n");

        //for(l=j=0;j<Mcol;j++)for(k=0;k<Mcol;k++,l++) temp_double[l] = ATA[j][k];
        memcpy(temp_double,ATA[0],Mcol*Mcol*sizeof*temp_double); 

        td=gsl_svd_golubreinsch(temp_double,Mcol,Mcol,0.,V,S,ATAm1[0],&cond_norm2,&ndep,work);
        printf("cond_norm2= %f  ndep=%d\n",cond_norm2,ndep);

        }

    //printf("ATAm1\n");for(j=0;j<Mcol;j++){for(k=0;k<Mcol;k++)printf("%g ",ATAm1[j][k]);printf("\n");}

    gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,&gATAm1.matrix,&gA.matrix,0.0,&gATAm1AT.matrix);
    for(traceR=traceRR=0.,m=j=0;j<data->nsubjects;j++,m+=Mcol){
        for(k=0;k<data->nsubjects;k++){
            for(td=0.,n=m,l=0;l<Mcol;l++,n++)td+=A[n]*ATAm1AT[l][k];
            if(j==k){
                td = 1.-td;
                traceR += td;
                }
            traceRR += td*td;
            }
        }
    printf("traceR=%g\n",traceR);
    if(traceR>0.){
        for(j=0;j<Mcol;j++)for(ATy[j]=0.,l=j,k=0;k<data->nsubjects;k++,l+=Mcol)ATy[j]+=A[l]*data->x[k][i];
        for(yTy=0.,j=0;j<data->nsubjects;j++)yTy+=data->x[j][i]*data->x[j][i];

        //printf("yTy=%f\n",yTy);

        for(devmsq=yTy,j=0;j<Mcol;j++){
            devmsq -= ATAm1[j][j]*ATy[j]*ATy[j];
            for(k=j+1;k<Mcol;k++)devmsq -= 2.*ATAm1[j][k]*ATy[k]*ATy[j];
            }
        var=devmsq/traceR;
        printf("var=%g\n",var);
        if(var>0.){
            //for(stat=yTy,j=0;j<Mcol;j++)for(k=0;k<Mcol;k++)stat-=ATAm1[j][k]*ATy[k]*ATy[j];
            //printf("stat=%g\n",stat);
            //R2=1.-var*traceR/stat;
            //printf("R2=%g\n",R2);
            for(stat=0.,j=0;j<data->nsubjects;j++)stat+=data->x[j][i];
            stat=yTy-stat*stat/(double)data->nsubjects;
            R2=1.-devmsq/stat;
            printf("R2=%g\n",R2);
            }
        }     
    }
#endif
//START180404
#if 0
for(i=0;i<data->npoints;i++){
    printf("\ni=%d dependent variable = %s\n",i,data->colptr[i+1]);
#endif
for(i=0;i<nidx;i++){
    printf("\ni=%d dependent variable = %s\n",i,data->colptr[idx[i]+1]);fflush(stdout);

    #if 0
    if(!lcconstant)
        {for(l=j=0;j<data->nsubjects;j++)for(k=0;k<data->npoints;k++)if(k!=i)A[l++]=data->x[j][k];}
    else
        {for(l=j=0;j<data->nsubjects;j++){A[l++]=1.;for(k=0;k<data->npoints;k++)if(k!=i)A[l++]=data->x[j][k];}}
    #endif
    if(!lcconstant)
        {for(l=j=0;j<data->nsubjects;j++)for(k=0;k<nidx;k++)if(idx[k]!=idx[i])A[l++]=data->x[j][idx[k]];}
    else
        {for(l=j=0;j<data->nsubjects;j++){A[l++]=1.;for(k=0;k<nidx;k++)if(idx[k]!=idx[i])A[l++]=data->x[j][idx[k]];}}

    //printf("i=%d A\n",i);for(l=j=0;j<data->nsubjects;j++){for(k=0;k<Mcol;k++,l++)printf("%g ",A[l]);printf("\n");}fflush(stdout);

    gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,&gA.matrix,&gA.matrix,0.0,&gATA.matrix);
    //printf("ATA\n");for(j=0;j<Mcol;j++){for(k=0;k<Mcol;k++)printf("%g ",ATA[j][k]);printf("\n");}

    //for(l=j=0;j<Mcol;j++)for(k=0;k<Mcol;k++,l++) temp_double[l] = ATA[j][k];
    memcpy(temp_double,ATA[0],Mcol*Mcol*sizeof*temp_double);

    if(cond_norm1(temp_double,Mcol,&cond,ATAm1[0],perm))cond=0.;
    printf("Condition number cond_norm1: %f\n",cond);
    if(cond>condmax||cond==0.) {
        if(cond>condmax) printf("Condition number greater than %f\n",condmax);
        else printf("Design matrix not invertible.\n");
        printf("Computing singular value decomposition.\n");

        //for(l=j=0;j<Mcol;j++)for(k=0;k<Mcol;k++,l++) temp_double[l] = ATA[j][k];
        memcpy(temp_double,ATA[0],Mcol*Mcol*sizeof*temp_double);

        td=gsl_svd_golubreinsch(temp_double,Mcol,Mcol,0.,V,S,ATAm1[0],&cond_norm2,&ndep,work);
        printf("cond_norm2= %f  ndep=%d\n",cond_norm2,ndep);

        }

    //printf("ATAm1\n");for(j=0;j<Mcol;j++){for(k=0;k<Mcol;k++)printf("%g ",ATAm1[j][k]);printf("\n");}


    #if 0
    gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,&gATAm1.matrix,&gA.matrix,0.0,&gATAm1AT.matrix);
    for(traceR=traceRR=0.,m=j=0;j<data->nsubjects;j++,m+=Mcol){
        for(k=0;k<data->nsubjects;k++){
            for(td=0.,n=m,l=0;l<Mcol;l++,n++)td+=A[n]*ATAm1AT[l][k];
            if(j==k){
                td = 1.-td;
                traceR += td;
                }
            traceRR += td*td;
            }
        }
    printf("traceR=%g\n",traceR);
    if(traceR>0.){
        for(j=0;j<Mcol;j++)for(ATy[j]=0.,l=j,k=0;k<data->nsubjects;k++,l+=Mcol)ATy[j]+=A[l]*data->x[k][i];
        for(yTy=0.,j=0;j<data->nsubjects;j++)yTy+=data->x[j][i]*data->x[j][i];

        //printf("yTy=%f\n",yTy);

        for(devmsq=yTy,j=0;j<Mcol;j++){
            devmsq -= ATAm1[j][j]*ATy[j]*ATy[j];
            for(k=j+1;k<Mcol;k++)devmsq -= 2.*ATAm1[j][k]*ATy[k]*ATy[j];
            }
        var=devmsq/traceR;
        printf("var=%g\n",var);
        if(var>0.){
            //for(stat=yTy,j=0;j<Mcol;j++)for(k=0;k<Mcol;k++)stat-=ATAm1[j][k]*ATy[k]*ATy[j];
            //printf("stat=%g\n",stat);
            //R2=1.-var*traceR/stat;
            //printf("R2=%g\n",R2);
            for(stat=0.,j=0;j<data->nsubjects;j++)stat+=data->x[j][i];
            stat=yTy-stat*stat/(double)data->nsubjects;
            R2=1.-devmsq/stat;
            printf("R2=%g\n",R2);
            }
        }
    #endif
    #if 0
    //START180404
    for(j=0;j<Mcol;j++)for(ATy[j]=0.,l=j,k=0;k<data->nsubjects;k++,l+=Mcol)ATy[j]+=A[l]*data->x[k][i];
    gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,&gA.matrix,&gATAm1.matrix,0.0,&gAATAm1.matrix);
    for(j=0;j<data->nsubjects;j++)for(yhat[j]=0.,k=0;k<Mcol;k++)yhat[j]+=AATAm1[j][k]*ATy[k];
    for(SSE=0.,j=0;j<data->nsubjects;j++){
        td=data->x[j][i]-yhat[j];
        SSE+=td*td;
        }
    for(yTy=0.,j=0;j<data->nsubjects;j++)yTy+=data->x[j][i]*data->x[j][i];
    //printf("yTy=%f\n",yTy);

    for(stat=0.,j=0;j<data->nsubjects;j++)stat+=data->x[j][i];
    stat=yTy-stat*stat/(double)data->nsubjects;
    R2=1.-SSE/stat;
    printf("R2=%g    SSE=%f stat=%f\n",R2,SSE,stat);
    #endif
    //START180405
    gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,&gATAm1.matrix,&gA.matrix,0.0,&gATAm1AT.matrix);
    for(traceR=traceRR=0.,m=j=0;j<data->nsubjects;j++,m+=Mcol){
        for(k=0;k<data->nsubjects;k++){
            for(td=0.,n=m,l=0;l<Mcol;l++,n++)td+=A[n]*ATAm1AT[l][k];
            if(j==k){
                td = 1.-td;
                traceR += td;
                }
            traceRR += td*td;
            }
        }
    printf("traceR=%g\n",traceR);
    if(traceR>0.){

        //for(j=0;j<Mcol;j++)for(ATy[j]=0.,l=j,k=0;k<data->nsubjects;k++,l+=Mcol)ATy[j]+=A[l]*data->x[k][i];
        //for(yTy=0.,j=0;j<data->nsubjects;j++)yTy+=data->x[j][i]*data->x[j][i];
        //START180411
        for(j=0;j<Mcol;j++)for(ATy[j]=0.,l=j,k=0;k<data->nsubjects;k++,l+=Mcol)ATy[j]+=A[l]*data->x[k][idx[i]];
        for(yTy=0.,j=0;j<data->nsubjects;j++)yTy+=data->x[j][idx[i]]*data->x[j][idx[i]];

        for(devmsq=yTy,j=0;j<Mcol;j++){
            devmsq -= ATAm1[j][j]*ATy[j]*ATy[j];
            for(k=j+1;k<Mcol;k++)devmsq -= 2.*ATAm1[j][k]*ATy[k]*ATy[j];
            }
        var=devmsq/traceR;
        printf("var=%g  devmsq=%g traceR=%g\n",var,devmsq,traceR);

        //for(stat=0.,j=0;j<data->nsubjects;j++)stat+=data->x[j][i];
        //START180411
        for(stat=0.,j=0;j<data->nsubjects;j++)stat+=data->x[j][idx[i]];

        stat=yTy-stat*stat/(double)data->nsubjects;
        if(var>0.){
            //for(stat=yTy,j=0;j<Mcol;j++)for(k=0;k<Mcol;k++)stat-=ATAm1[j][k]*ATy[k]*ATy[j];
            //printf("stat=%g\n",stat);
            //R2=1.-var*traceR/stat;
            //printf("R2=%g\n",R2);
            //for(stat=0.,j=0;j<data->nsubjects;j++)stat+=data->x[j][i];
            //stat=yTy-stat*stat/(double)data->nsubjects;
            R2=1.-devmsq/stat;
            //printf("R2=%g\n",R2);

            //gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,&gA.matrix,&gATAm1.matrix,0.0,&gAATAm1.matrix);
            //for(j=0;j<data->nsubjects;j++)for(yhat[j]=0.,k=0;k<Mcol;k++)yhat[j]+=AATAm1[j][k]*ATy[k];
            //for(SSE=0.,j=0;j<data->nsubjects;j++){
            //    td=data->x[j][i]-yhat[j];
            //    SSE+=td*td;
            //    }

            printf("R2=%g devmsq=%g stat=%g\n",R2,devmsq,stat);
            //printf("R2=%g SSE=%g stat=%g\n",1.-SSE/stat,SSE,stat);
            }
        gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,&gA.matrix,&gATAm1.matrix,0.0,&gAATAm1.matrix);

        //for(k=0;k<Mcol;k++)printf("ATy[%d]=%f\n",k,ATy[k]);

        for(j=0;j<data->nsubjects;j++)for(yhat[j]=0.,k=0;k<Mcol;k++)yhat[j]+=AATAm1[j][k]*ATy[k];
        for(SSE=0.,j=0;j<data->nsubjects;j++){

            //printf("data->x[%d][%d]=%f yhat[%d]=%f\n",j,i,data->x[j][idx[i]],j,yhat[j]);

            //td=data->x[j][i]-yhat[j];
            //START180411
            td=data->x[j][idx[i]]-yhat[j];

            SSE+=td*td;
            }
        printf("R2= %g SSE=%g stat=%g\n",1.-SSE/stat,SSE,stat);
        }





    }


}
