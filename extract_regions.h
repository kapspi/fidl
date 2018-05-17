/* Copyright 7/24/17 Washington University.  All Rights Reserved.
   extract_regions.h  $Revision: 1.1 $*/
#ifndef __EXTRACT_REGIONS_H__
    #define __EXTRACT_REGIONS_H__

    typedef struct {
        int     nregions,vol,*nvoxels_region,*harolds_num,**voxel_indices;
        size_t *length;
        char    **region_names;
        } Regions;

    #ifdef __cplusplus
        extern "C" {
    #endif

    Regions *extract_regions(char *region_file,int fidl_aviseg_gt0,int vol,float *image,int nreg,int SunOS_Linux,
        char **ifh_region_names);
    Regions *extract_regions_guts(int nregions,int lenvol,float *image,int fidl_aviseg_gt0,int *reg_val);
    void free_regions(Regions *reg);

    #ifdef __cplusplus
        }//extern
    #endif

#endif
