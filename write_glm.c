/* Copyright 12/31/99 Washington University.  All Rights Reserved.
   write_glm.c  $Revision: 12.132 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "fidl.h"
#include "write_glm.h"
#include "subs_util.h"

//int write_glm(char *glm_file,LinearModel *glm,int what_to_write,int vol,int swapbytes)
//START180418
int write_glm(char *glm_file,LinearModel *glm,int what_to_write,int swapbytes)

{
short	lablen;
ptrdiff_t len;
long    tl;

int i,j,k,l,vol;
float *temp_float;

FILE	*fp;
if(what_to_write==(int)WRITE_GLM_DSGNONLY || what_to_write==(int)WRITE_GLM_THRU_FZSTAT) {
    if(!(fp=fopen_sub(glm_file,"w"))) return 0; 
    glm->ifh->glm_rev = glm->fstat ? (int)GLM_FILE_REV : -25;
    put_ifh(fp,glm->ifh,1);
    fprintf(fp,"START_BINARY\n");
    if(glm->ifh->glm_cifti_xmlsize) {
        if(!fwrite_sub(glm->cifti_xmldata,sizeof*glm->cifti_xmldata,(size_t)glm->ifh->glm_cifti_xmlsize,fp,swapbytes)) {
            printf("Error: Could not write glm->cifti_xmldata to %s in write_glm.\n",glm_file);
            return 0;
            }
        }

    //for(i=0;i<(size_t)glm->ifh->glm_nc;i++) {
    //START160908
    for(i=0;i<glm->ifh->glm_nc;i++) {

        lablen = (short)glm->lcontrast_labels[i]-1;
        if(!fwrite_sub(&lablen,sizeof(short),1,fp,swapbytes)) {
            printf("Error: Could not write lablen to %s in write_glm.\n",glm_file);
            return 0;
            }
        if(!fwrite_sub(glm->contrast_labels[i],sizeof(char),(size_t)lablen,fp,swapbytes)) {
            printf("Error: Could not write glm->contrast_labels to %s in write_glm.\n",glm_file);
            return 0;
            }
        }
    if(!fwrite_sub(glm->AT[0],sizeof(float),(size_t)(glm->ifh->glm_Mcol*glm->ifh->glm_Nrow),fp,swapbytes)) {
        printf("Error: Could not write glm->AT to %s in write_glm.\n",glm_file);
        return 0;
        }
    if(glm->c) {
        if(!fwrite_sub(glm->c,sizeof*glm->c,(size_t)(glm->ifh->glm_Mcol*glm->ifh->glm_nc),fp,swapbytes)) {
            printf("Error: Could not write glm->c to %s in write_glm.\n",glm_file);
            return 0;
            }
        }
    if(glm->cnorm) {
        if(!fwrite_sub(glm->cnorm,sizeof*glm->cnorm,(size_t)(glm->ifh->glm_tot_eff*glm->ifh->glm_nc),fp,swapbytes)) {
            printf("Error: Could not write glm->cnorm to %s in write_glm.\n",glm_file);
            return 0;
            }
        }
    if(!fwrite_sub(glm->valid_frms,sizeof*glm->valid_frms,(size_t)glm->ifh->glm_tdim,fp,swapbytes)) {
        printf("Error: Could not write glm->valid_frms to %s in write_glm.\n",glm_file);
        return 0;
        }
    if(glm->delay) {
        if(!fwrite_sub(glm->delay,sizeof*glm->delay,(size_t)glm->ifh->glm_tot_eff,fp,swapbytes)) {
            printf("Error: Could not write glm->delay to %s in write_glm.\n",glm_file);
            return 0;
            }
        }
    if(glm->stimlen) {
        if(!fwrite_sub(glm->stimlen,sizeof*glm->stimlen,(size_t)glm->ifh->glm_tot_eff,fp,swapbytes)) {
            printf("Error: Could not write glm->stimlen to %s in write_glm.\n",glm_file);
            return 0;
            }
        }
    if(glm->lcfunc) {
        if(!fwrite_sub(glm->lcfunc,sizeof*glm->lcfunc,(size_t)glm->ifh->glm_tot_eff,fp,swapbytes)) {
            printf("Error: Could not write glm->lcfunc to %s in write_glm.\n",glm_file);
            return 0;
            }
        }
    if(what_to_write==(int)WRITE_GLM_DSGNONLY) return 1; 
    if((tl=ftell(fp))==-1) {
        printf("Error:0 tl=%ld  Something went wrong. Abort!\n",tl);
        return 0;
        }
    glm->start_data = (int)tl + sizeof(glm->start_data);
    if(!fwrite_sub(&glm->start_data,sizeof(glm->start_data),1,fp,swapbytes)) {
        printf("Error: Could not write glm->start_data to %s in write_glm.\n",glm_file);
        return 0;
        }
    if(fseek(fp,(long)glm->start_data,(int)SEEK_SET))return 0;

    //START180418
    vol=glm->ifh->glm_xdim*glm->ifh->glm_ydim*glm->ifh->glm_zdim;

    if(glm->mask){
        if(!fwrite_sub(glm->mask,sizeof*glm->mask,(size_t)vol,fp,swapbytes)) {
            printf("Error: Could not write glm->mask to %s in write_glm.\n",glm_file);
            return 0;
            }
        vol=glm->nmaski;
        }


    #if 0
    if(glm->ATAm1) {
        len = glm->ifh->glm_Mcol*glm->ifh->glm_Mcol;
        if(!fwrite_sub(glm->ATAm1[0],sizeof(double),(size_t)len,fp,swapbytes)) {
            printf("Error: Could not write glm->ATAm1 to %s in write_glm.\n",glm_file);
            return 0;
            }
        }
    #endif
    //START160908
    if(glm->ATAm1){
        for(len=0,i=glm->ifh->glm_Mcol;i>=1;i--)len+=i;
        if(!(temp_float=malloc(sizeof*temp_float*len))){
            printf("fidlError: Unable to malloc temp_float\n");
            return 0;
            }
        for(l=j=0;j<glm->ifh->glm_Mcol;j++)for(k=j;k<glm->ifh->glm_Mcol;k++,l++)temp_float[l]=(float)glm->ATAm1[j][k];
        if(!fwrite_sub(temp_float,sizeof*temp_float,(size_t)len,fp,swapbytes)) {
            printf("Error: Could not write glm->ATAm1 to %s in write_glm.\n",glm_file);
            return 0;
            }
        free(temp_float);
        }


    if(glm->ATAm1vox) {
        for(len=0,i=glm->ifh->glm_Mcol;i>=1;i--) len+=i;
        if(!fwrite_sub(glm->ATAm1vox[0],sizeof*glm->ATAm1vox[0],(size_t)(len*vol),fp,swapbytes)) {
            printf("Error: Could not write glm->ATAm1 to %s in write_glm.\n",glm_file);
            return 0;
            }
        }
    if(glm->df) {
        if(!fwrite_sub(glm->df,sizeof*glm->df,(size_t)vol,fp,swapbytes)) {
            printf("Error: Could not write glm->df to %s in write_glm.\n",glm_file);
            return 0;
            }
        }

    #if 0
    if(glm->sd) {
        if(!fwrite_sub(glm->sd,sizeof*glm->sd,(size_t)vol,fp,swapbytes)) {
            printf("Error: Could not write glm->sd to %s in write_glm.\n",glm_file);
            return 0;
            }
        }
    if(glm->var) {
        if(!fwrite_sub(glm->var,sizeof*glm->var,(size_t)vol,fp,swapbytes)) {
            printf("Error: Could not write glm->var to %s in write_glm.\n",glm_file);
            return 0;
            }
        }
    #endif
    //START160906
    if(glm->var){
        if(!(temp_float=malloc(sizeof*temp_float*vol))){
            printf("fidlError: Unable to malloc temp_float\n");
            return 0;
            }
        for(i=0;i<vol;i++)temp_float[i]=(float)glm->var[i];
        if(!fwrite_sub(temp_float,sizeof*temp_float,(size_t)vol,fp,swapbytes)) {
            printf("fidlError: Could not write glm->var to %s in write_glm.\n",glm_file);
            return 0;
            }
        free(temp_float);
        }



    if(glm->ifh->glm_nF) {
        if(glm->fstat) {
            if(!fwrite_sub(glm->fstat,sizeof(float),(size_t)(glm->ifh->glm_nF*vol),fp,swapbytes)) {
                printf("Error: Could not write fstat to %s in write_glm.\n",glm_file);
                return 0;
                }
            }
        if(!fwrite_sub(glm->fzstat,sizeof(float),(size_t)(glm->ifh->glm_nF*vol),fp,swapbytes)) {
            printf("Error: Could not write fzstat to %s in write_glm.\n",glm_file);
            return 0;
            }
        }
    fclose(fp);
    }
else{
    printf("fidlError: Invalid value of what_to_write=%d in write_glm\n",what_to_write);
    return 0;
    }
return 1;
}

#if 0
int write_glm_grand_mean(char *glm_file,LinearModel *glm,int vol,long seek_bytes,int swapbytes){
    FILE *fp;
    size_t i;
    int i1;
    for(i1=0,i=0;i<(size_t)vol;i++) if(glm->grand_mean[i] <= (float)GRAND_MEAN_THRESH) ++i1;
    if(!(fp=fopen_sub(glm_file,"r+")))return 0;
    if(fseek(fp,seek_bytes,(int)SEEK_END))return 0; 
    if(!(fwrite_sub(glm->grand_mean,sizeof(float),(size_t)vol,fp,swapbytes))){
        printf("fidlError: Could not write glm->grand_mean to %s in write_glm.\n",glm_file);
        return 0;
        }
    fclose(fp);
    return 1;
    }
#endif
//START180418
int write_glm_grand_mean(char *glm_file,LinearModel *glm,int lcseek,int swapbytes){
    FILE *fp;

    //if(!(fp=fopen_sub(glm_file,"r+")))return 0;
    //START180419
    if(!(fp=fopen_sub(glm_file,lcseek?"r+":"a")))return 0;

    if(lcseek){
        if(fseek(fp,(long)(-glm->nmaski*(glm->ifh->glm_Mcol+1)*sizeof(float)),(int)SEEK_END))return 0;
        }
    if(!(fwrite_sub(glm->grand_mean,sizeof(float),(size_t)glm->nmaski,fp,swapbytes))){
        printf("fidlError: Could not write glm->grand_mean to %s in write_glm.\n",glm_file);
        return 0;
        }
    fclose(fp);
    return 1;
    }

