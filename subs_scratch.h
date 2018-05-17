/* Copyright 4/23/15 Washington University.  All Rights Reserved.
   subs_scratch.h  $Revision: 1.2 $ */

//START170616
#include "dim_param2.h"
#include "get_atlas_param.h"
#include "subs_mask.h"
#include "read_frames_file.h"

typedef struct {
    int uns_count,small;
    } Scratch;
Files_Struct_new *create_scratchnames(Files_Struct *tc_files,Regions_By_File *fbf,char *scratchdir,char *appendstr);

#if 0
int create_scratchfiles(Files_Struct *tc_files,Regions_By_File *fbf,Dim_Param *dp,Files_Struct_new *sn,int SunOS_Linux,
    char *xform_file,Atlas_Param *ap,int num_wfiles,Regions_By_File *rbf,int num_regions,Mask_Struct *ms,
    Files_Struct *wfiles,int fs_nframes,float fwhm);
#endif
//START170616
int create_scratchfiles(Files_Struct *tc_files,Regions_By_File *fbf,Dim_Param2 *dp,Files_Struct_new *sn,int SunOS_Linux,
    char *xform_file,Atlas_Param *ap,int num_wfiles,Regions_By_File *rbf,int num_regions,Mask_Struct *ms,
    Files_Struct *wfiles,int fs_nframes,float fwhm);

void assignXnew(int num_tc_files,int *num_regions_by_file,Files_Struct_new *sn,int num_regions,int num_wfiles,int goose,
    int j,int p,int *sindex,FS *fs,double **X,Scratch *s);
void assignXgsl(int num_tc_files,int *num_regions_by_file,Files_Struct_new *sn,int num_regions,int num_wfiles,int goose,int j,
    int p,int *sindex,FS *fs,gsl_matrix *X,Scratch *s);
