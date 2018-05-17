/* Copyright 12/11/17 Washington University.  All Rights Reserved.
   fidl_sim.cxx  $Revision: 1.6 $*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "fidl.h"
#include "gauss_smoth2.h"
#include "spatial_extent2.h"

//START160523
#include "subs_mask.h"
#include "subs_util.h"

int main(int argc,char **argv)
{
char *maskf=NULL,*outf=NULL,timestr[23],rngf[MAXNAME],outftable[MAXNAME];
int i,j,k,nsim=1000,nthresh=0,SunOS_Linux,*count,*index=NULL,nreg,lizard,lct=1;
size_t i1,j1,k1;
unsigned long s=0;
float fwhm=0.;
double df1=0.,df2=0.,*thresh=NULL,*temp_double,*stat,scls=0,*temp_double2,*temp_double3=NULL,*temp_double4=NULL,*df1s=NULL,*df2s=NULL;

//Mask_Struct *ms;
//START171211
mask ms;

Spatial_Extent2 *se;
gauss_smoth2_struct *gs=NULL;
Interfile_header *ifh;
FILE *fp;
if(argc<9) {
    fprintf(stderr,"fidl_sim -df1 18 -df2 342 -fwhm 2 -mask /home/usr/fidl/lib/glm_atlas_mask_222.4dfp.img -thresh 3.0 4.0\n");
    fprintf(stderr,"  -nsim:   Number of simulations. Default is 1000.\n");

    //START171211
    fprintf(stderr,"  If -df1 is not specified, then standard normal variates are generated.\n");
    fprintf(stderr,"  If -df1 and -df2 are both specified, then gaussianized F variates are generated.\n");
    fprintf(stderr,"  If only -df1 is specified, then you need to indicate -t (default) or -chisq.\n");
    fprintf(stderr,"    -t:     Generate gaussianized t variates.\n");
    fprintf(stderr,"    -chisq: Generate gaussianized chi-squared statistics.\n");

    fprintf(stderr,"  -df1:    Numerator degrees of freedom.\n");
    fprintf(stderr,"  -df2:    Denominator degrees of freedom.\n");
    fprintf(stderr,"  -fwhm:   Gaussian smoothing in voxels. Default is no smoothing.\n");
    fprintf(stderr,"  -mask:   Limits simulation.\n");
    fprintf(stderr,"  -thresh: Thresholds.\n");
    fprintf(stderr,"  -seed:   0 to 2^32-1. Default is 0.\n");
    fprintf(stderr,"  -out:   Text output file. Default is fidl_simTIMESTAMP.txt\n");
    exit(-1);
    }
for(i=1;i<argc;i++) {
    if(!strcmp(argv[i],"-nsim") && argc > i+1) nsim = atoi(argv[++i]);
    if(!strcmp(argv[i],"-df1") && argc > i+1) df1 = atof(argv[++i]);
    if(!strcmp(argv[i],"-df2") && argc > i+1) df2 = atof(argv[++i]);
    if(!strcmp(argv[i],"-fwhm") && argc > i+1) fwhm = atof(argv[++i]);
    if(!strcmp(argv[i],"-mask") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1]) maskf = argv[++i];
    if(!strcmp(argv[i],"-thresh") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++nthresh;
        if(!(thresh=malloc(sizeof*thresh*nthresh))) {
            printf("fidlError: Unable to malloc thresh\n");
            exit(-1);
            }
        for(j=0;j<nthresh;j++) thresh[j]=atof(argv[++i]);
        }
    if(!strcmp(argv[i],"-seed") && argc > i+1) s = strtoul(argv[++i],NULL,0);
    if(!strcmp(argv[i],"-out") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1]) outf = argv[++i];

    //START171211
    if(!strcmp(argv[i],"-chisq")) lct=0;

    }
if((SunOS_Linux=checkOS())==-1) exit(-1);
if(!nsim) {
    printf("nsim = %d  Need to specify the number of simulations with -nsim\n",nsim);
    exit(-1);
    }
if(!df1) {
    printf("-df1 not specified. We will simulate Z statistics.\n");
    }
else if(!df2) {
    printf("-df2 not specified. We will simulate T statistics.\n");
    }
if(!maskf) {
    printf("Need to specify the mask with -mask\n");
    exit(-1);
    }
if(!nthresh) {
    printf("Need to specify the threshold(s) with -thresh\n");
    exit(-1);
    }
if(!outf) {
    if(!(outf=malloc(sizeof*outf*36))) {
        printf("fidlError: Unable to malloc outf\n");
        exit(-1);
        }
    sprintf(outf,"fidl_sim%s.txt",make_timestr2(timestr));
    }

//if(!(ms=read_mask(maskf,SunOS_Linux,(LinearModel*)NULL))) exit(-1);
//START171211
if(!(ms.read_mask(maskf,SunOS_Linux,(LinearModel*)NULL)))exit(-1);
ms.get_param(mslenbrain,msbrnidx,msvol,msxdim,msydim,mszdim);

//STARTHERE


if(!(count=malloc(sizeof*count*(mslenbrain+1)*nthresh))) {
    printf("fidlError: Unable to malloc count\n");
    exit(-1);
    }
for(i=0;i<(mslenbrain+1)*nthresh;i++) count[i]=0;
if(!(temp_double=malloc(sizeof*temp_double*msvol))) {
    printf("fidlError: Unable to malloc temp_double\n");
    exit(-1);
    }
if(fwhm) {
    if(!(gs=gauss_smoth2_init(ms->xdim,ms->ydim,ms->zdim,fwhm,fwhm))) exit(-1);
    if(!(stat=malloc(sizeof*stat*ms->lenvol))) {
        printf("fidlError: Unable to malloc stat\n");
        exit(-1);
        }
    for(i=0;i<ms->lenvol;i++) temp_double[i]=0.;
    temp_double[((int)rint((double)ms->zdim/2.)-1)*ms->xdim*ms->ydim + (int)rint((double)ms->ydim/2.)*ms->xdim 
        + (int)rint((double)ms->xdim/2.)] = 1.;
    gauss_smoth2(temp_double,stat,gs);
    for(scls=0.,i=0;i<ms->lenvol;i++) if(stat[i]) scls+=stat[i]*stat[i];
    scls = sqrt(scls);
    }
else {
    stat = temp_double;
    }
if(nthresh>1) {
    if(!(temp_double2=malloc(sizeof*temp_double2*ms->lenvol))) {
        printf("fidlError: Unable to malloc temp_double2\n");
        exit(-1);
        }
    }
else {
    temp_double2 = stat;
    }
if(df1) {
    if(!(temp_double3=malloc(sizeof*temp_double3*ms->lenbrain))) {
        printf("fidlError: Unable to malloc temp_double3\n");
        exit(-1);
        }
    if(!(temp_double4=malloc(sizeof*temp_double4*ms->lenbrain))) {
        printf("fidlError: Unable to malloc temp_double4\n");
        exit(-1);
        }
    if(!(df1s=malloc(sizeof*df1s*ms->lenbrain))) {
        printf("fidlError: Unable to malloc df1s\n");
        exit(-1);
        }
    for(i=0;i<ms->lenbrain;i++) df1s[i] = df1;
    if(!(index=malloc(sizeof*index*ms->lenbrain))) {
        printf("fidlError: Unable to malloc index\n");
        exit(-1);
        }
    if(df2) {
        if(!(df2s=malloc(sizeof*df2s*ms->lenbrain))) {
            printf("fidlError: Unable to malloc df2s\n");
            exit(-1);
            }
        for(i=0;i<ms->lenbrain;i++) df2s[i] = df2;
        }
    }

printf("seed = %lu\nfhwm = %f\nmask = %s\n",s,fwhm,maskf);
printf("thresh = "); for(i=0;i<nthresh;i++) printf("%g ",thresh[i]); printf("\n");
printf("ms->xdim=%d ms->ydim=%d ms->zdim=%d ms->lenvol=%d ms->lenbrain=%d\nscls=%f\n",ms->xdim,ms->ydim,ms->zdim,ms->lenvol,
    ms->lenbrain,scls);
if(!(se=spatial_extent2init(ms->xdim,ms->ydim,ms->zdim,ms->lenbrain,ms->brnidx))) exit(-1);
const gsl_rng_type *T = gsl_rng_mt19937;
gsl_rng *r = gsl_rng_alloc(T); 
gsl_rng_set(r,s);
for(i=0;i<nsim;i++) {
    for(j=0;j<ms->lenvol;j++) temp_double[j]=0.;


    #if 0
    if(!df1)
        for(j=0;j<ms->lenbrain;j++) temp_double[ms->brnidx[j]] = gsl_ran_gaussian_ziggurat(r,1.); 
    else if(!df2) {
        for(j=0;j<ms->lenbrain;j++) temp_double3[j] = gsl_ran_tdist(r,df1); 
        t_to_z(temp_double3,temp_double4,ms->lenbrain,df1s,index);
        for(j=0;j<ms->lenbrain;j++) temp_double[ms->brnidx[j]] = temp_double4[j]; 
        }
    else {
        for(j=0;j<ms->lenbrain;j++) temp_double3[j] = gsl_ran_fdist(r,df1,df2); 
        f_to_z(temp_double3,temp_double4,ms->lenbrain,df1s,df2s,index);
        for(j=0;j<ms->lenbrain;j++) temp_double[ms->brnidx[j]] = temp_double4[j]; 
        }
    #endif
    //START171211
    if(!df1)
        for(j=0;j<ms->lenbrain;j++) temp_double[ms->brnidx[j]] = gsl_ran_gaussian_ziggurat(r,1.);
    else{
        if(!df2){
            if(lct){
                for(j=0;j<ms->lenbrain;j++) temp_double3[j] = gsl_ran_tdist(r,df1);
                t_to_z(temp_double3,temp_double4,ms->lenbrain,df1s,index);
                }
            else{
                for(j=0;j<ms->lenbrain;j++) temp_double3[j] = gsl_ran_chisq(r,df1);
                x2_to_z(temp_double3,temp_double4,ms->lenbrain,df1s);
                }
            }
        else{
            for(j=0;j<ms->lenbrain;j++) temp_double3[j] = gsl_ran_fdist(r,df1,df2);
            f_to_z(temp_double3,temp_double4,ms->lenbrain,df1s,df2s,index);
            }
        for(j=0;j<ms->lenbrain;j++) temp_double[ms->brnidx[j]] = temp_double4[j];
        }


    if(fwhm>0.) {
        gauss_smoth2(temp_double,stat,gs);
        for(j=0;j<ms->lenvol;j++) stat[j] /= scls;
        }
    for(j1=j=0;j<nthresh;j++,j1+=ms->lenbrain+1) {
        if(nthresh>1) for(k=0;k<ms->lenvol;k++) temp_double2[k]=stat[k];
        if((nreg=spatial_extent2(temp_double2,(float*)NULL,thresh[j],1,ms->lenvol,1,se))) {
            for(k=0;k<nreg;k++) count[j1+se->regsize[k]]++;
            }
        }
    }
if(!(fp=fopen_sub(outf,"w"))) exit(-1);
fprintf(fp,"number of simulations = %d seed = %lu\ndf1 =%f df2 = %f fwhm = %f\n%s\n",nsim,s,df1,df2,fwhm,maskf);
fprintf(fp,"thresholds = ");for(i=0;i<nthresh;i++)fprintf(fp,"%f ",thresh[i]);

//fprintf(fp," %s\n",!df1?"Z statistics":(!df2?"Gaussianized T statistics":"Gaussianized F statistics"));
//START171211
fprintf(fp," %s statistics\n",!df1?"Z":(!df2?(lct?"Gaussianized T":"Gaussianized chi-squared"):"Gaussianized F"));

fprintf(fp,"output state = %s\n\n",rngf);
for(i1=1,i=0;i<nthresh;i++,i1++) { /*i1++ skips 0*/
    fprintf(fp,"threshold = %f  CHECK i1=%lu\n",thresh[i],i1);
    for(j=1;j<=ms->lenbrain;j++,i1++) {
        if((lizard=count[i1])) {
            for(k1=i1+1,k=j+1;k<=ms->lenbrain;k++,k1++) lizard+=count[k1];
            fprintf(fp,"    number of regions with %d or more voxels = %d\n",j,lizard);
            }
        }
    }
fflush(fp);fclose(fp);
printf("Output written to %s\n",outf);

/*START140910*/
strcpy(outftable,outf);
if(!get_tail_sans_ext(outftable)) exit(-1);
strcat(outftable,"_table.txt");
if(!(fp=fopen_sub(outftable,"w"))) exit(-1);

//fprintf(fp,"#%s\nfwhm\tdf1\tdf2\tthreshold\tcluster\tp\n",maskf);
//START160608
fprintf(fp,"#%s nsim=%d\nfwhm\tdf1\tdf2\tthreshold\tcluster\tp\n",maskf,nsim);

for(i1=1,i=0;i<nthresh;i++,i1++) { /*i1++ skips 0*/
    for(j=1;j<=ms->lenbrain;j++,i1++) {
        if((lizard=count[i1])) {
            for(k1=i1+1,k=j+1;k<=ms->lenbrain;k++,k1++) lizard+=count[k1];
            fprintf(fp,"%.2f\t%.2f\t%.2f\t%.2f\t%d\t%g\n",fwhm,df1,df2,thresh[i],j,(double)lizard/(double)nsim);
            }
        }
    }
fflush(fp);fclose(fp);
printf("Output written to %s\n",outf);

strcpy(rngf,outf);
if(!get_tail_sans_ext(rngf)) exit(-1);
strcat(rngf,"_rng.4dfp.img");
if(!(fp=fopen_sub(rngf,"w"))) exit(-1);
if(gsl_rng_fwrite(fp,r)) printf("fidlError: gsl_rng_fwrite\n"); /*will need ifh, b/c binary file: bigendian or littleendian*/ 
fflush(fp);fclose(fp);
if(!(ifh=init_ifh(4,ms->xdim,ms->ydim,ms->zdim,nsim,0.,0.,0.,SunOS_Linux?0:1))) exit(-1);
ifh->df1=df1;ifh->df2=df2;ifh->fwhm=fwhm;
#if 0
if(!(ifh->mask=malloc(sizeof*ifh->mask*(strlen(maskf)+1)))) {
    printf("fidlError: Unable to malloc ifh->mask\n");
    exit(-1);
    }
strcpy(ifh->mask,maskf);
#endif
ifh->mask=maskf;
if(!write_ifh(rngf,ifh,0)) exit(-1); 
printf("Output state written to %s\n",rngf);
}
