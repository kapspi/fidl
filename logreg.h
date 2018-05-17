/* Copyright 2/6/18 Washington University.  All Rights Reserved.
   logreg.h  $Revision: 1.1 $ */
#ifndef __LOGREG_H__
    #define __LOGREG_H__

    #include "read_data.h"

    typedef struct {
        int *n0,*n1,*nindex,*index,*starti;
        double *y;
        } LRy;

    #ifdef __cplusplus
        extern "C" {
    #endif

    LRy *create_LRy(int num_regions,int ncov);
    int assign_LRy(Data *data,int *num_regions_by_file,int num_regions,LRy *lry);

    #ifdef __cplusplus
        }//extern
    #endif
#endif

