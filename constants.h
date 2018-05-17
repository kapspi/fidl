/* Copyright 7/14/17 Washington University.  All Rights Reserved.
   constants.h  $Revision: 1.1 $ */
#ifndef __CONSTANTS_H__
    #define __CONSTANTS_H__
    #define UNSAMPLED_VOXEL 1.e-37
    #define MAXNAME 2000
    #define T4SIZE 16

    #define MMPPIX_X_111 1.
    #define MMPPIX_Y_111 -1.
    #define MMPPIX_Z_111 -1.
    #define CENTER_X_111 89.
    #define CENTER_Y_111 -85.
    #define CENTER_Z_111 -101.
    #define XDIM_111 176
    #define YDIM_111 208
    #define ZDIM_111 176
    #define AREA_111 (XDIM_111*YDIM_111)
    #define VOL_111 (XDIM_111*YDIM_111*ZDIM_111)
    #define LENBRAIN_111 1726930

    #define MMPPIX_X_222 2.
    #define MMPPIX_Y_222 -2.
    #define MMPPIX_Z_222 -2.
    #define CENTER_X_222 129.
    #define CENTER_Y_222 -129.
    #define CENTER_Z_222 -82.
    #define XDIM_222 128
    #define YDIM_222 128
    #define ZDIM_222 75
    #define AREA_222 (XDIM_222*YDIM_222)
    #define VOL_222 (XDIM_222*YDIM_222*ZDIM_222)

    #define MMPPIX_X_333 3.
    #define MMPPIX_Y_333 -3.
    #define MMPPIX_Z_333 -3.
    #define CENTER_X_333 73.5
    #define CENTER_Y_333 -87.
    #define CENTER_Z_333 -84.
    #define XDIM_333 48
    #define YDIM_333 64
    #define ZDIM_333 48
    #define AREA_333 (XDIM_333*YDIM_333)
    #define VOL_333 (XDIM_333*YDIM_333*ZDIM_333)
    #define LENBRAIN_333 65549 /*119195*/ /*Nonzero voxels in glm_333_atlas_mask.*/

    #define MMPPIX_X_222MNI 2.
    #define MMPPIX_Y_222MNI -2.
    #define MMPPIX_Z_222MNI -2.
    #define CENTER_X_222MNI 91.
    #define CENTER_Y_222MNI -91.
    #define CENTER_Z_222MNI -109.
    #define XDIM_222MNI 91
    #define YDIM_222MNI 109
    #define ZDIM_222MNI 91
    #define AREA_222MNI (XDIM_222MNI*YDIM_222MNI)
    #define VOL_222MNI (XDIM_222MNI*YDIM_222MNI*ZDIM_222MNI)
    #define LENBRAIN_222MNI 292019

    #define MMPPIX_X_333MNI 0
    #define MMPPIX_Y_333MNI 0
    #define MMPPIX_Z_333MNI 0
    #define CENTER_X_333MNI 0
    #define CENTER_Y_333MNI 0
    #define CENTER_Z_333MNI 0
    #define XDIM_333MNI 60
    #define YDIM_333MNI 72
    #define ZDIM_333MNI 60
    #define AREA_333MNI (XDIM_333MNI*YDIM_333MNI)
    #define VOL_333MNI (XDIM_333MNI*YDIM_333MNI*ZDIM_333MNI)
    #define LENBRAIN_333MNI 0

    #define MMPPIX_X_111MNI 0
    #define MMPPIX_Y_111MNI 0
    #define MMPPIX_Z_111MNI 0
    #define CENTER_X_111MNI 0
    #define CENTER_Y_111MNI 0
    #define CENTER_Z_111MNI 0
    #define XDIM_111MNI 197
    #define YDIM_111MNI 233
    #define ZDIM_111MNI 189
    #define AREA_111MNI (XDIM_111MNI*YDIM_111MNI)
    #define VOL_111MNI (XDIM_111MNI*YDIM_111MNI*ZDIM_111MNI)
    #define LENBRAIN_111MNI 0

#endif
