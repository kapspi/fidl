/* Copyright 5/18/05 Washington University.  All Rights Reserved.
   find_regions_by_file_cover.c  $Revision: 1.21 $*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_sort_int.h>
#include "find_regions_by_file_cover.h"
#include "d2intvar.h"
#include "constants.h"

Regions_By_File *find_regions_by_file_cover(int num_region_files,int num_regions,Regions **reg,int *roi)
{
    int *nregions,i,j,m,n,*roinew; /* k,l */
    size_t i1,j1,*index;

    //START160726
    //double *roid;

    Regions_By_File *rbf;
    if(!num_region_files) {
        if(!(rbf=malloc(sizeof*rbf))) {
            printf("Error: Unable to allocate rbf\n");
            return NULL;
            }
        rbf->nvoxels = num_regions;
        if(!(rbf->indices=malloc(sizeof*rbf->indices*rbf->nvoxels))) {
            printf("Error: Unable to allocate rbf->indices\n");
            return NULL;
            }
        for(i=0;i<rbf->nvoxels;i++) rbf->indices[i] = i;
        }
    else {
        if(!(nregions=malloc(sizeof*nregions*num_region_files))) {
            printf("Error: Unable to allocate nregions\n");
            return NULL;
            }
        if(!(roinew=malloc(sizeof*roinew*num_regions))) {
            printf("Error: Unable to allocate roinew\n");
            return NULL;
            }
        if(!(index=malloc(sizeof*index*num_regions))) {
            printf("Error: Unable to allocate index\n");
            return NULL;
            }

        #if 0
        if(!(roid=malloc(sizeof*roid*num_regions))) {
            printf("Error: Unable to allocate roid\n");
            return NULL;
            }
        for(i=0;i<num_regions;i++,j++) roid[i] = (double)roi[i];
        gsl_sort_index(index,roid,(size_t)1,(size_t)num_regions);
        free(roid);
        #endif
        //START160726
        gsl_sort_int_index(index,roi,(size_t)1,(size_t)num_regions);

        for(i=0;i<num_regions;i++) roinew[i] = roi[index[i]];
        for(i=0;i<num_region_files;i++) nregions[i] = reg[i]->nregions;
        if(!(rbf=find_regions_by_file(num_region_files,num_regions,nregions,roinew))) return NULL;
        rbf->nreg = num_regions;
        if(!(rbf->nvoxels_region=malloc(sizeof*rbf->nvoxels_region*num_regions))) {
            printf("Error: Unable to allocate rbf->nvoxels_region\n");
            return NULL;
            } 
        if(!(rbf->harolds_num=malloc(sizeof*rbf->harolds_num*num_regions))) {
            printf("Error: Unable to allocate rbf->harolds_num\n");
            return NULL;
            }
        for(n=m=0;m<num_region_files;m++) if(reg[m]->length) n++;
        if(n==num_region_files) {
            if(!(rbf->region_names_ptr=malloc(sizeof*rbf->region_names_ptr*num_regions))) { 
                printf("Error: Unable to allocate rbf->region_names_ptr\n");
                return NULL;
                }
            if(!(rbf->length=malloc(sizeof*rbf->length*num_regions))) {
                printf("Error: Unable to allocate rbf->length\n");
                return NULL;
                }
            for(rbf->max_length=n=m=0;m<num_region_files;m++) {
                for(j=0;j<rbf->num_regions_by_file[m];j++,n++) {
                    rbf->region_names_ptr[index[n]] = reg[m]->region_names[rbf->roi_by_file[m][j]];

                    /*printf("here100\n");fflush(stdout);
                    printf("here100 index[%d]=%d\n",n,index[n]);fflush(stdout);
                    printf("here100 rbf->region_names_ptr[%d]=%s\n",index[n],rbf->region_names_ptr[index[n]]);fflush(stdout);*/

                    if((rbf->length[index[n]]=reg[m]->length[rbf->roi_by_file[m][j]]) > rbf->max_length)
                        rbf->max_length = rbf->length[n];
                    }
                }
            }
        else {
            rbf->region_names_ptr = (char**)NULL;
            rbf->length = (int*)NULL;
            }

        /*for(j=0;j<num_regions;j++){printf("rbf->region_names_ptr[%d]=%s\n",j,rbf->region_names_ptr[j]);fflush(stdout);}*/




        for(rbf->nvoxels_region_max=rbf->nvoxels=j=m=0;m<num_region_files;m++) {
            for(i=0;i<rbf->num_regions_by_file[m];i++,j++) {
                rbf->nvoxels += rbf->nvoxels_region[index[j]] = reg[m]->nvoxels_region[rbf->roi_by_file[m][i]];
                rbf->harolds_num[index[j]] = reg[m]->harolds_num[rbf->roi_by_file[m][i]];
                if(rbf->nvoxels_region_max < reg[m]->nvoxels_region[rbf->roi_by_file[m][i]]) 
                    rbf->nvoxels_region_max = reg[m]->nvoxels_region[rbf->roi_by_file[m][i]];
                }
            }
        if(!(rbf->indices=malloc(sizeof*rbf->indices*rbf->nvoxels))) {
            printf("Error: Unable to allocate rbf->indices\n");
            return NULL;
            }
        if(reg[0]->voxel_indices) {
            for(j1=0,m=0;m<num_region_files;m++) {
                for(i=0;i<rbf->num_regions_by_file[m];i++,j1++) {

                    /*for(n=l=0;l<index[k];l++) n += rbf->nvoxels_region[l];*/
                    /*START150610*/
                    for(n=0,i1=0;i1<index[j1];i1++)n+=rbf->nvoxels_region[i1]; 

                    for(j=0;j<reg[m]->nvoxels_region[rbf->roi_by_file[m][i]];j++) {
                        rbf->indices[n+j] = reg[m]->voxel_indices[rbf->roi_by_file[m][i]][j];
                        }
                    }
                }
            }


        #if 0
        if(!(roid=malloc(sizeof*roid*rbf->nvoxels))) {
            printf("Error: Unable to allocate roid\n");
            return NULL;
            }
        for(m=0;m<rbf->nvoxels;m++) roid[m]=(double)rbf->indices[m];
        gsl_sort(roid,1,rbf->nvoxels);
        for(n=rbf->nvoxels,m=0;m<n-1;) {
            if(roid[m]==roid[m+1]) {
                for(i=m+1;i<n;i++) roid[i-1]=roid[i];
                n--;
                }
            else {
                m++;
                }
            }
        rbf->nindices_uniqsort=n;
        if(!(rbf->indices_uniqsort=malloc(sizeof*rbf->indices_uniqsort*rbf->nindices_uniqsort))) {
            printf("Error: Unable to allocate rbf->indices_uniqsort\n");
            return NULL;
            }
        for(m=0;m<rbf->nindices_uniqsort;m++) rbf->indices_uniqsort[m]=(int)roid[m]; 
        free(roid);
        #endif
        //START160726
        if(!(rbf->indices_uniqsort=malloc(sizeof*rbf->indices_uniqsort*rbf->nvoxels))) {
            printf("Error: Unable to allocate rbf->indices_uniqsort\n");
            return NULL;
            }
        for(m=0;m<rbf->nvoxels;m++) rbf->indices_uniqsort[m]=rbf->indices[m];
        gsl_sort_int(rbf->indices_uniqsort,1,rbf->nvoxels);
        for(n=rbf->nvoxels,m=0;m<n-1;) {
            if(rbf->indices_uniqsort[m]==rbf->indices_uniqsort[m+1]) {
                for(i=m+1;i<n;i++) rbf->indices_uniqsort[i-1]=rbf->indices_uniqsort[i];
                n--;
                }
            else {
                m++;
                }
            }
        rbf->nindices_uniqsort=n;
        if(n<rbf->nvoxels)rbf->indices_uniqsort=realloc(rbf->indices_uniqsort,n*sizeof*rbf->indices_uniqsort);



        free(index);
        free(roinew);
        free(nregions);
        }
    return rbf;
}
Regions_By_File *find_regions_by_file(int num_region_files,int num_regions,int *nregions,int *roi)
{
    int i,j,k,m,n,region_count=0;
    Regions_By_File *rbf;
    if(!(rbf=malloc(sizeof*rbf))) {
        printf("Error: Unable to allocate rbf\n");
        return NULL;
        }
    if(!(rbf->num_regions_by_file=malloc(sizeof*rbf->num_regions_by_file*num_region_files))) {
        printf("Error: Unable to allocate rbf->num_regions_by_file\n");
        return NULL;
        }
    for(i=0;i<num_region_files;i++) rbf->num_regions_by_file[i]=0;
    if(!(rbf->file_index=malloc(sizeof*rbf->file_index*num_regions))) {
        printf("Error: Unable to allocate rbf->file_index\n");
        return NULL;
        }
    if(!(rbf->roi_index=malloc(sizeof*rbf->roi_index*num_regions))) {
        printf("Error: Unable to allocate rbf->roi_index\n");
        return NULL;
        }

    #if 0
    for(region_count=i=0;region_count<num_regions;) {
        for(n=m=0;m<num_region_files&&region_count<num_regions;m++) {
            k=0;
            if(roi[i]==-1) {
                rbf->file_index[region_count]=-1;
                rbf->roi_index[region_count]=-1;
                k++;i++;region_count++;
                }
            else {
                for(;roi[i]>=n&&roi[i]<nregions[m]+n;) {
                    /*printf("heree roi[%d]=%d nregions[%d]=%d\n",i,roi[i],m,nregions[m]); fflush(stdout);*/
                    for(j=n;j<nregions[m]+n;j++) {
                        if(j == roi[i]) {
                            rbf->file_index[region_count]=m;
                            rbf->roi_index[region_count]=roi[i]-n;
                            k++;i++;region_count++;
                            break;
                            }
                        }
                    if(region_count == num_regions) break;
                    }
                }
            rbf->num_regions_by_file[m] += k;
            n += nregions[m]; /*n must be incremented before m*/
            }
        }
    #endif
    /*START120807*/
    for(region_count=i=0;region_count<num_regions;) {
        for(n=m=0;m<num_region_files&&region_count<num_regions;m++) {
            k=0;
            if(roi[i]==-1) {
                rbf->file_index[region_count]=-1;
                rbf->roi_index[region_count]=-1;
                k++;i++;region_count++;
                }
            else {
                /*printf("hereA roi[%d]=%d nregions[%d]=%d n=%d\n",i,roi[i],m,nregions[m],n); fflush(stdout);*/
                for(;roi[i]>=n&&roi[i]<nregions[m]+n;) {
                    /*printf("hereB roi[%d]=%d nregions[%d]=%d n=%d\n",i,roi[i],m,nregions[m],n); fflush(stdout);*/
                    for(j=n;j<nregions[m]+n;j++) {
                        if(j == roi[i]) {
                            rbf->file_index[region_count]=m;
                            rbf->roi_index[region_count]=roi[i]-n;
                            k++;i++;region_count++;
                            break;
                            }
                        }
                    /*printf("hereC region_count=%d num_regions=%d\n",region_count,num_regions);fflush(stdout);*/
                    if(region_count == num_regions) break;
                    }
                }
            /*printf("hereD\n");fflush(stdout);*/
            rbf->num_regions_by_file[m] += k;
            n += nregions[m]; /*n must be incremented before m*/
            }
        }


    if(!(rbf->roi_by_file=d2intvar(num_region_files,rbf->num_regions_by_file))) return NULL;
    for(m=0;m<num_region_files;m++) rbf->num_regions_by_file[m]=0;
    for(region_count=i=0;region_count<num_regions;) {
        for(n=m=0;m<num_region_files&&region_count<num_regions;m++) {
            k=0;
            if(roi[i]==-1) {
                rbf->roi_by_file[m][rbf->num_regions_by_file[m]++] = roi[i++];
                k++;
                }
            else {
                for(;roi[i]>=n&&roi[i]<nregions[m]+n;) {
                    for(j=n;j<nregions[m]+n;j++) {
                        if(j == roi[i]) {
                            rbf->roi_by_file[m][rbf->num_regions_by_file[m]++] = roi[i++] - n;
                            k++;
                            break;
                            }
                        }
                    if(k+region_count == num_regions) break;
                    }
                }
            region_count += k;
            n += nregions[m]; /*n must be incremented before m*/
            }
        }
    for(rbf->num_regions_by_file_max=i=0;i<num_region_files;i++) 
        if(rbf->num_regions_by_file[i]>rbf->num_regions_by_file_max) rbf->num_regions_by_file_max=rbf->num_regions_by_file[i];
    rbf->nvoxels_region = NULL;
    rbf->length = NULL;
    rbf->indices = NULL;
    rbf->indices_uniqsort = NULL;
    rbf->indices_uniqsort = NULL;
    rbf->harolds_num = NULL;
    rbf->region_names_ptr = NULL;
    return rbf;
}
void free_regions_by_file(Regions_By_File *rbf)
{
    if(rbf->nvoxels_region) free(rbf->nvoxels_region);
    if(rbf->length) free(rbf->length);
    if(rbf->indices) free(rbf->indices);
    if(rbf->indices_uniqsort) free(rbf->indices_uniqsort);
    if(rbf->harolds_num) free(rbf->harolds_num);
    if(rbf->region_names_ptr) free(rbf->region_names_ptr);
    free(rbf->roi_by_file[0]);
    free(rbf->roi_by_file);
    free(rbf->file_index);
    free(rbf->roi_index);
    free(rbf->num_regions_by_file);
    free(rbf);
}

int crs(double *temp_double,double *region_stack,Regions_By_File *rbf,char *glmfile)
{
    int i,j,k,count;
    for(k=i=0;i<rbf->nreg;i++) {
        for(region_stack[i]=0.,count=j=0;j<rbf->nvoxels_region[i];j++,k++) {
            if(temp_double[rbf->indices[k]]!=(double)UNSAMPLED_VOXEL&&temp_double[rbf->indices[k]]!= 0.&&
                isfinite(temp_double[rbf->indices[k]])){
                region_stack[i] += temp_double[rbf->indices[k]];
                count++;
                }
            }
        count ? (region_stack[i]/=(double)count) : (region_stack[i]=(double)UNSAMPLED_VOXEL);
        if(!count&&glmfile) {
            printf("Error: Region %s has zero voxels in %s\n",rbf->region_names_ptr[i],glmfile);
            return 0;
            }
        }
    return 1;
}

/*START130815*/
#if 0
KEEP
int crs(double *temp_double,double *region_stack,int nreg,int *nvoxels_region,int *indices,int *maskidx,char **region_names_ptr,
    char *glmfile) {
    int i,j,k,count;
    for(k=i=0;i<nreg;i++) {
        for(region_stack[i]=0.,count=j=0;j<nvoxels_region[i];j++,k++) {
            td = !maskidx ? temp_double[indices[k]] : temp_double[maskidx[indices[k]]];
            if(td!=(double)UNSAMPLED_VOXEL && td!= 0. &&
              #ifdef __sun__
                !IsNANorINF(td)
              #else
                (!isnan(td) || !isinf(td))
              #endif
                ) {
                region_stack[i] += td;
                count++;
                }
            }
        count ? (region_stack[i]/=(double)count) : (region_stack[i]=(double)UNSAMPLED_VOXEL);
        if(!count&&glmfile) {
            printf("Error: Region %s has zero voxels in %s\n",region_names_ptr[i],glmfile);
            return 0;
            }
        }
    return 1;
    }
#endif

                                                                                        /*rbf->indices*/
void crsw(double *temp_double,double *region_stack,Regions_By_File *rbf,double *weights,int *weightsi)
{
    int i,j,k;
    double weights_sum;
    for(k=i=0;i<rbf->nreg;i++) {
        for(weights_sum=region_stack[i]=0.,j=0;j<rbf->nvoxels_region[i];j++,k++) {

            /*printf("%e ",temp_double[rbf->indices[k]]);
            if(fabs(temp_double[rbf->indices[k]])>(double)UNSAMPLED_VOXEL) printf("yes "); else printf("no ");*/

            #if 0
            if(temp_double[rbf->indices[k]] != (double)UNSAMPLED_VOXEL && temp_double[rbf->indices[k]] != 0. &&

              /*#ifndef LINUX*/
              #ifdef __sun__

                !IsNANorINF(temp_double[rbf->indices[k]])
              #else
                (!isnan(temp_double[rbf->indices[k]]) || !isinf(temp_double[rbf->indices[k]]))
              #endif
                ) {
                region_stack[i] += temp_double[rbf->indices[k]]*weights[weightsi[k]];
                weights_sum += weights[weightsi[k]];
                }
            #endif
            //START170724
            if(temp_double[rbf->indices[k]]!=(double)UNSAMPLED_VOXEL && temp_double[rbf->indices[k]]!=0. &&
                isfinite(temp_double[rbf->indices[k]])){
                region_stack[i] += temp_double[rbf->indices[k]]*weights[weightsi[k]];
                weights_sum += weights[weightsi[k]];
                }


            }
        /*printf("\ni=%d weights_sum=%f region_stack=%f\n",i,weights_sum,region_stack[i]);*/
        weights_sum ? (region_stack[i]/=weights_sum) : (region_stack[i]=(double)UNSAMPLED_VOXEL);
        /*printf("after i=%d weights_sum=%f region_stack=%f\n",i,weights_sum,region_stack[i]);*/
        }
}

