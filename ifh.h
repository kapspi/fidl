/* Copyright 12/31/99 Washington University.  All Rights Reserved.
   ifh.h  $Revision: 12.129 $ */
#ifndef __IFH_H__
    #define __IFH_H__
    #include <stdio.h>
    #define BYTE_IF   1
    #define SHORT_IF  2
    #define FLOAT_IF  4
    #define DOUBLE_IF 8
    typedef struct{
        char   *image_modality;
        char   *orig_system;
        char   *prog_version;
        char   *prog_date;
        char   *orig_inst;
        char   *file_name;
        char   *patient_id;
        char   *study_date;
        int    number_format;	/* BYTE_IF, SHORT_IF, FLOAT_IF, DOUBLE_IF */
        int    bytes_per_pix;	/* Number of bytes per voxel. 		*/
        int    orientation;	
        int    time_series_flag;
        int	   ndim;		/* Number of dimensions.		*/
        int    dim1;		/* First dimension (x for transverse) 	*/
        int    dim2;		/* Second dimension (y for transverse) 	*/
        int    dim3;		/* Third dimension (z for transverse) 	*/
        int    dim4;		/* Time dimension. 			*/
        int    bigendian;
        float  voxel_size_1;	/* Voxel size in first dimension.	*/
        float  voxel_size_2;        /* Voxel size in second dimension.       */
        float  voxel_size_3;        /* Voxel size in third dimension.       */
        float  slice_thickness;
        char   *origin1;		/* right/left				*/
        char   *origin2;		/* posterior/anterior			*/
        char   *origin3;		/* inferior/superior			*/
        float  global_min;
        float  global_max;
        float  smoothness;		/* Smoothness parameter as used by SPM. */
        char   *mri_param_file;
        char   *mri_seq_file;
        char   *mri_seq_desc;
        char   *paradigm_format;
        float  mmppix[3];		/* (x,y,z) */
        float  center[3];		/* (x,y,z) */ 
        float  fwhm;
        char   **region_names;	/* regions of interest selected by fidl */
        char   **behavior_names;	/* Names of behavioral variables. */
        float  df1;
        float  df2;
        float  box_correction;      /* epsilon */
        char *datades;
        char *mask;
        unsigned short *nextseed;
        int    glm_rev;            	/* GLM revision number of file format. */
        int    glm_Mcol;            /* GLM total number of estimated parameters. */
        int    glm_M_interest;      /* GLM number of effects of interest in dsgn matrix. */ 
        int    glm_xdim;            /* GLM horizontal dimension of image. */
        int    glm_ydim;            /* GLM vertical dimension of image. */
        int    glm_zdim;            /* GLM depth dimension of image. */
        int    glm_tdim;            /* GLM time dimension of images. */
        int    glm_Nrow;            /* GLM Number of valid time points (Row in design matrix). */
        float  glm_df;              /* GLM degrees of freedom. */
        int    glm_nc;		/* GLM number of contrasts */
        int    glm_period;          /* GLM length of each HRF in frames. */
        int    glm_num_trials;      /* GLM number of trials. */
        float  glm_TR;              /* GLM time between frames in seconds. */
        int    glm_tot_eff;       /* GLM total number of effects of interest. (subset of glm_all_eff) */
        int    glm_all_eff;		/* Total number of effects. */
        char   **glm_effect_label;  /* Label for each effect. */
        int    *glm_leffect_label;
        int    *glm_effect_length;  /* Number of parameters for effect. */
        int    *glm_effect_column;  /* Starting column in design matrix for effect. */
        float  *glm_effect_TR;      /*TR of each effect. May vary depending on interleaving. */
        float  *glm_effect_shift_TR;/* Offset from interleaving. */
        int    glm_num_files;       /* GLM number of files. */
        int    *glm_funclen;        /* GLM number of elements in basis set.*/
        int    *glm_functype;       /* GLM basis function type (!BOYNTON or !FRISTON) */
        float  glm_fwhm;            /* units of voxels */
        float  glm_W;               /* GLM smoothness estimate. */
        float  glm_fwhmx;           /* Smoothness estimate in units of FWHM. */
        float  glm_fwhmy;           /* Smoothness estimate in units of FWHM. */
        float  glm_fwhmz;           /* Smoothness estimate in units of FWHM. */
        float  glm_dxdy;            /* GLM voxel size in xy plane in mm. */
        float  glm_dz;              /* GLM voxel size along z axis in mm. */
        int    glm_nF;              /* Number of F statistics. */
        char   **glm_F_names,**glm_F_names2;       
        char   *glm_event_file;     /* Event file used in construction of the glm. */
        char   *glm_mask_file;
        int glm_masked;
        int glm_nt4files;char **glm_t4files;
        unsigned int glm_cifti_xmlsize; 
        char   *glm_boldtype;
        int nregions,nbehav;
        int nregfiles;
        char **regfiles;
        }Interfile_header;

    //START170714
    #ifdef __cplusplus
        extern "C" {
    #endif

    Interfile_header *read_ifh(char *file_name,Interfile_header *ifh1);
    Interfile_header *get_ifh(FILE *fp,Interfile_header *ifh1);
    int write_ifh(char *file_name,Interfile_header *ifh,int glm_text_hdr);
    void put_ifh(FILE *fp,Interfile_header *ifh,int glm_text_hdr);
    Interfile_header *init_ifh(int ndim,int dim1,int dim2,int dim3,int dim4,float dx1,float dx2,float dx3,int bigendian,
        float *center,float *mmppix);
    void free_ifh(Interfile_header *ifh,int dontfreeifh);
    void free_ifhregnames(Interfile_header *ifh);
    Interfile_header *init_ifh0(Interfile_header *ifh1);

    //START170714
    #ifdef __cplusplus
        }//extern
    #endif

#endif
