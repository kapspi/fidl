/* Copyright 3/16/18 Washington University.  All Rights Reserved.
   get_indbase.h  $Revision: 1.1 $*/
#ifndef __GET_INDBASE_H__
    #define __GET_INDBASE_H__


    #ifdef __cplusplus
        extern "C" {
    #endif

    int get_indbase(TC *tcs,Regions_By_File *fbf,LinearModel **glmstack,Meancol **meancol_stack,int *indbase_col,int subi);

    #ifdef __cplusplus
        }//extern
    #endif
#endif
