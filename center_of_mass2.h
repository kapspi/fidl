/* Copyright 3/26/14 Washington University.  All Rights Reserved.
   center_of_mass2.h  $Revision: 1.3 $ */
#ifndef __CENTER_OF_MASS2_H__
    #define __CENTER_OF_MASS2_H__

    #include "get_atlas_param.h"
    typedef struct {

        //START180223
        int *peaki;

        double *x,*y,*z,*px,*py,*pz,*coor,*peakcoor;
        float *peakval;
        } center_of_mass2struct;

    #ifdef __cplusplus
        extern "C" {
    #endif

    center_of_mass2struct *center_of_mass2init(int nvox,int nreg);
    void center_of_mass2free(center_of_mass2struct *com);
    void center_of_mass2(float *act,center_of_mass2struct *com,int nvox,int *indices,int nreg,int *nvoxreg,Atlas_Param *ap);

    #ifdef __cplusplus
        }//extern
    #endif
#endif
