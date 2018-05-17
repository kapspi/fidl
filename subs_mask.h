/* Copyright 6/05/15 Washington University.  All Rights Reserved.
   subs_mask.h  $Revision: 1.4 $ */
#ifndef __SUBS_MASK_H__
    #define __SUBS_MASK_H__
    #include "subs_glm.h"

    typedef struct {
        int xdim,ydim,zdim,vol,lenbrain,*brnidx,*maskidx,nuns,*unsi;
        float voxel_size_1,voxel_size_2,voxel_size_3,mmppix[3],center[3];
        } Mask_Struct;

    #ifdef __cplusplus
        extern "C" {
    #endif

    Mask_Struct *get_mask_struct(char *mask_file,int vol,int *indices,int SunOS_Linux,LinearModel *glm,int mslenvol);
    Mask_Struct *read_mask(char *mask_file,int SunOS_Linux,LinearModel *glm_in);
    void free_mask(Mask_Struct *ms);

    #ifdef __cplusplus
        }//extern
    #endif

#endif
