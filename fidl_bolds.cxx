/* Copyright 6/19/17 Washington University.  All Rights Reserved.
   fidl_bolds.cxx  $Revision: 1.23 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <gsl/gsl_cdf.h>
#include "fidl.h"
#include "gauss_smoth2.h"
#include "read_frames_file.h"
#include "subs_mask.h"
#include "dim_param2.h"
#include "get_atlas_param.h"
#include "subs_util.h"
#include "t4_atlas.h"
#include "shouldiswap.h"
#include "read_tc_string_new.h"
#include "filetype.h"
#include "subs_cifti.h"
#include "subs_nifti.h"
#include "write1.h"
#include "minmax.h"
#include "nifti_class.h"
#include "checkOS.h"
#include "make_timestr2.h"
#include "find_regions_by_file_cover.h"
#include "map_disk.h"
#include "get_atlas.h"
#include "read_xform.h"
#include "twoA_or_twoB.h"

//START180319
#include "check_dimensions.h"

char const* Fileext[]={"",".4dfp.img",".dscalar.nii",".nii",".dtseries.nii"};

static char rcsid[] = "$Header: /home/hannah/mcavoy/idl/clib/RCS/fidl_bolds.c,v 1.23 2017/05/09 16:43:33 mcavoy Exp $";

int main(int argc,char **argv){
char string[MAXNAME],filename[MAXNAME],*mask_file=NULL,*frames_file=NULL,string2[MAXNAME],write_back[MAXNAME],*scratchdir0=NULL,
    *strptr=NULL,fwhmstr[10],*string_ptr,normstr[4],*avgacrossfiles=NULL,*label=NULL,atlas[7]="",timestr[23],
    filename0[MAXNAME],*cleanup=NULL,*sumacrossfiles=NULL; //scratchdir0=NULL
int i,j,k,l,m,n=0,p=0,q,r,ii,jj,num_regions=0,num_region_files=0,num_tc_files=0,*roi=NULL,lc_names_only=0,swapbytes=0,
    *A_or_B_or_U=NULL,chunks=1,dummy,sum_tdim=0,num_frames=0,argc_tc_weights=0,num_tc_weights=0,SunOS_Linux,
    argc_frames=0,chunkvol=0,nxform_file=0,nt4_identify=0,*t4_identify=NULL,*nframe,lccompress=0,*nsubframereg=NULL,nscratchdir=0,
    lcflip=0;
int64_t dims[3];
size_t i1,j1;
float *t4=NULL,*temp_float=NULL,fwhm=0.,*temp_float2=NULL;
double *temp_double=NULL,*stat=NULL,*mean=NULL,*subframereg=NULL,*sum_framereg,*sum2_framereg,TR=0.,starttime=UNSAMPLED_VOXEL,
    *td1=NULL,*dptr,td=0.;
FILE *fp,*fp1;
Interfile_header *ifh=NULL;
Regions **reg;
Regions_By_File *rbf=NULL,*fbf=NULL;
Dim_Param2 *dp;
Atlas_Param *ap;
Memory_Map *mm=NULL;
Mask_Struct *ms=NULL;
FS *fs;
TCnew *tc_weights=NULL;
Files_Struct *tc_files=NULL,*region_files=NULL,*xform_file=NULL,*scratchdir=NULL;
gauss_smoth2_struct *gs=NULL;
W1 *w1=NULL;
XmlS *xmls;
niftiStack nS;

if(argc < 5) {
    fprintf(stderr,"  -tc_files:            imgs or a conc.\n");
    fprintf(stderr,"  -region_file:         *.4dfp.img file(s) that specifies regions of interest.\n");
    fprintf(stderr,"                        Time courses are averaged over the region.\n");
    fprintf(stderr,"  -regions_of_interest: Compute timecourses for selected regions in the region file(s).\n");
    fprintf(stderr,"                        First region is one.\n");
    fprintf(stderr,"  -frames:              File containing frames of interest. First frame is 1.\n");
    fprintf(stderr,"  -frames_of_interest:  Frames of interest. First frame is 1.\n");
    fprintf(stderr,"  -names_only           Generate filenames for creation of the anova driving file.\n");
    fprintf(stderr,"                        No computation is performed. No files are created.\n\n");
    fprintf(stderr,"  -xform_file:          t4 file defining the transform to atlas space.\n");
    fprintf(stderr,"  -t4:                  For use with more than a single t4. Identifies which t4 goes with each file.\n");
    fprintf(stderr,"                        First t4 is 1. Need a number for each file.\n");
    fprintf(stderr,"  -atlas:               Either 111, 222, 333, 222MNI . Use with -xform_file option.\n");
    fprintf(stderr,"  -compress:            Only voxels in the mask are analyzed.\n");
    fprintf(stderr,"  -mask:                Only voxels in the mask are analyzed. Output dimensions are kept.\n");
    fprintf(stderr,"  -chunks               Breaks the compressed anova scratch files into chunks.\n");
    fprintf(stderr,"                        This is done for large designs that could exceed the 2 gig limit.\n");
    fprintf(stderr,"  -scratchdir:          Files are output to this location. Include the backslash at the end.\n");
    fprintf(stderr,"                        Multiple directories ok. Same files written to all.\n");
    fprintf(stderr,"  -gauss_smoth:         Amount of smoothing to be done in units of voxels at fwhm with a 3D gaussian filter.\n");
    fprintf(stderr,"  -normalize ALL        Normalize to the mean of all frames in the file.\n");
    fprintf(stderr,"  -tc_weights           Weighting to apply to summed timepoints (eg contrast).\n");
    fprintf(stderr,"  -swapbytes            Swap bytes of output stack.\n");
    fprintf(stderr,"  -avgacrossfiles:      Name of output file.\n");

    //START170616
    fprintf(stderr,"  -sumacrossfiles:      Output root.\n");

    fprintf(stderr,"  -label:               Condition label for output text file.\n");
    fprintf(stderr,"  -TR:                  TR for output text file.\n");
    fprintf(stderr,"  -starttime:           Time for first line of output text file. Time is incremented by TR.\n");
    fprintf(stderr,"  -cleanup:             Remove directory.\n");
    exit(-1);
    }
fwhmstr[0] = 0;
normstr[0] = 0;
for(i=1;i<argc;i++){
    if(!strcmp(argv[i],"-tc_files") && argc > i+1){
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++)++num_tc_files;
        if(!(tc_files=read_files(num_tc_files,&argv[i+1])))exit(-1);
        i+=num_tc_files;
        }
    if(!strcmp(argv[i],"-region_file") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_region_files;
        if(!(region_files=get_files(num_region_files,&argv[i+1]))) exit(-1);
        i += num_region_files;
        }
    if(!strcmp(argv[i],"-regions_of_interest") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_regions;

        //if(!(roi=malloc(sizeof*roi*num_regions))){
        //START170619
        if(!(roi=(int*)malloc(sizeof*roi*num_regions))){

            printf("fidlError: Unable to malloc roi\n");
            exit(-1);
            }
        for(j=0;j<num_regions;j++) roi[j] = atoi(argv[++i]) - 1;
        }
    if(!strcmp(argv[i],"-frames") && argc > i+1)
        frames_file = argv[++i];
    if(!strcmp(argv[i],"-frames_of_interest") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_frames;
        argc_frames = i+1; 
        i += num_frames;
        }
    if(!strcmp(argv[i],"-names_only"))
        lc_names_only = 1;
    if(!strcmp(argv[i],"-xform_file") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++nxform_file;
        if(!(xform_file=get_files(nxform_file,&argv[i+1]))) exit(-1);
        i += nxform_file;
        }
    if(!strcmp(argv[i],"-t4") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++nt4_identify;

        //if(!(t4_identify=malloc(sizeof*t4_identify*nt4_identify))) {
        //START170619
        if(!(t4_identify=(int*)malloc(sizeof*t4_identify*nt4_identify))){

            printf("fidlError: Unable to malloc t4_identify\n");
            exit(-1);
            }
        for(j=0;j<nt4_identify;j++) t4_identify[j] = atoi(argv[++i]) - 1;
        }
    if(!strcmp(argv[i],"-atlas") && argc > i+1)
        strcpy(atlas,argv[++i]);
    if(!strcmp(argv[i],"-compress") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1]) {
        mask_file = argv[++i];
        lccompress=1;
        }
    if(!strcmp(argv[i],"-mask") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        mask_file = argv[++i];
    if(!strcmp(argv[i],"-chunks"))
        chunks = 20;
    if(!strcmp(argv[i],"-scratchdir") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++nscratchdir;
        if(!(scratchdir=get_files(nscratchdir,&argv[i+1]))) exit(-1);
        i += nscratchdir;
        }
    if(!strcmp(argv[i],"-gauss_smoth") && argc > i+1) {
        fwhm = atof(argv[++i]);
        sprintf(fwhmstr,"_fwhm%.1f",fwhm);
        }
    if(!strcmp(argv[i],"-normalize") && argc > i+1) {
        if(argc > i+1 && !strcmp(argv[i+1],"ALL")) {
            strcpy(normstr,"_NA");
            ++i;
            }
        }
    if(!strcmp(argv[i],"-tc_weights") && argc > i+1) {
        argc_tc_weights = i+1;
        for(j=1;i+j<argc;j++) {
            string_ptr = argv[i+j] + 1;
            if(*string_ptr == '.') string_ptr++;
            if(strchr(argv[i+j],'-') == argv[i+j] && !isdigit(*string_ptr)) break;
            ++num_tc_weights;
            }
        i += num_tc_weights;
        }
    if(!strcmp(argv[i],"-swapbytes"))
        swapbytes = 1;
    if(!strcmp(argv[i],"-avgacrossfiles") && argc > i+1)
        avgacrossfiles = argv[++i];

    //START170616
    if(!strcmp(argv[i],"-sumacrossfiles") && argc > i+1)
        sumacrossfiles = argv[++i];

    if(!strcmp(argv[i],"-label") && argc > i+1)
        label = argv[++i];
    if(!strcmp(argv[i],"-TR") && argc > i+1)
        TR = atof(argv[++i]);
    if(!strcmp(argv[i],"-starttime") && argc > i+1)
        starttime = atof(argv[++i]);
    if(!strcmp(argv[i],"-cleanup") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        cleanup=argv[++i];
    }
if(!lc_names_only)print_version_number(rcsid,stdout);
if((SunOS_Linux=checkOS())==-1) exit(-1);
if(!lc_names_only) {
    printf("SunOS_Linux=%d\n",SunOS_Linux);
    }
if(!num_tc_files) {
    printf("fidlError: No timecourse files. Abort!\n");
    exit(-1);
    }
if(nt4_identify) {
    if(nt4_identify!=(int)tc_files->nfiles) {
        printf("fidlError: nt4_identify=%d num_tc_files=%zd Must be equal. Abort!\n",nt4_identify,tc_files->nfiles);
        exit(-1);
        }
    fprintf(stderr,"t4_identify="); for(i=0;i<nt4_identify;i++) fprintf(stderr,"%d ",t4_identify[i]); fprintf(stderr,"\n");
    }
else {
    if(nxform_file) if(xform_file->nfiles>1) {printf("fidlError: Need to specify -t4\n");fflush(stdout);exit(-1);} 

    //if(!(t4_identify=malloc(sizeof*t4_identify*tc_files->nfiles))) {
    //START170619
    if(!(t4_identify=(int*)malloc(sizeof*t4_identify*tc_files->nfiles))){

        printf("fidlError: Unable to malloc t4_identify\n");
        exit(-1);
        }
    for(i1=0;i1<tc_files->nfiles;i1++) t4_identify[i1]=0;
    }
if(!scratchdir&&lccompress){

    if(!(scratchdir0=make_scratchdir(1,"SCRATCH")))exit(-1);
    //170619
    //if(!(scratchdir0=(char const*)make_scratchdir(1,"SCRATCH")))exit(-1);

    //if(!(scratchdir=malloc(sizeof*scratchdir))) {
    //START170619
    if(!(scratchdir=(Files_Struct*)malloc(sizeof*scratchdir))){

        printf("fidlError: Unable to malloc scratchdir\n");
        exit(-1);
        }
    scratchdir->nfiles=1;
    scratchdir->files=&scratchdir0;
    }
//scratchdir_nfiles=!scratchdir?1:scratchdir->nfiles;
if(!nscratchdir)nscratchdir=1;
if(num_frames) {
    if(!(frames_file=(char*)malloc(sizeof*frames_file*MAXNAME))) {
        printf("fidlError: Unable to malloc frames_file\n");
        exit(-1);
        }
    sprintf(frames_file,"%sfidlframes%s.dat",scratchdir?scratchdir->files[0]:"",make_timestr2(timestr));
    if(!(fp = fopen_sub(frames_file,"w"))) exit(-1);
    for(j=0;j<num_frames;j++) fprintf(fp,"%s\n",argv[argc_frames+j]);
    fclose(fp);
    }
if(!(fs=read_frames_file(frames_file,0,0,0))) exit(-1);

/*printf("num_tc_weights=%d fs->ntc=%d\n",num_tc_weights,fs->ntc);
printf("fs->num_frames_to_sum="); for(i=0;i<fs->ntc;i++) printf("%d ",fs->num_frames_to_sum[i]); printf("\n");*/
/*printf("fs->nframes=%d\n",fs->nframes);
printf("fs->frames="); for(i=0;i<fs->nframes;i++) printf("%d ",fs->frames[i]); printf("\n");*/

if(num_tc_weights) {
    if(!(tc_weights=read_tc_string_TCnew(num_tc_weights,fs->num_frames_to_sum,argc_tc_weights,argv,'+'))) exit(-1);
    }
if(!(dp=dim_param2(tc_files->nfiles,tc_files->files,SunOS_Linux)))exit(-1);
if(!xform_file&&!(dp->filetypeall==(int)CIFTI||dp->filetypeall==(int)CIFTI_DTSERIES))get_atlas(dp->volall,atlas); //unknown atlas ok


//if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL))) exit(-1);
//START170616
if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL,tc_files->files[0])))exit(-1);

if(!xform_file)ap->vol = dp->volall;
if(!atlas[0]){
    ap->xdim=dp->xdim[0];
    ap->ydim=dp->ydim[0];
    ap->zdim=dp->zdim[0];
    ap->voxel_size[0]=dp->dxdy[0];
    ap->voxel_size[1]=dp->dxdy[0];
    ap->voxel_size[2]=dp->dz[0];
    }
if(!lc_names_only) {
    if(!(ms=get_mask_struct(mask_file,dp->volall,(int*)NULL,SunOS_Linux,(LinearModel*)NULL,dp->volall))) exit(-1);
    chunkvol=(int)ceil((double)ms->lenbrain/(double)chunks);
    if(xform_file) {
        if(!(t4=(float*)malloc(sizeof*t4*(size_t)T4SIZE*xform_file->nfiles))) {
            printf("fidlError: Unable to malloc t4\n");
            exit(-1);
            }
        if(!(A_or_B_or_U=(int*)malloc(sizeof*A_or_B_or_U*xform_file->nfiles))) {
            printf("fidlError: Unable to malloc A_or_B_or_U\n");
            exit(-1);
            }
        for(i1=0;i1<xform_file->nfiles;i1++) {
            if(!read_xform(xform_file->files[i1],&t4[i1*(int)T4SIZE])) exit(-1);
            if((A_or_B_or_U[i1]=twoA_or_twoB(xform_file->files[i1])) == 2) exit(-1);
            }
        }
    if(num_region_files) {
        if(!check_dimensions(num_region_files,region_files->files,ap->vol)) exit(-1);
        if(!(reg=(Regions**)malloc(sizeof*reg*num_region_files))) {
            printf("fidlError: Unable to malloc reg\n");
            exit(-1);
            }

        printf("here0\n");fflush(stdout);

        for(i=0;i<num_region_files;i++) if(!(reg[i]=get_reg(region_files->files[i],0,0,(float*)NULL,0,SunOS_Linux,
            (char**)NULL))) exit(-1);

        printf("here1\n");fflush(stdout);

        if(!num_regions) for(m=0;m<num_region_files;m++) num_regions += reg[m]->nregions;
        if(!(rbf=find_regions_by_file_cover(num_region_files,num_regions,reg,roi))) exit(-1);

        //START180302
        //lcflip=1;
        //START180319 what you really want to check is c_orient or the string derived from it
        if(dp->filetypeall!=get_filetype(region_files->files[0]))lcflip=1;
        printf("lcflip= %d\n",lcflip);

        }
    else {
        num_regions=ms->lenbrain;
        if(ms->vol!=dp->volall){printf("fidlError: fidl_bolds ms->vol=%d dp->volall=%d Must be equal.\n",ms->vol,dp->volall);
            fflush(stdout);exit(-1);}
        }
    if(avgacrossfiles||sumacrossfiles){
        if(!(subframereg=(double*)malloc(sizeof*subframereg*dp->tdim_max*num_regions*(num_region_files?tc_files->nfiles:1)))){
            printf("fidlError: Unable to malloc subframereg\n");
            exit(-1);
            }
        if(!num_region_files){
            if(!(nsubframereg=(int*)malloc(sizeof*nsubframereg*dp->tdim_max*num_regions))){
                printf("fidlError: Unable to malloc nsubframereg\n");
                exit(-1);
                }
            for(i=0;i<dp->tdim_max*num_regions;i++){subframereg[i]=0.;nsubframereg[i]=0;}
            }
        }
    if(((avgacrossfiles||sumacrossfiles)&&!num_region_files)||(dp->filetypeall==(int)CIFTI||dp->filetypeall==(int)CIFTI_DTSERIES)){
        if(!(temp_float2=(float*)malloc(sizeof*temp_float2*ap->vol*dp->tdim_max)))
            {printf("fidlError: Unable to malloc temp_float2\n");exit(-1);}
        }
    if(!(temp_float=(float*)malloc(sizeof*temp_float*ap->vol))) {
        printf("fidlError: Unable to malloc temp_float\n");
        exit(-1);
        }
    if(!(temp_double=(double*)malloc(sizeof*temp_double*ap->vol))) {
        printf("fidlError: Unable to malloc temp_double\n");
        exit(-1);
        }
    if(!(stat=(double*)malloc(sizeof*stat*ap->vol))) {
        printf("fidlError: Unable to malloc stat\n");
        exit(-1);
        }
    if(fwhm>0.||num_region_files){
        if(!(td1=(double*)malloc(sizeof*td1*(fwhm>0.?ap->vol:num_regions)))){
            printf("fidlError: Unable to malloc td1\n");
            exit(-1);
            }
        }
    if(normstr[0]) {
        if(!(mean=(double*)malloc(sizeof*mean*dp->volall))) {
            printf("fidlError: Unable to malloc mean\n");
            exit(-1);
            }
        }
    if(fwhm>0.){if(!(gs=gauss_smoth2_init(ap->xdim,ap->ydim,ap->zdim,fwhm,fwhm)))exit(-1);}
    }
if(!(fbf=find_regions_by_file(tc_files->nfiles,fs->nframes,dp->tdim,fs->frames))) exit(-1);

if(!lc_names_only){
    n = chunks > 1 ? chunkvol : num_regions;
    p = chunks > 1 ? chunkvol : 1;

    //START170720
    if(dp->filetypeall==(int)NIFTI&&lccompress){
        if(!(ifh=init_ifh(4,n,1,1,1,ms->voxel_size_1,ms->voxel_size_2,ms->voxel_size_3,0,ms->center,ms->mmppix)))exit(-1);
        ifh->fwhm=fwhm;
        }

    }

for(jj=r=l=0;l<fs->nframes;) {
    for(dummy=sum_tdim=i1=0;i1<tc_files->nfiles;i1++) {
        if(fbf->num_regions_by_file[i1]) {
            dummy += fbf->num_regions_by_file[i1];
            sum_tdim += dp->tdim[i1];
            if(!lc_names_only) {
                if(dp->filetypeall==(int)IMG){
                    if(!(mm=map_disk(tc_files->files[i1],dp->vol[i1]*dp->tdim[i1],0,sizeof(float)))) exit(-1);
                    if(!(ifh = read_ifh(tc_files->files[i1],(Interfile_header*)NULL))) exit(-1);
                    ifh->dim4 = 1;
                    ifh->fwhm=fwhm;
                    if(num_region_files||(mask_file&&lccompress)||chunks>1){
                        ifh->dim1 = n;
                        ifh->dim2 = 1;
                        ifh->dim3 = 1;
                        }
                    }
                else if(dp->filetypeall==(int)CIFTI||dp->filetypeall==(int)CIFTI_DTSERIES){
                    if(!cifti_getstack(tc_files->files[i1],temp_float2))exit(-1);
                    }
                else if(!nS.niftiStack0(tc_files->files[i1]))exit(-1); 
                if(normstr[0]) {
                    for(i=0;i<dp->vol[i1];i++)mean[i]=0.;
                    if(!dp->swapbytes[i1]) {
                        for(k=i=0;i<dp->tdim[i1];i++)for(j=0;j<dp->vol[i1];j++,k++)mean[j]+=(double)mm->ptr[k];
                        }
                    else {
                        for(k=i=0;i<dp->tdim[i1];i++) {
                            for(j=0;j<dp->vol[i1];j++,k++)temp_float[j]=mm->ptr[k];
                            swap_bytes((unsigned char *)temp_float,sizeof(float),(size_t)dp->vol[i1]);
                            for(j=0;j<dp->vol[i1];j++,k++)mean[j]+=(double)temp_float[k];
                            }
                        }
                    for(i=0;i<dp->vol[i1];i++)mean[i]=mean[i]<.000000000001?1:mean[i]/(double)dp->tdim[i1];
                    }
                }
            if(!avgacrossfiles&&!sumacrossfiles){
                strcpy(string,tc_files->files[i1]);
                if(!(strptr=get_tail_sans_ext(string)))exit(-1);
                }
            for(q=0,i=0;i<fbf->num_regions_by_file[i1]&&fbf->file_index[l]==(int)i1;i++,l++) {

                //if(!avgacrossfiles){
                //START170616
                if(!avgacrossfiles&&!sumacrossfiles){

                    if(!q){sprintf(write_back,"%s_%d",strptr,fbf->roi_index[l]+1);}
                    else{sprintf(string2,"+%d",fbf->roi_by_file[i1][i]+1);strcat(write_back,string2);}
                    }
                if(!lc_names_only) {

                    //printf("fbf->num_regions_by_file[%zd]=%d i1=%zd q=%d\n",i1,fbf->num_regions_by_file[i1],i1,q);

                    if(!q)for(j=0;j<dp->vol[i1];j++)temp_double[j]=0;

                    //printf("fbf->roi_index[%d]=%d fbf->roi_by_file[%zd][%d]=%d\n",l,fbf->roi_index[l],i1,i,fbf->roi_by_file[i1][i]);
                    if(dp->filetypeall==(int)IMG){
                        for(k=dp->vol[i1]*fbf->roi_index[l],j=0;j<dp->vol[i1];j++,k++)temp_float[j]=mm->ptr[k];
                        if(dp->swapbytes[i1])swap_bytes((unsigned char *)temp_float,sizeof(float),(size_t)dp->vol[i1]);
                        }
                    else if(dp->filetypeall==(int)CIFTI||dp->filetypeall==(int)CIFTI_DTSERIES){
                        //ADD
                        }
                    else{

                        //if(!nS.niftiVol((int64_t)fbf->roi_index[l],temp_float,0))exit(-1);
                        //START180302
                        if(!nS.niftiVol((int64_t)fbf->roi_index[l],temp_float,lcflip))exit(-1);

                        }

                    for(j=0;j<dp->vol[i1];j++) {
                        if(temp_float[j]==(float)UNSAMPLED_VOXEL) {
                            temp_double[j] = (double)UNSAMPLED_VOXEL;
                            }
                        else if(temp_double[j]==(double)UNSAMPLED_VOXEL) {
                            /*do nothing*/
                            }
                        else if(num_tc_weights) {
                            temp_double[j] += tc_weights->tc[tc_weights->eachi[r]+q]*(double)temp_float[j];
                            }
                        else {
                            temp_double[j] += (double)temp_float[j];
                            }
                        }
                    if(q == (fs->num_frames_to_sum[r]-1)) {
                        if(fs->num_frames_to_sum[r]>1){
                            for(j=0;j<dp->vol[i1];j++)
                                if(temp_double[j]!=(double)UNSAMPLED_VOXEL)temp_double[j]/=(double)fs->num_frames_to_sum[r];
                            }
                        if(normstr[0]){
                            for(j=0;j<dp->vol[i1];j++)if(temp_double[j]!=(double)UNSAMPLED_VOXEL)temp_double[j]/=mean[j];
                            }
                        dptr=temp_double;
                        if(xform_file){
                            if(!t4_atlas(temp_double,stat,&t4[t4_identify[i1]*(int)T4SIZE],dp->xdim[i1],dp->ydim[i1],dp->zdim[i1],
                                dp->dxdy[i1],dp->dz[i1],A_or_B_or_U[t4_identify[i1]],dp->orientation[i1],ap,(double*)NULL))exit(-1);
                            dptr=stat;
                            }
                        if(fwhm>0.){
                            gauss_smoth2(dptr,td1,gs);
                            dptr=td1;
                            }

                        if(num_region_files){
                            crs(dptr,td1,rbf,(char*)NULL);

                            //if(!avgacrossfiles){
                            //START170616
                            if(!avgacrossfiles&&!sumacrossfiles){ 

                                for(j=0;j<num_regions;j++)temp_float[j]=(float)td1[j];
                                }
                            else{
                                for(j=0;j<num_regions;j++)subframereg[jj++]=td1[j];
                                }
                            }
                        else{

                            //if(!avgacrossfiles){
                            //START170616
                            if(!avgacrossfiles&&!sumacrossfiles){ 

                                for(j=0;j<ms->lenbrain;j++)temp_float[j]=dptr[ms->brnidx[j]]==(double)UNSAMPLED_VOXEL?
                                    (float)UNSAMPLED_VOXEL:(float)dptr[ms->brnidx[j]];
                                }
                            else{
                                for(k=ms->lenbrain*fbf->roi_index[l],j=0;j<ms->lenbrain;j++,k++){
                                    if(dptr[ms->brnidx[j]]!=(double)UNSAMPLED_VOXEL){
                                        subframereg[k]+=dptr[ms->brnidx[j]];
                                        nsubframereg[k]++;
                                        }
                                    }
                                }
                            }

                        }
                    }

                if(q++ == (fs->num_frames_to_sum[r]-1)) {
                    q = 0;
                    r++;
                    if(!avgacrossfiles&&!sumacrossfiles){ 
                        for(j=0;j<chunks;j++) {

                            strcpy(filename,write_back);
                            if(chunks > 1) {
                                sprintf(string2,"_CHUNK%d",j+1);
                                strcat(filename,string2);
                                }
                            if(num_tc_weights) {
                                sprintf(string2,"w%d",r);
                                strcat(filename,string2);
                                }

                            strcat(filename,normstr);
                            strcat(filename,fwhmstr);
                            if(scratchdir) sprintf(filename,"%s_%d",filename,l);
                            strcat(filename,".4dfp.img");

                            #if 0
                            if(!lc_names_only){
                                min_and_max_floatstack(&temp_float[j*p],n,&ifh->global_min,&ifh->global_max);
                                ifh->bigendian=shouldiswap(SunOS_Linux,swapbytes); 
                                }
                            #endif
                            //START170720
                            if(!lc_names_only){
                                ifh->bigendian=SunOS_Linux?0:1;
                                if(!lccompress)min_and_max_floatstack(&temp_float[j*p],n,&ifh->global_min,&ifh->global_max);
                                }




                            for(k=0;k<nscratchdir;k++){
                                sprintf(filename0,"%s%s",!scratchdir?"":scratchdir->files[k],filename);
                                if(!lc_names_only){

                                    //if(!writestack(filename0,&temp_float[j*p],sizeof(float),(size_t)n,swapbytes)) exit(-1);
                                    //START170720
                                    if(!writestack(filename0,&temp_float[j*p],sizeof(float),(size_t)n,0))exit(-1);

                                    if(!write_ifh(filename0,ifh,0)) exit(-1);
                                    }
                                printf("Time courses written to %s\n",filename0);
                                }
                            }
                        }
                    }
                }
            if(!lc_names_only) {

                #if 0
                free_ifh(ifh,0);
                if(dp->filetypeall==(int)IMG){if(!unmap_disk(mm))exit(-1);}else if(dp->filetypeall==(int)NIFTI){nifti_free(niftiPtr);}
                #endif
                //START170620
                if(dp->filetypeall==(int)IMG){free_ifh(ifh,0);if(!unmap_disk(mm))exit(-1);}
                else if(dp->filetypeall==(int)NIFTI){nS.~niftiStack();}

                }
            }
        }
    }

//if(avgacrossfiles) {
//START170616
if(avgacrossfiles||sumacrossfiles){

    if(num_region_files){
        if(!(sum_framereg=(double*)malloc(sizeof*sum_framereg*dp->tdim_max*num_regions))) {
            printf("fidlError: Unable to malloc sum_framereg\n");
            exit(-1);
            }
        for(i=0;i<dp->tdim_max*num_regions;i++) sum_framereg[i]=0.;
        if(!(sum2_framereg=(double*)malloc(sizeof*sum2_framereg*dp->tdim_max*num_regions))) {
            printf("fidlError: Unable to malloc sum2_framereg\n");
            exit(-1);
            }
        for(i=0;i<dp->tdim_max*num_regions;i++) sum2_framereg[i]=0.;
        if(!(nframe=(int*)malloc(sizeof*nframe*dp->tdim_max))) {
            printf("fidlError: Unable to malloc nframe\n");
            exit(-1);
            }
        for(i=0;i<dp->tdim_max;i++) nframe[i]=0;
        for(jj=i1=0;i1<tc_files->nfiles;i1++) {
            for(i=0;i<fbf->num_regions_by_file[i1];i++) {
                nframe[fbf->roi_by_file[i1][i]]++;
                for(k=fbf->roi_by_file[i1][i]*num_regions,j=0;j<num_regions;j++,jj++,k++) {
                    sum_framereg[k]+=subframereg[jj];
                    sum2_framereg[k]+=subframereg[jj]*subframereg[jj];
                    }
                }
            }
        for(ii=i=0;i<dp->tdim_max;i++,ii+=num_regions) {
            if(nframe[i]) {
                for(jj=ii,j=0;j<num_regions;j++,jj++) {
                    sum2_framereg[jj] = sqrt((sum2_framereg[jj]-sum_framereg[jj]*sum_framereg[jj]/(double)nframe[i])/
                        (double)(nframe[i]-1)/(double)nframe[i]);
                    sum_framereg[jj] /= (double)nframe[i];
                    }
                }
            }

        //if(!(fp=fopen_sub(avgacrossfiles,"w"))) exit(-1);
        //for(j=0;j<num_regions;j++)fprintf(fp,"\t%s %d",rbf->region_names_ptr[j],rbf->nvoxels_region[j]); fprintf(fp,"\n");
        //START180124
        if(!get_tail_sans_ext(avgacrossfiles))exit(-1);
        sprintf(filename,"%s_meansem.txt",avgacrossfiles);
        sprintf(string2,"%s_mean95CI.txt",avgacrossfiles);
        if(!(fp=fopen_sub(filename,"w")))exit(-1);
        if(!(fp1=fopen_sub(string2,"w")))exit(-1);
        for(j=0;j<num_regions;j++)fprintf(fp,"\t%s %d",rbf->region_names_ptr[j],rbf->nvoxels_region[j]); fprintf(fp,"\n");
        for(j=0;j<num_regions;j++)fprintf(fp1,"\t%s %d",rbf->region_names_ptr[j],rbf->nvoxels_region[j]); fprintf(fp1,"\n");


        for(ii=i=0;i<dp->tdim_max;i++,ii+=num_regions) {
            if(TR>0.&&starttime!=(double)UNSAMPLED_VOXEL) {
                fprintf(fp,"%f",starttime+i*TR);
                if(label) fprintf(fp,"_%s",label);
                }
            else if(label) fprintf(fp,"%s",label);

            //START180124
            if(TR>0.&&starttime!=(double)UNSAMPLED_VOXEL) {
                fprintf(fp1,"%f",starttime+i*TR);
                if(label) fprintf(fp1,"_%s",label);
                }
            else if(label) fprintf(fp1,"%s",label);


            //for(jj=ii,j=0;j<num_regions;j++,jj++) fprintf(fp,"\t%f %f",sum_framereg[jj],sum2_framereg[jj]);
            //fprintf(fp,"\n");
            //START180124
            if(nframe[i]>1){
                td=gsl_cdf_tdist_Qinv(.025,(double)(nframe[i]-1)); 
                printf("t(.05/2,df=%d)=%f\n",nframe[i]-1,td);     
                }
            for(jj=ii,j=0;j<num_regions;j++,jj++){
                fprintf(fp,"\t%f %f",sum_framereg[jj],sum2_framereg[jj]);
                fprintf(fp1,"\t%f %f",sum_framereg[jj],nframe[i]>1?sum2_framereg[jj]*td:sum2_framereg[jj]);
                }
            fprintf(fp,"\n");
            fprintf(fp1,"\n");

            }

        //fflush(fp);fclose(fp);printf("Output written to %s\n",avgacrossfiles);fflush(stdout);
        //START180124
        fflush(fp);fclose(fp);printf("Output written to %s\n",filename);
        fflush(fp1);fclose(fp1);printf("Output written to %s\n",string2);

        if(scratchdir){
            sprintf(string,"rm -rf %s",scratchdir->files[0]);
            if(system(string) == -1) printf("fidlError: unable to %s\n",string);
            }

        //START180124
        //if(!get_tail_sans_ext(avgacrossfiles))exit(-1);

        sprintf(filename,"%s_ind.txt",avgacrossfiles);
        if(!(fp=fopen_sub(filename,"w")))exit(-1);
        for(i=0;i<dp->tdim_max;i++){
            if(dp->tdim_max>1)fprintf(fp,"FRAME %d\n",i+1);
            for(j=0;j<num_regions;j++)fprintf(fp,"\t%s %d",rbf->region_names_ptr[j],rbf->nvoxels_region[j]);
            fprintf(fp,"\n");
            for(i1=0;i1<tc_files->nfiles;i1++){
                if(i<fbf->num_regions_by_file[i1]){
                    strcpy(string,tc_files->files[i1]);
                    if(!(strptr=get_tail_sans_ext(string)))exit(-1);
                    fprintf(fp,"%s",strptr);
                    for(jj=i*num_regions,j1=0;j1<i1;j1++)jj+=fbf->num_regions_by_file[j1]*num_regions;
                    for(j=0;j<num_regions;j++)fprintf(fp,"\t%f",subframereg[jj++]);
                    fprintf(fp,"\n");
                    }
                }
            }

        //printf("Individual subject values written to %s\n",filename);fflush(stdout);
        //START180124
        fflush(fp);fclose(fp);printf("Individual subject values written to %s\n",filename);fflush(stdout);

        if(num_frames){
            sprintf(string,"rm -f %s",frames_file);
            if(system(string)==-1)printf("fidError: Unable to %s\n",string);
            }
        }
    else{
        if(!(w1=write1_init()))exit(-1);
        w1->filetype=dp->filetypeall;
        w1->swapbytes=0;
        w1->temp_float=temp_float2;
        if(dp->filetypeall==(int)IMG){
            w1->temp_double=subframereg;;
            w1->lenbrain=ms->lenbrain*dp->tdim_max;
            w1->how_many=ap->vol*dp->tdim_max;
            if(!(ifh=read_ifh(tc_files->files[0],(Interfile_header*)NULL)))exit(-1);
            ifh->dim1=ap->xdim;ifh->dim2=ap->ydim;ifh->dim3=ap->zdim;ifh->dim4=dp->tdim_max;
            ifh->center[0]=(float)ap->center[0];ifh->center[1]=(float)ap->center[1];ifh->center[2]=(float)ap->center[2];
            ifh->mmppix[0]=(float)ap->mmppix[0];ifh->mmppix[1]=(float)ap->mmppix[1];ifh->mmppix[2]=(float)ap->mmppix[2];
            w1->ifh=ifh;
            }
        else if(dp->filetypeall==(int)CIFTI||dp->filetypeall==(int)CIFTI_DTSERIES){
            if(!(xmls=cifti_getxml(tc_files->files[0])))exit(-1);
            w1->xmlsize=xmls->size;;
            w1->cifti_xmldata=xmls->data;
            dims[0]=(int64_t)dp->xdim[0];dims[1]=(int64_t)dp->ydim[0];dims[2]=(int64_t)dp->zdim[0];
            w1->dims=dims;
            w1->tdim=(int64_t)dp->tdim_max;
            }
        else if(dp->filetypeall==(int)NIFTI){
            w1->file_hdr=tc_files->files[0];
            }

        #if 0
        for(l=k=i=0;i<dp->tdim_max;i++,l+=ap->vol)for(j=0;j<ms->lenbrain;j++,k++)
            temp_float2[l+ms->brnidx[j]]=subframereg[k]/nsubframereg[k];
        if(!write1(avgacrossfiles,w1))exit(-1);
        printf("Output written to %s\n",avgacrossfiles);
        #endif
        //START170616
        if(sumacrossfiles){
            sprintf(filename0,"%s%s",sumacrossfiles,Fileext[dp->filetype[0]]);
            for(l=k=i=0;i<dp->tdim_max;i++,l+=ap->vol)for(j=0;j<ms->lenbrain;j++,k++)temp_float2[l+ms->brnidx[j]]=subframereg[k];

            for(m=l=k=i=0;i<dp->tdim_max;i++,l+=ap->vol)for(j=0;j<ms->lenbrain;j++,k++){
                //m+=nsubframereg[k];
                //if(nsubframereg[k]>1)printf("subframereg[%d]=%f nsubframereg[%d]=%d\n",k,subframereg[k],k,nsubframereg[k]);
                if(subframereg[k]>0.)printf("subframereg[%d]=%f nsubframereg[%d]=%d\n",k,subframereg[k],k,nsubframereg[k]);
                }
            //printf("m=%d\n",m);

            if(!write1(filename0,w1))exit(-1);
            printf("Output written to %s\n",filename0);
            }
        if(avgacrossfiles){
            for(l=k=i=0;i<dp->tdim_max;i++,l+=ap->vol)for(j=0;j<ms->lenbrain;j++,k++) 
                temp_float2[l+ms->brnidx[j]]=subframereg[k]/nsubframereg[k];
            if(!write1(avgacrossfiles,w1))exit(-1);
            printf("Output written to %s\n",avgacrossfiles);
            }


        if(num_frames){
            sprintf(string,"rm -f %s",frames_file);
            if(system(string)==-1)printf("fidError: Unable to %s\n",string);
            }
        }
    }

//START170224
if(cleanup){
    sprintf(string,"rm -rf %s",cleanup);
    if(system(string)==-1)printf("fidlError: fidl_bolds  Unable to %s\n",string);
    }

exit(0);
}
