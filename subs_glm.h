/* Copyright 7/14/17 Washington University.  All Rights Reserved.
   subs_glm.h  $Revision: 1.1 $ */
#ifndef __SUBS_GLM_H__
    #define __SUBS_GLM_H__
    #define READ_ATAM1 2
    #define SMALL 3
    #include "constants.h"
    #include "ifh.h"
    typedef struct {
        float **AT,*fstat,*fzstat,*mask;
        double **ATAm1; //rev==-27 save only upper triangle as float,
        float *df,**ATAm1vox; //rev==-27 sd eliminated
        double *var;            /*rev<=-25, rev==-27 saved as float*/
        float *grand_mean;      /* Mean averaged over all BOLD files. */
        float *valid_frms;      /* valid_frms[i] > 0 if valid. */
        float  *c;              /* Contrasts */
        float  *cnorm;          /* Contrasts for Hotelling T**2*/
        char    **contrast_labels; /* Labels for each contrast. */
        size_t *lcontrast_labels; /*written and read as a short*/
        float   *Ysim;             /* Simulated, ideal data vector for one voxel. rev>=-22 */
        float   *stimlen;          /* Assumed duration of neuronal firing.      */
        float   *delay;            /* Assumed delay of neuronal firing.*/
        short   *lcfunc;           /* !TRUE: Encode function into design matrix.*/
        short   *effect_group;     /* Effect that an estimate belongs to. */
        int     start_data;        /* Byte offset at which computed data starts.*/
        char *cifti_xmldata;
        int *maski,nmaski;
        Interfile_header *ifh;     /* Pointer to interfile header. */
        } LinearModel;

    //START180412
    typedef struct {
        int     ncol,*col,ncoltrend,*coltrend;
        } Meancol;

    #ifdef __cplusplus
        extern "C" {
    #endif

    LinearModel *read_glm(char *glm_file,int dsgn_only,int SunOS_Linux);
    void free_glm(LinearModel *glm);
    long find_b(LinearModel *glm);

    //START180412
    Meancol *get_meancol(Interfile_header *glm_ifh,int print_version);

    #ifdef __cplusplus
        }//extern
    #endif

#endif
