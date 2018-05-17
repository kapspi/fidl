/* Copyright 12/31/99 Washington University.  All Rights Reserved.
   fidl.h  $Revision: 12.425 $ */
#ifndef __FIDL_H__
    #define __FIDL_H__
    #ifndef __STDC_FORMAT_MACROS
        #define __STDC_FORMAT_MACROS
        #include <inttypes.h>
    #endif
    #include <gsl/gsl_matrix.h>
    #include "ifh.h"
    #include "files_struct.h"
    #include "read_tc_string_new.h"
    #include "subs_glm.h"

    #ifdef __cplusplus
        extern "C" {
    #endif

    #define MAXNAME_CSH 255
    #define MAXREG 10000

    #define TRANSVERSE 2
    #define CORONAL 3
    #define SAGITTAL 4

    #define HRF_P 5
    #define HRF_DELTA 2
    #define HRF_TAU 1.25
    #define HRF_A 2.4125 /*1.93*HRF_TAU*/
    #define HRF_C 0.

    #define NEAREST_NEIGHBOR 1
    #define BILINEAR 2

    #define WRITE_GLM_DSGNONLY 6
    #define WRITE_GLM_THRU_FZSTAT 7
    #define WRITE_GLM_GRAND_MEAN  8 

    #define ERROR -1
    #define FALSE 0
    #define TRUE 1
    #define FWHM_SCL (sqrt(8.*log(2)))

    #define SPACE_DATA  1	
    #define SPACE_111   2  
    #define SPACE_222   3 
    #define SPACE_333   4

    /*#define GLM_FILE_REV -21*/     /* Grand mean should be correct. */
    /*#define GLM_FILE_REV -22*/
    /*#define GLM_FILE_REV -23*/     /*041014 eliminate Ysim*/
    /*#define GLM_FILE_REV -24*/     /*061205 eliminate extra bytes for contrasts from start_data*/
    /*#define GLM_FILE_REV -25*/         /*070115 var added*/ 
    //#define GLM_FILE_REV -26         /*100727 fstat added*/ 
    //#define GLM_FILE_REV -26         /*100727 fstat added*/ 
    #define GLM_FILE_REV -27         /*160906 glm_masked added, sd eliminated, var changed to float*/ 

    #define GLM_HDR_LEN_REV14 28 /* Length of header in short integers (2 bytes each). */
    #define GLM_HDR_LEN_REV15 30 /* Length of header in short integers (2 bytes each). */
    #define GLM_HDR_LEN_REV16 34 /* Length of header in short integers (2 bytes each). */

    /*#define GRAND_MEAN_THRESH 100*/
    #define GRAND_MEAN_THRESH 1

    #define BOYNTON 1
    #define FRISTON 2   ; Sum of 3 gamma function and their derivatives.
    #define BEHAVIORAL 3
    #define BOXCAR_STIM 4
    #define SPM_CANNONICAL 5

    #define GETMEM(A,B,type) if(!(A=(type *)malloc((B)*sizeof(type)))) { \
                                  fprintf(stdout,"Error allocating memory for " #A "\n"); \
                                  exit(-1); \
                                  }
    #define GETMEM_0(A,B,type) if(!(A=(type *)calloc((B),sizeof(type)))) { \
                                  fprintf(stdout,"Error allocating memory for " #A "\n"); \
                                  exit(-1); \
                                  }
    #define SHIFTX(A,B,C) if((A-1) < 0) \
                              B = -1; \
                          else \
                              B = A - 1;
    #define SHIFTY(A,B,C,D) if(A-D < 0) \
			        B = -1; \
			    else \
			        B = A - D;

    /*A: input index, B: output, C: lenvol, D: lenimg, E: lenimg*(int)fwhm)/2 */
    /***#define SHIFTZ(A,B,C,D,E) if((A-D > E) && (A-D < C - E)) \
	 		          B = A - D; \
			      else \
			          B = -1;***/
    #define SHIFTZ(A,B,C,D,E) if(A-D > 0) \
	 		          B = A - D; \
			      else \
			          B = -1;

    //START180316
    #if 0
    float	sum;
    #define PRINT_TOTAL(A) for(i=0,sum=0;i<lenvol;i++) \
			        sum += A [i]; \
			    printf("sum: %f\n",sum);
    #endif



    void f_to_z(double *f,double *zmap,int lenvol,double *df1,double *df2,int *index);
    void t_to_z(double *t,double *z,int lenvol,double *df,int *index);


    typedef struct {
        double delta,tau,A,C,duration;
        } Hrfparam;
    void boynton_model(int nt,double *t,Hrfparam *param,int lc_impulse,double *hrf);
    int boynton_contrast(int tot_eff,int *effect_length,float *effect_TR,float *effect_shift_TR,float dt,float init_delay,
        float *stimlen,float *cnorm,int Mcol,float *c);


    //START180412
    #if 0
    typedef struct {
        int     ncol,*col,ncoltrend,*coltrend;
        } Meancol;
    Meancol *get_meancol(Interfile_header *glm_ifh,int print_version);
    #endif

    typedef struct length_and_max {
        int *length;
        int max_length;
        } Length_And_Max;
    Length_And_Max *get_length_and_max(int nstrings,char **strings);
    void free_length_and_max(Length_And_Max *lam);


    typedef struct {
        char **tags_tc;
        char **tags_tc_effects;
        int *indices;
        } Tags_Tc_Struct;
    int get_tags_tc_guts(int i,TC *tcs,LinearModel *glm,TC *tc_weights,char *string,char *string3);
    Tags_Tc_Struct *get_tags_tc(TC *tcs,LinearModel *glm,TC *tc_weights);

    /*************************
    prototypes for utilities.c
    *************************/
    char *get_strings(char *line,char **strings,int how_many);

    int *strings_to_int(char *line,int *integer,int how_many);
    int **read_contrast_string(int num_contrasts,int num_glm_files,int argc_c,char **argv);
    int assign_region_names(Interfile_header *ifh,float *image,int lenvol,int nregions,double *coor,double *peakcoor,float *peakval);

    int print_version_number(char *rcsid,FILE *stream);

    int make_sphere(int x,int y,int z,float *temp_float,float value,int xdim,int ydim,int zdim,float diameter,float voxel_size,
        FILE *fp);
    int make_sphere_new(double x,double y,double z,float *temp_float,float value,int xdim,int ydim,int zdim,float diameter,
        float voxel_size,FILE *fp);

    void voxel_index_to_fidl_and_atlas(int voxel_index,int xdim,int ydim,int zdim,double *center,double *mmppix,
        int *atlas_coordinates,int *fidl_coordinates);
    void get_fidl_atlas_coord(int col,int row,int slice,int zdim,double *center,double *mmppix,int *fidl_coordinates,
        int *atlas_coordinates);

    int assign_glm_list_file_to_ifh(Interfile_header *ifh,char *glm_list_file);
    int assign_glm_list_file_to_ifh_FS(Interfile_header *ifh,Files_Struct *glm_list_file);

    //START180319
    //int check_dimensions(int nfiles,char **files_ptr,int lenvol);

    char **d2char(int dim1,int dim2);
    char **d2charvar(size_t dim1,size_t *dim2);

    int **d2int(int dim1,int dim2);

    size_t ***d3size_t(size_t dim1,size_t dim2,size_t dim3);
    int ***d3int(int dim1,int dim2,int dim3);
    float ***d3float(int dim1,int dim2,int dim3);
    double ***d3double(int dim1,int dim2,int dim3);

    //START180412
    //double ***d3doublevar(int dim1,int *dim2,int **dim3);

    void free_d2int(int **array);
    void free_d2float(float **array);
    
    void free_d3int(int ***array);
    void free_d3float(float ***array);
    void free_d3double(double ***array);
    void free_d2charvar(char **array);

    int count_strings(char *line, char *write_back,char wild_card);
    int count_strings_new(char *line,char *write_back,char wild_card,char *last_char);
    int count_strings_new_nocomma(char *line,char *write_back,char wild_card,char *last_char);
    int count_strings_new2(char *line,char *write_back,char wildcard1,char wildcard2,char *last_char,char *first_char,
        int no_first_char_to_write_back);
    int count_strings_new3(char *line,char *write_back,char wildcard1,char wildcard2,char *last_char,char *first_char,
        int no_first_char_to_write_back,char terminator);

    char *make_timestr(char *timestr);

    char *make_scratchdir(int noPWD,char *label);
    char *get_tail_sans_ext(char *write_back);
    char *get_dir(char *write_back);
    char *last_strstr(const char *haystack,const char *needle);
    char **get_region_files(int num_region_files,char **argv);
    void indexx(unsigned long n, double arr[], unsigned long indx[]);
    //int checkOS();
    void *d1(size_t size,size_t nitems);

    /*pca.c*/
    typedef struct {
        double **EIGVEC,*EIGVAL;
        int n;
        } PC;
    PC *allocate_PC(int nc);
    void free_pc(PC *pc);
    PC *pca(double *x,int nr,int nc);
    int eig(PC *pc);
    
    int ellipsestack(int N,int *x,int *y,int *z,float *stack,float *value,int xdim,int ydim,int zdim,float *xdia,float *ydia,
        float *zdia,float xvoxelsize,float yvoxelsize,float zvoxelsize);

    int maxminimgconc(char *filename,float *max,float *min);
    typedef struct {
        char *name;
        int nlines,nf,tlen,*len;
        double *data;
        } Dat;
    Dat *readdat(char *datfile);

    /*strutil.c*/
    char *getstr(char *line,char *str);
    char *grab_string_search(char *line,char *returnstr,char *searchstr,int *returnstrlen);
    char *get_line(char *line,char *returnstr,char *searchstr,int *returnstrlen);
    char *get_line_wnull(char *line,char *returnstr,char *searchstr,int *returnstrlen);
    char *get_line_wwosearchstr(char *line,char *returnstr,char *searchstr,int *returnstrlen);
    char *parse_line(char *line,char *returnstr,char *searchstr,int *returnstrlen);
    char *grab_string(char *line, char *string);
    float *strings_to_float(char *line,float *decimal,int how_many);
    double *strings_to_double(char *line,double *decimal,int how_many);
    char *grab_string_new(char *line,char *returnstr,int *returnstrlen);

    void orthogonalize(int m,int n,double *hipass,double *tau,double *Q,double *R);

    int autocorr_guts(char *infile,char *outfile,double *r,int vol,int tdim);    /* set to -1 for crosscorr*/
    int crosscorr_guts(char *infile,char *outfile,double *r,int vol,int tdim,int maxlik_unbias,double *instack);
    int crosscov_guts2(char *outfile,double *r,int vol1,double *in1,int vol2,double *in2,int tdim,int nlags,int maxlik_unbias);

    int crosscov_guts3(char *outfile,double *r,int vol1,double *in1,int vol2,double *in2,int tdim,int nlags,int maxlik_unbias,
        double *sd1,double *sd2);
    int cov_guts(char *infile,char *outfile,double *r,int vol,int tdim,int maxlik_unbias);
    int corr_guts(char *infile,char *outfile,double *r,double*den,int vol,int tdim);
    int var_guts(char *infile,char *outfile,double *r,int vol,int tdim,int maxlik_unbias);

    #if 0
    KEEP
    typedef struct {
        char *factor_labels;
        int nbehavcol,num_trials,num_levels,*lfactor_labels,*conditions,*frames,*frames_halfTR,*frames_quarterTR,*frames_floor,
            *offsets_halfTR,*offsets_quarterTR,*index_present,count_present;
        double TR,*times,*stimlen_vs_t,*behav_vs_t,max_stimlen_vs_t;
        } Sng;
    Sng *read_ev(char *event_file);
    #endif

    int delete_scratch_files(char **files,int nfiles);
    int gnu_cond(double *A,int m,int n,double tol,double *cond,int *ndepend);

    void r_to_z(double *r,double *t,double *z,double *df,int vol,int *idx); /*correlation df = n-2 */
    
    void count_estimates(float *AT,int n_interest,int t_valid,int *cnt);
    
    int sphere_nvox(int vol,float *temp_float,int xdim,int ydim,int zdim,float diameter,float voxel_size);

    float machine_epsilon(int print);
    double machine_epsilon_double(int print);

    void nrmlrv_doublestack(double *stack,int size,unsigned short *seed);
    int cifti_getDimensions(char *file,int64_t *rowSize,int64_t *colSize);

    #ifdef __cplusplus
        }//extern
    #endif
#endif
