/* Copyright 2/5/18 Washington University.  All Rights Reserved.
   fidl_logreg_montecarlo.cxx  $Revision: 1.1 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

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
#include "spatial_extent2.h"
#include "subs_util.h"

char const* Fileext[]={"",".4dfp.img",".dscalar.nii",".nii",".dtseries.nii"};

#define NSIM 100000

int main(int argc,char **argv)
{
char *maskf=NULL,*driverf=NULL,*root=NULL,atlas[7]="",filename[MAXNAME];
int i,j,k,i1,i2,j1,k1,k2,num_tc_files=0,SunOS_Linux,lenbrain,*brnidx=NULL,*ones,*eachn,*eachi,ntests,nvox,lizard,nthresh=0,*count,
    nreg,xdim,ydim,zdim;
float cnt,*tests;
double *df,*td0,*td1,*td2,*temp_double,*thresh=NULL;
unsigned long s=0;
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
Spatial_Extent2 *se=NULL;

if(argc<5){
    printf("    -tc_files: Checked for sizing and for type of file to output.\n");
    printf("    -mask:     Used to uncompress the scratch files.\n");
    printf("    -driver:   Specifies scratch files and behavioral measures (independent variables).\n");
    printf("               All work is done on the scratch files.\n");
    printf("    -root      Output root.\n");
    printf("    -thresh:   Thresholds.\n");
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
    if(!strcmp(argv[i],"-root") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        root = argv[++i];
    if(!strcmp(argv[i],"-thresh") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++nthresh;
        if(!(thresh=(double*)malloc(sizeof*thresh*nthresh))) {
            printf("fidlError: Unable to malloc thresh\n");
            exit(-1);
            }
        for(j=0;j<nthresh;j++) thresh[j]=strtod(argv[++i],NULL);
        }
    }
if((SunOS_Linux=checkOS())==-1)exit(-1);
if(!num_tc_files){printf("fidlError: -tc_files has not been specified. Need one to know what type of file to output.\n");exit(-1);}
if(!maskf){printf("fidlError: -mask has not been specified\n");exit(-1);}
if(!driverf){printf("fidlError: -driverf has not been specified\n");exit(-1);}

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

printf("dp->volall=%d lenbrain=%d\n",dp->volall,lenbrain);

if(!(tests=(float*)malloc(sizeof*tests*dp->volall))){
    printf("fidlError: Unable to malloc tests\n");
    exit(-1);
    }
for(i=0;i<dp->volall;i++)tests[i]=0.;


#if 0
for(cnt=0.,i=0;i<lenbrain;i++){
    if(lry->n0[i] && lry->n1[i] && !tests[brnidx[i]]){
        tests[brnidx[i]]=++cnt;
        for(j=i+1;j<lenbrain;j++){
            if(lry->n0[j]==lry->n0[i] && lry->n1[j]==lry->n1[i] && !tests[brnidx[j]]){
                for(k1=lry->starti[i],k2=lry->starti[j],k=0;k<lry->nindex[i];k++,k1++,k2++){
                    if((lry->y[k1]!=lry->y[k2])||(lry->index[k1]!=lry->index[k2]))break;
                    }
                if(k==lry->nindex[i])tests[brnidx[j]]=cnt;
                }
            }
        }
    }
#endif
//START810208

if(!(eachn=(int*)malloc(sizeof*eachn*lenbrain))){
    printf("fidlError: Unable to malloc eachn\n");
    exit(-1);
    }
for(i=0;i<lenbrain;i++)eachn[i]=0;
if(!(eachi=(int*)malloc(sizeof*eachi*lenbrain))){
    printf("fidlError: Unable to malloc eachi\n");
    exit(-1);
    }

for(cnt=0.,i1=-1,i2=i=0;i<lenbrain;i++){
    if(lry->n0[i] && lry->n1[i] && !tests[brnidx[i]]){

        //tests[brnidx[i]]=++cnt;
        tests[ eachi[i2++]=brnidx[i] ]=++cnt;

        eachn[++i1]++;
        for(j=i+1;j<lenbrain;j++){
            if(lry->n0[j]==lry->n0[i] && lry->n1[j]==lry->n1[i] && !tests[brnidx[j]]){
                for(k1=lry->starti[i],k2=lry->starti[j],k=0;k<lry->nindex[i];k++,k1++,k2++){
                    if((lry->y[k1]!=lry->y[k2])||(lry->index[k1]!=lry->index[k2]))break;
                    }
                if(k==lry->nindex[i]){

                    //tests[brnidx[j]]=cnt;
                    tests[ eachi[i2++]=brnidx[j] ]=cnt;

                    eachn[i1]++;
                    }
                }
            }
        }
    }
ntests=i1+1;
nvox=i2;

printf("Number of tests(cnt)=%f  Number of voxels(i2)=%d  Number of tests(i1+1)=%d\n",cnt,i2,i1+1);
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
w1->swapbytes=0;
w1->temp_float=tests;
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
sprintf(filename,"%s_tests%s%s",root,ap->str,Fileext[dp->filetypeall]);
if(!write1(filename,w1))exit(-1);
printf("Tests written to %s\n",filename);fflush(stdout);

if(!(df=(double*)malloc(sizeof*df*ntests))){
    printf("fidlError: Unable to malloc df\n");
    exit(-1);
    }
for(i=0;i<ntests;i++)df[i]=(double)data->npoints;
printf("df[0]=%f\n",df[0]);

if(!(td0=(double*)malloc(sizeof*td0*ntests))){
    printf("fidlError: Unable to malloc td0\n");
    exit(-1);
    }
if(!(td1=(double*)malloc(sizeof*td1*ntests))){
    printf("fidlError: Unable to malloc td1\n");
    exit(-1);
    }
if(!(temp_double=(double*)malloc(sizeof*temp_double*dp->volall))){
    printf("fidlError: Unable to malloc temp_double\n");
    exit(-1);
    }
for(i=0;i<dp->volall;i++)temp_double[i]=0.;
if(nthresh>1) {
    if(!(td2=(double*)malloc(sizeof*td2*dp->volall))) {
        printf("fidlError: Unable to malloc td2\n");
        exit(-1);
        }
    for(i=0;i<dp->volall;i++)td2[i]=0.;
    }
else {
    td2 = temp_double;
    }
if(!(count=(int*)malloc(sizeof*count*(nvox+1)*nthresh))) {
    printf("fidlError: Unable to malloc count\n");
    exit(-1);
    }
for(i=0;i<(nvox+1)*nthresh;i++) count[i]=0;

printf("nthresh= ");for(i=0;i<nthresh;i++)printf("%f ",thresh[i]);printf("\n");

if(!(se=spatial_extent2init(xdim,ydim,zdim,lenbrain,brnidx))) exit(-1);

printf("seed = %lu\n",s);
const gsl_rng_type *T = gsl_rng_mt19937;
gsl_rng *r = gsl_rng_alloc(T);
gsl_rng_set(r,s);
for(i=0;i<(int)NSIM;i++){
    for(j=0;j<ntests;j++)td0[j]=gsl_ran_chisq(r,df[0]);
    x2_to_z(td0,td1,ntests,df);


    //for(i2=j=0;j<=ntests;j++){
    //START170221
    for(i2=j=0;j<ntests;j++){

        for(k=0;k<eachn[j];k++,i2++){
            temp_double[eachi[i2]]=td1[j];
            }
        } 

    for(j1=j=0;j<nthresh;j++,j1+=nvox+1) {
        if(nthresh>1) for(k=0;k<nvox;k++) td2[eachi[k]]=temp_double[eachi[k]];

        //if((nreg=spatial_extent2(td2,(float*)NULL,thresh[j],1,dp->volall,1,se))) {
        if((nreg=spatial_extent2(td2,(float*)NULL,thresh[j],1,nvox,1,se))){ 
            //don't seed to exceed the number of voxels with tests in the image, though this is just to speed it up
            //won't change the answer

            for(k=0;k<nreg;k++) count[j1+se->regsize[k]]++;
            }
        }
    }

sprintf(filename,"%sFULL_table.txt",root);
if(!(fp=fopen_sub(filename,"w"))) exit(-1);
fprintf(fp,"#%s nsim=%d ntests=%d nvox=%d lenbrain=%d\ndf\tthreshold\tcluster\tp\n",maskf,(int)NSIM,ntests,nvox,lenbrain);
for(i1=1,i=0;i<nthresh;i++,i1++) { /*i1++ skips 0*/
    for(j=1;j<=nvox;j++,i1++) {
        if((lizard=count[i1])) {
            for(k1=i1+1,k=j+1;k<=nvox;k++,k1++) lizard+=count[k1];
            fprintf(fp,"%.2f\t%.2f\t%d\t%g\n",df[0],thresh[i],j,(double)lizard/(double)NSIM);
            }
        }
    }
fflush(fp);fclose(fp);
printf("Output written to %s\n",filename);
}
