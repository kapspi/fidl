/* Copyright 2/20/18 Washington University.  All Rights Reserved.
   fidl_logreg_fdr.cxx  $Revision: 1.1 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gsl/gsl_sort_float.h>
#include <gsl/gsl_cdf.h>

#include "checkOS.h"
#include "mask.h"
#include "read_data.h"
#include "logreg.h"
#include "get_atlas.h"
#include "get_atlas_param.h"
#include "dim_param2.h"
#include "files_struct.h"
#include "write1.h"
#include "filetype.h"
#include "subs_cifti.h"
#include "x2_to_z.h"
#include "subs_util.h"
#include "stack.h"
#include "fidl.h"

//START180316
#include "atlas.h"
#include "nifti_getmni.h"

//START180322
#include "spatial_extent2.h"


char const* Fileext[]={"",".4dfp.img",".dscalar.nii",".nii",".dtseries.nii"};

#define NSIM 100000

int main(int argc,char **argv)
{
char *maskf=NULL,*driverf=NULL,*root=NULL,*zf=NULL,atlas[7]="",filename[MAXNAME],*strptr,string[MAXNAME];
int i,j,k,i1,i2,j1,k1,k2,num_tc_files=0,SunOS_Linux,lenbrain,*brnidx=NULL,*ones,*eachn,*eachi,ntests,nvox,nq=0,xdim,ydim,zdim,
    lcstepup=0,lcstepdown=0,**eachiptr,min,extent=0,lcroot=0; 
long lesions=1;
size_t *p;
float cnt,*tests,*temp_float,*tf0,*tf1,*nonzero,xyz[3],coor[3],zthresh=(float)UNSAMPLED_VOXEL,*actmask=NULL;
double *q=NULL,qdivv,z,z0,qv,e,c,zu,zd,*temp_double=NULL;
FILE *fp;
mask ms;
Data *data=NULL;
LRy *lry=NULL;
Files_Struct *tc_files=NULL;
Atlas_Param *ap;
Dim_Param2 *dp;
W1 *w1=NULL;
Interfile_header *ifh=NULL;
XmlS *xmls;
int64_t dims[3];
stack zs;
Spatial_Extent2 *se=NULL;

if(argc<5){
    printf("    -tc_files: Checked for sizing and for type of file to output.\n");
    printf("    -mask:     Used to uncompress the scratch files.\n");
    printf("    -driver:   Specifies scratch files and behavioral measures (independent variables).\n");
    printf("               All work is done on the scratch files.\n");
    printf("    -root      Output root.\n");
    printf("    -q:        Q values. More than one is fine.\n");
    printf("    -z:        Gaussianized chisquared statistics to be FDR corrected.\n");

    //START180321
    printf("    -zthresh:  Threshold to be applied before FDR correction.\n");
    //START180322
    printf("    -extent:   Number of face connected voxels.\n");

    printf("    -lesions:  Minimum number of subjects with lesions.\n");
    printf("    -stepup    Benjanmini and Hochberg FDRBH\n");
    printf("    -stepdown  Benjamini and Liu FDRBL\n");
    printf("               See Ge et al Stat Sin. 2008: 18(3)881-904\n");
    printf("                   Some step-down procedures controlling the false discovery rate under dependence\n");
    exit(-1);
    }
for(i=1;i<argc;i++){
    if(!strcmp(argv[i],"-tc_files") && argc > i+1){
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_tc_files;
        if(!(tc_files=get_files(num_tc_files,&argv[i+1]))) exit(-1);
        i += num_tc_files;
        }
    if(!strcmp(argv[i],"-mask") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        maskf = argv[++i];
    if(!strcmp(argv[i],"-driver") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        driverf = argv[++i];

    //if(!strcmp(argv[i],"-root") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
    //    root = argv[++i];
    //START180508
    if(!strcmp(argv[i],"-root") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        {root = argv[++i];lcroot=1;}

    if(!strcmp(argv[i],"-q") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++nq;
        if(!(q=(double*)malloc(sizeof*q*nq))) {
            printf("fidlError: Unable to malloc q\n");
            exit(-1);
            }
        for(j=0;j<nq;j++) q[j]=strtod(argv[++i],NULL);
        }
    if(!strcmp(argv[i],"-z") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        zf = argv[++i];


    //START180321
    if(!strcmp(argv[i],"-zthresh") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        zthresh = strtof(argv[++i],NULL); 
    //START180322
    if(!strcmp(argv[i],"-extent") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        extent=(int)strtol(argv[++i],NULL,10);

    if(!strcmp(argv[i],"-lesions") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        lesions = strtol(argv[++i],NULL,10);
    if(!strcmp(argv[i],"-stepup"))lcstepup=1;
    if(!strcmp(argv[i],"-stepdown"))lcstepdown=1;
    }
if((SunOS_Linux=checkOS())==-1)exit(-1);
if(!num_tc_files){printf("fidlError: -tc_files has not been specified. Need one to know what type of file to output.\n");exit(-1);}
if(!maskf){printf("fidlError: -mask has not been specified\n");exit(-1);}
if(!driverf){printf("fidlError: -driverf has not been specified\n");exit(-1);}
if(!lcstepup&&!lcstepdown){printf("fidlInfo: Need -stepup or/and -stepdown to output FDR maps. Only test values will be output.\n");}

//START180328
if(!lesions){
    printf("Setting lesions to 1.\n");
    lesions=1;
    } 

if(!(dp=dim_param2(tc_files->nfiles,tc_files->files,SunOS_Linux)))exit(-1);
if(dp->volall==-1){printf("fidlError: All files must be the same size. Abort!\n");exit(-1);}

if(!(ms.read_mask(maskf,SunOS_Linux,(LinearModel*)NULL)))exit(-1);

if(!(data=read_data(driverf,1,0,0,0)))exit(-1);
printf("data->nsubjects=%d data->npoints_per_line[0]=%d data->total_npoints_per_line=%d data->ncol=%d data->npoints=%d\n",
    data->nsubjects,data->npoints_per_line[0],data->total_npoints_per_line,data->ncol,data->npoints);
printf("data->colptr=");for(i=0;i<data->ncol;i++)printf("%s ",data->colptr[i]);printf("\n");

brnidx=ms.get_brnidx(lenbrain,xdim,ydim,zdim);

if(!(lry=create_LRy(lenbrain,data->nsubjects)))exit(-1);
if(!(ones=(int*)malloc(sizeof*ones*data->nsubjects))){
    printf("fidlError: Unable to malloc ones\n");
    exit(-1);
    }
for(i=0;i<data->nsubjects;i++)ones[i]=1;
if(!(assign_LRy(data,ones,lenbrain,lry)))exit(-1);
free(ones);

//printf("dp->volall=%d lenbrain=%d\n",dp->volall,lenbrain);
printf("dp->volall=%d lenbrain=%d zthresh=%g\n",dp->volall,lenbrain,zthresh);

if(!(tests=(float*)malloc(sizeof*tests*dp->volall))){
    printf("fidlError: Unable to malloc tests\n");
    exit(-1);
    }
for(i=0;i<dp->volall;i++)tests[i]=0.;

if(!(eachn=(int*)malloc(sizeof*eachn*lenbrain))){
    printf("fidlError: Unable to malloc eachn\n");
    exit(-1);
    }
for(i=0;i<lenbrain;i++)eachn[i]=0;
if(!(eachi=(int*)malloc(sizeof*eachi*lenbrain))){
    printf("fidlError: Unable to malloc eachi\n");
    exit(-1);
    }

if(!(nonzero=(float*)malloc(sizeof*nonzero*dp->volall))){
    printf("fidlError: Unable to malloc nonzero\n");
    exit(-1);
    }
for(i=0;i<dp->volall;i++)nonzero[i]=0.;

//START180322
if(!(tf1=(float*)malloc(sizeof*tf1*dp->volall))){
    printf("fidlError: Unable to malloc tf1\n");
    exit(-1);
    }

//START180321
if(!(temp_float=zs.stack0(zf,SunOS_Linux)))exit(-1);


for(cnt=0.,min=lenbrain,i1=-1,i2=i=0;i<lenbrain;i++){
    nonzero[brnidx[i]]=(float)lry->n1[i];

    //if(lry->n0[i] && lry->n1[i]>=lesions && !tests[brnidx[i]] && temp_float[brnidx[i]]>=zthresh){
    //START180321
    if(lry->n0[i] && lry->n1[i]>=lesions && !tests[brnidx[i]] && fabsf(temp_float[brnidx[i]])>=zthresh){


        if(lry->n1[i]<min)min=lry->n1[i];
        tests[ eachi[i2++]=brnidx[i] ]=++cnt;
        eachn[++i1]++;
        for(j=i+1;j<lenbrain;j++){
            if(lry->n0[j]==lry->n0[i] && lry->n1[j]==lry->n1[i] && !tests[brnidx[j]]){
                for(k1=lry->starti[i],k2=lry->starti[j],k=0;k<lry->nindex[i];k++,k1++,k2++){
                    if((lry->y[k1]!=lry->y[k2])||(lry->index[k1]!=lry->index[k2]))break;
                    }
                if(k==lry->nindex[i]){
                    tests[ eachi[i2++]=brnidx[j] ]=cnt;
                    eachn[i1]++;
                    }
                }
            }
        }
    }
ntests=i1+1;
nvox=i2;

//START180315
if(!(eachiptr=(int**)malloc(sizeof*eachiptr*ntests))){
    printf("fidlError: Unable to malloc eachiptr\n");
    exit(-1);
    }
for(eachiptr[0]=&eachi[0],i=1;i<ntests;i++) eachiptr[i]=eachiptr[i-1]+eachn[i-1];

//printf("Number of tests(cnt)=%f  Number of voxels(i2)=%d  Number of tests(i1+1)=%d\n",cnt,i2,i1+1);
//printf("lesions=%ld Number of tests(cnt)=%f  Number of voxels(i2)=%d  Number of tests(i1+1)=%d\n",lesions,cnt,i2,i1+1);
printf("lesions=%ld min=%d Number of tests(cnt)=%f  Number of voxels(i2)=%d  Number of tests(i1+1)=%d\n",lesions,min,cnt,i2,i1+1);

for(i2=i=0;i<=i1;i++){
    //printf("eachn[%d]=%d\n",i,eachn[i]);
    //printf("    ");for(j=0;j<eachn[i];j++)printf("%d ",eachi[i2++]);printf("\n");
    i2+=eachn[i];
    }
printf("CHECK Number of voxels(i2)=%d\n",i2);

get_atlas(dp->volall,atlas);
if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL,tc_files->files[0])))exit(-1);
if(!(w1=write1_init()))exit(-1);
w1->filetype=dp->filetypeall;
if(dp->filetypeall==(int)IMG){
    w1->lenbrain=lenbrain;
    w1->how_many=ap->vol;
    if(!(ifh=read_ifh(tc_files->files[0],(Interfile_header*)NULL)))exit(-1);
    ifh->dim1=ap->xdim;ifh->dim2=ap->ydim;ifh->dim3=ap->zdim;ifh->dim4=1;
    ifh->center[0]=(float)ap->center[0];ifh->center[1]=(float)ap->center[1];ifh->center[2]=(float)ap->center[2];
    ifh->mmppix[0]=(float)ap->mmppix[0];ifh->mmppix[1]=(float)ap->mmppix[1];ifh->mmppix[2]=(float)ap->mmppix[2];
    w1->ifh=ifh;
    }
else if(dp->filetypeall==(int)CIFTI||dp->filetypeall==(int)CIFTI_DTSERIES){
    if(!(xmls=cifti_getxml(tc_files->files[0])))exit(-1);
    w1->xmlsize=xmls->size;;
    w1->cifti_xmldata=xmls->data;
    dims[0]=(int64_t)dp->xdim[0];dims[1]=(int64_t)dp->ydim[0];dims[2]=(int64_t)dp->zdim[0];
    w1->dims=dims;
    w1->tdim=(int64_t)1;
    }
else if(dp->filetypeall==(int)NIFTI){
    w1->file_hdr=tc_files->files[0];
    }

if(!root){
    strcpy(string,zf);
    if(!(root=get_tail_sans_ext(string)))exit(-1);
    }

w1->temp_float=tests;

//sprintf(filename,"%s_testsn%ldz%g%s%s",root,lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,ap->str,Fileext[dp->filetypeall]);
//START180508
sprintf(filename,"%s_testsn%ldz%g%s%s",root,lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,lcroot?ap->str:"",
    Fileext[dp->filetypeall]);

if(!write1(filename,w1))exit(-1);
printf("Tests written to %s\n",filename);fflush(stdout);

w1->temp_float=nonzero;

//sprintf(filename,"%s_lesions%s%s",root,ap->str,Fileext[dp->filetypeall]);
//START180508
sprintf(filename,"%s_lesions%s%s",root,lcroot?ap->str:"",Fileext[dp->filetypeall]);

if(!write1(filename,w1))exit(-1);
printf("Lesions written to %s\n",filename);fflush(stdout);

for(i=0;i<dp->volall;i++)tf1[i]=0.;
for(i=0;i<nvox;i++)tf1[eachi[i]]=temp_float[eachi[i]];
w1->temp_float=tf1;

//sprintf(filename,"%s_n%ldz%g%s%s",root,lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,ap->str,Fileext[dp->filetypeall]);
//START180508
sprintf(filename,"%s_n%ldz%g%s%s",root,lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,lcroot?ap->str:"",Fileext[dp->filetypeall]);

if(!write1(filename,w1))exit(-1);
printf("Image written to %s\n",filename);fflush(stdout);

if(extent){
    if(!(temp_double=(double*)malloc(sizeof*temp_double*dp->volall))) {
        printf("fidlError: Unable to malloc temp_double\n");
        exit(-1);
        }
    for(i=0;i<dp->volall;i++)temp_double[i]=0.;
    for(i=0;i<lenbrain;i++)
        temp_double[brnidx[i]]=tf1[brnidx[i]]==(float)UNSAMPLED_VOXEL?(double)UNSAMPLED_VOXEL:(double)tf1[brnidx[i]];
    if(!(actmask=(float*)malloc(sizeof*actmask*dp->volall))) {
        printf("fidlError: Unable to malloc actmask\n");
        exit(-1);
        }
    if(!(se=spatial_extent2init(xdim,ydim,zdim,lenbrain,brnidx)))exit(-1);

    #if 0 
    spatial_extent2(temp_double,actmask,(double)zthresh,extent,lenbrain,1,se);
    for(i=0;i<lenbrain;i++)if(!actmask[brnidx[i]]){tf1[brnidx[i]]=0.;}
    sprintf(filename,"%s_n%ldz%gvox%d%s%s",root,lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,extent,ap->str,
        Fileext[dp->filetypeall]);
    if(!write1(filename,w1))exit(-1);
    printf("Image written to %s\n",filename);fflush(stdout);
    #endif
    //START180507
    if(spatial_extent2(temp_double,actmask,(double)zthresh,extent,lenbrain,1,se)){
        for(i=0;i<lenbrain;i++)if(!actmask[brnidx[i]]){tf1[brnidx[i]]=0.;}

        //sprintf(filename,"%s_n%ldz%gvox%d%s%s",root,lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,extent,ap->str,
        //    Fileext[dp->filetypeall]);
        //START180508
        sprintf(filename,"%s_n%ldz%gvox%d%s%s",root,lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,extent,lcroot?ap->str:"",
            Fileext[dp->filetypeall]);

        if(!write1(filename,w1))exit(-1);
        printf("Image written to %s\n",filename);fflush(stdout);
        }

    }


//START180329
if(zthresh>(float)UNSAMPLED_VOXEL){
    for(i=0;i<dp->volall;i++)tf1[i]=0.;

    //for(i=0;i<lenbrain;i++)if(temp_float[brnidx[i]]>=zthresh)tf1[brnidx[i]]=temp_float[brnidx[i]];
    //START180507
    for(i=0;i<lenbrain;i++)if(fabsf(temp_float[brnidx[i]])>=zthresh)tf1[brnidx[i]]=temp_float[brnidx[i]];


    //sprintf(filename,"%s_z%g%s%s",root,zthresh,ap->str,Fileext[dp->filetypeall]);
    //START180508
    sprintf(filename,"%s_z%g%s%s",root,zthresh,lcroot?ap->str:"",Fileext[dp->filetypeall]);

    if(!write1(filename,w1))exit(-1);
    printf("Image written to %s\n",filename);fflush(stdout);

    if(extent){
        for(i=0;i<dp->volall;i++)temp_double[i]=0.;
        for(i=0;i<lenbrain;i++)temp_double[brnidx[i]]=(double)tf1[brnidx[i]];

        #if 0
        spatial_extent2(temp_double,actmask,(double)zthresh,extent,lenbrain,1,se);
        for(i=0;i<lenbrain;i++)if(!actmask[brnidx[i]]){tf1[brnidx[i]]=0.;}
        sprintf(filename,"%s_z%gvox%d%s%s",root,zthresh,extent,ap->str,Fileext[dp->filetypeall]);
        if(!write1(filename,w1))exit(-1);
        printf("Image written to %s\n",filename);fflush(stdout);
        #endif
        //START180507
        if(spatial_extent2(temp_double,actmask,(double)zthresh,extent,lenbrain,1,se)){
            for(i=0;i<lenbrain;i++)if(!actmask[brnidx[i]]){tf1[brnidx[i]]=0.;}

            //sprintf(filename,"%s_z%gvox%d%s%s",root,zthresh,extent,ap->str,Fileext[dp->filetypeall]);
            //START180508
            sprintf(filename,"%s_z%gvox%d%s%s",root,zthresh,extent,lcroot?ap->str:"",Fileext[dp->filetypeall]);

            if(!write1(filename,w1))exit(-1);
            printf("Image written to %s\n",filename);fflush(stdout);
            }

        }
    }


printf("q->z");for(i=0;i<nq;i++)printf(" %f->%f",q[i],gsl_cdf_ugaussian_Qinv(q[i]));printf("\n");
if(!(tf0=(float*)malloc(sizeof*tf0*ntests))){
    printf("fidlError: Unable to malloc tf0\n");
    exit(-1);
    }

//for(i1=i=0;i<ntests;i1+=eachn[i++])tf0[i]=temp_float[eachi[i1]];
//START180507
for(i=0;i<ntests;i++)tf0[i]=fabsf(temp_float[eachiptr[i][0]]);

//printf("CHECK i1=%d eachn[%d]=%d i1+eachn[%d]=%d\n",i1,i,eachn[i],i,i1+eachn[i]);

if(!(p=(size_t*)malloc(sizeof*p*ntests))){
    printf("fidlError: Unable to malloc p\n");
    exit(-1);
    }
gsl_sort_float_index(p,tf0,1,ntests);

strptr=root?root:get_tail_sans_ext(zf);
for(i=0;i<nq;i++){
    qdivv=q[i]/(double)ntests;
    z0=gsl_cdf_ugaussian_Qinv(q[i]);
    qv=q[i]*(double)ntests;
    for(k=1,j1=ntests;--j1>=0;k++){

        if(tf0[p[j1]]>=z0){
        //START180504
        //if(fabs((double)tf0[p[j1]])>=z0){

            zu=gsl_cdf_ugaussian_Qinv((double)k*qdivv);
            e = (double)(ntests-k+1);
            c=1.-pow(1.-(qv/e>1.?1.:qv/e),1./e);
            zd=gsl_cdf_ugaussian_Qinv(c);

            //printf("q=%g j1=%d tf0[%zd]=%f zu=%f zd=%f\n",q[i],j1,p[j1],tf0[p[j1]],zu,zd);
            //START180507
            printf("q=%g j1=%d tf0[%zd]=%f temp_float[%d]=%f zu=%f zd=%f\n",q[i],j1,p[j1],tf0[p[j1]],eachiptr[p[j1]][0],
                temp_float[eachiptr[p[j1]][0]],zu,zd);

            }
        }
    if(lcstepup){
        //step up (to a larger p), large z to small z, Benjanmini and Hochberg
        for(k=1,j1=ntests;--j1>=0;k++){

            if(tf0[p[j1]]>=z0){
            //START180504
            //if(fabs((double)tf0[p[j1]])>=z0){

                z=gsl_cdf_ugaussian_Qinv((double)k*qdivv);
                //printf("j1=%d tf0[%zd]=%f z=%f\n",j1,p[j1],tf0[p[j1]],z);

                if((double)tf0[p[j1]]<(double)z){j1++;break;}
                //START180504
                //if(fabs((double)tf0[p[j1]])<z){j1++;break;}

                }
            }
        if(j1==-1)j1=0;
        if(j1<ntests){
            for(j=0;j<dp->volall;j++)tf1[j]=0.;

            //for(j=ntests;--j>=j1;)for(k=0;k<eachn[p[j]];k++)tf1[eachiptr[p[j]][k]]=tf0[p[j]]; 
            //START180507
            //for(j=ntests;--j>=j1;)for(k=0;k<eachn[p[j]];k++)tf1[eachiptr[p[j]][k]]=tfsign[p[j]]; 
            for(j=ntests;--j>=j1;)for(k=0;k<eachn[p[j]];k++)tf1[eachiptr[p[j]][k]]=temp_float[eachiptr[p[j]][k]]; 


            w1->temp_float=tf1;

            //sprintf(filename,"%s_FDRBHq%gn%ldz%g%s%s",strptr,q[i],lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,ap->str,
            //    Fileext[dp->filetypeall]);
            //START180508
            sprintf(filename,"%s_FDRBHq%gn%ldz%g%s%s",strptr,q[i],lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,lcroot?ap->str:"",
                Fileext[dp->filetypeall]);

            if(!write1(filename,w1))exit(-1);
            printf("FDRBH written to %s\n",filename);fflush(stdout);

            printf("j1=%d ntests=%d ntests-j1=%d\n",j1,ntests,ntests-j1); 
            for(j=0;j<dp->volall;j++)tf1[j]=0.;

            for(k=2,j=ntests;--j>=j1;k++)tf1[eachiptr[p[j]][0]]=(float)k; //KEEP
            //for(k=2,j=ntests;--j>=j1;k++)tf1[eachiptr[p[j]][0]]=tf0[p[j]]; //CHECK

            //sprintf(filename,"%s_FDRBHq%gn%ldz%g%s_reg%s",strptr,q[i],lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,ap->str,
            //    Fileext[dp->filetypeall]);
            //180508
            sprintf(filename,"%s_FDRBHq%gn%ldz%g%s_reg%s",strptr,q[i],lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,
                lcroot?ap->str:"",Fileext[dp->filetypeall]);

            if(!write1(filename,w1))exit(-1);
            printf("Single voxel regions (one for each test) written to %s\n",filename);fflush(stdout);

            //sprintf(filename,"%s_FDRBHq%gn%ldz%g%s_reg.ifh",strptr,q[i],lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,ap->str);
            //START180508
            sprintf(filename,"%s_FDRBHq%gn%ldz%g%s_reg.ifh",strptr,q[i],lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,
                lcroot?ap->str:"");


            if(!(fp=fopen_sub(filename,"w")))exit(-1);
            for(k=2,j=ntests;--j>=j1;k++){
                getxyz(eachiptr[p[j]][0],xdim,ydim,xyz);
                if(!nifti_getmni(tc_files->files[0],xyz,coor))exit(-1);
                //printf("%f k= %d xyz= %g %g %g coor= %g %g %g\n",tf0[p[j]],k,xyz[0],xyz[1],xyz[2],coor[0],coor[1],coor[2]);
                fprintf(fp,"region names := %d val%d_%g_%g_%g\n",k-2,k,coor[0],coor[1],coor[2]);
                } 
            fclose(fp);
            printf("Region names written to %s\n",filename);fflush(stdout);
            
            //nifti_getmni for coordinates, atlas.cxx
            //flip the stack
            //write out as img region file
            //to check, print out the statistical value instead of region number

            //START180322
            if(extent){
                for(j=0;j<dp->volall;j++)temp_double[j]=0.;
                for(j=ntests;--j>=j1;)
                    for(k=0;k<eachn[p[j]];k++){

                        //temp_double[eachiptr[p[j]][k]]=(double)(tf1[eachiptr[p[j]][k]]=tf0[p[j]]);
                        //START180508
                        temp_double[eachiptr[p[j]][k]]=(double)(tf1[eachiptr[p[j]][k]]=temp_float[eachiptr[p[j]][k]]);

                        }

                #if 0
                spatial_extent2(temp_double,actmask,(double)zthresh,extent,lenbrain,1,se);
                for(j=0;j<lenbrain;j++)if(!actmask[brnidx[j]]){tf1[brnidx[j]]=0.;}
                sprintf(filename,"%s_FDRBHq%gn%ldz%gvox%d%s%s",strptr,q[i],lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,
                    extent,ap->str,Fileext[dp->filetypeall]);
                if(!write1(filename,w1))exit(-1);
                printf("Image written to %s\n",filename);fflush(stdout);
                #endif
                //START180507
                if(spatial_extent2(temp_double,actmask,(double)zthresh,extent,lenbrain,1,se)){
                    for(j=0;j<lenbrain;j++)if(!actmask[brnidx[j]]){tf1[brnidx[j]]=0.;}

                    //sprintf(filename,"%s_FDRBHq%gn%ldz%gvox%d%s%s",strptr,q[i],lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,
                    //    extent,ap->str,Fileext[dp->filetypeall]);
                    //START180508
                    sprintf(filename,"%s_FDRBHq%gn%ldz%gvox%d%s%s",strptr,q[i],lesions,zthresh==(float)UNSAMPLED_VOXEL?0.:zthresh,
                        extent,lcroot?ap->str:"",Fileext[dp->filetypeall]);

                    if(!write1(filename,w1))exit(-1);
                    printf("Image written to %s\n",filename);fflush(stdout);
                    }
    
                }


            }
        }
    if(lcstepdown){
        //step down (to a smaller p), small z to large z, Benjamini and Liu (see Ge Stat Sin. 2008: 18(3)881-904 
        //Some step-down procedures controlling the false discovery rate under dependence
        //The implementation follows the step up procedure
        //step up (to a larger p), large z to small z, Benjanmini and Hochberg
        for(k=1,j1=ntests;--j1>=0;k++){
            if(tf0[p[j1]]>=z0){
                e = (double)(ntests-k+1);
                c=1.-pow(1.-(qv/e>1.?1.:qv/e),1./e);
                z=gsl_cdf_ugaussian_Qinv(c);
                //printf("j1=%d tf0[%zd]=%f z=%f\n",j1,p[j1],tf0[p[j1]],z);
                if((double)tf0[p[j1]]<(double)z){j1++;break;}
                }
            }
        if(j1<ntests){
            for(j=0;j<dp->volall;j++)tf1[j]=0.;
            for(i1=j=0;j<j1;i1+=eachn[j++]);
            for(j=j1;j<ntests;j++){
                for(k=0;k<eachn[j];k++,i1++){
                    tf1[eachi[i1]]=tf0[p[j]];
                    }
                }
            w1->temp_float=tf1;

            //sprintf(filename,"%s_FDRBLq%gn%ld%s%s",strptr,q[i],lesions,ap->str,Fileext[dp->filetypeall]);
            //START180508
            sprintf(filename,"%s_FDRBLq%gn%ld%s%s",strptr,q[i],lesions,lcroot?ap->str:"",Fileext[dp->filetypeall]);

            if(!write1(filename,w1))exit(-1);
            printf("FDRBL written to %s\n",filename);fflush(stdout);
            }
        }
    }
}
