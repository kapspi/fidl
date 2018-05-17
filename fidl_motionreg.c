/* Copyright 9/30/14 Washington University.  All Rights Reserved.
   fidl_motionreg.c  $Revision: 1.11 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include "fidl.h"
#include "read_data.h"
#include "subs_util.h"
#include "cond_norm1.h"
#include "gsl_svd.h"
#include "d2double.h"
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
int main(int argc,char **argv)
{
char *outf=NULL,*label[]={"dx","dy","dz","X","Y","Z"},*label1[]={"R","R2","R'","R'2"},*strptr,filename[MAXNAME],dir[MAXNAME],
    string[MAXNAME],*tailptr,*datptr;
int ndat=0,*framesscrubbed,ndep,lcdeg_to_mm=0,lcR=0,lcR2=0,lcRd=0,lcRd2=0,R[4],i,j,k,l,i1,j1,j2,skip=0,numR;
size_t i0,k0;
double **y,mean,deg_to_mm,*fd,FD=0.,condmax=10000.,cond,cond_norm2,td,*ATA,*ATAm1,*temp_double,*V,*S,*work,
    convertfd[6],convertmm[6],*x;
Files_Struct *dat=NULL;
Data *data;
FILE *fp,*fp1;
if(argc<3) {
    fprintf(stderr,"  -dat: *.dat files or a conc listing the *.dat files from the see_movement script. One for each run.\n");
    fprintf(stderr,"        Or a conc listing the bold files.\n");
    fprintf(stderr,"  -out: Output name. File is in fidl 'external regressor' format. Use .ext as the extension.\n");
    fprintf(stderr,"        Each run generates 24 movement regressors [R R^2 R' R'^2] where R = [X Y Z pitch yaw roll]\n");
    fprintf(stderr,"        For example, three dat files would generate an output file with 72 columns.\n");
    fprintf(stderr,"        The scrubbed file takes the output name (sans extension), appending _scrub.txt .\n");
    fprintf(stderr,"  -FD:  Threshold for framewise displacement. Frames with values greater than this threshold are scrubbed.\n");
    fprintf(stderr,"  -skip:     Number of initial frames to skip. These frames are set to zero. This is important because \n");
    fprintf(stderr,"             regressors are normalized to be zero mean, so the mean is not computed over the skipped frames.\n");
    fprintf(stderr,"  -deg_to_mm Pitch, yaw and roll are always converted to mm for FD calculation. Setting this option puts\n");
    fprintf(stderr,"             these regressors in mm.\n");
    fprintf(stderr,"IF NONE OF THESE OPTIONS ARE SET, THEN ALL 24 MOTION REGRESSORS WILL BE MADE.\n");
    fprintf(stderr,"  -R:        [X Y Z pitch yaw roll]\n");
    fprintf(stderr,"  -R2:       [X^2 Y^2 Z^2 pitch^2 yaw^2 roll^2]\n");
    fprintf(stderr,"  -Rd:       [X' Y' Z' pitch' yaw' roll']\n");
    fprintf(stderr,"  -Rd2:      [X'^2 Y'^2 Z'^2 pitch'^2 yaw'^2 roll'^2]\n");
    exit(-1);
    }
for(i=1;i<argc;i++) {
    if(!strcmp(argv[i],"-dat") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++ndat;
        strptr = strrchr(argv[i+1],'.');
        if(!strcmp(strptr,".conc")) {
            if(!(dat=read_conc(ndat,&argv[i+1]))) exit(-1);
            }
        else if(!strcmp(strptr,".dat")){
            if(!(dat=get_files(ndat,&argv[i+1]))) exit(-1);
            }
        else {
            printf("Error: -dat not conc or dat. Abort!\n");fflush(stdout);
            exit(-1);
            }
        i+=ndat;
        }
    if(!strcmp(argv[i],"-out") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1]) outf = argv[++i];
    if(!strcmp(argv[i],"-FD") && argc > i+1) FD = atof(argv[++i]);
    if(!strcmp(argv[i],"-skip") && argc > i+1) skip = (size_t)atoi(argv[++i]);
    if(!strcmp(argv[i],"-deg_to_mm")) lcdeg_to_mm = 1;
    if(!strcmp(argv[i],"-R")) lcR = 1;
    if(!strcmp(argv[i],"-R2")) lcR2 = 1;
    if(!strcmp(argv[i],"-Rd")) lcRd = 1;
    if(!strcmp(argv[i],"-Rd2")) lcRd2 = 1;
    }
if(!ndat) {printf("fidlError: Need to specify -dat\n");fflush(stdout);exit(-1);}
if(!outf) {printf("fidlError: Need to specify -out\n");fflush(stdout);exit(-1);}
if(!lcR&&!lcR2&&!lcRd&&!lcRd2)lcR=lcR2=lcRd=lcRd2=1;
numR=(lcR+lcR2+lcRd+lcRd2)*6;
if(lcR)R[0]=1;if(lcR2)R[1]=1;if(lcRd)R[2]=1;if(lcRd2)R[3]=1;
printf("FD=%f  skip=%d lcdeg_to_mm=%d lcR=%d lcR2=%d lcRd=%d lcRd2=%d numR=%d\n",FD,skip,lcdeg_to_mm,lcR,lcR2,lcRd,lcRd2,numR);
if(!(framesscrubbed=malloc(sizeof*framesscrubbed*dat->nfiles))) {
    printf("fidlError: Unable to malloc framesscrubbed\n");
    exit(-1);
    }
for(i0=0;i0<dat->nfiles;i0++) framesscrubbed[i]=0;
gsl_permutation *perm = gsl_permutation_alloc(numR);
if(!(ATA=malloc(sizeof*ATA*numR*numR))) {
    printf("fidlError: Unable to malloc ATA\n");
    exit(-1);
    }
if(!(ATAm1=malloc(sizeof*ATAm1*numR*numR))) {
    printf("fidlError: Unable to malloc ATAm1\n");
    exit(-1);
    }
if(!(temp_double=malloc(sizeof*temp_double*numR*numR))) {
    printf("fidlError: Unable to malloc temp_double\n");
    exit(-1);
    }
if(!(V=malloc(sizeof*V*numR*numR))) {
    printf("fidlError: Unable to malloc V\n");
    exit(-1);
    }
if(!(S=malloc(sizeof*S*numR))) {
    printf("fidlError: Unable to malloc S\n");
    exit(-1);
    }
if(!(work=malloc(sizeof*work*numR))) {
    printf("fidlError: Unable to malloc work\n");
    exit(-1);
    }
gsl_matrix_view gATA = gsl_matrix_view_array(ATA,numR,numR);
deg_to_mm = (double)M_PI/180.*50.;
convertfd[0]=1.;convertfd[1]=1.;convertfd[2]=1.;
convertfd[3]=lcdeg_to_mm?1.:deg_to_mm;convertfd[4]=lcdeg_to_mm?1.:deg_to_mm;convertfd[5]=lcdeg_to_mm?1.:deg_to_mm;
convertmm[0]=1.;convertmm[1]=1.;convertmm[2]=1.;
convertmm[3]=lcdeg_to_mm?deg_to_mm:1.;convertmm[4]=lcdeg_to_mm?deg_to_mm:1.;convertmm[5]=lcdeg_to_mm?deg_to_mm:1.;
strcpy(filename,outf);
if(!(strptr=get_tail_sans_ext(filename))) exit(-1);
strcat(filename,"_scrub.txt");
if(!(fp1=fopen_sub(filename,"w"))) exit(-1);
if(!(fp=fopen_sub(outf,"w"))) exit(-1);
for(i1=1,i0=0;i0<dat->nfiles;i0++,i1++)for(j=0;j<6;j++)for(k=0;k<4;k++)
    if(R[k])fprintf(fp,"%s%s_%lu\t",label[j],label1[k],(unsigned long)i1); 
fprintf(fp,"\n");
for(i0=0;i0<dat->nfiles;i0++) {
    if(!strcmp(dat->files[i0]+dat->strlen_files[i0]-4,"img")) { 
        strcpy(dir,dat->files[i0]);
        if(!(datptr=get_dir(dir))) exit(-1);
        strcat(datptr,"movement/");
        strcpy(string,dat->files[i0]);
        tailptr=get_tail_sans_ext(string);
        strcat(datptr,tailptr);
        strcat(datptr,".dat");
        }
    else {
        datptr = dat->files[i0];
        }
    printf("datptr= %s\n",datptr); 

    //if(!(data=read_data(datptr,0,0,7,0))) exit(-1);
    //START180426
    if(!(data=read_data(datptr,0,0,7,0))){
        sprintf(string,"rm -rf %s",filename);
        printf("%s\n",string);
        if(system(string) == -1)printf("fidlError: unable to %s\n",string);
        sprintf(string,"rm -rf %s",outf);
        printf("%s\n",string);
        if(system(string) == -1)printf("fidlError: unable to %s\n",string);
        exit(-1);
        }

    if(!(x=malloc(sizeof*x*data->nsubjects))) {
        printf("fidlError: Unable to malloc x\n");
        exit(-1);
        }
    for(j=0;j<data->nsubjects;j++) x[j]=0.;
    if(!(y=d2double(data->nsubjects,numR))) exit(-1);
    if(!(fd=malloc(sizeof*fd*data->nsubjects))) {
        printf("fidlError: Unable to malloc fd\n");
        exit(-1);
        }
    for(j=0;j<data->nsubjects;j++) fd[j]=0.;
    for(j2=-1,j1=1,j=0;j<6;j++,j1++) {


        //for(k=skip-1;k<data->nsubjects;k++) x[k] = data->x[k][j1]*convertmm[j];
        //START160620
        //for(k=skip-1<0?0:skip-1;k<data->nsubjects;k++) x[k] = data->x[k][j1]*convertmm[j];
        //for(k=!skip?0:skip-1;k<data->nsubjects;k++) x[k] = data->x[k][j1]*convertmm[j];
        //START161006
        for(k=0;k<data->nsubjects;k++) x[k] = data->x[k][j1]*convertmm[j];
        for(k=1;k<data->nsubjects;k++) fd[k] += fabs(x[k]-x[k-1])*convertfd[j];

        if(lcR) {
            for(j2++,mean=0.,k=skip;k<data->nsubjects;k++) mean += y[k][j2] = x[k];
            mean /= (double)(data->nsubjects-skip);
            for(k=skip;k<data->nsubjects;k++) y[k][j2] -= mean;
            }
        if(lcR2) {
            for(j2++,mean=0.,k=skip;k<data->nsubjects;k++) mean += y[k][j2] = x[k]*x[k];
            mean /= (double)(data->nsubjects-skip);
            for(k=skip;k<data->nsubjects;k++) y[k][j2] -= mean;
            }

        //START161006
        //for(k=1;k<data->nsubjects;k++) fd[k] += fabs(x[k]-x[k-1])*convertfd[j];

        if(lcRd) {
            ++j2;
            if(!skip) y[skip][j2]=0.;
            for(mean=0.,k=(!skip?1:skip);k<data->nsubjects;k++) mean += y[k][j2] = x[k]-x[k-1];
            mean /= (double)(data->nsubjects-(!skip?1:skip));
            for(k=(!skip?1:skip);k<data->nsubjects;k++) y[k][j2] -= mean;
            }
        if(lcRd2) {
            ++j2;
            if(!skip) y[skip][j2]=0.;
            for(mean=0.,k=(!skip?1:skip);k<data->nsubjects;k++) mean += y[k][j2] = pow(x[k]-x[k-1],2);
            mean /= (double)(data->nsubjects-(!skip?1:skip));
            for(k=(!skip?1:skip);k<data->nsubjects;k++) y[k][j2] -= mean;
            }
        }
    for(j=0;j<data->nsubjects;j++) {

        //for(k=0;k<i;k++) for(l=0;l<numR;l++) fprintf(fp,"0\t");
        //START160620
        for(k0=0;k0<i0;k0++) for(l=0;l<numR;l++) fprintf(fp,"0\t");

        for(k=0;k<numR;k++) fprintf(fp,"%g\t",y[j][k]);

        //for(k=i+1;k<dat->nfiles;k++) for(l=0;l<numR;l++) fprintf(fp,"0\t");
        //START160620
        for(k0=i0+1;k0<dat->nfiles;k0++) for(l=0;l<numR;l++) fprintf(fp,"0\t");

        fprintf(fp,"\n");
        }
    for(j=0;j<(!skip?1:skip);j++) {

        //framesscrubbed[i]++;
        //START160620
        framesscrubbed[i0]++;

        fprintf(fp1,"0\n");
        }

    //for(j=skip+1;j<data->nsubjects;j++) {
    //START161006
    for(j=skip;j<data->nsubjects;j++) {

        fprintf(fp1,"%d\n",fd[j]>FD?0:1);

        //if(fd[j]>FD) framesscrubbed[i]++;
        //START160620
        if(fd[j]>FD) framesscrubbed[i0]++;

        }

    //for(j=0;j<data->nsubjects;j++)printf("fd[%d]=%f\n",j,fd[j]);


    gsl_matrix_view gA = gsl_matrix_view_array(y[skip],(data->nsubjects-skip),numR);
    gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,&gA.matrix,&gA.matrix,0.0,&gATA.matrix);
    for(j=0;j<numR*numR;j++) temp_double[j] = ATA[j];
    if(cond_norm1(temp_double,numR,&cond,ATAm1,perm))cond=0.;
    printf("    Condition number norm1: %f\n",cond);
    if(cond>condmax||cond==0.) {
       if(cond>condmax) printf("    Condition number greater than %f.\n",condmax); else printf("    Design matrix not invertible.\n");
       }
    printf("%s\n",(cond>condmax||cond==0.)?"    Computing singular value decomposition.":
        "    Checking for dependencies with a singular value decomposition.");
    for(j=0;j<numR*numR;j++) temp_double[j] = ATA[j];
    td=gsl_svd_golubreinsch(temp_double,numR,numR,0.,V,S,ATAm1,&cond_norm2,&ndep,work);
    printf("    gsl_svd_golubreinsch cond_norm2=%f ndep=%d tol=%g\n",cond_norm2,ndep,td);
    free(fd);
    free_d2double(y);
    free(x);
    free_data(data);
    }
fclose(fp);
printf("Output written to %s\n",outf);
fclose(fp1);
printf("Output written to %s\n",filename);

//for(i=0;i<dat->nfiles;i++) printf("    run %lu\t%d frames scrubbed\n",(unsigned long)i+1,framesscrubbed[i]);
//START160620
for(i0=0;i0<dat->nfiles;i0++) printf("    run %lu\t%d frames scrubbed\n",(unsigned long)i0+1,framesscrubbed[i0]);

fflush(stdout);
}
