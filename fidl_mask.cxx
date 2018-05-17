/* Copyright 2/27/18 Washington University.  All Rights Reserved.
   fidl_mask.cxx  $Revision: 1.1 $ */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dim_param2.h"
#include "mask.h"
#include "stack.h"
#include "extract_regions.h"
#include "checkOS.h"
#include "fidl.h"
#include "write1.h"
#include "filetype.h"
#include "subs_cifti.h"
#include "spatial_extent2.h"
using namespace std;
int main(int argc,char **argv)
{
char *file=NULL,*maskf=NULL,*out=NULL,string[MAXNAME];
int i,j,SunOS_Linux,*idx,nidx,*regvox=NULL,nregvox,*regi=NULL,*brnidx,lenbrain,vol,extent=0,xdim,ydim,zdim,sign=0;
size_t *len=NULL;
float *temp_float,*and0,thresh=(float)UNSAMPLED_VOXEL,*actmask,*mskimg=NULL;
double *temp_double;
Dim_Param2 *dp;
Interfile_header *ifh=NULL;
Regions *reg=NULL;
W1 *w1;
XmlS *xmls;
int64_t dims[3];
mask ms;
stack fs,msk;
Spatial_Extent2 *se;
if(argc<5) {
    cout<<"    -file:   Image to be masked.\n";
    cout<<"    -mask:   Mask.\n";
    cout<<"    -out:    Output image name.\n";
    cout<<"    -thresh: Apply threshold in addition to mask.\n";

    //START180320
    cout<<"    -extent: Number of face connected voxels.\n";

    //START180420
    cout<<"    -sign    Sign from the mask is applied to the image.\n";


    exit(-1);
    }
for(i=1;i<argc;i++) {
    if(!strcmp(argv[i],"-file") && argc > i+1)
        file = argv[++i];
    if(!strcmp(argv[i],"-mask") && argc > i+1)
        maskf = argv[++i];
    if(!strcmp(argv[i],"-out") && argc > i+1)
        out = argv[++i];
    if(!strcmp(argv[i],"-thresh") && argc > i+1)
        thresh=strtof(argv[++i],NULL);

    //START180320
    if(!strcmp(argv[i],"-extent") && argc > i+1)
        extent=(int)strtol(argv[++i],NULL,10);

    if(!strcmp(argv[i],"-sign"))
        sign=1;

    }
if(!file) {
    printf("fidlError: Need to specify image to be masked -file\n");
    exit(-1);
    }
if(!maskf) {
    printf("fidlError: Need to specify mask -mask\n");
    exit(-1);
    }

//cout<<"thresh = "<<thresh<<" extent = "<<extent<<endl;
//START180420
cout<<"thresh = "<<thresh<<" extent = "<<extent<<" sign = "<<sign<<endl;


if((SunOS_Linux=checkOS())==-1) exit(-1);
if(!(dp=dim_param2(1,&file,SunOS_Linux)))exit(-1);
if(!(ms.read_mask(maskf,SunOS_Linux,(LinearModel*)NULL)))exit(-1);

//brnidx=ms.get_brnidx(lenbrain,vol);
brnidx=ms.get_brnidx(lenbrain,vol,xdim,ydim,zdim);

cout<<"lenbrain = "<<lenbrain<<endl;
if(vol!=dp->volall) {
    printf("fidlError: vol=%d dp->volall=%d  Must be equal.\n",vol,dp->volall); 
    exit(-1);
    }
if(!(and0=(float*)malloc(sizeof*and0*dp->volall))) {
    printf("fidlError: Unable to malloc and0\n");
    exit(-1);
    }
for(i=0;i<dp->volall;i++)and0[i]=0.;
if(!(idx=(int*)malloc(sizeof*idx*lenbrain))) {
    printf("fidlError: Unable to malloc idx\n");
    exit(-1);
    }
if(dp->filetypeall==(int)IMG){
    if(!(ifh=read_ifh(file,(Interfile_header*)NULL))) exit(-1);
    if(ifh->nregions) {
        if(!(reg=extract_regions(file,0,0,(float*)NULL,0,SunOS_Linux,(char**)NULL))) exit(-1);
        if(!(regvox=(int*)malloc(sizeof*regvox*reg->nregions))) {
            printf("fidlError: Unable to malloc regvox\n");
            exit(-1);
            }
        if(!(regi=(int*)malloc(sizeof*regi*reg->nregions))) {
            printf("fidlError: Unable to malloc regi\n");
            exit(-1);
            }
        if(!(len=(size_t*)malloc(sizeof*len*reg->nregions))) {
            printf("fidlError: Unable to malloc len\n");
            exit(-1);
            }
        }
    }
if(!(temp_float=fs.stack0(file,SunOS_Linux)))exit(-1);

//START180420
if(sign){if(!(mskimg=msk.stack0(maskf,SunOS_Linux)))exit(-1);}

#if 0
for(nidx=i=0;i<lenbrain;i++)if(fabsf(temp_float[brnidx[i]])>=thresh&&!isnan(temp_float[brnidx[i]])) 
    {and0[brnidx[i]]=temp_float[brnidx[i]];idx[nidx++]=brnidx[i];}
#endif
//START180320
if(!extent){

    #if 0
    for(nidx=i=0;i<lenbrain;i++)if(fabsf(temp_float[brnidx[i]])>=thresh&&!isnan(temp_float[brnidx[i]])) 
        {and0[brnidx[i]]=temp_float[brnidx[i]];idx[nidx++]=brnidx[i];}
    #endif
    //START180420
    if(!sign){
        for(nidx=i=0;i<lenbrain;i++)if(fabsf(temp_float[brnidx[i]])>=thresh&&!isnan(temp_float[brnidx[i]])) 
            {and0[brnidx[i]]=temp_float[brnidx[i]];idx[nidx++]=brnidx[i];}
        }
    else{
        //for(nidx=i=0;i<lenbrain;i++)if(fabsf(temp_float[brnidx[i]])>=thresh&&!isnan(temp_float[brnidx[i]])){
        for(nidx=i=0;i<lenbrain;i++)if(fabsf(temp_float[brnidx[i]])>(float)UNSAMPLED_VOXEL&&!isnan(temp_float[brnidx[i]])
            &&fabsf(temp_float[brnidx[i]])>=thresh){
            //if(fabsf(temp_float[brnidx[i]])>=thresh){

                //printf("thresh=%g temp_float[%d]=%g mskimg[%d]=%g\n",thresh,brnidx[i],temp_float[brnidx[i]],brnidx[i],mskimg[brnidx[i]]);

                and0[brnidx[i]]=mskimg[brnidx[i]]<0.?-temp_float[brnidx[i]]:temp_float[brnidx[i]];
                idx[nidx++]=brnidx[i];

                //printf("thresh=%g temp_float[%d]=%g mskimg[%d]=%g and0[%d]=%g\n",thresh,brnidx[i],temp_float[brnidx[i]],brnidx[i],mskimg[brnidx[i]],brnidx[i],and0[brnidx[i]]);
                }
            }
        //}


    }
else{
    if(!(temp_double=(double*)malloc(sizeof*temp_double*dp->volall))) {
        printf("fidlError: Unable to malloc temp_double\n");
        exit(-1);
        }
    for(i=0;i<dp->volall;i++)temp_double[i]=0.;
    for(i=0;i<lenbrain;i++)
        temp_double[brnidx[i]]=temp_float[brnidx[i]]==(float)UNSAMPLED_VOXEL?(double)UNSAMPLED_VOXEL:(double)temp_float[brnidx[i]];
    if(!(actmask=(float*)malloc(sizeof*actmask*dp->volall))) {
        printf("fidlError: Unable to malloc actmask\n");
        exit(-1);
        }
    if(!(se=spatial_extent2init(xdim,ydim,zdim,lenbrain,brnidx)))exit(-1);
    spatial_extent2(temp_double,actmask,(double)thresh,extent,lenbrain,1,se);
    for(nidx=i=0;i<lenbrain;i++)if(actmask[brnidx[i]]){and0[brnidx[i]]=temp_float[brnidx[i]];idx[nidx++]=brnidx[i];}
    }

//cout<<"nidx = "<<nidx<<endl;
cout<<"number of voxels nidx = "<<nidx<<endl;

if(dp->filetypeall==(int)IMG){
    if(ifh->nregions) {
        for(nregvox=0,i=2;i<=(int)ifh->global_max;i++) { 
            for(regi[nregvox]=i-2,regvox[nregvox]=0,j=0;j<nidx;j++) {
                if((int)and0[idx[j]]==i) {
                    regvox[nregvox]++;
                    and0[idx[j]]=(float)nregvox+2.;
                    }
                }
            if(regvox[nregvox])nregvox++;
            }
        free_ifhregnames(ifh);
        for(i=0;i<nregvox;i++) {
            sprintf(string,"%d %s %d",i,reg->region_names[regi[i]],regvox[i]),
            len[i] = strlen(string) + 1;
            }
        if(!(ifh->region_names=d2charvar(nregvox,len))) exit(-1);
        ifh->nregions = nregvox;
        for(i=0;i<nregvox;i++) sprintf(ifh->region_names[i],"%d %s %d",i,reg->region_names[regi[i]],regvox[i]);
        }
    }
if(!out) {
    if(!(get_tail_sans_ext(file))) exit(-1);
    sprintf(string,"%s_masked.4dfp.img",file);
    out=string;
    }
if(!(w1=write1_init()))exit(-1);
w1->filetype=dp->filetypeall;
if(dp->filetypeall==(int)IMG){
    w1->how_many=dp->volall;
    ifh->bigendian=SunOS_Linux?0:1;
    w1->ifh=ifh;
    }
else if(dp->filetypeall==(int)CIFTI||dp->filetypeall==(int)CIFTI_DTSERIES){
    if(!(xmls=cifti_getxml(file)))exit(-1);
    w1->xmlsize=xmls->size;;
    w1->cifti_xmldata=xmls->data;
    dims[0]=(int64_t)dp->xdim[0];dims[1]=(int64_t)dp->ydim[0];dims[2]=(int64_t)dp->zdim[0];
    w1->dims=dims;
    w1->tdim=(int64_t)1;
    }
else if(dp->filetypeall==(int)NIFTI){
    w1->file_hdr=file;
    }

//for(i=0;i<lenbrain;i++)and0[brnidx[i]]=1.;

w1->temp_float=and0;
if(!write1(out,w1))exit(-1);
printf("Output written to %s\n",out);fflush(stdout);
}
