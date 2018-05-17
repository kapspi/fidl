/* Copyright 11/13/17 Washington University.  All Rights Reserved.
   utilities2.c  $Revision: 1.1 $*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "fidl.h"
#include "subs_util.h"
#include "map_disk.h"

int autocorr_guts(char *infile,char *outfile,double *r,int vol,int tdim) {
    int j,k,l,i1,i2;
    double den;
    Memory_Map *mm;
    FILE *fp;
    if(!(mm=map_disk(infile,vol*tdim,0,sizeof(double)))) return 0;
    if(!(fp=fopen_sub(outfile,"w"))) return 0;
    for(j=0;j<vol;j++) {
        for(i1=j,den=0.,k=0;k<tdim;k++,i1+=vol) den += mm->dptr[i1]*mm->dptr[i1];
        for(r[0]=1.,k=1;k<tdim;k++) {
            for(i1=j+k*vol,i2=j,r[k]=0.,l=k;l<tdim;l++,i1+=vol,i2+=vol) r[k] += mm->dptr[i1]*mm->dptr[i2];
            r[k] /= den;
            }
        if(!(fwrite_sub(r,sizeof(double),(size_t)tdim,fp,0))) {
            printf("Error writing to %s. Are you out of memory? Abort!\n",outfile);
            return 0;
            }
        }
    fclose(fp);
    if(!unmap_disk(mm)) return 0;
    return 1;
    }                                                                      /*set to -1 for crosscor*/
int crosscorr_guts(char *infile,char *outfile,double *r,int vol,int tdim,int maxlik_unbias,double *instack) {
    int j,k,l,m,i1,i2;
    double *den=NULL,*dptr;
    Memory_Map *mm=NULL;
    FILE *fp;
    if(infile) {
        if(!(mm=map_disk(infile,vol*tdim,0,sizeof(double)))) return 0; 
        dptr = mm->dptr;
        }
    else {
        dptr = instack;
        }
    if(maxlik_unbias<0) {
        if(!(den=malloc(sizeof*den*vol))) {
            printf("Error: Unable to malloc den in crosscor_guts\n");
            return 0;
            }
        for(j=0;j<vol;j++) for(i1=j,den[j]=0.,k=0;k<tdim;k++,i1+=vol) den[j]+=dptr[i1]*dptr[i1];
        for(j=0;j<vol;j++) den[j]=sqrt(den[j]);
        }
    if(!(fp=fopen_sub(outfile,"w"))) return 0;
    for(j=0;j<vol;j++) {
        for(m=0;m<vol;m++) {
            for(k=0;k<tdim-maxlik_unbias;k++) {
                for(i1=j+k*vol,i2=m,r[k]=0.,l=k;l<tdim;l++,i1+=vol,i2+=vol) r[k] += dptr[i1]*dptr[i2];
                r[k] /= maxlik_unbias<0 ? den[j]*den[m] : (double)(tdim-k-maxlik_unbias);
                /*printf("r[%d]=%f\n",k,r[k]);*/
                }
            if(!(fwrite_sub(r,sizeof(double),(size_t)(tdim-maxlik_unbias),fp,0))) {
                printf("Error writing to %s. Are you out of memory? Abort!\n",outfile);
                return 0;
                }
            }
        }
    fclose(fp);
    if(infile) if(!unmap_disk(mm)) return 0;
    if(maxlik_unbias<0) free(den);
    return 1;
    }
int crosscov_guts2(char *outfile,double *r,int vol1,double *in1,int vol2,double *in2,int tdim,int nlags,int maxlik_unbias)
{
    int i,j,k,l,m,i1,i2;
    FILE *fp;
    if(!(fp=fopen_sub(outfile,"w"))) return 0;
    for(j=0;j<vol1;j++) {
        for(m=0;m<vol2;m++) {
            for(i=0,k=nlags;k>0;k--,i++) {
                for(i1=j+k*vol1,i2=m,r[i]=0.,l=k;l<tdim;l++,i1+=vol1,i2+=vol2) r[i] += in1[i1]*in2[i2];
                r[i] /= (double)(tdim-k-maxlik_unbias);
                }
            for(k=0;k<nlags+1;k++,i++) {
                for(i1=m+k*vol2,i2=j,r[i]=0.,l=k;l<tdim;l++,i1+=vol2,i2+=vol1) r[i] += in2[i1]*in1[i2];
                r[i] /= (double)(tdim-k-maxlik_unbias);
                }
            if(!(fwrite_sub(r,sizeof(double),(size_t)(nlags*2+1),fp,0))) {
                printf("Error writing to %s. Are you out of memory? Abort!\n",outfile);
                return 0;
                }
            }
        }
    fclose(fp);
    return 1;
}

/*START121113*/
int crosscov_guts3(char *outfile,double *r,int vol1,double *in1,int vol2,double *in2,int tdim,int nlags,int maxlik_unbias,
    double *sd1,double *sd2) /*if sd1 sd2 not null, then crosscorr is computed*/
{                                                                                               
    int i,j,k,l,m,i1,i2;
    FILE *fp;
    if(sd1&&sd2) {
        for(j=0;j<vol1;j++) {
            for(i1=j,sd1[j]=0.,k=0;k<tdim;k++,i1+=vol1) sd1[j]+=in1[i1]*in1[i1];
            sd1[j]=sqrt(sd1[j]);
            }
        for(j=0;j<vol2;j++) {
            for(i1=j,sd2[j]=0.,k=0;k<tdim;k++,i1+=vol2) sd2[j]+=in2[i1]*in2[i1];
            sd2[j]=sqrt(sd2[j]);
            }
        }
    if(!(fp=fopen_sub(outfile,"w"))) return 0;
    for(j=0;j<vol1;j++) {
        for(m=0;m<vol2;m++) {
            for(i=0,k=nlags;k>0;k--,i++) {
                for(i1=j+k*vol1,i2=m,r[i]=0.,l=k;l<tdim;l++,i1+=vol1,i2+=vol2) r[i] += in1[i1]*in2[i2];
                r[i] /= sd1&&sd2 ? sd1[j]*sd2[m] : (double)(tdim-k-maxlik_unbias);
                }
            for(k=0;k<nlags+1;k++,i++) {
                for(i1=m+k*vol2,i2=j,r[i]=0.,l=k;l<tdim;l++,i1+=vol2,i2+=vol1) r[i] += in2[i1]*in1[i2];
                r[i] /= sd1&&sd2 ? sd1[j]*sd2[m] : (double)(tdim-k-maxlik_unbias);
                }
            if(!(fwrite_sub(r,sizeof(double),(size_t)(nlags*2+1),fp,0))) {
                printf("Error writing to %s. Are you out of memory? Abort!\n",outfile);
                return 0;
                }
            }
        }
    fclose(fp);
    return 1;
}




#if 0
Validated 091217. Do not need to remove the mean, already zero mean. 
int cov_guts(char *infile,char *outfile,double *r,int vol,int tdim,int maxlik_unbias)
{                                                                      /* neg don't normalize */
    int i,j,k,l,m,i1,i2;
    double m1,m2;
    Memory_Map *mm;
    if(!(mm=map_disk(infile,vol*tdim,0,sizeof(double)))) return 0;
    for(k=j=0;j<vol;j++) {
        for(m=0;m<vol;m++,k++) {
            for(i1=j,i2=m,m1=m2=0.,l=0;l<tdim;l++,i1+=vol,i2+=vol) {m1+=mm->dptr[i1];m2+=mm->dptr[i2];}
            m1/=(double)tdim;m2/=(double)tdim;
            for(i1=j,i2=m,r[k]=0.,l=0;l<tdim;l++,i1+=vol,i2+=vol) r[k] += (mm->dptr[i1]-m1)*(mm->dptr[i2]-m2);
            if(maxlik_unbias>=0) r[k]/=(double)(tdim-maxlik_unbias);
            }
        }
    if(!writestack(outfile,r,sizeof(double),(size_t)(vol*vol),0)) return 0;
    if(!unmap_disk(mm)) return 0;
    return 1;
}
#endif
                                                                    /* neg don't normalize */
int cov_guts(char *infile,char *outfile,double *r,int vol,int tdim,int maxlik_unbias) {
    int j,k,l,m,i1,i2;
    Memory_Map *mm;
    if(!(mm=map_disk(infile,vol*tdim,0,sizeof(double)))) return 0;
    for(k=j=0;j<vol;j++) {
        for(m=0;m<vol;m++,k++) {
            for(i1=j,i2=m,r[k]=0.,l=0;l<tdim;l++,i1+=vol,i2+=vol) r[k] += mm->dptr[i1]*mm->dptr[i2];
            if(maxlik_unbias>=0) r[k]/=(double)(tdim-maxlik_unbias);
            }
        }
    if(outfile) if(!writestack(outfile,r,sizeof(double),(size_t)(vol*vol),0)) return 0;
    if(!unmap_disk(mm)) return 0;
    return 1;
    }
int corr_guts(char *infile,char *outfile,double *r,double *den,int vol,int tdim) {
    int j,k,l,m,i1,i2;
    Memory_Map *mm;
    if(!(mm=map_disk(infile,vol*tdim,0,sizeof(double)))) return 0;
    for(j=0;j<vol;j++) for(i1=j,den[j]=0.,k=0;k<tdim;k++,i1+=vol) den[j]+=mm->dptr[i1]*mm->dptr[i1];
    for(j=0;j<vol;j++) den[j]=sqrt(den[j]);
    for(k=j=0;j<vol;j++) {
        for(m=0;m<vol;m++,k++) {
            for(i1=j,i2=m,r[k]=0.,l=0;l<tdim;l++,i1+=vol,i2+=vol) r[k] += mm->dptr[i1]*mm->dptr[i2];
            r[k]/=den[j]*den[m];
            }
        }
    if(outfile) if(!writestack(outfile,r,sizeof(double),(size_t)(vol*vol),0)) return 0;
    if(!unmap_disk(mm)) return 0;
    return 1;
    }                                                                /*neg don't normalize*/
int var_guts(char *infile,char *outfile,double *r,int vol,int tdim,int maxlik_unbias) {
    int j,l,i1;
    Memory_Map *mm;
    if(!(mm=map_disk(infile,vol*tdim,0,sizeof(double)))) return 0;
    for(j=0;j<vol;j++) {
        for(i1=j,r[j]=0.,l=0;l<tdim;l++,i1+=vol) r[j] += mm->dptr[i1]*mm->dptr[i1];
        if(maxlik_unbias>=0) r[j]/=(double)(tdim-maxlik_unbias);
        }
    if(outfile) if(!writestack(outfile,r,sizeof(double),(size_t)vol,0)) return 0;
    if(!unmap_disk(mm)) return 0;
    return 1;
    }
