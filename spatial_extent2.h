/* Copyright 3/23/15 Washington University.  All Rights Reserved.
   spatial_extent2.h  $Revision: 1.3 $*/

#ifndef __SPATIAL_EXTENT2_H__
    #define __SPATIAL_EXTENT2_H__

    #define N_NGHBRS_FACE 6
    #define N_NGHBRS_END_FACE 5
    typedef struct {
        int xdim,ydim,zdim,n_nghbrs_middle,n_nghbrs_end,plndim,vol,*offsets,*offsets_pln0,*offsets_plnN,*voxlist,*reglist,*regsize,
            nreg,nvox,nbrnidx,*brnidx;
        } Spatial_Extent2;

    #ifdef __cplusplus
        extern "C" {
    #endif

    Spatial_Extent2 *spatial_extent2init(int xdim,int ydim,int zdim,int nbrnidx,int *brnidx);
    void spatial_extent2free(Spatial_Extent2 *se);
    int spatial_extent2(double *crushed,float *actmask,double thresh,int n1,int n2,int p_z,Spatial_Extent2 *se);
                    // ******** crushed image includes extra voxels *********

    #ifdef __cplusplus
        }//extern
    #endif
#endif

