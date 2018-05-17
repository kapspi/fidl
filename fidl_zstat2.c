/* Copyright 9/22/16 Washington University.  All Rights Reserved.
   fidl_zstat2.c  $Revision: 1.7 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "fidl.h"
#include "gauss_smoth2.h"
#include "subs_mask.h"
#include "shouldiswap.h"
#include "subs_util.h"
#include "minmax.h"
#include "filetype.h"
#include "subs_nifti.h"
#include "subs_cifti.h"
#include "write1.h"
#include "get_atlas_param.h"
#include "t4_atlas.h"
#include "get_grand_mean_struct.h"

//START180412
#include "subs_glm.h"
#include "region.h"
#include "find_regions_by_file_cover.h"
#include "tags_tc_glmstack.h"
#include "get_limits.h"
#include "checkOS.h"
#include "check_dimensions.h"
#include "region.h"
#include "get_atlas.h"
#include "read_xform.h"
#include "twoA_or_twoB.h"
#include "get_indbase.h"

static char rcsid[] = "$Header: /home/hannah/mcavoy/idl/clib/RCS/fidl_zstat2.c,v 1.7 2016/12/20 18:39:24 mcavoy Exp $";
int main(int argc,char **argv)
{
char *input_file,*mask_file=NULL,string[MAXNAME],string2[MAXNAME],appendstr[MAXNAME],*stringptr,*one_file_root=NULL,delay_type_str[3],
    magnorm_str[2],delay_str[7],unscaled_str[2],*scratchdir=NULL,*strptr,strfpw[MAXNAME],**input_file_ptr,*dir=NULL,toolongs[MAXNAME],
    atlas[7]="";
int i,j,k,m,n,p,kk,vol=0,*contrast,c_index,num_contrasts=0,lcmag=0,lcsd=0,lcvar=0,lcmag_only=0,num_tc=0,how_many=0,num_delays=3,
    delay_flag=0,delay,cnorm_index=0,num_regions=0,*roi=NULL,lc_one_file=0,lc_anova=0,argc_tc=0,anova_regions_loop=1,
    *A_or_B_or_U=NULL,lc_names_only=0,num_region_files=0,chunks=1,period_one_file,dont_use_c,
    lc_largest_mag=0,*which_delay,lenvol_region=0,*temp_int,argc_tc_weights=0,num_tc_weights=0,lctstat=0,SunOS_Linux,swapbytesout,
    scratch=0,chunkvol,lcmsqa=0,*swapbytesin,lcquiet=0,bigendian=1,nfpw=0,nglmfiles=0,*Mcol,*nc,*contrasti=NULL,glmi=0,nxformfiles=0,
    logic_glm,logic_gms,tci,nregions=0,Mcolmax,lcindbase=0,*indbase_col=NULL,*t_to_zi=NULL,filetype=IMG;
long *startb;
float *temp_float=NULL,*t4=NULL,delay_inc=1,init_delay=0,dt,*c=NULL,*dlymaxmin=NULL,mag_min,mag_max; 
double *mag=NULL,*stat=NULL,*temp_double=NULL,scl=0.,*magmaxmin,*scl_stack,*scl_mag_stack,*hipass,*td=NULL,*df=NULL,*dptr,fwhm=0.,
   fwhm0=0.,*scl_stack2,*scl_mag_stack2;
FILE *fp,*fpw=NULL;
LinearModel **glmstack;
Interfile_header *ifh=NULL,*ifh_out,*ifh_out1=NULL;
Regions **reg;
Regions_By_File *rbf=NULL,*fbf;
Mask_Struct *ms;
TC *tcs,*tc_weights=NULL;
Tags_Tc_Struct2 *tags_tcs;
Atlas_Param *ap;
Files_Struct *region_files=NULL,*names_tc,*glmfiles=NULL,*xformfiles=NULL;
Grand_Mean_Struct **gms_stack=NULL;
Meancol **meancol_stack=NULL;
size_t st1,*filel;
Limits limits;
gauss_smoth2_struct *gs=NULL;
W1 *w1=NULL;

//int64_t dims[3];
//START161214
int64_t dims[4];

if(argc<3){
    fprintf(stderr,"Usage: compute_zstat -glm_file file_name -contrasts n1 n2 ... nn -tags t1 t2 ... tn\n");
    fprintf(stderr,"    -glm_file:                       Glm files\n");
    fprintf(stderr,"    -mag                             Store magnitude images.\n");
    fprintf(stderr,"    -mag ONLY                        Compute and store only magnitude images.\n"); 
    fprintf(stderr,"    -sd                              Store standard deviation images.\n");
    fprintf(stderr,"    -var                             Output variance.\n");
    fprintf(stderr,"    -contrasts:                      List of contrasts to be analyzed. The first contrast is 1.\n");
    fprintf(stderr,"    -xform_file:                     List of t4 files defining the transform to atlas space. One for each glm.\n");
    fprintf(stderr,"    -atlas:                          Either 111, 222, 333, 222MNI. Use with -xform_file\n");
    fprintf(stderr,"    -gauss_smoth:                   Amount of smoothing to be done in units of fwhm with a 3D gaussian filter.\n");
    fprintf(stderr,"                                    Magnitudes and time courses are smoothed after being put in atlas space.\n");
    fprintf(stderr,"    -compress:                      Only store nonzero voxels. You may specify a mask file.\n");
    fprintf(stderr,"                                    If no file is given, then the glm_atlas_mask is used by default.\n\n");
    fprintf(stderr,"    -region_file:                   *.4dfp.img file(s) that specifies regions of interest.\n");
    fprintf(stderr,"                                    Magnitudes and time courses are averaged over the region.\n");
    fprintf(stderr,"    -regions_of_interest:           Calculate statistics for selected regions in the region file(s).\n"); 
    fprintf(stderr,"                                    First region is one.\n"); 
    fprintf(stderr,"    -anova                          The timecourse or magnitude for each region is put in a separate\n");
    fprintf(stderr,"                                    file. The region name is appended to the filename to facilitate an\n"); 
    fprintf(stderr,"                                    online regional anova.\n"); 
    fprintf(stderr,"    -names_only                     Generate filenames for creation of the anova driving file.\n"); 
    fprintf(stderr,"                                    No computation is performed. No files are created.\n\n"); 
    fprintf(stderr,"    -chunks                         Breaks the compressed anova scratch files into chunks.\n"); 
    fprintf(stderr,"                                    This is done for large designs that could exceed the 2 gig limit.\n\n"); 
    fprintf(stderr,"    The following three options are used to compute z-statistics for contrasts with delays.\n");
    fprintf(stderr,"    New contrasts using the Boynton model are computed for each specified delay.\n");
    fprintf(stderr,"    All three options must be included for this processing to take place.\n");
    fprintf(stderr,"    -number_of_delays:              default = 3\n");
    fprintf(stderr,"    -delay_increment_in_seconds:    default = 1\n");
    fprintf(stderr,"    -initial_delay:                 default = 0 seconds (Time-shift from default (2.00 seconds))\n");
    fprintf(stderr,"    -largest_tstat                  Selected delay will yield the largest absolute tstat. This is the default.\n");
    fprintf(stderr,"    -largest_mag                    Selected delay will yield the largest absolute mag.\n");
    fprintf(stderr,"    -magnorm                        Contrasts are normalized to have a magnitude of 1.\n");
    fprintf(stderr,"                                    This is used for ANOVAs and t-tests, since the number of conditions\n");
    fprintf(stderr,"                                    in a given contrast can vary from subject to subject.\n");
    fprintf(stderr,"    -unscaled                  Timecourses and magnitudes in MR units.\n");
    fprintf(stderr,"    -scratchdir:               Scratch directory. Where to write the scratch files.\n"); 
    fprintf(stderr,"                               Include the backslash at the end.\n");
    fprintf(stderr,"    -tc_weights                Weighting to apply to summed timepoints (eg contrast).\n");
    fprintf(stderr,"    -tstat                     Output T statistics.\n");
    fprintf(stderr,"    -msqa                      Mean square amplitued power map. Default is root msqa.\n");
    fprintf(stderr,"    -littleendian              Write output in little endian. Big endian is the default.\n");
    fprintf(stderr,"                               All scratch files are written in the native endian.\n");
    fprintf(stderr,"    -quiet                     Echos only what glm is being processed.\n");
    fprintf(stderr,"    -one_file:                 Output written to a single file.\n");
    fprintf(stderr,"    -indbase                   Individual baseline applied to each effect for normalization to percent change.\n");
    fprintf(stderr,"    -dir:                      Directory. Where to write the output. Include the backslash at the end.\n"); 
    exit(-1);
    }
limits=get_limits();
magnorm_str[0] = 0; /*This must be up here.*/
delay_str[0] = 0; 
unscaled_str[0] = 0;
for(i=1;i<argc;i++) { 
    if(!strcmp(argv[i],"-glm_file") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++nglmfiles;
        if(!(glmfiles=get_files(nglmfiles,&argv[i+1]))) exit(-1);
        i += nglmfiles;
        }
    if(!strcmp(argv[i],"-mag")) {
        lcmag = 1;
        if(argc > i+1 && !strcmp(argv[i+1],"ONLY")) {
            lcmag_only = 1;
            ++i;
            }
        }
    if(!strcmp(argv[i],"-sd"))
        lcsd = 1;
    if(!strcmp(argv[i],"-var"))
        lcvar = 1;
    if(!strcmp(argv[i],"-contrasts") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_contrasts;
        if(!(contrast=malloc(sizeof*contrast*num_contrasts))) {
            printf("fidlError: Unable to malloc contrast\n");
            exit(-1);
            }
        if(!(contrasti=malloc(sizeof*contrasti*num_contrasts))) {
            printf("fidlError: Unable to malloc contrasti\n");
            exit(-1);
            }
        for(j=0;j<num_contrasts;j++) {
            contrast[j] = atoi(argv[++i]);
            contrasti[j] = contrast[j]-1;
            }
        }
    if(!strcmp(argv[i],"-xform_file") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++nxformfiles;
        if(!(xformfiles=get_files(nxformfiles,&argv[i+1]))) exit(-1);
        i += nxformfiles;
        }
    if(!strcmp(argv[i],"-gauss_smoth") && argc > i+1) {
	fwhm = atof(argv[++i]);
	sprintf(string,"_fwhm%.1f",fwhm);
	}
    if(!strcmp(argv[i],"-compress")) {
        if(argc > i+1 && strchr(argv[i+1],'-') != argv[i+1]) {
            mask_file = argv[++i];
            }
	else {
            printf("fidlError: No mask specified after -compress option. Abort!\n");
            exit(-1);
	    }
        }
    if(!strcmp(argv[i],"-region_file") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_region_files;
        if(!(region_files=get_files(num_region_files,&argv[i+1]))) exit(-1);
        i += num_region_files;
        }
    if(!strcmp(argv[i],"-regions_of_interest") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_regions;
        if(!(roi=malloc(sizeof*roi*num_regions))) {
            printf("fidlError: Unable to malloc roi\n");
            exit(-1);
            }
        for(j=0;j<num_regions;j++) roi[j] = atoi(argv[++i]) - 1;
	}
    if(!strcmp(argv[i],"-anova"))
        lc_anova = 1;
    if(!strcmp(argv[i],"-names_only"))
        lc_names_only = 1;
    if(!strcmp(argv[i],"-number_of_delays")) {
	++delay_flag;
        if(argc > i+1 && strchr(argv[i+1],'-') != argv[i+1]) num_delays = atoi(argv[++i]);
        }
    if(!strcmp(argv[i],"-delay_increment_in_seconds")) {
        ++delay_flag;
        if(argc > i+1 && strchr(argv[i+1],'-') != argv[i+1]) delay_inc = atoi(argv[++i]);
	}
    if(!strcmp(argv[i],"-initial_delay")) {
        ++delay_flag;
        if(argc > i+1) {
            k = strlen(argv[i+1]);
            for(m=j=0;j<k;j++) if(isdigit((int)argv[i+1][j])) m = 1; /*printf("%c\n",argv[i+1][j]);*/
            if(m) init_delay = atof(argv[++i]);
            } 
	}
    if(!strcmp(argv[i],"-largest_mag"))
        lc_largest_mag = 1;
    if(!strcmp(argv[i],"-magnorm")) {
        strcpy(magnorm_str,"1");
        }
    if(!strcmp(argv[i],"-chunks"))
        chunks = 20;
    if(!strcmp(argv[i],"-atlas") && argc > i+1)
        strcpy(atlas,argv[++i]);
    if(strstr(argv[i],"-unscaled")) { /*Need strstr so -unscaled_mag will also work.*/
        strcpy(unscaled_str,"U");
        }
    if(!strcmp(argv[i],"-scratchdir") && argc > i+1) {
        scratchdir = argv[++i];
        }
    if(!strcmp(argv[i],"-tc") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_tc;
        if(!(names_tc=get_files(num_tc,&argv[i+1]))) exit(-1);
        argc_tc = i+1;
        i += num_tc;
        }
    if(!strcmp(argv[i],"-tc_weights") && argc > i+1) {
        argc_tc_weights = i+1;
        for(j=1;i+j<argc;j++) {
            stringptr = argv[i+j] + 1;
            if(*stringptr == '.') stringptr++;
            if(strchr(argv[i+j],'-') == argv[i+j] && !isdigit((int)(*stringptr))) break;
            ++num_tc_weights;
            }
        i += num_tc_weights;
        }
    if(!strcmp(argv[i],"-tstat"))
        lctstat = 1;
    if(!strcmp(argv[i],"-msqa"))
        lcmsqa = 1;
    if(!strcmp(argv[i],"-littleendian"))
        bigendian = 0;
    if(!strcmp(argv[i],"-quiet"))
        lcquiet = 1;
    if(!strcmp(argv[i],"-one_file")) {
        lc_one_file = 1;
        if(argc > i+1 && strchr(argv[i+1],'-') != argv[i+1]) one_file_root = argv[++i];
        }
    if(!strcmp(argv[i],"-indbase"))
        lcindbase = 1;
    if(!strcmp(argv[i],"-dir") && argc > i+1)
        dir = argv[++i];
    }
if(!lc_names_only) print_version_number(rcsid,stdout);
if(anova_regions_loop>1 && chunks>1) {
    printf("fidlError: anova_regions_loop = %d  chunks = %d\nThey both can't be greater than one.\n",anova_regions_loop,chunks);
    exit(-1);
    }
if((SunOS_Linux=checkOS())==-1) exit(-1);
swapbytesout = shouldiswap(SunOS_Linux,bigendian);
if(scratchdir) swapbytesout = 0;
strcpy(delay_type_str,!lc_largest_mag?"LT":"LM");
if(delay_flag == 3) sprintf(delay_str,"_d%d%s",num_delays,delay_type_str);
appendstr[0] = 0;
if(!(glmstack=malloc(sizeof*glmstack*nglmfiles))) {
    printf("fidlError: Unable to malloc glmstack\n");
    exit(-1);
    }
if(!lcindbase) {
    if(!(gms_stack=malloc(sizeof*gms_stack*nglmfiles))) {
        printf("fidlError: Unable to malloc gms_stack\n");
        exit(-1);
        }
    }
else {
    if(!(meancol_stack=malloc(sizeof*meancol_stack*nglmfiles))) {
        printf("fidlError: Unable to malloc meancol_stack\n");
        exit(-1);
        }
    }
if(!(startb=malloc(sizeof*startb*nglmfiles))) {
    printf("fidlError: Unable to malloc startb\n");
    exit(-1);
    }
if(!(Mcol=malloc(sizeof*Mcol*nglmfiles))) {
    printf("fidlError: Unable to malloc Mcol\n");
    exit(-1);
    }
if(!(nc=malloc(sizeof*nc*nglmfiles))) {
    printf("fidlError: Unable to malloc nc\n");
    exit(-1);
    }
if(!(swapbytesin=malloc(sizeof*swapbytesin*nglmfiles))) {
    printf("fidlError: Unable to malloc swapbytesin\n");
    exit(-1);
    }

if(!(temp_int=malloc(sizeof*temp_int*nglmfiles))) {
    printf("fidlError: Unable to malloc temp_int\n");
    exit(-1);
    }
if(!(temp_int=malloc(sizeof*temp_int*nglmfiles*2))) {
    printf("fidlError: Unable to malloc temp_int\n");
    exit(-1);
    }
logic_glm = (lc_names_only&&!lc_anova)==1 ? 1 : (int)SMALL; 
logic_gms = (!lc_names_only&&!unscaled_str[0])||lc_anova;
for(kk=nglmfiles,Mcolmax=k=i=0;i<nglmfiles;i++) {
    if(!(glmstack[i]=read_glm(glmfiles->files[i],logic_glm,SunOS_Linux))) {
        printf("fidlError: reading %s  Abort!\n",glmfiles->files[i]);
        exit(-1);
        }
    if(!i){
        vol=glmstack[i]->ifh->glm_xdim*glmstack[i]->ifh->glm_ydim*glmstack[i]->ifh->glm_zdim;
        }
    else{
        if(vol!=glmstack[i]->ifh->glm_xdim*glmstack[i]->ifh->glm_ydim*glmstack[i]->ifh->glm_zdim) temp_int[k++] = i;
        if(glmstack[0]->ifh->glm_fwhm!=glmstack[i]->ifh->glm_fwhm) temp_int[kk++] = i;
        }
    if(logic_glm==(int)SMALL&&logic_gms) {
        if(!lcindbase) {

            //if(!(gms_stack[i]=get_grand_mean_struct(glmstack[i]->grand_mean,vol,glmfiles->files[i],(FILE*)NULL))) exit(-1);
            //START160920
            if(!(gms_stack[i]=get_grand_mean(glmstack[i],glmfiles->files[i],(FILE*)NULL)))exit(-1);

            }
        else {
            if(!(meancol_stack[i]=get_meancol(glmstack[i]->ifh,0))) exit(-1);
            }
        free(glmstack[i]->grand_mean);
        }
    startb[i] = find_b(glmstack[i]);
    if((Mcol[i]=glmstack[i]->ifh->glm_Mcol)>Mcolmax) Mcolmax = Mcol[i];
    nc[i] = glmstack[i]->ifh->glm_nc;
    swapbytesin[i] = shouldiswap(SunOS_Linux,glmstack[i]->ifh->bigendian);
    if(!i) {
        nregions = glmstack[i]->ifh->nregions;
        }
    else if(glmstack[i]->ifh->nregions!=nregions) {
        printf("fidlError: %s ifh->nregions=%d Must equal %d\n",glmfiles->files[i],glmstack[i]->ifh->nregions,nregions); 
        exit(-1);
        } 
    }
/*STARTCHECK*/
#if 0
for(i=0;i<nglmfiles;i++) {
    printf("meancol_stack[%d]->ncol=%d\n",i,meancol_stack[i]->ncol);
    }
#endif
/*ENDCHECK*/

if(!lc_names_only) {
    if(k) {
        printf("fidlError: GLMs all need to be in the same space. Abort!\n");
        for(m=0;m<k;m++) {
            i = temp_int[m];
            printf("fidlError: %s xdim=%d ydim=%d zdim=%d\n",glmfiles->files[i],glmstack[i]->ifh->glm_xdim,
                glmstack[i]->ifh->glm_ydim,glmstack[i]->ifh->glm_zdim);
            }
        printf("fidlError: all the rest have dimensions xdim=%d ydim=%d zdim=%d\n",glmstack[0]->ifh->glm_xdim,
            glmstack[0]->ifh->glm_ydim,glmstack[0]->ifh->glm_zdim);
        exit(-1);
        }
    if(kk>nglmfiles) {
        printf("fidlError: GLMs all need to be smoothed the same. Abort!\n");
        for(m=nglmfiles;m<k;m++) {
            i = temp_int[m];
            printf("fidlError: %s fwhm=%f\n",glmfiles->files[i],glmstack[i]->ifh->glm_fwhm);
            }
        printf("fidlError: all the rest have fwhm=%f\n",glmstack[0]->ifh->glm_fwhm);
        exit(-1);
        }
    fwhm0 = sqrt(fwhm*fwhm + glmstack[0]->ifh->glm_fwhm*glmstack[0]->ifh->glm_fwhm);
    }
free(temp_int);
if(num_region_files) {
    if(!(ifh = read_ifh(region_files->files[0],(Interfile_header*)NULL))) exit(-1);
    lenvol_region = ifh->dim1*ifh->dim2*ifh->dim3;
    free_ifh(ifh,(int)FALSE);
    if(!check_dimensions(num_region_files,region_files->files,lenvol_region)) exit(-1);
    if(!(reg=malloc(sizeof*reg*num_region_files))) {
        printf("fidlError: Unable to malloc reg\n");
        exit(-1);
        }
    for(m=0;m<num_region_files;m++) {

        //if(!(reg[m]=extract_regions(region_files->files[m],0,0,(float*)NULL,0,SunOS_Linux,(char**)NULL))) exit(-1);
        //START180412
        if(!(reg[m]=get_reg(region_files->files[m],0,0,(float*)NULL,0,SunOS_Linux,(char**)NULL))) exit(-1);

        }
    if(!num_regions) for(m=0;m<num_region_files;m++) num_regions += reg[m]->nregions;
    if(!(rbf=find_regions_by_file_cover(num_region_files,num_regions,reg,roi))) exit(-1);
    /**** START CHECKS ****/
    /*for(m=0;m<num_region_files;m++) {
        for(i=0;i<rbf->num_regions_by_file[m];i++) {
            printf("file %d  region %d\n",m,rbf->roi_by_file[m][i]);
            }
        }*/
    /**** END CHECKS ****/
    }
else if(lc_anova&&nregions) {
    if(!(reg=malloc(sizeof*reg*nglmfiles))) {
        printf("fidlError: Unable to malloc reg\n");
        exit(-1);
        }
    for(m=0;m<nglmfiles;m++) {

        //if(!(reg[m]=extract_regions((char*)NULL,0,0,(float*)NULL,glmstack[m]->ifh->nregions,SunOS_Linux,
        //    glmstack[m]->ifh->region_names))) exit(-1);
        //START180412
        if(!(reg[m]=get_reg((char*)NULL,0,0,(float*)NULL,glmstack[m]->ifh->nregions,SunOS_Linux,glmstack[m]->ifh->region_names))) 
            exit(-1);

        }

    if(!(rbf=find_regions_by_file_cover(nglmfiles,num_regions,reg,roi))) exit(-1);
    }
if(lc_anova) anova_regions_loop = num_regions;
if(!nxformfiles) {
    get_atlas(vol,atlas);
    }
else {
    if(num_region_files) get_atlas(lenvol_region,atlas);
    if(!(t4=malloc(sizeof*t4*(size_t)T4SIZE*nglmfiles))) {
        printf("fidlError: Unable to malloc t4\n");
        exit(-1);
        }
    if(!(A_or_B_or_U=malloc(sizeof*A_or_B_or_U*nglmfiles))) {
        printf("fidlError: Unable to malloc A_or_B_or_U\n");
        exit(-1);
        }
    for(i=0;i<nglmfiles;i++) {
        if(!read_xform(xformfiles->files[i],&t4[i*(int)T4SIZE])) exit(-1);
        if((A_or_B_or_U[i]=twoA_or_twoB(xformfiles->files[i])) == 2) exit(-1);
        }
    }

//if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL))) exit(-1);
//START161213
//if(!(ap=get_atlas_param(atlas,!strstr(atlas,"MNI")?(Interfile_header*)NULL:glmstack[0]->ifh))) exit(-1);
//START180412
if(!(ap=get_atlas_param(atlas,!strstr(atlas,"MNI")?(Interfile_header*)NULL:glmstack[0]->ifh,(char*)NULL)))exit(-1);


if(scratchdir&&dir) {
    printf("fidlError: You have specified both -scratchdir and -dir. Use only one.\n");fflush(stdout);
    exit(-1);
    }
if(scratchdir||lc_names_only)scratch = 1;
if(!scratchdir&&lc_names_only)if(!(scratchdir=make_scratchdir(1,"SCRATCH")))exit(-1);
if(dir)scratchdir=dir;
if(!atlas[0]){
    ap->vol = vol;
    ap->xdim = glmstack[0]->ifh->glm_xdim;
    ap->ydim = glmstack[0]->ifh->glm_ydim;
    ap->zdim = glmstack[0]->ifh->glm_zdim;
    ap->voxel_size[0] = glmstack[0]->ifh->glm_dxdy;
    ap->voxel_size[1] = glmstack[0]->ifh->glm_dxdy;
    ap->voxel_size[2] = glmstack[0]->ifh->glm_dz;
    if(!nxformfiles){
        ap->mmppix[0] = glmstack[0]->ifh->mmppix[0];
        ap->mmppix[1] = glmstack[0]->ifh->mmppix[1];
        ap->mmppix[2] = glmstack[0]->ifh->mmppix[2];
        ap->center[0] = glmstack[0]->ifh->center[0];
        ap->center[1] = glmstack[0]->ifh->center[1];
        ap->center[2] = glmstack[0]->ifh->center[2];
        }
    }
else if(!scratch){
    strcpy(appendstr,ap->str);
    }
if(!(ms=get_mask_struct(mask_file,ap->vol,(int*)NULL,SunOS_Linux,(LinearModel*)NULL,ap->vol))) exit(-1);
chunkvol = (int)ceil((double)ms->lenbrain/(double)chunks);

//if(!(ifh_out=init_ifh(4,ap->xdim,ap->ydim,ap->zdim,1,ap->voxel_size[0],ap->voxel_size[1],ap->voxel_size[2],bigendian)))exit(-1);
//START161213
if(!(ifh_out=init_ifh(4,ap->xdim,ap->ydim,ap->zdim,1,ap->voxel_size[0],ap->voxel_size[1],ap->voxel_size[2],bigendian,ap->centerf,
    ap->mmppixf)))exit(-1);

ifh_out->fwhm=(float)fwhm0;


#if 0
if(scratch){
    ifh_out->mmppix[0]=0;
    ifh_out->mmppix[1]=0;
    ifh_out->mmppix[2]=0;
    ifh_out->center[0]=0;
    ifh_out->center[1]=0;
    ifh_out->center[2]=0;
    ifh_out->bigendian=!SunOS_Linux?1:0;
    }
else{
    ifh_out->dim1=ap->xdim;
    ifh_out->dim2=ap->ydim;
    ifh_out->dim3=ap->zdim;
    ifh_out->mmppix[0]=ap->mmppix[0];
    ifh_out->mmppix[1]=ap->mmppix[1];
    ifh_out->mmppix[2]=ap->mmppix[2];
    ifh_out->center[0]=ap->center[0];
    ifh_out->center[1]=ap->center[1];
    ifh_out->center[2]=ap->center[2];
    }
#endif
//START161213
if(scratch){
    ifh_out->mmppix[0]=0;
    ifh_out->mmppix[1]=0;
    ifh_out->mmppix[2]=0;
    ifh_out->center[0]=0;
    ifh_out->center[1]=0;
    ifh_out->center[2]=0;
    ifh_out->bigendian=!SunOS_Linux?1:0;
    }

if(delay_flag==3&&!mask_file){

    #if 0
    if(!(ifh_out1=init_ifh(4,glmstack[0]->ifh->glm_xdim,glmstack[0]->ifh->glm_ydim,glmstack[0]->ifh->glm_zdim,1,
        glmstack[0]->ifh->glm_dxdy,glmstack[0]->ifh->glm_dxdy,glmstack[0]->ifh->glm_dz,bigendian)))exit(-1);
    #endif
    //START161213
    if(!(ifh_out1=init_ifh(4,glmstack[0]->ifh->glm_xdim,glmstack[0]->ifh->glm_ydim,glmstack[0]->ifh->glm_zdim,1,
        glmstack[0]->ifh->glm_dxdy,glmstack[0]->ifh->glm_dxdy,glmstack[0]->ifh->glm_dz,bigendian,glmstack[0]->ifh->center,
        glmstack[0]->ifh->mmppix)))exit(-1);

    ifh_out1->mmppix[0]=glmstack[0]->ifh->mmppix[0];
    ifh_out1->mmppix[1]=glmstack[0]->ifh->mmppix[1];
    ifh_out1->mmppix[2]=glmstack[0]->ifh->mmppix[2];
    ifh_out1->center[0]=glmstack[0]->ifh->center[0];
    ifh_out1->center[1]=glmstack[0]->ifh->center[1];
    ifh_out1->center[2]=glmstack[0]->ifh->center[2];
    }
if(fwhm>0.&&!scratch)strcat(appendstr,string);
/*THIS STUFF NEEDS TO STAY HERE WITH ITS USE OF string AND string2*/
if(!scratchdir) {
    for(j=i=0;i<(int)glmfiles->nfiles;i++) j += glmfiles->strlen_files[i]-3;
    }
else {
    for(j=glmfiles->nfiles*strlen(scratchdir),i=0;i<(int)glmfiles->nfiles;i++) { 
        strcpy(string,glmfiles->files[i]);
        if(!(strptr=get_tail_sans_ext(string))) exit(-1);
        j+=strlen(strptr)+1;
        }
    }
if(!(input_file_ptr=malloc(sizeof*input_file_ptr*glmfiles->nfiles))) {
    printf("fidlError: Unable to malloc input_file_ptr\n");fflush(stdout);
    exit(-1);
    }
if(!(input_file=malloc(sizeof*input_file*j))) {
    printf("fidlError: Unable to malloc input_file\n");fflush(stdout);
    exit(-1);
    }
for(strptr=input_file,i=0;i<(int)glmfiles->nfiles;i++) {
    input_file_ptr[i] = strptr;
    strcpy(string,glmfiles->files[i]);
    if(scratchdir) {
        if(!(stringptr=get_tail_sans_ext(string))) exit(-1);
        sprintf(strptr,"%s%s",scratchdir,stringptr);
        }
    else {
        *(strrchr(string,'.')) = 0;
        strcpy(strptr,string);
        }
    strptr += strlen(strptr)+1;
    }
if(!(filel=malloc(sizeof*filel*glmfiles->nfiles))) {
    printf("fidlError: Unable to malloc filel\n");
    exit(-1);
    }
for(st1=strlen(unscaled_str)+strlen(appendstr)+9,i=0;i<(int)glmfiles->nfiles;i++) filel[i]=strlen(input_file_ptr[i])+st1;
if(!lc_names_only||lc_anova) {
    if(!(temp_float=malloc(sizeof*temp_float*ap->vol))) {
        printf("fidlError: Unable to malloc temp_float\n");
        exit(-1);
        }
    if(!(ms=get_mask_struct(mask_file,ap->vol,(int*)NULL,SunOS_Linux,(LinearModel*)NULL,ap->vol))) exit(-1);
    if(ms->vol != ap->vol) {
        printf("fidlError: %s and %s not in the same space.\n",glmfiles->files[0],mask_file);
        printf("fidlError: ap->vol=%d ms->vol=%d\n",ap->vol,ms->vol);
        printf("fidlError: Images must either be in atlas space or the -xform_file option must be used.\n");
        exit(-1);
        }
    if(!(c=malloc(sizeof*c*Mcolmax))) {
        printf("fidlError: Unable to malloc c\n");
        exit(-1);
        }
    if(!(temp_double=malloc(sizeof*temp_double*ap->vol))) {
        printf("fidlError: Unable to malloc temp_double\n");
        exit(-1);
        }
    if(!(df=malloc(sizeof*df*ap->vol))) {
        printf("fidlError: Unable to malloc df\n");
        exit(-1);
        }
    if(!(td=malloc(sizeof*td*ap->vol))) {
        printf("fidlError: Unable to malloc td\n");
        exit(-1);
        }
    if(!(mag=malloc(sizeof*mag*ap->vol))) {
        printf("fidlError: Unable to malloc mag\n");
        exit(-1);
        }
    if(!(stat=malloc(sizeof*stat*ap->vol))) {
        printf("fidlError: Unable to malloc stat\n");
        exit(-1);
        }
    if(fwhm>0.) if(!(gs=gauss_smoth2_init(ap->xdim,ap->ydim,ap->zdim,fwhm,fwhm))) exit(-1);

    //filetype=get_filetype2(glmstack[0]->ifh->glm_cifti_xmlsize,glmstack[0]->ifh->glm_boldtype);
    //START151005
    filetype=scratch?(int)IMG:get_filetype2(glmstack[0]->ifh->glm_cifti_xmlsize,glmstack[0]->ifh->glm_boldtype);

    #if 0
    if(!(w1=malloc(sizeof*w1))){
        printf("fidlError: Unable to malloc w1\n");
        exit(-1);
        }
    #endif
    //START161214
    if(!(w1=write1_init()))exit(-1);

    w1->cifti_xmldata=glmstack[0]->cifti_xmldata;
    w1->xmlsize=glmstack[0]->ifh->glm_cifti_xmlsize;
    w1->filetype=filetype;
    w1->swapbytes=swapbytesout;
    if(filetype==(int)NIFTI){
        dims[0]=glmstack[0]->ifh->glm_xdim;dims[1]=glmstack[0]->ifh->glm_ydim;dims[2]=glmstack[0]->ifh->glm_zdim;

        //START161214
        dims[3]=1.;

        w1->dims=dims;
        w1->center=glmstack[0]->ifh->center;
        w1->mmppix=glmstack[0]->ifh->mmppix;
        }
    w1->temp_double=NULL;
    w1->lenbrain=0;
    }
if(num_contrasts) {
    if(!(fbf=find_regions_by_file(nglmfiles,num_contrasts,nc,contrasti))) exit(-1);
    if(!(magmaxmin=malloc(sizeof*magmaxmin*vol))) {
        printf("fidlError: Unable to malloc magmaxmin\n");
        exit(-1);
        }
    if(!(which_delay=malloc(sizeof*which_delay*vol))) {
        printf("fidlError: Unable to malloc which_delay\n");
        exit(-1);
        }

    //START160921
    //for(i=0;i<vol;i++) which_delay[i] = 0;

    if(delay_flag < 3)
        num_delays = 1;
    else {
        if(!(dlymaxmin=malloc(sizeof*dlymaxmin*vol))) {
            printf("fidlError: Unable to malloc dlymaxmain\n");
            exit(-1);
            }
        }
    if(!(scl_stack=malloc(sizeof*scl_stack*num_delays))) {
        printf("fidlError: Unable to malloc scl_stack\n");
        exit(-1);
        }
    if(!(scl_mag_stack=malloc(sizeof*scl_mag_stack*num_delays))) {
        printf("fidlError: Unable to malloc scl_mag_stack\n");
        exit(-1);
        }
    //START161214
    for(i=0;i<num_delays;i++) scl_mag_stack[i] = 1;

    //for(i=0;i<num_delays;i++) scl_mag_stack[i] = 1;
    //START160919
    if(!(scl_stack2=malloc(sizeof*scl_stack2*num_delays))) {
        printf("fidlError: Unable to malloc scl_stack\n");
        exit(-1);
        }
    if(!(scl_mag_stack2=malloc(sizeof*scl_mag_stack2*num_delays))) {
        printf("fidlError: Unable to malloc scl_mag_stack2\n");
        exit(-1);
        }
    for(i=0;i<num_delays;i++)scl_mag_stack2[i]=1;



    if(!lc_names_only&&!lcmag_only) {
        if(!(t_to_zi=malloc(sizeof*t_to_zi*vol))) {
            printf("fidlError: Unable to malloc t_to_zi\n");
            exit(-1);
            }
        }
    if(lc_one_file) {
        period_one_file = 0;
        if(!one_file_root) {
            sprintf(string,"%s_mag",input_file_ptr[0]);
            for(k=0;k<num_contrasts;k++) {
                strcat(string,"_");
                strcat(string,glmstack[0]->contrast_labels[fbf->roi_index[k]]);
                }
            strcat(string,appendstr);

            /*strcat(string,".4dfp.img");*/
            /*START150915*/
            strcat(string,Fileext[filetype]);

            }
        else {
            strcpy(string,one_file_root);

            /*if(!strstr(string,".4dfp.img")) strcat(string,".4dfp.img");*/
            /*START150915*/
            if(!strstr(string,".4dfp.img"))strcat(string,Fileext[filetype]);

            }

        #if 0
        if(!(fpw=fopen_sub(string,"w"))) exit(-1);
        #endif
        /*START150915*/
        if(!(fpw=open2(string,w1)))exit(-1);

        }
    min_and_max_init(&mag_min,&mag_max);
    for(k=0;k<num_contrasts;k++) {
        glmi = fbf->file_index[k];
        if(!(fp=fopen_sub(glmfiles->files[glmi],"r"))) exit(-1);
        if(!lc_names_only) {
            c_index = fbf->roi_index[k]*glmstack[glmi]->ifh->glm_Mcol;
            if(delay_flag == 3) cnorm_index = fbf->roi_index[k]*glmstack[glmi]->ifh->glm_tot_eff;

            //for(i=0;i<vol;i++) stat[i] = magmaxmin[i] = 0.;
            //START160921
            //for(i=0;i<vol;i++){stat[i]=magmaxmin[i]=0.;dlymaxmin[i]=0.;which_delay[i]=0;}
            //START161213
            for(i=0;i<vol;i++){stat[i]=magmaxmin[i]=0.;which_delay[i]=0;}
            if(delay_flag==3)for(i=0;i<vol;i++)dlymaxmin[i]=0.;

            for(dt=0,delay=0;delay<num_delays;delay++,dt+=delay_inc) {
                dont_use_c = 0;
                if(delay_flag == 3) {
                    dont_use_c = boynton_contrast(glmstack[glmi]->ifh->glm_tot_eff,glmstack[glmi]->ifh->glm_effect_length,
                        glmstack[glmi]->ifh->glm_effect_TR,glmstack[glmi]->ifh->glm_effect_shift_TR,dt,init_delay,
                        glmstack[glmi]->stimlen,&glmstack[glmi]->cnorm[cnorm_index],glmstack[glmi]->ifh->glm_Mcol,c);
                    }
                if(!dont_use_c) {
                    for(i=0;i<glmstack[glmi]->ifh->glm_Mcol;i++) c[i] = glmstack[glmi]->c[c_index+i];
                    }
                if(fseek(fp,startb[glmi],SEEK_SET)) {
	            printf("fidlError: occured while seeking to %ld in %s.\n",startb[glmi],glmfiles->files[glmi]);
	            exit(-1);
	            }

                //for(i=0;i<vol;i++) mag[i] = 0.;
                //START160921
                for(i=0;i<vol;i++)mag[i]=temp_double[i]=0.;

                #if 0
                for(j=0;j<glmstack[glmi]->ifh->glm_Mcol;j++) {
                    if(c[j]) {
                        if(!fread_sub(temp_float,sizeof(float),(size_t)vol,fp,swapbytesin[glmi],glmfiles->files[glmi])) exit(-1);
                        for(i=0;i<vol;i++) {
                            if(temp_float[i]==(float)UNSAMPLED_VOXEL) mag[i] = (double)UNSAMPLED_VOXEL;
                            else if(mag[i]!=(double)UNSAMPLED_VOXEL) mag[i] += (double)c[j] * (double)temp_float[i];
                            }
                        }
                    else {
                        if(fseek(fp,vol*sizeof(float),SEEK_CUR)) {
                            fprintf(stderr,"fidlError: occured while seeking to %p in %s.\n",fp,glmfiles->files[glmi]);
                            exit(-1);
                            }
                        }
                    }
                #endif
                //START160921
                for(j=0;j<glmstack[glmi]->ifh->glm_Mcol;j++){
                    if(c[j]){
                        if(!fread_sub(temp_float,sizeof(float),(size_t)glmstack[glmi]->nmaski,fp,swapbytesin[glmi],
                            glmfiles->files[glmi]))exit(-1);

                        //START161214
                        //for(i=0;i<glmstack[glmi]->nmaski;i++)printf("%f ",temp_float[i]);fflush(stdout);

                        for(i=0;i<glmstack[glmi]->nmaski;i++){
                            if(temp_float[i]==(float)UNSAMPLED_VOXEL)mag[glmstack[glmi]->maski[i]]=(double)UNSAMPLED_VOXEL;
                            else if(mag[glmstack[glmi]->maski[i]]!=(double)UNSAMPLED_VOXEL)
                                mag[glmstack[glmi]->maski[i]]+=(double)c[j]*(double)temp_float[i];
                            }
                        }
                    else{
                        if(fseek(fp,glmstack[glmi]->nmaski*sizeof(float),SEEK_CUR)){
                            printf("fidlError: occured while seeking to %p in %s.\n",fp,glmfiles->files[glmi]);
                            exit(-1);
                            }
                        }
                    }

                //START161214
                //printf("glmstack[%d]->nmaski=%d\n",glmi,glmstack[glmi]->nmaski);fflush(stdout);
                //for(i=0;i<glmstack[glmi]->nmaski;i++)printf("%f ",mag[glmstack[glmi]->maski[i]]);printf("\n");fflush(stdout);


                #if 0
                for(scl_stack[delay]=0.,i=0;i<glmstack[glmi]->ifh->glm_Mcol;i++) {
                    if(c[i]) {
                        for(j=0;j<glmstack[glmi]->ifh->glm_Mcol;j++) 
                            scl_stack[delay]+= (double)c[i]*(double)glmstack[glmi]->ATAm1[i][j]*(double)c[j];
                        }
                    }
                if((scl_stack[delay]=sqrt(scl_stack[delay])) <= 0.) {
                    fprintf(stderr,"fidlError: scl_stack[%d] = %f < 0  trouble: program aborted\n",delay,scl_stack[delay]);
                    exit(-1);
                    }
                if(magnorm_str[0]) {
                    for(scl_mag_stack[delay]=i=0;i<glmstack[glmi]->ifh->glm_Mcol;i++) scl_mag_stack[delay]+=(double)c[i]*(double)c[i];
                    if((scl_mag_stack[delay]=sqrt(scl_mag_stack[delay])) <= 0.) {
                        fprintf(stderr,"fidlError: scl_mag_stack[%d] = %f < 0  trouble: program aborted\n",delay,scl_mag_stack[delay]);
                        exit(-1);
                        }
                    }
                #endif
                //START160919
                for(scl_stack2[delay]=0.,i=0;i<glmstack[glmi]->ifh->glm_Mcol;i++){
                    if(c[i]){
                        for(j=0;j<glmstack[glmi]->ifh->glm_Mcol;j++) 
                            scl_stack2[delay]+=(double)c[i]*glmstack[glmi]->ATAm1[i][j]*(double)c[j];
                        }
                    }
                if(scl_stack2[delay]<=0.){
                    printf("fidlError: scl_stack2[%d]=%f < 0 trouble: program aborted\n",delay,scl_stack2[delay]);
                    exit(-1);
                    }
                scl_stack[delay]=sqrt(scl_stack2[delay]);
                if(magnorm_str[0]){
                    for(scl_mag_stack2[delay]=0.,i=0;i<glmstack[glmi]->ifh->glm_Mcol;i++)
                        scl_mag_stack2[delay]+=(double)c[i]*(double)c[i];
                    if(scl_mag_stack2[delay]<=0.){
                        printf("fidlError: scl_mag_stack2[%d]=%f < 0 trouble: program aborted\n",delay,scl_mag_stack2[delay]);
                        exit(-1);
                        }
                    scl_mag_stack[delay]=sqrt(scl_mag_stack2[delay]);

                    //START161214
                    printf("scl_mag_stack[%d]=%f\n",delay,scl_mag_stack[delay]);fflush(stdout);

                    }
                for(i=0;i<glmstack[glmi]->nmaski;i++)temp_double[glmstack[glmi]->maski[i]]=
                    mag[glmstack[glmi]->maski[i]]==(double)UNSAMPLED_VOXEL||glmstack[glmi]->var[i]<=0.?
                    (double)UNSAMPLED_VOXEL:mag[glmstack[glmi]->maski[i]]/sqrt(glmstack[glmi]->var[i]*scl_stack2[delay]);

                //START161214
                //for(i=0;i<glmstack[glmi]->nmaski;i++)printf("%f ",temp_double[glmstack[glmi]->maski[i]]);printf("\n");fflush(stdout);


                if(delay){
                    if(!lc_largest_mag){
                        for(i=0;i<glmstack[glmi]->nmaski;i++){
                            if(fabs(temp_double[glmstack[glmi]->maski[i]])>fabs(stat[glmstack[glmi]->maski[i]])){
                                stat[glmstack[glmi]->maski[i]]=temp_double[glmstack[glmi]->maski[i]];
                                magmaxmin[glmstack[glmi]->maski[i]]=mag[glmstack[glmi]->maski[i]];
                                dlymaxmin[glmstack[glmi]->maski[i]]=(float)HRF_DELTA+init_delay+dt;
                                which_delay[glmstack[glmi]->maski[i]]=delay;
                                }
                            }
                        }
                    else{
                        for(i=0;i<glmstack[glmi]->nmaski;i++){
                            if(fabs(mag[glmstack[glmi]->maski[i]])>fabs(magmaxmin[glmstack[glmi]->maski[i]])){
                                stat[glmstack[glmi]->maski[i]]=temp_double[glmstack[glmi]->maski[i]];
                                magmaxmin[glmstack[glmi]->maski[i]]=mag[glmstack[glmi]->maski[i]];
                                dlymaxmin[glmstack[glmi]->maski[i]]=(float)HRF_DELTA+init_delay+dt;
                                which_delay[glmstack[glmi]->maski[i]]=delay;
                                }
                            }
                        }
                    }
                else{
                    for(i=0;i<glmstack[glmi]->nmaski;i++){
                        stat[glmstack[glmi]->maski[i]]=temp_double[glmstack[glmi]->maski[i]];
                        magmaxmin[glmstack[glmi]->maski[i]]=mag[glmstack[glmi]->maski[i]];
                        }
                    if(delay_flag==3)
                        for(i=0;i<glmstack[glmi]->nmaski;i++)dlymaxmin[glmstack[glmi]->maski[i]]=(float)HRF_DELTA+init_delay+dt;
                    }

                //START161214
                //for(i=0;i<glmstack[glmi]->nmaski;i++)printf("%f ",stat[glmstack[glmi]->maski[i]]);printf("\n");fflush(stdout);
                //for(i=0;i<glmstack[glmi]->nmaski;i++)printf("%f ",magmaxmin[glmstack[glmi]->maski[i]]);printf("\n");fflush(stdout);

	        }
            if(!lcmag_only) {
                if(lctstat) {
                    how_many = vol;
                    dptr = stat;
                    if(nxformfiles) {
                        if(!t4_atlas(stat,temp_double,&t4[glmi*(int)T4SIZE],glmstack[glmi]->ifh->glm_xdim,
                            glmstack[glmi]->ifh->glm_ydim,glmstack[glmi]->ifh->glm_zdim,glmstack[glmi]->ifh->glm_dxdy,
                            glmstack[glmi]->ifh->glm_dz,A_or_B_or_U[glmi],glmstack[glmi]->ifh->orientation,ap,(double*)NULL))exit(-1);
                        how_many = ap->vol;
                        dptr = temp_double;
                        }
                    for(i=0;i<how_many;i++)temp_float[i] = (float)dptr[i];
                    sprintf(string,"%s_%s_tstat%s%s%s",input_file_ptr[glmi],glmstack[glmi]->contrast_labels[fbf->roi_index[k]],
                        delay_str,appendstr,Fileext[filetype]);
                    w1->temp_float=temp_float;
                    w1->how_many=how_many;
                    w1->ifh=ifh_out;
                    if(!write1(string,w1))exit(-1);
                    printf("T statistics written to %s\n",string);
                    }

                //for(i=0;i<vol;i++) df[i]=(double)glmstack[glmi]->ifh->glm_df;
                //START160926
                for(i=0;i<vol;i++)df[i]=0.;
                for(i=0;i<glmstack[glmi]->nmaski;i++)df[glmstack[glmi]->maski[i]]=(double)glmstack[glmi]->ifh->glm_df;

                t_to_z(stat,temp_double,vol,df,t_to_zi);
                how_many = vol;
                dptr = temp_double;
                if(nxformfiles) {
                    if(!t4_atlas(temp_double,stat,&t4[glmi*(int)T4SIZE],glmstack[glmi]->ifh->glm_xdim,glmstack[glmi]->ifh->glm_ydim,
                        glmstack[glmi]->ifh->glm_zdim,glmstack[glmi]->ifh->glm_dxdy,glmstack[glmi]->ifh->glm_dz,A_or_B_or_U[glmi],
                        glmstack[glmi]->ifh->orientation,ap,(double*)NULL)) exit(-1);
                    dptr = stat;
                    how_many = ap->vol;
                    }

                #if 0
                if(mask_file) {
                    for(i=0;i<ms->lenbrain;i++) temp_float[i] = (float)dptr[ms->brnidx[i]];
                    how_many = ms->lenbrain;
                    }
                else {
                    for(i=0;i<how_many;i++) temp_float[i] = (float)dptr[i];
                    }
                #endif
                //START160926
                for(i=0;i<ms->lenbrain;i++)temp_float[i]=(float)dptr[ms->brnidx[i]];


                sprintf(string,"%s_%s_zstat%s%s%s",input_file_ptr[glmi],glmstack[glmi]->contrast_labels[fbf->roi_index[k]],
                    delay_str,appendstr,Fileext[filetype]);
                    /*Magnitude normalization does not change the zstat, but included to be consistent with compute_avg_zstat.*/
                w1->temp_float=temp_float;

                //w1->how_many=how_many;
                //START160926
                w1->how_many=ms->lenbrain;

                w1->ifh=ifh_out;
                if(!write1(string,w1))exit(-1);
                printf("Z statistics written to %s\n",string);
	        }
            if(delay_flag==3&&!mask_file){

                //START160926
                //for(i=0;i<vol;i++) temp_float[i] = dlymaxmin[i];

                sprintf(string,"%s_%s_delaysec%s%s%s",input_file_ptr[glmi],glmstack[glmi]->contrast_labels[fbf->roi_index[k]],
                    delay_str,appendstr,Fileext[filetype]);

                #if 0
                w1->temp_float=temp_float;
                w1->how_many=vol;
                #endif
                //START160926
                w1->temp_float=dlymaxmin;
                w1->how_many=vol;

                w1->ifh=ifh_out1;
                if(!write1(string,w1))exit(-1);
                printf("Delays written to %s\n",string);
                }
	    }
        if(lcmag) {
            if(!lc_names_only) {

                #if 0
                for(i=0;i<vol;i++) temp_double[i] = magmaxmin[i]==(double)UNSAMPLED_VOXEL ? (double)UNSAMPLED_VOXEL :
                    magmaxmin[i]/scl_mag_stack[which_delay[i]];
                if(!unscaled_str[0]) {
                    for(i=0;i<vol;i++) {
                        if(temp_double[i]!=(double)UNSAMPLED_VOXEL) {
                            temp_double[i] *= gms_stack[glmi]->grand_mean[i] < gms_stack[glmi]->grand_mean_thresh ? 0. :
                                gms_stack[glmi]->pct_chng_scl/gms_stack[glmi]->grand_mean[i];
                            }
                        }
                    }
                #endif
                //START160921
                for(i=0;i<vol;i++)temp_double[i]=0.;
                //for(i=0;i<glmstack[glmi]->nmaski;i++)temp_double[glmstack[glmi]->maski[i]]= 
                //    magmaxmin[glmstack[glmi]->maski[i]]==(double)UNSAMPLED_VOXEL ? (double)UNSAMPLED_VOXEL :
                //    magmaxmin[glmstack[glmi]->maski[i]]/scl_mag_stack[which_delay[i]];
                //START161214
                for(i=0;i<glmstack[glmi]->nmaski;i++)temp_double[glmstack[glmi]->maski[i]]= 
                    magmaxmin[glmstack[glmi]->maski[i]]==(double)UNSAMPLED_VOXEL ? (double)UNSAMPLED_VOXEL :
                    magmaxmin[glmstack[glmi]->maski[i]]/scl_mag_stack[which_delay[glmstack[glmi]->maski[i]]];

                //START161214
                //for(i=0;i<glmstack[glmi]->nmaski;i++)printf("%f ",temp_double[glmstack[glmi]->maski[i]]);printf("\n");fflush(stdout);
                //for(i=0;i<glmstack[glmi]->nmaski;i++)printf("%f ",magmaxmin[glmstack[glmi]->maski[i]]);printf("\n");fflush(stdout);
                //for(i=0;i<glmstack[glmi]->nmaski;i++)printf("%f ",scl_mag_stack[which_delay[i]]);printf("\n");fflush(stdout);
                //for(i=0;i<glmstack[glmi]->nmaski;i++)printf("%f ",scl_mag_stack[which_delay[glmstack[glmi]->maski[i]]]);printf("\n");fflush(stdout);

                if(!unscaled_str[0]){
                    for(i=0;i<glmstack[glmi]->nmaski;i++)if(temp_double[glmstack[glmi]->maski[i]]!=(double)UNSAMPLED_VOXEL)
                        temp_double[glmstack[glmi]->maski[i]]*=gms_stack[glmi]->grand_mean[i]<gms_stack[glmi]->grand_mean_thresh? 
                        0.:gms_stack[glmi]->pct_chng_scl/gms_stack[glmi]->grand_mean[i];
                    }

                //for(i=0;i<glmstack[glmi]->nmaski;i++)printf("%f ",temp_double[glmstack[glmi]->maski[i]]);printf("\n");fflush(stdout);

                #if 0
                how_many = vol;
                if(nxformfiles) {
                    if(!t4_atlas(temp_double,stat,&t4[glmi*(int)T4SIZE],glmstack[glmi]->ifh->glm_xdim,glmstack[glmi]->ifh->glm_ydim,
                        glmstack[glmi]->ifh->glm_zdim,glmstack[glmi]->ifh->glm_dxdy,glmstack[glmi]->ifh->glm_dz,A_or_B_or_U[glmi],
                        glmstack[glmi]->ifh->orientation,ap,(double*)NULL)) exit(-1);
                    for(i=0;i<ap->vol;i++) temp_double[i] = stat[i];
                    how_many = ap->vol;
                    }
                if(fwhm>0.) {
                    gauss_smoth2(temp_double,stat,gs);
                    for(i=0;i<how_many;i++) temp_double[i] = stat[i];
                    }
                if(mask_file) {
                    for(i=0;i<ms->lenbrain;i++) temp_float[i] = (float)temp_double[ms->brnidx[i]];
                    how_many = ms->lenbrain;
                    }
                else if(num_region_files) {
                    if(!(crs(temp_double,stat,rbf,lc_anova?glmfiles->files[glmi]:(char*)NULL))) exit(-1);
                    for(i=0;i<num_regions;i++) temp_float[i] = (float)stat[i];
                    how_many = lc_anova ? 1 : num_regions;
                    }
                else
                    for(i=0;i<how_many;i++) temp_float[i] = (float)temp_double[i];
                #endif
                //START160921
                dptr=temp_double;
                how_many=vol;
                if(nxformfiles) {
                    if(!t4_atlas(temp_double,stat,&t4[glmi*(int)T4SIZE],glmstack[glmi]->ifh->glm_xdim,glmstack[glmi]->ifh->glm_ydim,
                        glmstack[glmi]->ifh->glm_zdim,glmstack[glmi]->ifh->glm_dxdy,glmstack[glmi]->ifh->glm_dz,A_or_B_or_U[glmi],
                        glmstack[glmi]->ifh->orientation,ap,(double*)NULL)) exit(-1);
                    dptr=stat;
                    how_many=ap->vol;
                    }
                if(fwhm>0.){
                    gauss_smoth2(dptr,td,gs);
                    dptr=td; 
                    }
                if(mask_file){
                    for(i=0;i<ms->lenbrain;i++)temp_float[i]=(float)dptr[ms->brnidx[i]];
                    how_many=ms->lenbrain;
                    }
                else if(num_region_files){
                    if(!(crs(dptr,stat,rbf,lc_anova?glmfiles->files[glmi]:(char*)NULL))) exit(-1);
                    for(i=0;i<num_regions;i++)temp_float[i]=(float)stat[i];
                    how_many=lc_anova?1:num_regions;
                    }
                else
                    for(i=0;i<how_many;i++)temp_float[i]=(float)dptr[i];

                }
            else if(lc_anova&&num_region_files) {
                how_many = 1;
                }


            //for(i=0;i<ms->lenbrain;i++)printf("%f ",temp_float[i]);printf("\n");fflush(stdout);


            m = chunks > 1 ? chunks : anova_regions_loop;
            n = chunks > 1 ? chunkvol : how_many;
            p = chunks > 1 ? chunkvol : 1;
            for(i=0;i<m;i++) {
                if(!lc_names_only) min_and_max_floatstack(&temp_float[i],how_many,&mag_min,&mag_max);
                if(!lc_one_file) {
                    sprintf(string,"%s_%s_mag%s%s%s%s",input_file_ptr[glmi],glmstack[glmi]->contrast_labels[fbf->roi_index[k]],
                        magnorm_str,unscaled_str,delay_str,appendstr);
                    if(anova_regions_loop != 1) {
                        strcat(string,"_");
                        strcat(string,rbf->region_names_ptr[i]);
                        }
                    if(chunks > 1) {
                        sprintf(string2,"_CHUNK%d",i+1);
                        strcat(string,string2);
                        }
                    strcat(string,scratch?".4dfp.img":Fileext[filetype]);
                    if(!lc_names_only) {
                        w1->temp_float=&temp_float[i*p];
                        w1->how_many=n;

                        //if(scratch&&(mask_file||num_region_files)){ifh_out->dim1=n;ifh_out->dim2=1;ifh_out->dim3=1;}
                        //START161213
                        //if(scratch&&chunks>1){ifh_out->dim1=n;ifh_out->dim2=1;ifh_out->dim3=1;}
                        //START161220
                        if(scratch){ifh_out->dim1=n;ifh_out->dim2=1;ifh_out->dim3=1;}

                        w1->ifh=ifh_out;
                        if(!write1(string,w1))exit(-1);
                        }
                    if(!lcquiet) printf("Magnitudes written to %s\n",string);
                    }
                else {
                    if(!fwrite_sub(temp_float,sizeof(float),(size_t)how_many,fpw,swapbytesout)) exit(-1);
                    }
                }
            }
        if(lcsd) {


            #if 0
            for(i=0;i<vol;i++) temp_double[i]=(double)glmstack[glmi]->sd[i]*scl_stack[which_delay[i]]/scl_mag_stack[which_delay[i]];
            if(!unscaled_str[0]) {
                for(i=0;i<vol;i++) {
                    if(temp_double[i]!=(double)UNSAMPLED_VOXEL) {
                        temp_double[i] *= gms_stack[glmi]->grand_mean[i] < gms_stack[glmi]->grand_mean_thresh ? 0 : 
                            gms_stack[glmi]->pct_chng_scl/gms_stack[glmi]->grand_mean[i];
                        }
                    }
                }
            how_many = vol;
            if(nxformfiles) {
                if(!t4_atlas(temp_double,stat,&t4[glmi*(int)T4SIZE],glmstack[glmi]->ifh->glm_xdim,glmstack[glmi]->ifh->glm_ydim,
                    glmstack[glmi]->ifh->glm_zdim,glmstack[glmi]->ifh->glm_dxdy,glmstack[glmi]->ifh->glm_dz,A_or_B_or_U[glmi],
                    glmstack[glmi]->ifh->orientation,ap,(double*)NULL)) exit(-1);
                for(i=0;i<ap->vol;i++) temp_double[i] = stat[i];
                how_many = ap->vol;
                }
            if(fwhm>0.) {
                gauss_smoth2(temp_double,stat,gs);
                for(i=0;i<how_many;i++) temp_double[i] = stat[i];
                }
            for(i=0;i<how_many;i++) temp_float[i] = (float)temp_double[i];
            #endif
            #if 0
            //START160919
            for(i=0;i<vol;i++)temp_double[i]=!glmstack[glmi]->var[i]?(double)UNSAMPLED_VOXEL:
                sqrt(glmstack[glmi]->var[i])*scl_stack[which_delay[i]]/scl_mag_stack[which_delay[i]];
            if(!unscaled_str[0]){
                for(i=0;i<vol;i++){
                    if(temp_double[i]!=(double)UNSAMPLED_VOXEL){
                        temp_double[i] *= gms_stack[glmi]->grand_mean[i] < gms_stack[glmi]->grand_mean_thresh ? 0 :
                            gms_stack[glmi]->pct_chng_scl/gms_stack[glmi]->grand_mean[i];
                        }
                    }
                }
            dptr=temp_double;
            how_many=vol;
            if(nxformfiles) {
                if(!t4_atlas(temp_double,stat,&t4[glmi*(int)T4SIZE],glmstack[glmi]->ifh->glm_xdim,glmstack[glmi]->ifh->glm_ydim,
                    glmstack[glmi]->ifh->glm_zdim,glmstack[glmi]->ifh->glm_dxdy,glmstack[glmi]->ifh->glm_dz,A_or_B_or_U[glmi],
                    glmstack[glmi]->ifh->orientation,ap,(double*)NULL)) exit(-1);
                dptr=stat;
                how_many = ap->vol;
                }
            if(fwhm>0.) {
                gauss_smoth2(dptr,td,gs);
                dptr=td;
                }
            for(i=0;i<how_many;i++)temp_float[i]=(float)dptr[i];
            #endif
            //START160921
            #if 0
            for(i=0;i<glmstack[glmi]->nmaski;i++)
                temp_double[glmstack[glmi]->maski[i]]=!glmstack[glmi]->var[i]?(double)UNSAMPLED_VOXEL:
                    sqrt(glmstack[glmi]->var[i])*scl_stack[which_delay[i]]/scl_mag_stack[which_delay[i]];
            #endif
            //START161214
            for(i=0;i<glmstack[glmi]->nmaski;i++)
                temp_double[glmstack[glmi]->maski[i]]=!glmstack[glmi]->var[i]?(double)UNSAMPLED_VOXEL:
                    sqrt(glmstack[glmi]->var[i])*scl_stack[which_delay[glmstack[glmi]->maski[i]]]/
                    scl_mag_stack[which_delay[glmstack[glmi]->maski[i]]];

            if(!unscaled_str[0]){
                for(i=0;i<glmstack[glmi]->nmaski;i++)if(temp_double[glmstack[glmi]->maski[i]]!=(double)UNSAMPLED_VOXEL)
                    temp_double[glmstack[glmi]->maski[i]]*=gms_stack[glmi]->grand_mean[i]<gms_stack[glmi]->grand_mean_thresh?0:
                    gms_stack[glmi]->pct_chng_scl/gms_stack[glmi]->grand_mean[i];
                }
            dptr=temp_double;
            if(nxformfiles){
                if(!t4_atlas(temp_double,stat,&t4[glmi*(int)T4SIZE],glmstack[glmi]->ifh->glm_xdim,glmstack[glmi]->ifh->glm_ydim,
                    glmstack[glmi]->ifh->glm_zdim,glmstack[glmi]->ifh->glm_dxdy,glmstack[glmi]->ifh->glm_dz,A_or_B_or_U[glmi],
                    glmstack[glmi]->ifh->orientation,ap,(double*)NULL)) exit(-1);
                dptr=stat;
                }
            if(fwhm>0.){
                gauss_smoth2(dptr,td,gs);
                dptr=td;
                }
            for(i=0;i<ap->vol;i++)temp_float[i]=(float)dptr[i];


            w1->temp_float=temp_float;
            sprintf(string,"%s_%s_sd%s%s%s%s%s",input_file_ptr[glmi],glmstack[glmi]->contrast_labels[fbf->roi_index[k]],
                magnorm_str,unscaled_str,delay_str,appendstr,Fileext[filetype]);

            //w1->how_many=how_many;
            //START160921
            w1->how_many=ap->vol;

            w1->ifh=ifh_out;
            if(!write1(string,w1))exit(-1);
            printf("Standard deviation written to %s\n",string);
            }
        if(lcvar) {
            if(!lc_names_only) {


                #if 0
                //KEEP THIS
                for(i=0;i<vol;i++) temp_double[i] = glmstack[glmi]->sd[i]==(float)UNSAMPLED_VOXEL ? (double)UNSAMPLED_VOXEL :
                    (double)glmstack[glmi]->sd[i]*scl_stack[which_delay[i]]/scl_mag_stack[which_delay[i]];
                if(!unscaled_str[0]) {
                    for(i=0;i<vol;i++) {
                        if(temp_double[i]!=(double)UNSAMPLED_VOXEL) {
                            temp_double[i] *= gms_stack[glmi]->grand_mean[i] < gms_stack[glmi]->grand_mean_thresh ? 0 :
                                gms_stack[glmi]->pct_chng_scl/gms_stack[glmi]->grand_mean[i];
                            }
                        }
                    }
                for(i=0;i<vol;i++) if(temp_double[i]!=(double)UNSAMPLED_VOXEL) temp_double[i]*=temp_double[i];
                how_many = vol;
                if(nxformfiles) {
                    if(!t4_atlas(temp_double,stat,&t4[glmi*(int)T4SIZE],glmstack[glmi]->ifh->glm_xdim,glmstack[glmi]->ifh->glm_ydim,
                        glmstack[glmi]->ifh->glm_zdim,glmstack[glmi]->ifh->glm_dxdy,glmstack[glmi]->ifh->glm_dz,A_or_B_or_U[glmi],
                        glmstack[glmi]->ifh->orientation,ap,(double*)NULL)) exit(-1);
                    for(i=0;i<ap->vol;i++) temp_double[i] = stat[i];
                    how_many = ap->vol;
                    }
                if(mask_file) {
                    for(i=0;i<ms->lenbrain;i++) temp_float[i] = (float)temp_double[ms->brnidx[i]];
                    how_many = ms->lenbrain;
                    }
                else {
                    for(i=0;i<how_many;i++) temp_float[i] = (float)temp_double[i];
                    }
                #endif
                //START160921
                #if 0
                for(i=0;i<glmstack[glmi]->nmaski;i++)temp_double[glmstack[glmi]->maski[i]]=glmstack[glmi]->var[i]<=0.?
                    (double)UNSAMPLED_VOXEL:glmstack[glmi]->var[i]*scl_stack2[which_delay[i]]/scl_mag_stack2[which_delay[i]];
                #endif
                //START161214
                for(i=0;i<glmstack[glmi]->nmaski;i++)temp_double[glmstack[glmi]->maski[i]]=glmstack[glmi]->var[i]<=0.?
                    (double)UNSAMPLED_VOXEL:glmstack[glmi]->var[i]*scl_stack2[which_delay[glmstack[glmi]->maski[i]]]/
                    scl_mag_stack2[which_delay[glmstack[glmi]->maski[i]]];

                if(!unscaled_str[0]){
                    for(i=0;i<glmstack[glmi]->nmaski;i++)if(temp_double[glmstack[glmi]->maski[i]]!=(double)UNSAMPLED_VOXEL)
                        temp_double[glmstack[glmi]->maski[i]]*=gms_stack[glmi]->grand_mean[i]<gms_stack[glmi]->grand_mean_thresh?0:
                        gms_stack[glmi]->pct_chng_scl/gms_stack[glmi]->grand_mean[i];
                    }
                dptr=temp_double;
                how_many=vol;
                if(nxformfiles){
                    if(!t4_atlas(temp_double,stat,&t4[glmi*(int)T4SIZE],glmstack[glmi]->ifh->glm_xdim,glmstack[glmi]->ifh->glm_ydim,
                        glmstack[glmi]->ifh->glm_zdim,glmstack[glmi]->ifh->glm_dxdy,glmstack[glmi]->ifh->glm_dz,A_or_B_or_U[glmi],
                        glmstack[glmi]->ifh->orientation,ap,(double*)NULL)) exit(-1);
                    dptr=stat;
                    how_many = ap->vol;
                    }
                if(mask_file){
                    for(i=0;i<ms->lenbrain;i++)temp_float[i]=(float)dptr[ms->brnidx[i]];
                    how_many=ms->lenbrain;
                    }
                else{
                    for(i=0;i<how_many;i++)temp_float[i]=(float)dptr[i];
                    }


                }
            else if(lc_anova&&num_region_files) {
                how_many = 1;
                }
            m = chunks > 1 ? chunks : anova_regions_loop;
            n = chunks > 1 ? chunkvol : how_many;
            p = chunks > 1 ? chunkvol : 1;
            for(i=0;i<m;i++) {
                if(!lc_names_only) min_and_max_floatstack(&temp_float[i],how_many,&mag_min,&mag_max);
                if(!lc_one_file) {
                    sprintf(string,"%s_%s_var%s%s%s%s",input_file_ptr[glmi],glmstack[glmi]->contrast_labels[fbf->roi_index[k]],
                        magnorm_str,unscaled_str,delay_str,appendstr);
                    if(anova_regions_loop != 1) {
                        strcat(string,"_");
                        strcat(string,rbf->region_names_ptr[i]);
                        }
                    if(chunks > 1) {
                        sprintf(string2,"_CHUNK%d",i+1);
                        strcat(string,string2);
                        }
                    strcat(string,scratch?".4dfp.img":Fileext[filetype]);
                    if(!lc_names_only) {
                        w1->temp_float=&temp_float[i*p];
                        w1->how_many=n;
                        w1->ifh=ifh_out;

                        //if(scratch&&(mask_file||num_region_files)){ifh_out->dim1=n;ifh_out->dim2=1;ifh_out->dim3=1;}
                        //START161213
                        //if(scratch&&chunks>1){ifh_out->dim1=n;ifh_out->dim2=1;ifh_out->dim3=1;}
                        //START161220
                        if(scratch){ifh_out->dim1=n;ifh_out->dim2=1;ifh_out->dim3=1;}

                        if(!write1(string,w1))exit(-1);
                        }
                    if(!lcquiet) printf("Variance written to %s\n",string);
                    }
                else {
                    printf("fidlError: 140326 This path has not been instaniated.\n");fflush(stdout);exit(-1); 
                    }
                }
            }
        fclose(fp);
        }
    if(lc_one_file) {
        fclose(fpw);
        if(filetype==(int)IMG){

            //if(scratch&&(mask_file||num_region_files)){ifh_out->dim1=how_many;ifh_out->dim2=1;ifh_out->dim3=1;}
            //START161213
            //if(scratch&&chunks>1){ifh_out->dim1=how_many;ifh_out->dim2=1;ifh_out->dim3=1;}
            //START161220
            if(scratch){ifh_out->dim1=how_many;ifh_out->dim2=1;ifh_out->dim3=1;}

            ifh_out->global_min = mag_min;
            ifh_out->global_max = mag_max;
            ifh_out->dim4 = num_contrasts;
            if(!write_ifh(string,ifh_out,(int)FALSE)) exit(-1);
            }
        printf("Magnitudes written to %s\n",string);
        }
    }
if(num_tc) {
    if(!(tcs=read_tc_string_new(num_tc,(int*)NULL,argc_tc,argv))) exit(-1);
    #if 0
    printf("tcs->num_tc=%d\n",tcs->num_tc);
    printf("tcs->num_tc_to_sum="); for(i=0;i<tcs->num_tc;i++) printf("%d ",tcs->num_tc_to_sum[i][0]); printf("\n");
    printf("tcs->tc=\n");
    for(i=0;i<tcs->num_tc;i++) {
        for(j=0;j<tcs->num_tc_to_sum[i][0];j++) printf("%d ",(int)tcs->tc[i][0][j]);
        printf("\n");
        }
    #endif
    if(!(fbf=find_regions_by_file(nglmfiles,tcs->total,Mcol,tcs->tcstacki))) exit(-1);
    if(num_tc_weights) {
        if(!(tc_weights=read_tc_string_new(num_tc_weights,(int*)NULL,argc_tc_weights,argv))) exit(-1);
        #if 0
        printf("tc_weights->num_tc=%d\n",tc_weights->num_tc);
        printf("tc_weights->num_tc_to_sum=");
        for(i=0;i<tc_weights->num_tc;i++) printf("%d ",tc_weights->num_tc_to_sum[i][0]); printf("\n");
        printf("tc_weights->tc=\n");
        for(i=0;i<tc_weights->num_tc;i++) {
            for(k=0;k<tc_weights->each[i];k++) {
                for(j=0;j<tc_weights->num_tc_to_sum[i][0];j++) printf("%f ",tc_weights->tc[i][k][j]);
                }
            printf("\n");
            }
        #endif
        if(tc_weights->num_tc!=tcs->num_tc) {
            printf("fidlError: tc_weights->num_tc=%d tcs->num_tc=%d  They must be equal. Abort!\n",tc_weights->num_tc,tcs->num_tc);
            exit(-1);
            }
        for(i=0;i<tc_weights->num_tc;i++) {
            if(tcs->num_tc_to_sum[i][0]!=tc_weights->each[i]) {
                printf("tcs->num_tc_to_sum[%d][0]=%d tc_weights->each[%d]=%d  They must be equal. Abort!\n",i,
                    tcs->num_tc_to_sum[i][0],i,tc_weights->each[i]);
                exit(-1);
                }
            }
        }
    if(!(tags_tcs=get_tags_tc_glmstack(tcs,nglmfiles,glmstack,scratch?(TC*)NULL:tc_weights,fbf))) exit(-1);
    if(!(hipass=malloc(sizeof*hipass*tcs->num_tc))) {
        printf("fidlError: Unable to malloc hipass\n");
        exit(-1);
        }
    for(i=0;i<tcs->num_tc;i++) hipass[i] = strstr(tags_tcs->tags_tcptr[i],"Hz") ? 1 : 0;
    if(lc_one_file) {
        period_one_file = nfpw = 0;
        sprintf(strfpw,"%s%s",!one_file_root?input_file_ptr[0]:one_file_root,Fileext[filetype]);
        if(!(fpw=open2(strfpw,w1)))exit(-1);
        }
    if(!lc_names_only&&lcindbase) {
        if(!(indbase_col=malloc(sizeof*indbase_col*tcs->total))) {
            printf("fidlError: Unable to malloc indbase_col\n");
            exit(-1);
            }
        if(!(get_indbase(tcs,fbf,glmstack,meancol_stack,indbase_col,0))) exit(-1);
        printf("indbase_col\n");
        for(i=0;i<tcs->total;i++) printf("%d ",indbase_col[i]); printf("\n");
        }
    for(kk=k=0;k<tcs->num_tc;k++) {
        if(!lc_names_only||(lc_anova&&!k)) {
            for(i=0;i<ap->vol;i++)temp_double[i]=td[i]=0.;
            if(!num_tc_weights) scl = !hipass[k] ? 0. : tcs->num_tc_to_sum[k][0]/2.;
            }
        for(i=0;i<tcs->num_tc_to_sum[k][0];i++,kk++) {
            glmi = fbf->file_index[kk];
            if(!lc_names_only||(lc_anova&&!k&&!nregions)) {
                tci = fbf->roi_index[kk];
                if(!(fp=fopen_sub(glmfiles->files[glmi],"r"))) exit(-1);

                //if(glmstack[glmi]->ifh->glm_masked)for(j=0;j<vol;j++)td[j]=0.;
                //START161027
                if(glmstack[glmi]->ifh->glm_masked)for(j=0;j<ms->lenbrain;j++)td[ms->brnidx[j]]=(double)UNSAMPLED_VOXEL;

                if(fseek(fp,startb[glmi]+(long)(sizeof(float)*glmstack[glmi]->nmaski*tci),(int)SEEK_SET)){
                    printf("fidlError: occured while seeking to %p in %s.\n",fp,glmfiles->files[glmi]);
                    exit(-1);
                    }
                if(!fread_sub(temp_float,sizeof(float),(size_t)glmstack[glmi]->nmaski,fp,swapbytesin[glmi],glmfiles->files[glmi])){
                    printf("fidlError: reading parameter estimates from %s.\n",glmfiles->files[glmi]);
                    exit(-1);
                    }
                for(j=0;j<glmstack[glmi]->nmaski;j++) 
                    td[glmstack[glmi]->maski[j]]=temp_float[j]==(float)UNSAMPLED_VOXEL?(double)UNSAMPLED_VOXEL:(double)temp_float[j];
                if(!unscaled_str[0]){
                    if(!lcindbase){
                        for(j=0;j<glmstack[glmi]->nmaski;j++)if(td[glmstack[glmi]->maski[j]]!=(double)UNSAMPLED_VOXEL)
                            td[glmstack[glmi]->maski[j]]*=gms_stack[glmi]->grand_mean[j]<gms_stack[glmi]->grand_mean_thresh?
                            0.:gms_stack[glmi]->pct_chng_scl/gms_stack[glmi]->grand_mean[j];
                        }
                    else{
                        if(fseek(fp,startb[glmi]+(long)(sizeof(float)*glmstack[glmi]->nmaski*indbase_col[kk]),SEEK_SET)){
                            printf("fidlError: occured while seeking to %p in %s.\n",fp,glmfiles->files[glmi]);
                            exit(-1);
                            }
                        if(!fread_sub(temp_float,sizeof(float),(size_t)glmstack[glmi]->nmaski,fp,swapbytesin[glmi],
                            glmfiles->files[glmi])){
                            printf("fidlError: reading parameter estimates from %s.\n",glmfiles->files[glmi]);
                            exit(-1);
                            }
                        for(j=0;j<glmstack[glmi]->nmaski;j++)if(td[glmstack[glmi]->maski[j]]!=(double)UNSAMPLED_VOXEL)
                            td[glmstack[glmi]->maski[j]]*=temp_float[j]<(float)GRAND_MEAN_THRESH?0.:100./(double)temp_float[j]; 

                        }
                    }
                dptr=td;
                if(nxformfiles){
                    if(!t4_atlas(td,stat,&t4[glmi*(int)T4SIZE],glmstack[glmi]->ifh->glm_xdim,glmstack[glmi]->ifh->glm_ydim,
                       glmstack[glmi]->ifh->glm_zdim,glmstack[glmi]->ifh->glm_dxdy,glmstack[glmi]->ifh->glm_dz,A_or_B_or_U[glmi],
                       glmstack[glmi]->ifh->orientation,ap,(double*)NULL))exit(-1);
                    dptr=stat;
                    }
                if(!num_tc_weights){
                    if(!hipass[k]){
                        for(j=0;j<ms->lenbrain;j++)
                            if(dptr[ms->brnidx[j]]!=(double)UNSAMPLED_VOXEL)dptr[ms->brnidx[j]]/=glmstack[glmi]->ATAm1[tci][tci];
                        scl+=1./glmstack[glmi]->ATAm1[tci][tci];
                        }
                    else {
                        for(j=0;j<ms->lenbrain;j++)
                            if(dptr[ms->brnidx[j]]!=(double)UNSAMPLED_VOXEL)dptr[ms->brnidx[j]]*=dptr[ms->brnidx[j]];
                        }
                    }
                else{
                    for(j=0;j<ms->lenbrain;j++)if(dptr[ms->brnidx[j]]!=(double)UNSAMPLED_VOXEL)
                        dptr[ms->brnidx[j]]*=tc_weights->tc[k][0][i];
                    }
                for(j=0;j<ms->lenbrain;j++){
                    if(dptr[ms->brnidx[j]]==(double)UNSAMPLED_VOXEL)temp_double[ms->brnidx[j]]=(double)UNSAMPLED_VOXEL;
                    else if(temp_double[ms->brnidx[j]]!=(double)UNSAMPLED_VOXEL)temp_double[ms->brnidx[j]]+=dptr[ms->brnidx[j]];
                    }
                fclose(fp);
                }
            }
        if(!lc_names_only||(lc_anova&&!k&&!nregions)){
            if(!num_tc_weights){
                for(i=0;i<ms->lenbrain;i++)if(temp_double[ms->brnidx[i]]!=(double)UNSAMPLED_VOXEL)temp_double[ms->brnidx[i]]/=scl;
                }
            if(hipass[k]&&!lcmsqa){
                fprintf(stderr,"fidlInformation: Taking square root.\n");
                for(i=0;i<ms->lenbrain;i++)if(temp_double[ms->brnidx[i]]!=(double)UNSAMPLED_VOXEL)
                    temp_double[ms->brnidx[i]]=sqrt(temp_double[ms->brnidx[i]]);
                }
            dptr=temp_double;
            if(fwhm>0.){
                gauss_smoth2(dptr,stat,gs);
                dptr=stat;
                }
            how_many=ap->vol;
            if(mask_file){
                for(i=0;i<ms->lenbrain;i++)temp_float[i]=(float)dptr[ms->brnidx[i]];
                how_many=ms->lenbrain;
                }
            else if(num_region_files){
                if(!(crs(dptr,stat,rbf,(char*)NULL)))exit(-1);
                for(i=0;i<num_regions;i++)temp_float[i]=(float)stat[i];
                how_many=lc_anova?1:num_regions;
                }
            else if(roi){
                for(i=0;i<num_regions;i++)temp_float[i]=(float)dptr[roi[i]];
                }
            else{
                for(i=0;i<ap->vol;i++)temp_float[i]=(float)dptr[i];
                }

            }

        m = chunks > 1 ? chunks : anova_regions_loop;
        if(!lc_anova) {
            n = chunks > 1 ? chunkvol : how_many;
            }
        else {
            n=1;
            }
        p = chunks > 1 ? chunkvol : 1;
        toolongs[0]=0;
        if(strlen(tags_tcs->tags_tcptr[k])>(limits.name_max-filel[glmi])) {
            sprintf(toolongs,"toolong%d",k+1);

            //fprintf(stderr,"fidlError: %s is too long. Changing name to %s\n",tags_tcs->tags_tcptr[k],toolongs);
            //START180503
            if(!scratchdir)fprintf(stderr,"fidlError: %s is too long. Changing name to %s\n",tags_tcs->tags_tcptr[k],toolongs);

            }
        for(i=0;i<m;i++) {
            sprintf(string,"%s_%s%s%s",input_file_ptr[glmi],!toolongs[0]?tags_tcs->tags_tcptr[k]:toolongs,unscaled_str,appendstr);
            if(anova_regions_loop > 1) sprintf(string,"%s_%s",string,rbf->region_names_ptr[i]);
            if(tags_tcs->tags_tc_effects) sprintf(string,"%s_%s",string,tags_tcs->tags_tc_effectsptr[k]);
            if(chunks > 1) sprintf(string,"%s_CHUNK%d",string,i+1);
            strcat(string,scratch?".4dfp.img":Fileext[filetype]);
            if(!lc_names_only) {
                w1->temp_float=&temp_float[i*p];
                w1->how_many=n;

                //if(scratch&&(mask_file||num_region_files)){ifh_out->dim1=n;ifh_out->dim2=1;ifh_out->dim3=1;}
                //START161213
                //if(scratch&&chunks>1){ifh_out->dim1=n;ifh_out->dim2=1;ifh_out->dim3=1;}
                //START161220
                if(scratch){ifh_out->dim1=n;ifh_out->dim2=1;ifh_out->dim3=1;}

                w1->ifh=ifh_out; 
                if(!write1(string,w1))exit(-1);
                }
            if(!lcquiet) printf("Time courses written to %s\n",string);
            }
        if(lc_one_file) {
            if(!fwrite_sub(temp_float,sizeof(float),(size_t)(m*n),fpw,SunOS_Linux)) exit(-1);
            nfpw+=(m*n);
            }
        }
    if(lc_one_file) {
        fclose(fpw);
        if(filetype==(int)IMG){

            //if(scratch&&(mask_file||num_region_files)){ifh_out->dim1=nfpw;ifh_out->dim2=1;ifh_out->dim3=1;}
            //START161213
            //if(scratch&&chunks>1){ifh_out->dim1=nfpw;ifh_out->dim2=1;ifh_out->dim3=1;}
            //START161220
            if(scratch){ifh_out->dim1=nfpw;ifh_out->dim2=1;ifh_out->dim3=1;}

            if(!write_ifh(strfpw,ifh_out,(int)FALSE))exit(-1);
            }
        if(!lcquiet)printf("One file written to %s\n",strfpw);
        }
    }
exit(0);
}
