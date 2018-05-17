/* Copyright 9/08/16 Washington University.  All Rights Reserved.
   d2double.h  $Revision: 1.2 $*/
#ifndef __D2DOUBLE_H__
    #define __D2DOUBLE_H__

    #ifdef __cplusplus
        extern "C" {
    #endif

    double **d2double(int dim1,int dim2);
    void free_d2double(double **array);

    #ifdef __cplusplus
        }//extern
    #endif
#endif
