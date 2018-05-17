/* Copyright 12/31/99 Washington University.  All Rights Reserved.
   t4_atlas.c  $Revision: 12.108 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef __sun__
    #include <stdint.h>
#endif
#include <gsl/gsl_blas.h>

//START160908
#include "d2double.h"

#include "fidl.h"
#include "t4_atlas.h"
int _t4_atlas(int argc,char **argv)
{
    float *imgin,*t4_img,*t4,lpxy,lpz,*center_in;
    int i,xdim,ydim,zdim,twoAis0_twoBis1,orientation,atlas1,vol;
    double *dimg,*t4_dimg,*center=NULL;	
    char *atlas=NULL;
    Atlas_Param *ap;
    imgin = (float *)argv[0];
    t4_img = (float *)argv[1];
    t4 = (float *)argv[2];
    lpxy = *(float *)argv[6];
    lpz = *(float *)argv[7];
    #ifdef __sun__
        xdim = (int)argv[3];
        ydim = (int)argv[4];
        zdim = (int)argv[5];
        twoAis0_twoBis1 = (int)argv[8];
        orientation = (int)argv[9];
        atlas1 = (int)argv[10];
    #else
        xdim = (intptr_t)argv[3];
        ydim = (intptr_t)argv[4];
        zdim = (intptr_t)argv[5];
        twoAis0_twoBis1 = (intptr_t)argv[8];
        orientation = (intptr_t)argv[9];
        atlas1 = (intptr_t)argv[10];
    #endif
    if(argc>11) {
        center_in = (float *)argv[11];
        if(!(center=malloc(sizeof*center*3))) {
            printf("fidlError: Unable to malloc center\n");
            return 0;
            }
        for(i=0;i<3;i++) center[i] = (double)center_in[i];
        t4 = (float*)NULL;
        } 
    if(atlas1==111) atlas="111"; else if(atlas1==222) atlas="222"; else if(atlas1==333) atlas="333";

    //if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL))) return 0; 
    //START170616
    if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL,(char*)NULL))) return 0; 

    vol = xdim*ydim*zdim;
    if(!(dimg=malloc(sizeof*dimg*vol))) {
        printf("fidlError: Unable to malloc dimg\n");
        return 0;
        }
    for(i=0;i<vol;i++) dimg[i] = (double)imgin[i];
    if(!(t4_dimg=malloc(sizeof*t4_dimg*ap->vol))) {
        printf("fidlError: Unable to malloc t4_dimg\n");
        return 0;
        }

    if(!t4_atlas(dimg,t4_dimg,t4,xdim,ydim,zdim,lpxy,lpz,twoAis0_twoBis1,orientation,ap,center)) return 0;
    for(i=0;i<ap->vol;i++) t4_img[i] = (float)t4_dimg[i];
    if(center) free(center);
    free_atlas_param(ap);
    free(dimg);
    free(t4_dimg);
    return 1;
}
int t4_atlas(double *imgin,double *t4_img,float *t4_in,int xdim,int ydim,int zdim,float lpxy,float lpz,int twoAis0_twoBis1,
    int orientation,Atlas_Param *ap,double *center_in)
{
int	i,j,k,vox,ix,iy,iz,ki,ioff,koff,lenslc;

float	t4[16];

double 	wx,wy,wz,wx1,wy1,wz1,xk,yk,zk,xj,yj,zj,xt,yt,zt,dx[3],dy[3],dz[3],
	mmpixt[3],centert[3],mmpixo[3],centero[3],offx,offy,offz,x,y,z,
	**data_img,*tmpimg;

if(orientation != (int)TRANSVERSE) {
    fprintf(stderr,"Data must be acquired in the transverse plane for FIDL's atlas transformation to be correct.\n");
    fprintf(stderr,"Proper transformations for data aquired in the coronal and sagittal planes can be added.\n");
    fprintf(stderr,"Talk to McAvoy and he will do it.\n");
    return 0; 
    }
lenslc = xdim*ydim;
if(!(data_img = d2double(zdim,lenslc))) return 0;
if(!(tmpimg=malloc(sizeof*tmpimg*ap->vol))) {
    printf("fidlError: Unable to malloc tmpimg\n");
    return 0;
    }
for(i=0;i<ap->vol;i++) tmpimg[i] = 0.;
if(t4_in) {
    for(i=0;i<16;i++) t4[i] = t4_in[i];
    if(!twoAis0_twoBis1) to_711_2b(t4); 
    }
else {
    for(i=0;i<16;i++) t4[i] = 0.;
    t4[0]=t4[5]=t4[10]=t4[15]=1.;
    }
/*printf("t4\n"); for(k=i=0;i<4;i++) for(j=0;j<4;j++,k++) printf("%f ",t4[k]); printf("\n");
printf("lpxy=%f lpz=%f\n",lpxy,lpz);*/

/* Change voxel ordering convention. */
for(k=0,koff=(zdim-1)*lenslc;k<zdim;k++,koff-=lenslc) {
    for(j=0,ioff=0;j<ydim;j++,ioff+=xdim) {
        for(i=0;i<xdim;i++) { 
            data_img[k][i+ioff] = imgin[koff+ioff+xdim-1-i];
            }
        }
    }

mmpixt[0] =  lpxy;
mmpixt[1] = -lpxy;
mmpixt[2] = -lpz;

if(t4_in) {
    centert[0] = mmpixt[0]*(double)(xdim/2-1);
    centert[1] = mmpixt[1]*(double)(ydim/2);
    centert[2] = mmpixt[2]*(double)(zdim/2);
    }
else {
    if(!center_in) {
        printf("fidlError: center_in is not specified\n");
        return 0;
        }
    for(i=0;i<3;i++) centert[i] = center_in[i]; 
    }

mmpixo[0] = ap->mmppix[0];
mmpixo[1] = ap->mmppix[1];
mmpixo[2] = ap->mmppix[2];
centero[0] = ap->center[0];
centero[1] = ap->center[1];
centero[2] = ap->center[2];

offx = -t4[0]*centero[0] - t4[1]*centero[1] - t4[2]* centero[2] + t4[3];
offy = -t4[4]*centero[0] - t4[5]*centero[1] - t4[6]* centero[2] + t4[7];
offz = -t4[8]*centero[0] - t4[9]*centero[1] - t4[10]*centero[2] + t4[11];

dx[0] = mmpixo[0]*t4[0];
dx[1] = mmpixo[0]*t4[4];
dx[2] = mmpixo[0]*t4[8];
dy[0] = mmpixo[1]*t4[1];
dy[1] = mmpixo[1]*t4[5];
dy[2] = mmpixo[1]*t4[9];
dz[0] = mmpixo[2]*t4[2];
dz[1] = mmpixo[2]*t4[6];
dz[2] = mmpixo[2]*t4[10];

xk = offx; yk = offy; zk = offz;
xj = 0.; yj = 0.; zj = 0.;
x  = 0.; y  = 0.; z  = 0.;
for(k=0,vox=0;k<ap->zdim;k++) {
    xk += dz[0];
    yk += dz[1];
    zk += dz[2];
    xj = xk;
    yj = yk;
    zj = zk;
    for(j=0;j<ap->ydim;j++) {
        xj += dy[0];
        yj += dy[1];
        zj += dy[2];
        x = xj;
        y = yj;
        z = zj;
        for(i=0;i<ap->xdim;i++,vox++) {
            x += dx[0];
            y += dx[1];
            z += dx[2];
            xt = (x + centert[0])/mmpixt[0];
            yt = (y + centert[1])/mmpixt[1];
            zt = (z + centert[2])/mmpixt[2];
            ix = (int)(xt + xdim) - xdim;
            iy = (int)(yt + ydim) - ydim;
            iz = (int)(zt + zdim) - zdim;
            if((ix >= 0) && (ix < xdim-1)) {
                if((iy >= 0) && (iy < ydim-1)) {
                    if((iz >= 0) && (iz < zdim-1)) {
                        wx = xt - (double)ix;
                        wy = yt - (double)iy;
                        wz = zt - (double)iz;
                        wx1 = 1. - wx;
                        wy1 = 1. - wy;
                        wz1 = 1. - wz;
                        ki = ix + iy*xdim;


                        #if 0
                        tmpimg[vox] = 
				  wx1*wy1*wz1*data_img[iz  ][ki]
				+ wx1*wy1*wz *data_img[iz+1][ki]
				+ wx1*wy *wz1*data_img[iz  ][ki+xdim]
				+ wx1*wy *wz *data_img[iz+1][ki+xdim]
				+ wx *wy1*wz1*data_img[iz  ][ki+1]
				+ wx *wy1*wz *data_img[iz+1][ki+1]
				+ wx *wy *wz1*data_img[iz  ][ki+xdim+1]
				+ wx *wy *wz *data_img[iz+1][ki+xdim+1];
                        #endif

                        if(data_img[iz][ki]!=(double)UNSAMPLED_VOXEL && data_img[iz+1][ki]!=(double)UNSAMPLED_VOXEL &&
                           data_img[iz][ki+xdim]!=(double)UNSAMPLED_VOXEL && data_img[iz+1][ki+xdim]!=(double)UNSAMPLED_VOXEL &&
                           data_img[iz][ki+1]!=(double)UNSAMPLED_VOXEL && data_img[iz+1][ki+1]!=(double)UNSAMPLED_VOXEL &&
                           data_img[iz][ki+xdim+1]!=(double)UNSAMPLED_VOXEL && data_img[iz+1][ki+xdim+1]!=(double)UNSAMPLED_VOXEL) {
                            tmpimg[vox] =
                                  wx1*wy1*wz1*data_img[iz  ][ki]
                                + wx1*wy1*wz *data_img[iz+1][ki]
                                + wx1*wy *wz1*data_img[iz  ][ki+xdim]
                                + wx1*wy *wz *data_img[iz+1][ki+xdim]
                                + wx *wy1*wz1*data_img[iz  ][ki+1]
                                + wx *wy1*wz *data_img[iz+1][ki+1]
                                + wx *wy *wz1*data_img[iz  ][ki+xdim+1]
                                + wx *wy *wz *data_img[iz+1][ki+xdim+1];

                            #if 0
                            if(vox == 8893) {
                            /*if(data_img[iz  ][ki] > .2) {*/
                                /*printf("vox=%d\n",vox);*/
                                printf("wx1=%f wy1=%f wz1=%f wx=%f wy=%f wz=%f iz=%d ki=%d xdim=%d\n",
                                    wx1,wy1,wz1,wx,wy,wz,iz,ki,xdim);
                                printf("data_img=%f %f %f %f %f %f %f %f\n",data_img[iz  ][ki],data_img[iz+1][ki],
                                    data_img[iz  ][ki+xdim],data_img[iz+1][ki+xdim],data_img[iz  ][ki+1],data_img[iz+1][ki+1],
                                    data_img[iz  ][ki+xdim+1],data_img[iz+1][ki+xdim+1]);
                                /*exit(-1);*/
                                }
                            #endif


                            }
                        else { 
                            tmpimg[vox] = (double)UNSAMPLED_VOXEL;
                            }
                        }
                    else {
                        tmpimg[vox] = (double)UNSAMPLED_VOXEL;
                        }
                    }
                else {
                    tmpimg[vox] = (double)UNSAMPLED_VOXEL;
                    }
                }
            else {
                tmpimg[vox] = (double)UNSAMPLED_VOXEL;
                }
            }
        }
    }

/* Change voxel ordering convention. */
for(k=0,koff=(ap->zdim-1)*ap->area;k<ap->zdim;k++,koff-=ap->area) {
    for(j=0,ioff=0;j<ap->ydim;j++,ioff+=ap->xdim) {
        for(i=0;i<ap->xdim;i++) 
            t4_img[k*ap->area+i+ioff] = tmpimg[koff+ioff+ap->xdim-1-i];
        }
    }

#if 0
for(k=0,koff=(zdim-1)*lenslc;k<zdim;k++,koff-=lenslc) {
    for(j=0,ioff=0;j<ydim;j++,ioff+=xdim) {
        for(i=0;i<xdim;i++) if(data_img[k][i+ioff] != imgin[koff+ioff+xdim-1-i]) printf("here0\n");
        }
    }
#endif

free_d2double(data_img);
free(tmpimg);
return 1;
}


#if 0
void to_711_2b(float *t4)
{
    float b[16] = {1.051053, -0.002200,  0.018579,   -0.4981,
                   0.000148,  1.040993,  0.105308,    5.5848,
                  -0.019619, -0.108215,  1.004962,    0.9322,
                   0.000000,  0.000000,  0.000000,    1.0000};
    float t4b[16];
    int	i;
    matmul(t4,b,t4b,4);
    for(i=0;i<16;i++) t4[i] = t4b[i];
}
#endif
/*START140730*/
void to_711_2b(float *t4)
{
    float b[16] = {1.051053, -0.002200,  0.018579, -0.4981,
                   0.000148,  1.040993,  0.105308,  5.5848,
                  -0.019619, -0.108215,  1.004962,  0.9322,
                   0.000000,  0.000000,  0.000000,  1.0000},t4b[16];
    size_t i;
    gsl_matrix_float_view t4g = gsl_matrix_float_view_array(t4,4,4);
    gsl_matrix_float_view bg = gsl_matrix_float_view_array(b,4,4);
    gsl_matrix_float_view t4bg = gsl_matrix_float_view_array(t4b,4,4);
    gsl_blas_sgemm(CblasNoTrans,CblasNoTrans,1.,&t4g.matrix,&bg.matrix,0.,&t4bg.matrix);
    for(i=0;i<16;i++) t4[i] = t4b[i];
}
