/* Copyright 12/31/99 Washington University.  All Rights Reserved.
   utilities.c  $Revision: 1.318 $*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#ifdef __sun__
    #include <nan.h>
#endif
#include "fidl.h"
#include "subs_util.h"
#include "map_disk.h"

char *get_strings(char *line,char **strings,int how_many){
    int j;
    char temp[MAXNAME],*temp_ptr;

    for(j=0;j<how_many;j++) {
        if(isspace(*line)) line++;
        temp_ptr = temp;
        while(!isspace(*line) && *line != 0) *temp_ptr++ = *line++;
        *temp_ptr = 0;
        if(!(strings[j]=malloc(sizeof*strings[j]*(strlen(temp)+1)))) {
            printf("Error: Unable to malloc strings[%d]\n",j);
            return NULL;
            }
        strcpy(strings[j],temp);
        }
    return line;
    }

int *strings_to_int(char *line,int *ptr,int how_many){
    int i;
    char *strptr;
    for(i=0;i<how_many;i++) {
        *ptr++=(int)strtol(line,&strptr,10);
        line = ++strptr;
        }
    return ptr;
    }

int **read_contrast_string(int num_contrasts,int num_glm_files,int argc_c,char **argv) {
    char string[MAXNAME];
    int i,j,m,**contrast;

    if(!(contrast=malloc(sizeof*contrast*num_contrasts))) {
        printf("Error: Unable to malloc contrast\n");
        return NULL;
        }
    for(i=0;i<num_contrasts;i++,argc_c++) {
        if(!(contrast[i]=malloc(sizeof*contrast[i]*num_glm_files))) {
            printf("Error: Unable to malloc contrast[%d]\n",i);
            return NULL;
            } 
        m = count_strings(argv[argc_c],string,0);
        if(m == 1)
            for(j=0;j<num_glm_files;j++) contrast[i][j] = atoi(argv[argc_c]);
        else if(m == num_glm_files)
            strings_to_int(string,contrast[i],m);
        else {
            fprintf(stderr,"ERROR i=%d :  %d time courses listed, num_glm_files = %d\n",i,m,num_glm_files);
            exit(-1);
            }
        }
    return contrast;
    }

Length_And_Max *get_length_and_max(int nstrings,char **strings){
    Length_And_Max *lam;
    int i;

    if(!(lam=malloc(sizeof*lam))) {
        printf("Error: Unable to malloc lam\n");
        return (Length_And_Max*)NULL;
        }
    if(!(lam->length=malloc(sizeof*lam->length*nstrings))) {
        printf("Error: Unable to malloc lam->length\n");
        return (Length_And_Max*)NULL;
        }
    for(lam->max_length=i=0;i<nstrings;i++) {
        lam->length[i] = strlen(strings[i]);
        if(lam->length[i] > lam->max_length) lam->max_length = lam->length[i];
        }
    return lam;
    }
void free_length_and_max(Length_And_Max *lam) {
    free(lam->length);
    free(lam);
    }
int assign_region_names(Interfile_header *ifh,float *image,int lenvol,int nregions,double *coor,double *peakcoor,float *peakval){
    char region_name[MAXNAME];
    int i,j,k,*nvoxels_region; /* *temp_int */
    size_t *tstv;

    if(!(nvoxels_region=malloc(sizeof*nvoxels_region*nregions))) {
        printf("Error: Unable to malloc nvoxels_region in assign_region_names\n");
        return 0;
        }
    for(i=0;i<nregions;i++) nvoxels_region[i]=0;

    #if 0
    if(!(temp_int=malloc(sizeof*temp_int*nregions))) {
        printf("Error: Unable to malloc temp_int in assign_region_names\n");
        return 0;
        }
    #endif
    /*START120824*/
    if(!(tstv=malloc(sizeof*tstv*nregions))) {
        printf("Error: Unable to malloc tstv in assign_region_names\n");
        return 0;
        }

    for(i=0;i<lenvol;i++) for(j=0;j<nregions;j++) if((int)image[i] == j+2) nvoxels_region[j]++;
    if(!coor) {
        if(peakcoor) {
            for(k=i=0;i<nregions;i++,k+=3) {
                sprintf(region_name,"%d %+d_%+d_%+d %d",i,(int)rint(peakcoor[k]),(int)rint(peakcoor[k+1]),(int)rint(peakcoor[k+2]),
                    nvoxels_region[i]);
                tstv[i] = strlen(region_name) + 1;
                }
            }
        else {
            for(i=0;i<nregions;i++) {
                sprintf(region_name,"%d voxel_value%d %d",i,i+2,nvoxels_region[i]);
                tstv[i] = strlen(region_name) + 1;
                }
            }

        }
    else {
        if(!peakcoor) {
            for(j=i=0;i<nregions;i++,j+=3) {
                sprintf(region_name,"%d %+d_%+d_%+d %d",i,(int)rint(coor[j]),(int)rint(coor[j+1]),(int)rint(coor[j+2]),
                    nvoxels_region[i]);
                tstv[i] = strlen(region_name) + 1;
                }
            }
        else {
            for(k=j=i=0;i<nregions;i++,j+=3,k+=3) {
                sprintf(region_name,"%d com%+d_%+d_%+d_p%+d_%+d_%+d=%.2f %d",i,(int)rint(coor[j]),(int)rint(coor[j+1]),
                    (int)rint(coor[j+2]),(int)rint(peakcoor[k]),(int)rint(peakcoor[k+1]),(int)rint(peakcoor[k+2]),peakval[i],
                    nvoxels_region[i]);
                tstv[i] = strlen(region_name) + 1;
                }
            }
        }
    if(!(ifh->region_names=d2charvar(nregions,tstv))) return 0;
    ifh->nregions = nregions;
    if(!coor) {
        if(peakcoor) {
            for(k=i=0;i<nregions;i++,k+=3) sprintf(ifh->region_names[i],"%d %+d_%+d_%+d %d",i,
                (int)rint(peakcoor[k]),(int)rint(peakcoor[k+1]),(int)rint(peakcoor[k+2]),nvoxels_region[i]);
            }
        else {
            for(i=0;i<nregions;i++) sprintf(ifh->region_names[i],"%d voxel_value%d %d",i,i+2,nvoxels_region[i]);
            }
        }
    else {
        if(!peakcoor) {

            #if 0
            /*Doesn't work on linux.*/
            for(j=i=0;i<nregions;i++) sprintf(ifh->region_names[i],"%d %+d_%+d_%+d %d",i,(int)rint(coor[j++]),(int)rint(coor[j++]),
                (int)rint(coor[j++]),nvoxels_region[i]);
            #endif
            for(j=i=0;i<nregions;i++,j+=3) sprintf(ifh->region_names[i],"%d %+d_%+d_%+d %d",i,(int)rint(coor[j]),
                (int)rint(coor[j+1]),(int)rint(coor[j+2]),nvoxels_region[i]);

            }
        else {

            #if 0
            /*Doesn't work on linux.*/
            for(k=j=i=0;i<nregions;i++) sprintf(ifh->region_names[i],"%d com%+d_%+d_%+d_p%+d_%+d_%+d=%.2f %d",i,
                (int)rint(coor[j++]),(int)rint(coor[j++]),(int)rint(coor[j++]),(int)rint(peakcoor[k++]),(int)rint(peakcoor[k++]),
                (int)rint(peakcoor[k++]),peakval[i],nvoxels_region[i]);
            #endif
            for(k=j=i=0;i<nregions;i++,j+=3,k+=3) sprintf(ifh->region_names[i],"%d com%+d_%+d_%+d_p%+d_%+d_%+d=%.2f %d",i,
                (int)rint(coor[j]),(int)rint(coor[j+1]),(int)rint(coor[j+2]),(int)rint(peakcoor[k]),(int)rint(peakcoor[k+1]),
                (int)rint(peakcoor[k+2]),peakval[i],nvoxels_region[i]);

            }
        }

    /*free(temp_int);*/
    /*START120824*/
    free(tstv);

    free(nvoxels_region);
    return 1;
    }

//START180316
#if 0
int get_indbase(TC *tcs,Regions_By_File *fbf,LinearModel **glmstack,Meancol **meancol_stack,int *indbase_col,int subi){
    int i,j,k,l,kk,glmi,tci;
    for(kk=k=0;k<tcs->num_tc;k++) {
        for(i=0;i<tcs->num_tc_to_sum[k][subi];i++,kk++) {
            indbase_col[kk] = -1;
            glmi = fbf->file_index[kk];
            tci = fbf->roi_index[kk];
            for(j=0;j<glmstack[glmi]->ifh->glm_Nrow;j++) {
                if(fabs(glmstack[glmi]->AT[tci][j]) > (double)UNSAMPLED_VOXEL) {
                    for(l=0;l<meancol_stack[glmi]->ncol;l++) {
                        if(glmstack[glmi]->AT[meancol_stack[glmi]->col[l]][j] > (float)UNSAMPLED_VOXEL) {
                            if(indbase_col[kk]==-1) {
                                indbase_col[kk] = meancol_stack[glmi]->col[l];
                                }
                            else {
                                printf("Error: get_indbase More than one baseline found!\n");
                                return 0;
                                }
                            }
                        }
                    if(indbase_col[kk]==-1) {
                        printf("Error: get_indbase Baseline not found!\n");
                        return 0;
                        }
                    break;
                    }
                }
            }
        }
    return 1;
    }
#endif


/**********************************************************************************************************/
void voxel_index_to_fidl_and_atlas(int voxel_index,int xdim,int ydim,int zdim,double *center,double *mmppix,
    int *atlas_coordinates,int *fidl_coordinates)
/**********************************************************************************************************/
{
    /* slice+1             col                 row    [fidl coordinates = (col,row)]
       z_fidl              x_fidl              y_fidl
       fidl_coordinates[2] fidl_coordinates[0] fidl_coordinates[1] */

    int area,slice,col_row,row,col;

    area = xdim*ydim;
    slice = voxel_index/area;
    col_row = voxel_index-slice*area;
    row = col_row/xdim;
    col = col_row - row*xdim;

    /*fidl_coordinates[0] = col;
    fidl_coordinates[1] = row;
    fidl_coordinates[2] = slice+1;
    atlas_coordinates[0] = (int)(center[0] - mmppix[0]*(col+1));
    atlas_coordinates[1] = (int)(mmppix[1]*(row+1) - center[1]);
    atlas_coordinates[2] = (int)(mmppix[2]*(zdim-slice) - center[2]);*/

    get_fidl_atlas_coord(col,row,slice,zdim,center,mmppix,atlas_coordinates,fidl_coordinates);
}

/**************************************************************************************************************/
void get_fidl_atlas_coord(int col,int row,int slice,int zdim,double *center,double *mmppix,int *fidl_coordinates,
    int *atlas_coordinates)
/**************************************************************************************************************/
{
    /* slice+1             col                 row    [fidl coordinates = (col,row)]
       z_fidl              x_fidl              y_fidl
       fidl_coordinates[2] fidl_coordinates[0] fidl_coordinates[1] */

    fidl_coordinates[0] = col;
    fidl_coordinates[1] = row;
    fidl_coordinates[2] = slice+1;
    atlas_coordinates[0] = rint(center[0] - mmppix[0]*(col+1));
    atlas_coordinates[1] = rint(mmppix[1]*(row+1) - center[1]);
    atlas_coordinates[2] = rint(mmppix[2]*(zdim-slice) - center[2]);
}

int print_version_number(char *rcsid,FILE *stream)
{
    int i,status=1;
    char dum[MAXNAME],program[MAXNAME],date[11],version[10];
    sscanf(rcsid,"%s %s %s %s %s %s %s %s",dum,program,version,date,dum,dum,dum,dum);
    *strrchr(program,'.') = 0;
    if((i=fprintf(stream,"%s Version %s, %s\n",strrchr(program,'/')+1,version,date))<0) {
        printf("Error: fprintf returned %d  Should return a value >= 0\n",i);
        status = 0;
        }
    return status;
}










int get_tags_tc_guts(int i,TC *tcs,LinearModel *glm,TC *tc_weights,char *string,char *string3)
{
    char string2[MAXNAME],**strings;
    int j,k,m,n=0,p=0,*integers;
    string[0] = string3[0] = 0;
    if(!(strings=malloc(sizeof*strings*tcs->num_tc_to_sum[i][0]))) {
        printf("Error: Unable to malloc strings in get_tags_tc\n");
        return 0;
        }
    if(!(integers=malloc(sizeof*integers*tcs->num_tc_to_sum[i][0]))) {
        printf("Error: Unable to malloc integers in get_tags_tc\n");
        return 0;
        }
    for(j=0;j<tcs->num_tc_to_sum[i][0];j++) {
        for(m=k=0;m<(int)tcs->tc[i][0][j];m+=glm->ifh->glm_effect_length[k++]);k--;
        grab_string(glm->ifh->glm_effect_label[k],string2);
        if(!(strings[j]=malloc(sizeof*strings[j]*(strlen(string2)+1)))) {
            printf("Error: Unable to malloc strings[%d] in get_tags_tc\n",j);
            return 0;
            }
        strcpy(strings[j],string2);
        integers[j] = (int)tcs->tc[i][0][j] - m + glm->ifh->glm_effect_length[k];
        if(!j) {
            n = p = 1;
            }
        else {
            for(m=0;m<j;m++) {
                n = strcmp(string2,strings[m]) ? 1 : 0;
                if(!n) break;
                }
            for(m=0;m<j;m++) {
                p = integers[j] != integers[m] ? 1 : 0;
                if(!p) break;
                }
            }
        if(n) {
            if(j) strcat(string,"+");
            strcat(string,string2);
            }
        if(p) {
            sprintf(string2,"%d+",integers[j]);
            strcat(string3,string2);
            }
        if(tc_weights) sprintf(string,"%s_%.3f",string,tc_weights->tc[i][0][j]);
        }
    for(j=0;j<tcs->num_tc_to_sum[i][0];j++) free(strings[j]);
    free(strings);
    free(integers);
    if(!tc_weights) *strrchr(string3,'+') = 0;
    return 1;
}
Tags_Tc_Struct *get_tags_tc(TC *tcs,LinearModel *glm,TC *tc_weights)
{
    char string[MAXNAME],string3[MAXNAME];
    int i,j,k,l,m,total_tags_tc_effects_len;
    size_t *tags_tc_len,*tags_tc_effects_len;
    Tags_Tc_Struct *tags_tcs;

    if(!(tags_tcs=malloc(sizeof*tags_tcs))) {
        printf("Error: Unable to malloc tags_tcs in get_tags_tc\n");
        return NULL;
        }
    if(!(tags_tc_len=malloc(sizeof*tags_tc_len*tcs->num_tc))) {
        printf("Error: Unable to malloc tags_tc_len in get_tags_tc\n");
        return NULL;
        }
    if(!(tags_tc_effects_len=malloc(sizeof*tags_tc_effects_len*tcs->num_tc))) {
        printf("Error: Unable to malloc tags_tc_effects_len in get_tags_tc\n");
        return NULL;
        }
    for(total_tags_tc_effects_len=i=0;i<tcs->num_tc;i++) {
        if(!get_tags_tc_guts(i,tcs,glm,tc_weights,string,string3)) return NULL;
        tags_tc_len[i] = strlen(string)+1;
        total_tags_tc_effects_len += tags_tc_effects_len[i] = strlen(string3)+1;
        }
    if(!(tags_tcs->tags_tc=d2charvar(tcs->num_tc,tags_tc_len))) return NULL;
    if(total_tags_tc_effects_len==tcs->num_tc) {
        tags_tcs->tags_tc_effects=NULL;
        }
    else {
        if(!(tags_tcs->tags_tc_effects=d2charvar(tcs->num_tc,tags_tc_effects_len))) return NULL;
        }
    for(i=0;i<tcs->num_tc;i++) {
        if(!get_tags_tc_guts(i,tcs,glm,tc_weights,string,string3)) return NULL;
        strcpy(tags_tcs->tags_tc[i],string);
        if(tags_tcs->tags_tc_effects) strcpy(tags_tcs->tags_tc_effects[i],string3);
        }
    for(j=i=0;i<tcs->num_tc;i++) j += tcs->num_tc_to_sum[i][0];
    if(!(tags_tcs->indices=malloc(sizeof*tags_tcs->indices*j))) {
        printf("Error: Unable to malloc tags_tcs->index_first\n");
        return NULL;
        }
    for(l=i=0;i<tcs->num_tc;i++) {
        for(j=0;j<tcs->num_tc_to_sum[i][0];j++,l++) {
            for(m=k=0;m<(int)tcs->tc[i][0][j];m+=glm->ifh->glm_effect_length[k++]);k--;
            tags_tcs->indices[l] = k;
            }
        }
    free(tags_tc_len);
    free(tags_tc_effects_len);
    return tags_tcs;
}

//START170724
#if 0                                                                                        /*rbf->indices*/
void crsw(double *temp_double,double *region_stack,Regions_By_File *rbf,double *weights,int *weightsi)
{
    int i,j,k;
    double weights_sum;
    for(k=i=0;i<rbf->nreg;i++) {
        for(weights_sum=region_stack[i]=0.,j=0;j<rbf->nvoxels_region[i];j++,k++) {

            /*printf("%e ",temp_double[rbf->indices[k]]);
            if(fabs(temp_double[rbf->indices[k]])>(double)UNSAMPLED_VOXEL) printf("yes "); else printf("no ");*/

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
            }
        /*printf("\ni=%d weights_sum=%f region_stack=%f\n",i,weights_sum,region_stack[i]);*/
        weights_sum ? (region_stack[i]/=weights_sum) : (region_stack[i]=(double)UNSAMPLED_VOXEL);
        /*printf("after i=%d weights_sum=%f region_stack=%f\n",i,weights_sum,region_stack[i]);*/
        }
}
#endif



#if 0
Region_Stats *rs(double *temp_double,Regions_By_File *rbf)
{
    int i,j,k,count;
    double sum,sum2,min,max,td;
    Region_Stats *rs;
    if(!(rs=malloc(sizeof*rs))) {
        printf("Error: Unable to malloc rs\n");
        return (Region_Stats*)NULL;
        }
    if(!(rs->mean=malloc(sizeof*rs->mean*rbf->nreg))) { 
        printf("Error: Unable to malloc rs->mean\n");
        return (Region_Stats*)NULL;
        }
    if(!(rs->sd=malloc(sizeof*rs->sd*rbf->nreg))) { 
        printf("Error: Unable to malloc rs->sd\n");
        return (Region_Stats*)NULL;
        }
    if(!(rs->min=malloc(sizeof*rs->min*rbf->nreg))) { 
        printf("Error: Unable to malloc rs->sd\n");
        return (Region_Stats*)NULL;
        }
    if(!(rs->max=malloc(sizeof*rs->max*rbf->nreg))) { 
        printf("Error: Unable to malloc rs->sd\n");
        return (Region_Stats*)NULL;
        }
    if(!(rs->nsampledvoxels=malloc(sizeof*rs->nsampledvoxels*rbf->nreg))) { 
        printf("Error: Unable to malloc rs->nsampledvoxels\n");
        return (Region_Stats*)NULL;
        }
    if(!(rs->sum=malloc(sizeof*rs->sum*rbf->nreg))) { 
        printf("Error: Unable to malloc rs->sum\n");
        return (Region_Stats*)NULL;
        }
    if(!(rs->sum2=malloc(sizeof*rs->sum2*rbf->nreg))) { 
        printf("Error: Unable to malloc rs->sum2\n");
        return (Region_Stats*)NULL;
        }
    for(k=i=0;i<rbf->nreg;i++) {
        for(min=1.e20,max=-1.e20,sum=sum2=0.,count=j=0;j<rbf->nvoxels_region[i];j++,k++) {
            td=temp_double[rbf->indices[k]];
            if(td!=(double)UNSAMPLED_VOXEL && td!=0. &&

              /*#ifndef LINUX*/
              #ifdef __sun__

                !IsNANorINF(td)
              #else
                (!isnan(td) || !isinf(td))
              #endif
                ) {
                sum += td;
                sum2 += td*td;
                if(td > max) max = td;
                if(td < min) min = td;
                count++;
                }
            }
        rs->nsampledvoxels[i] = count;
        if(count) {
            rs->mean[i] = sum/(double)count;
            /*printf("i=%d count=%d sum=%f rs->mean=%f\n",i,count,sum,rs->mean[i]);*/
            rs->sd[i] = sqrt((sum2 - (sum*sum)/(double)count)/(double)(count-1));
            rs->min[i] = min;
            rs->max[i] = max;
            rs->sum[i] = sum;
            rs->sum2[i] = sum2;
            }
        else {
            rs->mean[i] = (double)UNSAMPLED_VOXEL;
            rs->sd[i] = (double)UNSAMPLED_VOXEL;
            rs->min[i] = (double)UNSAMPLED_VOXEL;
            rs->max[i] = (double)UNSAMPLED_VOXEL;
            rs->sum[i] = (double)UNSAMPLED_VOXEL;
            rs->sum2[i] = (double)UNSAMPLED_VOXEL;
            }
        }
    return rs;
}
void free_rs(Region_Stats *rs)
{
    free(rs->sum2);
    free(rs->sum);
    free(rs->nsampledvoxels);
    free(rs->max);
    free(rs->min);
    free(rs->sd);
    free(rs->mean);
    free(rs);
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
int write_conc(char *root,int nfiles,char **identify,int rootonly)
{
    char concname[MAXNAME],filename[MAXNAME];
    int i;
    FILE *fp;
    if(nfiles==1) {
        sprintf(filename,"%s.4dfp.img",root);
        printf("File written to %s\n",filename);
        }
    else {
        sprintf(concname,"%s.conc",root);
        if(!(fp=fopen_sub(concname,"w"))) return 0;

        /*START130926*/
        /*fprintf(fp,"number_of_files: %d\n",nfiles);*/

        for(i=0;i<nfiles;i++) {
            if(!rootonly) sprintf(filename,"%s_b%d.4dfp.img",root,i+1); else sprintf(filename,"%s.4dfp.img",root);
            fprintf(fp,"%s %s\n",filename,!identify?"":identify[i]);
            }
        fclose(fp);
        printf("Concatenated file written to %s\n",concname);
        }
    return 1;
}
#endif


#if 0
void set_ifh_and_constants(int how_many,Interfile_header *ifh,int *space,char *space_str)
{
    Atlas_Param *ap;
    if(*space=get_atlas(how_many)) {
        ap = get_atlas_param(*space,(Interfile_header*)NULL);
        ifh->dim1 = ap->xdim;
        ifh->dim2 = ap->ydim;
        ifh->dim3 = ap->zdim;
        ifh->mmppix[0] = ap->mmppix[0];
        ifh->mmppix[1] = ap->mmppix[1];
        ifh->mmppix[2] = ap->mmppix[2];
        ifh->center[0] = ap->center[0];
        ifh->center[1] = ap->center[1];
        ifh->center[2] = ap->center[2];
        strcpy(space_str,ap->str);
        free_atlas_param(ap);
        }
}
#endif
/*START140401*/
/*START150408*/
#if 0
void set_ifh_and_constants(int how_many,Interfile_header *ifh,char *space,char *space_str)
{
    Atlas_Param *ap;
    if(space=get_atlas(how_many)) {
        ap = get_atlas_param(space,(Interfile_header*)NULL);
        ifh->dim1 = ap->xdim;
        ifh->dim2 = ap->ydim;
        ifh->dim3 = ap->zdim;
        ifh->mmppix[0] = ap->mmppix[0];
        ifh->mmppix[1] = ap->mmppix[1];
        ifh->mmppix[2] = ap->mmppix[2];
        ifh->center[0] = ap->center[0];
        ifh->center[1] = ap->center[1];
        ifh->center[2] = ap->center[2];
        strcpy(space_str,ap->str);
        free_atlas_param(ap);
        }
}
#endif


/*void ellipsestack(int N,int *x,int *y,int *z,float *stack,float *value,int xdim,int ydim,int zdim,float *xdia,float *ydia,
    float *zdia,float xvoxelsize,float yvoxelsize,float zvoxelsize)*/
/*START130426*/
int ellipsestack(int N,int *x,int *y,int *z,float *stack,float *value,int xdim,int ydim,int zdim,float *xdia,float *ydia,
    float *zdia,float xvoxelsize,float yvoxelsize,float zvoxelsize)
{
    int i,j,k,l,xstart,xend,ystart,yend,zstart,zend,index,nvox=0; /*xradi,yradi,zradi*/
    float xrad,yrad,zrad,xrad2,yrad2,zrad2;
    for(l=0;l<N;l++) {

        #if 0
        xrad = xdia[l]/2./xvoxelsize;
        xradi = (int)xrad;
        xrad2 = xrad*xrad;
        yrad = ydia[l]/2./yvoxelsize;
        yradi = (int)yrad;
        yrad2 = yrad*yrad;
        zrad = zdia[l]/2./zvoxelsize;
        zradi = (int)zrad;
        zrad2 = zrad*zrad;
        zstart = z[l]-zradi < 0 ? 0 : z[l]-zradi;
        zend = z[l]+zradi > zdim-1 ? zdim-1 : z[l]+zradi;
        ystart = y[l]-yradi < 0 ? 0 : y[l]-yradi;
        yend = y[l]+yradi > ydim-1 ? ydim-1 : y[l]+yradi;
        xstart = x[l]-xradi < 0 ? 0 : x[l]-xradi;
        xend = x[l]+xradi > xdim-1 ? xdim-1 : x[l]+xradi;
        #endif
        /*START130426*/
        xrad = xdia[l]/2./xvoxelsize;
        xrad2 = xrad*xrad;
        yrad = ydia[l]/2./yvoxelsize;
        yrad2 = yrad*yrad;
        zrad = zdia[l]/2./zvoxelsize;
        zrad2 = zrad*zrad;
        zstart = (int)rint(z[l]-zrad < 0. ? 0. : z[l]-zrad);
        zend = (int)rint(z[l]+zrad > (double)(zdim-1) ? (double)(zdim-1) : z[l]+zrad);
        ystart = (int)rint(y[l]-yrad < 0. ? 0. : y[l]-yrad);
        yend = (int)rint(y[l]+yrad > (double)(ydim-1) ? (double)(ydim-1) : y[l]+yrad);
        xstart = (int)rint(x[l]-xrad < 0. ? 0. : x[l]-xrad);
        xend = (int)rint(x[l]+xrad > (double)(xdim-1) ? (double)(xdim-1) : x[l]+xrad);


        for(k=zstart;k<=zend;k++) {
            for(j=ystart;j<=yend;j++) {
                for(i=xstart;i<=xend;i++) {

                    /*if(((z[l]-k)*(z[l]-k)/zrad2+(y[l]-j)*(y[l]-j)/yrad2+(x[l]-i)*(x[l]-i)/xrad2) <= 1) {*/
                    /*START130426*/
                    if(((float)((z[l]-k)*(z[l]-k))/zrad2+(float)((y[l]-j)*(y[l]-j))/yrad2+(float)((x[l]-i)*(x[l]-i))/xrad2) <= 1.) {

                        index = (k-1)*xdim*ydim + j*xdim + i;

                        /*if(!stack[index]) stack[index] = value[l];*/
                        /*START130426*/
                        if(!stack[index]) {
                            stack[index] = value[l];
                            nvox++;
                            }

                        }
                    }
                }
            }
        }

    /*START130426*/
    return nvox;
}

//START180319
#if 0
int check_dimensions(int nfiles,char **files_ptr,int vol) {
    int i;
    Interfile_header *ifh=NULL;
    for(i=0;i<nfiles;i++){
        if(strcmp(files_ptr[i],"NA")){
            if(!(ifh=read_ifh(files_ptr[i],ifh)))return 0;
            if(ifh->dim1*ifh->dim2*ifh->dim3!=vol){
                printf("fidlError: Dimensions of images are not equal. Problem file: %s\n",files_ptr[i]);
                printf("fidlError: Should be %d instead of %d\n",vol,ifh->dim1*ifh->dim2*ifh->dim3);
                return 0;
                }
            free_ifh(ifh,1);
            }
        }
    free(ifh);
    return 1;
    }
#endif

int assign_glm_list_file_to_ifh(Interfile_header *ifh,char *glm_list_file){
    if(ifh->file_name) free(ifh->file_name);
    if(!(ifh->file_name=malloc(sizeof*ifh->file_name*(strlen(glm_list_file)+1)))) {
        printf("Error: Unable to malloc ifh->file_name in assign_glm_list_file_to_ifh\n");
        return 0;
        } 
    strcpy(ifh->file_name,glm_list_file);
    return 1;
    }
int assign_glm_list_file_to_ifh_FS(Interfile_header *ifh,Files_Struct *glm_list_file){
    size_t i,total;
    char *strptr;
    if(ifh->file_name) free(ifh->file_name);
    for(total=i=0;i<glm_list_file->nfiles;i++) total += glm_list_file->strlen_files[i]+1;
    if(!(ifh->file_name=malloc(sizeof*ifh->file_name*total))) {
        printf("Error: Unable to malloc ifh->file_name in assign_glm_list_file_to_ifh_FS\n");
        return 0;
        }
    for(strptr=ifh->file_name,i=0;i<glm_list_file->nfiles;i++) {
        strcpy(strptr,glm_list_file->files[i]);
        strptr += glm_list_file->strlen_files[i]+1;
        } 
    return 1;
    }


char **d2char(int dim1,int dim2)
{
    int i;
    char **array;

    if(!(array=(char **)malloc((size_t)(dim1*sizeof(char*))))) {
        printf("allocation failure 1 in d2char\n");
        return (char **)NULL;
        }
    if(!(array[0]=(char *)malloc((size_t)(dim1*dim2*sizeof(char))))) {
        printf("allocation failure 2 in d2char\n");
        return (char **)NULL;
        }
    for(i=1;i<dim1;i++) array[i]=array[i-1]+dim2;
    return array;
}
int **d2int(int dim1,int dim2)
{
    int i,**array;
 
    if(!(array=(int **)malloc((size_t)(dim1*sizeof(int*))))) {
        printf("allocation failure 1 in d2int\n");
        return (int **)NULL;
        }
    if(!(array[0]=(int *)calloc((size_t)(dim1*dim2),sizeof(int)))) {
        printf("allocation failure 2 in d2int\n");
        return (int **)NULL;
        }
    for(i=1;i<dim1;i++) array[i]=array[i-1]+dim2;
    return array;
}

/*START130410*/
size_t ***d3size_t(size_t dim1,size_t dim2,size_t dim3)
{
    size_t i,j,***array;

    if(!(array=(size_t ***)malloc((size_t)(dim1*sizeof(size_t**))))) {
        printf("allocation failure 1 in d3size_t\n");
        return NULL;
        }
    if(!(array[0]=(size_t **)malloc((size_t)(dim1*dim2*sizeof(size_t*))))) {
        printf("allocation failure 2 in d3size_t\n");
        return NULL;
        }
    if(!(array[0][0]=(size_t *)calloc((size_t)(dim1*dim2*dim3),sizeof(size_t)))) {
        printf("allocation failure 3 in d3size_t\n");
        return NULL;
        }
    for(i=0;i<dim1;i++) {
        array[i] = array[0] + dim2*i;
        array[i][0] = array[0][0] + dim2*dim3*i;
        for(j=0;j<dim2;j++) array[i][j] = array[i][0] + dim3*j;
        }
    return array;
}

int ***d3int(int dim1,int dim2,int dim3)
{
    int i,j,***array;

    if(!(array=(int ***)malloc((size_t)(dim1*sizeof(int**))))) {
        printf("allocation failure 1 in d3int\n");
        return (int ***)NULL;
        }
    if(!(array[0]=(int **)malloc((size_t)(dim1*dim2*sizeof(int*))))) {
        printf("allocation failure 2 in d3int\n");
        return (int ***)NULL;
        }
    if(!(array[0][0]=(int *)calloc((size_t)(dim1*dim2*dim3),sizeof(int)))) {
        printf("allocation failure 3 in d3int\n");
        return (int ***)NULL;
        }
    for(i=0;i<dim1;i++) {
        array[i] = array[0] + dim2*i;
        array[i][0] = array[0][0] + dim2*dim3*i;
        for(j=0;j<dim2;j++) array[i][j] = array[i][0] + dim3*j;
        }
    return array;
}
float ***d3float(int dim1,int dim2,int dim3)
{
    int i,j;
    float ***array;

    if(!(array=(float ***)malloc((size_t)(dim1*sizeof(float**))))) {
        printf("allocation failure 1 in d3float\n");
        return (float ***)NULL;
        }
    if(!(array[0]=(float **)malloc((size_t)(dim1*dim2*sizeof(float*))))) {
        printf("allocation failure 2 in d3float\n");
        return (float ***)NULL;
        }
    if(!(array[0][0]=(float *)calloc((size_t)(dim1*dim2*dim3),sizeof(float)))) {
        printf("allocation failure 3 in d3float\n");
        return (float ***)NULL;
        }
    for(i=0;i<dim1;i++) {
        array[i] = array[0] + dim2*i;
        array[i][0] = array[0][0] + dim2*dim3*i;
        for(j=0;j<dim2;j++) array[i][j] = array[i][0] + dim3*j;
        }
    return array;
}
double ***d3double(int dim1,int dim2,int dim3)
{
    int i,j;
    double ***array;

    if(!(array=(double ***)malloc((size_t)(dim1*sizeof(double**))))) {
        printf("allocation failure 1 in d3double\n");
        return (double ***)NULL;
        }
    if(!(array[0]=(double **)malloc((size_t)(dim1*dim2*sizeof(double*))))) {
        printf("allocation failure 2 in d3double\n");
        return (double ***)NULL;
        }
    if(!(array[0][0]=(double *)calloc((size_t)(dim1*dim2*dim3),sizeof(double)))) {
        printf("allocation failure 3 in d3double\n");
        return (double ***)NULL;
        }
    for(i=0;i<dim1;i++) {
        array[i] = array[0] + dim2*i;
        array[i][0] = array[0][0] + dim2*dim3*i;
        for(j=0;j<dim2;j++) array[i][j] = array[i][0] + dim3*j;
        }
    return array;
}

void free_d2int(int **array)
{
    free(array[0]);
    free(array);
}
#if 0
void free_d2double(double **array)
{
    free(array[0]);
    free(array);
}
#endif
void free_d3int(int ***array)
{
    free(array[0][0]);
    free(array[0]);
    free(array);
}
void free_d3float(float ***array)
{
    free(array[0][0]);
    free(array[0]);
    free(array);
}
void free_d3double(double ***array)
{
    free(array[0][0]);
    free(array[0]);
    free(array);
}

/*START150324*/
#if 0
double **d2double_mult(double **A,double **B,int NrowA,int NcolA,int NrowB,int NcolB,int Transpose,double **C)
{
    int     i,j,k;

    switch(Transpose) {
        case TRANSPOSE_NONE:
            if(!C) {
                if(!(C = d2double(NrowA,NcolB))) break;
                }
            for(i=0;i<NrowA;i++) {
                for(j=0;j<NcolB;j++) {
                    C[i][j] = 0.;
                    for(k=0;k<NcolA;k++) C[i][j] += A[i][k]*B[k][j];
                    }
                }
            break;
        case TRANSPOSE_FIRST:
            if(!C) {
                if(!(C = d2double(NcolA,NcolB))) break;
                }
            for(i=0;i<NcolA;i++) {
                for(j=0;j<NcolB;j++) {
                    C[i][j] = 0.;
                    for(k=0;k<NrowA;k++) C[i][j] += A[k][i]*B[k][j];
                    }
                }
            break;
        case TRANSPOSE_SECOND:
            if(!C) {
                if(!(C = d2double(NrowA,NrowB))) break;
                }
            for(i=0;i<NrowA;i++) {
                for(j=0;j<NrowB;j++) {
                    C[i][j] = 0.;
                    for(k=0;k<NcolB;k++) C[i][j] += A[i][k]*B[j][k];
                    }
                }
            break;
        default:
            printf("Invalid transpose parameter in d2double_mult\n");
        }
    return C;
}
#endif

#if 0
double **d2double_kronprod(double **A,double **B,int NrowA,int NcolA,int NrowB,int NcolB,double **C)
{
    int i,j,k,l,m,n,startm=0,startn=0;

    if(!C) C = d2double(NrowA*NrowB,NcolA*NcolB);
    if(C) {    
        for(i=0;i<NrowA;i++,startm+=NrowB)
            for(j=0;j<NcolA;j++,startn+=NcolB)
                for(m=startm,k=0;k<NrowB;k++,m++)
                    for(n=startn,l=0;l<NcolB;l++,n++)
                        C[m][n] = A[i][j]*B[k][l]; 
        }
    return C;
}
#endif

int count_strings_new(char *line,char *write_back,char wild_card,char *last_char) {
    int num_strings = 0,length,count_length=0;
    length = strlen(line);
    while(*line) {
        if(isspace(*line) || *line == '"' || *line == '\'' || *line == ',' || *line == wild_card) {
            *last_char = *line++;
            count_length++;
            }
        else {
            while (!(isspace(*line)) && *line != '"' && *line != '\'' && *line != ','&& *line!=0 && *line!=wild_card) {
                *write_back++ = *line++;
                count_length++;
                *last_char = *line;
                }
            *write_back++ = ' ';
            ++num_strings;
            }
        if(count_length >= length) break; /*This was added for argv.*/
        }
    *(--write_back) = 0;
    return num_strings;
    }
int count_strings_new_nocomma(char *line,char *write_back,char wild_card,char *last_char) {
    int num_strings = 0,length,count_length=0;
    length = strlen(line);
    while(*line) {
        if(isspace(*line) || *line == '"' || *line == '\'' || *line == wild_card) {
            *last_char = *line++;
            count_length++;
            }
        else {
            while(!(isspace(*line)) && *line != '"' && *line != '\'' && *line!=0 && *line!=wild_card) {
                *write_back++ = *line++;
                count_length++;
                *last_char = *line;
                }
            *write_back++ = ' ';
            ++num_strings;
            }
        if(count_length >= length) break; /*This was added for argv.*/
        }
    *(--write_back) = 0;
    return num_strings;
    }
int count_strings_new2(char *line,char *write_back,char wildcard1,char wildcard2,char *last_char,char *first_char,
    int no_first_char_to_write_back)
{
    int num_strings = 0,length,count_length=0;

    /**first_char = (char)NULL;*/
    /*START141229*/
    /* *first_char = (char*)NULL;*/
    /* *first_char = NULL;*/
    /* *first_char = (charptr_t)NULL;*/
    /* *first_char = (uintptr_t)NULL;*/
    /* *first_char = (wchar_t)NULL;*/
    /* *first_char = (sig_atomic_t)NULL;*/
    /**first_char = '\0';*/
    *first_char = 0;


    length = strlen(line);
    while(*line) {
        if(isspace(*line) || *line == '"' || *line == '\'' || *line == ',' || *line==wildcard1 || *line==wildcard2) {
            *last_char = *line++;
            count_length++;
            }
        else {
            while(!(isspace(*line)) && *line!='"' && *line!='\'' && *line!=',' && *line!=0 && *line!=wildcard1 && *line!=wildcard2) {
                if(!(*first_char)) {
                    *first_char = *line;
                    if(no_first_char_to_write_back) *line++;
                    }
                *write_back++ = *line++;
                count_length++;
                *last_char = *line;
                }
            *write_back++ = ' ';
            ++num_strings;
            }
        if(count_length >= length) break; /*This was added for argv.*/
        }
    *(--write_back) = 0;
    return num_strings;
}
char *make_timestr(char *timestr) /*min 16 elements, eg timestr[16]*/
{
    time_t timep;
    timep = time(NULL);
    strftime(timestr,16,"%y%m%d_%T",localtime(&timep));
    return timestr;
}


#if 0
char *make_scratchdir(int noPWD)
{
    int pathl=0;
    char *scratchdir,cmd[MAXNAME],string[MAXNAME],*path=NULL,timestr[16];
    if(!noPWD) {
        if(!(path=getenv("PWD"))) {
            printf("fidlError: Unable to get environment variable PWD\n");
            return NULL;
            }
        pathl = strlen(path);
        }
    if(!(scratchdir=malloc(sizeof*scratchdir*(26+pathl)))) {
        printf("fidlError: Unable to malloc scratchdir\n");
        return NULL;
        }
    scratchdir[0] = 0; 
    if(!noPWD) sprintf(scratchdir,"%s/",path);
    sprintf(scratchdir,"%sSCRATCH_%s/",scratchdir,make_timestr(timestr));
    sprintf(cmd,"mkdir %s",scratchdir);
    if(!system(cmd) == -1) {
        printf("fidlError: Unable to %s\n",cmd);
        return NULL;
        }
    return scratchdir;
}
#endif
/*START150423*/
#if 0
/*START140715*/
char *make_scratchdir(int noPWD,char *label)
{
    int pathl=0;
    char *scratchdir,cmd[MAXNAME],path[MAXNAME],*pwd,timestr[16];
    if(!noPWD) {
        if(!(pwd=getenv("PWD"))) {
            printf("fidlError: Unable to get environment variable PWD\n");
            return NULL;
            }
        sprintf(path,"%s/",pwd);
        pathl = strlen(path);
        }
    if(!(scratchdir=malloc(sizeof*scratchdir*(strlen(label)+19+pathl)))) {
        printf("fidlError: Unable to malloc scratchdir\n");
        return NULL;
        }
    sprintf(scratchdir,"%s%s_%s/",noPWD?"":path,label,make_timestr(timestr));
    sprintf(cmd,"mkdir %s",scratchdir);
    if(!system(cmd) == -1) {
        printf("fidlError: Unable to %s\n",cmd);
        return NULL;
        }
    return scratchdir;
}
#endif








char *get_tail_sans_ext(char *write_back) {
    char *strptr;

    #if 0
    if((strptr=strstr(write_back,".4dfp.img"))) {
        *strptr = 0;
        }
    else if((strptr=strstr(write_back,".glm"))) {
        *strptr = 0;
        }
    else if((strptr=strrchr(write_back,'.'))) {
        *strptr = 0;
        }
    #endif
    //START170714
    if((strptr=strstr(write_back,".4dfp.img")))*strptr=0;
    else if((strptr=strstr(write_back,".nii.gz")))*strptr=0;
    else if((strptr=strrchr(write_back,'.')))*strptr=0;


    if((strptr=strrchr(write_back,'/'))) {
        strptr++;
        }
    else {
        strptr = write_back;
        }
    if(!strptr) printf("Error: strptr is a null pointer. Abort!\n");
    return strptr;
    }
char *get_dir(char *write_back) {
    char *strptr;
    if((strptr=strrchr(write_back,'/'))) *(++strptr)=0;
    strptr = write_back;
    return strptr;
    }

/*START141229*/
/*static char *last_strstr(const char *haystack, const char *needle)*/
char *last_strstr(const char *haystack, const char *needle)
{
    char *result=NULL,*p;
    if(*needle=='\0') return (char *)haystack;
    for(;;) {
        if(!(p=strstr(haystack, needle))) break;
        result = p;
        haystack = p+1;
        }
    return result;
}


















/*START150805*/
#if 0
int do_fseek(char *filename,FILE *stream,long offset,int whence) {
    int status;
    if((status=fseek(stream,offset,whence))) {
        printf("fidlError: occured while seeking to %ld in %s.\n",offset,filename);
        printf("fidlError: perhaps this file is missing some bytes and should be recomputed.\n");
        }
    return status;
    }
#endif

/*START110728*/
#if 0
int fwrite_sub(void *ptr,size_t size,size_t nitems,FILE *fp,int swapbytes)
{
    unsigned char *buf;
    int len;

    if(swapbytes) {
        len = size*nitems;
        if(!(buf=malloc(sizeof*buf*len))) {
            printf("fwrite_sub size=%d nitems=%d len=%d\n",size,nitems,len);
            printf("Error: fwrite_sub Unable to malloc buf\n");
            return 0;
            }
        if(!memcpy(buf,ptr,len)) return 0;
        swap_bytes(buf,size,nitems);
        }
    else {
        buf = (unsigned char *)ptr;
        }
    if((len=fwrite(buf,size,nitems,fp)) != nitems) {
        printf("Error: fwrite_sub failed. Wrote %d  expecting to write %d Abort!\n",len,nitems);
        printf("Error: Are you out of memory?\n");
        return 0;
        }
    if(swapbytes) free(buf);
    return 1;
}
#endif








#if 0
int get_sizes(char *type,size_t *s)
{
    int status=1;
    if(!strcmp("char",type)) {
        s[0] = sizeof(char*);
        s[1] = sizeof(char);
        }
    else if(!strcmp("int",type)) {
        s[0] = sizeof(int*);
        s[1] = sizeof(int);
        }
    else if(!strcmp("float",type)) {
        s[0] = sizeof(float*);
        s[1] = sizeof(float);
        }
    else if(!strcmp("double",type)) {
        s[0] = sizeof(double*);
        s[1] = sizeof(double);
        }
    else {
        printf("Error:%s is an unknown type. Abort!\n");
        status = 0;
        }
    return status;
}
double **d2(char *type,int dim1,int dim2)
{
    double **array=NULL;
    size_t s[2];
    int i;

    if(get_sizes(type,s)) {
        printf("dim1=%d dim2=%d s[0]=%d s[1]=%d\n",dim1,dim2,s[0],s[1]);
        if(!(array=malloc((size_t)(dim1*s[0])))) {
            printf("allocation failure 1 in d2\n");
            }
        else {
            if(!(array[0]=calloc((size_t)(dim1*dim2),s[1]))) {
                printf("allocation failure 2 in d2\n");
                }
            else {
                for(i=1;i<dim1;i++) array[i]=array[i-1]+dim2;
                }
            }
        }
    return array;
}
unsigned char **d2var(char *type,int dim1,int *dim2)
{
    unsigned char **array=NULL;
    size_t s[2];
    int i,length;

    if(get_sizes(type,s)) {
        if(!(array=malloc((size_t)(dim1*s[0])))) {
            printf("allocation failure 1 in d2var\n");
            }
        else {
            for(length=i=0;i<dim1;i++) length += dim2[i];
            if(!(array[0]=calloc((size_t)length,s[1]))) {
                printf("allocation failure 2 in d2var\n");
                }
            else {
                for(i=1;i<dim1;i++) array[i]=array[i-1]+dim2[i-1];
                }
            }
        }
    return array;
}
#endif

Dat *readdat(char *datfile)
{
    char *strptr,line[MAXNAME],write_back[MAXNAME],*ptr;
    int i,j,k,l,len,nlines,nstrings,dummy;
    Dat *dat;
    FILE *fp;

    if(!(fp = fopen_sub(datfile,"r"))) return NULL;
    if(!(dat=malloc(sizeof*dat))) {
        printf("Error: Unable to malloc dat\n");
        return NULL;
        }
    for(len=0,dat->nlines=-1;fgets(line,sizeof(line),fp);) {
        if((nstrings=count_strings(line,write_back,' ')) > 0) {
            if(++dat->nlines==0) {
                dat->nf = nstrings;
                }
            else if(dat->nlines>0) {
                grab_string_new(write_back,line,&j);
                len += j+1;
                }
            }
        }
    dat->tlen = len;
    if(!(dat->name=malloc(sizeof*dat->name*len))) {
        printf("Error: Unable to malloc meansd->regname\n");
        return NULL;
        }
    if(!(dat->len=malloc(sizeof*dat->len*dat->nlines))) {
        printf("Error: Unable to malloc dat->len\n");
        return NULL;
        }
    if(!(dat->data=malloc(sizeof*dat->data*dat->nf*dat->nlines))) {
        printf("Error: Unable to malloc dat->data\n");
        return NULL;
        }
    for(rewind(fp),l=dat->nf-1,ptr=dat->name,k=i=0,nlines=-1;fgets(line,sizeof(line),fp);) {
        if((nstrings=count_strings(line,write_back,' ')) > 0) {
            if(++nlines>0) {
                strptr=grab_string_new(write_back,line,&dat->len[i]);
                dat->len[i]++;
                memcpy(ptr,line,dat->len[i]);
                ptr += dat->len[i++];
                for(j=0;j<l;j++,k++) {
                    strptr=grab_string_new(strptr,line,&dummy);
                    dat->data[k] = atof(line);
                    }
                }
            }
        }
    fclose(fp);
    return dat;
}


#if 0
MOVED TO utilities2.c 171113
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
#endif










#if 0
KEEP
Sng *read_ev(char *event_file)
{
    char line[MAXNAME],write_back[MAXNAME],dummy,*strptr,*strptr2,*strptr3;
    int i,j,first,nstrings,len,total_len,nbehavcol,maxcol;
    double *dptr,td;
    FILE *fp;
    Sng *sng;

    if(!(fp=fopen_sub(event_file,"r"))) return NULL;
    if(!(sng=malloc(sizeof*sng))) {
        printf("Error: Unable to malloc sng\n");
        return NULL;
        }
    printf("Reading %s\n",event_file);
    for(first=sng->nbehavcol=sng->num_trials=0,j=1;fgets(line,sizeof(line),fp);j++) {
        if((nstrings=count_strings_new(line,write_back,' ',&dummy))) {
            if(!first) {
                strptr=grab_string_new(write_back,line,&len);
                sng->TR=atof(line);
                sng->num_levels=nstrings-1; 
                for(strptr2=strptr,total_len=i=0;i<sng->num_levels;i++) {
                    strptr2=grab_string_new(strptr2,line,&len);
                    total_len+=len+1;
                    }
                if(!(sng->factor_labels=malloc(sizeof*sng->factor_labels*total_len))) {
                    printf("Error: Unable to malloc sng->factor_labels\n");
                    return NULL;
                    }
                if(!(sng->lfactor_labels=malloc(sizeof*sng->lfactor_labels*sng->num_levels))) {
                    printf("Error: Unable to malloc sng->lfactor_labels\n");
                    return NULL;
                    }
                for(strptr3=sng->factor_labels,strptr2=strptr,i=0;i<sng->num_levels;i++) {
                    strptr2=grab_string_new(strptr2,line,&len);
                    strcpy(strptr3,line);
                    sng->lfactor_labels[i]=len+1;
                    }
                first=1;
                }
            else if(nstrings<2) {
                printf("Error: Line %d has %d field. Must have at least 2 fields.\n",j,nstrings);
                return NULL;
                }
            else {
                sng->num_trials++;
                if((nbehavcol=(nstrings-3))>sng->nbehavcol) sng->nbehavcol=nbehavcol;
                }
            }
        }
    if(!(sng->times=malloc(sizeof*sng->times*sng->num_trials))) {
        printf("Error: Unable to malloc sng->times\n");
        return NULL;
        }
    if(!(sng->conditions=malloc(sizeof*sng->conditions*sng->num_trials))) {
        printf("Error: Unable to malloc sng->conditions\n");
        return NULL;
        }
    if(!(sng->stimlen_vs_t=malloc(sizeof*sng->stimlen_vs_t*sng->num_trials))) {
        printf("Error: Unable to malloc sng->stimlen_vs_t\n");
        return NULL;
        }
    for(i=0;i<sng->num_trials;i++) sng->stimlen_vs_t[i]=0.;
    if(!(sng->behav_vs_t=malloc(sizeof*sng->behav_vs_t*sng->nbehavcol*sng->num_trials))) {
        printf("Error: Unable to malloc sng->behav_vs_t\n");
        return NULL;
        }
    for(i=0;i<sng->nbehavcol*sng->num_trials;i++) sng->behav_vs_t[i]=(double)UNSAMPLED_VOXEL;
    for(rewind(fp),sng->max_stimlen_vs_t=0.,maxcol=sng->nbehavcol+3,dptr=sng->behav_vs_t,first=i=0;fgets(line,sizeof(line),fp);) {
        if((nstrings=count_strings_new(line,write_back,' ',&dummy))) {
            if(!first) {
                first=1;
                }
            else {
                strptr=grab_string_new(write_back,line,&len);
                sng->times[i]=atof(line); 
                strptr=grab_string_new(strptr,line,&len);
                sng->conditions[i]=atoi(line);
                if(nstrings>=3) {
                    strptr=grab_string_new(strptr,line,&len);
                    if((sng->stimlen_vs_t[i]=atof(line))>sng->max_stimlen_vs_t) sng->max_stimlen_vs_t=sng->stimlen_vs_t[i];
                    for(j=3;j<nstrings;j++,dptr++) {
                        strptr=grab_string_new(strptr,line,&len);
                        if(!strcmp(strptr,"NA")||!strcmp(strptr,"nA")||!strcmp(strptr,"Na")||!strcmp(strptr,"na")) *dptr=atof(strptr);
                        }
                    }
                dptr+=maxcol-nstrings;
                i++;
                }
            }
        } 
    if(!(sng->frames=malloc(sizeof*sng->frames*sng->num_trials))) {
        printf("Error: Unable to malloc sng->frames\n");
        return NULL;
        }
    if(!(sng->frames_halfTR=malloc(sizeof*sng->frames_halfTR*sng->num_trials))) {
        printf("Error: Unable to malloc sng->frames_halfTR\n");
        return NULL;
        }
    if(!(sng->frames_quarterTR=malloc(sizeof*sng->frames_quarterTR*sng->num_trials))) {
        printf("Error: Unable to malloc sng->frames_quarterTR\n");
        return NULL;
        }
    if(!(sng->frames_floor=malloc(sizeof*sng->frames_floor*sng->num_trials))) {
        printf("Error: Unable to malloc sng->frames_floor\n");
        return NULL;
        }
    if(!(sng->offsets_halfTR=malloc(sizeof*sng->offsets_halfTR*sng->num_trials))) {
        printf("Error: Unable to malloc sng->offsets_halfTR\n");
        return NULL;
        }
    if(!(sng->offsets_quarterTR=malloc(sizeof*sng->offsets_quarterTR*sng->num_trials))) {
        printf("Error: Unable to malloc sng->offsets_quarterTR\n");
        return NULL;
        }
    for(i=0;i<sng->num_trials;i++) {
        /*td=sng->times[i]/sng->TR;*/
        sng->frames[i]=sng->frames_halfTR[i]=sng->frames_quarterTR[i]=(int)rint((td=sng->times[i]/sng->TR));
        sng->frames_floor[i]=(int)floor(td);
        if((td-=sng->frames_floor[i])<.25) {
            /*do nothing*/
            }
        else if(td<.75) {
            sng->offsets_halfTR[i]=1;
            sng->frames_halfTR[i]+=1;
            }
        else {
            sng->frames_halfTR[i]+=1;
            }
        if(td<.125) {
            /*do nothing*/
            }
        else if(td<.375) {
            sng->offsets_quarterTR[i]=3;
            sng->frames_quarterTR[i]+=1;
            }
        else if(td<.625) {
            sng->offsets_quarterTR[i]=2;
            sng->frames_quarterTR[i]+=1;
            }
        else if(td<.875) {
            sng->offsets_quarterTR[i]=1;
            sng->frames_quarterTR[i]+=1;
            }
        else {
            sng->frames_quarterTR[i]+=1;
            }
        }
    if(!(sng->index_present=malloc(sizeof*sng->index_present*sng->num_levels))) {
        printf("Error: Unable to malloc sng->index_present\n");
        return NULL;
        }
    for(sng->count_present=0,i=0;i<sng->num_levels;i++) {
        for(j=0;j<sng->num_trials;j++) {
            if(sng->conditions[j]==i) {
                sng->index_present[sng->count_present++]=i;
                break;
                }
            }
        }
    if(!(sng->index_present=realloc(sng->index_present,sizeof*sng->index_present*sng->count_present))) {
        printf("Error: Unable to realloc sng->index_present\n");
        return NULL;
        }
    return sng;
}
#endif


#if 0
int delete_scratch_files(char **files,int nfiles)
{
    int i,status=1;
    printf("Deleting scratch files.\n");
    for(i=0;i<nfiles;i++) {
        if(remove(files[i])) {
            printf("Error: Unable to remove %s\n",files[i]);
            status=0;
            }
        *(strrchr(files[i],'.')) = 0;
        strcat(files[i],".ifh");
        if(remove(files[i])) {
            printf("Error: Unable to remove %s\n",files[i]);
            status=0;
            }
        }
    return status;
}
#endif
//START161027
int delete_scratch_files(char **files,int nfiles){
    int i,status=1;
    printf("Deleting scratch files.\n");
    for(i=0;i<nfiles;i++){
        if(strcmp(files[i],"NA")){
            if(remove(files[i])) {
                printf("Error: Unable to remove %s\n",files[i]);
                status=0;
                }
            *(strrchr(files[i],'.')) = 0;
            strcat(files[i],".ifh");
            if(remove(files[i])) {
                printf("Error: Unable to remove %s\n",files[i]);
                status=0;
                }
            }
        }
    return status;
    }






char *getstr(char *line,char *str) /*SEGEMENTATION FAULT AS A SHAREABLE OBJECT*/
{
    char *ptr,*ptr1;
    if((ptr=strstr(line,str))) {
        ptr += strlen(str);
        while(*ptr == ' ') ++ptr;
        if((ptr1=strstr(line,"\n"))) *ptr1 = 0;
        }
    return ptr;
}
int count_strings(char *line,char *write_back,char wild_card)
{
    int num_strings = 0,length,count_length=0;

    length = strlen(line);
    /*printf("length = %d\n",length);*/

    while(*line) {
        if(isspace(*line) || *line == '"' || *line == '\'' || *line == ',' || *line == wild_card) {
            line++;
            count_length++;
            }
        else {
            /*while (!(isspace(*line)) && *line != '"' && *line != '\'' && *line != ',' && *line != '+') {*/
            /*while (!(isspace(*line)) && *line != '"' && *line != '\'' && *line != ',' && *line != wild_card) {*/
            while (!(isspace(*line)) && *line != '"' && *line != '\'' && *line != ','&& *line!=0 && *line!=wild_card) {
                /*if(!(isspace(*line))) printf("yes1 %d\n",isspace(*line));
                if(*line != '"') printf("yes2\n");
                if(*line != '\'')printf("yes3\n");
                if(*line != ',') printf("yes4\n");
                if(*line != wild_card)printf("yes5\n");
                printf("line[char]=%c\n",*line);*/
                *write_back++ = *line++;
                count_length++;
                /*printf("here0 count_length=%d\n",count_length);*/
                }
            *write_back++ = ' ';
            ++num_strings;
            }
        if(count_length >= length) break; /*This was added for argv.*/
        }
    *(--write_back) = 0;
    /*printf("here bottom count_strings\n");*/
    return num_strings;
}
char *get_line(char *line,char *returnstr,char *searchstr,int *returnstrlen)
{
    char *strptr;
    if((line=strstr(line,searchstr))) {
        if((strptr=strstr(line,"\n"))) *strptr = 0;
        line += strlen(searchstr);
        while(isspace(*line)) line++;
        for(*returnstrlen=0;*line != 0;(*returnstrlen)++) *returnstr++ = *line++;
        *returnstr = 0;
        }
    return line;
}

/*START150610*/
#if 0
char *get_line_wnull(char *line,char *returnstr,char *searchstr,int *returnstrlen)
{
    char *strptr;
    if((line=strstr(line,searchstr))) {
        for(strptr=line;isspace(*(--strptr)););*(++strptr)=0;
        if((strptr=strstr(line,"\n"))) *strptr = 0;
        line += strlen(searchstr);
        while(isspace(*line)) line++;
        for(*returnstrlen=0;*line != 0;(*returnstrlen)++) *returnstr++ = *line++;
        *returnstr = 0;
        }
    return line;
}
#endif

char *get_line_wwosearchstr(char *line,char *returnstr,char *searchstr,int *returnstrlen)
{
    char *strptr;
    if((strptr=strstr(line,searchstr))){line=strptr;line+=strlen(searchstr);}
    if((strptr=strstr(line,"\n"))) *strptr = 0;
    while(isspace(*line)) line++;
    for(*returnstrlen=0;*line != 0;(*returnstrlen)++) *returnstr++ = *line++;
    *returnstr = 0;
    return line;
}
char *parse_line(char *line,char *returnstr,char *searchstr,int *returnstrlen)
{
    char *strptr=NULL,*frontofline;
    frontofline=line;
    if((line=strstr(line,searchstr))) {
        if((strptr=strstr(line,"\n"))) *strptr = 0;
        strptr = line += strlen(searchstr);
        while(isspace(*line)) line++;
        for(*returnstrlen=0;*line != 0;(*returnstrlen)++) *returnstr++ = *line++;
        *returnstr = 0;
        }
    *strptr=0;
    return frontofline;
}
char *grab_string(char *line,char *string)
{
    while(isspace(*line)) line++;
    while(!isspace(*line) && *line != 0) *string++ = *line++;
    *string = 0;
    return line;
}

#if 0
char *grab_string_search(char *line,char *returnstr,char *searchstr,int *returnstrlen)
{
    if((line=strstr(line,searchstr))) {
        line += strlen(searchstr);
        while(isspace(*line)) line++;
        for(*returnstrlen=0;!isspace(*line) && *line != 0;(*returnstrlen)++) *returnstr++ = *line++;
        *returnstr = 0;
        }
    return line;
}
#endif
