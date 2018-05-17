/* Copyright 10/2/15 Washington University.  All Rights Reserved.
   get_atlas_param.h  $Revision: 1.5 $ */
#ifndef __GET_ATLAS_PARAM_H__
    #define __GET_ATLAS_PARAM_H__
    #include "ifh.h"
    typedef struct {
        char str[9];
        double mmppix[3],center[3];
        float voxel_size[3],mmppixf[3],centerf[3];
        int xdim,ydim,zdim,vol,area,c_orient[3];
        }Atlas_Param;

    //START170619
    #ifdef __cplusplus
        extern "C" {
    #endif

    Atlas_Param *get_atlas_param(char *atlas,Interfile_header *ifh,char *filename);
    void free_atlas_param(Atlas_Param *ap);

    //START170619
    #ifdef __cplusplus
        }//extern
    #endif

#endif
