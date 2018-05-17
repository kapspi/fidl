/* Copyright 3/16/18 Washington University.  All Rights Reserved.
   tags_tc_glmstack.h  $Revision: 1.1 $*/

//START180412
#include "find_regions_by_file_cover.h"
#include "read_tc_string_new.h"
#include "subs_glm.h"

#ifndef __TAGS_TC_GLMSTACK_H__
    #define __TAGS_TC_GLMSTACK_H__ 

    typedef struct {
        char *tags_tc,**tags_tcptr,*tags_tc_effects,**tags_tc_effectsptr;
        } Tags_Tc_Struct2;

    #ifdef __cplusplus
        extern "C" {
    #endif

    int get_tags_tc_guts_glmstack(int i,int *ii,TC *tcs,LinearModel **glmstack,TC *tc_weights,char *string,char *string3,
        Regions_By_File *fbf,char **stringsptr,int *integers);
    Tags_Tc_Struct2 *get_tags_tc_glmstack(TC *tcs,int nglmfiles,LinearModel **glmstack,TC *tc_weights,Regions_By_File *fbf);

    #ifdef __cplusplus
        }//extern
    #endif
#endif
