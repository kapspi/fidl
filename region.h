/* Copyright 3/16/18 Washington University.  All Rights Reserved.
   region.h  $Revision: 1.1 $*/

#include <stddef.h>

#ifndef __REGION_H__
    #define __REGION_H__

    typedef struct {
        int     nregions,vol,*nvoxels_region,*harolds_num,**voxel_indices;
        size_t *length;
        char    **region_names;
        } Regions;

    #ifdef __cplusplus
        extern "C" {
    #endif

    Regions *get_reg(char *region_file,int fidl_aviseg_gt0,int vol,float *image,int nreg,int SunOS_Linux,char **ifh_region_names);
    Regions *get_reg_guts(int nregions,int lenvol,float *image,int fidl_aviseg_gt0,int *reg_val);
    void free_reg(Regions *reg);

    #ifdef __cplusplus
        }//extern
    #endif
#endif
