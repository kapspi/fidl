/* Copyright 10/2/15 Washington University.  All Rights Reserved.
   t4_atlas.h  $Revision: 1.2 $ */
#include "get_atlas_param.h"

#ifdef __cplusplus
    extern "C" {
#endif

int t4_atlas(double *imgin,double *t4_img,float *t4_in,int xdim,int ydim,int zdim,float lpxy,float lpz,int twoAis0_twoBis1,
    int orientation,Atlas_Param *ap,double *center_in);
void to_711_2b(float *t4);

#ifdef __cplusplus
    }//extern
#endif
