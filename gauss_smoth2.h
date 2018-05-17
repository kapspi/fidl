/* Copyright 3/23/15 Washington University.  All Rights Reserved.
   gauss_smoth2.h  $Revision: 1.4 $ */
typedef struct {
    char atlas[7];
    int xdim,ydim,zdim,slc,vol,lxy,lz,*i,ni;
    double *f,*scl_z,*tmp,*tmp1,*hxy,*hz,*image;
    } gauss_smoth2_struct;

#ifdef __cplusplus
    extern "C" {
#endif

gauss_smoth2_struct *gauss_smoth2_init(int xdim,int ydim,int zdim,double fwhmxy,double fwhmz);
void gauss_smoth2(double *image,double *g,gauss_smoth2_struct *gs);
void gauss_smoth2_free(gauss_smoth2_struct *gs);

#ifdef __cplusplus
    }//extern
#endif
