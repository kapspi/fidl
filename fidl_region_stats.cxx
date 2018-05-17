/* Copyright 2/22/18 Washington University.  All Rights Reserved.
   fidl_region_stats.c  $Revision: 1.1 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <math.h>
#include "fidl.h"
#include "center_of_mass2.h"
#include "stack.h"
#include "checkOS.h"
#include "find_regions_by_file_cover.h"
#include "files_struct.h"
#include "get_atlas.h"
#include "subs_util.h"
#include "write1.h"
int main(int argc,char **argv)
{
char *statistical_image=NULL,*region_center_of_mass_name="region_center_of_mass.txt",*date=NULL,atlas[7]="",*strptr,filename[MAXNAME];
int i,j,k,vol,SunOS_Linux,num_region_files=0,num_regions=0,*roi=NULL;
float *stat,*stat2;//*regstack
FILE *fp;
//Interfile_header *ifh;
Atlas_Param *ap;
Regions **reg;
Regions_By_File *rbf;
Files_Struct *region_files=NULL;
center_of_mass2struct *com;

stack zs;//rs
W1 *w1=NULL;

if (argc < 5) {
    fprintf(stderr,"Calculate the center of mass and peak for selected regions.\n\n");
    fprintf(stderr,"compute_region_stats -statistical_image all_cond_slice_ave_zstat_222_mcomp.4dfp.img \\\n");
    fprintf(stderr,"                     -region_file all_cond_good_regionsA.4dfp.img \\\n");
    fprintf(stderr,"                     -regions_of_interest 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17\n\n");
    fprintf(stderr,"        -statistical_image:          Z-statistic or magnitude image\n");
    fprintf(stderr,"        -region_center_of_mass_name: Name of output. Default is region_center_of_mass.txt\n");
    fprintf(stderr,"        -date:                       Printed at top of output.\n\n");
    fprintf(stderr,"        -region_file:                *.4dfp.img file(s) that specifies regions of interest.\n");
    fprintf(stderr,"        -regions_of_interest:        Calculate statistics for selected regions in the region file.\n");
    fprintf(stderr,"                                     First region is one.\n");
    exit(-1);
    }
for(i=1;i<argc;i++) {
    if(!strcmp(argv[i],"-statistical_image") && argc > i+1)
        statistical_image = argv[++i];
    if(!strcmp(argv[i],"-region_file") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_region_files;
        if(!(region_files=get_files(num_region_files,&argv[i+1]))) exit(-1);
        i += num_region_files;
        }
    if(!strcmp(argv[i],"-regions_of_interest") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_regions;
        if(!(roi=(int*)malloc(sizeof*roi*num_regions))) {
            printf("fidlError: Unable to malloc roi\n");
            exit(-1);
            }
        for(j=0;j<num_regions;j++) roi[j] = atoi(argv[++i]) - 1;
        }
    if(!strcmp(argv[i],"-region_center_of_mass_name") && argc > i+1)
        region_center_of_mass_name = argv[++i];
    if(!strcmp(argv[i],"-date") && argc > i+1)
        date = argv[++i];
    }
if(!statistical_image) {
    printf("Error: Need to specify -statistical_image\n");
    exit(-1);
    }
if(!num_region_files) {
    printf("Error: Need to specify -region_file\n");
    exit(-1);
    }
if((SunOS_Linux=checkOS())==-1) exit(-1);

if(!(stat=zs.stack0(statistical_image,SunOS_Linux)))exit(-1);
vol=zs.getvol();
get_atlas(vol,atlas);
if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL,statistical_image))) exit(-1);

//for(i=0;i<vol;i++)if(stat[i]>0.)printf("stat[%d]=%f\n",i,stat[i]);

//if(!(stat=zs.flip()))exit(-1);
//if(!(stat=zs.flip(stat)))exit(-1);
if(!(stat2=zs.flip(stat)))exit(-1);

if(!check_dimensions(num_region_files,region_files->files,vol)) exit(-1);
if(!(reg=(Regions**)malloc(sizeof*reg*num_region_files))) {
    printf("fidlError: Unable to malloc reg\n");
    exit(-1);
    }

for(i=0;i<num_region_files;i++) if(!(reg[i]=extract_regions(region_files->files[i],0,0,(float*)NULL,0,SunOS_Linux,(char**)NULL))) 
    exit(-1);

if(!num_regions) for(i=0;i<num_region_files;i++) num_regions += reg[i]->nregions;
if(!(rbf=find_regions_by_file_cover(num_region_files,num_regions,reg,roi))) exit(-1);


//for(i=0;i<vol;i++)if(stat[i]>0.)printf("stat[%d]=%f\n",i,stat[i]);

if(!(com=center_of_mass2init(rbf->nvoxels,num_regions))) exit(-1);
center_of_mass2(stat,com,rbf->nvoxels,rbf->indices,num_regions,rbf->nvoxels_region,ap);

if(!(fp=fopen_sub(region_center_of_mass_name,"w"))) exit(-1);
if(date) fprintf(fp,"%s\n",date);
fprintf(fp,"statistical image: %s\n",statistical_image);
fprintf(fp,"region file: %s\n",region_files->files[0]);
for(i=1;i<num_region_files;i++) fprintf(fp,"             %s\n",region_files->files[i]);
fprintf(fp,"\n");
for(j=k=i=0;i<num_regions;i++,j+=3,k+=3) {
    fprintf(fp,"REGION : %s %d\n",rbf->region_names_ptr[i],rbf->nvoxels_region[i]);
    fprintf(fp,"CENTER OF MASS\n");
    fprintf(fp,"    atlas coordinates: %.1f %.1f %.1f\n",com->coor[j],com->coor[j+1],com->coor[j+2]);
    if(statistical_image) {
        fprintf(fp,"PEAK : %f\n",com->peakval[i]);
        fprintf(fp,"    atlas coordinates: %.1f %.1f %.1f\n",com->peakcoor[k],com->peakcoor[k+1],com->peakcoor[k+2]);
        }
    fprintf(fp,"\n");
    }
fclose(fp);
printf("Output witten to %s\n",region_center_of_mass_name);fflush(stdout);

//START180223
for(i=0;i<vol;i++)stat[i]=0.;
for(i=0;i<num_regions;i++)stat[com->peaki[i]]=(float)(i+2);
if(!(w1=write1_init()))exit(-1);
w1->temp_float=stat;
w1->how_many=vol;
if(!(w1->ifh=read_ifh(region_files->files[0],(Interfile_header*)NULL)))exit(-1);
w1->ifh->number_format=(int)FLOAT_IF;

//for(i=0;i<num_regions;i++)sprintf(w1->ifh->region_names[i],"region names := %d %s 1",i,rbf->region_names_ptr[i]); 
for(i=0;i<num_regions;i++)printf("%s\n",rbf->region_names_ptr[i]); 

strptr=get_tail_sans_ext(region_files->files[0]);
sprintf(filename,"%s_peak.4dfp.img",strptr);
if(!write1(filename,w1))exit(-1);
printf("Peaks (1 voxel regions) written to %s\n",filename);fflush(stdout);
}

#if 0
        if(mask_file&&dp->filetypeall==(int)NIFTI){
            sprintf(string,"%d%d%d",c_orient[0],c_orient[1],c_orient[2]);
            if((cf_flip=(int)strtol(string,NULL,10))!=52)lcflip=1;
            printf("cf_flip=%d lcflip=%d\n",cf_flip,lcflip);
            if(lcflip){
                //if(c_orient[0]==4)glm->ifh->mmppix[0]*=-1.;
                //if(c_orient[1]==1)glm->ifh->mmppix[1]*=-1.;
                if(c_orient[0]==4&&c_orient[1]==1){
                    glm->ifh->mmppix[0]*=-1.;glm->ifh->mmppix[1]*=-1.;
                    glm->ifh->center[0]*=-1;glm->ifh->center[1]=-glm->ifh->center[0];
                    }
                }




region names                     := 0 Lant 96k
    if(!(ifh->region_names=d2charvar(ifh->nregions,tstv))) return NULL;
    for(i=0,rewind(fp);fgets(line,(int)MAXNAME,fp);) {
        if(strstr(line,"region names")) {
            get_line(line,junk,":=",&len);
            strcpy(ifh->region_names[i++],junk);
            }
        }
#endif
