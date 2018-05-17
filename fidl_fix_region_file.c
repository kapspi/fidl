/* Copyright 7/19/17 Washington University.  All Rights Reserved.
   fidl_fix_region_file.c  $Revision: 1.1 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "ifh.h"
#include "checkOS.h"
#include "subs_util.h"
#include "minmax.h"
int main(int argc,char **argv)
{
char *file=NULL,*out=NULL,*regname=NULL,string[MAXNAME],*strptr;
int i,SunOS_Linux,vol,cnt;
float *temp_float;
Interfile_header *ifh;
if(argc<3){
    fprintf(stderr,"Corrects files that have the first region defined with a value of 1.\n");
    fprintf(stderr,"    -file:    Region files to be fixed.\n");
    fprintf(stderr,"    -out:     Optional.\n");
    fprintf(stderr,"    -regname: Optional.\n");
    exit(-1);
    }
for(i=1;i<argc;i++){
    if(!strcmp(argv[i],"-file") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])file=argv[++i];
    if(!strcmp(argv[i],"-out") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])out=argv[++i];
    if(!strcmp(argv[i],"-regname") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])regname=argv[++i];
    }
if((SunOS_Linux=checkOS())==-1) exit(-1);
if(!(ifh=read_ifh(file,(Interfile_header*)NULL)))exit(-1);
vol=ifh->dim1*ifh->dim2*ifh->dim3;
if(!(temp_float=malloc(sizeof*temp_float*vol))) exit(-1);
if(!readstack(file,(float*)temp_float,sizeof*temp_float,(size_t)vol,SunOS_Linux,ifh->bigendian))exit(-1);
min_and_max(temp_float,vol,&ifh->global_min,&ifh->global_max);
if(ifh->global_max <= ifh->nregions || !ifh->nregions) {
    for(cnt=i=0;i<vol;i++)if(temp_float[i]>=1){++temp_float[i];cnt++;}
    min_and_max(temp_float,vol,&ifh->global_min,&ifh->global_max);
    if(!out)out=file;
    if(!writestack(out,temp_float,sizeof*temp_float,(size_t)vol,0))exit(-1);
    ifh->bigendian=SunOS_Linux?0:1;
    if(!ifh->nregions&&regname){
        ifh->nregions=1;
        sprintf(string,"0 %s %d",regname,cnt);
        strptr=string;
        ifh->region_names=&strptr;
        }
    if(!write_ifh(out,ifh,0))exit(-1);
    printf("Output written to %s\n",out);
    }
else {
    printf("Message: %s is ok.\n",out);
    }
}
