/* Copyright 9/4/12 Washington University.  All Rights Reserved.
   fidl_copy_contrasts.c  $Revision: 1.3 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "constants.h"
#include "subs_glm.h"
#include "fidl.h"
#include "checkOS.h"
#include "d2float.h"
#include "shouldiswap.h"
#include "write_glm.h"
#include "subs_util.h"
#include "make_timestr2.h"
//static char rcsid[] = "$Header: /home/hannah/mcavoy/idl/clib/RCS/fidl_copy_contrasts.c,v 1.3 2012/09/18 23:13:40 mcavoy Exp $";
int main(int argc,char **argv)
{
char *cglmfile=NULL,*glmfile=NULL,**contrast_labels1,string[MAXNAME],timestr[23],filename[32];
int ncontrasts=0,*contrastsi=NULL,SunOS_Linux,nc1,ncclabi,*cclabi,nwhich,*which,missing1,missing2,swapbytes;
size_t *lcontrast_labels1;
long startb;
int i,j,k,l,i1,l1,l2;
float *fptr,tf,**cnorm1,**c1,*ccnorm,*cc,*temp_float;
LinearModel *cglm,*glm;
FILE *fp,*op;
//print_version_number(rcsid,stdout);
if(argc<5) {
    fprintf(stderr,"    -cglm:        Contrasts from this glm will be compied to the other.\n");
    fprintf(stderr,"    -contrasts:   Contrasts from -cglm to be copied to others. The first contrast is 1.\n");
    fprintf(stderr,"    -glm:         Other.\n");
    exit(-1);
    }
for(i=1;i<argc;i++) {
    if(!strcmp(argv[i],"-cglm") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        cglmfile = argv[++i];
    if(!strcmp(argv[i],"-glm") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        glmfile = argv[++i];
    if(!strcmp(argv[i],"-contrasts") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++ncontrasts;
        if(!(contrastsi=malloc(sizeof*contrastsi*ncontrasts))) {
            printf("fidlError: Unable to malloc contrasts\n");
            exit(-1);
            }
        for(j=0;j<ncontrasts;j++) contrastsi[j] = atoi(argv[++i])-1;
        }
    }
if(!cglmfile) {printf("fidlError: Need to specify -cglm\n");fflush(stdout);exit(-1);}
if(!glmfile) {printf("fidlError: Need to specify -glm\n");fflush(stdout);exit(-1);}
if(!ncontrasts) {printf("fidlError: Need to specify -contrasts\n");fflush(stdout);exit(-1);}
if((SunOS_Linux=checkOS())==-1) exit(-1);

if(!(cglm=read_glm(cglmfile,1,SunOS_Linux))) {
    printf("fidlError: reading %s  Abort!\n",cglmfile);
    exit(-1);
    }
if(!(glm=read_glm(glmfile,0,SunOS_Linux))) {
    printf("fidlError: reading %s  Abort!\n",glmfile);
    exit(-1);
    }

/*printf("glm->ifh->glm_rev=%d\n",glm->ifh->glm_rev);*/
/*printf("glm->ifh->glm_nc=%d\n",glm->ifh->glm_nc);
printf("glm->lcontrast_labels= ");for(i=0;i<glm->ifh->glm_nc;i++)printf("%d ",glm->lcontrast_labels[i]);printf("\n");
printf("glm->contrast_labels\n");for(i=0;i<glm->ifh->glm_nc;i++)printf("    %s\n",glm->contrast_labels[i]);printf("\n");*/


/*printf("cglm->contrast_labels[%d]=%s\n",contrastsi[0],cglm->contrast_labels[contrastsi[0]]);
for(cc=&cglm->c[contrastsi[0]*cglm->ifh->glm_Mcol],i=0;i<cglm->ifh->glm_Mcol;i++) printf("cc[%d]=%f ",i,cc[i]);printf("\n");
for(i=0;i<cglm->ifh->glm_Mcol;i++) printf("c[%d]=%f ",contrastsi[0]*cglm->ifh->glm_Mcol+i,cglm->c[contrastsi[0]*cglm->ifh->glm_Mcol+i]);printf("\n");*/


//START180419
startb = find_b(glm);
//printf("here0 glm->start_data=%d\n",glm->start_data);fflush(stdout);

if(!(cnorm1=d2float(glm->ifh->glm_nc+ncontrasts,glm->ifh->glm_tot_eff))) exit(-1);
for(fptr=cnorm1[0],i=0;i<glm->ifh->glm_nc*glm->ifh->glm_tot_eff;i++) *fptr++ = glm->cnorm[i];
if(!(c1=d2float(glm->ifh->glm_nc+ncontrasts,glm->ifh->glm_Mcol))) exit(-1);
for(fptr=c1[0],i=0;i<glm->ifh->glm_nc*glm->ifh->glm_Mcol;i++) *fptr++ = glm->c[i];
if(!(cclabi=malloc(sizeof*cclabi*ncontrasts))) {
    printf("fidlError: Unable to malloc cclabi\n");
    exit(-1);
    }
if(!(which=malloc(sizeof*which*glm->ifh->glm_Mcol))) {
    printf("Unable to malloc which\n");
    exit(-1);
    }
for(i1=glm->ifh->glm_nc,ncclabi=i=0;i<ncontrasts;i++) {
    ccnorm=&cglm->cnorm[contrastsi[i]*cglm->ifh->glm_tot_eff];
    cc=&cglm->c[contrastsi[i]*cglm->ifh->glm_Mcol];


    for(cclabi[ncclabi]=-1,nwhich=missing1=missing2=j=0;j<cglm->ifh->glm_tot_eff;j++) {
        if(fabsf(ccnorm[j])>.01) {
            for(k=0;k<glm->ifh->glm_tot_eff;k++) {
                if(!strcmp(cglm->ifh->glm_effect_label[j],glm->ifh->glm_effect_label[k])) {
                    cclabi[ncclabi]=contrastsi[i];
                    cnorm1[i1][k] = ccnorm[j];
                    for(l2=cglm->ifh->glm_effect_column[j],l1=glm->ifh->glm_effect_column[k],l=0;l<glm->ifh->glm_effect_length[k];
                        l++,l1++,l2++) {
                        c1[i1][l1] = cc[l2];

                        /*printf("cc[%d]=%f\n",l2,cc[l2]);*/

                        which[nwhich++]=l1;
                        }
                    break;
                    }
                }
            if(k==glm->ifh->glm_tot_eff) {
                if(cglm->ifh->glm_effect_length[j]==1) {
                    if(cc[j*cglm->ifh->glm_Mcol]<0.) missing1++;
                    }
                else {
                    for(tf=0.,l2=j*cglm->ifh->glm_Mcol,l=0;l<cglm->ifh->glm_effect_length[j];l++,l2++) tf+=cc[l2];
                    if(fabsf(tf)>.01) missing2++;
                    }
                }
            }
        }


    if(cclabi[ncclabi]>-1) {
        for(tf=0.,j=0;j<glm->ifh->glm_Mcol;j++) tf+=c1[i1][j];

        /*printf("c[%d][]= ",i1);for(j=0;j<glm->ifh->glm_Mcol;j++) printf("%f ",c1[i1][j]);printf("\n");
        printf("tf=%f\n",tf);fflush(stdout);*/

        if(fabsf(tf)>.01) {
            if(missing1||missing2) {
                printf("Don't know how to renormalize contrast %d, missing1=%d missing2=%d\n",contrastsi[i]+1,missing1,missing2);
                }
            else {
                /*for(k=0;k<nwhich;k++) c1[i1][which[k]]/=tf;*/
                for(j=0;j<nwhich;j++) c1[i1][which[j]]/=tf;
                }                
            }
        /*printf("after c[%d][]= ",i1);for(j=0;j<glm->ifh->glm_Mcol;j++)printf("%f ",c1[i1][j]);printf("\n");*/
        if(!missing1&&!missing2) {i1++;ncclabi++;}

        }
    }

nc1 = glm->ifh->glm_nc + ncclabi;
if(!(lcontrast_labels1=malloc(sizeof*lcontrast_labels1*nc1))) {
    printf("fidlError: Unable to malloc lcontrast_labels1\n");
    exit(-1);
    }

for(k=j=i=0;i<glm->ifh->glm_nc;i++,j++) k += lcontrast_labels1[j] = glm->lcontrast_labels[i];
for(i=0;i<ncclabi;i++,j++) k += lcontrast_labels1[j] = cglm->lcontrast_labels[contrastsi[i]];
/*for(j=i=0;i<glm->ifh->glm_nc;i++,j++) lcontrast_labels1[j] = (size_t)glm->lcontrast_labels[i];
for(i=0;i<ncclabi;i++,j++) lcontrast_labels1[j] = (size_t)cglm->lcontrast_labels[contrastsi[i]];*/

/*if(!(contrast_labels1=d2charvar(nc1,lcontrast_labels1))) exit(-1);*/
if(!(contrast_labels1=malloc(sizeof*contrast_labels1*nc1))) {
    printf("fidlError: Unable to malloc contrast_labels1\n");
    exit(-1);
    }
if(!(contrast_labels1[0]=malloc(sizeof*contrast_labels1[0]*k))) {
    printf("fidlError: Unable to malloc contrast_labels1[0]\n");
    exit(-1);
    }
for(i=1;i<nc1;i++) contrast_labels1[i]= contrast_labels1[i-1] + lcontrast_labels1[i-1];




for(j=i=0;i<glm->ifh->glm_nc;i++,j++) strcpy(contrast_labels1[j],glm->contrast_labels[i]);
for(i=0;i<ncclabi;i++,j++) strcpy(contrast_labels1[j],cglm->contrast_labels[contrastsi[i]]);
free(glm->lcontrast_labels);
free(glm->contrast_labels[0]);
free(glm->contrast_labels);
free(glm->c);
free(glm->cnorm);
glm->ifh->glm_nc = nc1;
/*printf("after\n");
printf("glm->ifh->glm_nc=%d\n",glm->ifh->glm_nc);*/

glm->lcontrast_labels = lcontrast_labels1;
/*for(i=0;i<glm->ifh->glm_nc;i++) glm->lcontrast_labels[i] = (int)lcontrast_labels1[i];*/

/*printf("glm->lcontrast_labels= ");for(i=0;i<glm->ifh->glm_nc;i++)printf("%d ",glm->lcontrast_labels[i]);printf("\n");*/

glm->contrast_labels = contrast_labels1;
/*printf("glm->contrast_labels\n");for(i=0;i<glm->ifh->glm_nc;i++)printf("    %s\n",glm->contrast_labels[i]);printf("\n");*/
glm->c = c1[0];
glm->cnorm = cnorm1[0];


swapbytes = shouldiswap(SunOS_Linux,glm->ifh->bigendian);



#if 0
vol = glm->ifh->glm_xdim*glm->ifh->glm_ydim*glm->ifh->glm_zdim;
if(!(temp_float=malloc(sizeof*temp_float*vol))) {
    printf("fidlErrror: Unable to malloc temp_float\n");
    exit(-1);
    }
if(!write_glm("tmp.glm",glm,(int)WRITE_GLM_THRU_FZSTAT,vol,swapbytes)) {
    printf("fidlError: writing WRITE_GLM_THRU_FZSTAT to %s\n","tmp.glm");
    exit(-1);
    }
if(!(op=fopen_sub("tmp.glm","a"))) exit(-1);
if(!(fwrite_sub(glm->grand_mean,sizeof(float),(size_t)vol,op,swapbytes))) {
    printf("fidlError: Could not write grand_mean to %s in linmod.\n","tmp.glm");
    exit(-1);
    }
if(!(fp=fopen_sub(glmfile,"r"))) exit(-1);
if(fseek(fp,startb,SEEK_SET)) {
    printf("fidlError: occured while seeking to %ld in %s.\n",startb,glmfile);
    exit(-1);
    }
for(i=0;i<glm->ifh->glm_Mcol;i++) {
    if(!fread_sub(temp_float,sizeof*temp_float,(size_t)vol,fp,swapbytes,glmfile)) {
        printf("fidlError: reading parameter estimates from %s.\n",glmfile);
        exit(-1);
        }
    if(!(fwrite_sub(temp_float,sizeof*temp_float,(size_t)vol,op,swapbytes))) {
        printf("fidlError: Could not write b to %s in linmod. eff=%d\n","tmp.glm",i);
        exit(-1);
        }
    }
fclose(fp);fclose(op);
#endif
//START180418
if(!(temp_float=malloc(sizeof*temp_float*glm->nmaski))) {
    printf("fidlErrror: Unable to malloc temp_float\n");
    exit(-1);
    }

sprintf(filename,"fidl%s.glm",make_timestr2(timestr));


#if 0
if(!write_glm("tmp.glm",glm,(int)WRITE_GLM_THRU_FZSTAT,swapbytes)){
    printf("fidlError: writing WRITE_GLM_THRU_FZSTAT to %s\n","tmp.glm");
    exit(-1);
    }
if(!write_glm_grand_mean("tmp.glm",glm,0,swapbytes))exit(-1);
#endif
//START180419
if(!write_glm(filename,glm,(int)WRITE_GLM_THRU_FZSTAT,swapbytes)){
    printf("fidlError: writing WRITE_GLM_THRU_FZSTAT to %s\n",filename);
    exit(-1);
    }
if(!write_glm_grand_mean(filename,glm,0,swapbytes))exit(-1);


if(!(fp=fopen_sub(glmfile,"r"))) exit(-1);
if(!(op=fopen_sub(filename,"a"))) exit(-1);
//printf("here0 glm->start_data=%d\n",glm->start_data);fflush(stdout);

if(fseek(fp,startb,SEEK_SET)) {
    printf("fidlError: occured while seeking to %ld in %s.\n",startb,glmfile);
    exit(-1);
    }
for(i=0;i<glm->ifh->glm_Mcol;i++) {
    if(!fread_sub(temp_float,sizeof*temp_float,(size_t)glm->nmaski,fp,swapbytes,glmfile)) {
        printf("fidlError: reading parameter estimates from %s.\n",glmfile);
        exit(-1);
        }
    if(!(fwrite_sub(temp_float,sizeof*temp_float,(size_t)glm->nmaski,op,swapbytes))) {

        //printf("fidlError: Could not write b to %s in linmod. eff=%d\n","tmp.glm",i);
        //START180419
        printf("fidlError: Could not write b to %s in linmod. eff=%d\n",filename,i);

        exit(-1);
        }
    }
fclose(fp);fclose(op);


//sprintf(string,"mv tmp.glm %s",glmfile);
//START180419
sprintf(string,"mv %s %s",filename,glmfile);

system(string);
printf("Output written to %s\n",glmfile);fflush(stdout);
}
