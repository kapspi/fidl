/* Copyright 8/13/07 Washington University.  All Rights Reserved.
   fidl_corrvox.c  $Revision: 1.4 $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fidl.h>

//START170724
typedef struct {
    int *nsampledvoxels;
    double *r;
    } Region_Pearson;
Region_Pearson *region_pearson(double *temp_double1,double *temp_double2,Regions_By_File *rbf);


main(int argc,char **argv)
{
char *xform_file=NULL,*output="fidl_corrvox.txt";

int i,j,k,l,m,jj,num_bold_files=0,num_target_files=0,num_regions=0,num_region_files=0,*roi=NULL,count,A_or_B_or_U,atlas,SunOS_Linux,
    swapbytes_bold,swapbytes_target;
 
float *t4,*temp_float;

double *temp_double_bold,*temp_double_target,*stat;

FILE *op;
Interfile_header *ifh;
Regions **reg;
Regions_By_File *rbf;
Region_Pearson **rp;
Dim_Param *dp_bold,*dp_target;
Atlas_Param *ap;
Memory_Map *mm_bold,*mm_target;
Files_Struct *bold_files,*target_files,*region_files;
Mask_Struct *ms;

if (argc < 5) {
    fprintf(stderr,"        -bold_files:               Correlation is between the bold and target.\n");
    fprintf(stderr,"        -target_files:             Correlation is between the bold and target.\n");
    fprintf(stderr,"        -region_file:              *.4dfp.img file(s) that specifies regions of interest.\n");
    fprintf(stderr,"        -regions_of_interest:      First region is one.\n");
    fprintf(stderr,"        -xform_file:               t4 file defining the transform to atlas space.\n");
    fprintf(stderr,"        -output:                   Name of text file that correlations are written to.\n");
    exit(-1);
    }

for(i=1;i<argc;i++) {
    if(!strcmp(argv[i],"-bold_files") && argc > i+1) {
        for(j=1;i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_bold_files;
        if(!(bold_files=get_files(num_bold_files,&argv[i+1]))) exit(-1);
        i += num_bold_files;
        }
    if(!strcmp(argv[i],"-target_files") && argc > i+1) {
        for(j=1;i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_target_files;
        if(!(target_files=get_files(num_target_files,&argv[i+1]))) exit(-1);
        i += num_target_files;
        }
    if(!strcmp(argv[i],"-region_file") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_region_files;
        if(!(region_files=get_files(num_region_files,&argv[i+1]))) exit(-1);
        i += num_region_files;
        }
    if(!strcmp(argv[i],"-regions_of_interest") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_regions;
        if(!(roi=malloc(sizeof*roi*num_regions))) {
            printf("Error: Unable to malloc roi\n");
            exit(-1);
            }
        for(j=0;j<num_regions;j++) roi[j] = atoi(argv[++i]) - 1;
        }
    if(!strcmp(argv[i],"-xform_file") && argc > i+1)
        xform_file = argv[++i];
    if(!strcmp(argv[i],"-output") && argc > i+1)
        output = argv[++i];
    }
if(!num_bold_files) {
    printf("Error: No -bold_files  Abort!\n");
    exit(-1);
    }
if(!num_target_files) {
    printf("Error: No -target_files  Abort!\n");
    exit(-1);
    }
if(num_bold_files!=num_target_files) {
    printf("Error: num_bold_files=%d num_target_files=%d  Must be equal. Abort!\n",num_bold_files,num_target_files);
    exit(-1);
    }
if(!num_region_files) {
    printf("No region files. Abort!\n");
    exit(-1);
    }
if((SunOS_Linux=checkOS())==-1) exit(-1);
printf("SunOS_Linux=%d\n",SunOS_Linux);
if(!(dp_bold=dim_param(num_bold_files,bold_files->files,SunOS_Linux))) exit(-1);
if(!(dp_target=dim_param(num_target_files,target_files->files,SunOS_Linux))) exit(-1);
if(dp_bold->vol!=dp_target->vol) {
    printf("Error: dp_bold->vol=%d dp_target->vol=%d Must be equal. Abort!\n",dp_bold->vol,dp_target->vol);
    exit(-1);
    }
if(dp_bold->tdim_total!=dp_target->tdim_total) {
    printf("Error: dp_bold->tdim_total=%d dp_target->tdim_total=%d Must be equal. Abort!\n",dp_bold->tdim_total,
        dp_target->tdim_total);
    exit(-1);
    }
if(xform_file) {
    if(!(t4=malloc(sizeof*t4*(size_t)T4SIZE))) {
        printf("Error: Unable to malloc t4\n");
        exit(-1);
        }
    if(!read_xform(xform_file,t4)) exit(-1);
    if((A_or_B_or_U=twoA_or_twoB(xform_file)) == 2) {
        fprintf(stdout,"Error: Illegally named t4 file.\n");
        exit(-1);
        }
    atlas = 222;
    }
else {
    atlas = get_atlas(dp_bold->vol);
    }
if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL))) exit(-1);
if(!xform_file) ap->vol = dp_bold->vol;
if(!atlas) {
    ap->xdim = dp_bold->xdim;
    ap->ydim = dp_bold->ydim;
    ap->zdim = dp_bold->zdim;
    ap->voxel_size[0] = dp_bold->dxdy;
    ap->voxel_size[1] = dp_bold->dxdy;
    ap->voxel_size[2] = dp_bold->dz;
    }
if(!check_dimensions(num_region_files,region_files->files,ap->vol)) exit(-1);
if(!(reg=malloc(sizeof*reg*num_region_files))) {
    printf("Error: Unable to malloc reg\n"); 
    exit(-1);
    }
for(i=0;i<num_region_files;i++) if(!(reg[i]=extract_regions(region_files->files[i],0,0,(float*)NULL,0,SunOS_Linux,(char**)NULL))) 
    exit(-1);
if(!num_regions) for(m=0;m<num_region_files;m++) num_regions += reg[m]->nregions;
if(!(rbf=find_regions_by_file_cover(num_region_files,num_regions,reg,roi))) exit(-1);
if(xform_file) {
    if(!(ms=get_mask_struct((char*)NULL,ap->vol,(int*)NULL,SunOS_Linux,(LinearModel*)NULL))) exit(-1);
    }
else {
    if(!(ms=get_mask_struct((char*)NULL,rbf->nvoxels,rbf->indices,SunOS_Linux,(LinearModel*)NULL))) exit(-1);
    }

if(!(rp=malloc(sizeof*rp*dp_bold->tdim_total))) {
    printf("Error: Unable to malloc rp\n");
    exit(-1);
    }
if(!(temp_float=malloc(sizeof*temp_float*ap->vol))) {
    printf("Error: Unable to malloc temp_float\n"); 
    exit(-1);
    }
if(!(temp_double_bold=malloc(sizeof*temp_double_bold*ap->vol))) {
    printf("Error: Unable to malloc temp_double_bold\n");
    exit(-1);
    }
if(!(temp_double_target=malloc(sizeof*temp_double_target*ap->vol))) {
    printf("Error: Unable to malloc temp_double_target\n");
    exit(-1);
    }
if(!(stat=malloc(sizeof*stat*ap->vol))) {
    printf("Error: Unable to malloc stat\n");
    exit(-1);
    }

for(l=i=0;i<num_bold_files;i++) {
    if(!(mm_bold=map_disk(bold_files->files[i],dp_bold->vol*dp_bold->tdim[i],0))) exit(-1);
    if(!(mm_target=map_disk(target_files->files[i],dp_target->vol*dp_target->tdim[i],0))) exit(-1);
    printf("Processing %s\n",bold_files->files[i]);
    printf("Processing %s\n",target_files->files[i]);
    swapbytes_bold = shouldiswap(SunOS_Linux,dp_bold->bigendian[i]);
    swapbytes_target = shouldiswap(SunOS_Linux,dp_target->bigendian[i]);
    for(jj=j=0;j<dp_bold->tdim[i];j++,l++,jj+=dp_bold->vol) {
        if(!swapbytes_bold) {
            for(k=0;k<ms->lenbrain;k++) temp_double_bold[ms->brnidx[k]] = mm_bold->ptr[jj+ms->brnidx[k]]==(float)UNSAMPLED_VOXEL ?
                (double)UNSAMPLED_VOXEL : (double)mm_bold->ptr[jj+ms->brnidx[k]];
            }
        else {
            for(k=0;k<ms->lenbrain;k++) temp_float[k] = mm_bold->ptr[jj+ms->brnidx[k]];
            swap_bytes((unsigned char *)temp_float,sizeof(float),(size_t)ms->lenbrain);
            for(k=0;k<ms->lenbrain;k++) temp_double_bold[ms->brnidx[k]] = temp_float[k]==(float)UNSAMPLED_VOXEL ?
                (double)UNSAMPLED_VOXEL : (double)temp_float[k];
            }
        if(!swapbytes_target) {
            for(k=0;k<ms->lenbrain;k++) temp_double_target[ms->brnidx[k]] = mm_target->ptr[jj+ms->brnidx[k]]==(float)UNSAMPLED_VOXEL ?
                (double)UNSAMPLED_VOXEL : (double)mm_target->ptr[jj+ms->brnidx[k]];
            }
        else {
            for(k=0;k<ms->lenbrain;k++) temp_float[k] = mm_target->ptr[jj+ms->brnidx[k]];
            swap_bytes((unsigned char *)temp_float,sizeof(float),(size_t)ms->lenbrain);
            for(k=0;k<ms->lenbrain;k++) temp_double_target[ms->brnidx[k]] = temp_float[k]==(float)UNSAMPLED_VOXEL ?
                (double)UNSAMPLED_VOXEL : (double)temp_float[k];
            }
        if(xform_file) {
            if(!t4_atlas(temp_double_bold,stat,t4,dp_bold->xdim,dp_bold->ydim,dp_bold->zdim,dp_bold->dxdy,dp_bold->dz,A_or_B_or_U,
                dp_bold->orientation,ap)) exit(-1);
            for(k=0;k<ap->vol;k++) temp_double_bold[k] = stat[k];
            if(!t4_atlas(temp_double_target,stat,t4,dp_target->xdim,dp_target->ydim,dp_target->zdim,dp_target->dxdy,dp_target->dz,
                A_or_B_or_U,dp_target->orientation,ap)) exit(-1);
            for(k=0;k<ap->vol;k++) temp_double_target[k] = stat[k];
            }
        if(!(rp[l]=region_pearson(temp_double_bold,temp_double_target,rbf))) exit(-1);
        }
    if(!unmap_disk(mm_bold)) exit(-1);
    if(!unmap_disk(mm_target)) exit(-1);
    }
if(!(op = fopen_sub(output,"w"))) exit(-1);
for(i=0;i<region_files->nfiles;i++) fprintf(op,"%s\n",region_files->files[i]);
fprintf(op,"\n");
if(dp_bold->tdim_total==1) {
    for(l=i=0;i<num_bold_files;l+=dp_bold->tdim[i++]) {
        fprintf(op,"%s\n%s\n\n",bold_files->files[i],target_files->files[i]);
        fprintf(op,"region\tnvox\tnsampledvox\tr\n");
        for(k=0;k<num_regions;k++) {
            for(j=0;j<dp_bold->tdim[i];j++) {
                m = l+j;
                fprintf(op,"%s\t%d\t%d\t%7.2f\n",rbf->region_names_ptr[k],rbf->nvoxels_region[k],rp[m]->nsampledvoxels[k],
                    rp[m]->r[k]);
                }
            }
        fprintf(op,"\n");
        }
    }
else {
    for(l=i=0;i<num_bold_files;l+=dp_bold->tdim[i++]) {
        fprintf(op,"%s\n%s\n\n",bold_files->files[i],target_files->files[i]);
        for(k=0;k<num_regions;k++) {
            fprintf(op,"REGION: %s NVOXELS: %d\n",rbf->region_names_ptr[k],rbf->nvoxels_region[k]);
            fprintf(op,"frame r    nsampledvoxels\n");
            fprintf(op,"-------------------------------------------------------------\n");
            for(j=0;j<dp_bold->tdim[i];j++) {
                m = l+j;
                fprintf(op,"%3d %7.2f %7d\n",j+1,rp[m]->r[k],rp[m]->nsampledvoxels[k]);
                }
            }
        fprintf(op,"\n");
        }
    }
fflush(op);
fclose(op);
printf("Regional pearson correlations written %s\n",output);
}



Region_Pearson *region_pearson(double *temp_double1,double *temp_double2,Regions_By_File *rbf)
{
    int i,j,k,count;
    double sum_1,sum2_1,sum_2,sum2_2,sum_12,td1,td2,SSxx,SSxy,SSyy;
    Region_Pearson *rp;
    if(!(rp=malloc(sizeof*rp))) {
        printf("Error: Unable to malloc rp\n");
        return NULL;
        }
    if(!(rp->nsampledvoxels=malloc(sizeof*rp->nsampledvoxels*rbf->nreg))) {
        printf("Error: Unable to malloc rp->nsampledvoxels\n");
        return NULL;
        }
    if(!(rp->r=malloc(sizeof*rp->r*rbf->nreg))) {
        printf("Error: Unable to malloc rp->r\n");
        return NULL;
        }
    for(k=i=0;i<rbf->nreg;i++) {
        for(sum_1=sum2_1=sum_2=sum2_2=sum_12=0.,count=j=0;j<rbf->nvoxels_region[i];j++,k++) {
            td1=temp_double1[rbf->indices[k]];
            td2=temp_double2[rbf->indices[k]];
            if(td1!=(double)UNSAMPLED_VOXEL && td1!=0. && td2!=(double)UNSAMPLED_VOXEL && td2!=0. &&

              /*#ifndef LINUX*/
              #ifdef __sun__

                !IsNANorINF(td1) && !IsNANorINF(td2)
              #else
                (!isnan(td1) || !isinf(td1)) && (!isnan(td2) || !isinf(td2))
              #endif
                ) {
                sum_1 += td1;
                sum2_1 += td1*td1;
                sum_2 += td2;
                sum2_2 += td2*td2;
                sum_12 += td1*td2;
                count++;
                }
            }
        rp->nsampledvoxels[i] = count;
        if(count) {
            SSxx = sum2_1 - sum_1*sum_1/(double)count;
            SSxy = sum_12 - sum_1*sum_2/(double)count;
            SSyy = sum2_2 - sum_2*sum_2/(double)count;
            rp->r[i] = SSxy/sqrt(SSxx*SSyy);
            }
        else {
            rp->r[i] = (double)UNSAMPLED_VOXEL;
            }
        }
    return rp;
}

