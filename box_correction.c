/* Copyright 12/31/99 Washington University.  All Rights Reserved.
   box_correction.c  $Revision: 1.64 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "nrutil.h"
#include "fidl.h"
#include "anova_header_new.h"
#include "dmatrix_mult.h"
#include "box_correction.h"
#include "subs_util.h"

//START180116
//#include "subs_glm.h"
//#include "map_disk.h"
//#include "subs_mask.h"

#define B_THRESHOLD 1000000
double calculate_epsilon_triangular(double **transformed_ATAm1,int num_tc)
{
    int i,j;
    double numerator=0,denominator=0;
    for(i=1;i<=num_tc;i++) numerator += transformed_ATAm1[i][i];
    numerator *= numerator;
    for(i=1;i<=num_tc;i++) for(j=1;j<=i;j++) denominator += transformed_ATAm1[i][j]*transformed_ATAm1[i][j];
    return(numerator/denominator/num_tc);
}
double calculate_epsilon_whole_matrix(double **transformed_ATAm1,int num_tc)
{
    int i,j;
    double numerator=0,denominator=0;

    for(i=1;i<=num_tc;i++) numerator += transformed_ATAm1[i][i];
    numerator *= numerator;
    for(i=1;i<=num_tc;i++) for(j=1;j<=num_tc;j++) denominator += transformed_ATAm1[i][j]*transformed_ATAm1[i][j];
    return numerator/denominator/num_tc;
}

double calculate_epsilon_whole_matrix_box(double **transformed_ATAm1,int num_tc)
{
    int i,j;
    double num_tc_sq,vbartt=0,vtdot,vbardotdot=0,den_term1=0,den_term2=0,epsilon;

    num_tc_sq = num_tc*num_tc;
    for(i=0;i<num_tc;i++) vbartt += transformed_ATAm1[i+1][i+1];
    vbartt /= num_tc;
    for(i=1;i<=num_tc;i++) {
        vtdot = 0;
        for(j=1;j<=num_tc;j++) {
            vtdot += transformed_ATAm1[i][j];
            den_term1 += transformed_ATAm1[i][j]*transformed_ATAm1[i][j];
            }
        vbardotdot += vtdot;
        den_term2 += (vtdot*vtdot)/num_tc_sq;
        }
    vbardotdot /= num_tc_sq;
    epsilon = (num_tc_sq*(vbartt-vbardotdot)*(vbartt-vbardotdot))/((num_tc-1)
        *(den_term1-2*num_tc*den_term2+num_tc_sq*vbardotdot*vbardotdot));
    return epsilon; 
}

void min_and_max_var(double **transformed_ATAm1,int num_tc,double *min,double *max)
{
    int i;

    for(*min=1.e20,*max=-1.e20,i=1;i<=num_tc;i++) {
        if(transformed_ATAm1[i][i] > *max) *max = transformed_ATAm1[i][i];
        if(transformed_ATAm1[i][i] < *min) *min = transformed_ATAm1[i][i];
        }
}

int calculate_and_print_correlation_coefficient(double **cov,int dim,Interfile_header *ifh,char *filename)
{
    int i,j,k;
    float *temp_float;

    if(!(temp_float=malloc(sizeof*temp_float*dim*dim))) {
        printf("Error: Unable to malloc temp_float in calculate_and_print_correlation_coefficient\n");
        return 0;
        }
    for(k=0,i=1;i<=dim;i++) for(j=1;j<=dim;j++,k++) temp_float[k] = (float)(cov[i][j]/sqrt(cov[i][i])/sqrt(cov[j][j]));
    ifh->dim1 = dim;
    ifh->dim2 = dim;
    ifh->dim3 = 1;
    ifh->dim4 = 1;
    ifh->number_format = (int)FLOAT_IF;
    if(!writestack(filename,temp_float,sizeof(float),(size_t)(dim*dim),0)) exit(-1);
    fprintf(stderr,"Output written to %s\n",filename);
    if(!write_ifh(filename,ifh,(int)FALSE)) return 0;
    free(temp_float);
    return 1;
}












#if 0
double **cov_avg_reduced_ATA(char **glm_files,int num_glm_files,TC *tcs,int swapbytes,int tc_contrast)
{
    int i,j,k;
    double **A,**ATA,**ATAxtransformT,**transformed_ATA,**avg_transformed_ATA,**ATAm1,**transform,denominator,min,max,cond;
    LinearModel *glm;

    avg_transformed_ATA = (double **)dmatrix_0(1,tcs->num_tc,1,tcs->num_tc);
    for(j=0;j<num_glm_files;j++) {
        if(!(glm = read_glm(glm_files[j],1,swapbytes))) {
            printf("Error: cov_avg_reduced_ATA -> read_glm\n");
            return NULL;
            }
        A = dmatrix(1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
        for(i=0;i<glm->ifh->glm_Nrow;i++) for(k=0;k<glm->ifh->glm_Mcol;k++) A[i+1][k+1] = (double)glm->AT[k][i];
        ATA = dmatrix_mult(A,A,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,(int)TRANSPOSE_FIRST);
        transform = dmatrix_0(1,tcs->num_tc,1,glm->ifh->glm_Mcol);
        if(!tc_contrast) {
            for(i=0;i<tcs->num_tc;i++) {
                if(tcs->num_tc_to_sum[i][j] == 1)
                    transform[i+1][(int)tcs->tc[i][j][0]] = 1;
                else {
                    for(denominator=k=0;k<tcs->num_tc_to_sum[i][j];k++)
                        denominator += 1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]];
                    for(k=0;k<tcs->num_tc_to_sum[i][j];k++)
                        transform[i+1][(int)tcs->tc[i][j][k]] = (1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]])/denominator;
                    }
                 }
            }
        else {
            for(i=0;i<tcs->num_tc;i++) 
                for(k=0;k<glm->ifh->glm_Mcol;k++)
                    transform[i+1][k+1] = glm->c[((int)tcs->tc[i][j][0]-1)*glm->ifh->glm_Mcol+k];
            }
        ATAxtransformT = dmatrix_mult(ATA,transform,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,glm->ifh->glm_Mcol,
            (int)TRANSPOSE_SECOND);
        transformed_ATA = dmatrix_mult(transform,ATAxtransformT,tcs->num_tc,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,
            (int)TRANSPOSE_NONE);
        for(i=1;i<=tcs->num_tc;i++) for(k=1;k<=tcs->num_tc;k++) avg_transformed_ATA[i][k] += transformed_ATA[i][k];
        free_dmatrix(A,1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
        free_dmatrix(ATA,1,glm->ifh->glm_Mcol,1,glm->ifh->glm_Mcol);
        free_dmatrix(transform,1,tcs->num_tc,1,glm->ifh->glm_Mcol);
        free_dmatrix(ATAxtransformT,1,glm->ifh->glm_Mcol,1,tcs->num_tc);
        free_dmatrix(transformed_ATA,1,tcs->num_tc,1,tcs->num_tc);
        free_glm(glm,(int)TRUE);
        }
    for(i=1;i<=tcs->num_tc;i++) for(k=1;k<=tcs->num_tc;k++) avg_transformed_ATA[i][k] /= num_glm_files;
    ATAm1 = dmatrix(1,tcs->num_tc,1,tcs->num_tc);
    inverse_cover(avg_transformed_ATA,ATAm1,tcs->num_tc,(double)B_THRESHOLD,"avg_transformed_ATA");
    #if 0
    printf("ATAm1\n");
    for(i=1;i<=tcs->num_tc;i++) {
        for(j=1;j<=tcs->num_tc;j++) printf("%f ",ATAm1[i][j]);
        printf("\n");
        }
    printf("\n");
    #endif
    free_dmatrix(avg_transformed_ATA,1,tcs->num_tc,1,tcs->num_tc);
    return ATAm1;
}
#endif









/*   sub1 cov reg1
          cov reg2
          . . .
          cov regn
     sub2 cov reg1
          cov reg2
          . . .
          cov regn
     . . .
     subN cov reg1
          cov reg2
          . . .
          cov regn
*/

#if 0
int subcov_reduced_ATA(char **glm_files,int num_glm_files,TC *tcs,double *subcov,int SunOS_Linux,int tc_contrast,Files_Struct *Vconc)
{
    int i,j,k,l,m,n,o,p,q,qq,m1,n1,oo,oo1,total_length,nreg,aadvance,nextreg,nextrow,*length,*area,nglms_each,ntc_each,a1,a2;
    double **A,**ATA,**ATAxtransformT,**transformed_ATA,**avg_transformed_ATA,**ATAm1,**transform,denominator,
        **transformed_ATVA,**ATAm1xtransformed_ATVA,**cov,**VA,**ATVA,**ATVAxtransformT;
    LinearModel *glm;
    Interfile_header *ifh;
    Files_Struct *fs;
    Memory_Map **mm;
    printf("************** subcov_reduced_ATA *********************\n");

    #if 0
    for(i=0;i<tcs->num_tc;i++) {
        printf("i=%d\n",i);
        for(j=0;j<tcs->each[i];j++) printf("%d ",(int)tcs->tc[i][j][0]); 
        printf("\n");
        }
    #endif

    nglms_each = num_glm_files/tcs->each[0];
    ntc_each = tcs->num_tc/nglms_each;

    printf("tcs->num_tc=%d nglms_each=%d ntc_each=%d\n",tcs->num_tc,nglms_each,ntc_each);

    transformed_ATA = dmatrix(1,ntc_each,1,ntc_each);
    ATAm1 = dmatrix(1,ntc_each,1,ntc_each);
    if(Vconc) {
        if(nglms_each!=1) {
            printf("Error: This segment of code needs to be rewritten to handle more than one glm per subject.\n");
            return 0;
            }
        transformed_ATVA = dmatrix(1,tcs->num_tc,1,tcs->num_tc);
        ATAm1xtransformed_ATVA = dmatrix(1,tcs->num_tc,1,tcs->num_tc);
        cov = dmatrix(1,tcs->num_tc,1,tcs->num_tc);
        }

    for(l=j=p=0;p<tcs->each[0];p++) {
        for(qq=q=0;q<nglms_each;q++,j++) {
            if(!(glm = read_glm(glm_files[j],1,SunOS_Linux))) {
                printf("Error: subcov_reduced_ATA -> read_glm\n");
                return 0;
                }
            A = dmatrix(1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);

            /*for(i=1;i<=glm->ifh->glm_Nrow;i++) for(k=1;k<=glm->ifh->glm_Mcol;k++) A[i][k] = (double)glm->A[i][k];*/
            /*START61*/
            for(i=0;i<glm->ifh->glm_Nrow;i++) for(k=0;k<glm->ifh->glm_Mcol;k++) A[i+1][k+1] = (double)glm->AT[k][i];

            ATA = dmatrix_mult(A,A,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,(int)TRANSPOSE_FIRST);

            /*printf("ATA\n");
            for(i=1;i<=glm->ifh->glm_Mcol;i++) {
                for(k=1;k<=glm->ifh->glm_Mcol;k++) printf("%f ",ATA[i][k]);
                printf("\n");
                }*/

            transform = dmatrix_0(1,ntc_each,1,glm->ifh->glm_Mcol);
            if(!tc_contrast) {
                for(i=0;i<ntc_each;i++,qq++) {
                    if(tcs->num_tc_to_sum[qq][p] == 1) {
                        transform[i+1][(int)tcs->tc[qq][p][0]] = 1;
                        }
                    else {
/*THIS IS WRONG*/
                        for(denominator=k=0;k<tcs->num_tc_to_sum[qq][p];k++)
                            denominator += 1./ATA[(int)tcs->tc[qq][p][k]][(int)tcs->tc[qq][p][k]];
                        for(k=0;k<tcs->num_tc_to_sum[qq][p];k++) transform[i+1][(int)tcs->tc[qq][p][k]] =
                            (1./ATA[(int)tcs->tc[qq][p][k]][(int)tcs->tc[qq][p][k]])/denominator;
                        }
                     }
                }
            else {
                for(i=0;i<ntc_each;i++,qq++)
                    for(k=0;k<glm->ifh->glm_Mcol;k++)
                        transform[i+1][k+1] = glm->c[((int)tcs->tc[qq][p][0]-1)*glm->ifh->glm_Mcol+k];
                }
            ATAxtransformT = dmatrix_mult(ATA,transform,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,ntc_each,glm->ifh->glm_Mcol,
                (int)TRANSPOSE_SECOND);
            dmatrix_mult_nomem(transform,ATAxtransformT,ntc_each,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,ntc_each,(int)TRANSPOSE_NONE,
                transformed_ATA);
            inverse_cover(transformed_ATA,ATAm1,ntc_each,(double)B_THRESHOLD,"transformed_ATA");

            printf("transformed_ATA\n");
            for(i=1;i<=ntc_each;i++) {
                for(k=1;k<=ntc_each;k++) printf("%f ",transformed_ATA[i][k]);
                printf("\n");
                }


            if(!Vconc) {
                /*printf("ATAm1\n");*/
                a1 = q*ntc_each; 
                a2 = (nglms_each-q-1)*ntc_each;
                for(i=1;i<=ntc_each;i++,l+=a2) {
                    for(l+=a1,k=1;k<=ntc_each;k++,l++) {
                        subcov[l] = ATAm1[i][k];
                        /*printf("%f ",ATAm1[i][k]);*/
                        /*printf("i=%d k=%d l=%d\n",i,k,l);*/
                        }
                    /*printf("\n");*/
                    }
                }
            else {
                printf("Using data %s\n",Vconc->files[j]);
                VA = dmatrix(1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
                ATVA = dmatrix(1,glm->ifh->glm_Mcol,1,glm->ifh->glm_Mcol);
                ATVAxtransformT = dmatrix(1,glm->ifh->glm_Mcol,1,tcs->num_tc);
                if(!(fs=read_conc(Vconc->files[j]))) return 0;
                if(!(length=malloc(sizeof*length*fs->nfiles))) {
                    printf("Error: Unable to malloc length\n");
                    exit(-1);
                    }
                if(!(area=malloc(sizeof*area*fs->nfiles))) {
                    printf("Error: Unable to malloc area\n");
                    exit(-1);
                    }
                for(total_length=i=0;i<fs->nfiles;i++) {
                    if(!(ifh=read_ifh(fs->files[i],(Interfile_header*)NULL))) return 0;
                    length[i] = ifh->dim1;
                    area[i] = length[i]*length[i];
                    total_length += ifh->dim1;
                    if(ifh->dim2 != ifh->dim1) {
                        printf("Error: ifh->dim2 = %d ifh->dim1 = %d They must be equal. Abort!\n",ifh->dim2,ifh->dim1);
                        return 0;
                        }
                    if(!i && !j) {
                        nreg = ifh->dim3;
                        }
                    else if(ifh->dim3 != nreg) {
                        printf("Error: ifh->dim3=%d nreg=%d  Must be equal. Abort!\n",ifh->dim3,nreg);
                        return 0;
                        }
                    if(ifh->dim4 != 1) {
                        printf("Error: ifh->dim4 = %d. It must equal 1. Abort!\n",ifh->dim4);
                        return 0;
                        }
                    free_ifh(ifh,(int)FALSE);
                    }
                if(total_length != glm->ifh->glm_Nrow) {
                    printf("Error: total_length=%d glm->ifh->glm_Nrow=%d  Must be equal. Abort!\n",total_length,glm->ifh->glm_Nrow);
                    return 0;
                    }
                if(!(mm=malloc(sizeof*mm*fs->nfiles))) {
                    printf("Error: Unable to malloc mm\n");
                    return 0;
                    }
                for(i=0;i<fs->nfiles;i++) if(!(mm[i]=map_disk(fs->files[i],area[i]*nreg,0,sizeof(float)))) return 0;
                for(i=0;i<nreg;i++) {
                    if(mm[0]->ptr[i*area[0]]==(float)UNSAMPLED_VOXEL) {
                        l += tcs->num_tc*tcs->num_tc;
                        continue;
                        }
                    for(m1=aadvance=1,k=0;k<fs->nfiles;k++,aadvance+=length[k]) {
                        nextreg = i*area[k];
                        for(nextrow=0,m=0;m<length[k];m++,m1++,nextrow+=length[k]) {
                            for(n1=1,n=0;n<glm->ifh->glm_Mcol;n++,n1++) {
                                VA[m1][n1]=0.;
                                for(oo1=aadvance,oo=nextreg+nextrow,o=0;o<length[k];o++,oo++,oo1++) {
                                    VA[m1][n1] += (double)mm[k]->ptr[oo]*A[oo1][n1];
                                    /*printf("mm[%d]->ptr[%d]=%f A[%d][%d]=%f VA[%d][%d]=%f\n",
                                        k,oo,mm[k]->ptr[oo],oo1,n1,A[oo1][n1],m1,n1,VA[m1][n1]);*/
                                    }
                                /*printf("VA[%d][%d]=%f\n",m1,n1,VA[m1][n1]);*/
                                }
                            }
                        }

                    #if 0
                    printf("VA=\n");
                    for(k=1;k<=glm->ifh->glm_Nrow;k++) {
                        for(m=1;m<=glm->ifh->glm_Mcol;m++) printf("%f ",VA[k][m]);
                        printf("\n");
                        exit(-1);
                        }
                    exit(-1);
                    #endif
    
                    dmatrix_mult_nomem(A,VA,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,
                        (int)TRANSPOSE_FIRST,ATVA);
    
                    #if 0
                    printf("ATVA=\n");
                    for(k=1;k<=glm->ifh->glm_Mcol;k++) {
                        for(m=1;m<=glm->ifh->glm_Mcol;m++) printf("%f ",ATVA[k][m]);
                        printf("\n");
                        }
                    exit(-1);
                    #endif

                    dmatrix_mult_nomem(ATVA,transform,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,glm->ifh->glm_Mcol,
                        (int)TRANSPOSE_SECOND,ATVAxtransformT);
                    dmatrix_mult_nomem(transform,ATVAxtransformT,tcs->num_tc,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,
                        (int)TRANSPOSE_NONE,transformed_ATVA);
                    dmatrix_mult_nomem(ATAm1,transformed_ATVA,tcs->num_tc,tcs->num_tc,tcs->num_tc,tcs->num_tc,
                        (int)TRANSPOSE_NONE,ATAm1xtransformed_ATVA);
                    dmatrix_mult_nomem(ATAm1xtransformed_ATVA,ATAm1,tcs->num_tc,tcs->num_tc,tcs->num_tc,tcs->num_tc,
                        (int)TRANSPOSE_NONE,cov);
                    for(k=1;k<=tcs->num_tc;k++) for(m=1;m<=tcs->num_tc;m++,l++) subcov[l] = cov[k][m];

                    #if 0
                    printf("subject index j=%d  region index i=%d cov=\n",j,i);
                    for(k=1;k<=tcs->num_tc;k++) {
                        for(m=1;m<=tcs->num_tc;m++) printf("%f ",cov[k][m]);
                        printf("\n");
                        }
                    #endif
    
                    }
                free_dmatrix(ATVAxtransformT,1,glm->ifh->glm_Mcol,1,tcs->num_tc);
                free_dmatrix(ATVA,1,glm->ifh->glm_Mcol,1,glm->ifh->glm_Mcol);
                free_dmatrix(VA,1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
                for(i=0;i<fs->nfiles;i++) if(!(unmap_disk(mm[i]))) return 0;
                free(mm);
                free(area);
                free(length);
                free_files_struct(fs);
                }
            free_dmatrix(A,1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
            free_dmatrix(ATA,1,glm->ifh->glm_Mcol,1,glm->ifh->glm_Mcol);
            free_dmatrix(transform,1,ntc_each,1,glm->ifh->glm_Mcol);
            free_dmatrix(ATAxtransformT,1,glm->ifh->glm_Mcol,1,ntc_each);
            free_glm(glm,(int)TRUE);
            }
        }
    if(Vconc) {
        free_dmatrix(cov,1,tcs->num_tc,1,tcs->num_tc);
        free_dmatrix(ATAm1xtransformed_ATVA,1,tcs->num_tc,1,tcs->num_tc);
        free_dmatrix(transformed_ATVA,1,tcs->num_tc,1,tcs->num_tc);
        }
    free_dmatrix(ATAm1,1,tcs->num_tc,1,tcs->num_tc);
    free_dmatrix(transformed_ATA,1,tcs->num_tc,1,tcs->num_tc);
    return 1;
}
#endif

#if 0
int subcov_reduced_ATAgsl(char **glm_files,int nsub,int *nglmpersub,TC *tcs,double *subcov,int SunOS_Linux,int tc_contrast,
    Files_Struct *Vconc)
{
    int i,j,k,l,m,n,o,p,q,qq,m1,n1,oo,oo1,total_length,nreg,aadvance,nextreg,nextrow,*length,*area,nglms_each,ntc_each,a1,a2;
    double **A,**ATA,**ATAxtransformT,**transformed_ATA,**avg_transformed_ATA,**ATAm1,**transform,denominator,
        **transformed_ATVA,**ATAm1xtransformed_ATVA,**cov,**VA,**ATVA,**ATVAxtransformT;
    LinearModel *glm;
    Interfile_header *ifh;
    Files_Struct *fs;
    Memory_Map **mm;
    printf("************** subcov_reduced_ATA *********************\n");

    #if 0
    for(i=0;i<tcs->num_tc;i++) {
        printf("i=%d\n",i);
        for(j=0;j<tcs->each[i];j++) printf("%d ",(int)tcs->tc[i][j][0]); 
        printf("\n");
        }
    #endif

    nglms_each = num_glm_files/tcs->each[0];
    ntc_each = tcs->num_tc/nglms_each;

    printf("tcs->num_tc=%d nglms_each=%d ntc_each=%d\n",tcs->num_tc,nglms_each,ntc_each);

    transformed_ATA = dmatrix(1,ntc_each,1,ntc_each);

    printf("here100\n");
    fflush(stdout);

    ATAm1 = dmatrix(1,ntc_each,1,ntc_each);
    if(Vconc) {
        if(nglms_each!=1) {
            printf("Error: This segment of code needs to be rewritten to handle more than one glm per subject.\n");
            return 0;
            }
        transformed_ATVA = dmatrix(1,tcs->num_tc,1,tcs->num_tc);
        ATAm1xtransformed_ATVA = dmatrix(1,tcs->num_tc,1,tcs->num_tc);
        cov = dmatrix(1,tcs->num_tc,1,tcs->num_tc);
        }

    for(l=j=p=0;p<tcs->each[0];p++) {
        for(qq=q=0;q<nglms_each;q++,j++) {

            if(!(glm = read_glm(glm_files[j],1,SunOS_Linux))) {
                printf("Error: subcov_reduced_ATA -> read_glm\n");
                return 0;
                }
            A = dmatrix(1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
            for(i=1;i<=glm->ifh->glm_Nrow;i++) for(k=1;k<=glm->ifh->glm_Mcol;k++) A[i][k] = (double)glm->A[i][k];
            ATA = dmatrix_mult(A,A,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,(int)TRANSPOSE_FIRST);

            /*printf("here1\n");
            fflush(stdout);*/

            /*printf("ATA\n");
            for(i=1;i<=glm->ifh->glm_Mcol;i++) {
                for(k=1;k<=glm->ifh->glm_Mcol;k++) printf("%f ",ATA[i][k]);
                printf("\n");
                }*/

            transform = dmatrix_0(1,ntc_each,1,glm->ifh->glm_Mcol);
            if(!tc_contrast) {
                for(i=0;i<ntc_each;i++,qq++) {
                    if(tcs->num_tc_to_sum[qq][p] == 1) {
                        transform[i+1][(int)tcs->tc[qq][p][0]] = 1;
                        }
                    else {
                        for(denominator=k=0;k<tcs->num_tc_to_sum[qq][p];k++)
                            denominator += 1./ATA[(int)tcs->tc[qq][p][k]][(int)tcs->tc[qq][p][k]];
                        for(k=0;k<tcs->num_tc_to_sum[qq][p];k++) transform[i+1][(int)tcs->tc[qq][p][k]] =
                            (1./ATA[(int)tcs->tc[qq][p][k]][(int)tcs->tc[qq][p][k]])/denominator;
                        }
                     }
                }
            else {
                for(i=0;i<ntc_each;i++,qq++)
                    for(k=0;k<glm->ifh->glm_Mcol;k++)
                        transform[i+1][k+1] = glm->c[((int)tcs->tc[qq][p][0]-1)*glm->ifh->glm_Mcol+k];
                }
            ATAxtransformT = dmatrix_mult(ATA,transform,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,ntc_each,glm->ifh->glm_Mcol,
                (int)TRANSPOSE_SECOND);
            dmatrix_mult_nomem(transform,ATAxtransformT,ntc_each,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,ntc_each,(int)TRANSPOSE_NONE,
                transformed_ATA);
            inverse_cover(transformed_ATA,ATAm1,ntc_each,(double)B_THRESHOLD,"transformed_ATA");

            /*printf("here2\n");
            fflush(stdout);*/

            if(!Vconc) {
                /*printf("ATAm1\n");*/
                a1 = q*ntc_each; 
                a2 = (nglms_each-q-1)*ntc_each;
                for(i=1;i<=ntc_each;i++,l+=a2) {
                    for(l+=a1,k=1;k<=ntc_each;k++,l++) {
                        subcov[l] = ATAm1[i][k];
                        /*printf("%f ",ATAm1[i][k]);*/
                        /*printf("i=%d k=%d l=%d\n",i,k,l);*/
                        }
                    /*printf("\n");*/
                    }
                }
            else {
                printf("Using data %s\n",Vconc->files[j]);
                VA = dmatrix(1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
                ATVA = dmatrix(1,glm->ifh->glm_Mcol,1,glm->ifh->glm_Mcol);
                ATVAxtransformT = dmatrix(1,glm->ifh->glm_Mcol,1,tcs->num_tc);
                if(!(fs=read_conc(Vconc->files[j]))) return 0;
                if(!(length=malloc(sizeof*length*fs->nfiles))) {
                    printf("Error: Unable to malloc length\n");
                    exit(-1);
                    }
                if(!(area=malloc(sizeof*area*fs->nfiles))) {
                    printf("Error: Unable to malloc area\n");
                    exit(-1);
                    }
                for(total_length=i=0;i<fs->nfiles;i++) {
                    if(!(ifh=read_ifh(fs->files[i](Interfile_header*)NULL))) return 0;
                    length[i] = ifh->dim1;
                    area[i] = length[i]*length[i];
                    total_length += ifh->dim1;
                    if(ifh->dim2 != ifh->dim1) {
                        printf("Error: ifh->dim2 = %d ifh->dim1 = %d They must be equal. Abort!\n",ifh->dim2,ifh->dim1);
                        return 0;
                        }
                    if(!i && !j) {
                        nreg = ifh->dim3;
                        }
                    else if(ifh->dim3 != nreg) {
                        printf("Error: ifh->dim3=%d nreg=%d  Must be equal. Abort!\n",ifh->dim3,nreg);
                        return 0;
                        }
                    if(ifh->dim4 != 1) {
                        printf("Error: ifh->dim4 = %d. It must equal 1. Abort!\n",ifh->dim4);
                        return 0;
                        }
                    free_ifh(ifh,(int)FALSE);
                    }
                if(total_length != glm->ifh->glm_Nrow) {
                    printf("Error: total_length=%d glm->ifh->glm_Nrow=%d  Must be equal. Abort!\n",total_length,glm->ifh->glm_Nrow);
                    return 0;
                    }
                if(!(mm=malloc(sizeof*mm*fs->nfiles))) {
                    printf("Error: Unable to malloc mm\n");
                    return 0;
                    }
                for(i=0;i<fs->nfiles;i++) if(!(mm[i]=map_disk(fs->files[i],area[i]*nreg,0))) return 0;
                for(i=0;i<nreg;i++) {
                    if(mm[0]->ptr[i*area[0]]==(float)UNSAMPLED_VOXEL) {
                        l += tcs->num_tc*tcs->num_tc;
                        continue;
                        }
                    for(m1=aadvance=1,k=0;k<fs->nfiles;k++,aadvance+=length[k]) {
                        nextreg = i*area[k];
                        for(nextrow=0,m=0;m<length[k];m++,m1++,nextrow+=length[k]) {
                            for(n1=1,n=0;n<glm->ifh->glm_Mcol;n++,n1++) {
                                VA[m1][n1]=0.;
                                for(oo1=aadvance,oo=nextreg+nextrow,o=0;o<length[k];o++,oo++,oo1++) {
                                    VA[m1][n1] += (double)mm[k]->ptr[oo]*A[oo1][n1];
                                    /*printf("mm[%d]->ptr[%d]=%f A[%d][%d]=%f VA[%d][%d]=%f\n",
                                        k,oo,mm[k]->ptr[oo],oo1,n1,A[oo1][n1],m1,n1,VA[m1][n1]);*/
                                    }
                                /*printf("VA[%d][%d]=%f\n",m1,n1,VA[m1][n1]);*/
                                }
                            }
                        }

                    #if 0
                    printf("VA=\n");
                    for(k=1;k<=glm->ifh->glm_Nrow;k++) {
                        for(m=1;m<=glm->ifh->glm_Mcol;m++) printf("%f ",VA[k][m]);
                        printf("\n");
                        exit(-1);
                        }
                    exit(-1);
                    #endif
    
                    dmatrix_mult_nomem(A,VA,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,
                        (int)TRANSPOSE_FIRST,ATVA);
    
                    #if 0
                    printf("ATVA=\n");
                    for(k=1;k<=glm->ifh->glm_Mcol;k++) {
                        for(m=1;m<=glm->ifh->glm_Mcol;m++) printf("%f ",ATVA[k][m]);
                        printf("\n");
                        }
                    exit(-1);
                    #endif

                    dmatrix_mult_nomem(ATVA,transform,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,glm->ifh->glm_Mcol,
                        (int)TRANSPOSE_SECOND,ATVAxtransformT);
                    dmatrix_mult_nomem(transform,ATVAxtransformT,tcs->num_tc,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,
                        (int)TRANSPOSE_NONE,transformed_ATVA);
                    dmatrix_mult_nomem(ATAm1,transformed_ATVA,tcs->num_tc,tcs->num_tc,tcs->num_tc,tcs->num_tc,
                        (int)TRANSPOSE_NONE,ATAm1xtransformed_ATVA);
                    dmatrix_mult_nomem(ATAm1xtransformed_ATVA,ATAm1,tcs->num_tc,tcs->num_tc,tcs->num_tc,tcs->num_tc,
                        (int)TRANSPOSE_NONE,cov);
                    for(k=1;k<=tcs->num_tc;k++) for(m=1;m<=tcs->num_tc;m++,l++) subcov[l] = cov[k][m];

                    #if 0
                    printf("subject index j=%d  region index i=%d cov=\n",j,i);
                    for(k=1;k<=tcs->num_tc;k++) {
                        for(m=1;m<=tcs->num_tc;m++) printf("%f ",cov[k][m]);
                        printf("\n");
                        }
                    #endif
    
                    }
                free_dmatrix(ATVAxtransformT,1,glm->ifh->glm_Mcol,1,tcs->num_tc);
                free_dmatrix(ATVA,1,glm->ifh->glm_Mcol,1,glm->ifh->glm_Mcol);
                free_dmatrix(VA,1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
                for(i=0;i<fs->nfiles;i++) if(!(unmap_disk(mm[i]))) return 0;
                free(mm);
                free(area);
                free(length);
                free_files_struct(fs);
                }
            free_dmatrix(A,1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
            free_dmatrix(ATA,1,glm->ifh->glm_Mcol,1,glm->ifh->glm_Mcol);
            free_dmatrix(transform,1,ntc_each,1,glm->ifh->glm_Mcol);
            free_dmatrix(ATAxtransformT,1,glm->ifh->glm_Mcol,1,ntc_each);
            free_glm(glm,(int)TRUE);
            }
        }
    if(Vconc) {
        free_dmatrix(cov,1,tcs->num_tc,1,tcs->num_tc);
        free_dmatrix(ATAm1xtransformed_ATVA,1,tcs->num_tc,1,tcs->num_tc);
        free_dmatrix(transformed_ATVA,1,tcs->num_tc,1,tcs->num_tc);
        }
    free_dmatrix(ATAm1,1,tcs->num_tc,1,tcs->num_tc);
    free_dmatrix(transformed_ATA,1,tcs->num_tc,1,tcs->num_tc);
    return 1;
}
#endif












































/*START140807*/
#if 0
void get_transform(TC *tcs,int tc_contrast,int j,MAT *ATA,int Mcol,float *c,MAT *transform)
{
    int i,k,index;
    double denominator;

    if(!tc_contrast) {
        for(i=0;i<tcs->num_tc;i++) {
            if(tcs->num_tc_to_sum[i][j] == 1)
                transform->me[i][(int)tcs->tc[i][j][0]-1] = 1;
            else {
                for(denominator=k=0;k<tcs->num_tc_to_sum[i][j];k++) {
                    index = (int)tcs->tc[i][j][k]-1;
                    denominator += 1/ATA->me[index][index];
                    }
                for(k=0;k<tcs->num_tc_to_sum[i][j];k++) {
                    index = (int)tcs->tc[i][j][k]-1;
                    transform->me[i][index] = (1/ATA->me[index][index])/denominator;
                    }
                }
             }
        }
    else {
        for(i=0;i<tcs->num_tc;i++) {
            index = ((int)tcs->tc[i][j][0]-1)*Mcol;
            for(k=0;k<Mcol;k++) transform->me[i][k] = (double)c[index+k];
            }
        }
}
int subcov_ATAm1(char **glm_files,int num_glm_files,TC *tcs,double *subcov,int swapbytes,int tc_contrast,Files_Struct *Vconc)
{
    int i,j,k,l,m,n,o,m1,n1,oo,oo1,total_length,nreg,aadvance,nextreg,nextrow,*length,*area;
    LinearModel *glm;
    Interfile_header *ifh;
    Files_Struct *fs;
    Memory_Map **mm;
    MAT *transformed_ATAm1,*transformed_ATVA,*transformed_ATAm1xtransformed_ATVA,*cov,*ATAm1,*ATAm1xtransformT,*A,*VA,*ATVA,
        *ATVAxtransformT,*transform,*ATA;

    printf("************** subcov_ATAm1 *********************\n");
    transformed_ATAm1 = m_get(tcs->num_tc,tcs->num_tc);
    if(Vconc) {
        transformed_ATVA = m_get(tcs->num_tc,tcs->num_tc);
        transformed_ATAm1xtransformed_ATVA = m_get(tcs->num_tc,tcs->num_tc);
        cov = m_get(tcs->num_tc,tcs->num_tc);
        }
    for(l=j=0;j<num_glm_files;j++) {
        if(!(glm = read_glm(glm_files[j],1,swapbytes))) {
            printf("Error: subcov_reduced_ATA -> read_glm\n");
            return 0;
            }
        A = m_get(glm->ifh->glm_Nrow,glm->ifh->glm_Mcol);
        ATA = m_get(glm->ifh->glm_Mcol,glm->ifh->glm_Mcol);
        ATAm1 = m_get(glm->ifh->glm_Mcol,glm->ifh->glm_Mcol);
        transform = m_get(tcs->num_tc,glm->ifh->glm_Mcol);
        ATAm1xtransformT = m_get(glm->ifh->glm_Mcol,tcs->num_tc);
        for(i=0;i<glm->ifh->glm_Nrow;i++)for(k=0;k<glm->ifh->glm_Mcol;k++)A->me[i][k]=(double)glm->AT[k][i];
        mtrm_mlt(A,A,ATA);
        m_inverse(ATA,ATAm1);
        get_transform(tcs,tc_contrast,j,ATAm1,glm->ifh->glm_Mcol,glm->c,transform);
        mmtr_mlt(ATAm1,transform,ATAm1xtransformT);
        m_mlt(transform,ATAm1xtransformT,transformed_ATAm1);
        if(!Vconc) {
            for(i=0;i<tcs->num_tc;i++) for(k=0;k<tcs->num_tc;k++,l++) subcov[l] = transformed_ATAm1->me[i][k];
            }
        else {
            printf("Using data %s\n",Vconc->files[j]); 
            VA = m_get(glm->ifh->glm_Nrow,glm->ifh->glm_Mcol);
            ATVA = m_get(glm->ifh->glm_Mcol,glm->ifh->glm_Mcol);
            ATVAxtransformT = m_get(glm->ifh->glm_Mcol,tcs->num_tc);
            if(!(fs=read_conc(1,&Vconc->files[j]))) return 0;
            if(!(length=malloc(sizeof*length*fs->nfiles))) {
                printf("Error: Unable to malloc length\n");
                exit(-1);
                }
            if(!(area=malloc(sizeof*area*fs->nfiles))) {
                printf("Error: Unable to malloc area\n");
                exit(-1);
                }
            for(total_length=i=0;i<fs->nfiles;i++) {
                if(!(ifh=read_ifh(fs->files[i],(Interfile_header*)NULL))) return 0;
                length[i] = ifh->dim1;
                area[i] = length[i]*length[i];
                total_length += ifh->dim1;
                if(ifh->dim2 != ifh->dim1) {
                    printf("Error: ifh->dim2 = %d ifh->dim1 = %d They must be equal. Abort!\n",ifh->dim2,ifh->dim1);
                    return 0;
                    }
                if(!i && !j) {
                    nreg = ifh->dim3;
                    }
                else if(ifh->dim3 != nreg) {
                    printf("Error: ifh->dim3=%d nreg=%d  Must be equal. Abort!\n",ifh->dim3,nreg);
                    return 0;
                    }
                if(ifh->dim4 != 1) {
                    printf("Error: ifh->dim4 = %d. It must equal 1. Abort!\n",ifh->dim4);
                    return 0;
                    }
                free_ifh(ifh,(int)FALSE);
                }
            if(total_length != glm->ifh->glm_Nrow) {
                printf("Error: total_length=%d glm->ifh->glm_Nrow=%d  Must be equal. Abort!\n",total_length,glm->ifh->glm_Nrow);
                return 0;
                }
            if(!(mm=malloc(sizeof*mm*fs->nfiles))) {
                printf("Error: Unable to malloc mm\n");
                return 0;
                }
            for(i=0;i<fs->nfiles;i++) if(!(mm[i]=map_disk(fs->files[i],area[i]*nreg,0,sizeof(float)))) return 0;
            for(i=0;i<nreg;i++) {
                if(mm[0]->ptr[i*area[0]]==(float)UNSAMPLED_VOXEL) {
                    l += tcs->num_tc*tcs->num_tc;
                    continue;
                    }
                for(m1=aadvance=0,k=0;k<fs->nfiles;k++,aadvance+=length[k]) {
                    nextreg = i*area[k];
                    for(nextrow=0,m=0;m<length[k];m++,m1++,nextrow+=length[k]) {
                        for(n1=0,n=0;n<glm->ifh->glm_Mcol;n++,n1++) {
                            VA->me[m1][n1]=0.;
                            for(oo1=aadvance,oo=nextreg+nextrow,o=0;o<length[k];o++,oo++,oo1++) {
                                VA->me[m1][n1] += (double)mm[k]->ptr[oo]*A->me[oo1][n1];
                                }
                            }
                        }
                    }
                mtrm_mlt(A,VA,ATVA);
                mmtr_mlt(ATVA,transform,ATVAxtransformT);
                m_mlt(transform,ATVAxtransformT,transformed_ATVA);
                m_mlt(transformed_ATAm1,transformed_ATVA,transformed_ATAm1xtransformed_ATVA);
                m_mlt(transformed_ATAm1xtransformed_ATVA,transformed_ATAm1,cov);        
                for(k=0;k<tcs->num_tc;k++) for(m=0;m<=tcs->num_tc;m++,l++) subcov[l] = cov->me[k][m];
                }
            for(i=0;i<fs->nfiles;i++) if(!(unmap_disk(mm[i]))) return 0;
            free(mm);
            free(area);
            free(length);
            free_files_struct(fs);
            if(m_free(ATVAxtransformT)) {
                printf("Error: m_free(ATVAxtransformT)\n");
                return 0;
                }
            if(m_free(ATVA)) {
                printf("Error: m_free(ATVA)\n");
                return 0;
                }
            if(m_free(VA)) {
                printf("Error: m_free(VA)\n");
                return 0;
                }
            }
        if(m_free(ATAm1xtransformT)) {
            printf("Error: m_free(ATAm1xtransformT)\n");
            return 0;
            }
        if(m_free(transform)) {
            printf("Error: m_free(transform)\n");
            return 0;
            }
        if(m_free(ATAm1)) {
            printf("Error: m_free(ATAm1)\n");
            return 0;
            }
        if(m_free(ATA)) {
            printf("Error: m_free(ATA)\n");
            return 0;
            }
        if(m_free(A)) {
            printf("Error: m_free(A)\n");
            return 0;
            }
        free_glm(glm,(int)TRUE);
        }
    if(Vconc) {
        if(m_free(cov)) {
            printf("Error: m_free(cov)\n");
            return 0;
            }
        if(m_free(transformed_ATAm1xtransformed_ATVA)) {
            printf("Error: m_free(transformed_ATAm1xtransformed_ATVA)\n");
            return 0;
            }
        if(m_free(transformed_ATVA)) {
            printf("Error: m_free(transformed_ATVA)\n");
            return 0;
            }
        }
    if(m_free(transformed_ATAm1)) {
        printf("Error: m_free(transformed_ATAm1)\n");
        return 0;
        }
    return 1;
}
#endif






#if 0
/*************************************************************************/
double **cov_tc_avg_reduced_ATA(char **glm_files,int num_glm_files,TC *tcs,int swapbytes)
/*************************************************************************/
{
int i,j,k;

double **A,**ATA,**ATAxtransformT,**transformed_ATA,**avg_transformed_ATA,**ATAm1,**transform,denominator,min,max,cond;

LinearModel *glm;

avg_transformed_ATA = (double **)dmatrix_0(1,tcs->num_tc,1,tcs->num_tc);
for(j=0;j<num_glm_files;j++) {
    if(!(glm = read_glm(glm_files[j],1,swapbytes))) {
        printf("Error: cov_tc_avg_reduced_ATA -> read_glm\n");
        return NULL;
        }
    A = dmatrix(1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);

    /*for(i=1;i<=glm->ifh->glm_Nrow;i++) for(k=1;k<=glm->ifh->glm_Mcol;k++) A[i][k] = (double)glm->A[i][k];*/
    /*START61*/
    for(i=0;i<glm->ifh->glm_Nrow;i++) for(k=0;k<glm->ifh->glm_Mcol;k++) A[i+1][k+1] = (double)glm->AT[k][i];

    ATA = dmatrix_mult(A,A,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,(int)TRANSPOSE_FIRST);
    transform = (double **)dmatrix_0(1,tcs->num_tc,1,glm->ifh->glm_Mcol);
    for(i=0;i<tcs->num_tc;i++) {
        if(tcs->num_tc_to_sum[i][j] == 1)
            transform[i+1][(int)tcs->tc[i][j][0]] = 1;
        else {
            for(denominator=k=0;k<tcs->num_tc_to_sum[i][j];k++)
                denominator += 1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]];
            for(k=0;k<tcs->num_tc_to_sum[i][j];k++)
                transform[i+1][(int)tcs->tc[i][j][k]] = (1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]])/denominator;
            }
         }
    ATAxtransformT = dmatrix_mult(ATA,transform,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,glm->ifh->glm_Mcol,(int)TRANSPOSE_SECOND);
    transformed_ATA = dmatrix_mult(transform,ATAxtransformT,tcs->num_tc,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,(int)TRANSPOSE_NONE);
    for(i=1;i<=tcs->num_tc;i++) for(k=1;k<=tcs->num_tc;k++) avg_transformed_ATA[i][k] += transformed_ATA[i][k];

    free_dmatrix(A,1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
    free_dmatrix(ATA,1,glm->ifh->glm_Mcol,1,glm->ifh->glm_Mcol);
    free_dmatrix(transform,1,tcs->num_tc,1,glm->ifh->glm_Mcol);
    free_dmatrix(ATAxtransformT,1,glm->ifh->glm_Mcol,1,tcs->num_tc);
    free_dmatrix(transformed_ATA,1,tcs->num_tc,1,tcs->num_tc);
    free_glm(glm,(int)TRUE);
    }
for(i=1;i<=tcs->num_tc;i++)
    for(k=1;k<=tcs->num_tc;k++)
        avg_transformed_ATA[i][k] /= num_glm_files;



#if 1
ATAm1 = dmatrix(1,tcs->num_tc,1,tcs->num_tc);
/*inverse_cover(avg_transformed_ATA,ATAm1,tcs->num_tc,"avg_transformed_ATA",(double)B_THRESHOLD);*/
inverse_cover(avg_transformed_ATA,ATAm1,tcs->num_tc,(double)B_THRESHOLD,"avg_transformed_ATA");
#endif

#if 0
printf("ATAm1\n");
for(i=1;i<=tcs->num_tc;i++) {
    for(j=1;j<=tcs->num_tc;j++) printf("%f ",ATAm1[i][j]);
    printf("\n");
    }
printf("\n");
#endif


free_dmatrix(avg_transformed_ATA,1,tcs->num_tc,1,tcs->num_tc);
return ATAm1;
}
#endif

#if 0
/**********************************************************************************/
int subcov_tc_reduced_ATA(char **glm_files,int num_glm_files,TC *tcs,double *subcov,int swapbytes)
/**********************************************************************************/
{
int i,j,k,l;
double **A,**ATA,**ATAxtransformT,**transformed_ATA,**avg_transformed_ATA,**ATAm1,**transform,denominator,min,max,cond;
LinearModel *glm;

printf("*********************** subcov_tc_reduced_ATA **************************\n");

for(l=j=0;j<num_glm_files;j++) {
    if(!(glm = read_glm(glm_files[j],1,swapbytes))) {
        printf("Error: subcov_tc_reduced_ATA -> read_glm\n");
        return 0;
        }
    A = dmatrix(1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);

    /*for(i=1;i<=glm->ifh->glm_Nrow;i++) for(k=1;k<=glm->ifh->glm_Mcol;k++) A[i][k] = (double)glm->A[i][k];*/
    /*START61*/
    for(i=0;i<glm->ifh->glm_Nrow;i++) for(k=0;k<glm->ifh->glm_Mcol;k++) A[i+1][k+1] = (double)glm->AT[k][i];

    ATA = dmatrix_mult(A,A,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,
        (int)TRANSPOSE_FIRST);
    transform = (double **)dmatrix_0(1,tcs->num_tc,1,glm->ifh->glm_Mcol);
    for(i=0;i<tcs->num_tc;i++) {
        if(tcs->num_tc_to_sum[i][j] == 1)
            transform[i+1][(int)tcs->tc[i][j][0]] = 1;
        else {
            for(denominator=k=0;k<tcs->num_tc_to_sum[i][j];k++)
                denominator += 1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]];
            for(k=0;k<tcs->num_tc_to_sum[i][j];k++)
                transform[i+1][(int)tcs->tc[i][j][k]] = (1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]])/denominator;
            }
         }
    ATAxtransformT = dmatrix_mult(ATA,transform,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,glm->ifh->glm_Mcol,(int)TRANSPOSE_SECOND);
    transformed_ATA = dmatrix_mult(transform,ATAxtransformT,tcs->num_tc,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,(int)TRANSPOSE_NONE);



    ATAm1 = dmatrix(1,tcs->num_tc,1,tcs->num_tc);
    /*inverse_cover(transformed_ATA,ATAm1,tcs->num_tc,"transformed_ATA",(double)B_THRESHOLD);*/
    inverse_cover(transformed_ATA,ATAm1,tcs->num_tc,(double)B_THRESHOLD,"transformed_ATA");





    for(i=1;i<=tcs->num_tc;i++) for(k=1;k<=tcs->num_tc;k++,l++) subcov[l] = ATAm1[i][k];
    free_dmatrix(A,1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
    free_dmatrix(ATA,1,glm->ifh->glm_Mcol,1,glm->ifh->glm_Mcol);
    free_dmatrix(transform,1,tcs->num_tc,1,glm->ifh->glm_Mcol);
    free_dmatrix(ATAxtransformT,1,glm->ifh->glm_Mcol,1,tcs->num_tc);
    free_dmatrix(transformed_ATA,1,tcs->num_tc,1,tcs->num_tc);
    free_dmatrix(ATAm1,1,tcs->num_tc,1,tcs->num_tc);
    free_glm(glm,(int)TRUE);
    }
return 1;
}
#endif

#if 0
int subcov_reduced_ATA_V(char **glm_files,int num_glm_files,TC *tcs,double *subcov,int swapbytes,int tc_contrast,Files_Struct *V_files)
{
int i,j,k,l,m,vol,SunOS_Linux;
float *temp_float;
double **A,**ATA,**ATAxtransformT,**transformed_ATA,**avg_transformed_ATA,**ATAm1,**transform,denominator,min,max,cond,**V,**VA,
       **ATVA,**ATVAxtransformT,**transformed_ATVA,**ATAm1xtransformed_ATVA,**cov;
LinearModel *glm;
Interfile_header *ifh;

if((SunOS_Linux=checkOS())==-1) return 0;
for(l=j=0;j<num_glm_files;j++) {

    if(!(ifh=read_ifh(V_files->files[j],(Interfile_header*)NULL))) return 0;
    vol = ifh->dim1*ifh->dim2;
    if(!(temp_float=malloc(sizeof*temp_float*vol))) {
        printf("Error: Unable to malloc temp_float in subcov_tc_reduced_ATA_V\n");
        return 0;
        }
    V = dmatrix(1,ifh->dim1,1,ifh->dim2);
    printf("Reading %s\n",V_files->files[j]);
    if(!readstack(V_files->files[j],(float*)temp_float,sizeof(float),(size_t)vol,SunOS_Linux)) return 0;
    for(m=0,i=1;i<=ifh->dim1;i++) for(k=1;k<=ifh->dim2;k++,m++) V[i][k] = (double)temp_float[m];

    if(!(glm = read_glm(glm_files[j],1,swapbytes))) {
        printf("Error: subcov_tc_reduced_ATA -> read_glm\n");
        return 0;
        }
    A = dmatrix(1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);

    /*for(i=1;i<=glm->ifh->glm_Nrow;i++) for(k=1;k<=glm->ifh->glm_Mcol;k++) A[i][k] = glm->A[i][k];*/
    /*START61*/
    for(i=0;i<glm->ifh->glm_Nrow;i++) for(k=0;k<glm->ifh->glm_Mcol;k++) A[i+1][k+1] = glm->AT[k][i];

    ATA = dmatrix_mult(A,A,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,
        (int)TRANSPOSE_FIRST);
    transform = dmatrix_0(1,tcs->num_tc,1,glm->ifh->glm_Mcol);



    #if 0
    for(i=0;i<tcs->num_tc;i++) {
        if(tcs->num_tc_to_sum[i][j] == 1)
            transform[i+1][(int)tcs->tc[i][j][0]] = 1;
        else {
            for(denominator=k=0;k<tcs->num_tc_to_sum[i][j];k++)
                denominator += 1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]];
            for(k=0;k<tcs->num_tc_to_sum[i][j];k++)
                transform[i+1][(int)tcs->tc[i][j][k]] = (1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]])/denominator;
            }
         }
    #endif

    if(!tc_contrast) {
        for(i=0;i<tcs->num_tc;i++) {
            if(tcs->num_tc_to_sum[i][j] == 1)
                transform[i+1][(int)tcs->tc[i][j][0]] = 1;
            else {
                for(denominator=k=0;k<tcs->num_tc_to_sum[i][j];k++)
                    denominator += 1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]];
                for(k=0;k<tcs->num_tc_to_sum[i][j];k++)
                    transform[i+1][(int)tcs->tc[i][j][k]] = (1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]])/denominator;
                }
             }
        }
    else {
        for(i=0;i<tcs->num_tc;i++)
            for(k=0;k<glm->ifh->glm_Mcol;k++)
                transform[i+1][k+1] = glm->c[((int)tcs->tc[i][j][0]-1)*glm->ifh->glm_Mcol+k];
        }









    ATAxtransformT = dmatrix_mult(ATA,transform,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,glm->ifh->glm_Mcol,(int)TRANSPOSE_SECOND);
    transformed_ATA = dmatrix_mult(transform,ATAxtransformT,tcs->num_tc,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,(int)TRANSPOSE_NONE);

    VA = dmatrix_mult(V,A,ifh->dim1,ifh->dim2,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,(int)TRANSPOSE_NONE);

                #if 0
                printf("VA=\n");
                for(k=1;k<=glm->ifh->glm_Nrow;k++) {
                    for(m=1;m<=glm->ifh->glm_Mcol;m++) printf("%f ",VA[k][m]);
                    printf("\n");
                    exit(-1);
                    }
                exit(-1);
                #endif


    ATVA = dmatrix_mult(A,VA,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,ifh->dim1,glm->ifh->glm_Mcol,(int)TRANSPOSE_FIRST);
    ATVAxtransformT = dmatrix_mult(ATVA,transform,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,glm->ifh->glm_Mcol,(int)TRANSPOSE_SECOND);
    transformed_ATVA = dmatrix_mult(transform,ATVAxtransformT,tcs->num_tc,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,(int)TRANSPOSE_NONE);




    ATAm1 = dmatrix(1,tcs->num_tc,1,tcs->num_tc);
    inverse_cover(transformed_ATA,ATAm1,tcs->num_tc,(double)B_THRESHOLD,"transformed_ATA");





    ATAm1xtransformed_ATVA=dmatrix_mult(ATAm1,transformed_ATVA,tcs->num_tc,tcs->num_tc,tcs->num_tc,tcs->num_tc,(int)TRANSPOSE_NONE);
    cov=dmatrix_mult(ATAm1xtransformed_ATVA,ATAm1,tcs->num_tc,tcs->num_tc,tcs->num_tc,tcs->num_tc,(int)TRANSPOSE_NONE); 
    for(i=1;i<=tcs->num_tc;i++) for(k=1;k<=tcs->num_tc;k++,l++) subcov[l] = cov[i][k];

                #if 1
                printf("cov=\n");
                for(i=1;i<=tcs->num_tc;i++) {
                    for(k=1;k<=tcs->num_tc;k++) printf("%f ",cov[i][k]);
                    printf("\n");
                    }
                #endif





    free_dmatrix(A,1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
    free_dmatrix(ATA,1,glm->ifh->glm_Mcol,1,glm->ifh->glm_Mcol);
    free_dmatrix(transform,1,tcs->num_tc,1,glm->ifh->glm_Mcol);
    free_dmatrix(ATAxtransformT,1,glm->ifh->glm_Mcol,1,tcs->num_tc);
    free_dmatrix(transformed_ATA,1,tcs->num_tc,1,tcs->num_tc);
    free_dmatrix(ATAm1,1,tcs->num_tc,1,tcs->num_tc);
    free_glm(glm,(int)TRUE);

    free_ifh(ifh,(int)FALSE);
    free(temp_float);
    free_dmatrix(V,1,ifh->dim1,1,ifh->dim2);
    free_dmatrix(VA,1,ifh->dim1,1,glm->ifh->glm_Mcol);
    free_dmatrix(ATVA,1,glm->ifh->glm_Mcol,1,glm->ifh->glm_Mcol);
    free_dmatrix(ATVAxtransformT,1,glm->ifh->glm_Mcol,1,tcs->num_tc);
    free_dmatrix(transformed_ATVA,1,tcs->num_tc,1,tcs->num_tc);
    free_dmatrix(ATAm1xtransformed_ATVA,1,tcs->num_tc,1,tcs->num_tc);
    free_dmatrix(cov,1,tcs->num_tc,1,tcs->num_tc);
    }
return 1;
}
#endif

#if 0
/***************************************************************************************************/
double **cov_mag_avg_reduced_ATA(char **glm_files,int num_glm_files,int **contrast,int num_contrasts,int swapbytes)
/***************************************************************************************************/
{ 
int i,j,k;

double **A,**ATA,**ATAxtransformT,**transformed_ATA,**avg_transformed_ATA,**ATAm1,**transform,denominator,min,max,cond;

LinearModel *glm;

avg_transformed_ATA = (double **)dmatrix_0(1,num_contrasts,1,num_contrasts);

for(j=0;j<num_glm_files;j++) {
    glm = read_glm(glm_files[j],1,swapbytes);
    A = (double **)dmatrix(1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);

    /*for(i=1;i<=glm->ifh->glm_Nrow;i++) for(k=1;k<=glm->ifh->glm_Mcol;k++) A[i][k] = (double)glm->A[i][k];*/
    /*START61*/
    for(i=0;i<glm->ifh->glm_Nrow;i++) for(k=0;k<glm->ifh->glm_Mcol;k++) A[i+1][k+1] = (double)glm->AT[k][i];

    ATA = dmatrix_mult(A,A,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,(int)TRANSPOSE_FIRST);
    transform = (double **)dmatrix(1,num_contrasts,1,glm->ifh->glm_Mcol);
    for(i=0;i<num_contrasts;i++)
        for(k=0;k<glm->ifh->glm_Mcol;k++)
            transform[i+1][k+1] = glm->c[(contrast[i][j]-1)*glm->ifh->glm_Mcol+k];
    ATAxtransformT = dmatrix_mult(ATA,transform,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,num_contrasts,glm->ifh->glm_Mcol,(int)TRANSPOSE_SECOND);
    transformed_ATA = dmatrix_mult(transform,ATAxtransformT,num_contrasts,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,num_contrasts,
                      (int)TRANSPOSE_NONE);
    for(i=1;i<=num_contrasts;i++)
        for(k=1;k<=num_contrasts;k++)
            avg_transformed_ATA[i][k] += transformed_ATA[i][k];

    free_dmatrix(A,1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
    free_dmatrix(ATA,1,glm->ifh->glm_Mcol,1,glm->ifh->glm_Mcol);
    free_dmatrix(transform,1,num_contrasts,1,glm->ifh->glm_Mcol);
    free_dmatrix(ATAxtransformT,1,glm->ifh->glm_Mcol,1,num_contrasts);
    free_dmatrix(transformed_ATA,1,num_contrasts,1,num_contrasts);
    free_glm(glm,(int)TRUE);
    }
for(i=1;i<=num_contrasts;i++)
    for(k=1;k<=num_contrasts;k++)
        avg_transformed_ATA[i][k] /= num_glm_files;




#if 0
cond = condition_number(avg_transformed_ATA,num_contrasts);
printf("condition number = %f\n",cond);
if(cond > (double)B_THRESHOLD) {
    printf("Error: Condition number too big. Suspected numerical problem. Is this a widely spaced design?\n");
    printf("       Epsilon will be set to 1.\n");
    }
ATAm1 = inverse(avg_transformed_ATA,num_contrasts);
min_and_max_var(ATAm1,num_contrasts,&min,&max);
printf("smallest variance = %f  largest variance = %f\n",min,max);
if(max > (double)B_THRESHOLD && cond < (double)B_THRESHOLD) {
    printf("Error: Variance too big. Suspected numerical problem. Is this a widely spaced design?\n");
    printf("       Epsilon will be set to 1.\n");
    }
#endif

ATAm1 = dmatrix(1,num_contrasts,1,num_contrasts);
inverse_cover(avg_transformed_ATA,ATAm1,num_contrasts,(double)B_THRESHOLD,"transformed_ATA");
free_dmatrix(avg_transformed_ATA,1,num_contrasts,1,num_contrasts);
if(max > (double)B_THRESHOLD || cond > (double)B_THRESHOLD) {
    free_dmatrix(ATAm1,1,num_contrasts,1,num_contrasts);
    ATAm1 = (double **)NULL;
    }
return ATAm1;
}
#endif

#if 0
int print_cov_matrix(char *filename,double **ATAm1,int dim,Interfile_header *ifh)
{
    int i,j,k,SunOS_Linux;
    float *temp_float;
    if((SunOS_Linux=checkOS())==-1) return 0;
    if(!(temp_float=malloc(sizeof*temp_float*dim*dim))) {
        printf("Error: Unable to malloc temp_float in print_cov_matrix\n");
        return 0;
        }
    if(!ifh) if(!(ifh=init_ifh(4,dim,dim,1,1,2,2,2))) return 0;
    ifh->dim1 = dim;
    ifh->dim2 = dim;
    ifh->dim3 = 1;
    ifh->dim4 = 1;
    ifh->number_format = (int)FLOAT_IF;
    for(j=0,i=1;i<=dim;i++) for(k=1;k<=dim;k++,j++) temp_float[j] = (float)ATAm1[i][k];
    if(!writestack(filename,temp_float,sizeof*temp_float,(size_t)(dim*dim),SunOS_Linux)) return 0; 
    fprintf(stdout,"Output written to %s\n",filename);
    if(!write_ifh(filename,ifh,(int)FALSE)) return 0;
    free(temp_float);
    return 1;
}
#endif

int print_cov_matrix(char *filename,double **ATAm1,int dim,Interfile_header *ifh,int bigendian,int swapbytes)
{
    int i,j,k;
    float *temp_float;
    if(!(temp_float=malloc(sizeof*temp_float*dim*dim))) {
        printf("Error: Unable to malloc temp_float in print_cov_matrix\n");
        return 0;
        }

    //if(!ifh) if(!(ifh=init_ifh(4,dim,dim,1,1,2,2,2,bigendian))) return 0;
    //START161104
    if(!ifh)if(!(ifh=init_ifh(4,dim,dim,1,1,2,2,2,bigendian,(float*)NULL,(float*)NULL))) return 0;

    ifh->dim1 = dim;
    ifh->dim2 = dim;
    ifh->dim3 = 1;
    ifh->dim4 = 1;
    ifh->number_format = (int)FLOAT_IF;
    for(j=0,i=1;i<=dim;i++) for(k=1;k<=dim;k++,j++) temp_float[j] = (float)ATAm1[i][k];
    if(!writestack(filename,temp_float,sizeof*temp_float,(size_t)(dim*dim),swapbytes)) return 0;
    if(!write_ifh(filename,ifh,(int)FALSE)) return 0;
    fprintf(stdout,"Output written to %s\n",filename);
    free(temp_float);
    return 1;
}





























#if 0
/********************************************************************************************************/
double box_tc_avg_reduced_ATA(char **glm_files,int num_glm_files,TC *tcs,int *are_variances_heterogeneous,
    int lc_one_condition,int lc_print_cov_matrix,int lc_box)
/********************************************************************************************************/
{
int i,j,k;

double **A,**ATA,**ATAxtransformT,**transformed_ATA,**avg_transformed_ATA,**ATAm1,**transform,denominator,epsilon,min,max,
       *temp_double;

LinearModel *glm;

avg_transformed_ATA = (double **)dmatrix_0(1,tcs->num_tc,1,tcs->num_tc);

for(j=0;j<num_glm_files;j++) {
    glm = read_glm(glm_files[j],(int)TRUE);
    A = (double **)dmatrix(1,glm->Nrow,1,glm->Mcol);
    for(i=1;i<=glm->Nrow;i++)
        for(k=1;k<=glm->Mcol;k++)
            A[i][k] = (double)glm->A[i][k];
    ATA = dmatrix_mult(A,A,glm->Nrow,glm->Mcol,glm->Nrow,glm->Mcol,(int)TRANSPOSE_FIRST);
    transform = (double **)dmatrix_0(1,tcs->num_tc,1,glm->Mcol);
    for(i=0;i<tcs->num_tc;i++) {
        if(tcs->num_tc_to_sum[i][j] == 1)
            transform[i+1][(int)tcs->tc[i][j][0]] = 1;
        else {
            for(denominator=k=0;k<tcs->num_tc_to_sum[i][j];k++)
                denominator += 1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]];
            for(k=0;k<tcs->num_tc_to_sum[i][j];k++)
                transform[i+1][(int)tcs->tc[i][j][k]] = (1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]])/denominator;
            }
         }
    ATAxtransformT = dmatrix_mult(ATA,transform,glm->Mcol,glm->Mcol,tcs->num_tc,glm->Mcol,(int)TRANSPOSE_SECOND);
    transformed_ATA = dmatrix_mult(transform,ATAxtransformT,tcs->num_tc,glm->Mcol,glm->Mcol,tcs->num_tc,
                      (int)TRANSPOSE_NONE);

    for(i=1;i<=tcs->num_tc;i++)
        for(k=1;k<=tcs->num_tc;k++)
            avg_transformed_ATA[i][k] += transformed_ATA[i][k];

    /*for(i=1;i<=tcs->num_tc;i++) {
        for(k=1;k<=tcs->num_tc;k++) {
            avg_transformed_ATA[i][k] += transformed_ATA[i][k];
            if(transformed_ATA[i][k] > 50000) {
                printf("Error: %s  Covariance structure does not look right. Abort!\n",glm_files[j]);
                return (double)-1;
                }
            }
        }*/

    /*printf("%s\n",glm_files[j]);
    for(i=1;i<=tcs->num_tc;i++) {
        for(k=1;k<=tcs->num_tc;k++) {
            avg_transformed_ATA[i][k] += transformed_ATA[i][k];
            if(transformed_ATA[i][k] > 50000) {
                printf("Error: %s  Covariance structure does not look right. Abort!\n",glm_files[j]);
                return (double)-1;
                }
            printf("%5.2f ",transformed_ATA[i][k]);
            }
        printf("\n");
        }*/



    free_dmatrix(A,1,glm->Nrow,1,glm->Mcol);
    free_dmatrix(ATA,1,glm->Mcol,1,glm->Mcol);
    free_dmatrix(transform,1,tcs->num_tc,1,glm->Mcol);
    free_dmatrix(ATAxtransformT,1,glm->Mcol,1,tcs->num_tc);
    free_dmatrix(transformed_ATA,1,tcs->num_tc,1,tcs->num_tc);
    /*free_glm(glm,(int)READ_ATAM1);*/
    if(j==num_glm_files-1 && lc_print_cov_matrix==(int)TRUE)
        /*don't free*/;
    else
        free_glm(glm,(int)TRUE);
    }
for(i=1;i<=tcs->num_tc;i++)
    for(k=1;k<=tcs->num_tc;k++)
        avg_transformed_ATA[i][k] /= num_glm_files;

/*printf("avg_transformed_ATA\n");
for(i=1;i<=tcs->num_tc;i++) {
    for(k=1;k<=tcs->num_tc;k++) {
        printf("%.4f ",avg_transformed_ATA[i][k]);
        }
    printf("\n");
    }*/

ATAm1 = inverse(avg_transformed_ATA,tcs->num_tc);


/*printf("ATAm1\n");
for(i=1;i<=tcs->num_tc;i++) {
    for(k=1;k<=tcs->num_tc;k++) {
        printf("%f ",ATAm1[i][k]);
        }
    printf("\n");
    }*/


min_and_max_var(ATAm1,tcs->num_tc,&min,&max);
/*epsilon = calculate_epsilon(ATAm1,tcs->num_tc,frames);
printf("epsilon(avg_reduced_ATA) = %f   smallest variance = %f  largest variance = %f\n",epsilon,min,max);*/

if(lc_box) {
    epsilon = calculate_epsilon_whole_matrix_box(ATAm1,tcs->num_tc);
    printf("calculate_epsilon_whole_matrix_box = %f\n",epsilon);
    }

if(lc_one_condition == (int)FALSE) {
    printf("Correction for more than one condition. -> triangular\n");
    epsilon = calculate_epsilon_triangular(ATAm1,tcs->num_tc);
    }
else {
    /*printf("Correction for a single condition. -> whole matrix\n");
    epsilon = calculate_epsilon_whole_matrix(ATAm1,tcs->num_tc);*/
    printf("Correction for a single condition. -> Box analytical\n");
    epsilon = calculate_epsilon_whole_matrix_box(ATAm1,tcs->num_tc);
    }
printf("epsilon(avg_reduced_ATA) = %f   smallest variance = %f  largest variance = %f\n",epsilon,min,max);
if(max/min > 3) {
    /*printf("3*%f=%f %f\n",min,3*min,max);*/
    *are_variances_heterogeneous = (int)TRUE;
    printf("\tVariances are heterogeneous. Box correction will be used to correct for heterogeneity.\n");
    }
if(max > 1000000) {
    printf("Error: Variance too big. Suspected numerical problem. Is this a widely spaced design?\n");
    printf("       Epsilon will be set to 1.\n");
    epsilon = 1;
    }

if(lc_print_cov_matrix) {
    GETMEM(temp_double,tcs->num_tc*tcs->num_tc,double);
    glm->ifh->dim1 = tcs->num_tc;
    glm->ifh->dim2 = tcs->num_tc;
    glm->ifh->dim3 = 1;
    glm->ifh->dim4 = 1;
    glm->ifh->number_format = (int)DOUBLE_IF;
    for(j=0,i=1;i<=tcs->num_tc;i++)
        for(k=1;k<=tcs->num_tc;k++,j++) {
            temp_double[j] = (float)ATAm1[i][k];
            }
    write_double("ATAinv.4dfp.img",temp_double,tcs->num_tc*tcs->num_tc);
    fprintf(stderr,"Output written to ATAinv.4dfp.img\n");
    if(!write_ifh("ATAinv.4dfp.img",glm->ifh,(int)FALSE)) exit(-1);
    free(temp_double);
    free_glm(glm,(int)TRUE);
    }

free_dmatrix(avg_transformed_ATA,1,tcs->num_tc,1,tcs->num_tc);
free_dmatrix(ATAm1,1,tcs->num_tc,1,tcs->num_tc);

return epsilon;
}

/*************************************************************************************************/
double box_mag_avg_reduced_ATA(char **glm_files,int num_glm_files,int **contrast,int num_contrasts,
                               int *are_variances_heterogeneous,int lc_one_condition)
/*************************************************************************************************/
{ /*START*/
int i,j,k;

double **A,**ATA,**ATAxtransformT,**transformed_ATA,**avg_transformed_ATA,**ATAm1,**transform,denominator,epsilon,min,max;

LinearModel *glm;

avg_transformed_ATA = (double **)dmatrix_0(1,num_contrasts,1,num_contrasts);

for(j=0;j<num_glm_files;j++) {
    glm = read_glm(glm_files[j],(int)READ_ATAM1);
    A = (double **)dmatrix(1,glm->Nrow,1,glm->Mcol);
    for(i=1;i<=glm->Nrow;i++)
        for(k=1;k<=glm->Mcol;k++)
            A[i][k] = (double)glm->A[i][k];
    ATA = dmatrix_mult(A,A,glm->Nrow,glm->Mcol,glm->Nrow,glm->Mcol,(int)TRANSPOSE_FIRST);
    transform = (double **)dmatrix(1,num_contrasts,1,glm->Mcol);
    for(i=0;i<num_contrasts;i++)
        for(k=0;k<glm->Mcol;k++)
            transform[i+1][k+1] = glm->c[(contrast[i][j]-1)*glm->Mcol+k];
    ATAxtransformT = dmatrix_mult(ATA,transform,glm->Mcol,glm->Mcol,num_contrasts,glm->Mcol,(int)TRANSPOSE_SECOND);
    transformed_ATA = dmatrix_mult(transform,ATAxtransformT,num_contrasts,glm->Mcol,glm->Mcol,num_contrasts,
                      (int)TRANSPOSE_NONE);
    for(i=1;i<=num_contrasts;i++)
        for(k=1;k<=num_contrasts;k++)
            avg_transformed_ATA[i][k] += transformed_ATA[i][k];

    free_dmatrix(A,1,glm->Nrow,1,glm->Mcol);
    free_dmatrix(ATA,1,glm->Mcol,1,glm->Mcol);
    free_dmatrix(transform,1,num_contrasts,1,glm->Mcol);
    free_dmatrix(ATAxtransformT,1,glm->Mcol,1,num_contrasts);
    free_dmatrix(transformed_ATA,1,num_contrasts,1,num_contrasts);
    free_glm(glm,(int)READ_ATAM1);
    }
for(i=1;i<=num_contrasts;i++)
    for(k=1;k<=num_contrasts;k++)
        avg_transformed_ATA[i][k] /= num_glm_files;
ATAm1 = inverse(avg_transformed_ATA,num_contrasts);

min_and_max_var(avg_transformed_ATA,num_contrasts,&min,&max);

if(lc_one_condition == (int)FALSE) {
    printf("Correction for more than one condition. -> triangular\n");
    epsilon = calculate_epsilon_triangular(ATAm1,num_contrasts);
    }
else {
    /*printf("Correction for a single condition. -> whole matrix\n");
    epsilon = calculate_epsilon_whole_matrix(ATAm1,num_contrasts);*/
    printf("Correction for a single condition. -> Box analytical\n");
    epsilon = calculate_epsilon_whole_matrix_box(ATAm1,num_contrasts);
    }
printf("epsilon(avg_reduced_ATA) = %f   smallest variance = %f  largest variance = %f\n",epsilon,min,max);
if(max/min > 3) {
    /*printf("3*%f=%f %f\n",min,3*min,max);*/
    *are_variances_heterogeneous = (int)TRUE;
    printf("\tVariances are heterogeneous. Box correction will be used to correct for heterogeneity.\n");
    }
free_dmatrix(avg_transformed_ATA,1,num_contrasts,1,num_contrasts);
free_dmatrix(ATAm1,1,num_contrasts,1,num_contrasts);

return epsilon;
}
#endif








/*************************************************************************************************************/
double box_tc_avg_reduced_ATA_wYYT(char **glm_files,int num_glm_files,TC *tcs,int *are_variances_heterogeneous,
    char **YYT_files,int swapbytes)
/*************************************************************************************************************/
{
int i,j,k,m,lenvol,SunOS_Linux;
float *temp_float=NULL;
double **A,**ATA,**ATAxtransformT,**transformed_ATA,**avg_transformed_ATA,**ATAm1,**transform,denominator,epsilon,min,max,
       *temp_double=NULL,**YYT,**YYTA,**ATYYTA,**ATYYTAxtransformT,**transformed_ATYYTA,**avg_transformed_ATYYTA,
       **avg_transformed_ATYYTAxATAm1,**cov;
LinearModel *glm;
Interfile_header *ifh=NULL;

/*START150324*/
if((SunOS_Linux=checkOS())==-1) exit(-1);

avg_transformed_ATA = (double **)dmatrix_0(1,tcs->num_tc,1,tcs->num_tc);
avg_transformed_ATYYTA = (double **)dmatrix_0(1,tcs->num_tc,1,tcs->num_tc);
ATAm1 = dmatrix(1,tcs->num_tc,1,tcs->num_tc);
for(j=0;j<num_glm_files;j++) {
    printf("j=%d ",j);
    glm = read_glm(glm_files[j],1,swapbytes);
    A = dmatrix(1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
    for(i=0;i<glm->ifh->glm_Nrow;i++) for(k=0;k<glm->ifh->glm_Mcol;k++) A[i+1][k+1] = (double)glm->AT[k][i];
    ATA = dmatrix_mult(A,A,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,(int)TRANSPOSE_FIRST);
    if(!(ifh = read_ifh(YYT_files[j],(Interfile_header*)NULL))) exit(-1);
    lenvol = ifh->dim1*ifh->dim2;
    YYT = (double **)dmatrix(1,ifh->dim1,1,ifh->dim2);

    #if 0
    if(ifh->number_format == (int)FLOAT_IF) {
        printf("FLOAT\n");
        GETMEM(temp_float,lenvol,float);
        if(!read_float(YYT_files[j],temp_float,lenvol)) exit(-1);
        for(m=0,i=1;i<=ifh->dim1;i++)
            for(k=1;k<=ifh->dim2;k++,m++)
                YYT[i][k] = (double)temp_float[m];
        free(temp_float);
        }
    else {
        printf("DOUBLE\n");
        GETMEM(temp_double,lenvol,double);
        /*if(!read_double(YYT_files[j],temp_double,lenvol)) exit(-1);*/
        for(m=0,i=1;i<=ifh->dim1;i++)
            for(k=1;k<=ifh->dim2;k++,m++)
                YYT[i][k] = temp_double[m];
        free(temp_double);
        }
    #endif
    /*START150324*/
    if(ifh->number_format == (int)FLOAT_IF) {
        if(!(temp_float=malloc(sizeof*temp_float*lenvol))) {
            printf("Error: Unable to malloc temp_float\n");
            exit(-1);
            }
        }
    else {
        if(!(temp_double=malloc(sizeof*temp_double*lenvol))) {
            printf("Error: Unable to malloc temp_double\n");
            exit(-1);
            }
        }
    if(!readstack(YYT_files[j],ifh->number_format==(int)FLOAT_IF?(void*)temp_float:(void*)temp_double,
        ifh->number_format==(int)FLOAT_IF?sizeof(float):sizeof(double),(size_t)lenvol,SunOS_Linux,ifh->bigendian)) exit(-1);
    if(ifh->number_format == (int)FLOAT_IF) {
        printf("FLOAT\n");
        for(m=0,i=1;i<=ifh->dim1;i++)
            for(k=1;k<=ifh->dim2;k++,m++)
                YYT[i][k] = (double)temp_float[m];
        free(temp_float);
        }
    else {
        printf("DOUBLE\n");
        for(m=0,i=1;i<=ifh->dim1;i++)
            for(k=1;k<=ifh->dim2;k++,m++)
                YYT[i][k] = temp_double[m];
        free(temp_double);
        }



    /*printf("YYT\n"); for(m=1;m<=10;m++) printf("%f\n",YYT[1][m]);
    printf("YYT\n"); for(m=1;m<=10;m++) printf("%f\n",YYT[m][1]);*/
    YYTA = dmatrix_mult(YYT,A,ifh->dim1,ifh->dim2,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,(int)TRANSPOSE_NONE);
    ATYYTA = dmatrix_mult(A,YYTA,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,ifh->dim1,glm->ifh->glm_Mcol,(int)TRANSPOSE_FIRST);

    /*printf("ATYYTA\n"); for(m=1;m<=10;m++) printf("%f\n",ATYYTA[1][m]);
    printf("ATYYTA\n"); for(m=1;m<=10;m++) printf("%f\n",ATYYTA[m][1]);*/

    transform = (double **)dmatrix_0(1,tcs->num_tc,1,glm->ifh->glm_Mcol);
    for(i=0;i<tcs->num_tc;i++) {
        if(tcs->num_tc_to_sum[i][j] == 1)
            transform[i+1][(int)tcs->tc[i][j][0]] = 1;
        else {
            for(denominator=k=0;k<tcs->num_tc_to_sum[i][j];k++)
                denominator += 1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]];
            for(k=0;k<tcs->num_tc_to_sum[i][j];k++)
                transform[i+1][(int)tcs->tc[i][j][k]] = (1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]])/denominator;
            }
         }
    ATAxtransformT = dmatrix_mult(ATA,transform,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,glm->ifh->glm_Mcol,(int)TRANSPOSE_SECOND);
    transformed_ATA = dmatrix_mult(transform,ATAxtransformT,tcs->num_tc,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,
                      (int)TRANSPOSE_NONE);
    for(i=1;i<=tcs->num_tc;i++)
        for(k=1;k<=tcs->num_tc;k++) avg_transformed_ATA[i][k] += transformed_ATA[i][k];
    ATYYTAxtransformT = dmatrix_mult(ATYYTA,transform,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,glm->ifh->glm_Mcol,(int)TRANSPOSE_SECOND);
    transformed_ATYYTA = dmatrix_mult(transform,ATYYTAxtransformT,tcs->num_tc,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,tcs->num_tc,
                         (int)TRANSPOSE_NONE);
    for(i=1;i<=tcs->num_tc;i++)
        for(k=1;k<=tcs->num_tc;k++)
            avg_transformed_ATYYTA[i][k] += transformed_ATYYTA[i][k];

    free_dmatrix(A,1,glm->ifh->glm_Nrow,1,glm->ifh->glm_Mcol);
    free_dmatrix(ATA,1,glm->ifh->glm_Mcol,1,glm->ifh->glm_Mcol);
    free_dmatrix(transform,1,tcs->num_tc,1,glm->ifh->glm_Mcol);
    free_dmatrix(ATAxtransformT,1,glm->ifh->glm_Mcol,1,tcs->num_tc);
    free_dmatrix(transformed_ATA,1,tcs->num_tc,1,tcs->num_tc);
    free_dmatrix(YYT,1,ifh->dim1,1,ifh->dim2);
    free_dmatrix(YYTA,1,ifh->dim1,1,glm->ifh->glm_Mcol);
    free_dmatrix(ATYYTA,1,glm->ifh->glm_Mcol,1,glm->ifh->glm_Mcol);
    free_dmatrix(ATYYTAxtransformT,1,glm->ifh->glm_Mcol,1,tcs->num_tc);
    free_dmatrix(transformed_ATYYTA,1,tcs->num_tc,1,tcs->num_tc);
    free_glm(glm);
    if(j<num_glm_files-1) free_ifh(ifh,(int)FALSE);
    }
for(i=1;i<=tcs->num_tc;i++)
    for(k=1;k<=tcs->num_tc;k++) {
        avg_transformed_ATA[i][k] /= (int)num_glm_files;
        avg_transformed_ATYYTA[i][k] /= (int)num_glm_files;
        }

/*ATAm1 = inverse(avg_transformed_ATA,tcs->num_tc);*/
/*ATAm1 = inverse(avg_transformed_ATA,tcs->num_tc,(double**)NULL);*/
inverse(avg_transformed_ATA,tcs->num_tc,ATAm1);

avg_transformed_ATYYTAxATAm1 = dmatrix_mult(avg_transformed_ATYYTA,ATAm1,tcs->num_tc,tcs->num_tc,tcs->num_tc,tcs->num_tc,
                               (int)TRANSPOSE_NONE);
cov = dmatrix_mult(ATAm1,avg_transformed_ATYYTAxATAm1,tcs->num_tc,tcs->num_tc,tcs->num_tc,tcs->num_tc,(int)TRANSPOSE_NONE);

min_and_max_var(cov,tcs->num_tc,&min,&max);
epsilon = calculate_epsilon_triangular(cov,tcs->num_tc);
printf("epsilon(avg_reduced_ATA_wYYT - triangular) = %f   smallest variance = %f  largest variance = %f\n",epsilon,min,max);
if(max/min > 3) {
    *are_variances_heterogeneous = (int)TRUE;
    printf("\tVariances are heterogeneous. Box correction will be used to correct for heterogeneity.\n");
    }
if(ifh->number_format == (int)FLOAT_IF) {
    printf("FLOAT\n");
    GETMEM(temp_float,tcs->num_tc*tcs->num_tc,float);
    for(j=0,i=1;i<=tcs->num_tc;i++)
        for(k=1;k<=tcs->num_tc;k++,j++) {
            temp_float[j] = (float)cov[i][k];
            }
    if(!writestack("fullcov.4dfp.img",temp_float,sizeof(float),(size_t)(tcs->num_tc*tcs->num_tc),0)) exit(-1);
    fprintf(stderr,"Output written to fullcov.4dfp.img\n");
    for(j=0,i=1;i<=tcs->num_tc;i++)
        for(k=1;k<=tcs->num_tc;k++,j++) {
            temp_float[j] = (float)ATAm1[i][k];
            }
    if(!writestack("ATAinv.4dfp.img",temp_float,sizeof(float),(size_t)(tcs->num_tc*tcs->num_tc),0)) exit(-1);
    fprintf(stderr,"Output written to ATAinv.4dfp.img\n");
    free(temp_float);
    }
else {
    printf("DOUBLE\n");
    GETMEM(temp_double,tcs->num_tc*tcs->num_tc,double);
    for(j=0,i=1;i<=tcs->num_tc;i++)
        for(k=1;k<=tcs->num_tc;k++,j++) {
            temp_double[j] = cov[i][k];
            }

    /*if(!write_double("fullcov.4dfp.img",temp_double,tcs->num_tc*tcs->num_tc)) exit(-1);*/
    /*START150324*/
    if(!writestack("fullcov.4dfp.img",temp_double,sizeof(double),(size_t)(tcs->num_tc*tcs->num_tc),0)) exit(-1);

    fprintf(stderr,"Output written to fullcov.4dfp.img\n");
    for(j=0,i=1;i<=tcs->num_tc;i++)
        for(k=1;k<=tcs->num_tc;k++,j++) {
            temp_double[j] = ATAm1[i][k];
            }

    /*if(!write_double("ATAinv.4dfp.img",temp_double,tcs->num_tc*tcs->num_tc)) exit(-1);;*/
    /*START150324*/
    if(!writestack("ATAinv.4dfp.img",temp_double,sizeof(double),(size_t)(tcs->num_tc*tcs->num_tc),0)) exit(-1);

    fprintf(stderr,"Output written to ATAinv.4dfp.img\n");
    free(temp_double);
    }
ifh->dim1 = tcs->num_tc;
ifh->dim2 = tcs->num_tc;
ifh->dim3 = 1;
ifh->dim4 = 1;
if(!write_ifh("fullcov.4dfp.img",ifh,(int)FALSE)) exit(-1);
if(!write_ifh("ATAinv.4dfp.img",ifh,(int)FALSE)) exit(-1);


calculate_and_print_correlation_coefficient(ATAm1,tcs->num_tc,ifh,"ATAinv_corrcoeff.4dfp.img");
calculate_and_print_correlation_coefficient(cov,tcs->num_tc,ifh,"fullcov_corrcoeff.4dfp.img");

free_dmatrix(avg_transformed_ATA,1,tcs->num_tc,1,tcs->num_tc);
free_dmatrix(ATAm1,1,tcs->num_tc,1,tcs->num_tc);
free_dmatrix(avg_transformed_ATYYTA,1,tcs->num_tc,1,tcs->num_tc);
free_dmatrix(avg_transformed_ATYYTAxATAm1,1,tcs->num_tc,1,tcs->num_tc);
free_dmatrix(cov,1,tcs->num_tc,1,tcs->num_tc);
free_ifh(ifh,(int)FALSE);

return epsilon;
}















#if 0
/*******************************************************************************************************************/
double box_tc_avg_reduced_ATA_wYYT_and_Y(char **glm_files,int num_glm_files,TC *tcs,int *are_variances_heterogeneous,
    char **YYT_files,char **Y_files)
/*******************************************************************************************************************/
{
int i,j,k,m,lenvol;

float *temp_float;

double **A,**ATA,**ATAxtransformT,**transformed_ATA,**avg_transformed_ATA,**ATAm1,**transform,denominator,epsilon,min,max,
       *temp_double,**YYT,**YYTA,**ATYYTA,**ATYYTAxtransformT,**transformed_ATYYTA,**avg_transformed_ATYYTA,
       **avg_transformed_ATYYTAxATAm1,**cov,*Y;

LinearModel *glm;
Interfile_header *ifh;


avg_transformed_ATA = (double **)dmatrix_0(1,tcs->num_tc,1,tcs->num_tc);
avg_transformed_ATYYTA = (double **)dmatrix_0(1,tcs->num_tc,1,tcs->num_tc);

for(j=0;j<num_glm_files;j++) {
    printf("j=%d\n",j);
    glm = read_glm(glm_files[j],(int)TRUE);
    A = (double **)dmatrix(1,glm->Nrow,1,glm->Mcol);
    for(i=1;i<=glm->Nrow;i++)
        for(k=1;k<=glm->Mcol;k++)
            A[i][k] = (double)glm->A[i][k];
    ATA = dmatrix_mult(A,A,glm->Nrow,glm->Mcol,glm->Nrow,glm->Mcol,(int)TRANSPOSE_FIRST);

    if((ifh = read_ifh(YYT_files[j])) == NULL) exit(-1);
    lenvol = ifh->dim1*ifh->dim2;
    GETMEM(temp_double,lenvol,double);


    /*YYT = (double **)dmatrix(1,ifh->dim1,1,ifh->dim2);
    read_double(YYT_files[j],temp_double,lenvol);
    for(m=0,i=1;i<=ifh->dim1;i++)
        for(k=1;k<=ifh->dim2;k++,m++)
            YYT[i][k] = temp_double[m];*/

    /*Y2 = (double **)dmatrix(1,ifh->dim1,1,ifh->dim1);
    read_double(Y_files[j],temp_double,ifh->dim1);
    for(i=1;i<=ifh->dim1;i++)
        for(k=1;k<=ifh->dim1;k++)
            Y2[i][k] = temp_double[i-1]*temp_double[k-1];*/

    YYT = (double **)dmatrix(1,ifh->dim1,1,ifh->dim2);
    read_double(YYT_files[j],temp_double,lenvol);
    GETMEM(Y,ifh->dim1,double);
    read_double(Y_files[j],Y,ifh->dim1);
    for(m=0,i=1;i<=ifh->dim1;i++)
        for(k=1;k<=ifh->dim2;k++,m++)
            YYT[i][k] = temp_double[m] - Y[i-1]*Y[k-1];


    /*printf("YYT\n"); for(m=1;m<=10;m++) printf("%f\n",YYT[1][m]);
    printf("YYT\n"); for(m=1;m<=10;m++) printf("%f\n",YYT[m][1]);*/
    YYTA = dmatrix_mult(YYT,A,ifh->dim1,ifh->dim2,glm->Nrow,glm->Mcol,(int)TRANSPOSE_NONE);
    ATYYTA = dmatrix_mult(A,YYTA,glm->Nrow,glm->Mcol,ifh->dim1,glm->Mcol,(int)TRANSPOSE_FIRST);

    /*printf("ATYYTA\n"); for(m=1;m<=10;m++) printf("%f\n",ATYYTA[1][m]);
    printf("ATYYTA\n"); for(m=1;m<=10;m++) printf("%f\n",ATYYTA[m][1]);*/

    transform = (double **)dmatrix_0(1,tcs->num_tc,1,glm->Mcol);
    for(i=0;i<tcs->num_tc;i++) {
        if(tcs->num_tc_to_sum[i][j] == 1)
            transform[i+1][(int)tcs->tc[i][j][0]] = 1;
        else {
            for(denominator=k=0;k<tcs->num_tc_to_sum[i][j];k++)
                denominator += 1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]];
            for(k=0;k<tcs->num_tc_to_sum[i][j];k++)
                transform[i+1][(int)tcs->tc[i][j][k]] = (1/ATA[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]])/denominator;
            }
         }
    ATAxtransformT = dmatrix_mult(ATA,transform,glm->Mcol,glm->Mcol,tcs->num_tc,glm->Mcol,(int)TRANSPOSE_SECOND);
    transformed_ATA = dmatrix_mult(transform,ATAxtransformT,tcs->num_tc,glm->Mcol,glm->Mcol,tcs->num_tc,
                      (int)TRANSPOSE_NONE);
    for(i=1;i<=tcs->num_tc;i++)
        for(k=1;k<=tcs->num_tc;k++)
            avg_transformed_ATA[i][k] += transformed_ATA[i][k];

    ATYYTAxtransformT = dmatrix_mult(ATYYTA,transform,glm->Mcol,glm->Mcol,tcs->num_tc,glm->Mcol,(int)TRANSPOSE_SECOND);
    transformed_ATYYTA = dmatrix_mult(transform,ATYYTAxtransformT,tcs->num_tc,glm->Mcol,glm->Mcol,tcs->num_tc,
                         (int)TRANSPOSE_NONE);
    for(i=1;i<=tcs->num_tc;i++)
        for(k=1;k<=tcs->num_tc;k++)
            avg_transformed_ATYYTA[i][k] += transformed_ATYYTA[i][k];

    free_dmatrix(A,1,glm->Nrow,1,glm->Mcol);
    free_dmatrix(ATA,1,glm->Mcol,1,glm->Mcol);
    free_dmatrix(transform,1,tcs->num_tc,1,glm->Mcol);
    free_dmatrix(ATAxtransformT,1,glm->Mcol,1,tcs->num_tc);
    free_dmatrix(transformed_ATA,1,tcs->num_tc,1,tcs->num_tc);
    free(temp_double);
    free(Y);
    free_dmatrix(YYT,1,ifh->dim1,1,ifh->dim2);
    free_dmatrix(YYTA,1,ifh->dim1,1,glm->Mcol);
    free_dmatrix(ATYYTA,1,glm->Mcol,1,glm->Mcol);
    free_dmatrix(ATYYTAxtransformT,1,glm->Mcol,1,tcs->num_tc);
    free_dmatrix(transformed_ATYYTA,1,tcs->num_tc,1,tcs->num_tc);
    free_glm(glm,(int)TRUE);
    if(j<num_glm_files-1) free_ifh(ifh,(int)FALSE);
    }
for(i=1;i<=tcs->num_tc;i++)
    for(k=1;k<=tcs->num_tc;k++) {
        avg_transformed_ATA[i][k] /= (int)num_glm_files;
        avg_transformed_ATYYTA[i][k] /= (int)num_glm_files;
        }

ATAm1 = inverse(avg_transformed_ATA,tcs->num_tc);
avg_transformed_ATYYTAxATAm1 = dmatrix_mult(avg_transformed_ATYYTA,ATAm1,tcs->num_tc,tcs->num_tc,tcs->num_tc,tcs->num_tc,
                               (int)TRANSPOSE_NONE);
cov = dmatrix_mult(ATAm1,avg_transformed_ATYYTAxATAm1,tcs->num_tc,tcs->num_tc,tcs->num_tc,tcs->num_tc,(int)TRANSPOSE_NONE);

min_and_max_var(cov,tcs->num_tc,&min,&max);
epsilon = calculate_epsilon_triangular(cov,tcs->num_tc);
printf("epsilon(avg_reduced_ATA_wYYT_and_Y - triangular) = %f   smallest variance = %f  largest variance = %f\n",epsilon,min,max);
if(max/min > 3) {
    *are_variances_heterogeneous = (int)TRUE;
    printf("\tVariances are heterogeneous. Box correction will be used to correct for heterogeneity.\n");
    }
GETMEM(temp_double,tcs->num_tc*tcs->num_tc,double);
/*for(j=0,i=1;i<=tcs->num_tc;i++)
    for(k=1;k<=tcs->num_tc;k++,j++) {
        temp_float[j] = (float)avg_transformed_ATYYTAxATAm1[i][k];
        }
if(!writestack("ATYYTAxATAinv.4dfp.img",temp_float,sizeof(float),(size_t)(tcs->num_tc*tcs->num_tc),0)) exit(-1);
fprintf(stderr,"Output written to ATYYTAxATAinv.4dfp.img\n");*/
ifh->dim1 = tcs->num_tc;
ifh->dim2 = tcs->num_tc;
ifh->dim3 = 1;
ifh->dim4 = 1;
for(j=0,i=1;i<=tcs->num_tc;i++)
    for(k=1;k<=tcs->num_tc;k++,j++) {
        temp_double[j] = cov[i][k];
        }
write_double("ATAinv.4dfp.img",temp_double,tcs->num_tc*tcs->num_tc);
fprintf(stderr,"Output written to ATAinv.4dfp.img\n");
if(!write_ifh("ATAinv.4dfp.img",ifh,(int)FALSE)) exit(-1);
free(temp_double);

calculate_and_print_correlation_coefficient(ATAm1,tcs->num_tc,ifh,"ATAinv_corrcoeff.4dfp.img");
calculate_and_print_correlation_coefficient(cov,tcs->num_tc,ifh,"fullcov_corrcoeff.4dfp.img");

free_dmatrix(avg_transformed_ATA,1,tcs->num_tc,1,tcs->num_tc);
free_dmatrix(ATAm1,1,tcs->num_tc,1,tcs->num_tc);
free_dmatrix(avg_transformed_ATYYTA,1,tcs->num_tc,1,tcs->num_tc);
free_dmatrix(avg_transformed_ATYYTAxATAm1,1,tcs->num_tc,1,tcs->num_tc);
free_dmatrix(cov,1,tcs->num_tc,1,tcs->num_tc);
free_ifh(ifh,(int)FALSE);

return epsilon;
}
/*END*/



#if 0
/***********************************************************************************************************************/
double box_correction_tc_avgATAm1(char **glm_files,int num_glm_files,TC *tcs,int frames,int *are_variances_heterogeneous)
/***********************************************************************************************************************/
{
int i,j,k;

double **ATAm1,**transform,denominator,**ATAm1xtransformT,**transformed_ATAm1,**avg_transformed_ATAm1,epsilon,min,max;

LinearModel *glm;

avg_transformed_ATAm1 = (double **)dmatrix_0(1,tcs->num_tc,1,tcs->num_tc);

for(j=0;j<num_glm_files;j++) {
    glm = read_glm(glm_files[j],(int)READ_ATAM1);
    ATAm1 = (double **)dmatrix(1,glm->Mcol,1,glm->Mcol);
    for(i=1;i<=glm->Mcol;i++)
        for(k=1;k<=glm->Mcol;k++)
            ATAm1[i][k] = (double)glm->ATAm1[i][k];
    transform = (double **)dmatrix_0(1,tcs->num_tc,1,glm->Mcol);
    for(i=0;i<tcs->num_tc;i++) {
        if(tcs->num_tc_to_sum[i][j] == 1)
            transform[i+1][(int)tcs->tc[i][j][0]] = 1;
        else {
            for(denominator=k=0;k<tcs->num_tc_to_sum[i][j];k++)
                denominator += 1/ATAm1[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]];
            for(k=0;k<tcs->num_tc_to_sum[i][j];k++)
                transform[i+1][(int)tcs->tc[i][j][k]] = (1/ATAm1[(int)tcs->tc[i][j][k]][(int)tcs->tc[i][j][k]])/denominator;
            }
         }
    ATAm1xtransformT = dmatrix_mult(ATAm1,transform,glm->Mcol,glm->Mcol,tcs->num_tc,glm->Mcol,(int)TRANSPOSE_SECOND);
    transformed_ATAm1 = dmatrix_mult(transform,ATAm1xtransformT,tcs->num_tc,glm->Mcol,glm->Mcol,tcs->num_tc,
                        (int)TRANSPOSE_NONE);
    for(i=1;i<=tcs->num_tc;i++)
        for(k=1;k<=tcs->num_tc;k++)
            avg_transformed_ATAm1[i][k] += transformed_ATAm1[i][k];

    free_dmatrix(transform,1,tcs->num_tc,1,glm->Mcol);
    free_dmatrix(ATAm1,1,glm->Mcol,1,glm->Mcol);
    free_dmatrix(ATAm1xtransformT,1,glm->Mcol,1,tcs->num_tc);
    free_dmatrix(transformed_ATAm1,1,tcs->num_tc,1,tcs->num_tc);
    free_glm(glm,(int)READ_ATAM1);
    }
for(i=1;i<=tcs->num_tc;i++)
    for(k=1;k<=tcs->num_tc;k++)
        avg_transformed_ATAm1[i][k] /= num_glm_files;

epsilon = calculate_epsilon(avg_transformed_ATAm1,tcs->num_tc,frames);
min_and_max_var(avg_transformed_ATAm1,tcs->num_tc,&min,&max);
printf("epsilon = %f   smallest variance = %f  largest variance = %f\n",epsilon,min,max);
if(max/min > 3) {
    /*printf("3*%f=%f %f\n",min,3*min,max);*/
    *are_variances_heterogeneous = (int)TRUE;
    printf("\tVariances are heterogeneous. Box correction will be used to correct for heterogeneity.\n");
    }
free_dmatrix(avg_transformed_ATAm1,1,tcs->num_tc,1,tcs->num_tc);

return epsilon;
}
#endif


#if 0
/*****************************************************************************************************************************/
double box_correction_mag_avgATAm1(char **glm_files,int num_glm_files,int **contrast,int num_contrasts,int frames,
                                   int *are_variances_heterogeneous)
/*****************************************************************************************************************************/
{
int i,j,k;

double **ATAm1,**transform,denominator,**ATAm1xtransformT,**transformed_ATAm1,**avg_transformed_ATAm1,epsilon,epsilon_min=1,min,max;

LinearModel *glm;

avg_transformed_ATAm1 = (double **)dmatrix_0(1,num_contrasts,1,num_contrasts);

for(j=0;j<num_glm_files;j++) {
    glm = read_glm(glm_files[j],(int)READ_ATAM1);
    ATAm1 = (double **)dmatrix(1,glm->Mcol,1,glm->Mcol);
    for(i=1;i<=glm->Mcol;i++)
        for(k=1;k<=glm->Mcol;k++)
            ATAm1[i][k] = (double)glm->ATAm1[i][k];
    transform = (double **)dmatrix(1,num_contrasts,1,glm->Mcol);
    for(i=0;i<num_contrasts;i++)
        for(k=0;k<glm->Mcol;k++)
            transform[i+1][k+1] = glm->c[(contrast[i][j]-1)*glm->Mcol+k];
    ATAm1xtransformT = dmatrix_mult(ATAm1,transform,glm->Mcol,glm->Mcol,num_contrasts,glm->Mcol,(int)TRANSPOSE_SECOND);
    /*printf("\ntransformed_ATAm1\n");
    for(i=1;i<=glm->Mcol;i++) {
        for(k=1;k<=num_contrasts;k++) printf("%f ",ATAm1xtransformT[i][k]);
        printf("\n");
        }*/

    transformed_ATAm1 = dmatrix_mult(transform,ATAm1xtransformT,num_contrasts,glm->Mcol,glm->Mcol,num_contrasts,
                                     (int)TRANSPOSE_NONE);
    /*printf("\ntransformed_ATAm1\n");
    for(i=1;i<=num_contrasts;i++) {
        for(k=1;k<=num_contrasts;k++) printf("%f ",transformed_ATAm1[i][k]);
        printf("\n");
        }*/

    for(i=1;i<=num_contrasts;i++)
        for(k=1;k<=num_contrasts;k++)
            avg_transformed_ATAm1[i][k] += transformed_ATAm1[i][k];

    free_dmatrix(transform,1,num_contrasts,1,glm->Mcol);
    free_dmatrix(ATAm1,1,glm->Mcol,1,glm->Mcol);
    free_dmatrix(ATAm1xtransformT,1,glm->Mcol,1,num_contrasts);
    free_dmatrix(transformed_ATAm1,1,num_contrasts,1,num_contrasts);
    free_glm(glm,(int)READ_ATAM1);
    }
for(i=1;i<=num_contrasts;i++)
    for(k=1;k<=num_contrasts;k++)
        avg_transformed_ATAm1[i][k] /= num_glm_files;

epsilon = calculate_epsilon(avg_transformed_ATAm1,num_contrasts,frames);
min_and_max_var(avg_transformed_ATAm1,num_contrasts,&min,&max);
printf("epsilon = %f   smallest variance = %f  largest variance = %f\n",epsilon,min,max);
if(max/min > 3) {
    /*printf("3*%f=%f %f\n",min,3*min,max);*/
    *are_variances_heterogeneous = (int)TRUE;
    printf("\tVariances are heterogeneous. Box correction will be used to correct for heterogeneity.\n");
    }
free_dmatrix(avg_transformed_ATAm1,1,num_contrasts,1,num_contrasts);

return epsilon;
}
#endif
#endif

void get_transform_gsl(TC *tcs,int tc_contrast,int j,double **ATA,int Mcol,double *c,double **transform)
{
    int i,k,index;
    double denominator;
    if(!tc_contrast) {
        for(i=0;i<tcs->num_tc;i++) {
            if(tcs->num_tc_to_sum[i][j] == 1)
                transform[i][(int)tcs->tc[i][j][0]-1] = 1.;
            else {
                for(denominator=k=0;k<tcs->num_tc_to_sum[i][j];k++) {
                    index = (int)tcs->tc[i][j][k]-1;
                    denominator += 1./ATA[index][index];
                    }
                for(k=0;k<tcs->num_tc_to_sum[i][j];k++) {
                    index = (int)tcs->tc[i][j][k]-1;
                    transform[i][index] = (1./ATA[index][index])/denominator;
                    }
                }
             }
        }
    else {
        for(i=0;i<tcs->num_tc;i++) {
            index = ((int)tcs->tc[i][j][0]-1)*Mcol;
            for(k=0;k<Mcol;k++) transform[i][k] = (double)c[index+k];
            }
        }
}
