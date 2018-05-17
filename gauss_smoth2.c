/* Copyright 12/31/99 Washington University.  All Rights Reserved.
   gauss_smoth2.c  $Revision: 1.8 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef __sun__
    #include <stdint.h>
#endif
#include "fidl.h"
#include "gauss_smoth2.h"
#include "get_atlas.h"
gauss_smoth2_struct *gauss_smoth2_init(int xdim,int ydim,int zdim,double fwhmxy,double fwhmz) {
    int kdim;
    size_t i;
    double sigmaxy,sigmaz,hm1,x,h0,sum;
    gauss_smoth2_struct *gs;
    if(!(gs=malloc(sizeof*gs))) {
        printf("fidlError: Unable to malloc gs\n");
        return NULL;
        }
    gs->xdim=xdim;gs->ydim=ydim;gs->zdim=zdim;gs->slc=xdim*ydim;gs->vol=xdim*ydim*zdim;
    if(!(gs->f=malloc(sizeof*gs->f*gs->vol))) {
        printf("fidlError: Unable to malloc gs->f\n");
        return NULL;
        }

    //START160928
    if(!(gs->image=malloc(sizeof*gs->image*gs->vol))) {
        printf("fidlError: Unable to malloc gs->image\n");
        return NULL;
        }
    if(!(gs->i=malloc(sizeof*gs->i*gs->vol))) {
        printf("fidlError: Unable to malloc gs->i\n");
        return NULL;
        }

    get_atlas(gs->vol,gs->atlas);
    sigmaxy = fwhmxy/2.35;
    sigmaz = fwhmz/2.35;
    if(!(gs->scl_z=malloc(sizeof*gs->scl_z*gs->zdim))) {
        printf("Error: Unable to malloc gs->scl_z\n");
        return NULL;
        }
    kdim = 2*(int)(3.*sigmaxy+.5) + 1;
    gs->lxy = kdim/2 + 1;
    if(!(gs->tmp=malloc(sizeof*gs->tmp*(gs->xdim+gs->ydim+2*gs->lxy)))) {
        printf("Error: Unable to malloc gs->tmp\n");
        return NULL;
        }
    if(!(gs->tmp1=malloc(sizeof*gs->tmp1*(gs->xdim+gs->ydim+2*gs->lxy)))) {
        printf("Error: Unable to malloc gs->tmp1\n");
        return NULL;
        }
    if(!(gs->hxy=malloc(sizeof*gs->hxy*(kdim==1?3:kdim)))) {
        printf("Error: Unable to malloc gs->hxy\n");
        return NULL;
        }
    gs->hxy[0] = 2.*(hm1=erf(0.5/sqrt(2.)/sigmaxy)); 
    for(x=1.5,sum=gs->hxy[0]/2.,i=1;i<(size_t)gs->lxy;i++) {
        gs->hxy[i] = (h0=erf(x/sqrt(2.)/sigmaxy)) - hm1;
        sum += gs->hxy[i];
        hm1 = h0;
        x = x + 1.;
        }
    for(i=0;i<(size_t)gs->lxy;i++) gs->hxy[i] /= 2.*sum;
    kdim = 2*(int)(3.*sigmaz+.5) + 1;
    gs->lz = kdim/2 + 1;
    if(!(gs->hz=malloc(sizeof*gs->hz*(kdim==1?3:kdim)))) {
        printf("Error: Unable to malloc gs->hz\n");
        return NULL;
        }
    gs->hz[0] = 2.*(hm1=erf(0.5/sqrt(2.)/sigmaz));
    for(x=1.5,sum=gs->hz[0]/2.,i=1;i<(size_t)gs->lz;i++) {
        gs->hz[i] = (h0=erf(x/sqrt(2.)/sigmaz)) - hm1;
        sum += gs->hz[i];
        hm1 = h0;
        x = x + 1.;
        }
    for(i=0;i<(size_t)gs->lz;i++) gs->hz[i] /= 2.*sum;
    return gs;
    }
void gauss_smoth2(double *image,double *g,gauss_smoth2_struct *gs) {
    int i,j,k,m,n,ioff,moff,koff;
    double sum;

    //for(i=0;i<gs->vol;i++) gs->f[i]=g[i]= 0.;
    //START160928
    for(gs->ni=i=0;i<gs->vol;i++){
        gs->f[i]=g[i]=0.;
        if(image[i]==(double)UNSAMPLED_VOXEL){gs->image[i]=0.;gs->i[gs->ni++]=i;}
        else{gs->image[i]=image[i];}
        }

    for(i=0;i<gs->xdim+gs->ydim+2*gs->lxy;i++) gs->tmp[i]=gs->tmp1[i]=0.;
    /* First convolve along all rows. */
    for(j=0,ioff=0;j<gs->ydim*gs->zdim;j++,ioff+=gs->xdim) {

        #if 0
        for(i=ioff,k=gs->lxy;i<ioff+gs->xdim;i++,k++) gs->tmp[k] = image[i];
        for(i=ioff,k=gs->xdim+2*gs->lxy-1;i<ioff+gs->lxy;i++,k--) gs->tmp[k] = image[i];
        for(i=ioff+gs->xdim-1,k=0;i>=ioff+gs->xdim-gs->lxy;i--,k++) gs->tmp[k] = image[i];
        #endif
        //START160928
        for(i=ioff,k=gs->lxy;i<ioff+gs->xdim;i++,k++) gs->tmp[k] = gs->image[i];
        for(i=ioff,k=gs->xdim+2*gs->lxy-1;i<ioff+gs->lxy;i++,k--) gs->tmp[k] = gs->image[i];
        for(i=ioff+gs->xdim-1,k=0;i>=ioff+gs->xdim-gs->lxy;i--,k++) gs->tmp[k] = gs->image[i];

        for(i=gs->lxy;i<gs->xdim+gs->lxy;i++) {
            gs->tmp1[i] = gs->hxy[0]*gs->tmp[i];
            for(m=1;m<gs->lxy;m++) gs->tmp1[i] += gs->hxy[m]*(gs->tmp[i+m] + gs->tmp[i-m]);
            }
        for(i=ioff,k=gs->lxy;i<ioff+gs->xdim;i++,k++) g[i] = gs->tmp1[k];
        }
    /* Now convolve along all columns. */
    for(n=0,koff=0;n<gs->zdim;n++,koff+=gs->slc) {
        for(ioff=koff;ioff<koff+gs->xdim;ioff++) {
            for(i=ioff,k=gs->lxy;i<ioff+gs->slc;i+=gs->xdim,k++) gs->tmp[k] = g[i];
            for(i=ioff,k=gs->ydim+2*gs->lxy-1;i<ioff+gs->xdim*gs->lxy;i+=gs->xdim,k--) gs->tmp[k] = g[i];
            if(!strcmp(gs->atlas,"333"))  {
                /* Don't wrap data in 333 space because brain is so close to the bottom.  Instead repeat upper rows.  
                   Result will be invalid here, but this is a noise region anyway.  */
                for(i=ioff,k=0;i>=ioff+gs->xdim*(gs->ydim-gs->lxy);i+=gs->xdim,k++) gs->tmp[k] = g[i];
                }
            else {
                for(i=ioff+gs->xdim*(gs->ydim-1),k=0;i>=ioff+gs->xdim*(gs->ydim-gs->lxy);i-=gs->xdim,k++) gs->tmp[k] = g[i];
                }
            for(i=gs->lxy;i<gs->ydim+gs->lxy;i++) {
                gs->tmp1[i] = gs->hxy[0]*gs->tmp[i];
                for(m=1;m<gs->lxy;m++) gs->tmp1[i] += gs->hxy[m]*(gs->tmp[i+m] + gs->tmp[i-m]);
                }
            for(i=ioff,k=gs->lxy;i<ioff+gs->slc;i+=gs->xdim,k++) gs->f[i] = gs->tmp1[k];
            }
        }
    /* Now convolve along slice axis. */
    for(k=0;k<gs->zdim;k++) {
        sum = gs->hz[0];
        if(k<gs->lz-1) {
            for(m=1;m<=k;m++) sum += 2*gs->hz[m];
            for(m=k+1;m<gs->lz;m++) sum += gs->hz[m];
            }
        else if(k>gs->zdim-gs->lz) {
            for(m=1,moff=0;m<gs->zdim-k-1;m++,moff+=gs->slc) sum += 2.*gs->hz[m];
            for(m=gs->zdim-k-1;m<gs->lz;m++) sum += gs->hz[m];
            }
        else {
            for(m=1,moff=0;m<gs->lz;m++,moff+=gs->slc) sum += 2.*gs->hz[m];
            }
        gs->scl_z[k] = 1./sum;
        }
    for(ioff=0;ioff<gs->slc;ioff++) {
        for(j=0,i=ioff;i<ioff+gs->slc*gs->zdim;i+=gs->slc,j++) {
            if(j<gs->lz-1) {
                g[i] = gs->hz[0]*gs->f[i];
                for(m=1,moff=gs->slc;m<=j;m++,moff+=gs->slc) g[i] += gs->hz[m]*(gs->f[i+moff] + gs->f[i-moff]);
                for(m=j+1;m<gs->lz;m++,moff+=gs->slc) g[i] += gs->hz[m]*gs->f[i+moff];
                }
            else if(j>gs->zdim-gs->lz) {
                g[i] = gs->hz[0]*gs->f[i];
                for(m=1,moff=gs->slc;m<gs->zdim-j-1;m++,moff+=gs->slc) g[i] += gs->hz[m]*(gs->f[i+moff] + gs->f[i-moff]);
                for(m=gs->zdim-j-1;m<gs->lz;m++,moff+=gs->slc) g[i] += gs->hz[m]*gs->f[i-moff];
                }
            else {
                g[i] = gs->hz[0]*gs->f[i];
                for(m=1,moff=gs->slc;m<gs->lz;m++,moff+=gs->slc) g[i] += gs->hz[m]*(gs->f[i+moff] + gs->f[i-moff]);
                }
            g[i] *= gs->scl_z[j];
            }
        }

    //START160928
    if(gs->ni)for(i=0;i<gs->ni;i++)g[gs->i[i]]=(double)UNSAMPLED_VOXEL;

    }
void gauss_smoth2_free(gauss_smoth2_struct *gs) {
    free(gs->hz);
    free(gs->hxy);
    free(gs->tmp1);
    free(gs->tmp);
    free(gs->scl_z);
    free(gs->f);
    free(gs);
    }
int _gauss_smoth2(int argc,char **argv) {
    gauss_smoth2_struct *gs;
    float *imgin,*imgout;
    double fwhmxy,fwhmz,*dimgin,*dimgout;
    size_t i;
    int xdim,ydim,zdim,vol;
    imgin = (float*)argv[0];
    imgout = (float*)argv[1];
    #ifdef __sun__
        xdim = (int)argv[2];
        ydim = (int)argv[3];
        zdim = (int)argv[4];
    #else
        xdim = (intptr_t)argv[2];
        ydim = (intptr_t)argv[3];
        zdim = (intptr_t)argv[4];
    #endif
    fwhmxy = (double)(*(float*)argv[5]);
    fwhmz = (double)(*(float*)argv[6]);
    vol = xdim*ydim*zdim;
    if(!(dimgin=malloc(sizeof*dimgin*vol))) {
        printf("Error: Unable to malloc dimgin\n");
        return 0;
        }
    if(!(dimgout=malloc(sizeof*dimgout*vol))) {
        printf("Error: Unable to malloc dimgout\n");
        return 0;
        }
    for(i=0;i<(size_t)vol;i++) dimgin[i] = (double)imgin[i];
    if(!(gs=gauss_smoth2_init(xdim,ydim,zdim,fwhmxy,fwhmz))) return 0;
    gauss_smoth2(dimgin,dimgout,gs);
    gauss_smoth2_free(gs);
    for(i=0;i<(size_t)vol;i++) imgout[i] = (float)dimgout[i];
    free(dimgout);
    free(dimgin);
    return 1;
    }
