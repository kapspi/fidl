/* Copyright 8/21/15 Washington University.  All Rights Reserved.
   filetype.c  $Revision: 1.3 $*/
#include <string.h>
#include <stdio.h>
#include "filetype.h"

//START170619
//Filetype[]={"","img","cifti","nifti","cifti"};
//Fileext[]={"",".4dfp.img",".dscalar.nii",".nii",".dtseries.nii"};
//START170706
#ifdef __cplusplus
    Filetype[]={"","img","cifti","nifti","cifti"};
    Fileext[]={"",".4dfp.img",".dscalar.nii",".nii",".dtseries.nii"};
#endif

int get_filetype1(char* glm_boldtype){
    for(int i=1;i<=3;++i)if(!strcmp(glm_boldtype,Filetype[i]))return i;
    printf("fidlError: get_filetype1 glm_boldtype=%s Unrecognized filetype.\n",glm_boldtype);fflush(stdout);return 0;
    } 
int get_filetype2(unsigned int xmlsize,char* glm_boldtype){
    return xmlsize<=0?(!strcmp(glm_boldtype,"nifti")?NIFTI:IMG):CIFTI;
    }

//START170713
int get_filetype(char *file){
    char *strptr;
    strptr=strrchr(file,'.');
    if(!strcmp(strptr,".img"))return IMG;

    //START170713
    else if(!strcmp(strptr,".glm"))return GLM;


    else if(!strcmp(strptr,".nii")){
        if(!strcmp(strptr-9,".dtseries.nii"))return CIFTI_DTSERIES;
        else if(!strcmp(strptr-8,".dscalar.nii"))return CIFTI;else{return NIFTI;}}
    else if(!strcmp(strptr-4,".nii.gz"))return NIFTI;
    else{printf("fidlError: get_filetype %s Unrecognized file type.\n",file);return 0;}
    }

