/* Copyright 8/10/15 Washington University.  All Rights Reserved.
   filetype.h  $Revision: 1.5 $*/
#ifndef __FILETYPE_H__
    #define __FILETYPE_H__

    #define IMG 1
    #define CIFTI 2
    #define NIFTI 3
    #define CIFTI_DTSERIES 4
    #define GLM 5
    
    //static char *Filetype[]={"","img","cifti","nifti","cifti"};
    //static char *Fileext[]={"",".4dfp.img",".dscalar.nii",".nii",".dtseries.nii"};
    //START170619
    //extern char const* Filetype[];
    //extern char const* Fileext[];
    //START170706
    #ifndef __cplusplus
        static char *Filetype[]={"","img","cifti","nifti","cifti"};
        static char *Fileext[]={"",".4dfp.img",".dscalar.nii",".nii",".dtseries.nii"};
    #else
        extern char const* Filetype[];
        extern char const* Fileext[];
    #endif
    
    #ifdef __cplusplus
        extern "C" {
    #endif

    int get_filetype1(char* glm_boldtype);
    int get_filetype2(unsigned int xmlsize,char* glm_boldtype);
    int get_filetype(char *file);

    #ifdef __cplusplus
        }//extern
    #endif
#endif
