/* Copyright 3/14/18 Washington University.  All Rights Reserved.
   fidl_lesion.cxx  $Revision: 1.1 $ */
//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <iostream>

#include "checkOS.h"
#include "mask.h"
#include "dim_param2.h"
#include "files_struct.h"
#include "spatial_extent2.h"

#include "stack.h"
#include "fidl.h"

using namespace std;

int main(int argc,char **argv)
{
char *maskf=NULL,string[MAXNAME],*strptr,*saveptr;
int i,j,jj,nfiles=0,SunOS_Linux,lenbrain,*brnidx=NULL,xdim,ydim,zdim,min,max,min0,max0,nreg0,nvox0; 
    //*nreg=NULL,*nvox=NULL,*regsize=NULL
size_t i1;
float *temp_float=NULL;
double *temp_double=NULL,sum,sum2,mean,sd,sum0,sum20,mean_infvol,mean_infno;
//FILE *fp;
Files_Struct *files=NULL;
Spatial_Extent2 *se;
Dim_Param2 *dp;
mask ms;
stack fs;

if(argc<5){
    cout<<"    -files: Lesion files."<<endl;
    cout<<"    -mask:  Lesion mask."<<endl;
    exit(-1);
    }
for(i=1;i<argc;i++){
    if(!strcmp(argv[i],"-files") && argc > i+1){
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++nfiles;
        if(!(files=get_files(nfiles,&argv[i+1]))) exit(-1);
        i += nfiles;
        }
    if(!strcmp(argv[i],"-mask") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        maskf = argv[++i];
    }
if((SunOS_Linux=checkOS())==-1)exit(-1);
if(!nfiles){cout<<"fidlError: -files has not been specified. These should be the lesion files."<<endl;exit(-1);}
if(!maskf){cout<<"fidlError: -mask has not been specified"<<endl;exit(-1);}

if(!(dp=dim_param2(files->nfiles,files->files,SunOS_Linux)))exit(-1);
if(dp->volall==-1){cout<<"fidlError: All files must be the same size. Abort!"<<endl;exit(-1);}

if(!(ms.read_mask(maskf,SunOS_Linux,(LinearModel*)NULL)))exit(-1);
brnidx=ms.get_brnidx(lenbrain,xdim,ydim,zdim);

try{
    temp_float=new float[dp->volall];
    temp_double=new double[dp->volall];
    //nreg=new int[files->nfiles];
    //nvox=new int[files->nfiles];
    //regsize=new int[files->nfiles*lenbrain];
    }
catch(bad_alloc& ba){
    cout<<"bad_alloc caught: "<<ba.what()<<endl;
    }

if(!(se=spatial_extent2init(xdim,ydim,zdim,lenbrain,brnidx)))exit(-1);

cout<<"name\tinfvol\tinfno\tinfmean\tinfsd\tinfmin\tinfmax\t\tinfarcts"<<endl;
for(sum0=sum20=0.,min0=lenbrain,nreg0=nvox0=max0=jj=0,i1=0;i1<files->nfiles;i1++){
    if(!(fs.stack1(files->files[i1],SunOS_Linux,temp_float)))exit(-1);

    for(j=0;j<dp->volall;j++)temp_double[j]=temp_float[j]==(float)UNSAMPLED_VOXEL?(double)UNSAMPLED_VOXEL:(double)temp_float[j];

    //if((nreg[i1]=spatial_extent2(temp_double,(float*)NULL,0.5,1,lenbrain,1,se))){
    //if((nreg1=spatial_extent2(temp_double,(float*)NULL,0.5,1,lenbrain,1,se))){
    //    nvox[i1]=se->nvox;
        //for(j=0;j<nreg[i1];j++,jj++)regsize[jj]=se->regsize[j];
    //    }
    //nreg1=spatial_extent2(temp_double,(float*)NULL,0.5,1,lenbrain,1,se);
    spatial_extent2(temp_double,(float*)NULL,0.5,1,lenbrain,1,se);

    //cout<<files->files[i1]<<endl;
    //cout<<"    nreg["<<i1<<"]= "<<nreg[i1]<<" nvox["<<i1<<"]= "<<nvox[i1]<<endl;
    //cout<<"    regsize= ";for(jj-=nreg[i1],j=0;j<nreg[i1];j++,jj++)cout<<regsize[jj]<<" ";cout<<endl;

    strcpy(string,files->files[i1]);
    strptr=get_tail_sans_ext(string);
    strtok_r(strptr,"_",&strptr);
    //strptr++;
    saveptr=strtok_r(strptr,"_",&strptr);
    //cout<<"    saveptr= "<<saveptr<<endl;

    //for(sum=sum2=0.,min=lenbrain,max=j=0;j<nreg[i1];j++){
    for(sum=sum2=0.,min=lenbrain,max=j=0;j<se->nreg;j++){
        sum+=se->regsize[j];sum2+=se->regsize[j]*se->regsize[j];
        if(se->regsize[j]<min)min=se->regsize[j];
        if(se->regsize[j]>max)max=se->regsize[j];

        sum0+=se->regsize[j];sum20+=se->regsize[j]*se->regsize[j];
        if(se->regsize[j]<min0)min0=se->regsize[j];
        if(se->regsize[j]>max0)max0=se->regsize[j];
        }
    //nreg0+=nreg[i1];
    nreg0+=se->nreg;
    nvox0+=se->nvox;

    //mean=sum/(double)nreg[i1];
    //sd=nreg[i1]>1?sqrt((sum2-sum*sum/(double)nreg[i1])/(double)(nreg[i1]-1)):0.;
    mean=sum/(double)se->nreg;
    sd=se->nreg>1?sqrt((sum2-sum*sum/(double)se->nreg)/(double)(se->nreg-1)):0.;
    //cout<<saveptr<<"\t"<<nvox[i1]<<"\t"<<nreg[i1]<<"\t"<<mean<<"\t"<<sd<<"\t"<<min<<"\t"<<max<<"\t";
    cout<<saveptr<<"\t"<<se->nvox<<"\t"<<se->nreg<<"\t"<<mean<<"\t"<<sd<<"\t"<<min<<"\t"<<max<<"\t";
    //for(j=0;j<nreg[i1];j++)cout<<"\t"<<se->regsize[j];cout<<endl;
    for(j=0;j<se->nreg;j++)cout<<"\t"<<se->regsize[j];cout<<endl;
    }
mean=sum0/(double)nreg0;
sd=sqrt((sum2-sum*sum/(double)nreg0)/(double)(nreg0-1));
mean_infvol=(double)nvox0/(double)files->nfiles;
mean_infno=(double)nreg0/(double)files->nfiles;
cout<<"mean\t"<<mean_infvol<<"\t"<<mean_infno<<"\t"<<mean<<"\t"<<sd<<"\t"<<min0<<"\t"<<max0<<endl;
}
