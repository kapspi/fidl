/* Copyright 10/20/05 Washington University.  All Rights Reserved.
   subs_scratch.c  $Revision: 1.14 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include "matrix.h"
#include "fidl.h"
#include "gauss_smoth2.h"
#include "subs_scratch.h"
Files_Struct_new *create_scratchnames(Files_Struct *tc_files,Regions_By_File *fbf,char *scratchdir,char *appendstr){
    char filename[MAXNAME],*str_ptr,*strptr,write_back[MAXNAME];

    //int j,k,m;
    //START170616
    size_t j1,k1,m1;

    Files_Struct_new *sn;
    if(!(sn=malloc(sizeof*sn))) {
        printf("Error: Unable to malloc sn\n");
        return NULL;
        }

    #if 0
    for(sn->nfiles=m=0;m<tc_files->nfiles;m++) {
        /*printf("fbf->num_regions_by_file[%d]=%d\n",m,fbf->num_regions_by_file[m]);*/
        if(fbf->num_regions_by_file[m]) ++sn->nfiles;
        }
    /*printf("sn->nfiles=%d\n",sn->nfiles);*/
    #endif
    //START170616
    for(sn->nfiles=m1=0;m1<tc_files->nfiles;m1++)if(fbf->num_regions_by_file[m1])++sn->nfiles;

    if(!sn->nfiles) {
        printf("Error: sn->files=%d\n",sn->nfiles);
        return NULL;
        }
    if(!(sn->strlen_files=malloc(sizeof*sn->strlen_files*sn->nfiles))) {
        printf("fidlError: Unable to malloc sn->strlen_files in create_scratchnames\n");
        return NULL;
        }

    #if 0
    for(k=j=m=0;m<tc_files->nfiles;m++) {
        if(fbf->num_regions_by_file[m]) {
            strcpy(write_back,tc_files->files[m]);
            if(!(str_ptr=get_tail_sans_ext(write_back))) return NULL;
            sprintf(filename,"%s%s%s.4dfp.img",scratchdir,str_ptr,appendstr);
            k += sn->strlen_files[j++] = strlen(filename)+1;
            }
        }
    if(!(sn->files=malloc(sizeof*sn->files*k))) {
        printf("Error: Unable to malloc sn->files in create_scratchnames\n");
        return NULL;
        }
    for(strptr=sn->files,j=m=0;m<tc_files->nfiles;m++) {
        if(fbf->num_regions_by_file[m]) {
            strcpy(filename,tc_files->files[m]);
            if(!(str_ptr=get_tail_sans_ext(filename))) return NULL;
            sprintf(strptr,"%s%s%s.4dfp.img",scratchdir,str_ptr,appendstr);
            strptr += sn->strlen_files[j++];
            }
        }
    #endif
    //START170616
    for(k1=j1=m1=0;m1<tc_files->nfiles;m1++){
        if(fbf->num_regions_by_file[m1]){
            strcpy(write_back,tc_files->files[m1]);
            if(!(str_ptr=get_tail_sans_ext(write_back)))return NULL;
            sprintf(filename,"%s%s%s.4dfp.img",scratchdir,str_ptr,appendstr);
            k1+=sn->strlen_files[j1++]=strlen(filename)+1;
            }
        }
    if(!(sn->files=malloc(sizeof*sn->files*k1))) {
        printf("fidlError: Unable to malloc sn->files in create_scratchnames\n");
        return NULL;
        }
    for(strptr=sn->files,j1=m1=0;m1<tc_files->nfiles;m1++){
        if(fbf->num_regions_by_file[m1]){
            strcpy(filename,tc_files->files[m1]);
            if(!(str_ptr=get_tail_sans_ext(filename)))return NULL;
            sprintf(strptr,"%s%s%s.4dfp.img",scratchdir,str_ptr,appendstr);
            strptr+=sn->strlen_files[j1++];
            }
        }


    return sn;
    }

#if 0
int create_scratchfiles(Files_Struct *tc_files,Regions_By_File *fbf,Dim_Param *dp,Files_Struct_new *sn,int SunOS_Linux,
    char *xform_file,Atlas_Param *ap,int num_wfiles,Regions_By_File *rbf,int num_regions,Mask_Struct *ms,
    Files_Struct *wfiles,int fs_nframes,float fwhm){
#endif
//START170616
int create_scratchfiles(Files_Struct *tc_files,Regions_By_File *fbf,Dim_Param2 *dp,Files_Struct_new *sn,int SunOS_Linux,
    char *xform_file,Atlas_Param *ap,int num_wfiles,Regions_By_File *rbf,int num_regions,Mask_Struct *ms,
    Files_Struct *wfiles,int fs_nframes,float fwhm){


    int i,j,k,l,m,p,A_or_B_or_U,swapbytes;

    //START170616
    size_t m1;

    float *temp_float,*t4;
    double *temp_double,*stat,*weights;
    char *strptr;
    Memory_Map *mm,**mm_wfiles=NULL;
    FILE *fp;
    gauss_smoth2_struct *gs=NULL;
    if(!(temp_float=malloc(sizeof*temp_float*ap->vol))) {
        printf("Error: Unable to malloc temp_float in create_scratchfiles\n");
        return 0;
        }
    if(!(temp_double=malloc(sizeof*temp_double*ap->vol))) {
        printf("Error: Unable to malloc temp_double in create_scratchfiles\n");
        return 0;
        }
    if(!(stat=malloc(sizeof*stat*ap->vol))) {
        printf("Error: Unable to malloc stat in create_scratchfiles\n");
        return 0;
        }
    if(rbf) {
        if(!(weights=malloc(sizeof*weights*ap->vol))) {
            printf("Error: Unable to malloc weights in create_scratchfiles\n");
            return 0;
            }
        if(wfiles) {
            if(!check_dimensions(num_wfiles,wfiles->files,ap->vol)) return 0;
            if(!(mm_wfiles=malloc(sizeof*mm_wfiles*num_wfiles))) {
                printf("Error: Unable to malloc mm_wfiles in create_scratchfiles\n");
                return 0;
                }

            //for(i=0;i<num_wfiles;i++) if(!(mm_wfiles[i]=map_disk(wfiles->files[i],ap->vol,0))) return 0;
            //START170616
            for(i=0;i<num_wfiles;i++)if(!(mm_wfiles[i]=map_disk(wfiles->files[i],ap->vol,0,sizeof(float))))return 0;

            }
        else {
            for(i=0;i<ap->vol;i++) weights[i] = 1;
            }
        }
    if(xform_file) {
        if(!(t4=malloc(sizeof*t4*(size_t)T4SIZE))) {
            printf("Error: Unable to malloc t4 in create_scratchfiles\n");
            return 0;
            }
        if(!read_xform(xform_file,t4)) return 0;
        if((A_or_B_or_U=twoA_or_twoB(xform_file))==2) {
            printf("Error: Illegally named t4 file.\n");
            return 0;
            }
        }
    if(fwhm>0.){if(!(gs=gauss_smoth2_init(ap->xdim,ap->ydim,ap->zdim,fwhm,fwhm)))return 0;}


    #if 0
    for(strptr=sn->files,k=l=m=0;m<tc_files->nfiles;m++) {
        if(fbf->num_regions_by_file[m]) {
            if(!(mm=map_disk(tc_files->files[m],dp->vol[m]*dp->tdim[m],0))) return 0;
            swapbytes=shouldiswap(SunOS_Linux,dp->bigendian[m]);
            if(!(fp=fopen_sub(strptr,"w"))) return 0;
            printf("Processing %s\nPrinting to %s\n",tc_files->files[m],strptr);
            for(i=0;i<fbf->num_regions_by_file[m];i++,k++) {
                p=dp->vol[m]*fbf->roi_by_file[m][i];
                if(!swapbytes) {
                    for(j=0;j<dp->vol[m];j++) temp_double[j] = mm->ptr[p+j]==(float)UNSAMPLED_VOXEL ? (double)UNSAMPLED_VOXEL :
                        (double)mm->ptr[p+j];
                    }
                else {
                    for(j=0;j<dp->vol[m];j++) temp_float[j] = mm->ptr[p+j];
                    swap_bytes((unsigned char *)temp_float,sizeof(float),(size_t)dp->vol[m]);
                    for(j=0;j<dp->vol[m];j++) temp_double[j] = temp_float[j]==(float)UNSAMPLED_VOXEL ? (double)UNSAMPLED_VOXEL :
                        (double)temp_float[j];
                    }
                if(xform_file) {
                    if(!t4_atlas(temp_double,stat,t4,dp->xdim[m],dp->ydim[m],dp->zdim[m],dp->dxdy[m],dp->dz[m],A_or_B_or_U,
                        dp->orientation[m],ap))return 0;
                    for(j=0;j<ap->vol;j++) temp_double[j] = stat[j];
                    }
                if(fwhm > 0.) {
                    gauss_smoth2(temp_double,stat,gs);
                    for(j=0;j<ap->vol;j++) temp_double[j] = stat[j];
                    }
                for(p=0;p<num_wfiles;p++) {
                    if(rbf) {
                        if(mm_wfiles) for(j=0;j<ap->vol;j++) weights[j] = (double)mm_wfiles[p]->ptr[j];
                        crsw(temp_double,stat,rbf,weights,rbf->indices);
                        for(j=0;j<num_regions;j++) temp_double[j] = stat[j];
                        for(j=0;j<num_regions;j++) printf("%f ",temp_double[j]); printf("\n");
                        }
                    else {
                        for(j=0;j<ms->lenbrain;j++) temp_double[j] = temp_double[ms->brnidx[j]];
                        }
                    if(!(fwrite_sub(temp_double,sizeof(double),(size_t)num_regions,fp,0))) {
                        printf("Error writing to %s. Are you out of memory? Abort!\n",strptr);
                        return 0;
                        }
                    }
                }
            if(!unmap_disk(mm)) return 0;
            fclose(fp);
            strptr += sn->strlen_files[l++];
            }
        }
    #endif
    //START170616
    for(strptr=sn->files,k=l=m1=0;m1<tc_files->nfiles;m1++){
        if(fbf->num_regions_by_file[m1]){
            if(!(mm=map_disk(tc_files->files[m1],dp->vol[m1]*dp->tdim[m1],0))) return 0;
            swapbytes=shouldiswap(SunOS_Linux,dp->bigendian[m1]);
            if(!(fp=fopen_sub(strptr,"w")))return 0;
            printf("Processing %s\nPrinting to %s\n",tc_files->files[m1],strptr);
            for(i=0;i<fbf->num_regions_by_file[m1];i++,k++) {
                p=dp->vol[m1]*fbf->roi_by_file[m1][i];
                if(!swapbytes) {
                    for(j=0;j<dp->vol[m1];j++)temp_double[j]=mm->ptr[p+j]==(float)UNSAMPLED_VOXEL?(double)UNSAMPLED_VOXEL:
                        (double)mm->ptr[p+j];
                    }
                else {
                    for(j=0;j<dp->vol[m1];j++)temp_float[j]=mm->ptr[p+j];
                    swap_bytes((unsigned char *)temp_float,sizeof(float),(size_t)dp->vol[m1]);
                    for(j=0;j<dp->vol[m1];j++)temp_double[j]=temp_float[j]==(float)UNSAMPLED_VOXEL?(double)UNSAMPLED_VOXEL:
                        (double)temp_float[j];
                    }
                if(xform_file) {
                    if(!t4_atlas(temp_double,stat,t4,dp->xdim[m1],dp->ydim[m1],dp->zdim[m1],dp->dxdy[m1],dp->dz[m1],A_or_B_or_U,
                        dp->orientation[m1],ap))return 0;
                    for(j=0;j<ap->vol;j++) temp_double[j] = stat[j];
                    }
                if(fwhm>0.){
                    gauss_smoth2(temp_double,stat,gs);
                    for(j=0;j<ap->vol;j++)temp_double[j]=stat[j];
                    }
                for(p=0;p<num_wfiles;p++) {
                    if(rbf) {
                        if(mm_wfiles)for(j=0;j<ap->vol;j++) weights[j] = (double)mm_wfiles[p]->ptr[j];
                        crsw(temp_double,stat,rbf,weights,rbf->indices);
                        for(j=0;j<num_regions;j++) temp_double[j] = stat[j];
                        for(j=0;j<num_regions;j++) printf("%f ",temp_double[j]); printf("\n");
                        }
                    else {
                        for(j=0;j<ms->lenbrain;j++) temp_double[j] = temp_double[ms->brnidx[j]];
                        }
                    if(!(fwrite_sub(temp_double,sizeof(double),(size_t)num_regions,fp,0))) {
                        printf("fidlError writing to %s. Are you out of memory? Abort!\n",strptr);
                        return 0;
                        }
                    }
                }
            if(!unmap_disk(mm)) return 0;
            fclose(fp);
            strptr += sn->strlen_files[l++];
            }
        }


    if(k!=fs_nframes) {
        printf("Error: k = %d. It should equal %d. Abort!\n",k,fs_nframes);
        return 0;
        }
    if(wfiles) {
        for(i=0;i<num_wfiles;i++) if(!unmap_disk(mm_wfiles[i])) return 0;
        free(mm_wfiles);
        }
    if(rbf) free(weights);
    if(xform_file) free(t4);
    free(temp_float);
    free(temp_double);
    free(stat);
    return 1;
    }


void assignXnew(int num_tc_files,int *num_regions_by_file,Files_Struct_new *sn,int num_regions,int num_wfiles,int goose,int j,int p,
    int *sindex,FS *fs,double **X,Scratch *s)
{
    char *strptr;
    int i,k,l,m,q,uns_count,small,index;
    Memory_Map *mm;

    for(strptr=sn->files,q=1,small=uns_count=l=m=0;m<num_tc_files && !uns_count;m++) {
        if(num_regions_by_file[m]) {
            if(!(mm=map_disk_double(strptr,num_regions*num_regions_by_file[m]*num_wfiles,0))) exit(-1);
            if(goose==2) for(i=1;i<=fs->nlines;i++) X[1][i]=1.;
            for(k=goose,i=0;i<num_regions_by_file[m] && !uns_count;i++) {
                /*printf("X[%d][%d]=%g\n",sindex[q-1]+k,q,mm->dptr[num_regions*(p+num_wfiles*i)+j]);*/


                #if 0
                if((X[sindex[q-1]+k][q]=mm->dptr[num_regions*(p+num_wfiles*i)+j]) == (double)UNSAMPLED_VOXEL) uns_count++;
                if(fabs(X[sindex[q-1]+k][q]) < (double)UNSAMPLED_VOXEL) {
                    small++;
                    }
                #endif

                index = sindex[q-1]+k;
                if((X[index][q]=mm->dptr[num_regions*(p+num_wfiles*i)+j]) == (double)UNSAMPLED_VOXEL) uns_count++;
                if(fabs(X[index][q]) < (double)UNSAMPLED_VOXEL) small++;



                if(++k == (fs->frames_per_line[q-1]+goose)) {
                    k = goose;
                    q++;
                    }
                }
            if(!unmap_disk(mm)) exit(-1);
            strptr += sn->strlen_files[l++];
            }
        }
    s->uns_count = uns_count;
    s->small = small;
}


/*START140729*/
#if 0
void assignX(int num_tc_files,int *num_regions_by_file,Files_Struct_new *sn,int num_regions,int num_wfiles,int goose,int j,
    int p,int *sindex,FS *fs,MAT *X,Scratch *s)
{
    char *strptr;
    int i,k,l,m,q,uns_count,small,index;
    Memory_Map *mm;

    if(goose==1) for(i=0;i<fs->nlines;i++) X->me[0][i]=1.;
    for(strptr=sn->files,q=0,small=uns_count=l=m=0;m<num_tc_files && !uns_count;m++) {
        if(num_regions_by_file[m]) {
            if(!(mm=map_disk_double(strptr,num_regions*num_regions_by_file[m]*num_wfiles,0))) exit(-1);
            /*if(goose==1) for(i=0;i<fs->nlines;i++) X->me[0][i]=1.;*/
            for(k=goose,i=0;i<num_regions_by_file[m] && !uns_count;i++) {
                /*printf("sindex[%d]=%d k=%d X[%d][%d]=%g\n",q,sindex[q],k,sindex[q]+k,q,
                    mm->dptr[num_regions*(p+num_wfiles*i)+j]);*/
                index = sindex[q]+k;
                if((X->me[index][q]=mm->dptr[num_regions*(p+num_wfiles*i)+j]) == (double)UNSAMPLED_VOXEL) uns_count++;
                if(fabs(X->me[index][q]) < (double)UNSAMPLED_VOXEL) small++;
                if(++k == (fs->frames_per_line[q]+goose)) {
                    k = goose;
                    q++;
                    }
                }
            if(!unmap_disk(mm)) exit(-1);
            strptr += sn->strlen_files[l++];
            }
        }
    s->uns_count = uns_count;
    s->small = small;
    /*printf("here101\n");*/
}
#endif


void assignXgsl(int num_tc_files,int *num_regions_by_file,Files_Struct_new *sn,int num_regions,int num_wfiles,int goose,int j,
    int p,int *sindex,FS *fs,gsl_matrix *X,Scratch *s)
{
    char *strptr;
    int i,k,l,m,q,uns_count,small,index,i2;
    Memory_Map *mm;
    if(goose==1) for(i=0;i<fs->nlines;i++) gsl_matrix_set(X,0,i,1.);
    for(strptr=sn->files,q=0,small=uns_count=l=m=0;m<num_tc_files && !uns_count;m++) {
        if(num_regions_by_file[m]) {
            if(!(mm=map_disk_double(strptr,num_regions*num_regions_by_file[m]*num_wfiles,0))) exit(-1);
            /*if(goose==1) for(i=0;i<fs->nlines;i++) gsl_matrix_set(X,0,i,1.);*/
            for(k=goose,i=0;i<num_regions_by_file[m] && !uns_count;i++) {
                index = sindex[q]+k;
                i2 = num_regions*(p+num_wfiles*i)+j;
                gsl_matrix_set(X,index,q,mm->dptr[i2]);
                if(mm->dptr[i2] == (double)UNSAMPLED_VOXEL) uns_count++;
                if(fabs(mm->dptr[i2]) < (double)UNSAMPLED_VOXEL) small++;
                if(++k == (fs->frames_per_line[q]+goose)) {
                    k = goose;
                    q++;
                    }
                }
            if(!unmap_disk(mm)) exit(-1);
            strptr += sn->strlen_files[l++];
            }
        }
    s->uns_count = uns_count;
    s->small = small;
}
void assignXtrblockgsl(int num_tc_files,int *num_regions_by_file,Files_Struct_new *sn,int num_regions,int num_wfiles,int goose,int j,
    int p,int *sindex,FS *fs,double *X,Scratch *s,int lengthX)
{
    char *strptr;
    int i,k,l,m,q,uns_count,small,index,i2;
    Memory_Map *mm;
    gsl_vector_view col;
    gsl_matrix_view Xgsl = gsl_matrix_view_array(X,fs->nlines,lengthX);
    if(goose==1) {
        col = gsl_matrix_column(&Xgsl.matrix,0);
        gsl_vector_set_all(&col.vector,1.0);
        }
    for(strptr=sn->files,q=0,small=uns_count=l=m=0;m<num_tc_files && !uns_count;m++) {
        if(num_regions_by_file[m]) {
            if(!(mm=map_disk_double(strptr,num_regions*num_regions_by_file[m]*num_wfiles,0))) exit(-1);
            for(k=goose,i=0;i<num_regions_by_file[m] && !uns_count;i++) {
                index = sindex[q]+k;
                i2 = num_regions*(p+num_wfiles*i)+j;
                gsl_matrix_set(&Xgsl.matrix,q,index,mm->dptr[i2]);
                if(mm->dptr[i2] == (double)UNSAMPLED_VOXEL) uns_count++;
                if(fabs(mm->dptr[i2]) < (double)UNSAMPLED_VOXEL) small++;
                if(++k == (fs->frames_per_line[q]+goose)) {
                    k = goose;
                    q++;
                    }
                }
            if(!unmap_disk(mm)) exit(-1);
            strptr += sn->strlen_files[l++];
            }
        }
    s->uns_count = uns_count;
    s->small = small;
}
