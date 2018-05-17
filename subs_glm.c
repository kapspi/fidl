/* Copyright 11/10/05 Washington University.  All Rights Reserved.
   subs_glm.c  $Revision: 1.42 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "d2double.h"
#include "subs_util.h"
#include "shouldiswap.h"
#include "d2float.h"
#include "subs_glm.h"
LinearModel *read_glm(char *glm_file,int dsgn_only,int SunOS_Linux)
    /* dsgn_only = 1          implies that only design parameters should be read.
       dsgn_only = READ_ATAM1 implies that design parameters and ATAm1 should be read.
       dsgn_only = SMALL */
{
    char *strptr;
    short	lablen;

    //ptrdiff_t   len;
    //START161214
    int len;

    //START160908
    //size_t      i,nc,tot_eff,Mcol,tdim;

    int	        i,j,k,l,vol,rev,nF=0,bigendian=1,swapbytes;

    //START160908
    float *temp_float;

    LinearModel *glm;
    fpos_t      pos;
    FILE	*fp;
    if(!(glm=malloc(sizeof*glm))) {
        printf("Unable to malloc glm!\n");
        return NULL;
        } 
    glm->Ysim=(float*)NULL;
    glm->effect_group=(short*)NULL;
    glm->fstat=(float*)NULL;
    glm->fzstat=(float*)NULL;
    glm->AT=(float**)NULL;
    glm->ATAm1=(double**)NULL;
    glm->ATAm1vox=(float**)NULL;
    glm->df=(float*)NULL;
    glm->c=(float*)NULL;
    glm->cnorm=(float*)NULL;
    glm->valid_frms=(float*)NULL;
    glm->delay=(float*)NULL;
    glm->stimlen=(float*)NULL;
    glm->lcfunc=(short*)NULL;

    //START160908
    //glm->sd=(float*)NULL;

    glm->var=(double*)NULL;
    glm->grand_mean=(float*)NULL;
    glm->contrast_labels=(char**)NULL;
    glm->cifti_xmldata=(char*)NULL;

    //START160906
    glm->mask=(float*)NULL;glm->maski=(int*)NULL;glm->nmaski=0;


    if(!(fp=fopen_sub(glm_file,"r"))) return NULL; 
    if(!(glm->ifh=get_ifh(fp,(Interfile_header*)NULL))) return NULL; 

    //START160908
    #if 0
    nc = (size_t)glm->ifh->glm_nc;
    Mcol = (size_t)glm->ifh->glm_Mcol;
    tot_eff = (size_t)glm->ifh->glm_tot_eff;
    tdim = (size_t)glm->ifh->glm_tdim;
    #endif

    rev = glm->ifh->glm_rev;
    nF = glm->ifh->glm_nF;
    bigendian = glm->ifh->bigendian;
    vol = glm->ifh->glm_xdim*glm->ifh->glm_ydim*glm->ifh->glm_zdim;
    swapbytes = shouldiswap(SunOS_Linux,bigendian);
    if(glm->ifh->glm_cifti_xmlsize) {
        if(!(glm->cifti_xmldata=(char*)malloc(sizeof*glm->cifti_xmldata*glm->ifh->glm_cifti_xmlsize))) {
            printf("fidlError: Unable to malloc glm->ifh->glm_cifti_xmlsize\n");
            return NULL;
            }
        if(!(fread_sub(glm->cifti_xmldata,sizeof(char),(size_t)glm->ifh->glm_cifti_xmlsize,fp,swapbytes,glm_file))) {
            printf("fidlError: Could not read glm->cifti_xmldata from %s\n",glm_file);
            return NULL;
            }
        }
    if(glm->ifh->glm_nc) {

        #if 0
        if(!(glm->lcontrast_labels=malloc(sizeof*glm->lcontrast_labels*nc))) {
            printf("fidlError: Unable to malloc glm->lcontrast_labels\n");
            return NULL;
            }
        #endif
        //START160908
        if(!(glm->lcontrast_labels=malloc(sizeof*glm->lcontrast_labels*glm->ifh->glm_nc))) {
            printf("fidlError: Unable to malloc glm->lcontrast_labels\n");
            return NULL;
            }


        if(fgetpos(fp,&pos)) {
            printf("fidlError: fgetpos failed. Abort!\n");
            return NULL;
            }

        //for(len=i=0;i<nc;i++) {
        //START16908
        for(len=i=0;i<glm->ifh->glm_nc;i++) {

            if(!(fread_sub(&lablen,sizeof(short),1,fp,swapbytes,glm_file))) {
                printf("fidlError: Could not read lablen from %s\n",glm_file);
                return NULL;
                }
            if(fseek(fp,(long)lablen,(int)SEEK_CUR)){printf("fidlError: read_glm fseek %ld\n",(long)lablen);return NULL;}
            len += glm->lcontrast_labels[i] = (size_t)lablen+1;
            }

        #if 0
        if(!(glm->contrast_labels=malloc(sizeof*glm->contrast_labels*nc))) {
            printf("fidlError: Unable to malloc glm->contrast_labels\n");
            return NULL;
            }
        #endif
        //START160908
        if(!(glm->contrast_labels=malloc(sizeof*glm->contrast_labels*glm->ifh->glm_nc))) {
            printf("fidlError: Unable to malloc glm->contrast_labels\n");
            return NULL;
            }

        if(!(glm->contrast_labels[0]=malloc(sizeof*glm->contrast_labels[0]*len))) {
            printf("fidlError: Unable to malloc glm->contrast_labels[0]\n");
            return NULL;
            }
        if(fsetpos(fp,&pos)) {
            printf("Error: fsetpos failed. Abort!\n");
            return NULL;
            }

        //for(strptr=glm->contrast_labels[0],i=0;i<nc;i++) {
        //START16908
        for(strptr=glm->contrast_labels[0],i=0;i<glm->ifh->glm_nc;i++){

            if(fseek(fp,sizeof(short),(int)SEEK_CUR)){printf("fidlError: read_glm fseek %zd\n",sizeof(short));return NULL;}
            lablen = glm->lcontrast_labels[i] - 1;
            if(!(fread_sub(strptr,sizeof(char),(size_t)lablen,fp,swapbytes,glm_file))) {
                printf("Could not read glm->contrast_labels from %s\n",glm_file);
                return NULL;
                }
            glm->contrast_labels[i] = strptr;
            *(strptr+=lablen)=0;
            strptr++;
            }
        }

    #if 0
    len = Mcol*glm->ifh->glm_Nrow;
    if(!(glm->AT=malloc(sizeof*glm->AT*Mcol))) {
        printf("Unable to malloc glm->AT\n");
        return NULL;
        }
    #endif
    //START160908
    len=glm->ifh->glm_Mcol*glm->ifh->glm_Nrow;
    if(!(glm->AT=malloc(sizeof*glm->AT*glm->ifh->glm_Mcol))) {
        printf("Unable to malloc glm->AT\n");
        return NULL;
        }


    if(!(glm->AT[0]=malloc(sizeof*glm->AT[0]*len))) {
        printf("Unable to malloc glm->AT[0]\n");
        return NULL;
        }

    //for(i=1;i<Mcol;i++) glm->AT[i]=glm->AT[i-1]+glm->ifh->glm_Nrow;
    //START160908
    for(i=1;i<glm->ifh->glm_Mcol;i++)glm->AT[i]=glm->AT[i-1]+glm->ifh->glm_Nrow;

    if(!(fread_sub(glm->AT[0],sizeof(float),len,fp,swapbytes,glm_file))) {
        printf("Could not read glm->AT from %s\n",glm_file);
        return NULL;
        }
    if(glm->ifh->glm_nc) {

        //len = Mcol*nc;
        //START160908
        len = glm->ifh->glm_Mcol*glm->ifh->glm_nc;

        if(!(glm->c=malloc(sizeof*glm->c*len))) {
            printf("Unable to malloc glm->c\n");
            return NULL;
            }
        if(!(fread_sub(glm->c,sizeof(float),len,fp,swapbytes,glm_file))) {
            printf("Could not read glm->c from %s\n",glm_file);
            return NULL;
            }

        //len = tot_eff*nc;
        //START160908
        len=glm->ifh->glm_tot_eff*glm->ifh->glm_nc;

        if(!(glm->cnorm=malloc(sizeof*glm->cnorm*len))) {
            printf("Unable to malloc glm->cnorm\n");
            return NULL;
            }
        if(!(fread_sub(glm->cnorm,sizeof(float),len,fp,swapbytes,glm_file))) {
            printf("Could not read glm->cnorm from %s\n",glm_file);
            return NULL;
            }
        }

    #if 0
    if(!(glm->valid_frms=malloc(sizeof*glm->valid_frms*tdim))) {
        printf("Unable to malloc glm->valid_frms\n");
        return NULL;
        }
    if(!(fread_sub(glm->valid_frms,sizeof(float),tdim,fp,swapbytes,glm_file))) {
        printf("Could not read glm->valid_frms from %s\n",glm_file);
        return NULL; 
        }
    #endif
    //START160908
    if(!(glm->valid_frms=malloc(sizeof*glm->valid_frms*glm->ifh->glm_tdim))) {
        printf("Unable to malloc glm->valid_frms\n");
        return NULL;
        }
    if(!(fread_sub(glm->valid_frms,sizeof(float),glm->ifh->glm_tdim,fp,swapbytes,glm_file))) {
        printf("Could not read glm->valid_frms from %s\n",glm_file);
        return NULL;
        }



    #if 0
    if(rev >= -22) {
        len = tdim*tot_eff;
        if(!(glm->Ysim=malloc(sizeof*glm->Ysim*len))) {
            printf("Unable to malloc glm->Ysim\n");
            return NULL;
            }
        if(!(fread_sub(glm->Ysim,sizeof(float),len,fp,swapbytes,glm_file))) {
            printf("Could not read glm->Ysim from %s\n",glm_file);
            return NULL; 
            }
        }
    if(!glm->ifh->glm_compressed) {
        if(!(glm->delay=malloc(sizeof*glm->delay*tot_eff))) {
            printf("Unable to malloc glm->delay\n");
            return NULL;
            }
        if(!(fread_sub(glm->delay,sizeof(float),tot_eff,fp,swapbytes,glm_file))) {
            printf("Could not read glm->delay from %s\n",glm_file);
            return NULL;
            }
        if(!(glm->stimlen=malloc(sizeof*glm->stimlen*tot_eff))) {
            printf("Unable to malloc glm->stimlen\n");
            return NULL;
            }
        if(!(fread_sub(glm->stimlen,sizeof(float),tot_eff,fp,swapbytes,glm_file))) {
            printf("Could not read glm->stimlen from %s\n",glm_file);
            return NULL;
            }
        }
    if(rev >= -22) {
        if(!(glm->effect_group=malloc(sizeof*glm->effect_group*tot_eff))) {
            printf("Unable to malloc glm->effect_group\n");
            return NULL;
            }
        if(!(fread_sub(glm->effect_group,sizeof(short),tot_eff,fp,swapbytes,glm_file))) {
            printf("Could not read glm->effect_group from %s\n",glm_file);
            return NULL; 
            }
        }
    if(!glm->ifh->glm_compressed) {
        if(!(glm->lcfunc=malloc(sizeof*glm->lcfunc*tot_eff))) {
            printf("Unable to malloc glm->lcfunc\n");
            return NULL;
            }
        if(!(fread_sub(glm->lcfunc,sizeof(short),tot_eff,fp,swapbytes,glm_file))) {
            printf("Could not read glm->lcfunc from %s\n",glm_file);
            return NULL; 
            }
        }
    #endif
    //START160906
    if(rev>=-22){

        //len = tdim*tot_eff;
        //START160908
        len = glm->ifh->glm_tdim*glm->ifh->glm_tot_eff;

        if(!(glm->Ysim=malloc(sizeof*glm->Ysim*len))) {
            printf("Unable to malloc glm->Ysim\n");
            return NULL;
            }
        if(!(fread_sub(glm->Ysim,sizeof(float),len,fp,swapbytes,glm_file))) {
            printf("Could not read glm->Ysim from %s\n",glm_file);
            return NULL;
            }
        }

    #if 0
    if(!(glm->delay=malloc(sizeof*glm->delay*tot_eff))) {
        printf("Unable to malloc glm->delay\n");
        return NULL;
        }
    if(!(fread_sub(glm->delay,sizeof(float),tot_eff,fp,swapbytes,glm_file))) {
        printf("Could not read glm->delay from %s\n",glm_file);
        return NULL;
        }
    if(!(glm->stimlen=malloc(sizeof*glm->stimlen*tot_eff))) {
        printf("Unable to malloc glm->stimlen\n");
        return NULL;
        }
    if(!(fread_sub(glm->stimlen,sizeof(float),tot_eff,fp,swapbytes,glm_file))) {
        printf("Could not read glm->stimlen from %s\n",glm_file);
        return NULL;
        }
    if(rev>=-22){
        if(!(glm->effect_group=malloc(sizeof*glm->effect_group*tot_eff))) {
            printf("Unable to malloc glm->effect_group\n");
            return NULL;
            }
        if(!(fread_sub(glm->effect_group,sizeof(short),tot_eff,fp,swapbytes,glm_file))) {
            printf("Could not read glm->effect_group from %s\n",glm_file);
            return NULL;
            }
        }
    if(!(glm->lcfunc=malloc(sizeof*glm->lcfunc*tot_eff))) {
        printf("Unable to malloc glm->lcfunc\n");
        return NULL;
        }
    if(!(fread_sub(glm->lcfunc,sizeof(short),tot_eff,fp,swapbytes,glm_file))) {
        printf("Could not read glm->lcfunc from %s\n",glm_file);
        return NULL;
        }
    #endif
    //START160908
    if(!(glm->delay=malloc(sizeof*glm->delay*glm->ifh->glm_tot_eff))){
        printf("Unable to malloc glm->delay\n");
        return NULL;
        }
    if(!(fread_sub(glm->delay,sizeof(float),glm->ifh->glm_tot_eff,fp,swapbytes,glm_file))){
        printf("Could not read glm->delay from %s\n",glm_file);
        return NULL;
        }
    if(!(glm->stimlen=malloc(sizeof*glm->stimlen*glm->ifh->glm_tot_eff))){
        printf("Unable to malloc glm->stimlen\n");
        return NULL;
        }
    if(!(fread_sub(glm->stimlen,sizeof(float),glm->ifh->glm_tot_eff,fp,swapbytes,glm_file))){
        printf("Could not read glm->stimlen from %s\n",glm_file);
        return NULL;
        }
    if(rev>=-22){
        if(!(glm->effect_group=malloc(sizeof*glm->effect_group*glm->ifh->glm_tot_eff))){
            printf("Unable to malloc glm->effect_group\n");
            return NULL;
            }
        if(!(fread_sub(glm->effect_group,sizeof(short),glm->ifh->glm_tot_eff,fp,swapbytes,glm_file))){
            printf("Could not read glm->effect_group from %s\n",glm_file);
            return NULL;
            }
        }
    if(!(glm->lcfunc=malloc(sizeof*glm->lcfunc*glm->ifh->glm_tot_eff))){
        printf("Unable to malloc glm->lcfunc\n");
        return NULL;
        }
    if(!(fread_sub(glm->lcfunc,sizeof(short),glm->ifh->glm_tot_eff,fp,swapbytes,glm_file))){
        printf("Could not read glm->lcfunc from %s\n",glm_file);
        return NULL;
        }






    if(rev<-17){
        if(!(fread_sub(&glm->start_data,sizeof(glm->start_data),1,fp,swapbytes,glm_file))) {
            printf("fidlError: Could not read glm->start_data from %s\n",glm_file);
            return NULL;
            }
        if(dsgn_only==1) {
            fclose(fp);
            return glm;
            }
        if(fseek(fp,(long)glm->start_data,(int)SEEK_SET)){printf("fidlError: read_glm fseek %ld\n",(long)glm->start_data);return NULL;}
        if(glm->ifh->glm_masked){
            if(!(glm->mask=malloc(sizeof*glm->mask*vol))){
                printf("Unable to malloc glm->mask\n");
                return NULL;
                }
            if(!(fread_sub(glm->mask,sizeof*glm->mask,vol,fp,swapbytes,glm_file))) {
                printf("Could not read glm->mask from %s\n",glm_file);
                return NULL;
                }
            for(glm->nmaski=i=0;i<vol;i++)if(glm->mask[i])glm->nmaski++;
            }
        else{
            glm->nmaski=vol;
            }
        if(!(glm->maski=malloc(sizeof*glm->maski*glm->nmaski))){
            printf("Unable to malloc glm->maski\n");
            return NULL;
            }
        if(glm->ifh->glm_masked){
            for(glm->nmaski=i=0;i<vol;i++)if(glm->mask[i])glm->maski[glm->nmaski++]=i;
            vol=glm->nmaski;
            }
        else{
            for(glm->nmaski=i=0;i<vol;i++)glm->maski[glm->nmaski++]=i;
            }



        }

    //len = Mcol*Mcol;
    //START160908
    len=glm->ifh->glm_Mcol*glm->ifh->glm_Mcol;


    #if 0
    if(!glm->ifh->glm_compressed) {
        if(glm->ifh->glm_df!=(float)UNSAMPLED_VOXEL) {
            if(!(glm->ATAm1=d2double(Mcol,Mcol))) {
                printf("Unable to malloc glm->ATAm1\n");
                return NULL;
                }
            if(!(fread_sub(glm->ATAm1[0],sizeof(double),len,fp,swapbytes,glm_file))) {
                printf("Could not read glm->ATAm1 from %s\n",glm_file);
                return NULL;
                }
            }
        else {
            for(len=0,i=Mcol;i>=1;i--) len+=i;
            if(!(glm->ATAm1vox=d2float(len,vol))) {
                printf("Unable to malloc glm->ATAm1vox\n");
                return NULL;
                }
            if(!(fread_sub(glm->ATAm1vox[0],sizeof*glm->ATAm1vox[0],vol*len,fp,swapbytes,glm_file))) {
                printf("Could not read glm->ATAm1vox from %s\n",glm_file);
                return NULL;
                }
            if(!(glm->df=malloc(sizeof*glm->df*vol))) {
                printf("Unable to malloc glm->df\n");
                return NULL;
                }
            if(!(fread_sub(glm->df,sizeof*glm->df,vol,fp,swapbytes,glm_file))) {
                printf("Could not read glm->df from %s\n",glm_file);
                return NULL;
                }
            }
        }
    else {
        if(fseek(fp,sizeof(double)*len,(int)SEEK_CUR)) {
            printf("fidlError: occurred while seeking past glm->ATAm1 in %s.\n",glm_file);
            return NULL;
            }
        }
    #endif
    //START160906
    if(glm->ifh->glm_df!=(float)UNSAMPLED_VOXEL) {

        #if 0
        if(!(glm->ATAm1=d2double(Mcol,Mcol))) {
            printf("Unable to malloc glm->ATAm1\n");
            return NULL;
            }
        #endif
        //START160908
        if(!(glm->ATAm1=d2double(glm->ifh->glm_Mcol,glm->ifh->glm_Mcol)))return NULL;



        if(rev<=-27){

            //START160908
            for(len=0,i=glm->ifh->glm_Mcol;i>=1;i--)len+=i;

            if(!(temp_float=malloc(sizeof*temp_float*len))) {
                printf("Unable to malloc temp_float\n");
                return NULL;
                }
            if(!(fread_sub(temp_float,sizeof*temp_float,len,fp,swapbytes,glm_file))) {
                printf("Could not read glm->ATAm1 from %s\n",glm_file);
                return NULL;
                }

            //for(dptr=glm->ATAm1[0],i=0;i<len;i++)*dptr++=(double)temp_float[i];
            //START160908
            for(l=j=0;j<glm->ifh->glm_Mcol;j++)for(k=j;k<glm->ifh->glm_Mcol;k++,l++)
                glm->ATAm1[j][k]=glm->ATAm1[k][j]=(double)temp_float[l];

            free(temp_float);
            }
        else{
            if(!(fread_sub(glm->ATAm1[0],sizeof(double),len,fp,swapbytes,glm_file))) {
                printf("Could not read glm->ATAm1 from %s\n",glm_file);
                return NULL;
                }
            }
        }
    else {

        //for(len=0,i=Mcol;i>=1;i--) len+=i;
        //START160908
        for(len=0,i=glm->ifh->glm_Mcol;i>=1;i--)len+=i;

        //printf("subs_glm here0 glm->ATAm1vox=d2float(len,vol) len=%d vol=%d\n",len,vol);fflush(stdout);

        if(!(glm->ATAm1vox=d2float(len,vol))) {
            printf("Unable to malloc glm->ATAm1vox\n");
            return NULL;
            }
        if(!(fread_sub(glm->ATAm1vox[0],sizeof*glm->ATAm1vox[0],vol*len,fp,swapbytes,glm_file))) {
            printf("Could not read glm->ATAm1vox from %s\n",glm_file);
            return NULL;
            }
        if(!(glm->df=malloc(sizeof*glm->df*vol))) {
            printf("Unable to malloc glm->df\n");
            return NULL;
            }
        if(!(fread_sub(glm->df,sizeof*glm->df,vol,fp,swapbytes,glm_file))) {
            printf("Could not read glm->df from %s\n",glm_file);
            return NULL;
            }
        }


    if(dsgn_only == (int)READ_ATAM1) {
        fclose(fp);
        return(glm);
        }

    #if 0
    if(!glm->ifh->glm_compressed) {
        if(!(glm->sd=malloc(sizeof*glm->sd*vol))) {
            printf("Unable to malloc glm->sd\n");
            return NULL;
            }
        if(!(fread_sub(glm->sd,sizeof(float),vol,fp,swapbytes,glm_file))) {
            printf("Could not read glm->sd from %s\n",glm_file);
            return NULL;
            }
        if(!(glm->var=malloc(sizeof*glm->var*vol))) {
            printf("Unable to malloc glm->var\n");
            return NULL;
            }
        if(rev<=-25) {
            if(!(fread_sub(glm->var,sizeof(double),vol,fp,swapbytes,glm_file))) {
                printf("Could not read glm->var from %s\n",glm_file);
                return NULL;
                }
            }
        else {
            for(i=0;i<(size_t)vol;i++) glm->var[i] = (double)(glm->sd[i]*glm->sd[i]);
            }
        }
    #endif
    #if 0
    //START160906
    if(!(glm->sd=malloc(sizeof*glm->sd*vol))){
        printf("Unable to malloc glm->sd\n");
        return NULL;
        }
    if(!(glm->var=malloc(sizeof*glm->var*vol))){
        printf("Unable to malloc glm->var\n");
        return NULL;
        }
    if(rev>=-26){
        if(!(fread_sub(glm->sd,sizeof(float),vol,fp,swapbytes,glm_file))) {
            printf("Could not read glm->sd from %s\n",glm_file);
            return NULL;
            }
        }
    if(rev==-25||rev==-26) {
        if(!(fread_sub(glm->var,sizeof*glm->var,vol,fp,swapbytes,glm_file))) {
            printf("Could not read glm->var from %s\n",glm_file);
            return NULL;
            }
        }
    else if(rev<=-27){
        if(!(temp_float=malloc(sizeof*temp_float*vol))) {
            printf("Unable to malloc temp_float\n");
            return NULL;
            }
        if(!(fread_sub(temp_float,sizeof*temp_float,vol,fp,swapbytes,glm_file))) {
            printf("Could not read temp_float from %s\n",glm_file);
            return NULL;
            }
        for(i=0;i<vol;i++)glm->sd[i]=(float)sqrt((glm->var[i]=(double)temp_float[i]));
        free(temp_float);
        }
    else {
        for(i=0;i<(size_t)vol;i++)glm->var[i]=(double)glm->sd[i]*(double)glm->sd[i]);
        }
    #endif
    //START160908
    if(!(glm->var=malloc(sizeof*glm->var*vol))){
        printf("fidlError: Unable to malloc glm->var\n");
        return NULL;
        }
    if(rev==-25||rev==-26) {
        if(fseek(fp,sizeof(float)*vol,(int)SEEK_CUR)) {
            printf("fidlError: occurred while seeking past sd in %s.\n",glm_file);
            return NULL;
            }
        if(!(fread_sub(glm->var,sizeof*glm->var,vol,fp,swapbytes,glm_file))) {
            printf("fidlError: Could not read glm->var from %s\n",glm_file);
            return NULL;
            }
        }
    else{
        if(!(temp_float=malloc(sizeof*temp_float*vol))) {
            printf("Unable to malloc temp_float\n");
            return NULL;
            }
        if(!(fread_sub(temp_float,sizeof*temp_float,vol,fp,swapbytes,glm_file))) {
            printf("Could not read temp_float from %s\n",glm_file);
            return NULL;
            }
        if(rev==-27){for(i=0;i<vol;i++)glm->var[i]=(double)temp_float[i];}
        else{for(i=0;i<vol;i++)glm->var[i]=(double)temp_float[i]*(double)temp_float[i];}
        free(temp_float);
        }




    if(nF) {
        if(dsgn_only==(int)SMALL) {
            if(rev<=-26) {
                if(fseek(fp,sizeof(float)*nF*vol,(int)SEEK_CUR)) {
                    printf("Error: occurred while seeking past fstat in %s.\n",glm_file);
                    return NULL;
                    }
                }
            if(fseek(fp,sizeof(float)*nF*vol,(int)SEEK_CUR)) {
                printf("Error: occurred while seeking past fzstat in %s.\n",glm_file);
                return NULL;
                }
            }
        else {
            if(rev<=-26) {
                if(!(glm->fstat=malloc(sizeof*glm->fstat*nF*vol))) {
                    printf("Unable to malloc glm->fstat\n");
                    return NULL;
                    }
                if(!(fread_sub(glm->fstat,sizeof(float),nF*vol,fp,swapbytes,glm_file))) {
                    printf("Could not read glm->fstat from %s\n",glm_file);
                    return NULL;
                    }
                }
            if(!(glm->fzstat=malloc(sizeof*glm->fzstat*nF*vol))) {
                printf("Unable to malloc glm->fzstat\n");
                return NULL;
                }
            if(!(fread_sub(glm->fzstat,sizeof(float),nF*vol,fp,swapbytes,glm_file))) {
                printf("Could not read glm->fzstat from %s\n",glm_file);
                return NULL;
                }
            }
        }

    #if 0
    if(!glm->ifh->glm_compressed) {
        if(!(glm->grand_mean=malloc(sizeof*glm->grand_mean*vol))) {
            printf("Unable to malloc glm->grand_mean\n");
            return NULL;
            }
        if(!(fread_sub(glm->grand_mean,sizeof(float),vol,fp,swapbytes,glm_file))) {
            printf("Could not read glm->grand_mean from %s\n",glm_file);
            return NULL;
            }
        }
    #endif
    //START160906
    if(!(glm->grand_mean=malloc(sizeof*glm->grand_mean*vol))) {
        printf("Unable to malloc glm->grand_mean\n");
        return NULL;
        }
    if(!(fread_sub(glm->grand_mean,sizeof(float),vol,fp,swapbytes,glm_file))) {
        printf("Could not read glm->grand_mean from %s\n",glm_file);
        return NULL;
        }


    fclose(fp);
    return glm;
}

void free_glm(LinearModel *glm)
{
    free_ifh(glm->ifh,0);

    /*START150330*/
    if(glm->cifti_xmldata)free(glm->cifti_xmldata);

    if(glm->contrast_labels) {
        free(glm->lcontrast_labels);
        free(glm->contrast_labels[0]);
        free(glm->contrast_labels);
        }
    if(glm->AT){free(glm->AT[0]);free(glm->AT);}
    if(glm->c)free(glm->c);
    if(glm->cnorm)free(glm->cnorm);
    if(glm->valid_frms)free(glm->valid_frms);
    if(glm->Ysim) free(glm->Ysim);
    if(glm->delay)free(glm->delay);
    if(glm->stimlen)free(glm->stimlen);
    if(glm->effect_group) free(glm->effect_group);
    if(glm->lcfunc)free(glm->lcfunc);
    if(glm->ATAm1){free(glm->ATAm1[0]);free(glm->ATAm1);}
    if(glm->ATAm1vox){free(glm->ATAm1vox[0]);free(glm->ATAm1vox);}
    if(glm->df) free(glm->df);

    //START160908
    //if(glm->sd)free(glm->sd);

    if(glm->var)free(glm->var);
    if(glm->fzstat)free(glm->fzstat);
    if(glm->fstat) free(glm->fstat);
    if(glm->grand_mean)free(glm->grand_mean);
    free(glm);
}

#if 0
long find_b(LinearModel *glm) {
    size_t i,len,Mcol;
    int vol;
    long startb;
    startb = (long)(glm->start_data);
    START160906
    if(glm->ifh->glm_compressed) return startb;
    vol = glm->ifh->glm_xdim*glm->ifh->glm_ydim*glm->ifh->glm_zdim;
    startb += (long)(sizeof(float)*vol +                                    /*sd*/
                  sizeof(float)*vol*glm->ifh->glm_nF +                      /*fzstat*/
                  sizeof(float)*vol);                                       /*grand_mean*/
    if(glm->ifh->glm_df!=(float)UNSAMPLED_VOXEL) {
        startb += sizeof(double)*glm->ifh->glm_Mcol*glm->ifh->glm_Mcol;     /*ATAm1*/
        }
    else {
        Mcol = (size_t)glm->ifh->glm_Mcol;
        for(len=0,i=Mcol;i>=1;i--) len+=i;
        startb += sizeof(float)*vol*len;                                         /*ATAm1vox*/
        startb += sizeof(float)*vol;                                                 /*df*/
        }
    if(glm->ifh->glm_rev<=-25) startb += (long)(sizeof(double)*vol);                 /*var*/
    if(glm->ifh->glm_rev<=-26) startb += (long)(sizeof(float)*vol*glm->ifh->glm_nF); /*fstat*/
    return startb;
    }
#endif
//START160908
long find_b(LinearModel *glm) {

    #if 0
    size_t i,len,Mcol;
    int vol;
    #endif
    //START160919
    int vol,i,len;

    long startb;


    #if 0
    vol=!glm->ifh->glm_masked?glm->ifh->glm_xdim*glm->ifh->glm_ydim*glm->ifh->glm_zdim:glm->nmaski;
    startb=(long)(glm->start_data
        +sizeof(float)*vol*glm->ifh->glm_nF                                                     /*fzstat*/                 
        +sizeof(float)*vol);                                                                    /*grand_mean*/
    #endif
    //START160926
    startb=(long)(glm->start_data);
    if(glm->mask)startb+=sizeof(float)*glm->ifh->glm_xdim*glm->ifh->glm_ydim*glm->ifh->glm_zdim; /*mask*/
    vol=glm->nmaski; 
    startb+=sizeof(float)*vol*glm->ifh->glm_nF                                                  /*fzstat*/                 
        +sizeof(float)*vol;                                                                     /*grand_mean*/

    #if 0
    if(glm->ifh->glm_df!=(float)UNSAMPLED_VOXEL) {
        if(glm->ifh->glm_rev>=-26)startb+=sizeof(double)*glm->ifh->glm_Mcol*glm->ifh->glm_Mcol; /*ATAm1*/
        else startb+=sizeof(float)*glm->ifh->glm_Mcol*glm->ifh->glm_Mcol;                       /*ATAm1*/
        }
    else {
        Mcol = (size_t)glm->ifh->glm_Mcol;
        for(len=0,i=Mcol;i>=1;i--) len+=i;
        startb += sizeof(float)*vol*len;                                                        /*ATAm1vox*/
        startb += sizeof(float)*vol;                                                            /*df*/
        }
    #endif
    //START160920
    if(glm->ifh->glm_df!=(float)UNSAMPLED_VOXEL) {
        if(glm->ifh->glm_rev>=-26)startb+=sizeof(double)*glm->ifh->glm_Mcol*glm->ifh->glm_Mcol; /*ATAm1*/
        else{
            for(len=0,i=glm->ifh->glm_Mcol;i>=1;i--)len+=i;
            startb+=sizeof(float)*len;                                                          /*ATAm1*/
            }
        }
    else {
        for(len=0,i=glm->ifh->glm_Mcol;i>=1;i--)len+=i; 
        startb += sizeof(float)*vol*len;                                                        /*ATAm1vox*/
        startb += sizeof(float)*vol;                                                            /*df*/
        }


    if(glm->ifh->glm_rev>=-26)startb+=(long)(sizeof(float)*vol);                                /*sd*/
    if(glm->ifh->glm_rev==-25||glm->ifh->glm_rev==-26)startb+=(long)(sizeof(double)*vol);       /*var*/
    else if(glm->ifh->glm_rev==-27)startb+=(long)(sizeof(float)*vol);                           /*var*/
    if(glm->ifh->glm_rev<=-26)startb+=(long)(sizeof(float)*vol*glm->ifh->glm_nF);               /*fstat*/
    return startb;
    }

//START180412
Meancol *get_meancol(Interfile_header *glm_ifh,int print_version)
{
    int i,j,meancol_trend=-1,meancol_baseline=-1,eff=0,efftrend=0;
    Meancol *meancol;
    //if(print_version) print_version_number(rcsid,stdout);
    for(i=0;i<glm_ifh->glm_all_eff;i++) {
        if(print_version) printf("glm_ifh->glm_effect_label[%d]: %s\n",i,glm_ifh->glm_effect_label[i]);
        fflush(stdout);

        #if 0
        if(!strcmp("Trend",glm_ifh->glm_effect_label[i])) {
            meancol_trend = glm_ifh->glm_effect_column[i];
            efftrend = i;
            }
        else if(!strcmp("Baseline",glm_ifh->glm_effect_label[i])) {
            meancol_baseline = glm_ifh->glm_effect_column[i];
            eff=i;
            }
        #endif
        //START180412
        if(strstr(glm_ifh->glm_effect_label[i],"Trend")) {
            meancol_trend = glm_ifh->glm_effect_column[i];
            efftrend = i;
            }
        else if(strstr(glm_ifh->glm_effect_label[i],"Baseline")) {
            meancol_baseline = glm_ifh->glm_effect_column[i];
            eff=i;
            }



        }
    if(meancol_baseline==-1 && meancol_trend==-1) {
        printf("Warning: meancol_baseline = -1 and meancol_trend = -1\n");
        return NULL;
        }
    if(!(meancol=malloc(sizeof*meancol))) {
        printf("fidlError: Unable to malloc meancol\n");
        return NULL;
        }
    meancol->ncoltrend=0;
    if(meancol_baseline == -1) { /* old  baseline|trend (together) labeled as "Trend" */

        meancol->ncol = glm_ifh->glm_num_files;
        //START160107
        //meancol->ncol = glm_ifh->glm_effect_length[efftrend]/2;

        if(!(meancol->col=malloc(sizeof*meancol->col*meancol->ncol))) {
            printf("fidlError: Unable to malloc meancol->col\n");
            return NULL;
            }
        meancol->col[0] = meancol_trend;
        for(i=1;i<meancol->ncol;i++) meancol->col[i] = meancol->col[i-1] + 2;
        }
    else { /*trend (separate), baseline (separate) labeled separately as "Trend" and "Baseline"*/
        meancol->ncol = glm_ifh->glm_effect_length[eff];
        if(!(meancol->col=malloc(sizeof*meancol->col*meancol->ncol))) {
            printf("fidlError: Unable to malloc meancol->col\n");
            return NULL;
            }
        for(j=glm_ifh->glm_effect_column[eff],i=0;i<meancol->ncol;i++,j++) meancol->col[i] = j;
        if(meancol_trend!=-1) {
            meancol->ncoltrend = glm_ifh->glm_effect_length[efftrend];
            if(!(meancol->coltrend=malloc(sizeof*meancol->coltrend*meancol->ncoltrend))) {
                printf("fidlError: Unable to malloc meancol->coltrend\n");
                return NULL;
                }
            for(j=glm_ifh->glm_effect_column[efftrend],i=0;i<meancol->ncoltrend;i++,j++) meancol->coltrend[i] = j;
            }
        }
    if(print_version) {
        printf("grand_mean: %s columns =",meancol_baseline == -1 ? "Trend" : "Baseline");
        for(i=0;i<meancol->ncol;i++) fprintf(stdout," %d",meancol->col[i]);
        fprintf(stdout,"\n");
        fflush(stdout);
        }
    return meancol;
}
