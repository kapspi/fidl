/* Copyright 7/24/17 Washington University.  All Rights Reserved.
   find_regions_by_file_cover.h  $Revision: 1.1 $*/
#ifndef __FIND_REGIONS_BY_FILE_COVER_H__
    #define __FIND_REGIONS_BY_FILE_COVER_H__
 
    //#include "extract_regions.h"
    //START180316
    #include "region.h"

    typedef struct {
        int **roi_by_file,*num_regions_by_file,*nvoxels_region,*length,*indices,*harolds_num,max_length,nvoxels,nreg,*file_index,
            nvoxels_region_max,*roi_index,*indices_uniqsort,nindices_uniqsort,num_regions_by_file_max;
        char **region_names_ptr;
        } Regions_By_File;

    #ifdef __cplusplus
        extern "C" {
    #endif

    Regions_By_File *find_regions_by_file_cover(int num_region_files,int num_regions,Regions **reg,int *roi);
    Regions_By_File *find_regions_by_file(int num_region_files,int num_regions,int *nregions,int *roi);
    void free_regions_by_file(Regions_By_File *rbf);
    int crs(double *temp_double,double *region_stack,Regions_By_File *rbf,char *glmfile);
                                                                                            /*rbf->indices*/
    void crsw(double *temp_double,double *region_stack,Regions_By_File *rbf,double *weights,int *weightsi);

    #ifdef __cplusplus
        }//extern
    #endif

#endif
