/* Copyright 1/21/16 Washington University.  All Rights Reserved.
   fidl_anova4.c  $Revision: 1.13 $ */
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

//START161028
#ifdef NAN
    //NAN is supported
#endif

//START161028
#if 0
#ifdef __sun__
    #include <nan.h>
#endif
#endif

#include <limits.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "nrutil.h"
#include "fidl.h"
#include "anova_header_new.h"
#include "monte_carlo_mult_comp.h"
#include "dmatrix_mult.h"
#include "cond_norm1.h"
#include "box_correction.h"
#include "get_atlas_coor.h"
#include "subs_cifti.h"
#include "subs_mask.h"
#include "shouldiswap.h"
#include "filetype.h"
#include "subs_nifti.h"
#include "write1.h"
#include "subs_util.h"
#include "minmax.h"
#include "t4_atlas.h"
#include "gsl_svd.h"
#include "get_grand_mean_struct.h"
#include "d2double.h"

//START180116
#include "map_disk.h"
#include "checkOS.h"
#include "get_atlas.h"
#include "read_xform.h"
#include "twoA_or_twoB.h"

typedef struct{
    char *rdriver,*rscript,*rdata0,*rdata,*rdatao,model[MAXNAME],**namesfixef;
    int *lmerl,nlmef,nlines,*lmerp,nbeta; //nbetase
    }LME;
LME *init_lme(int adaa0,char *scratchdir,char *output);

//int get_rdriver(char *driver_file,char *rdriver,int nstr);
//START170405
int get_rdriver(char *driver_file,char *rdriver,int nstr,int *notfactor,int *numeric);

void get_lme_models(FILE *fp,AnovaDesign *ad,int *lmerl,int indent);
int get_rscript_guts(char *file,AnovaDesign *ad,LME *lme,int regionalanalysis);
int get_rscript_guts2(AnovaDesign *ad,LME *lme,int regionalanalysis);
int get_rscript(AnovaDesign *ad,char *scratchdir,LME *lme,int chunk,int regionalanalysis,int lc_names_only);
char **get_lmef(AnovaDesign *ad,int *lmerl,int nlmef);
int get_rscript_guts_lmerTest(char *file,AnovaDesign *ad,LME *lme,int regionalanalysis);
int get_rscript_guts2_lmerTest(AnovaDesign *ad,LME *lme,int regionalanalysis);
char* get_rpath();
char** get_rscript_lmerTest(AnovaDesign *ad,char *scratchdir,LME *lme,int chunk,int regionalanalysis,int lc_names_only,char *rpath,
    char *output);
static char rcsid[] = "$Header: /home/hannah/mcavoy/idl/clib/RCS/fidl_anova4.c,v 1.13 2017/04/06 19:18:33 mcavoy Exp $";
int main(int argc,char **argv)
{
char string[MAXNAME],*string_ptr,*driver_file=NULL,*mask_file=NULL,*directory=NULL,string2[MAXNAME],correction,
    *regional_anova_name=NULL,regional_anova_name_str[MAXNAME],*Nimage_name=NULL,*space_str=NULL,*glm_list_file=NULL,
    *cov_matrix_dir=NULL,*wcmstr="WCM",*scratchdir=NULL,**pstr=NULL,*randomstr="RANDOM",*betweenstr="BETWEEN",*withinstr="WITHIN",
    *cellmeansf=NULL,atlas[7]="",atlas_glm[7],*cifti_xmldata=NULL,rscriptcall0[1000],rscriptcall[1000],*regvoxs[]={"region","voxel"},
    **lmef=NULL,*lmebetasef=NULL,lmebetasef_str[MAXNAME],*output=NULL,*rpath,*prepend="";
int i,j,k,l,m,n,o,p,kk,jj,jjj,ll,lenvol=0,lccleanup=0,lccleanup_only=0,num_region_names=0,how_many=0,flag=0,
    num_unsampled_voxels=0,*unsampled_voxels_idx,lc_F_uncorrected=0,lc_Z_uncorrected=0,text=0,modulo,
    num_glm_files=0,num_tc=0,num_contrasts=0,argc_tc=0,argc_c=0,frames=0,lc_Z_uncorrected_BOX=0,lc_box_correction_only=0,
    *lt2subjects_voxels_idx,num_voxels_lt2subjects,*nsubjects=NULL,min,max,n_threshold_extent=0,*extent=NULL,num_YYT_files=0,
    lc_Z_monte_carlo=0,lc_Z_BOX_monte_carlo=0,lenvol_whole=0,chunk=0,*subject_unsampled_count,lc_statview=0,*temp_int,nepsilon=0,
    SunOS_Linux,lcsphericity_new=0,lenbrain=0,dfSSE=0,npvalstr=0,lccellmeans=0,goose=0,df1start,end_fstat,
    voloption=0,ndimxyz=0,*dimxyz=NULL,tc_contrast,nreg=0,area,nextsub,pp,qq,q,ii,swapbytes,lcsphericity_ATAm1=0,bigendian=1,
    cellmeansn=0,*cellmeansi=NULL,lcWCM=0,lcssftest=0,lcGIGA=0,check=0,GIGAcheck=0,GIGAcellmeansn,*GIGAcellmeansi,istart,nglms_each=1,
    lcGIGAdesign=0,lcclassical=0,nglmpersub=0,*glmpersub=NULL,*Mcolsub,*Mcol,*coli,*tcstacki=NULL,glmpersubmax=0,Mcolsubmax,Mcolmax,
    Nrowmax=0,Mcoli,glmi,c_index,lccheckdim=1,lcwarnNaNInf=1,classical_WCM_GIGA,lccheckunsampled=1,lcmapone=0,lcunscaled=0,*nc,
    nxformfiles=0,*A_or_B_or_U=NULL,*work=NULL,ndep,condmax=10000,lclmerr=0,regvoxi,rc,rnvox=1000,filetype=IMG,cnt,
    lcdontrunanova=0,lcoutPROCXX=0,lc_names_only=0,lclmerTest=0,chunk0=0,nlmerpmap=0,lcuseF=1; 
float *temp_float=NULL,*temp_float_box,*t4=NULL,center[3],mmppix[3]; //,outputnum=0.,vo;
double *fstatmap=NULL,*zstat=NULL,td=0.,p_val=0.,*fstatmap_nobox,voxel_threshold=0.01,*fstatmap_box,*zstat_box,*dferror_stack=NULL,
    *threshold=NULL,**cov=NULL,**M,**covMT,**S,*subcov=NULL,**kbar,**kbarsq,**Ssq,trsqkbar,trkbarsq,trU,SSE=0.,
    *epsilon1=NULL,*epsilon2=NULL,*GIGAfstatmap=NULL,*GIGAchatmap=NULL,*GIGAhtildaprimeprimemap=NULL,*GIGAhtildamap=NULL,
    *dfeffect_stack=NULL,*temp_double=NULL,*epsilon1GIGAdesign=NULL,*epsilon2GIGAdesign=NULL,*GIGAdesignfstatmap=NULL,
    *chatGIGAdesign=NULL,*avg=NULL,*A,*ATA,*ATAbig,*transform,*transformATA,*transformATAtransform,*ATAm1,*dferror=NULL,
    *dfuneqvar=NULL,dfeqvar,*stat=NULL,*fstatmapuneqvar=NULL,*V,*Ssvd,*worksvd,cond,cond_norm2,td1,*lmerpmap=NULL,*lmerpmapp=NULL,
    *rstack=NULL,outputnum=0.; /* Y2ijkm,Y2ijkdot */

//int64_t dims[3];
//START170302
int64_t dims[4]={0,0,0,1};

size_t i1,j1,ri,incr; //k1
unsigned int ui,xmlsize=0;
AnovaDesign *ad;
Interfile_header *ifh=NULL,*ifh1,*ifh_reg;
Memory_Map **mm=NULL;
LinearModel **glmstack;
TC *tcs=NULL;
Mask_Struct *ms=NULL;
Helmert *h=NULL,*kr=NULL,*GIGAh=NULL,*GIGAhSPH=NULL;
CellMeansModel *cmm=NULL,*GIGAcmm=NULL;
FILE *fp,*fprn=NULL,*fprn1=NULL;
Files_Struct *region_names=NULL,*glm_files=NULL,*YYT_files=NULL,*pvalstr,*xformfiles=NULL;
Atlas_Param *ap=NULL;
Classical *c=NULL;
Algina *al=NULL,*alSPH=NULL;
Treats *withtreat=NULL,*bettreat=NULL;
Regions_By_File *fbf;
gsl_matrix_view ATAm1gsl,ATAbiggsl,transformgsl,Agsl,ATAgsl,subATAbiggsl,subtransformgsl,transformATAgsl,transformATAtransformgsl; 
gsl_permutation *perm;
Anovas *anovas=NULL;
SS *ss=NULL;
Mcmc *mcmc=NULL;
Grand_Mean_Struct **gms_stack=NULL;
LME *lme=NULL;
W1 *w1=NULL;
if(argc < 3) {
    fprintf(stderr,"    -driver:           File that specifies the factor levels and associated files for the anova.\n");
    fprintf(stderr,"    -uncompress:       Specify mask file to put back into image format.\n");
    fprintf(stderr,"    -regions:          List of region names for a regional anova.\n");
    fprintf(stderr,"    -voxel_threshold:  Mean of the absolute value of all the data points entering the anova at a particular\n");
    fprintf(stderr,"                       voxel.  The mean value must exceed the threshold for the voxel to be processed by\n"); 
    fprintf(stderr,"                       the anova, otherwise the voxel is set to 1e-37.\n");
    fprintf(stderr,"                       The default is 0.01.\n\n");
    fprintf(stderr,"    -directory:        Specify directory for output statistic files. Include backslash at the end.\n");
    fprintf(stderr,"    -clean_up          Delete the files used in the anova computation.\n");
    fprintf(stderr,"    -clean_up ONLY     Don't do any computation, just delete the files used in the anova computation.\n\n");
    fprintf(stderr,"    -p_val             Desired p-value for Worsley's f field multiple comparisons.\n");
    fprintf(stderr,"    -output:           Output options: F_uncorrected Z_uncorrected Z_uncorrected_BOX box_correction_only Z_monte_carlo Z_BOX_monte_carlo PROC01 ... PROC20\n");
    fprintf(stderr,"    -statview          Output text file for statview input. (Region is a factor.)\n");
    fprintf(stderr,"    -text              Print out text file of regional analysis. (Region is a factor.)\n\n");
    fprintf(stderr,"    -correction:       Type of multiple comparison correction: most_sensitive, Bonferroni, or Worsley.\n\n");
    fprintf(stderr,"    -glm_files:        List of *.glm files for computation of box correction.\n");
    fprintf(stderr,"    -tc:               List of time courses for computation of box correction. The first tc is 1.\n");
    fprintf(stderr,"    -contrasts:        List of contrasts for computation of box correction. The first contrast is 1.\n");
    fprintf(stderr,"    -frames:           This option is used with -output box_correction_only\n");
    fprintf(stderr,"    -regional_anova_name: Output filename for a regional anova.\n");
    fprintf(stderr,"    -Nimage_name:      Output filename for number of subjects at each voxel.\n");
    fprintf(stderr,"    -print_cov_matrix: Covariance matrices will be printed to the specified directory.\n");
    fprintf(stderr,"    -threshold_extent  Monte Carlo thresholds with spatial extent.\n");
    fprintf(stderr,"                       Ex. -threshold_extent \"3.75 18\" \"4 12\"\n");
    fprintf(stderr,"    -pval:             Associated p value for -threshold_extent. .05 is the default.\n");
    fprintf(stderr,"    -YYT_files:        List of YYT files for computation of box correction.\n");
    fprintf(stderr,"                       Files are *.4dfp.img double stacks.\n");
    fprintf(stderr,"    -glm_list_file:    If present this goes into the ifh 'name of data file' field of the output files.\n");
    fprintf(stderr,"    -scratchdir:       Location of precomputed scratchfiles. Include the backslash at the end.\n");
    fprintf(stderr,"    -WCM               Use weighted cell means analysis.\n");
    fprintf(stderr,"    -GIGA              Use Algina'a Generalized Improved General Approximation.\n");
    fprintf(stderr,"    -epsilon:          Value of epsilon for monte carlo simulations.\n");
    fprintf(stderr,"    -cellmeans:        Name of cell means file.\n");
    fprintf(stderr,"    -sphericity_new    New sphericity correction.\n");
    fprintf(stderr,"    -sphericity_ATAm1  Even newer sphericity correction. Still being evaluated.\n");
    fprintf(stderr,"    -dimxyz:           x,y,z image dimensions on output. Data is chunked and does not conform to an atlas.\n");
    fprintf(stderr,"    -littleendian      Write output in little endian. Big endian is the default.\n");
    fprintf(stderr,"    -atlas             Identifies the image output size.\n");
    fprintf(stderr,"    -GIGAdesign        Use the design matrix from the glm in Algina'a Generalized Improved General Approximation.\n");
    fprintf(stderr,"    -classical         Do the classical analysis.\n");
    fprintf(stderr,"    -glmpersub:        Number of glms for each subject. One number per subject.\n");
    fprintf(stderr,"    -dontcheckdim       Skip dimension checking.\n");
    fprintf(stderr,"    -dontwarnNaNInf     Don't warn about NaN or Inf.\n");
    fprintf(stderr,"    -dontcheckunsampled Don't check subjects for unsampled voxels.\n");
    fprintf(stderr,"    -unscaled          Data in MR units. Used for single subject F test.\n");
    fprintf(stderr,"    -xform_file:       t4 files defining the transform to atlas space. One for each glm.\n") ;
    fprintf(stderr,"                       Used for single subject F test.\n");
    fprintf(stderr,"    -lmerr:            Linear Mixed Effects via a call to lmer by an R script.\n") ;
    fprintf(stderr,"    -lmebetase:        Linear Mixed Effects. Name of file for estimates and their standard errors.\n");
    fprintf(stderr,"    -dontrunanova      Stops after the sphericity correction is computed.\n");
    fprintf(stderr,"    -outPROCXX         Every value in the output files will be XX from '-output PROCXX'.\n");
    fprintf(stderr,"    -names_only        Generate filenames only.\n");
    fprintf(stderr,"    -lmerTest:         Linear Mixed Effects via a call to lmerTest by an R script.\n") ;

    //START170706
    fprintf(stderr,"    -prepend:          Put this in front of the output name.\n") ;

    exit(-1);
    }
for(i=1;i<argc;i++) {
    if(!strcmp(argv[i],"-driver") && argc > i+1)
        driver_file = argv[++i];
    if(!strcmp(argv[i],"-uncompress")) {
        if(argc > i+1 && strchr(argv[i+1],'-') != argv[i+1]) {
            mask_file = argv[++i];
            }
        else {
            printf("No mask file specified for -uncompress option.\n");
            exit(-1);
            }
        }
    if(!strcmp(argv[i],"-regions") && argc > i+1) {
        for(j=1;i+j<argc;j++) {
	    string_ptr = argv[i+j] + 1;
	    if(strchr(argv[i+j],'-') == argv[i+j] && !isdigit(*string_ptr)) break;
	    ++num_region_names;
	    }
        if(!(region_names=get_files(num_region_names,&argv[i+1]))) exit(-1);
        i += num_region_names;
        }
    if(!strcmp(argv[i],"-voxel_threshold") && argc > i+1)
	voxel_threshold = atof(argv[++i]);

    #if 0
    if(!strcmp(argv[i],"-directory") && argc > i+1) {
        i1=strlen(argv[i+1]);
        if(argv[i+1][i1-1]!='/')i1++;
        if(!(directory=malloc(sizeof*directory*i1))) {
            printf("fidlError: fidl_anova4 Unable to malloc directory\n");
            exit(-1);
            }
        strncpy(directory,argv[++i],i1);
        if(directory[i1-1]=='/')directory[i1-1]=0;
        }
    #endif
    //START161027
    if(!strcmp(argv[i],"-directory") && argc > i+1)
        directory = argv[++i];


    if(!strcmp(argv[i],"-clean_up")) {
	lccleanup = 1;
        if(argc > i+1 && !strcmp(argv[i+1],"ONLY")) {
            lccleanup_only = 1;
            ++i;
            }
        }
    if(!strcmp(argv[i],"-p_val") && argc > i+1)
        p_val = atof(argv[++i]);
    if(!strcmp(argv[i],"-output") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) {
	    if(!strcmp(argv[i+j],"F_uncorrected")) lc_F_uncorrected = 1;
            else if(!strcmp(argv[i+j],"Z_uncorrected")) lc_Z_uncorrected = 1; 
            else if(!strcmp(argv[i+j],"Z_uncorrected_BOX")) lc_Z_uncorrected_BOX = 1; 
            else if(!strcmp(argv[i+j],"box_correction_only")) lc_box_correction_only = 1; 
            else if(!strcmp(argv[i+j],"Z_monte_carlo")) lc_Z_monte_carlo = 1; 
            else if(!strcmp(argv[i+j],"Z_BOX_monte_carlo")) lc_Z_BOX_monte_carlo = 1;
            else if(strstr(argv[i+j],"PROC")){
                output=argv[i+j];
                outputnum=strtod(argv[i+j]+4,NULL);
                }
	    }
	i+=lc_F_uncorrected+lc_Z_uncorrected+lc_Z_uncorrected_BOX+lc_box_correction_only+lc_Z_monte_carlo+lc_Z_BOX_monte_carlo
            +output?1:0;
	}
    if(!strcmp(argv[i],"-statview"))
	lc_statview = 1;
    if(!strcmp(argv[i],"-text"))
	text = 1;
    if(!strcmp(argv[i],"-correction") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) {
	    if(!strcmp(argv[i+j],"most_sensitive")) correction = 'm';
            else if(!strcmp(argv[i+j],"Bonferroni")) correction = 'B'; 
            else if(!strcmp(argv[i+j],"Worsley")) correction = 'W'; 
	    }
	++i;
	}
    if(!strcmp(argv[i],"-glm_files") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_glm_files;
        if(!(glm_files=get_files(num_glm_files,&argv[i+1]))) exit(-1);
        i += num_glm_files;
        }
    if(!strcmp(argv[i],"-tc") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_tc;
        argc_tc = i+1;
        i += num_tc;
        }
    if(!strcmp(argv[i],"-contrasts") && argc > i+1) {
        for(j=1;i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_contrasts;
        argc_c = i+1;
        i += num_contrasts;
        }
    if(!strcmp(argv[i],"-frames") && argc > i+1)
        frames = atoi(argv[++i]);
    if(!strcmp(argv[i],"-regional_anova_name") && argc > i+1)
        regional_anova_name = argv[++i];
    if(!strcmp(argv[i],"-Nimage_name") && argc > i+1)
        Nimage_name = argv[++i];
    if(!strcmp(argv[i],"-print_cov_matrix") && argc > i+1) {
        l= strlen(argv[i+1]);
        if(!(cov_matrix_dir=malloc(sizeof*cov_matrix_dir*(l+1)))) {
            printf("Error: Unable to malloc cov_matrix_dir\n");
            exit(-1);
            }
        strcpy(cov_matrix_dir,argv[++i]);
        if(cov_matrix_dir[l-2] == '/') cov_matrix_dir[l-2] = 0;
        }
    if(!strcmp(argv[i],"-threshold_extent") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++n_threshold_extent;
        if(!(threshold=malloc(sizeof*threshold*n_threshold_extent))) {
            printf("Error: Unable to malloc threshold\n");
            exit(-1);
            }
        if(!(extent=malloc(sizeof*extent*n_threshold_extent))) {
            printf("Error: Unable to malloc extent\n");
            exit(-1);
            }
        for(j=0;j<n_threshold_extent;j++) sscanf(argv[++i],"%lf %d",&threshold[j],&extent[j]);
        }
    if(!strcmp(argv[i],"-pval") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++npvalstr;
        if(!(pvalstr=get_files(npvalstr,&argv[i+1]))) exit(-1);
        pstr = pvalstr->files;
        i += npvalstr;
        }
    if(!strcmp(argv[i],"-YYT_files") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_YYT_files;
        if(!(YYT_files=get_files(num_YYT_files,&argv[i+1]))) exit(-1);
        i += num_YYT_files;
        }
    if(!strcmp(argv[i],"-glm_list_file") && argc > i+1)
        glm_list_file = argv[++i];
    if(!strcmp(argv[i],"-scratchdir") && argc > i+1)
        scratchdir = argv[++i];
    if(!strcmp(argv[i],"-WCM"))
        lcWCM = 1;
    if(!strcmp(argv[i],"-GIGA"))
        lcGIGA = 1;
    if(!strcmp(argv[i],"-epsilon") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++nepsilon;
        if(!(epsilon1=malloc(sizeof*epsilon1*nepsilon))) {
            printf("Error: Unable to malloc epsilon1\n");
            exit(-1);
            }
        if(!(epsilon2=malloc(sizeof*epsilon2*nepsilon))) {
            printf("Error: Unable to malloc epsilon2\n");
            exit(-1);
            }
        for(j=0;j<nepsilon;j++) epsilon1[j] = epsilon2[j] = atof(argv[++i]);
        }
    if(!strcmp(argv[i],"-cellmeans") && argc > i+1)
        cellmeansf = argv[++i];
    if(!strcmp(argv[i],"-sphericity_new")) 
        lcsphericity_new = lcclassical = 1;
    if(!strcmp(argv[i],"-sphericity_ATAm1"))
        lcsphericity_ATAm1 = 1;
    if(!strcmp(argv[i],"-dimxyz") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++ndimxyz;
        if(ndimxyz!=3) {
            printf("Error: Must specify three numbers with -dimxyz\n");
            exit(-1);
            }
        if(!(dimxyz=malloc(sizeof*dimxyz*ndimxyz))) {
            printf("Error: Unable to malloc dimxyz\n");
            exit(-1);
            }
        for(voloption=1,j=0;j<ndimxyz;j++) voloption *= dimxyz[j] = atoi(argv[++i]);
        }
    if(!strcmp(argv[i],"-littleendian"))
        bigendian = 0;
    if(!strcmp(argv[i],"-atlas") && argc > i+1)
        strcpy(atlas,argv[++i]);
    if(!strcmp(argv[i],"-GIGAdesign"))
        lcGIGAdesign = 1;
    if(!strcmp(argv[i],"-classical"))
        lcclassical = 1;
    if(!strcmp(argv[i],"-glmpersub") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++nglmpersub;
        if(!(glmpersub=malloc(sizeof*glmpersub*nglmpersub))) {
            printf("Error: Unable to malloc glmpersub\n");
            exit(-1);
            }
        for(glmpersubmax=j=0;j<nglmpersub;j++) if((glmpersub[j]=atoi(argv[++i]))>glmpersubmax) glmpersubmax=glmpersub[j];
        }
    if(!strcmp(argv[i],"-dontcheckdim"))
        lccheckdim = 0;
    if(!strcmp(argv[i],"-dontwarnNaNInf"))
        lcwarnNaNInf = 0;
    if(!strcmp(argv[i],"-dontcheckunsampled"))
        lccheckunsampled = 0;
    if(!strcmp(argv[i],"-unscaled"))
        lcunscaled = 1;
    if(!strcmp(argv[i],"-xform_file") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++nxformfiles;
        if(!(xformfiles=get_files(nxformfiles,&argv[i+1]))) exit(-1);
        i += nxformfiles;
        }
    if(!strcmp(argv[i],"-lmerr"))
	lclmerr = 1;
    if(!strcmp(argv[i],"-lmebetase") && argc > i+1)
        lmebetasef = argv[++i];
    if(!strcmp(argv[i],"-dontrunanova"))
        lcdontrunanova = 1;
    if(!strcmp(argv[i],"-outPROCXX"))
        lcoutPROCXX = 1;
    if(!strcmp(argv[i],"-names_only"))
        lc_names_only = 1;
    if(!strcmp(argv[i],"-lmerTest"))
        lclmerTest = 1;

    //START170706
    if(!strcmp(argv[i],"-prepend") && argc > i+1)
        prepend = argv[++i];

    }
if((SunOS_Linux=checkOS())==-1) exit(-1);
swapbytes = shouldiswap(SunOS_Linux,bigendian);
if(!lc_names_only){
    print_version_number(rcsid,stdout);fflush(stdout);
    if(!n_threshold_extent)lc_Z_monte_carlo=lc_Z_BOX_monte_carlo=0;
    if(num_glm_files&&!nglmpersub){printf("fidlError: Need to specify -glmpersub\n");fflush(stdout);exit(-1);}
    if(npvalstr) {
        if(npvalstr!=n_threshold_extent) {
            printf("fidlError: -pval need to have the same number of items as -threshold_extent\n");fflush(stdout);exit(-1); 
            }
        }
    nreg = num_region_names && YYT_files ? num_region_names : 1; 
    }

//if(!(ad=read_driver2(driver_file,nreg,1,0,lclmerr?':':'_'))) exit(-1);
//START170302
if(!(ad=read_driver2(driver_file,nreg,1,0,lclmerr?':':'_',0))) exit(-1);

//for(i=0;i<ad->num_datafiles;i++)printf("ad->datafiles[%d]=%s\n",i,ad->datafiles[i]);
chunk = ad->num_datafiles/ad->chunks;
if(!lc_names_only){
    //for(i=0;i<ad->num_factors0;i++)printf("%s \"%s\" %d\n",ad->fnptr[i],!ad->notfactor[i]?"factor":"numeric",ad->Perlman_levels[i]);

    printf("ad->num_datafiles=%d ad->chunks=%d chunk=%d\n",ad->num_datafiles,ad->chunks,chunk);
    printf("ad->num_factors0=%d ad->num_factors=%d ad->num_sources=%d ad->aa=%d ad->aa1=%d ad->num_datafiles=%d\n",
        ad->num_factors0,ad->num_factors,ad->num_sources,ad->aa,ad->aa1,ad->num_datafiles);
    printf("ad->Perlman_levels0=");for(i=0;i<ad->num_factors0;i++)printf("%d ",ad->Perlman_levels0[i]);printf("\n");
    printf("ad->Perlman_levels=");for(i=0;i<ad->num_factors;i++)printf("%d ",ad->Perlman_levels[i]);printf("\n");
    printf("ad->notfactor=");for(i=0;i<ad->num_factors0;i++)printf("%d ",ad->notfactor[i]);printf("\n");
    printf("ad->nnotfactor=%d ",ad->nnotfactor);
    printf("ad->notfactori=");for(i=0;i<ad->nnotfactor;i++)printf("%d ",ad->notfactori[i]);printf("\n");
    printf("ad->dferror_within=");for(i=0;i<ad->aa;i++)printf("%d ",ad->dferror_within[i]);printf("\n");
    printf("ad->dferror=");for(i=0;i<ad->aa;i++)printf("%d ",ad->dferror[i]);printf("\n");
    printf("ad->fstatfilesptr=");for(i=0;i<ad->num_sources;i++)printf("%s ",ad->fstatfilesptr[i]);printf("\n");
    printf("ad->within_between=");for(i=0;i<ad->num_factors;i++)printf("%d ",ad->within_between[i]);printf("\n");
    printf("ad->fnptr0=");for(i=0;i<ad->num_factors0;i++)printf("%s ",ad->fnptr0[i]);printf("\n");
    printf("ad->fnptr=");for(i=0;i<ad->num_factors;i++)printf("%s ",ad->fnptr[i]);printf("\n");
    for(i=1;i<ad->num_factors0;i++)if(ad->Perlman_levels0[i]<=1){
        printf("%s has %d levels. Must have at least 2 levels.\n",ad->fnptr0[i],ad->Perlman_levels0[i]);fflush(stdout);exit(-1);
        }

    #if 0
    for(j=i=0;i<ad->num_factors0;i++){
        printf("%s %d \"%s\" %s %s\n",ad->fnptr0[i],ad->Perlman_levels0[i],
            !ad->notfactor[i]?"factor":"numeric",!ad->notfactor[i]?(ad->within_between[j++]?betweenstr:withinstr):"",!i?randomstr:"");
        }
    #endif
    //START170405
    for(j=i=0;i<ad->num_factors0;i++){
        printf("%s %d \"%s\" %s %s\n",ad->fnptr0[i],ad->Perlman_levels0[i],
            !ad->notfactor[i]?"factor":(ad->numeric0[i]?"numeric":"SKIP"),
            !ad->notfactor[i]?(ad->within_between[j++]?betweenstr:withinstr):"",!i?randomstr:"");
        }


    if((chunk/ad->Perlman_levels[0])!=ad->withprod) {
        printf("fidlError: chunk/ad->Perlman_levels[0]=%d ad->withprod=%d Must be equal.\n",chunk/ad->Perlman_levels[0],ad->withprod);
        fflush(stdout);exit(-1);
        }
    }
if(lclmerr) {
    if(!(lme=init_lme(ad->aa0,scratchdir,output)))exit(-1);

    //if(!(lme->nlines=get_rdriver(driver_file,lme->rdriver,ad->num_factors0)))exit(-1);
    //START170405
    if(!(lme->nlines=get_rdriver(driver_file,lme->rdriver,ad->num_factors0,ad->notfactor,ad->numeric0)))exit(-1);

    if(!lc_names_only)printf("R driver written to %s\n",lme->rdriver);
    if(!get_rscript(ad,scratchdir,lme,chunk,regional_anova_name||text||!ad->datafiles,lc_names_only))exit(-1);
    if(!lc_names_only)printf("R script written to %s\n",lme->rscript);fflush(stdout);
    sprintf(rscriptcall0,"Rscript --vanilla %srscript.r --args ",scratchdir);
    if(!(lmef=get_lmef(ad,lme->lmerl,lme->nlmef)))exit(-1);
    voxel_threshold=0.;
    }
if(lclmerTest){
    if(!(rpath=get_rpath()))exit(-1);

    //printf("here3 directory=%s\n",directory);fflush(stdout);
    //printf("here3 output=%s\n",output);fflush(stdout);

    if(!(lme=init_lme(0,directory,output)))exit(-1);

    //if(!(lme->nlines=get_rdriver(driver_file,lme->rdriver,ad->num_factors0)))exit(-1);
    //START170405
    if(!(lme->nlines=get_rdriver(driver_file,lme->rdriver,ad->num_factors0,ad->notfactor,ad->numeric0)))exit(-1);

    if(!lc_names_only)printf("R driver written to %s\n",lme->rdriver);

    #if 0
    if(!(lmef=get_rscript_lmerTest(ad,scratchdir,lme,chunk,regional_anova_name||text||!ad->datafiles,lc_names_only,rpath,output)))
        exit(-1);
    #endif
    //START161027
    if(!(lmef=get_rscript_lmerTest(ad,directory,lme,chunk,regional_anova_name||text||!ad->datafiles,lc_names_only,rpath,output)))
        exit(-1);

    if(!lc_names_only)printf("R script written to %s\n",lme->rscript);fflush(stdout);
    sprintf(rscriptcall0,"%s/Rscript --vanilla %s --args ",rpath,lme->rscript);
    //printf("here0 lme->nlmef=%d lme->lmerp[0]=%d\n",lme->nlmef,lme->lmerp[0]);fflush(stdout);exit(-1);
    //printf("here0 lme->nlmef=%d lme->nbeta=%d\n",lme->nlmef,lme->nbeta);fflush(stdout);//exit(-1);

    //START161201
    voxel_threshold=0.;

    }
if(num_glm_files&&(nglmpersub!=ad->Perlman_levels[RANDOM])) {
    printf("fidlError: nglmpersub=%d ad->Perlman_levels[RANDOM]=%d\n",nglmpersub,ad->Perlman_levels[RANDOM]);
    exit(-1);
    }
if(!nepsilon&&!lclmerr&&!lclmerTest){
    if(!(epsilon1=malloc(sizeof*epsilon1*ad->aa*nreg))) {
        printf("fidlError: Unable to malloc epsilon1\n");
        exit(-1);
        }
    if(!(epsilon2=malloc(sizeof*epsilon2*ad->aa*nreg))) {
        printf("fidlError: Unable to malloc epsilon2\n");
        exit(-1);
        }
    for(k=0;k<ad->aa*nreg;k++) epsilon1[k] = epsilon2[k] = 1.;
    }
if(lcGIGAdesign) {
    if(!(epsilon1GIGAdesign=malloc(sizeof*epsilon1GIGAdesign*ad->aa*nreg))) {
        printf("Error: Unable to malloc epsilon1GIGAdesign\n");
        exit(-1);
        }
    if(!(epsilon2GIGAdesign=malloc(sizeof*epsilon2GIGAdesign*ad->aa*nreg))) {
        printf("Error: Unable to malloc epsilon2GIGAdesign\n");
        exit(-1);
        }
    if(!(chatGIGAdesign=malloc(sizeof*chatGIGAdesign*ad->aa*nreg))) {
        printf("Error: Unable to malloc chatGIGAdesign\n");
        exit(-1);
        }
    for(k=0;k<ad->aa*nreg;k++) epsilon1GIGAdesign[k] = epsilon2GIGAdesign[k] = chatGIGAdesign[k] = 1.;
    }
#ifndef MONTE_CARLO
    #if 0
    printf("ad->between=%d ad->within=%d ad->withprod=%d ad->betprod=%d\n",ad->between,ad->within,ad->withprod,ad->betprod);
    printf("ad->dfeffect="); for(k=0;k<ad->aa;k++) printf("%d ",ad->dfeffect[k]); printf("\n");
    #endif
#endif
if(!lc_box_correction_only){
    if(!lclmerr&&!lclmerTest)if(!(cellmeansi=malloc(sizeof*cellmeansi*ad->num_sources))){
        printf("Error: Unable to malloc cellmeansi\n");
        exit(-1);
        }
    if(ad->Perlman_levels[0]==1) {
        lcssftest=1;

        //START170302
        //printf("    We will do a single subject F test.\n");

        }
    if(lc_statview) {
        if(!(GIGAh=helmert(ad,1,0))) exit(-1);
        if(!(al=algina_init(ad,GIGAh))) exit(-1);
        }
    else {
        if(!lcssftest&&!lclmerr&&!lclmerTest) {
            for(k=i=0;i<ad->num_sources;i+=2,k++) { /*i=0 source is mean*/
                if(ad->dferror[k] <= 0) {
                    printf("Invalid ad->dferror[%d] = %d\n",k,ad->dferror[k]);
                    exit(-1);
                    }
                }
            }
        if(!lclmerr&&!lclmerTest)if(nonprop(ad)){
            printf("Unequal cell design.  Need to have proportional cell sizes for the classical analysis.\n");
            lcclassical=0;
            if(!ad->nwithin) {
                lcWCM=1;
                }
            else {
                lcGIGAdesign=1;
                }
            }
        if(lcclassical) {
            printf("We will do the classical analysis.\n");
            }
        if(lcWCM) {
            printf("We will use a full rank model, weighted cell means.\n");
            lcGIGA=0;
            }

        //if(lcssftest) printf("We will compute the single subject F test.\n");
        //START170302
        if(lcssftest)printf("We will do a single subject F test.\n");

        if(lcGIGA) {
            printf("We will use Algina's Generalized Improved General Approximation.\n");
            lcWCM=0;
            }
        if(lcGIGAdesign) {
            printf("We will use Algina's Generalized Improved General Approximation with the design matrix.\n");
            lcWCM=0;
            }
        if(lcWCM||lcssftest) {

            //START170302
            lcGIGA=lcGIGAdesign=0; 

            if(!(cmm=malloc(sizeof*cmm))) {
                printf("fidlError: Unable to malloc cmm\n");
                exit(-1);
                }
            cmm->ntreatments=ad->num_cells_notRANDOM;
            if(!(cmm->uhat=malloc(sizeof*cmm->uhat*cmm->ntreatments))) {
                printf("fidlError: Unable to malloc cmm->uhat\n");
                exit(-1);
                }
            if(!(cmm->weight=malloc(sizeof*cmm->weight*cmm->ntreatments))) {
                printf("fidlError: Unable to malloc cmm->weight\n");
                exit(-1);
                }
            if(!(cmm->n=malloc(sizeof*cmm->n*cmm->ntreatments))) {
                printf("fidlError: Unable to malloc cmm->n\n");
                exit(-1);
                }
            if(!(kr=kronecker(ad))) {
                printf("fidlError: Something went wrong in kronecker routine. Abort!\n");
                exit(-1);
                }
            if(!lc_names_only)printf("cmm->ntreatments=%d kr->ncol=%d\n",cmm->ntreatments,kr->ncol);
            cmm_design(ad,cmm);
            cellmeansn=1;
            cellmeansi[0]=ad->num_sources-2;

            //START170302
            #if 0
            if(lcssftest) {
                if(!(ss=init_ss(kr))) {
                    printf("fidlError: Something went wrong in init_ss. Abort!\n");
                    exit(-1);
                    }
                }
            #endif

            }
        else if(!lclmerr&&!lclmerTest){
            cellmeansn=ad->num_sources;
            for(i=0;i<ad->num_sources;i++) cellmeansi[i]=i;
            if(!(c=classical(ad))) exit(-1);
            }
        if(lcGIGA||lcGIGAdesign) {
            GIGAcellmeansn=1;
            if(!(GIGAcellmeansi=malloc(sizeof*GIGAcellmeansi*GIGAcellmeansn))) {
                printf("fidlError: Unable to malloc GIGAcellmeansi\n");
                exit(-1);
                }
            GIGAcellmeansi[0]=ad->num_sources-2;
            GIGAcheck=1;
            if(!(GIGAh=helmert(ad,1,0))) exit(-1);
            if(lcGIGAdesign) if(!(GIGAhSPH=helmert(ad,1,1))) exit(-1);
            if(!(GIGAcmm=malloc(sizeof*GIGAcmm))) {
                printf("fidlError: Unable to malloc GIGAcmm\n");
                exit(-1);
                }
            GIGAcmm->ntreatments=ad->num_cells_notRANDOM;
            /*printf("ad->num_cells_notRANDOM=%d ad->withprod=%d GIGAh->ncol=%d GIGAh->ncol_between=%d\n",ad->num_cells_notRANDOM,
                ad->withprod,GIGAh->ncol,GIGAh->ncol_between);*/
            if(!(GIGAcmm->n=malloc(sizeof*GIGAcmm->n*GIGAcmm->ntreatments))) {
                printf("fidlError: Unable to malloc GIGAcmm->n\n");
                exit(-1);
                }
            if(!(al=algina_init(ad,GIGAh))) exit(-1);
            if(lcGIGAdesign&&!lc_statview) if(!(alSPH=algina_init(ad,GIGAhSPH))) exit(-1);
            if(!(anovas=malloc(sizeof*anovas))) {
                printf("fidlError: Unable to malloc anovas\n");
                exit(-1);
                }
            }
        }
    if(regional_anova_name) {
        lenvol_whole = 1;
        goose = !cmm ? ad->max_length+38+1 : 0;
        if(!num_region_names&&num_glm_files) {
            if(!(fp=fopen_sub(glm_files->files[0],"r"))) exit(-1);
            if(!(ifh_reg=get_ifh(fp,(Interfile_header*)NULL))) exit(-1);
            if(ifh_reg->nregions) {
                if(!(region_names=malloc(sizeof*region_names))) {
                    printf("fidlError: Unable to malloc region_names in fidl_anova_new\n");
                    exit(-1);
                    }
                region_names->files = ifh_reg->region_names;
                num_region_names = region_names->nfiles = ifh_reg->nregions;
                }
            fclose(fp);
            }
         }

    }
if(lcssftest) if(!num_glm_files) {printf("fidlError: Need to specify -glm_files\n");exit(-1);}
if(!(glmstack=malloc(sizeof*glmstack*glmpersubmax))) {
    printf("fidlError: Unable to malloc glmstack\n");
    exit(-1);
    }
if(glm_files){
    for(j=0;j<glmpersub[0];j++) {
        if(!(glmstack[j]=read_glm(glm_files->files[j],(int)SMALL,SunOS_Linux))) {
            printf("fidlError: reading %s  Abort!\n",glm_files->files[j]);
            exit(-1);
            }
        }
    filetype=get_filetype2(glmstack[0]->ifh->glm_cifti_xmlsize,glmstack[0]->ifh->glm_boldtype);
    if(filetype==(int)NIFTI){
        dims[0]=glmstack[0]->ifh->glm_xdim;dims[1]=glmstack[0]->ifh->glm_ydim;dims[2]=glmstack[0]->ifh->glm_zdim;
        center[0]=glmstack[0]->ifh->center[0];center[1]=glmstack[0]->ifh->center[1];center[2]=glmstack[0]->ifh->center[2];
        mmppix[0]=glmstack[0]->ifh->mmppix[0];mmppix[1]=glmstack[0]->ifh->mmppix[1];mmppix[2]=glmstack[0]->ifh->mmppix[2];
        }
    if(!lcssftest){
        if(filetype==(int)CIFTI){
            xmlsize=glmstack[0]->ifh->glm_cifti_xmlsize;
            if(!(cifti_xmldata=malloc(sizeof*cifti_xmldata*xmlsize))) {
                printf("fidlError: Unable to malloc cifti_xmldata\n");
                exit(-1);
                }
            for(ui=0;ui<xmlsize;ui++)cifti_xmldata[ui]=glmstack[0]->cifti_xmldata[ui];
            }
        }
    else{ 
        if(filetype==(int)CIFTI)cifti_xmldata=glmstack[0]->cifti_xmldata;
        if(glmpersub[0]>1) {
            if(!(gms_stack=malloc(sizeof*gms_stack*glmpersub[0]))) {
                printf("fidlError: Unable to malloc gms_stack\n");
                exit(-1);
                }
            for(j=0;j<glmpersub[0];j++) {

                #if 0
                vol = glmstack[j]->ifh->glm_xdim*glmstack[j]->ifh->glm_ydim*glmstack[j]->ifh->glm_zdim;
                if(!(gms_stack[j]=get_grand_mean_struct(glmstack[j]->grand_mean,vol,glm_files->files[j],(FILE*)NULL))) exit(-1);
                #endif
                //START161027
                if(!(gms_stack[j]=get_grand_mean(glmstack[j],glm_files->files[j],(FILE*)NULL)))exit(-1);


                }
            }
        }
    }


if(!lc_names_only){
    if(ad->datafiles){
        for(j=0;j<ad->num_datafiles;j++)if(!ad->NA[j])break;
        if(!(ifh=read_ifh(ad->datafiles[j],(Interfile_header*)NULL))) exit(-1);
        lenvol = ifh->dim1*ifh->dim2*ifh->dim3;
        if(lccheckdim) {
            printf("Checking dimensions\n"); fflush(stdout);
            if(!check_dimensions(ad->num_datafiles,ad->datafiles,lenvol)) exit(-1);
            printf("Finished checking dimensions\n"); fflush(stdout);
            }
        lenvol_whole = lenvol*ad->chunks;
        if(atlas[0]) {
            if(mask_file) {
                printf("fidlError: -uncompress and -atlas should not both be used. Abort!\n");fflush(stdout);
                exit(-1);
                }

            //if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL)))exit(-1);
            //START170706
            if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL,(char*)NULL)))exit(-1);

            lenvol_whole = ap->vol;
            }
        else if(!mask_file&&num_glm_files){
            lenvol_whole=glmstack[0]->ifh->glm_xdim*glmstack[0]->ifh->glm_ydim*glmstack[0]->ifh->glm_zdim;

            //if(!(ap=get_atlas_param(atlas,glmstack[0]->ifh)))exit(-1);
            //START170706
            if(!(ap=get_atlas_param(atlas,glmstack[0]->ifh,(char*)NULL)))exit(-1);

            }
        if((lcGIGA||lcGIGAdesign)&&cellmeansf) {
            if(!(avg=malloc(sizeof*avg*lenvol_whole*ad->num_cells_notRANDOM))) {
                printf("fidlError: Unable to malloc avg\n");
                exit(-1);
                }
            }
        if(!(ms=get_mask_struct(mask_file,lenvol_whole,(int*)NULL,SunOS_Linux,(LinearModel*)NULL,0)))exit(-1);
        lenbrain = voloption ? voloption : ms->lenbrain;

        //how_many = voloption ? voloption : ms->lenvol;
        //START180116
        how_many = voloption ? voloption : ms->vol;

        printf("lenvol=%d lenvol_whole=%d lenbrain=%d how_many=%d chunk=%d\n",lenvol,lenvol_whole,lenbrain,how_many,chunk);
        //printf("ms->brnidx=");for(i=0;i<lenbrain;i++)printf("%d ",ms->brnidx[i]);printf("\n");
        fflush(stdout);
        if(!num_region_names){
            if(!voloption) {
                if(!ap) {
                    get_atlas(how_many,atlas);
                    ifh1=lcssftest?(atlas[0]?NULL:glmstack[0]->ifh):(filetype==(int)NIFTI?ifh:(output?ifh:NULL));

                    //if(!(ap=get_atlas_param(atlas,ifh1))) exit(-1);
                    //START170706
                    if(!(ap=get_atlas_param(atlas,ifh1,(char*)NULL))) exit(-1);

                    if(output)ap->xdim=lenvol_whole;
                    }
                ifh->dim1 = ap->xdim;
                ifh->dim2 = ap->ydim;
                ifh->dim3 = ap->zdim;
                ifh->mmppix[0] = ap->mmppix[0];
                ifh->mmppix[1] = ap->mmppix[1];
                ifh->mmppix[2] = ap->mmppix[2];
                ifh->center[0] = ap->center[0];
                ifh->center[1] = ap->center[1];
                ifh->center[2] = ap->center[2];
                space_str = ap->str;
                }
            else {
                ifh->dim1 = dimxyz[0];
                ifh->dim2 = dimxyz[1];
                ifh->dim3 = dimxyz[2];
                }
            ifh->bigendian=bigendian;
            if(ifh->datades) free(ifh->datades);
            if(!(ifh->datades=malloc(sizeof*ifh->datades*14))) {
                printf("fidlError: Unable to malloc ifh->datades\n");
                exit(-1);
                }
            strcpy(ifh->datades,"gaussianizedF");
            if(mask_file) {
                if(ifh->mask) free(ifh->mask);
                if(!(ifh->mask=malloc(sizeof*ifh->mask*(strlen(mask_file)+1)))) {
                    printf("fidlError: Unable to malloc ifh->mask\n");
                    exit(-1);
                    }
                strcpy(ifh->mask,mask_file);
                }
            if(glm_list_file) assign_glm_list_file_to_ifh(ifh,glm_list_file);
            }
        }
    else {
        lenvol = how_many = lenbrain = lenvol_whole = 1;
        }
    if(!lc_box_correction_only && !lccleanup_only) {
        if(!(nsubjects=malloc(sizeof*nsubjects*(text||lc_statview||!ad->datafiles?1:lenbrain)))) {
            printf("Error: Unable to malloc nsubjects\n");
            exit(-1);
            }
        if(!(mm=malloc(sizeof*mm*chunk))) {
            printf("Error: Unable to malloc mm\n");
            exit(-1);
            }
        if(!lcssftest) if(!(dferror=malloc(sizeof*dferror*lenvol_whole*ad->aa))) {
            printf("Error: Unable to malloc dferror\n");
            exit(-1);
            }
        if(!(dferror_stack=malloc(sizeof*dferror_stack*lenvol_whole))) {
            printf("Error: Unable to malloc dferror_stack\n");
            exit(-1);
            }
        if(!(dfeffect_stack=malloc(sizeof*dfeffect_stack*lenvol_whole))) {
            printf("Error: Unable to malloc dfeffect_stack\n");
            exit(-1);
            }
        if(!(temp_double=malloc(sizeof*temp_double*(lcssftest?how_many:lenvol_whole)))) {
            printf("Error: Unable to malloc temp_double\n");
            exit(-1);
            }

        //START170302
        #if 0
        if(lcssftest) {
            if(!(stat=malloc(sizeof*stat*how_many))) {
                printf("Error: Unable to malloc stat\n");
                exit(-1);
                }
            if(!(ss->eqvar=malloc(sizeof*ss->eqvar*lenbrain))) {
                printf("Error: Unable to malloc ss->eqvar\n");
                exit(-1);
                }
            if(!(ss->uneqvar=malloc(sizeof*ss->uneqvar*lenbrain))) {
                printf("Error: Unable to malloc ss->uneqvar\n");
                exit(-1);
                }
            if(!(dfuneqvar=malloc(sizeof*dfuneqvar*lenbrain))) {
                printf("Error: Unable to malloc dfuneqvar\n");
                exit(-1);
                }
            if(!(ss->fstatuneqvar=malloc(sizeof*ss->fstatuneqvar*ad->num_sources))) {
                printf("Error: Unable to malloc ss->fstatuneqvar\n");
                exit(-1);
                }
            }
        #endif

        if(!(temp_float=malloc(sizeof*temp_float*how_many))) {
            printf("Error: Unable to malloc temp_float\n");
            exit(-1);
            }
        for(j=0;j<how_many;j++) temp_float[j] = 0.; 
        if(lc_statview) {
            /*do nothing*/
            }
        else {
            if(regional_anova_name || text || !ad->datafiles) {

                //sprintf(regional_anova_name_str,"%s%s%s",directory?directory:"",directory?"/":"",regional_anova_name);
                //START161027
                sprintf(regional_anova_name_str,"%s%s",directory?directory:"",regional_anova_name);

                if(!(fprn = fopen_sub(regional_anova_name_str,"w"))) exit(-1);
                if(lmebetasef){

                    //sprintf(lmebetasef_str,"%s%s%s",directory?directory:"",directory?"/":"",lmebetasef);
                    //START161027
                    sprintf(lmebetasef_str,"%s%s",directory?directory:"",lmebetasef);

                    if(!(fprn1=fopen_sub(lmebetasef_str,"w"))) exit(-1);
                    }
                }
            if(!fprn) {
                if(!(zstat=malloc(sizeof*zstat*lenvol_whole))) {
                    printf("Error: Unable to malloc zstat\n");
                    exit(-1);
                    }
                if(!(work=malloc(sizeof*work*lenvol_whole))) {
                    printf("Error: Unable to malloc work\n");
                    exit(-1);
                    }
                if(lcclassical||lcWCM||lcssftest) {
                    if(!(fstatmap_box=malloc(sizeof*fstatmap_box*lenvol_whole))) {
                        printf("Error: Unable to malloc fstatmap_box\n");
                        exit(-1);
                        }
                    if(!(fstatmap_nobox=malloc(sizeof*fstatmap_nobox*lenvol_whole))) {
                        printf("Error: Unable to malloc fstatmap_nobox\n");
                        exit(-1);
                        }
                    if(!(zstat_box=malloc(sizeof*zstat_box*lenvol_whole))) {
                        printf("Error: Unable to malloc zstat_box\n");
                        exit(-1);
                        }
                    if(!(temp_float_box=malloc(sizeof*temp_float_box*how_many))) {
                        printf("Error: Unable to malloc temp_float_box\n");
                        exit(-1);
                        }
                    for(k=0;k<how_many;k++) temp_float_box[k] = 0.;
                    }
                }
            }
        if(lcclassical||lcWCM||lcssftest) {
            if(!(fstatmap=malloc(sizeof*fstatmap*ad->num_sources*lenbrain))) {
                printf("Error: Unable to malloc fstatmap\n");
                exit(-1);
                }
            for(j=0;j<ad->num_sources*lenbrain;j++) fstatmap[j] = (double)UNSAMPLED_VOXEL;
            if(lcssftest) {

                #if 0
                if(!(fstatmapuneqvar=malloc(sizeof*fstatmapuneqvar*ad->num_sources*lenbrain))) {
                    printf("Error: Unable to malloc fstatmapuneqvar\n");
                    exit(-1);
                    }
                for(j=0;j<ad->num_sources*lenbrain;j++) fstatmapuneqvar[j] = (double)UNSAMPLED_VOXEL;
                #endif
                //START170302
                if(!(stat=malloc(sizeof*stat*how_many))) {
                    printf("Error: Unable to malloc stat\n");
                    exit(-1);
                    }
                if(!(ss=init_ss(kr))){
                    printf("fidlError: Something went wrong in init_ss. Abort!\n");
                    exit(-1);
                    }
                ss->uneqvar=NULL;
                if(!(ss->eqvar=malloc(sizeof*ss->eqvar*lenbrain))) {
                    printf("Error: Unable to malloc ss->eqvar\n");
                    exit(-1);
                    }
                if(glmpersub[0]>1){
                    if(!(ss->uneqvar=malloc(sizeof*ss->uneqvar*lenbrain))) {
                        printf("fidlError: Unable to malloc ss->uneqvar\n");
                        exit(-1);
                        }
                    if(!(dfuneqvar=malloc(sizeof*dfuneqvar*lenbrain))) {
                        printf("fidlError: Unable to malloc dfuneqvar\n");
                        exit(-1);
                        }
                    if(!(ss->fstatuneqvar=malloc(sizeof*ss->fstatuneqvar*ad->num_sources))) {
                        printf("fidlError: Unable to malloc ss->fstatuneqvar\n");
                        exit(-1);
                        }
                    if(!(fstatmapuneqvar=malloc(sizeof*fstatmapuneqvar*ad->num_sources*lenbrain))) {
                        printf("Error: Unable to malloc fstatmapuneqvar\n");
                        exit(-1);
                        }
                    for(j=0;j<ad->num_sources*lenbrain;j++) fstatmapuneqvar[j] = (double)UNSAMPLED_VOXEL;
                    }

                }
            }
        if(lcGIGA) {
            if(!(GIGAfstatmap=malloc(sizeof*GIGAfstatmap*ad->num_sources*lenbrain))) {
                printf("Error: Unable to malloc GIGAfstatmap\n");
                exit(-1);
                }
            for(j=0;j<ad->num_sources*lenbrain;j++) GIGAfstatmap[j] = (double)UNSAMPLED_VOXEL;
            if(!(GIGAchatmap=malloc(sizeof*GIGAchatmap*ad->num_sources*lenbrain))) {
                printf("Error: Unable to malloc GIGAchatmap\n");
                exit(-1);
                }
            if(!(GIGAhtildaprimeprimemap=malloc(sizeof*GIGAhtildaprimeprimemap*ad->num_sources*lenbrain))) {
                printf("Error: Unable to malloc GIGAhtildaprimeprimemap\n");
                exit(-1);
                }
            if(!(GIGAhtildamap=malloc(sizeof*GIGAhtildamap*ad->num_sources*lenbrain))) {
                printf("Error: Unable to malloc GIGAhtildamap\n");
                exit(-1);
                }
            }
        if(lcGIGAdesign) {
            if(!(GIGAdesignfstatmap=malloc(sizeof*GIGAdesignfstatmap*ad->num_sources*lenbrain))) {
                printf("fidlError: Unable to malloc GIGAdesignfstatmap\n");
                exit(-1);
                }
            for(j=0;j<ad->num_sources*lenbrain;j++) GIGAdesignfstatmap[j] = (double)UNSAMPLED_VOXEL;
            }
        if(lclmerr||lclmerTest){
            nlmerpmap=lme->nlmef*4+lme->nbeta*(lclmerr?2:3); //4:Pr,F,NumDF,DenDF
            printf("nlmerpmap=%d\n",nlmerpmap);fflush(stdout);
            if(!(lmerpmap=malloc(sizeof*lmerpmap*nlmerpmap*lenbrain))){
                printf("fidlError: Unable to malloc lmerpmap\n");
                exit(-1);
                }
            if(!lcoutPROCXX){
                for(j=0;j<nlmerpmap*lenbrain;j++)lmerpmap[j]=0.;
                }
            else{
                for(k=i=0;i<lenbrain;i++)for(j=0;j<nlmerpmap;j++,k++)lmerpmap[k]=(double)i;
                //START161201
                //KEEP for(k=i=0;i<lenbrain;i++)for(j=0;j<nlmerpmap;j++,k++)lmerpmap[k]=outputnum;

                }

            //printf("here-1 rnvox<lenbrain?rnvox:lenbrain=%d rnvox=%d lenbrain=%d chunk*(rnvox<lenbrain?rnvox:lenbrain)=%d\n",
            //    rnvox<lenbrain?rnvox:lenbrain,rnvox,lenbrain,chunk*(rnvox<lenbrain?rnvox:lenbrain));fflush(stdout);

            if(!(rstack=malloc(sizeof*rstack*chunk*(rnvox<lenbrain?rnvox:lenbrain)))){
                printf("fidlError: Unable to malloc rstack\n");
                exit(-1);
                }
            }
        if(n_threshold_extent) if(!(mcmc=monte_carlo_mult_comp_init(ifh->dim1,ifh->dim2,ifh->dim3,lenbrain,ms->brnidx))) exit(-1);
        if(!(w1=write1_init()))exit(-1);
        w1->cifti_xmldata=cifti_xmldata;
        w1->xmlsize=xmlsize;
        w1->filetype=filetype;
        w1->how_many=how_many;
        w1->swapbytes=swapbytes;
        w1->dims=dims;
        w1->temp_float=temp_float;
        w1->center=center;
        w1->mmppix=mmppix;
        w1->temp_double=temp_double;
        w1->ifh=ifh;
        w1->lenbrain=lenbrain;
        }
    }
#ifndef MONTE_CARLO
    if(lc_statview) {
        /*do nothing*/
        }        
    else if(lcclassical||lcWCM||lcssftest||lcGIGAdesign) {
        if(num_glm_files) {
            if(num_contrasts) {
                j = num_contrasts;
                k = argc_c;
                tc_contrast = 1;
                }
            else if(num_tc) {
                j = num_tc;
                k = argc_tc;
                tc_contrast = 0;
                }
            else {
                printf("fidlError: num_tc and num_contrasts both 0. Abort!!!\n");
                exit(-1);
                }
            if(!(temp_int=malloc(sizeof*temp_int*j))) {
                printf("fidlError: Unable to malloc temp_int\n");
                exit(-1);
                }
            for(i=0;i<j;i++) temp_int[i] = ad->Perlman_levels[RANDOM];
            if(!(tcs=read_tc_string_new(j,temp_int,k,argv))) exit(-1);
            free(temp_int);
            #if 0
            printf("num_tc=%d tcs->num_tc=%d\n",num_tc,tcs->num_tc);
            printf("tcs->each="); for(i=0;i<tcs->num_tc;i++) printf("%d ",tcs->each[i]); printf("\n");
            printf("tcs->num_tc_to_sum\n");
            for(i=0;i<tcs->num_tc;i++) {
                for(j=0;j<tcs->each[i];j++) printf("%d ",tcs->num_tc_to_sum[i][j]);
                printf("\n");
                }
            for(i=0;i<tcs->num_tc;i++) {
                printf("i=%d\n",i);
                for(j=0;j<tcs->each[i];j++) printf("%d ",(int)tcs->tc[i][j][0]);
                printf("\n");
                }
            #endif
            nglms_each = num_glm_files/tcs->each[0];
            if(lcssftest) {
                if(nxformfiles) {
                    if(!(t4=malloc(sizeof*t4*(size_t)T4SIZE*nxformfiles))) {
                        printf("fidlError: Unable to malloc t4\n");
                        exit(-1);
                        }
                    if(!(A_or_B_or_U=malloc(sizeof*A_or_B_or_U*nxformfiles))) {
                        printf("fidlError: Unable to malloc A_or_B_or_U\n");
                        exit(-1);
                        }
                    for(i=0;i<nxformfiles;i++) {
                        if(!read_xform(xformfiles->files[i],&t4[i*(int)T4SIZE])) exit(-1);
                        if((A_or_B_or_U[i]=twoA_or_twoB(xformfiles->files[i])) == 2) exit(-1);
                        }
                    }

                //for(i=0;i<lenbrain;i++) ss->eqvar[i]=ss->uneqvar[i]=dfuneqvar[i]=0.;
                //START170302
                for(i=0;i<lenbrain;i++)ss->eqvar[i]=0.;
                if(glmpersub[0]>1)for(i=0;i<lenbrain;i++)ss->uneqvar[i]=dfuneqvar[i]=0.;

                for(dfeqvar=0.,j=0;j<glmpersub[0];j++) {

                    //for(i=0;i<ms->lenvol;i++) temp_double[i]=0;
                    //180116
                    for(i=0;i<ms->vol;i++) temp_double[i]=0;

                    #if 0
                    for(i=0;i<lenbrain;i++) temp_double[ms->brnidx[i]] = glmstack[j]->sd[ms->brnidx[i]]==(float)UNSAMPLED_VOXEL ? 
                        (double)UNSAMPLED_VOXEL : (double)glmstack[j]->sd[ms->brnidx[i]];
                    #endif
                    //START161027
                    if(glmstack[j]->ifh->glm_masked)
                        for(i=0;i<glmstack[j]->nmaski;i++)temp_double[glmstack[j]->maski[i]]=glmstack[j]->var[i]<=0.? 
                            (double)UNSAMPLED_VOXEL:sqrt((double)glmstack[j]->var[i]);
                    else
                        for(i=0;i<lenbrain;i++)temp_double[ms->brnidx[i]]=glmstack[j]->var[ms->brnidx[i]]<=0.? 
                            (double)UNSAMPLED_VOXEL:sqrt((double)glmstack[j]->var[ms->brnidx[i]]);

                    #if 0
                    if(glmpersub[0]>1) {
                        for(i=0;i<lenbrain;i++) {
                            if(temp_double[ms->brnidx[i]]!=(double)UNSAMPLED_VOXEL) {
                                temp_double[ms->brnidx[i]] *= gms_stack[j]->grand_mean[ms->brnidx[i]] < 
                                    gms_stack[j]->grand_mean_thresh ? 0 : 
                                    gms_stack[j]->pct_chng_scl/gms_stack[j]->grand_mean[ms->brnidx[i]];
                                }
                            }
                        }
                    #endif
                    //START161027
                    if(glmpersub[0]>1){
                        if(glmstack[j]->ifh->glm_masked){
                            for(i=0;i<glmstack[j]->nmaski;i++)if(temp_double[glmstack[j]->maski[i]]!=(double)UNSAMPLED_VOXEL)
                                temp_double[glmstack[j]->maski[i]]*=gms_stack[j]->grand_mean[i]<gms_stack[j]->grand_mean_thresh?0: 
                                gms_stack[j]->pct_chng_scl/gms_stack[j]->grand_mean[i];
                            }
                        else
                            for(i=0;i<lenbrain;i++)if(temp_double[ms->brnidx[i]]!=(double)UNSAMPLED_VOXEL)
                                temp_double[ms->brnidx[i]]*=gms_stack[j]->grand_mean[ms->brnidx[i]]<gms_stack[j]->grand_mean_thresh?0: 
                                gms_stack[j]->pct_chng_scl/gms_stack[j]->grand_mean[ms->brnidx[i]];
                        }

                    for(i=0;i<lenbrain;i++) if(temp_double[ms->brnidx[i]]!=(double)UNSAMPLED_VOXEL) 
                        temp_double[ms->brnidx[i]]*=temp_double[ms->brnidx[i]];
                    if(!get_atlas(glmstack[j]->ifh->glm_xdim*glmstack[j]->ifh->glm_ydim*glmstack[j]->ifh->glm_zdim,atlas_glm)&&
                        atlas[0]){
                        if(!t4_atlas(temp_double,stat,t4,glmstack[j]->ifh->glm_xdim,glmstack[j]->ifh->glm_ydim,
                            glmstack[j]->ifh->glm_zdim,glmstack[j]->ifh->glm_dxdy,glmstack[j]->ifh->glm_dz,A_or_B_or_U[0],
                            glmstack[j]->ifh->orientation,ap,(double*)NULL)) exit(-1);
                        for(i=0;i<lenbrain;i++) temp_double[ms->brnidx[i]] = stat[ms->brnidx[i]];
                        }
                    for(i=0;i<lenbrain;i++) {
                        ss->eqvar[i] += (double)glmstack[j]->ifh->glm_df*temp_double[ms->brnidx[i]];

                        #if 0
                        ss->uneqvar[i] += temp_double[ms->brnidx[i]];
                        dfuneqvar[i] += temp_double[ms->brnidx[i]]*temp_double[ms->brnidx[i]]/(double)glmstack[j]->ifh->glm_df;
                        #endif
                        //START170302
                        if(glmpersub[0]>1){
                            ss->uneqvar[i]+=temp_double[ms->brnidx[i]];
                            dfuneqvar[i]+=temp_double[ms->brnidx[i]]*temp_double[ms->brnidx[i]]/(double)glmstack[j]->ifh->glm_df;
                            }

                        }
                    dfeqvar += (double)glmstack[j]->ifh->glm_df;
                    }

                #if 0
                for(i=0;i<lenbrain;i++) {
                    ss->eqvar[i] /= dfeqvar;
                    dferror_stack[i] = dfeqvar;
                    dfuneqvar[i] = ss->uneqvar[i]*ss->uneqvar[i]/dfuneqvar[i];
                    }
                #endif
                //START170302
                for(i=0;i<lenbrain;i++){ss->eqvar[i]/=dfeqvar;dferror_stack[i]=dfeqvar;}
                if(glmpersub[0]>1)for(i=0;i<lenbrain;i++)dfuneqvar[i]=ss->uneqvar[i]*ss->uneqvar[i]/dfuneqvar[i];


                if(!(ss->Sstackinv=get_Sstackinv(0,glmstack,tcs,tc_contrast,ad,kr,glmpersub[0]))) exit(-1);


                }
            else if(lcsphericity_new || lcsphericity_ATAm1 || lcGIGAdesign) {
                if(!(subcov=malloc(sizeof*subcov*tcs->num_tc*tcs->num_tc*ad->Perlman_levels[RANDOM]*nreg))) {
                    printf("fidlError: Unable to malloc subcov\n");
                    exit(-1);
                    }
                /*START50 Will need to add in nreg.*/
                for(i=0;i<tcs->num_tc*tcs->num_tc*ad->Perlman_levels[RANDOM]*nreg;i++) subcov[i]=0.;
                if(lcsphericity_new||lcGIGAdesign) {

                    #if 0
                    if(!subcov_reduced_ATA(glm_files->files,num_glm_files,tcs,subcov,SunOS_Linux,tc_contrast,YYT_files)) exit(-1);
                    #endif
#if 1
                    printf("New sphericity code 090903\n");
                    if(!(Mcolsub=malloc(sizeof*Mcolsub*ad->Perlman_levels[RANDOM]))) {
                        printf("Error: Unable to malloc Mcolsub\n");
                        exit(-1);
                        }
                    if(!(Mcol=malloc(sizeof*Mcol*num_glm_files))) {
                        printf("fidlError: Unable to malloc Mcol\n");
                        exit(-1);
                        }
                    if(!(nc=malloc(sizeof*Mcol*num_glm_files))) {
                        printf("fidlError: Unable to malloc Mcol\n");
                        exit(-1);
                        }
                    for(Mcolsubmax=Mcolmax=k=i=0;i<ad->Perlman_levels[RANDOM];i++) {
                        fflush(stdout);
                        for(ifh1=(Interfile_header*)NULL,Mcolsub[i]=j=0;j<glmpersub[i];j++,k++) {
                            if(!(fp=fopen_sub(glm_files->files[k],"r"))) exit(-1);
                            if(!(ifh1=get_ifh(fp,ifh1))) exit(-1);
                            if((Mcol[k]=ifh1->glm_Mcol)>Mcolmax) Mcolmax = ifh1->glm_Mcol;
                            if(ifh1->glm_Nrow>Nrowmax) Nrowmax = ifh1->glm_Nrow;
                            Mcolsub[i] += ifh1->glm_Mcol;
                            nc[k] = ifh1->glm_nc;
                            free_ifh(ifh1,1);
                            fclose(fp);
                            }
                        free(ifh1);
                        if(Mcolsub[i]>Mcolsubmax) Mcolsubmax = Mcolsub[i];
                        } 
                    if(!(A=malloc(sizeof*A*Nrowmax*Mcolmax))) {
                        printf("fidlError: Unable to malloc A\n");
                        exit(-1);
                        }
                    i=tcs->num_tc>Mcolmax?tcs->num_tc:Mcolmax;
                    if(!(ATA=malloc(sizeof*ATA*i*i))) {
                        printf("fidlError: Unable to malloc ATA\n");
                        exit(-1);
                        }

                    if(!(ATAbig=malloc(sizeof*ATAbig*Mcolsubmax*Mcolsubmax))) {
                        printf("fidlError: Unable to malloc ATAbig\n");
                        exit(-1);
                        }
                    if(!(transform=malloc(sizeof*transform*tcs->num_tc*Mcolsubmax))) {
                        printf("fidlError: Unable to malloc transform\n");
                        exit(-1);
                        }
                    if(!(transformATA=malloc(sizeof*transformATA*tcs->num_tc*Mcolsubmax))) {
                        printf("fidlError: Unable to malloc transformATA\n");
                        exit(-1);
                        }
                    if(!(transformATAtransform=malloc(sizeof*transformATAtransform*tcs->num_tc*tcs->num_tc))) {
                        printf("fidlError: Unable to malloc transformATAtransform\n");
                        exit(-1);
                        }
                    if(!(ATAm1=malloc(sizeof*ATAm1*tcs->num_tc*tcs->num_tc))) {
                        printf("fidlError: Unable to malloc ATAm1\n");
                        exit(-1);
                        }
                    ATAm1gsl = gsl_matrix_view_array(ATAm1,tcs->num_tc,tcs->num_tc);
                    if(!(coli=malloc(sizeof*coli*glmpersubmax))) {
                        printf("fidlError: Unable to malloc coli\n");
                        exit(-1);
                        }
                    if(tc_contrast) {
                        if(!(tcstacki=malloc(sizeof*tcstacki*tcs->num_tc))) {
                            printf("fidlError: Unable to malloc tcstacki\n");
                            exit(-1);
                            }
                        }
                    perm = gsl_permutation_alloc(tcs->num_tc);
                    if(!(V=malloc(sizeof*V*tcs->num_tc*tcs->num_tc))) {
                        printf("fidlError: Unable to malloc V\n");
                        exit(-1);
                        }
                    if(!(Ssvd=malloc(sizeof*Ssvd*tcs->num_tc))) {
                        printf("fidlError: Unable to malloc Ssvd\n");
                        exit(-1);
                        }
                    if(!(worksvd=malloc(sizeof*worksvd*tcs->num_tc))) {
                        printf("fidlError: Unable to malloc worksvd\n");
                        exit(-1);
                        }


                    #if 1
                    for(Mcoli=jjj=jj=i=0;i<ad->Perlman_levels[RANDOM];Mcoli+=glmpersub[i++]) {
                        /*printf("i=%d subject %d\n",i,i+1);fflush(stdout);*/
                        ATAbiggsl = gsl_matrix_view_array(ATAbig,Mcolsub[i],Mcolsub[i]);
                        gsl_matrix_set_zero(&ATAbiggsl.matrix);
                        transformgsl = gsl_matrix_view_array(transform,tcs->num_tc,Mcolsub[i]);
                        gsl_matrix_set_zero(&transformgsl.matrix);

                        /*#if 1*/
                        for(j1=j=0;j<glmpersub[i];j++,jj++) {
                            coli[j] = j1;

                            #if 0
                            if(!(glmstack[j]=read_glm(glm_files->files[jj],1,SunOS_Linux))) {
                                printf("fidlError: reading %s  Abort!\n",glm_files->files[jj]);
                                exit(-1);
                                }
                            #endif
                            /*START150811*/
                            if(i)if(!(glmstack[j]=read_glm(glm_files->files[jj],1,SunOS_Linux))) {
                                printf("fidlError: reading %s  Abort!\n",glm_files->files[jj]);
                                exit(-1);
                                }

                            /*printf("%s\n",glm_files->files[jj]);*/

                            for(ll=0,k=0;k<glmstack[j]->ifh->glm_Nrow;k++) {
                                for(l=0;l<glmstack[j]->ifh->glm_Mcol;l++,ll++) A[ll]=(double)glmstack[j]->AT[l][k];
                                }
                            #if 0
                            printf("j=%d A\n",j);
                            for(ll=k=0;k<glmstack[j]->ifh->glm_Nrow;k++) {
                                for(l=0;l<glmstack[j]->ifh->glm_Mcol;l++,ll++) printf("%f ",A[ll]);
                                printf("\n");
                                }
                            printf("\n");
                            #endif
                            Agsl = gsl_matrix_view_array(A,glmstack[j]->ifh->glm_Nrow,glmstack[j]->ifh->glm_Mcol);
                            ATAgsl = gsl_matrix_view_array(ATA,glmstack[j]->ifh->glm_Mcol,glmstack[j]->ifh->glm_Mcol);
                            gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,&Agsl.matrix,&Agsl.matrix,0.0,&ATAgsl.matrix);
                            subATAbiggsl=gsl_matrix_submatrix(&ATAbiggsl.matrix,j1,j1,(size_t)glmstack[j]->ifh->glm_Mcol,
                                (size_t)glmstack[j]->ifh->glm_Mcol);
                            gsl_matrix_memcpy(&subATAbiggsl.matrix,&ATAgsl.matrix);
                            j1+=glmstack[j]->ifh->glm_Mcol;
                            }
                        /*#if 0*/

                        if(!tc_contrast) {
                            for(l=j=0;j<tcs->num_tc;j++,l+=Mcolsub[i]) {
                                for(k=0;k<tcs->num_tc_to_sum[j][i];k++) transform[l+((int)tcs->tc[j][i][k])-1]=1.;
                                }
                            }
                        else {
                            for(j=0;j<tcs->num_tc;j++) tcstacki[j]=(int)tcs->tc[j][i][0]-1;

                            #if 0
                            printf("here8 tcs->num_tc=%d\n",tcs->num_tc);fflush(stdout);
                            printf("here8 tcstacki=");for(j=0;j<tcs->num_tc;j++)printf("%d ",tcstacki[j]);printf("\n");fflush(stdout);
                            printf("here8 Mcol[%d]=%d\n",Mcoli,Mcol[Mcoli]);fflush(stdout);
                            #endif

                            if(!(fbf=find_regions_by_file(glmpersub[i],tcs->num_tc,&nc[Mcoli],tcstacki))) exit(-1);
                            for(j=0;j<tcs->num_tc;j++) {
                                glmi = fbf->file_index[j];
                                subtransformgsl = gsl_matrix_submatrix(&transformgsl.matrix,0,coli[glmi],(size_t)tcs->num_tc,
                                    (size_t)glmstack[glmi]->ifh->glm_Mcol);
                                c_index = fbf->roi_index[j]*glmstack[glmi]->ifh->glm_Mcol;
                                for(k=0;k<glmstack[glmi]->ifh->glm_Mcol;k++) {
                                    gsl_matrix_set(&subtransformgsl.matrix,j,k,(double)glmstack[glmi]->c[c_index+k]); 
                                    }
                                }
                            free_regions_by_file(fbf); 
                            }

                        /*#if 0*/

                        for(j=0;j<glmpersub[i];j++) free_glm(glmstack[j]);
                        transformATAgsl = gsl_matrix_view_array(transformATA,tcs->num_tc,Mcolsub[i]);
                        transformATAtransformgsl = gsl_matrix_view_array(transformATAtransform,tcs->num_tc,tcs->num_tc);
                        gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,&transformgsl.matrix,&ATAbiggsl.matrix,0.0,
                            &transformATAgsl.matrix);
                        gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,&transformATAgsl.matrix,&transformgsl.matrix,0.0,
                            &transformATAtransformgsl.matrix);

                        #if 0
                        printf("i=%d subject %d transform  Mcolsub[%d]=%d\n",i,i+1,i,Mcolsub[i]);
                        for(l=j=0;j<tcs->num_tc;j++) {
                            for(k=0;k<Mcolsub[i];k++,l++) printf("%f ",transform[l]);
                            printf("\n");
                            }
                        printf("\n\n");
                        #endif
                        #if 0
                        printf("i=%d subject %d transformATAtransform\n",i,i+1);
                        for(l=j=0;j<tcs->num_tc;j++) {
                            for(k=0;k<tcs->num_tc;k++,l++) printf("%f ",transformATAtransform[l]);
                            printf("\n");
                            }
                        printf("\n\n");
                        #endif

                        #if 0
                        gsl_matrix_set_identity(&ATAm1gsl.matrix);
                        gsl_linalg_cholesky_decomp(&transformATAtransformgsl.matrix);
                        for(j=0;j<tcs->num_tc;j++) {
                            ATAm1vgsl = gsl_matrix_row(&ATAm1gsl.matrix,j);
                            gsl_linalg_cholesky_svx(&transformATAtransformgsl.matrix,&ATAm1vgsl.vector);
                            }
                        #endif

                        /*printf("Mcolmax=%d tcs->num_tc=%d\n",Mcolmax,tcs->num_tc);fflush(stdout);*/

                        for(j=0;j<tcs->num_tc*tcs->num_tc;j++) ATA[j] = transformATAtransform[j];
                        if(cond_norm1(ATA,tcs->num_tc,&cond,ATAm1,perm))cond=0.;
                        /*printf("Condition number norm1: %f\n",cond);*/
                        if(cond>condmax||cond==0.) {
                            printf("    i=%d subject %d\n",i,i+1); fflush(stdout);
                            if(cond>condmax) printf("    Condition number greater than %d.",condmax);
                            else printf("    Design matrix not invertible.");
                            printf(" Computing singular value decomposition.\n");
                            td=gsl_svd_golubreinsch(transformATAtransform,tcs->num_tc,tcs->num_tc,0.,V,Ssvd,ATAm1,&cond_norm2,&ndep,
                                worksvd);
                            printf("    gsl_svd_golubreinsch cond_norm2=%f ndep=%d tol=%g\n",cond_norm2,ndep,td);
                            }
                        for(j=0;j<tcs->num_tc*tcs->num_tc;j++,jjj++) subcov[jjj] = ATAm1[j];
                        }
                    #endif

                    free(Mcolsub);
                    free(Mcol);
                    free(glmstack);
                    free(A);
                    free(ATA);
                    free(ATAbig);
                    free(transform);
                    free(transformATA);
                    free(transformATAtransform);
                    free(ATAm1);
                    free(coli);
                    if(tc_contrast) free(tcstacki); 
                    gsl_permutation_free(perm);
#endif

                    #if 0
                    printf("subcov\n");
                    for(l=i=0;i<ad->Perlman_levels[0];i++) {
                        printf("subject %d\n",i);
                        for(j=0;j<tcs->num_tc;j++) {
                            for(k=0;k<tcs->num_tc;k++,l++) printf("%f ",subcov[l]);
                            printf("\n");
                            }
                        }
                    #endif

                    }
                else if(lcsphericity_ATAm1) {

                    #if 0
                    printf("Using -sphericity_ATAm1\n");
                    if(!subcov_ATAm1(glm_files->files,num_glm_files,tcs,subcov,SunOS_Linux,tc_contrast,YYT_files)) exit(-1);
                    #endif
                    /*START140807*/
                    printf("fidlError: subcov_ATAm1 needs to rewritten\n");
                    exit(-1);

                    }
                else {
                    printf("fidlError: Unknown sphericity correction option.\n");
                    exit(-1);
                    }
                }
            else {

                /*KEEP*/
                #if 0
                if(!num_YYT_files) {
                    if(!(cov=cov_avg_reduced_ATA(glm_files->files,num_glm_files,tcs,SunOS_Linux,tc_contrast))) exit(-1);
                    }
                else {
                    box_tc_avg_reduced_ATA_wYYT(glm_files->files,num_glm_files,tcs,&i,YYT_files->files,SunOS_Linux);
                    exit(-1);
                    }
                #endif
                /*printf("fidlError: This needs to be written. Abort! 140430\n");fflush(stdout);exit(-1);*/

                }
            if(cov||subcov) {
                if(!h) if(!(h=helmert(ad,0,0))) exit(-1);
                if(cov) {
                    if(cov_matrix_dir) {
                        strcpy(string,cov_matrix_dir);
                        if((string_ptr=strrchr(string,'/'))) *string_ptr = 0;
                        sprintf(string2,"mkdir %s",string);
                        system(string2);
                        sprintf(string,"mkdir %s",cov_matrix_dir);
                        system(string);
                        sprintf(string,"%s/cov_global.4dfp.img",cov_matrix_dir);
                        print_cov_matrix(string,cov,h->ncol,ifh,bigendian,swapbytes);
                        }
                    printf("analytical BOX = %f\n",calculate_epsilon_whole_matrix_box(cov,h->ncol));
                    printf("empirical BOX = %f\n",calculate_epsilon_triangular(cov,h->ncol));
                    for(n=2,m=k=0,l=1;l<ad->aa;l++,k++,n+=2) {
                        M = (double **)dmatrix(1,h->box_dfeffect[k],1,h->ncol);
                        for(i=1;i<=h->box_dfeffect[k];i++,m++) {
                            for(j=1;j<=h->ncol;j++) {
                                M[i][j] = h->contrasts[m*h->ncol+j-1];
                                }
                            }
                        covMT = dmatrix_mult(cov,M,h->ncol,h->ncol,h->box_dfeffect[k],h->ncol,(int)TRANSPOSE_SECOND);
                        S = dmatrix_mult(M,covMT,h->box_dfeffect[k],h->ncol,h->ncol,h->box_dfeffect[k],(int)TRANSPOSE_NONE);
                        epsilon1[l] = epsilon2[l] = (double)calculate_epsilon_whole_matrix(S,h->box_dfeffect[k]);
                        printf("%s epsilon1 = %f epsilon2 = %f\n",ad->fstatfilesptr[n],epsilon1[l],epsilon2[l]);
                        if(cov_matrix_dir) {
                            sprintf(string,"%s/cov_%s.4dfp.img",cov_matrix_dir,ad->fstatfilesptr[n]);
                            print_cov_matrix(string,S,h->box_dfeffect[k],ifh,bigendian,swapbytes);
                            }
                        free_dmatrix(M,1,h->box_dfeffect[k],1,h->ncol);
                        free_dmatrix(covMT,1,h->ncol,1,h->box_dfeffect[k]);
                        free_dmatrix(S,1,h->box_dfeffect[k],1,h->box_dfeffect[k]);
                        }
                    free_dmatrix(cov,1,h->ncol,1,h->ncol);
                    fflush(stdout);
                    }
                else if(subcov) {
                    if(lcsphericity_new) {
                        printf("Using -sphericity_new\n");
                        printf("******************* NEW CODE ***************************\n");
                        cov = dmatrix(1,h->ncol,1,h->ncol);
                        for(o=2,m=k=0,p=1;p<ad->aa;p++,k++,o+=2) {
                            printf("\n%s\n",ad->fstatfilesptr[o]);
                            M = dmatrix(1,h->box_dfeffect[k],1,h->ncol);
                            kbar = dmatrix(1,h->box_dfeffect[k],1,h->box_dfeffect[k]);
                            kbarsq = dmatrix(1,h->box_dfeffect[k],1,h->box_dfeffect[k]);
                            covMT = dmatrix(1,h->ncol,1,h->box_dfeffect[k]);
                            S = dmatrix(1,h->box_dfeffect[k],1,h->box_dfeffect[k]);
                            Ssq = dmatrix(1,h->box_dfeffect[k],1,h->box_dfeffect[k]);
                            for(i=1;i<=h->box_dfeffect[k];i++,m++) {
                                for(j=1;j<=h->ncol;j++) {
                                    M[i][j] = h->contrasts[m*h->ncol+j-1];
                                    }
                                }
                            #if 0
                            printf("M\n");
                            for(i=1;i<=h->box_dfeffect[k];i++) {
                                for(j=1;j<=h->ncol;j++) printf("%f ",M[i][j]);
                                printf("\n");
                                }
                            #endif
                            area = h->ncol*h->ncol;
                            nextsub = area*nreg; 
                            /*printf("h->ncol=%d nreg=%d area=%d nextsub=%d\n",h->ncol,nreg,area,nextsub);*/
                            for(pp=p,qq=q=0;q<nreg;q++,qq+=area,pp+=ad->aa) {
                                for(j=1;j<=h->box_dfeffect[k];j++) {
                                    for(l=1;l<=h->box_dfeffect[k];l++) {
                                        kbar[j][l]=0.;
                                        kbarsq[j][l]=0.;
                                        }
                                    }
                                for(ii=qq,i=0;i<ad->Perlman_levels[RANDOM];i++,ii+=nextsub) {
                                    for(n=ii,j=1;j<=h->ncol;j++) for(l=1;l<=h->ncol;l++,n++) cov[j][l] = subcov[n]; 
                                    #if 0
                                    printf("%s\n",glm_files->files[i]);
                                    printf("ii=%d\n",ii);
                                    printf("HEREHERE region index q=%d  subject index i=%d cov\n",q,i);
                                    for(j=1;j<=h->ncol;j++) {
                                        for(l=1;l<=h->ncol;l++) printf("%f ",cov[j][l]); 
                                        printf("\n");
                                        }
                                    #endif
                                    dmatrix_mult_nomem(cov,M,h->ncol,h->ncol,h->box_dfeffect[k],h->ncol,(int)TRANSPOSE_SECOND,covMT);
                                    dmatrix_mult_nomem(M,covMT,h->box_dfeffect[k],h->ncol,h->ncol,h->box_dfeffect[k],
                                        (int)TRANSPOSE_NONE,S);
                                    for(j=1;j<=h->box_dfeffect[k];j++) for(l=1;l<=h->box_dfeffect[k];l++) kbar[j][l] += S[j][l]; 
                                    #if 0
                                    printf("S i=%d\n",i);
                                    for(j=1;j<=h->box_dfeffect[k];j++) {
                                        for(l=1;l<=h->box_dfeffect[k];l++) printf("%f ",S[j][l]); 
                                        printf("\n");
                                        }
                                    #endif
                                    dmatrix_mult_nomem(S,S,h->box_dfeffect[k],h->box_dfeffect[k],h->box_dfeffect[k],
                                        h->box_dfeffect[k],(int)TRANSPOSE_NONE,Ssq);
                                    for(j=1;j<=h->box_dfeffect[k];j++) for(l=1;l<=h->box_dfeffect[k];l++) kbarsq[j][l] += Ssq[j][l]; 
                                    }
                                dmatrix_mult_nomem(kbar,kbar,h->box_dfeffect[k],h->box_dfeffect[k],h->box_dfeffect[k],
                                    h->box_dfeffect[k],(int)TRANSPOSE_NONE,Ssq);
                                for(j=1;j<=h->box_dfeffect[k];j++) for(l=1;l<=h->box_dfeffect[k];l++) S[j][l] = (kbarsq[j][l] - 
                                    Ssq[j][l]/(double)ad->Perlman_levels[RANDOM])/(double)(ad->Perlman_levels[RANDOM]-1); 
                                #if 0
                                printf("S=\n");
                                for(j=1;j<=h->box_dfeffect[k];j++) {
                                    for(l=1;l<=h->box_dfeffect[k];l++) printf("%f ",S[j][l]);
                                    printf("\n");
                                    }
                                #endif
                                for(trsqkbar=0.,j=1;j<=h->box_dfeffect[k];j++) trsqkbar += kbar[j][j]; 
                                trsqkbar *= trsqkbar;
                                for(trkbarsq=0.,j=1;j<=h->box_dfeffect[k];j++) trkbarsq += Ssq[j][j];
                                /*printf("trsqkbar=%f trkbarsq=%f\n",trsqkbar,trkbarsq);*/
                                ad->dfeffectSPH[pp] = trsqkbar/trkbarsq;
                                epsilon1[pp] = ad->dfeffectSPH[pp]/(double)h->box_dfeffect[k];
                                trsqkbar /= ad->Perlman_levels[RANDOM]*ad->Perlman_levels[RANDOM];
                                trkbarsq /= ad->Perlman_levels[RANDOM]*ad->Perlman_levels[RANDOM];
                                for(trU=0.,j=1;j<=h->box_dfeffect[k];j++) trU += S[j][j];
                                ad->dferrorSPH[pp] =(ad->Perlman_levels[RANDOM]-1)*trsqkbar/(trkbarsq+trU/ad->Perlman_levels[RANDOM]);
                                epsilon2[pp] = ad->dferrorSPH[pp]/
                                    (double)(h->box_dfeffect[k]*(ad->Perlman_levels[RANDOM]-ad->betprod));
                                printf("ad->dfeffectSPH=%f ad->dfeffect=%d epsilon1[%d]=%f h->box_dfeffect=%d\n",
                                    ad->dfeffectSPH[pp],ad->dfeffect[p],pp,epsilon1[pp],h->box_dfeffect[k]);
                                printf("ad->dferrorSPH=%f ad->dferror=%d epsilon2[%d]=%f\n",ad->dferrorSPH[pp],ad->dferror[p],pp,
                                    epsilon2[pp]);
                                if(epsilon1[pp]>1.) {
                                    printf("epsilon1[%d]=%f Greater than 1. Capping it at 1!\n",pp,epsilon1[pp]);
                                    /*epsilon1[p] = 1.;*/
                                    epsilon1[pp] = 1.;
                                    }
                                if(epsilon2[pp]>1.) {
                                    printf("epsilon2[%d]=%f Greater than 1. Capping it at 1!\n",pp,epsilon2[pp]);
                                    /*epsilon2[p] = 1.;*/
                                    epsilon2[pp] = 1.;
                                    }
                                free_dmatrix(M,1,h->box_dfeffect[k],1,h->ncol);
                                free_dmatrix(kbar,1,h->box_dfeffect[k],1,h->box_dfeffect[k]);
                                free_dmatrix(kbarsq,1,h->box_dfeffect[k],1,h->box_dfeffect[k]);
                                free_dmatrix(covMT,1,h->ncol,1,h->box_dfeffect[k]);
                                free_dmatrix(S,1,h->box_dfeffect[k],1,h->box_dfeffect[k]);
                                free_dmatrix(Ssq,1,h->box_dfeffect[k],1,h->box_dfeffect[k]);
                                } 
                            }
                        free_dmatrix(cov,1,h->ncol,1,h->ncol);
                        fflush(stdout);
                        }

                    if(lcGIGAdesign) {
                        printf("\nUsing -GIGAdesign\n");
                        printf("GIGAhSPH->ncol=%d ad->withprod=%d\n",GIGAhSPH->ncol,ad->withprod);
                        printf("GIGAhSPH->ncol_between=%d ad->betprod=%d\n",GIGAhSPH->ncol_between,ad->betprod);
    
                        area = GIGAhSPH->ncol*GIGAhSPH->ncol;
                        nextsub = area*nreg;
                        for(pp=1,qq=q=0;q<nreg;q++,qq+=area) {
                            for(i=0;i<GIGAhSPH->ncol_between*GIGAhSPH->ncol*GIGAhSPH->ncol;i++) alSPH->sstack[i]=0.;
                            for(ii=qq,ll=i=0;i<GIGAhSPH->ncol_between;i++,ll+=GIGAhSPH->ncol*GIGAhSPH->ncol) {
                                for(j=0;j<al->nsubpergroup[i];j++,ii+=nextsub) {
                                    for(n=ii,l=ll,k=0;k<GIGAhSPH->ncol*GIGAhSPH->ncol;k++,l++,n++) alSPH->sstack[l]+=subcov[n];
                                    }
                                for(l=ll,j=0;j<GIGAhSPH->ncol*GIGAhSPH->ncol;j++,l++)alSPH->sstack[l]/=(double)alSPH->nsubpergroup[i];
                                }
    
                            #if 0
                            printf("alSPH->nsubpergroup=");
                            for(i=0;i<GIGAhSPH->ncol_between;i++) printf("%d ",alSPH->nsubpergroup[i]); printf("\n");
                            printf("alSPH->sstack\n");
                            for(l=i=0;i<GIGAhSPH->ncol_between;i++) {
                                for(j=0;j<GIGAhSPH->ncol;j++) { 
                                    for(k=0;k<GIGAhSPH->ncol;k++,l++) printf("%f ",alSPH->sstack[l]);
                                    printf("\n");
                                    }
                                }
                            #endif
                            #if 0
                            printf("\n");
                            for(l=i=0;i<ad->betprod;i++) {
                                sprintf(string,"group%d_Sg.dat",i+1);
                                if(!(fp=fopen_sub(string,"w"))) exit(-1);
                                for(j=0;j<ad->withprod;j++) {
                                    for(k=0;k<ad->withprod;k++,l++) fprintf(fp,"%f ",al->sstack[l]);
                                    fprintf(fp,"\n");
                                    }
                                fclose(fp);
                                }
                            #endif
    
                            if(algina_guts_anova(ad,GIGAhSPH,ad->Perlman_levels[RANDOM],alSPH->nsubpergroup,alSPH->sstack,alSPH,0,3,
                                (Anovas*)NULL)) exit(-1);


                            for(o=2,i=0;i<ad->aa1;i++,pp++,o+=2) {
                                printf("\n%s\n",ad->fstatfilesptr[o]);
                                epsilon1GIGAdesign[pp] = ad->htildaprimeprime[i]/(double)h->box_dfeffect[i];
                                epsilon2GIGAdesign[pp] =
                                    ad->htilda[i]/(double)(h->box_dfeffect[i]*(ad->Perlman_levels[RANDOM]-GIGAhSPH->ncol_between));
                                chatGIGAdesign[pp] = ad->chat[i];
                                printf("ad->htildaprimeprime[%d]=%f h->box_dfeffect[%d]=%d epsilon1GIGAdesign[%d]=%f\n",
                                    i,ad->htildaprimeprime[i],i,h->box_dfeffect[i],pp,epsilon1GIGAdesign[pp]);
                                printf("ad->htilda[%d]=%f den=%d epsilon2GIGAdesign[%d]=%f\n",i,ad->htilda[i],
                                    h->box_dfeffect[i]*(ad->Perlman_levels[RANDOM]-ad->betprod),pp,epsilon2GIGAdesign[pp]);
                                printf("chatGIGAdesign[%d]=%f\n",pp,chatGIGAdesign[pp]);
                                if(epsilon1GIGAdesign[pp]>1.) {
                                    printf("    epsilon1GIGAdesign[%d]=%f Greater than 1. Capping it at 1!\n",pp,
                                        epsilon1GIGAdesign[pp]);
                                    epsilon1GIGAdesign[pp] = 1.;
                                    }
                                if(epsilon2[pp]>1.) {
                                    printf("    epsilon2GIGAdesign[%d]=%f Greater than 1. Capping it at 1!\n",pp,
                                        epsilon2GIGAdesign[pp]);
                                    epsilon2GIGAdesign[pp] = 1.;
                                    }
                                }

                            //START151208
                            //printf("here5 lcdontrunanova=%d\n",lcdontrunanova);
                            if(lcdontrunanova)exit(0); 


                            }
                        /*START57*/
                        /*free_algina(alSPH);*/ /*100726 segmentation fault*/
                        }
                    free(subcov);
                    }
                }
            }
        }
#endif



if(!lc_box_correction_only) {
    if(!lccleanup_only) {

        #ifndef MONTE_CARLO
            if(text || lc_statview || !ad->datafiles) {
                for(k=j=0;j<lenvol;j++)
                    for(i=0;i<ad->aa;i++,k++) dferror[k] = (double)(ad->dferror_within[i]*(ad->Perlman_levels[RANDOM]-ad->betprod));
                nsubjects[0] = ad->Perlman_levels[0];
                }
            else if(!lc_names_only){
                for(j=0;j<lenbrain;j++) nsubjects[j] = ad->Perlman_levels[0];
                if(lccheckunsampled) {
                    if(!(subject_unsampled_count=malloc(sizeof*subject_unsampled_count*ad->Perlman_levels[0]))) {
                        printf("fidlError: Unable to malloc subject_unsampled_count\n");
                        exit(-1);
                        }
                    for(j=0;j<ad->Perlman_levels[0];j++) subject_unsampled_count[j] = 0; 
                    /*printf("newline=%d ad->withprod=%d\n",newline,ad->withprod); fflush(stdout);*/

                    if(how_many<lenbrain) {
                        printf("fidlError: Big problem. how_many=%d lenbrain=%d  temp_float was allocated with how_many\n",how_many,
                            lenbrain);
                        exit(-1);
                        }
                    printf("Checking for unsampled voxels. This bit of code assumes all files for a subject are contiguous.\n");
                    fflush(stdout);
                    for(ii=i=0;i<ad->Perlman_levels[0];i++) {
                        /*printf("Checking subject %d for unsampled voxels.\n",i+1); fflush(stdout);*/
                        for(j=0;j<lenbrain;j++) temp_float[j]=0.;
                        for(j=0;j<ad->withprod;j++) {
                            for(kk=k=0;k<ad->chunks;k++,ii++) {

                                #if 0
                                if(!(mm[0] = map_disk(ad->datafiles[ii],lenvol,0,sizeof(float)))) exit(-1);
                                for(flag=l=0;l<lenvol;l++,kk++) {
                                    if(kk==lenbrain) break;
                                    td = (double)mm[0]->ptr[l];
                                    if(mm[0]->ptr[l]==(float)UNSAMPLED_VOXEL) temp_float[kk]=1.;
                                    #ifdef __sun__
                                      else if(IsNANorINF(td)) {
                                    #else
                                      else if(isnan(td)||isinf(td)) {
                                    #endif
                                        temp_float[kk]=1.;
                                        flag=1;
                                        }
                                    }
                                unmap_disk(mm[0]);
                                #endif
                                //START161026
                                #if 0
                                //printf("%s\n",ad->datafiles[ii]);
                                if(ad->NA[ii]){
                                    //printf("here0a\n");
                                    #if 0
                                    for(l=0;l<lenvol;l++,kk++){
                                        if(kk==lenbrain)break;
                                        temp_float[kk]=1.; 
                                        }
                                    #endif
                                    }
                                else{
                                    //printf("here0b\n");
                                    if(!(mm[0]=map_disk(ad->datafiles[ii],lenvol,0,sizeof(float))))exit(-1);
                                    for(flag=l=0;l<lenvol;l++,kk++){
                                        if(kk==lenbrain)break;
                                        if(mm[0]->ptr[l]==(float)UNSAMPLED_VOXEL){
                                            temp_float[kk]=1.;
                                            }
                                        else if(!isfinite(mm[0]->ptr[l])){
                                            temp_float[kk]=1.;
                                            flag=1;
                                            }
                                        }
                                    unmap_disk(mm[0]);
                                    }
                                #endif
                                //START161028
                                if(!ad->NA[ii]){
                                    if(!(mm[0]=map_disk(ad->datafiles[ii],lenvol,0,sizeof(float))))exit(-1);
                                    for(flag=l=0;l<lenvol;l++,kk++){
                                        if(kk==lenbrain)break;
                                        if(mm[0]->ptr[l]==(float)UNSAMPLED_VOXEL){
                                            temp_float[kk]=1.;
                                            }
                                        else if(!isfinite(mm[0]->ptr[l])){
                                            temp_float[kk]=1.;
                                            flag=1;
                                            }
                                        }
                                    unmap_disk(mm[0]);
                                    }




                                if(flag&&lcwarnNaNInf) printf("WARNING: %s contains NaN and/or +-Inf\n",ad->datafiles[ii]);
                                }
                            }
                        for(j=0;j<lenbrain;j++) {
                            if(temp_float[j]>0.) {
                                subject_unsampled_count[i]++;
                                --nsubjects[j];
                                }
                            }
                        }
                    if(!cmm) {
                        printf("\nNumber of unsampled voxels for each subject.\n");
                        for(i=0;i<ad->Perlman_levels[0];i++) printf("    subject%d %d\n",i+1,subject_unsampled_count[i]);
                        printf("\n");
                        fflush(stdout);
                        }
                    free(subject_unsampled_count);
                    } 



                for(num_unsampled_voxels=num_voxels_lt2subjects=j=0;j<lenbrain;j++)
                    if(nsubjects[j]<ad->Perlman_levels[0]) {
                        ++num_unsampled_voxels;
                        if(nsubjects[j]<2) ++num_voxels_lt2subjects;
                        }
                printf("num_unsampled_voxels=%d\n",num_unsampled_voxels);
                printf("num_voxels_lt2subjects=%d\n",num_voxels_lt2subjects);
                if(!(unsampled_voxels_idx=malloc(sizeof*unsampled_voxels_idx*num_unsampled_voxels))) {
                    printf("Error: Unable to malloc unsampled_voxels_idx\n");
                    exit(-1);
                    }
                if(!(lt2subjects_voxels_idx=malloc(sizeof*lt2subjects_voxels_idx*num_voxels_lt2subjects))) {
                    printf("Error: Unable to malloc lt2subjects_voxels_idx\n");
                    exit(-1);
                    }
                for(num_unsampled_voxels=num_voxels_lt2subjects=j=0;j<lenbrain;j++)
                    if(nsubjects[j]<ad->Perlman_levels[0]) {
                        unsampled_voxels_idx[num_unsampled_voxels++] = j;
                        if(nsubjects[j]<2) lt2subjects_voxels_idx[num_voxels_lt2subjects++] = j;
                        }
                for(min=50000,max=0,i=0;i<lenbrain;i++) {
                    if(nsubjects[i] > max) max = nsubjects[i];
                    if(nsubjects[i] < min) min = nsubjects[i];
                    }
                printf("minimum number of subjects = %d\nmaximum number of subjects = %d\n",min,max);
                #if 0
                //START160128
                if(!lc_names_only){
                    for(num_unsampled_voxels=num_voxels_lt2subjects=j=0;j<lenbrain;j++)
                        if(nsubjects[j]<ad->Perlman_levels[0]) {
                            ++num_unsampled_voxels;
                            if(nsubjects[j]<2) ++num_voxels_lt2subjects;
                            }
                    printf("num_unsampled_voxels=%d\n",num_unsampled_voxels);
                    printf("num_voxels_lt2subjects=%d\n",num_voxels_lt2subjects);
                    if(!(unsampled_voxels_idx=malloc(sizeof*unsampled_voxels_idx*num_unsampled_voxels))) {
                        printf("Error: Unable to malloc unsampled_voxels_idx\n");
                        exit(-1);
                        }
                    if(!(lt2subjects_voxels_idx=malloc(sizeof*lt2subjects_voxels_idx*num_voxels_lt2subjects))) {
                        printf("Error: Unable to malloc lt2subjects_voxels_idx\n");
                        exit(-1);
                        }
                    for(num_unsampled_voxels=num_voxels_lt2subjects=j=0;j<lenbrain;j++)
                        if(nsubjects[j]<ad->Perlman_levels[0]) {
                            unsampled_voxels_idx[num_unsampled_voxels++] = j;
                            if(nsubjects[j]<2) lt2subjects_voxels_idx[num_voxels_lt2subjects++] = j;
                            }
                    for(min=50000,max=0,i=0;i<lenbrain;i++) {
                        if(nsubjects[i] > max) max = nsubjects[i];
                        if(nsubjects[i] < min) min = nsubjects[i];
                        }
                    printf("minimum number of subjects = %d\nmaximum number of subjects = %d\n",min,max);
                    }
                #endif





                if(!lcssftest) {
                    for(k=j=0;j<lenbrain;j++) for(i=0;i<ad->aa;i++,k++)
                        dferror[k] = (double)(nsubjects[j]<=ad->betprod ? 0 : ad->dferror_within[i]*(nsubjects[j]-ad->betprod));
                    }
                for(j=0;j<how_many;j++) temp_float[j] = 0.;
                if(Nimage_name) {
                    for(j=0;j<lenbrain;j++) temp_float[ms->brnidx[j]] = (float)nsubjects[j];

                    //sprintf(regional_anova_name_str,"%s%s%s",directory?directory:"",directory?"/":"",Nimage_name);
                    //START161027
                    sprintf(regional_anova_name_str,"%s%s",directory?directory:"",Nimage_name);

                    if(!writestack(regional_anova_name_str,temp_float,sizeof*temp_float,(size_t)how_many,swapbytes)) exit(-1);
                    min_and_max_init(&ifh->global_min,&ifh->global_max);
                    min_and_max_intstack(nsubjects,lenbrain,&ifh->global_min,&ifh->global_max);
                    if(!write_ifh(regional_anova_name_str,ifh,(int)FALSE)) exit(-1); 
                    fprintf(stdout,"Nimage written to %s\n",regional_anova_name_str);
                    }
                }
        #else
            if(!(nsubjects=malloc(sizeof*nsubjects*lenbrain))) { 
                printf("Unable to malloc nsubjects\n");
                exit(-1);
                }
            for(j=0;j<lenbrain;j++) nsubjects[j] = ad->Perlman_levels[0];

            #if 0
            for(k=j=0;j<lenvol_whole;j++) for(i=0;i<ad->aa;i++,k++)
                dferror[k] = ad->dferror_within[i]*(ad->Perlman_levels[RANDOM] - ad->betprod);
            #endif
            /*START140404*/
            for(k=j=0;j<lenvol_whole;j++) for(i=0;i<ad->aa;i++,k++)
                dferror[k] = (double)(ad->dferror_within[i]*(ad->Perlman_levels[RANDOM] - ad->betprod));

        #endif


        if(lc_statview) {
            if(!statview_guts(lenvol,al,ad,regional_anova_name)) exit(-1);
            exit(0);
            }
        else if(!lc_names_only&&!lcoutPROCXX){
            if(regional_anova_name || text || !ad->datafiles) {
                if(glm_list_file) assign_glm_list_file_to_ifh(ifh,glm_list_file);
                if(glm_list_file) fprintf(fprn,"DATA : %s\n",glm_list_file);
                if(cmm) {
                    if(lcWCM) fprintf(fprn,"Weighted cell means analysis\n"); 
                    }
                regvoxi=0;
                modulo = 1;
                }
            else {
                regvoxi=1;
                modulo = 20000;
                }
            #ifndef MONTE_CARLO
	        printf("voxel_threshold = %f\n",voxel_threshold);
                if(ad->datafiles) printf("A total of %d %s(s) per scratch file will be processed.\n",lenvol,regvoxs[regvoxi]);
            #endif

            //START160407
            //newline = ad->num_cells/ad->Perlman_levels[0];

            end_fstat = cmm ? ad->aa1 : ad->num_sources;

/*RUNS NO COMPUTATION*/
//#if 0
            #if 1

            if(lmerpmap)lmerpmapp=lmerpmap;
            for(ri=0,rc=lcmapone=qq=pp=k=n=0;n<ad->chunks;n++) {

                if(ad->datafiles&&!lcmapone) {
                    for(i=0;i<chunk;i++) {
                        j = i*ad->chunks + n;
                        /*printf("i=%d j=%d\n",i,j);fflush(stdout);*/

                        //if(!(mm[i] = map_disk(ad->datafiles[j],lenvol,0,sizeof(float)))) break;
                        //START161026
                        if(!ad->NA[j])if(!(mm[i]=map_disk(ad->datafiles[j],lenvol,0,sizeof(float))))break;

                        //for(j=0;j<lenvol;j++)printf("%f ",mm[i]->ptr[j]);printf("\n");


                        }
                    if(i<chunk) {
                        for(j=0;j<i;j++) unmap_disk(mm[j]);
                        printf("We will map the scratch files one at a time.\n");
                        fflush(stdout);
                        lcmapone=1;
                        }
                    }
                for(i=0;i<lenvol;i++,k++) {
                    if(ad->datafiles) {
                        if(k==lenbrain) break;
                        #ifndef MONTE_CARLO
                            if(num_region_names&&!lclmerr&&!lclmerTest){
                                fprintf(fprn,"REGION : %s\n",region_names->files[i]);
                                }
                            if(!(k%modulo)) {
                                printf("Processing %s = %d\n",regvoxs[regvoxi],k);
                                fflush(stdout);
                                }
                        #endif
                        }
                    if(nsubjects[k]>1 || (lcssftest&&nsubjects[k]==1)) {
                        for(j=0;j<ad->num_cells;j++) ad->data[j] = 0.;
                        /*Y2ijkm=0.; KEEP*/
                        if(ad->datafiles) {


                            #if 0
                            for(td1=0.,j=0;j<chunk;j++) {
                                j1=lcmapone?0:j;
                                if(lcmapone)if(!(mm[0] = map_disk(ad->datafiles[j*ad->chunks+n],lenvol,0,sizeof(float))))break;
                                td=(double)mm[0]->ptr[i];
                                #ifdef __sun__
                                  if(IsNANorINF(td)) {
                                #else
                                  if(isnan(td)||isinf(td)) {
                                #endif
                                    ad->data[ad->address[j]]=(float)UNSAMPLED_VOXEL;
                                    }
                                else {
                                    td1+=fabs((double)(ad->data[ad->address[j]]=mm[j1]->ptr[i]));
                                    }
                                if(lcmapone)unmap_disk(mm[0]);
                                }
                            #endif
                            //START161026
                            for(td1=0.,chunk0=j=0;j<chunk;j++) {
                                if(ad->NA[j*ad->chunks+n]){
                                    ad->data[ad->address[j]]=(float)UNSAMPLED_VOXEL;
                                    }
                                else{
                                    if(lcmapone)if(!(mm[0]=map_disk(ad->datafiles[j*ad->chunks+n],lenvol,0,sizeof(float))))break;
                                    if(!isfinite(mm[0]->ptr[i]))ad->data[ad->address[j]]=(float)UNSAMPLED_VOXEL;
                                    else{td1+=fabs((double)(ad->data[ad->address[j]]=mm[lcmapone?0:j]->ptr[i]));chunk0++;}
                                    if(lcmapone)unmap_disk(mm[0]);
                                    }
                                }

                            }
                        else {
                            for(td1=0.,j=0;j<chunk;j++) td1+=fabs((double)(ad->data[ad->address[j]]=ad->driver_data[j]));

                            //START161026
                            chunk0=chunk;

                            }

                        //td1/=(double)chunk;
                        //START161026
                        td1/=(double)chunk0;

                        if(fprn) printf("td1=%f voxel_threshold=%f\n",td1,voxel_threshold);
                        if(td1 > voxel_threshold) { 
                            if(lclmerr||lclmerTest) {

                                #if 0
                                KEEP
                                if(!(fp=fopen_sub(rdata,"w"))) exit(-1);
                                fprintf(fp,"bold\n");
                                for(j=0;j<chunk;j++)fprintf(fp,"%f\n",ad->data[ad->address[j]]);
                                fclose(fp);
                                if(system(rscriptcall)==-1){printf("fidlError: %s\n",rscriptcall);exit(-1);}
                                if(!(fp=fopen_sub(rscriptout,"r")))exit(-1);
                                for(j=k;fgets(string,sizeof(string),fp);j+=lenbrain)lmerpmap[j]=strtod(string,NULL);
                                fclose(fp);
                                #endif


                                //for(j=0;j<chunk;j++,ri++)rstack[ri]=(double)ad->data[ad->address[j]];
                                //START161028
                                for(j=0;j<chunk;j++,ri++)rstack[ri]=ad->data[ad->address[j]]==(float)UNSAMPLED_VOXEL?NAN:
                                    (double)ad->data[ad->address[j]];

                                if(++rc==rnvox||(k+1)==lenbrain) {
                                    if(!writestack(lme->rdata,rstack,sizeof*rstack,ri,0)) exit(-1);
                                    sprintf(rscriptcall,"%s%d",rscriptcall0,rc);

                                    //printf("%s\n",rscriptcall);fflush(stdout);
                                    //START180116
                                    if(!lc_names_only){printf("%s\n",rscriptcall);fflush(stdout);}

                                    if(system(rscriptcall)==-1){printf("fidlError: %s\n",rscriptcall);exit(-1);}

                                    #if 0
                                    incr=(size_t)((lme->nlmef+lme->nbeta*(lclmerr?2:3))*rc);
                                    #endif
                                    //START161103
                                    //printf("lme->nlmef=%d lme->nbeta=%d rc=%d (lme->nlmef+lme->nbeta*(lclmerr?2:3))*rc=%d\n",
                                    //    lme->nlmef,lme->nbeta,rc,(lme->nlmef+lme->nbeta*(lclmerr?2:3))*rc);
                                    //incr=(size_t)((lme->nlmef*3+lme->nbeta*(lclmerr?2:3))*rc);
                                    incr=(size_t)(nlmerpmap*rc);


                                    if(!readstack(lme->rdatao,lmerpmapp,sizeof*lmerpmapp,incr,1,0)) exit(-1);
                                    lmerpmapp+=incr;rc=0;ri=0;


                                    //START161028
                                    //printf("here100\n");fflush(stdout);
                                    //exit(-1);

                                    }

                                //START161027
                                //printf("here100\n");fflush(stdout);
                                //exit(-1);

                                }
                            if(lcclassical||lcWCM||lcssftest) {
                                if(cellmeans(ad,nsubjects[k],cmm,fprn,lccellmeans,cellmeansn,cellmeansi,&check)) {
                                    if(anova(ad,fprn,&epsilon1[pp],&epsilon2[pp],&dferror[k*ad->aa],kr,cmm,SSE,dfSSE,
                                        goose,c,lcWCM,lcGIGA,(double*)NULL,ss,k)) { /*ANOVA*/
                                        printf("ms->brnidx[%d]=%d\n",k,ms->brnidx[k]);
                                        if(!fprn) print_atlas_coor(ms->brnidx[k],ap);
                                        }
                                    else {
                                        for(j=0;j<end_fstat;j++) fstatmap[j*lenbrain+k] = ad->fstat[j];

                                        //START170302
                                        //printf("ad->fstat=");for(j=0;j<end_fstat;j++)printf("%f ",ad->fstat[j]);printf("\n");

                                        //if(ss) for(j=0;j<end_fstat;j++) fstatmapuneqvar[j*lenbrain+k] = ss->fstatuneqvar[j];
                                        //START170302
                                        if(ss&&(glmpersub[0]>1))
                                            for(j=0;j<end_fstat;j++)fstatmapuneqvar[j*lenbrain+k]=ss->fstatuneqvar[j];

                                        }
                                    if(fprn) fprintf(fprn,"\n");
                                    }
                                }
                            if(lcGIGA||lcGIGAdesign) { /*ANOVA - benefit of this clumsy coding-> can run both at once*/
                                if(algina(al,ad,GIGAh)) continue;
                                if(avg) for(j=0;j<ad->num_cells_notRANDOM;j++,qq++) avg[qq]=al->uhat_rearranged[j];
                                if(lcGIGA) {
                                    anovas->epsilon1 = &epsilon1[pp];
                                    anovas->epsilon2 = &epsilon2[pp];
                                    anovas->chatv = (double*)NULL;
                                    anovas->dferror = &dferror[k*ad->aa];
                                    anovas->goose = goose;
                                    anovas->lcGIGA = 1;
                                    anovas->fprn = fprn;
                                    anovas->cmm = GIGAcmm;
                                    classical_WCM_GIGA = 2;
                                    if(algina_guts_anova(ad,GIGAh,nsubjects[k],al->n,al->sstack,al,classical_WCM_GIGA-2,
                                        classical_WCM_GIGA,anovas)) {
                                        printf("ms->brnidx[%d]=%d\n",k,ms->brnidx[k]);
                                        if(!fprn) print_atlas_coor(ms->brnidx[k],ap);
                                        }
                                    else {
                                        for(jj=j=0;j<ad->aa1;j++,jj+=lenbrain) {
                                            jjj=jj+k;
                                            GIGAfstatmap[jjj] = ad->fstat[j];
                                            GIGAchatmap[jjj] = ad->chat[j];
                                            GIGAhtildaprimeprimemap[jjj] = ad->htildaprimeprime[j];
                                            GIGAhtildamap[jjj] = ad->htilda[j];
                                            }
                                        for(j=0;j<ad->betprod;j++) al->nold[j]=al->n[j];
                                        }
                                    if(fprn) fprintf(fprn,"\n");
                                    }
                                if(lcGIGAdesign) {
                                    anovas->epsilon1 = &epsilon1GIGAdesign[pp]; 
                                    anovas->epsilon2 = &epsilon2GIGAdesign[pp]; 
                                    anovas->chatv = &chatGIGAdesign[pp];
                                    anovas->dferror = &dferror[k*ad->aa];
                                    anovas->goose = goose;
                                    anovas->lcGIGA = 1;
                                    anovas->fprn = fprn;
                                    anovas->cmm = GIGAcmm;
                                    classical_WCM_GIGA = 3;
                                    if(algina_guts_anova(ad,GIGAh,nsubjects[k],al->n,al->sstack,al,classical_WCM_GIGA-2,
                                        classical_WCM_GIGA,anovas)) {
                                        printf("ms->brnidx[%d]=%d\n",k,ms->brnidx[k]);
                                        if(!fprn) print_atlas_coor(ms->brnidx[k],ap);
                                        }
                                    else {
                                        for(jj=j=0;j<ad->aa1;j++,jj+=lenbrain) {
                                            jjj=jj+k;
                                            GIGAdesignfstatmap[jjj] = ad->fstat[j];
                                            }
                                        for(j=0;j<ad->betprod;j++) al->nold[j]=al->n[j];

                                        //printf("k=%d ",k);for(j=0;j<end_fstat;j++) printf("%f ",ad->fstat[j]); printf("\n");
                                        /*if(k==342) exit(-1);*/
                                        }
                                    if(fprn) fprintf(fprn,"\n");
                                    }
                                }
                            }
                        else if(num_region_names) {
                            fprintf(fprn,"td=%f voxel_threshold=%f\n",td,voxel_threshold);
                            }
                        }
                    else if(num_region_names) {
                        fprintf(fprn,"    Region has zero voxels.\n\n");
                        }
                    if(nreg>1) pp += ad->aa;
                    } /*for(i=0;i<lenvol;i++)*/


                //if(ad->datafiles&&!lcmapone) for(i=0;i<chunk;i++) unmap_disk(mm[i]);
                //START161101
                if(ad->datafiles&&!lcmapone)for(i=0;i<chunk;i++)if(!ad->NA[i*ad->chunks+n])unmap_disk(mm[i]);


                } /*for(n=0;n<ad->chunks;n++)*/
            #endif

//#endif
/*END0*/

            #ifndef MONTE_CARLO
                if(ad->datafiles) {
                    printf("Finished processing all %ss.\n",regvoxs[regvoxi]);
                    fflush(stdout);
                    }
            #endif
            } /*else*/

    //START160127
    #if 0
        } /*if(!lccleanup_only) {*/
    if(!lccleanup_only) {
    #endif

        if(lc_statview) {
            /*do nothing*/
            }
        else if(lclmerr||lclmerTest){
            if(fprn) {
                fprintf(fprn,"%s\n\n",lme->model);
                for(k=i=0;i<lenbrain;i++) {
                    fprintf(fprn,"%s\tp\tz\n",region_names->files[i]);
                    for(j=0;j<lme->nlmef;j++,k++) {
                        fprintf(fprn,"%s\t%g\t%g\n",lmef[j],lmerpmap[k]/2.,gsl_cdf_ugaussian_Qinv(lmerpmap[k]/2.));
                        }
                    fprintf(fprn,"\n");

                    //START161103
                    k+=lme->nlmef*3;//F,NumDF,DenDf

                    if(fprn1){
                        if(lclmerr){
                            fprintf(fprn1,"%s\tModel\tEstimate\tStd. Error\n",region_names->files[i]);
                            for(jj=j=0;j<lme->nlmef+1;j++){
                                fprintf(fprn1,"\t%s%s\n",!j?"full model":"model for ",!j?"":lmef[j-1]);
                                for(l=0;l<lme->lmerp[j];l++,jj++,k++){
                                    fprintf(fprn1,"\t\t%s\t%.5f\t%.5f\n",lme->namesfixef[jj],lmerpmap[k],lmerpmap[k+lme->nbeta]);
                                    }
                                }
                            k+=lme->nbeta;
                            }
                        else{
                            fprintf(fprn1,"%s\tEstimate\tStd. Error\tPr(>|t|)\n",region_names->files[i]);
                            for(l=0;l<lme->lmerp[0];l++,k++){
                                fprintf(fprn1,"\t%s\t%.5f\t%.5f\t%.5f\n",lme->namesfixef[l],lmerpmap[k],lmerpmap[k+lme->nbeta],
                                    lmerpmap[k+2*lme->nbeta]);
                                }
                            k+=2*lme->nbeta;
                            }
                        fprintf(fprn1,"\n");
                        }
                    }
                }
            else {
                if(!lc_names_only){
                    if(filetype==(int)IMG){
                        if(ifh->file_name)free(ifh->file_name);
                        if(!(ifh->file_name=malloc(sizeof*ifh->file_name*(strlen(lme->model)+1)))) {
                            printf("fidlError: Unable to malloc ifh->file_name\n");
                            exit(-1);
                            }
                        strcpy(ifh->file_name,lme->model);
                        }
                    for(i1=0;i1<(size_t)how_many;i1++)temp_float[i1]=0.;
                    }

                //for(i1=0;i1<(size_t)lme->nlmef;i1++) {
                //START161103
                for(i=0;i<lme->nlmef;i++) {

                    //string[0] = 0;
                    //if(directory) sprintf(string,"%s/",directory);
                    //strcat(string,lmef[i1]);
                    //START161027
                    //sprintf(string,"%s%s",directory?directory:"",lmef[i1]);
                    //START161103
                    sprintf(string,"%s%s",directory?directory:"",lmef[i]);

                    sprintf(string2,"%s_zstat%s%s%s",string,space_str?space_str:"",output?output:"",Fileext[filetype]);

                    //printf("%s\n",lmef[i]);


                    if(!lc_names_only){

                        #if 0
                        if(!lcoutPROCXX){
                            for(k1=i1,j1=0;j1<(size_t)lenbrain;j1++,k1+=(size_t)lme->nlmef)
                                temp_float[ms->brnidx[j1]]=(temp_double[j1]=lmerpmap[k1]==(double)UNSAMPLED_VOXEL?
                                    (double)UNSAMPLED_VOXEL:gsl_cdf_ugaussian_Qinv(lmerpmap[k1]/2.))==(double)UNSAMPLED_VOXEL?
                                    (float)UNSAMPLED_VOXEL:(float)temp_double[j1];
                            }
                        else{ 
                            for(k1=i1,j1=0;j1<(size_t)lenbrain;j1++,k1+=(size_t)lme->nlmef)
                                temp_float[ms->brnidx[j1]]=(temp_double[j1]=lmerpmap[k1]==(double)UNSAMPLED_VOXEL?
                                    (double)UNSAMPLED_VOXEL:lmerpmap[k1])==(double)UNSAMPLED_VOXEL?
                                    (float)UNSAMPLED_VOXEL:(float)temp_double[j1];
                            }
                        #endif
                        //START161103
                        if(!lcoutPROCXX){
                            if(!lcuseF){
                                for(k=i,j=0;j<lenbrain;j++,k+=lme->nlmef*4)
                                    temp_float[ms->brnidx[j]]=(temp_double[j]=lmerpmap[k]==(double)UNSAMPLED_VOXEL?
                                        (double)UNSAMPLED_VOXEL:gsl_cdf_ugaussian_Qinv(lmerpmap[k]/2.))==(double)UNSAMPLED_VOXEL?
                                        (float)UNSAMPLED_VOXEL:(float)temp_double[j];
                                }
                            else{
                                for(k=i+lme->nlmef,j=0;j<lenbrain;j++,k+=lme->nlmef*4)temp_double[j]=lmerpmap[k];
                                for(k=i+2*lme->nlmef,j=0;j<lenbrain;j++,k+=lme->nlmef*4)dfeffect_stack[j]=lmerpmap[k];
                                for(k=i+3*lme->nlmef,j=0;j<lenbrain;j++,k+=lme->nlmef*4)dferror_stack[j]=lmerpmap[k];
                                f_to_z(temp_double,zstat,lenbrain,dfeffect_stack,dferror_stack,work);
                                for(j=0;j<lenbrain;j++)temp_float[ms->brnidx[j]]=(float)zstat[j];

                                //for(j=0;j<lenbrain;j++)if(!isfinite(temp_double[j]))printf("j=%d %f\n",j,temp_double[j]);

                                }
                            }
                        else{
                            for(k=i,j=0;j<lenbrain;j++,k+=lme->nlmef*4)
                                temp_float[ms->brnidx[j]]=(temp_double[j]=lmerpmap[k]==(double)UNSAMPLED_VOXEL?
                                    (double)UNSAMPLED_VOXEL:lmerpmap[k])==(double)UNSAMPLED_VOXEL?
                                    (float)UNSAMPLED_VOXEL:(float)temp_double[j];
                            }



                        if(!write1(string2,w1))exit(-1);
                        }
                    printf("Z statistics written to %s\n",string2);fflush(stdout);
                    if(filetype==(int)IMG&&!lc_names_only){
                        if(lc_Z_monte_carlo) {
                            if(!monte_carlo_mult_comp(temp_double,threshold,extent,n_threshold_extent,string2,ifh,0,temp_double,
                                string,swapbytes,pstr,(double*)NULL,(char*)NULL,mcmc,ap)) exit(-1);
                            }
                        }
                    }
                }
            }
        else {
            if(!fprn) {
                df1start = 1;
                istart = 0;
                if(lcclassical||lcssftest){
                    for(j=0;j<how_many;j++) temp_float[j]=0.;
                    if(lc_F_uncorrected) {
                        for(kk=df1start,i=istart,n=2;n<ad->num_sources;n+=2,i++,kk++) { /*i=0 is the mean*/
                            for(l=i*lenbrain,j=0;j<lenbrain;j++) temp_float[ms->brnidx[j]] = (float)fstatmap[l+j];

                            //sprintf(string,"%s%s_",directory?directory:"",ad->fstatfilesptr[n]);
                            //START170706
                            sprintf(string,"%s%s%s_",directory?directory:"",prepend,ad->fstatfilesptr[n]);

                            if(!cmm) strcat(string,"classic"); 
                            else if(lcssftest) strcat(string,"eqvar"); 
                            else if(lcWCM) strcat(string,wcmstr);
                            sprintf(string2,"%sfstat%s%s",string,space_str?space_str:"",Fileext[filetype]);
                            if(filetype==(int)IMG){ifh->df1=(float)ad->dfeffect[kk];ifh->df2=(float)ad->dferror[kk];}
                            if(!write1(string2,w1))exit(-1);
                            printf("F statistics written to %s\n",string2);fflush(stdout);
                            }
                        if(lcssftest&&glmpersub[0]>1){

                            for(kk=df1start,i=istart,n=2;n<ad->num_sources;n+=2,i++,kk++) { /*i=0 is the mean*/
                                for(l=i*lenbrain,j=0;j<lenbrain;j++) temp_float[ms->brnidx[j]] = (float)fstatmapuneqvar[l+j];

                                //sprintf(string,"%s%s",directory?directory:"",ad->fstatfilesptr[n]);
                                //START170706
                                sprintf(string,"%s%s%s",directory?directory:"",prepend,ad->fstatfilesptr[n]);

                                sprintf(string2,"%s_uneqvarfstat%s%s",string,space_str?space_str:"",Fileext[filetype]);
                                if(filetype==(int)IMG){ifh->df1=(float)ad->dfeffect[kk];ifh->df2=(float)ad->dferror[kk];}
                                if(!write1(string2,w1))exit(-1);
                                printf("F statistics written to %s\n",string2);fflush(stdout);
                                }
                            }
                        }
                    if(lc_Z_uncorrected||lc_Z_monte_carlo) {
                        for(kk=df1start,i=istart,n=2;n<ad->num_sources;n+=2,i++,kk++) { /*i=0 is the mean*/
                            for(j=0;j<lenbrain;j++) dfeffect_stack[j] = (double)ad->dfeffect[kk];
                            if(!lcssftest) for(j=0;j<lenbrain;j++) dferror_stack[j] = (double)dferror[ad->aa*j+kk]; 
                            f_to_z(&fstatmap[i*lenbrain],zstat,lenbrain,dfeffect_stack,dferror_stack,work);
                            for(j=0;j<lenbrain;j++) temp_float[ms->brnidx[j]] = (float)zstat[j];

                            //sprintf(string,"%s%s_",directory?directory:"",ad->fstatfilesptr[n]);
                            //START170706
                            sprintf(string,"%s%s%s_",directory?directory:"",prepend,ad->fstatfilesptr[n]);

                            if(!cmm) strcat(string,"classic");
                            else if(lcssftest) strcat(string,"eqvar");
                            else if(lcWCM) strcat(string,wcmstr);
                            sprintf(string2,"%szstat%s%s",string,space_str?space_str:"",Fileext[filetype]);
                            if(filetype==(int)IMG){ifh->df1=(float)ad->dfeffect[kk];ifh->df2=(float)ad->dferror[kk];}
                            if(lc_Z_uncorrected){
                                if(!write1(string2,w1))exit(-1); 
                                printf("Z statistics written to %s\n",string2);fflush(stdout);
                                }
                            if(filetype==(int)IMG){
                                if(n_threshold_extent) {
                                    if(!monte_carlo_mult_comp(zstat,threshold,extent,n_threshold_extent,string2,ifh,0,zstat,string,
                                        swapbytes,pstr,(double*)NULL,(char*)NULL,mcmc,ap)) exit(-1);
                                    }
                                }
                            if(lcssftest&&(glmpersub[0]>1)){
                                f_to_z(&fstatmapuneqvar[i*lenbrain],zstat,lenbrain,dfeffect_stack,dfuneqvar,work);
                                for(j=0;j<lenbrain;j++) temp_float[ms->brnidx[j]] = (float)zstat[j];

                                //sprintf(string,"%s%s",directory?directory:"",ad->fstatfilesptr[n]);
                                //START170706
                                sprintf(string,"%s%s%s",directory?directory:"",prepend,ad->fstatfilesptr[n]);

                                sprintf(string2,"%s_uneqvarzstat%s%s",string,space_str?space_str:"",Fileext[filetype]);
                                if(filetype==(int)IMG){ifh->df1=(float)ad->dfeffect[kk];ifh->df2=(float)ad->dferror[kk];}
                                if(lc_Z_uncorrected){
                                    if(!write1(string2,w1))exit(-1);
                                    printf("Z statistics written to %s\n",string2);fflush(stdout);
                                    }
                                if(filetype==(int)IMG){
                                    if(n_threshold_extent){
                                        if(!monte_carlo_mult_comp(zstat,threshold,extent,n_threshold_extent,string2,ifh,0,zstat,
                                            string,swapbytes,pstr,(double*)NULL,(char*)NULL,mcmc,ap)) exit(-1);
                                        }
                                    }

                                }
                            }
                        }

                    /****************************************************
                    Box reduced degrees of freedom zmap. No thresholding. 
                    ****************************************************/
                    if((lc_Z_uncorrected_BOX||lc_Z_BOX_monte_carlo)&&!lcssftest) {
                        for(kk=df1start,i=istart,n=2;n<ad->num_sources;n+=2,i++,kk++) { /*i=0 is the mean*/
                            printf("%s epsilon1[%d]=%f epsilon2[%d]=%f\n",ad->fstatfilesptr[n],kk,epsilon1[kk],kk,epsilon2[kk]);
                            for(j=0;j<lenbrain;j++) {
                                dfeffect_stack[j] = (double)ad->dfeffect[kk]*epsilon1[kk];
                                dferror_stack[j] = (double)dferror[ad->aa*j+kk]*epsilon2[kk];
                                }
                            f_to_z(&fstatmap[i*lenbrain],zstat,lenbrain,dfeffect_stack,dferror_stack,work);
                            for(j=0;j<lenbrain;j++) temp_float[ms->brnidx[j]] = (float)zstat[j];
                            min_and_max_init(&ifh->global_min,&ifh->global_max);
                            min_and_max_doublestack(zstat,lenbrain,&ifh->global_min,&ifh->global_max);

                            #if 0
                            string[0] = 0;
                            if(directory) sprintf(string,"%s/",directory);
                            strcat(string,ad->fstatfilesptr[n]);
                            strcat(string,"_");
                            #endif
                            //START161027
                            sprintf(string,"%s%s_",directory?directory:"",ad->fstatfilesptr[n]);

                            if(cmm) strcat(string,!lcssftest?wcmstr:"classic");
                            sprintf(string2,"%sSPHzstat%s%s",string,space_str?space_str:"",Fileext[filetype]);
                            if(filetype==(int)IMG){
                                ifh->df1=(float)UNSAMPLED_VOXEL;
                                ifh->df2=(float)UNSAMPLED_VOXEL;
                                ifh->box_correction=(float)UNSAMPLED_VOXEL;
                                }
                            if(lc_Z_uncorrected_BOX){
                                if(!write1(string2,w1))exit(-1);
                                printf("Z statistics written to %s\n",string2);fflush(stdout);
                                }
                            if(filetype==(int)IMG){
                                 if(n_threshold_extent) {
                                     if(!monte_carlo_mult_comp(zstat,threshold,extent,n_threshold_extent,string2,ifh,0,zstat,string,
                                         swapbytes,pstr,(double*)NULL,(char*)NULL,mcmc,ap)) exit(-1);
                                     }
                                 }
                             printf("\n");
                            }
                        } /*if(lc_Z_uncorrected_BOX)*/
                    }
                if(lcGIGA) {
                    for(kk=df1start,i=0,n=2;n<ad->num_sources;n+=2,i++,kk++) { /*i=0 is the mean*/
                        for(jj=i*lenbrain,j=0;j<lenbrain;j++,jj++) temp_double[j]=GIGAfstatmap[jj]/GIGAchatmap[jj];
                        f_to_z(temp_double,zstat,lenbrain,&GIGAhtildaprimeprimemap[i*lenbrain],&GIGAhtildamap[i*lenbrain],work);
                        for(j=0;j<lenbrain;j++) temp_float[ms->brnidx[j]] = (float)zstat[j];
                        min_and_max_init(&ifh->global_min,&ifh->global_max);
                        min_and_max_doublestack(zstat,lenbrain,&ifh->global_min,&ifh->global_max);

                        #if 0
                        string[0] = 0;
                        if(directory) sprintf(string,"%s/",directory);
                        strcat(string,ad->fstatfilesptr[n]);
                        strcat(string,"_");
                        #endif
                        //START161027
                        sprintf(string,"%s%s_",directory?directory:"",ad->fstatfilesptr[n]);

                        if(cmm) strcat(string,wcmstr);
                        sprintf(string2,"%sIGAzstat%s%s",string,space_str?space_str:"",Fileext[filetype]);
                        if(filetype==(int)IMG){
                            ifh->df1 = (float)UNSAMPLED_VOXEL;
                            ifh->df2 = (float)UNSAMPLED_VOXEL;
                            ifh->box_correction = (float)UNSAMPLED_VOXEL;
                            }
                        if(!write1(string2,w1))exit(-1);
                        printf("Z statistics written to %s\n",string2);fflush(stdout);
                        if(filetype==(int)IMG){
                            if(n_threshold_extent) {
                                if(!monte_carlo_mult_comp(zstat,threshold,extent,n_threshold_extent,string2,ifh,0,zstat,string,
                                    swapbytes,pstr,(double*)NULL,(char*)NULL,mcmc,ap)) exit(-1);
                                }
                            }

                        printf("\n");
                        }
                    } 
                if(lcGIGAdesign) {
                    for(kk=df1start,i=0,n=2;n<ad->num_sources;n+=2,i++,kk++) { /*i=0 is the mean*/
                        /*UNSAMPLED_VOXEL is the flag that tells us that you need the Nimage to get back to the fstats*/
                        /*printf("i=%d n=%d kk=%d\n",i,n,kk);fflush(stdout);*/
                        ifh->box_correction = (float)UNSAMPLED_VOXEL;

                        #if 0
                        string[0] = 0;
                        if(directory) sprintf(string,"%s/",directory);
                        strcat(string,ad->fstatfilesptr[n]);
                        #endif
                        //START161027
                        sprintf(string,"%s%s",directory?directory:"",ad->fstatfilesptr[n]);

                        if(lc_F_uncorrected||lc_Z_uncorrected||lc_Z_monte_carlo) {
                            ifh->df1 = (float)ad->dfeffect[kk];
                            ifh->df2 = (float)ad->dferror[kk];
                            for(jj=i*lenbrain,j=0;j<lenbrain;j++,jj++) temp_double[j]=GIGAdesignfstatmap[jj];
                            if(lc_F_uncorrected) {
                                for(j=0;j<lenbrain;j++) temp_float[ms->brnidx[j]] = (float)temp_double[j];
                                sprintf(string2,"%s_fstat%s%s",string,space_str?space_str:"",Fileext[filetype]);
                                if(!write1(string2,w1))exit(-1);
                                printf("F statistics written to %s\n",string2);fflush(stdout);
                                }
                            if(lc_Z_uncorrected||lc_Z_monte_carlo) {
                                for(j=0;j<lenbrain;j++) {
                                    dferror_stack[j] = dferror[ad->aa*j+kk];
                                    dfeffect_stack[j] = (double)ad->dfeffect[kk];
                                    }
                                for(cnt=j=0;j<lenbrain;j++)if(temp_double[j]>1.)cnt++;
                                if(!cnt)printf("**** All F statistics for %s are <=1 ****\n",string);
                                f_to_z(temp_double,zstat,lenbrain,dfeffect_stack,dferror_stack,work);
                                for(j=0;j<lenbrain;j++) temp_float[ms->brnidx[j]] = (float)zstat[j];
                                sprintf(string2,"%s_zstat%s%s",string,space_str?space_str:"",Fileext[filetype]);
                                if(!write1(string2,w1))exit(-1);
                                printf("Z statistics written to %s\n",string2);fflush(stdout);
                                if(filetype==(int)IMG){
                                    if(lc_Z_monte_carlo) {
                                        if(!monte_carlo_mult_comp(zstat,threshold,extent,n_threshold_extent,string2,ifh,0,zstat,string,
                                           swapbytes,pstr,(double*)NULL,(char*)NULL,mcmc,ap)) exit(-1);
                                        }
                                    }


                                }
                            }
                        if(lc_Z_uncorrected_BOX||lc_Z_BOX_monte_carlo) {
                            ifh->df1 = (float)ad->dfeffect[kk]*epsilon1GIGAdesign[kk];
                            ifh->df2 = (float)ad->dferror[kk]*epsilon2GIGAdesign[kk];
                            printf("%s epsilon1GIGAdesign[%d]=%f epsilon2GIGAdesign[%d]=%f chatGIGAdesign[%d]=%f\n",
                                ad->fstatfilesptr[n],kk,epsilon1GIGAdesign[kk],kk,epsilon2GIGAdesign[kk],kk,chatGIGAdesign[kk]);
                            printf("ad->dfeffect[%d]=%d ad->dferror[%d]=%d\n",kk,ad->dfeffect[kk],kk,ad->dferror[kk]);
                            fflush(stdout);
                            for(jj=i*lenbrain,j=0;j<lenbrain;j++,jj++) {
                                dfeffect_stack[j] = (double)ad->dfeffect[kk]*epsilon1GIGAdesign[kk];
                                dferror_stack[j] = dferror[ad->aa*j+kk]*epsilon2GIGAdesign[kk];
                                temp_double[j]=GIGAdesignfstatmap[jj]/chatGIGAdesign[kk];
                                }
                            f_to_z(temp_double,zstat,lenbrain,dfeffect_stack,dferror_stack,work);
                            for(j=0;j<lenbrain;j++) temp_float[ms->brnidx[j]] = (float)zstat[j];
                            sprintf(string2,"%s_IGADzstat%s%s",string,space_str?space_str:"",Fileext[filetype]);
                            if(lc_Z_uncorrected_BOX){
                                if(!write1(string2,w1))exit(-1);
                                printf("Z statistics written to %s\n",string2);fflush(stdout);
                                }
                            if(filetype==(int)IMG){
                                if(lc_Z_BOX_monte_carlo) {
                                    if(!monte_carlo_mult_comp(zstat,threshold,extent,n_threshold_extent,string2,ifh,0,zstat,string,
                                        swapbytes,pstr,(double*)NULL,(char*)NULL,mcmc,ap)) exit(-1);
                                    }
                                }

                            }
                        printf("\n");
                        }
                    }
	        } /*if(!fprn)*/
            } /*else*/
        } /*if(!lccleanup_only)*/
    if(fprn){
        fclose(fprn);
        printf("Tabular results printed to %s\n",regional_anova_name_str);
        }
    if(fprn1){
        fclose(fprn1);
        printf("Estimates and their standard errors written to %s\n",lmebetasef_str);
        }
    if(lccleanup || lccleanup_only) {
        if(scratchdir) {
            sprintf(string,"rm -rf %s",scratchdir);
            if(system(string) == -1) printf("Error: unable to %s\n",string);
            }
        else {
            if(ad->datafiles) delete_scratch_files(ad->datafiles,ad->num_datafiles);
            }
        }
    if(avg) {
        if(ad->nwithin) if(!(withtreat=get_treatment_str(ad,ad->nwithin,ad->withini))) exit(-1);
        if(ad->nbetween) if(!(bettreat=get_treatment_str(ad,ad->nbetween,ad->betweeni))) exit(-1);
        #if 0
        if(ad->nwithin) {
            printf("WITHIN\n");
            for(i=0;i<ad->withprod;i++) printf("%s\n",withtreat->tptr[i]);
            }
        if(ad->nbetween) {
            printf("BETWEEN\n");
            for(i=0;i<ad->betprod;i++) printf("%s\n",bettreat->tptr[i]);
            }
        #endif

        //sprintf(string,"%s%s%s",directory?directory:"",directory?"/":"",cellmeansf);
        //START161027
        sprintf(string,"%s%s",directory?directory:"",cellmeansf);

        if(!(fprn=fopen_sub(string,"w"))) exit(-1);
        for(ii=i=0;i<ad->betprod;i++,ii+=ad->withprod) {
            if(ad->nbetween) fprintf(fprn,"%s\n",bettreat->tptr[i]);
            /*if(ad->nbetween) printf("%s\n",bettreat->tptr[i]);*/
            if(region_names) {

                /*for(j=0;j<region_names->nfiles;j++) fprintf(fprn,"\t%s",region_names->files[j]); fprintf(fprn,"\n");*/
                /*START150526*/
                for(j1=0;j1<region_names->nfiles;j1++) fprintf(fprn,"\t%s",region_names->files[j1]); fprintf(fprn,"\n");

                }
            for(j=0;j<ad->withprod;j++) {
                if(ad->nwithin) fprintf(fprn,"%s",withtreat->tptr[j]);
                for(kk=ii+j,k=0;k<lenvol_whole;k++,kk+=ad->num_cells_notRANDOM) fprintf(fprn,"\t%f",avg[kk]);
                fprintf(fprn,"\n");
                }
            /*for(j=0;j<ad->withprod;j++) {
                printf("%s",withtreat->tptr[j]);
                for(kk=ii+j,k=0;k<lenvol_whole;k++,kk+=ad->num_cells_notRANDOM) printf("\t%f",avg[kk]);
                printf("\n");
                }*/
            }
        fflush(fprn);
        fclose(fprn);

        //printf("Cellmeans printed to %s\n",cellmeansf);
        //START151008
        printf("Cellmeans printed to %s\n",string);

        }
    fflush(stdout);

    //printf("DONE\n");
    //START160128
    if(!lc_names_only)printf("DONE\n");

    } /*if(!lc_box_correction_only)*/ 
}//ENDMAIN
int cellmeans(AnovaDesign *ad,int nsubjects,CellMeansModel *cmm,FILE *fprn,int lccellmeans,int cellmeansn,int *cellmeansi,int *check)
{
    int i,j,k=0,l,count,address,withprod;
    double sum,sum2=0,sumsq;
    if(cmm) for(i=0;i<cmm->ntreatments;i++) {cmm->uhat[i] = 0.;cmm->n[i] = 0;} 
    for(i=0;i<ad->num_factors;i++) ad->sumcount[i] = 0;
    for(i=0;i<ad->num_sources;i++) ad->bracket[i] = 0;
    for(i=0;i<cellmeansn;i++) {
        if(lccellmeans) pcellheader(cellmeansi[i],fprn,ad);
        for(j=0;j<ad->num_factors;j++) ad->level[j] = 0;
        do {
            sum = sumsq = 0.;
            count = 0;
            l=0;
            do {
                address = offset(ad->level,ad->num_factors,ad->Perlman_levels);
                if(ad->replications[address] && (ad->data[address]!=(float)UNSAMPLED_VOXEL)) {
                    sum += (double)ad->data[address];
                    /*printf("ad->data[%d]=%f\n",address,ad->data[address]);*/
                    count++;
                    sumsq += (double)ad->data[address]*(double)ad->data[address];
                    }
                } while(nextlevel(ad->level,cellmeansi[i],0,ad->num_factors,ad->Perlman_levels));
            sum2 += sumsq;
            if(!(*check) && !cmm) {
                if(!member(RANDOM,cellmeansi[i])) {
                    for(j=1;j<ad->num_factors;j++) {
                        if(member(j,cellmeansi[i]) && ad->nterms[cellmeansi[i]]==1) ad->sumcount[j] += count; /*main effect*/
                        }
                    }
                }
            if(lccellmeans) {
                if(!member(RANDOM,cellmeansi[i])) pcellstats(count,sum,sumsq,fprn,ad,cellmeansi[i]);
                }
            if(count) {
                if(cmm) {
                    cmm->uhat[k] = sum/(double)count;
                    cmm->n[k++] = count;
                    }
                ad->bracket[cellmeansi[i]] += sum*sum/(double)count;
                }
            } while(nextlevel(ad->level,cellmeansi[i],1,ad->num_factors,ad->Perlman_levels));
        }
    if(cmm) {
        cmm->SSWCELL = sum2 - ad->bracket[cellmeansi[0]];
        }
    else {
        withprod = ad->withprod*nsubjects;
        for(i=1;i<ad->num_factors;i++)
            if(ad->sumcount[i] != withprod) {
                printf("WARNING - Unbalanced factor ad->sumcount[%d]=%d withprod=%d Abort!\n",i,ad->sumcount[i],withprod);
                if(fprn) fprintf(fprn,"    WARNING - Unbalanced factor ad->sumcount[%d]=%d withprod=%d Abort!\n\n",
                    i,ad->sumcount[i],withprod);
                return 0;
                }
        }
    return 1;
}
/* check cell size proportions across between factors */
int nonprop(AnovaDesign *ad){
    int i,j,k,fact1,fact2;
    int **count;                /* counts for first row of factor */
    double  coeff;              /* coefficient constant? across rows */

    if(setsize(ad->between,ad->num_factors) <= 1) return 0; /*no possible problems*/
    for(i=0;i<ad->num_sources;i+=2) {
        if(!subset(i,ad->between) || setsize(i,ad->num_factors) != 2) continue;
        for(fact1=fact2=j=0;j<ad->num_factors;j++) {
            ad->level[j] = 0;
            if(member(j,i)) {if(fact1)fact2=j;else fact1=j;}
            }
        if(!(count=d2int(ad->Perlman_levels[fact1],ad->Perlman_levels[fact2])))exit(-1);
        do {
            do {
                if(ad->replications[offset(ad->level,ad->num_factors,ad->Perlman_levels)])
                    count[ad->level[fact1]][ad->level[fact2]]++;
                } while(nextlevel(ad->level,i,0,ad->num_factors,ad->Perlman_levels));
            } while(nextlevel(ad->level,i,1,ad->num_factors,ad->Perlman_levels));

        for(j = 1; j < ad->Perlman_levels[fact1]; j++) {
            coeff = (double) count[j][0] / (double) count[0][0];
            for(k = 1; k < ad->Perlman_levels[fact2]; k++) {
                if(fabs((double)count[j][k]/(double)count[0][k] - coeff) >= (double)FZERO) return 1;
                }
            }
        free_d2int(count);
        }
    return 0;
    }

#if 0
int anova(AnovaDesign *ad,FILE *fprn,int *error_source,double *epsilon1,double *epsilon2,double *dferror,Helmert *h,
    CellMeansModel *cmm,double SSE,int dfSSE,int goose,Classical *c,int classical_WCM_GIGA,Algina *al,int lcGIGA,
    double *chat,SS *ss,int v)
#endif
/*START150526*/
#if 0
int anova(AnovaDesign *ad,FILE *fprn,double *epsilon1,double *epsilon2,double *dferror,Helmert *h,
    CellMeansModel *cmm,double SSE,int dfSSE,int goose,Classical *c,int classical_WCM_GIGA,Algina *al,int lcGIGA,
    double *chat,SS *ss,int v)
#endif
/*START150527*/
int anova(AnovaDesign *ad,FILE *fprn,double *epsilon1,double *epsilon2,double *dferror,Helmert *h,CellMeansModel *cmm,double SSE,
    int dfSSE,int goose,Classical *c,int classical_WCM_GIGA,int lcGIGA,double *chat,SS *ss,int v)

{
    int i,j,k,l,ll,m,kk,n,l1,m1,flag_empty_cells=0,ppp;
    double F,**C,**XTXm1CT,**CXTXm1CT,**CXTXm1CTm1,*Cuhat,CXTXm1CTm1Cuhat,dot;
    gsl_matrix_view gA,gSinv;
    gsl_vector_view gybar,gSinvybar,guhat;
    int start=2,df1start=1;
    if(fprn||cmm) {
        flag_empty_cells=0;
        if(cmm&&!lcGIGA) {
            for(j=0;j<ad->withprod;j++) {
                if(!cmm->n[j]) {
                    printf("Empty cells!\ncmm->n="); for(k=0;k<ad->withprod;k++) printf("%d ",cmm->n[k]); printf("\n");
                    flag_empty_cells=1;
                    break;
                    }
                }
            }
        if(flag_empty_cells) {
            if(fprn) anova_fprn(fprn,goose,ad,classical_WCM_GIGA,cmm,flag_empty_cells,df1start,start,dferror,epsilon1,epsilon2,
                lcGIGA,chat);
            return 1;
            }
        }
    /* If source is odd, then RANDOM will be in its source, so we only want even sources for the main effects. */
    for(kk=df1start,ppp=l1=m1=k=l=ll=0,i=start;i<ad->num_sources;i+=2,k++,kk++) { /*i=0 is the mean*/
        if(!cmm) {
            /* COMPUTE sseffect by adding brackets, alternating signs */
            for(ad->sseffect[k]=0.,j=0;j<=i;j+=2) {
                if(subset(j,i)) ad->sseffect[k] += signedterm(ad->nterms[i],j,ad->num_factors,ad->bracket[j]);
                }
            fflush(stdout);
                    /* the error term for a source factor is WxS/B,
                       where W is the set of all within subjects factors
                       IN THE SOURCE, and B is the set of ALL between
                       subject factors in the WHOLE design. S is the
                       subjects or RANDOM factor.  A bracket term is used
                       in the error term if it includes all between subject
                       factors, and if the only other factors it includes are
                       within subject factors or RANDOM.
                    */
            for(ad->sserror[k]=0.,j=0;j<ad->num_sources;j++) {
                if(subset(j,c->error[kk])&&subset(ad->between,j))
                    ad->sserror[k]+=signedterm(c->nerror[kk],j,ad->num_factors,ad->bracket[j]);
                }
            ad->mseffect[k] = ad->sseffect[k] / (double)ad->dfeffect[kk];

            /*ad->mserror[k] = ad->sserror[k] / (double)dferror[kk];*/
            /*START140404*/
            ad->mserror[k] = ad->sserror[k] / dferror[kk];

            ad->fstat[k] = F = ad->mseffect[k]/ad->mserror[k];
            }

        /*else if(Sstackinv) {*/
        /*START140430*/
        else if(ss) {

            #if 0
            gA = gsl_matrix_view_array(&h->contrasts[l],h->box_dfeffect[k],h->ncol);
            gSinv = gsl_matrix_view_array(&Sstackinv[ll],h->box_dfeffect[k],h->box_dfeffect[k]);
            gybar = gsl_vector_view_array(ss->ybar,h->box_dfeffect[k]);
            gSinvybar = gsl_vector_view_array(ss->Sinvybar,h->box_dfeffect[k]);
            guhat = gsl_vector_view_array(cmm->uhat,h->ncol);
            gsl_blas_dgemv(CblasNoTrans,1.0,&gA.matrix,&guhat.vector,0.0,&gybar.vector);
            gsl_blas_dgemv(CblasNoTrans,1.0,&gSinv.matrix,&gybar.vector,0.0,&gSinvybar.vector);
            gsl_blas_ddot(&gybar.vector,&gSinvybar.vector,&dot);
            ad->fstat[k] = dot/(double)h->box_dfeffect[k]/var;
            #endif
            /*START140410*/
            gA = gsl_matrix_view_array(&h->contrasts[l],h->box_dfeffect[k],h->ncol);
            gSinv = gsl_matrix_view_array(&ss->Sstackinv[ll],h->box_dfeffect[k],h->box_dfeffect[k]);
            gybar = gsl_vector_view_array(ss->ybar,h->box_dfeffect[k]);
            gSinvybar = gsl_vector_view_array(ss->Sinvybar,h->box_dfeffect[k]);
            guhat = gsl_vector_view_array(cmm->uhat,h->ncol);

            //printf("cmm->uhat= ");for(j=0;j<h->ncol;j++)printf("%f ",cmm->uhat[j]);printf("\n");


            gsl_blas_dgemv(CblasNoTrans,1.0,&gA.matrix,&guhat.vector,0.0,&gybar.vector);
            gsl_blas_dgemv(CblasNoTrans,1.0,&gSinv.matrix,&gybar.vector,0.0,&gSinvybar.vector);
            gsl_blas_ddot(&gybar.vector,&gSinvybar.vector,&dot);
            ad->fstat[k] = dot/(double)h->box_dfeffect[k]/ss->eqvar[v];

            #if 0
            //START170302
            printf("dot=%f h->box_dfeffect[%d]=%d ss->eqvar[%d]=%f ad->fstat[%d]=%f\n",dot,k,h->box_dfeffect[k],v,ss->eqvar[v],k,
                ad->fstat[k]);
            #endif

            //ss->fstatuneqvar[k] = dot/(double)h->box_dfeffect[k]/ss->uneqvar[v];
            //START170302
            if(ss->uneqvar)ss->fstatuneqvar[k]=dot/(double)h->box_dfeffect[k]/ss->uneqvar[v];

            /*printf("ad->fstat[%d]=%f ss->fstatuneqvar[%d]=%f\n",k,ad->fstat[k],k,ss->fstatuneqvar[k]);fflush(stdout);*/


            /*printf("dot=%f h->box_dfeffect[%d]=%d var=%f ad->fstat[%d]=%f\n",dot,k,h->box_dfeffect[k],var,k,ad->fstat[k]);
            fflush(stdout);*/

            l+=h->box_dfeffect[k]*h->ncol;
            ll+=h->box_dfeffect[k]*h->box_dfeffect[k];
            }
        else if(classical_WCM_GIGA==1) {
            for(j=0;j<cmm->nind[k];j++) {
                for(n=m=0;m<h->ncol;m++) if(cmm->where[k][m] == j) n += cmm->n[m];
                for(m=0;m<h->ncol;m++) if(cmm->where[k][m] == j) cmm->weight[m] = (double)n;
                }
            for(m=0;m<h->ncol;m++) cmm->weight[m] = (double)cmm->n[m]/cmm->weight[m];
            C = dmatrix(1,h->box_dfeffect[k],1,h->ncol);
            CXTXm1CTm1 = dmatrix(1,h->box_dfeffect[k],1,h->box_dfeffect[k]);
            for(j=0;j<h->box_dfeffect[k];j++,l++) {
                for(m=0;m<h->ncol;m++) {
                    C[j+1][m+1] = h->contrasts[l*h->ncol+m]*cmm->weight[m]; /*WEIGHTING*/
                    /*C[j+1][m+1] = h->contrasts[l*h->ncol+m];*/
                    }
                }
            XTXm1CT = dmatrix(1,h->ncol,1,h->box_dfeffect[k]);
            for(j=0;j<h->box_dfeffect[k];j++) {
                for(m=0;m<h->ncol;m++) {
                    XTXm1CT[m+1][j+1] = C[j+1][m+1]/(double)cmm->n[m];
                    }
                }
            CXTXm1CT = dmatrix_mult(C,XTXm1CT,h->box_dfeffect[k],h->ncol,h->ncol,h->box_dfeffect[k],(int)TRANSPOSE_NONE);
            if(h->box_dfeffect[k] == 1) {
                CXTXm1CTm1 = (double **)dmatrix(1,1,1,1);
                CXTXm1CTm1[1][1] = 1/CXTXm1CT[1][1];
                }
            else if(!(inverse(CXTXm1CT,h->box_dfeffect[k],CXTXm1CTm1))) {
                printf("CXTXm1CT is not invertible. Abort!\n");
                printf("cmm->n="); for(j=0;j<cmm->ntreatments;j++) printf("%d ",cmm->n[j]); printf("\n");
                return 1;
                }
            if(!(Cuhat=malloc(sizeof*Cuhat*h->box_dfeffect[k]))) {
                printf("Error: Unable to malloc Cuhat\n");
                return 1;
                }
            for(j=0;j<h->box_dfeffect[k];j++) {
                Cuhat[j] = 0.;
                for(m=0;m<h->ncol;m++) {

                    /*Cuhat[j] += C[j+1][m+1]*cmm->uhat->ve[m];*/
                    /*START6*/
                    Cuhat[j] += C[j+1][m+1]*cmm->uhat[m];

                    }
                }
            for(ad->sseffect[k]=j=0;j<h->box_dfeffect[k];j++) {
                for(CXTXm1CTm1Cuhat=m=0;m<h->box_dfeffect[k];m++) CXTXm1CTm1Cuhat += CXTXm1CTm1[j+1][m+1] * Cuhat[m];
                ad->sseffect[k] += Cuhat[j]*CXTXm1CTm1Cuhat;
                }
            ad->sserror[k] = cmm->SSWCELL;
            free_dmatrix(C,1,h->box_dfeffect[k],1,h->ncol);
            free_dmatrix(XTXm1CT,1,h->ncol,1,h->box_dfeffect[k]);
            free_dmatrix(CXTXm1CT,1,h->box_dfeffect[k],1,h->box_dfeffect[k]);
            free_dmatrix(CXTXm1CTm1,1,h->box_dfeffect[k],1,h->box_dfeffect[k]);
            free(Cuhat);
            ad->mseffect[k] = ad->sseffect[k] / (double)ad->dfeffect[kk];

            /*ad->mserror[k] = ad->sserror[k] / (double)dferror[kk];*/
            /*START140404*/
            ad->mserror[k] = ad->sserror[k] / dferror[kk];

            ad->fstat[k] = F = ad->mseffect[k]/ad->mserror[k];
            }
        else /*if(classical_WCM_GIGA>=2)*/ {
            printf("SHOULD NOT BE HERE\n");
            fflush(stdout);
            }
        }

    if(fprn) anova_fprn(fprn,goose,ad,classical_WCM_GIGA,cmm,flag_empty_cells,df1start,start,dferror,epsilon1,epsilon2,lcGIGA,
        chat?chat:ad->chat);

    /*KEEP THIS*/
    #if 0
    if(fprn) {
        /*if(ad->replications[0]>1) {*/
        /*if(!cmm && ad->replications[0]>1) {*/
        if(!classical_WCM_GIGA&&ad->replications[0]>1) {
            MSE = SSE/(double)dfSSE;
            /*strptri = ad->fstatfiles;*/
            for(kk=k,k=l=0,i=0;i<ad->num_sources;i+=2,k++,kk++) { /*i=0 is the mean*/
                printf("kk=%d\n",kk);

                MSE = SSE/(double)dfSSE;
                ad->fstat[kk] = F = ad->mserror[k]/MSE;
                df1 = (double)dferror[k];
                df2 = (double)dfSSE;

                /*f_pdfc(&p_uncorrected,&F,1,&df1,&df2);*/
                /*START140722*/
                p_uncorrected=gsl_cdf_fdist_Q(F,df1,df2);

                f_to_z(&F,&zstat_uncorrected,1,&df1,&df2);

                df1 = (double)dferror[k]*epsilon2[k];
                df2 = (double)dfSSE*epsilon2[k];

                /*f_pdfc(&p_box_corrected,&F,1,&df1,&df2);*/
                /*START140722*/
                p_box_corrected=gsl_cdf_fdist_Q(F,df1,df2);

                f_to_z(&F,&zstat_box_corrected,1,&df1,&df2);
                sprintf(zstat_box_corrected_str,"%5.2f",zstat_box_corrected);
                sprintf(p_box_corrected_str,"%10.2g",p_box_corrected);

                fprintf(fprn,"%s%*.4f %4d %9.4f %8.4f %10.2g %5.2f %s %s\n",ad->fstatfilesptr[i+1],
                    ad->max_length-ad->fstatfilesl[i+1]+10,ad->sserror[k],dferror[k],ad->mserror[k],F,p_uncorrected,zstat_uncorrected,
                    p_box_corrected_str,zstat_box_corrected_str);
                }
            fprintf(fprn,"error%*.4f %4d %9.4f\n\n",ad->max_length-5+10,SSE,dfSSE,MSE);
            }
        }
    #endif

    return 0;
}
Helmert *helmert(AnovaDesign *ad,int GIGA,int design)
{
    int i,j,k,m,n,nrow,row,col,ncontrasts_factor,nrepeats0,blocksize,rowloop,nrepeats_pos,negative_value,startrow,index1,index2,
        *box_levels,increment,*box_levels_between=NULL,nrow_between=0;
    double magnitude,*dptr,*dptr2;
    Helmert *h;
    gsl_matrix_view gslA,gslAATstack;
    if(!(h=malloc(sizeof*h))) {
        printf("Error: Unable to malloc h\n");
        return NULL;
        }
    h->Cr=(double**)NULL;
    if(!(h->box_dfeffect=malloc(sizeof*h->box_dfeffect*ad->aa1))) {
        printf("Error: Unable to malloc h->box_dfeffect\n");
        return NULL;
        }
    if(!(box_levels=malloc(sizeof*box_levels*ad->num_factors))) {
        printf("Error: Unable to malloc box_levels\n");
        return NULL;
        }
    if(GIGA) {
        if(!(h->box_dfeffect_between=malloc(sizeof*h->box_dfeffect_between*ad->aa1))) {
            printf("Error: Unable to malloc h->box_dfeffect_between\n");
            return NULL;
            }
        if(!(box_levels_between=malloc(sizeof*box_levels_between*ad->num_factors))) {
            printf("Error: Unable to malloc box_levels_between\n");
            return NULL;
            }
        for(j=0;j<ad->num_factors;j++) box_levels_between[j] = member(j,ad->between) ? ad->Perlman_levels[j]-1 : 1;
        h->ncol_between = ad->betprod;
        }
    if(!GIGA||design) {
        for(j=0;j<ad->num_factors;j++) {
            box_levels[j] = member(j,ad->between) || !strcmp(ad->fnptr[j],"regionfidl") || !strcmp(ad->fnptr[j],"hemispherefidl") ?
                1 : ad->Perlman_levels[j]-1;
            }
        for(h->ncol=i=1;i<ad->num_factors;i++) {
            if(!member(i,ad->between) && strcmp(ad->fnptr[i],"regionfidl") && strcmp(ad->fnptr[i],"hemispherefidl"))
                h->ncol *= ad->Perlman_levels[i];
            }
        }
    else {
        for(j=0;j<ad->num_factors;j++) box_levels[j] = member(j,ad->between) ? 1 : ad->Perlman_levels[j]-1;
        h->ncol = ad->withprod;
        }
    for(k=0,i=2;i<ad->num_sources;i+=2,k++) { /*i=0 source is mean*/
        for(h->box_dfeffect[k]=j=1;j<ad->num_factors;j++) if(member(j,i)) h->box_dfeffect[k] *= box_levels[j];
        }
    for(nrow=i=0;i<ad->aa1;i++) nrow += h->box_dfeffect[i];
    if(!(h->contrasts=malloc(sizeof*h->contrasts*nrow*h->ncol))) {
        printf("Error: Unable to malloc h->contrasts\n");
        return NULL;
        }
    for(i=0;i<nrow*h->ncol;i++) h->contrasts[i]=0.;
    if(GIGA) { 
        for(k=0,i=2;i<ad->num_sources;i+=2,k++) { /*i=0 source is mean*/
            for(h->box_dfeffect_between[k]=j=1;j<ad->num_factors;j++)if(member(j,i))h->box_dfeffect_between[k]*=box_levels_between[j];
            }
        for(nrow_between=i=0;i<ad->aa1;i++) nrow_between += h->box_dfeffect_between[i];
        }
    if(!GIGA||design) {
        for(row=0,ncontrasts_factor=i=1;i<ad->num_factors;i++) {
            for(nrepeats0=1,j=i+1;j<ad->num_factors;j++) {
                if(!member(j,ad->between) && strcmp(ad->fnptr[j],"regionfidl") && strcmp(ad->fnptr[j],"hemispherefidl"))
                    nrepeats0 *= ad->Perlman_levels[j];
                }
            blocksize = ad->Perlman_levels[i]*nrepeats0;
            rowloop = (int)(h->ncol/blocksize);  /*gets truncated to int*/
            nrepeats_pos = nrepeats0;
            negative_value=-1;
            startrow = row;
            for(j=0;j<box_levels[i];j++) {
                if(!member(i,ad->between) && strcmp(ad->fnptr[i],"regionfidl") && strcmp(ad->fnptr[i],"hemispherefidl")) {
                    increment = blocksize - (nrepeats_pos + nrepeats0);
                    for(col=k=0;k<rowloop;k++) {
                        for(m=0;m<nrepeats_pos;m++,col++) h->contrasts[row*h->ncol+col] = 1;
                        for(m=0;m<nrepeats0;m++,col++) h->contrasts[row*h->ncol+col] = (double)negative_value;
                        col += abs(increment);
                        }
                    nrepeats_pos += nrepeats0;
                    negative_value--;
                    }
                else {
                    for(col=0;col<h->ncol;col++) h->contrasts[row*h->ncol+col] = 1;
                    }
                row++;
                }
            for(index1=m=0,j=1;j<ncontrasts_factor;j++,m++) {
                for(k=0;k<h->box_dfeffect[m];k++,index1++) {
                    for(index2=startrow,n=0;n<box_levels[i];n++,index2++) {
                        for(col=0;col<h->ncol;col++) {
                            h->contrasts[row*h->ncol+col] = h->contrasts[index1*h->ncol+col] * h->contrasts[index2*h->ncol+col];
                            }
                        row++;
                        }
                    }
                }
            ncontrasts_factor *= 2;
            startrow = row;
            }
        }
    else {
        for(row=0,ncontrasts_factor=i=1;i<ad->num_factors;i++) {
            for(nrepeats0=1,j=i+1;j<ad->num_factors;j++) if(!member(j,ad->between)) nrepeats0 *= ad->Perlman_levels[j];
            blocksize = ad->Perlman_levels[i]*nrepeats0;
            rowloop = (int)(h->ncol/blocksize);  /*gets truncated to int*/
            nrepeats_pos = nrepeats0;
            negative_value=-1;
            startrow = row;
            for(j=0;j<box_levels[i];j++) {
                if(!member(i,ad->between)) {
                    increment = blocksize - (nrepeats_pos + nrepeats0);
                    for(col=k=0;k<rowloop;k++) {
                        for(m=0;m<nrepeats_pos;m++,col++) h->contrasts[row*h->ncol+col] = 1;
                        for(m=0;m<nrepeats0;m++,col++) h->contrasts[row*h->ncol+col] = (double)negative_value;
                        col += abs(increment);
                        }
                    nrepeats_pos += nrepeats0;
                    negative_value--;
                    }
                else {
                    for(col=0;col<h->ncol;col++) h->contrasts[row*h->ncol+col] = 1;
                    }
                row++;
                }
            for(index1=m=0,j=1;j<ncontrasts_factor;j++,m++) {
                for(k=0;k<h->box_dfeffect[m];k++,index1++) {
                    for(index2=startrow,n=0;n<box_levels[i];n++,index2++) {
                        for(col=0;col<h->ncol;col++) {
                            h->contrasts[row*h->ncol+col] = h->contrasts[index1*h->ncol+col] * h->contrasts[index2*h->ncol+col];
                            }
                        row++;
                        }
                    }
                }
            ncontrasts_factor *= 2;
            startrow = row;
            }
        }
    free(box_levels);
    if(GIGA) {
        if(!(h->Cr=d2double(nrow_between,h->ncol_between))) return NULL;
        if(h->ncol_between==1) {
            for(i=0;i<nrow_between;i++) for(j=0;j<h->ncol_between;j++) h->Cr[i][j]=1.;
            }
        else {
            for(row=0,ncontrasts_factor=i=1;i<ad->num_factors;i++) {
                for(nrepeats0=1,j=i+1;j<ad->num_factors;j++) if(member(j,ad->between)) nrepeats0 *= ad->Perlman_levels[j];
                blocksize = ad->Perlman_levels[i]*nrepeats0;
                rowloop = (int)(h->ncol_between/blocksize);  /*gets truncated to int*/
                nrepeats_pos = nrepeats0;
                negative_value=-1;
                startrow = row;
                for(j=0;j<box_levels_between[i];j++) {
                    if(member(i,ad->between)) {
                        increment = blocksize - (nrepeats_pos + nrepeats0);
                        for(col=k=0;k<rowloop;k++) {
                            for(m=0;m<nrepeats_pos;m++,col++) h->Cr[row][col] = 1;
                            for(m=0;m<nrepeats0;m++,col++) h->Cr[row][col] = (double)negative_value;
                            col += abs(increment);
                            }
                        nrepeats_pos += nrepeats0;
                        negative_value--;
                        }
                    else {
                        for(col=0;col<h->ncol_between;col++) h->Cr[row][col] = 1;
                        }
                    row++;
                    }
                for(index1=m=0,j=1;j<ncontrasts_factor;j++,m++) {
                    for(k=0;k<h->box_dfeffect_between[m];k++,index1++) {
                        for(index2=startrow,n=0;n<box_levels_between[i];n++,index2++) {
                            for(col=0;col<h->ncol_between;col++) {
                                h->Cr[row][col] = h->Cr[index1][col] * h->Cr[index2][col];
                                }
                            row++;
                            }
                        }
                    }
                ncontrasts_factor *= 2;
                startrow = row;
                }
            }
        free(box_levels_between);
        }

    #if 0
    printf("UNNORMALIZED\n");
    printf("within\n");
    for(k=2,row=i=0;i<ad->aa1;i++,k+=2) {
        printf("%s\n",ad->fstatfilesptr[k]);
        for(j=0;j<h->box_dfeffect[i];j++,row++) {
            printf("row=%d  ",row);
            for(col=0;col<h->ncol;col++) printf("%3d ",(int)h->contrasts[row*h->ncol+col]);
            printf("\n");
            }
        }
    #endif
    #if 0
    if(h->Cr) {
        printf("between\n");
        for(k=2,row=i=0;i<ad->aa1;i++,k+=2) {
            printf("%s\n",ad->fstatfilesptr[k]);
            for(j=0;j<h->box_dfeffect_between[i];j++,row++) {
                printf("row=%d  ",row);
                for(col=0;col<h->ncol_between;col++) printf("%3d ",(int)h->Cr[row][col]);
                printf("\n");
                }
            }
        }
    #endif
    #if 1
    for(row=0;row<nrow;row++) {
        for(magnitude=col=0;col<h->ncol;col++) magnitude += h->contrasts[row*h->ncol+col]*h->contrasts[row*h->ncol+col];
        magnitude = sqrt(magnitude);
        for(col=0;col<h->ncol;col++) h->contrasts[row*h->ncol+col] /= magnitude;
        }

    /*START140509*/
    /*printf("h->contrasts\n");
    for(row=0;row<nrow;row++) {
        for(col=0;col<h->ncol;col++) printf("%f ",h->contrasts[row*h->ncol+col]);
        printf("\n");
        }
    fflush(stdout);*/
 

    #endif
    #if 0
    for(row=0;row<nrow_between;row++) {
        for(magnitude=col=0;col<h->ncol_between;col++) magnitude += h->Cr[row][col]*h->Cr[row][col];
        magnitude = sqrt(magnitude);
        for(col=0;col<h->ncol_between;col++) h->Cr[row][col] /= magnitude;
        }
    #endif
    #if 0
    printf("NORMALIZED\n");
    printf("within\n");
    for(k=2,row=i=0;i<ad->aa1;i++,k+=2) {
        printf("%s\n",ad->fstatfilesptr[k]);
        for(j=0;j<h->box_dfeffect[i];j++,row++) {
            for(col=0;col<h->ncol;col++) printf("row=%d col=%d h->contrasts[%d]=%f ",row,col,row*h->ncol+col,
                h->contrasts[row*h->ncol+col]);
            printf("\n");
            }
        }
    #endif
    #if 0
    for(k=2,row=i=0;i<ad->aa1;i++,k+=2) {
        sprintf(filename,"%s_helmert.dat",ad->fstatfilesptr[k]);
        if(!(fp=fopen_sub(filename,"w"))) exit(-1);
        for(j=0;j<h->box_dfeffect[i];j++,row++) {
            for(col=0;col<h->ncol;col++) fprintf(fp,"%f ",h->contrasts[row*h->ncol+col]);
            fprintf(fp,"\n");
            }
        fclose(fp);
        }
    #endif
    #if 0
    printf("between\n");
    for(k=2,row=i=0;i<ad->aa1;i++,k+=2) {
        printf("%s\n",ad->fstatfilesptr[k]);
        for(j=0;j<h->box_dfeffect_between[i];j++,row++) {
            for(col=0;col<h->ncol_between;col++) printf("%f ",h->Cr[row][col]);
            printf("\n");
            }
        }
    #endif


    #if 0
    if(GIGA) {
        h->AAT=m_get(h->ncol,h->ncol);
        if(!(h->AATstack=malloc(sizeof*h->AATstack*ad->aa1*h->ncol*h->ncol))) {
            printf("Error: Unable to malloc h->AATstack\n");
            return 0;
            }
        for(k=2,m=row=i=0;i<ad->aa1;i++,k+=2) {
            /*printf("%s ... calculating AAT\n",ad->fstatfilesptr[k]);*/
            A=m_get(h->box_dfeffect[i],h->ncol);
            for(j=0;j<h->box_dfeffect[i];j++,row++) for(col=0;col<h->ncol;col++) A->me[j][col]=h->contrasts[row*h->ncol+col];
            mtrm_mlt(A,A,h->AAT);
            for(j=0;j<h->ncol;j++) for(col=0;col<h->ncol;col++,m++) h->AATstack[m]=h->AAT->me[j][col];
            if(m_free(A)) {
                printf("Error: m_free(A)\n");
                return NULL;
                }
            }
        if(m_free(h->AAT)) {
            printf("Error: m_free(AAT)\n");
            return NULL;
            }
        #if 1
        printf("h->AATstack\n");
        for(k=2,m=row=i=0;i<ad->aa1;i++,k+=2) {
            printf("%s\n",ad->fstatfilesptr[k]);
            for(j=0;j<h->ncol;j++) {
                for(col=0;col<h->ncol;col++,m++) printf("%f ",h->AATstack[m]);
                printf("\n");
                }
            }
        #endif
        }
    #endif
    /*START56*/
    /*HERE56*/
    #if 1
    if(GIGA) {
        if(!(h->AATstack=malloc(sizeof*h->AATstack*ad->aa1*h->ncol*h->ncol))) {
            printf("Error: Unable to malloc h->AATstack\n");
            return 0;
            }
        for(dptr=h->AATstack,dptr2=h->contrasts,m=row=i=0;i<ad->aa1;dptr+=h->ncol*h->ncol,dptr2+=h->box_dfeffect[i++]*h->ncol) {
            gslA = gsl_matrix_view_array(dptr2,h->box_dfeffect[i],h->ncol);
            gslAATstack = gsl_matrix_view_array(dptr,h->ncol,h->ncol);
            gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,&gslA.matrix,&gslA.matrix,0.0,&gslAATstack.matrix);
            }
        #if 0
        printf("h->AATstack\n");
        for(k=2,m=i=0;i<ad->aa1;i++,k+=2) {
            printf("%s\n",ad->fstatfilesptr[k]);
            for(j=0;j<h->ncol;j++) {
                for(col=0;col<h->ncol;col++,m++) printf("%f ",h->AATstack[m]);
                printf("\n");
                }
            }
        #endif
        }
    #endif



    return h;
}
Helmert *kronecker(AnovaDesign *ad)
{
    int i,j,k,m,n,nrow,row,col,ncontrasts_factor,nrepeats0,blocksize,rowloop,nrepeats_pos,startrow,index1,index2,*box_levels,increment;
    Helmert *h;
    if(!(h=malloc(sizeof*h))) {
        printf("Error: Unable to malloc h\n");
        return NULL;
        }
    h->aa1 = ad->aa1;
    if(!(h->box_dfeffect=malloc(sizeof*h->box_dfeffect*ad->aa1))) {
        printf("Error: Unable to malloc h->box_dfeffect\n");
        return NULL;
        }
    if(!(box_levels=malloc(sizeof*box_levels*ad->num_factors))) {
        printf("Error: Unable to malloc box_levels\n");
        return NULL;
        }
    for(j=1;j<ad->num_factors;j++) box_levels[j] = ad->Perlman_levels[j]-1;
    for(k=0,i=2;i<ad->num_sources;i+=2,k++) { /*i=0 source is mean*/
        h->box_dfeffect[k] = 1;
        for(j=1;j<ad->num_factors;j++) if(member(j,i)) h->box_dfeffect[k] *= box_levels[j];
        }
    for(nrow=i=0;i<ad->aa1;i++) nrow += h->box_dfeffect[i];
    for(h->ncol=i=1;i<ad->num_factors;i++) h->ncol *= ad->Perlman_levels[i];
    #if 0
    printf("h->box_dfeffect= "); for(i=0;i<ad->num_sources/2-1;i++) printf("%d ",h->box_dfeffect[i]); printf("\n");
    printf("nrow=%d h->ncol=%d\n",nrow,h->ncol);
    printf("box_levels= "); for(i=1;i<ad->num_factors;i++) printf("%d ",box_levels[i]); printf("\n");
    #endif
    if(!(h->contrasts=malloc(sizeof*h->contrasts*nrow*h->ncol))) {
        printf("Error: Unable to malloc h->contrasts\n");
        return NULL;
        }
    for(i=0;i<nrow*h->ncol;i++) h->contrasts[i]=0;
    for(row=0,ncontrasts_factor=i=1;i<ad->num_factors;i++) {
        for(nrepeats0=1,j=i+1;j<ad->num_factors;j++) nrepeats0 *= ad->Perlman_levels[j];
        blocksize = ad->Perlman_levels[i]*nrepeats0;
        rowloop = (int)(h->ncol/blocksize);  /*gets truncated to int*/
        nrepeats_pos = nrepeats0;
        startrow = row;
        for(col=j=0;j<box_levels[i];j++,row++) {
            increment = blocksize - (nrepeats_pos + nrepeats0);
            for(col=j*nrepeats0,k=0;k<rowloop;k++) {
                for(m=0;m<nrepeats_pos;m++,col++) h->contrasts[row*h->ncol+col] = 1;
                for(m=0;m<nrepeats0;m++,col++) h->contrasts[row*h->ncol+col] = -1;
                col += abs(increment);
                }
            }
        for(index1=m=0,j=1;j<ncontrasts_factor;j++,m++) {
            for(k=0;k<h->box_dfeffect[m];k++,index1++) {
                for(index2=startrow,n=0;n<box_levels[i];n++,index2++) {
                    for(col=0;col<h->ncol;col++) {
                        h->contrasts[row*h->ncol+col] = h->contrasts[index1*h->ncol+col] * h->contrasts[index2*h->ncol+col];
                        }
                    row++;
                    }
                }
            }
        ncontrasts_factor *= 2;
        startrow = row;
        }

    #if 0
    /*Don't delete this.*/
    printf("UNNORMALIZED\n");
    for(row=i=0;i<ad->num_sources/2-1;i++) {
        printf("%s\n",ad->fstatfiles[i]);
        for(j=0;j<h->box_dfeffect[i];j++,row++) {
            for(col=0;col<h->ncol;col++) printf("%3d ",(int)h->contrasts[row][col]);
            printf("\n");
            }
        }
    #endif
    #if 0
    printf("h->contrasts\n");
    for(k=i=0;i<nrow;i++) {
        for(j=0;j<h->ncol;j++,k++) printf("%f ",h->contrasts[k]);
        printf("\n");
        } 
    exit(-1);
    #endif 

    free(box_levels);
    return h;
}
void cmm_design(AnovaDesign *ad,CellMeansModel *cmm)
{
    int i,j,k,l,m,n,col,rowloop,nrepeats0,value,mult; /*nterms*/

    if(!(cmm->where = d2int(ad->aa1,cmm->ntreatments))) exit(-1);

    for(n=0,l=2;l<ad->num_sources;l+=2,n++) {
        for(value=0,mult=1,i=ad->num_factors;--i>0;) {
            if(member(i,l)) {
                for(nrepeats0=1,j=i+1;j<ad->num_factors;j++) nrepeats0 *= ad->Perlman_levels[j];
                rowloop = (int)(cmm->ntreatments/nrepeats0);  /*gets truncated to int*/
                /*printf("n=%d nrepeats0=%d rowloop=%d\n",n,nrepeats0,rowloop);*/
                for(col=k=0;k<rowloop;k++) {
                    for(m=0;m<nrepeats0;m++,col++) cmm->where[n][col] += value;
                    if(!((k+1)%ad->Perlman_levels[i])) {
                        value = 0;
                        }
                    else {
                        value += mult;
                        }
                    }
                mult *= ad->Perlman_levels[i];
                }
            } 
        }

    #if 0
    /*Don't delete this.*/
    printf("cmm->where\n");
    for(l=0;l<ad->num_sources/2-1;l++) {
        for(i=0;i<cmm->ntreatments;i++) printf("%d ",cmm->where[l][i]); printf("\n");
        }
    #endif


    /*if(!(cmm->nind = d1int(ad->num_sources/2-1))) exit(-1);*/
    if(!(cmm->nind=malloc(sizeof*cmm->nind*(ad->aa1)))) {
        printf("Error: Unable to malloc cmm->nind\n");
        exit(-1);
        }


    for(n=0,l=2;l<ad->num_sources;l+=2,n++) {
        cmm->nind[n] = 1;
        for(i=ad->num_factors;--i>0;) if(member(i,l)) cmm->nind[n] *= ad->Perlman_levels[i]; 
        }
    /*printf("cmm->nind "); for(l=0;l<ad->num_sources/2-1;l++) printf("%d ",cmm->nind[l]); printf("\n");*/
    printf("cmm->nind "); for(l=0;l<ad->aa1;l++) printf("%d ",cmm->nind[l]); printf("\n");
}
void pcellheader(int source,FILE *fprn,AnovaDesign *ad)
{
    int i;
    if(!member(RANDOM,source)) {
        fprintf(fprn,"SOURCE: ");
        if(!source) {
            fprintf(fprn,"grand mean");
            }
        else {
            for(i=1;i<ad->num_factors;i++) if(member(i,source)) fprintf(fprn,"%s ",ad->fnptr[i]);
            }
        fprintf(fprn,"\n");
        for(i=1;i<ad->num_factors;i++) fprintf(fprn,"%-7.7s ",ad->fnptr[i]);
        fprintf(fprn,"   N       MEAN         SD         SE\n");
        }
}
void pcellstats(int count,double sum,double sumsq,FILE *fprn,AnovaDesign *ad,int source)
{
    double sd,se;
    int factor,i;

    for(i=ad->Perlman_levels[0],factor=1;factor<ad->num_factors;i+=ad->Perlman_levels[factor++]) {
        if(member(factor,source)) {
            fprintf(fprn,"%-7.7s ",ad->lnptr[i+ad->level[factor]]);
            }
        else {
            fprintf(fprn,"%-7.7s ", "");
            }
        }
    if(count) {
        fprintf(fprn,"%4d %10.4f ",count,sum/count);
        if(count > 1) {
            sd = sqrt((sumsq-sum*sum/count)/(count-1.0));
            se = sd / sqrt((double)count);
            fprintf(fprn,"%10.4f %10.4f",sd,se);
            }
        }
    else {
        printf("Empty cells are not allowed!");
        exit(-1);
        }
    fprintf(fprn,"\n");
}
double *get_Sstackinv(int j,LinearModel **glmstack,TC *tcs,int tc_contrast,AnovaDesign *ad,Helmert *h,int glmpersub0)
{

    /*size_t i,m,n,m1,n1,nn,o;*/
    /*START150522*/
    int i,m,n,m1,n1,nn,o;

    int k,l,signum,i1,i2,Mcol,nc;
    double **transform,*ATAm1xtransformT,*transformed_ATAm1,*Sstackinv,*Cxtransformed_ATAm1,*S,**ATAm1,**c=NULL,*c1=NULL;
    gsl_permutation *perm;
    for(l=k=i=0;i<ad->aa1;i++) {
        k += h->box_dfeffect[i]*h->box_dfeffect[i];
        if(l<h->box_dfeffect[i]) l = h->box_dfeffect[i];
        }
    if(!(Sstackinv=malloc(sizeof*Sstackinv*k))) {
        printf("fidlError: Unable to malloc Sstackinv\n");
        return NULL;
        }
    for(Mcol=0,i=0;i<glmpersub0;i++) Mcol+=glmstack[i]->ifh->glm_Mcol;
    /*printf("tcs->num_tc=%d Mcol=%d\n",tcs->num_tc,Mcol);fflush(stdout);*/
    if(!(ATAm1=d2double(Mcol,Mcol))) return NULL;
    for(m1=n1=i=0;i<glmpersub0;n1+=glmstack[i++]->ifh->glm_Mcol) {
        /*printf("i=%d m1=%d n1=%d\n",i,m1,n1);fflush(stdout);*/
        for(m=0;m<glmstack[i]->ifh->glm_Mcol;m++,m1++)
            for(nn=n1,n=0;n<glmstack[i]->ifh->glm_Mcol;n++,nn++) ATAm1[m1][nn]=glmstack[i]->ATAm1[m][n];
        } 

    #if 0
    for(i=0;i<glmpersub0;i++) {
        /*printf("i=%d\n",i);*/
        for(m=0;m<glmstack[i]->ifh->glm_Mcol;m++) {
            for(n=0;n<glmstack[i]->ifh->glm_Mcol;n++) printf("%f ",glmstack[i]->ATAm1[m][n]);
            printf("\n");
            }
        }
    printf("ATAm1\n");
    for(m=0;m<Mcol;m++) {
        for(n=0;n<Mcol;n++) printf("%f ",ATAm1[m][n]);
        printf("\n");
        }
    fflush(stdout);
    #endif

    if(!(transform=d2double(tcs->num_tc,Mcol))) return NULL;
    if(!(ATAm1xtransformT=malloc(sizeof*ATAm1xtransformT*Mcol*tcs->num_tc))) {
        printf("fidlError: Unable to malloc ATAm1xtransformT\n");
        return NULL;
        }

    if(!(transformed_ATAm1=malloc(sizeof*transformed_ATAm1*tcs->num_tc*tcs->num_tc))) {
        printf("fidlError: Unable to malloc transformed_ATAm1\n");
        return NULL;
        }
    if(!(Cxtransformed_ATAm1=malloc(sizeof*Cxtransformed_ATAm1*l*tcs->num_tc))) {
        printf("fidlError: Unable to malloc Cxtransformed_ATAm1\n");
        return NULL;
        }
    if(!(S=malloc(sizeof*S*l*l))) {
        printf("fidlError: Unable to malloc S\n");
        return NULL;
        }
    if(tc_contrast) {
        for(nc=0;i=0,i<glmpersub0;i++) nc+=glmstack[i]->ifh->glm_nc;
        if(!(c=d2double(nc,Mcol))) return NULL;
        for(m1=n1=i=0;i<glmpersub0;n1+=glmstack[i++]->ifh->glm_Mcol) {
            for(o=m=0;m<glmstack[i]->ifh->glm_nc;m++,m1++)
                for(nn=n1,n=0;n<glmstack[i]->ifh->glm_Mcol;n++,nn++,o++) c[m1][nn]=glmstack[i]->c[o];
            } 
        c1=c[0]; 
        }
    get_transform_gsl(tcs,tc_contrast,j,ATAm1,Mcol,c1,transform);
    gsl_matrix_view gATAm1 = gsl_matrix_view_array(ATAm1[0],Mcol,Mcol);
    gsl_matrix_view gtransform = gsl_matrix_view_array(transform[0],tcs->num_tc,Mcol);
    gsl_matrix_view gATAm1xtransformT = gsl_matrix_view_array(ATAm1xtransformT,Mcol,tcs->num_tc);
    gsl_matrix_view gtransformed_ATAm1 = gsl_matrix_view_array(transformed_ATAm1,tcs->num_tc,tcs->num_tc);
    gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,&gATAm1.matrix,&gtransform.matrix,0.0,&gATAm1xtransformT.matrix);
    gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,&gtransform.matrix,&gATAm1xtransformT.matrix,0.0,&gtransformed_ATAm1.matrix);
    for(i1=i2=k=0,i=2;i<ad->num_sources;i+=2,k++) {
        gsl_matrix_view gC = gsl_matrix_view_array(&h->contrasts[i1],h->box_dfeffect[k],h->ncol);
        gsl_matrix_view gCxtransformed_ATAm1 = gsl_matrix_view_array(Cxtransformed_ATAm1,h->box_dfeffect[k],tcs->num_tc);
        gsl_matrix_view gS = gsl_matrix_view_array(S,h->box_dfeffect[k],h->box_dfeffect[k]);
        gsl_matrix_view gSinv = gsl_matrix_view_array(&Sstackinv[i2],h->box_dfeffect[k],h->box_dfeffect[k]);
        gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,&gC.matrix,&gtransformed_ATAm1.matrix,0.0,&gCxtransformed_ATAm1.matrix);
        gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,&gCxtransformed_ATAm1.matrix,&gC.matrix,0.0,&gS.matrix);
        perm = gsl_permutation_alloc(h->box_dfeffect[k]);
        gsl_linalg_LU_decomp(&gS.matrix,perm,&signum);
        gsl_linalg_LU_invert(&gS.matrix,perm,&gSinv.matrix);
        i1 += h->box_dfeffect[k]*h->ncol;
        i2 += h->box_dfeffect[k]*h->box_dfeffect[k];
        gsl_permutation_free(perm);
        }
    if(tc_contrast) free_d2double(c);
    free(S);
    free(Cxtransformed_ATAm1);
    free(transformed_ATAm1);
    free(ATAm1xtransformT);
    free_d2double(transform);
    free_d2double(ATAm1);
    return Sstackinv;
}







Classical *classical(AnovaDesign *ad)
{
    int i,ii,error;
    Classical *c;
    if(!(c=malloc(sizeof*c))) {
        printf("Error: Unable to malloc c\n");
        return NULL;
        }
    if(!(c->error=malloc(sizeof*c->error*ad->aa))) {
        printf("Error: Unable to malloc c->error\n");
        return NULL;
        }
    if(!(c->nerror=malloc(sizeof*c->nerror*ad->aa))) {
        printf("Error: Unable to malloc c->nerror\n");
        return NULL;
        }
    for(ii=i=0;i<ad->num_sources;i+=2,ii++) { /*i=0 is the mean*/
        error = 0;
        enter(RANDOM,error); /*random factor always in the error term*/
        error = join(error,ad->between);
        error = join(error,i);
        c->nerror[ii] = setsize(error,ad->num_factors);
        c->error[ii] = error; 
        }
    return c;
}
Algina *algina_init(AnovaDesign *ad,Helmert *h){
    int i,j,k,l,m,ii,kk,ll,*alwhichgroupi,*alntreatpersub,*aladdress,address,index1,index2,index3;
    Algina *al;
    if(!(al=malloc(sizeof*al))) {
        printf("Error: Unable to malloc al\n");
        return NULL;
        }
    if(!(al->G=malloc(sizeof*al->G*ad->num_cells_notRANDOM*ad->num_cells_notRANDOM))) {
        printf("Error: Unable to malloc al->G\n");
        return NULL;
        }
    al->gslG = gsl_matrix_view_array(al->G,ad->num_cells_notRANDOM,ad->num_cells_notRANDOM);
    if(!(al->Sstar=malloc(sizeof*al->Sstar*ad->num_cells_notRANDOM*ad->num_cells_notRANDOM))) {
        printf("Error: Unable to malloc al->Sstar\n");
        return NULL;
        }
    al->gslSstar = gsl_matrix_view_array(al->Sstar,ad->num_cells_notRANDOM,ad->num_cells_notRANDOM);
    if(!(al->GSstar=malloc(sizeof*al->GSstar*ad->num_cells_notRANDOM*ad->num_cells_notRANDOM))) {
        printf("Error: Unable to malloc al->GSstar\n");
        return NULL;
        }
    al->gslGSstar = gsl_matrix_view_array(al->GSstar,ad->num_cells_notRANDOM,ad->num_cells_notRANDOM);
    for(j=i=0;i<ad->aa1;i++) if(h->box_dfeffect[i]>j) j=h->box_dfeffect[i];
    if(!(al->ATs=malloc(sizeof*al->ATs*j*h->ncol))) {
        printf("Error: Unable to malloc al->ATs\n");
        return NULL;
        }
    if(!(al->ATsA=malloc(sizeof*al->ATsA*j*j))) {
        printf("Error: Unable to malloc al->ATsA\n");
        return NULL;
        }

    //for(j=i=0;i<ad->aa1;i++) if(h->box_dfeffect_between[i]>j) j=h->box_dfeffect[i];
    //START151209
    for(j=i=0;i<ad->aa1;i++) if(h->box_dfeffect_between[i]>j) j=h->box_dfeffect_between[i];

    if(!(al->CrT=malloc(sizeof*al->CrT*j*h->ncol_between))) {
        printf("Error: Unable to malloc al->CrT\n");
        return NULL;
        }

    //START151209
    if(!(al->Cr=malloc(sizeof*al->Cr*j*h->ncol_between))) {
        printf("Error: Unable to malloc al->Cr\n");
        return NULL;
        }


    if(!(al->CrTNm1=malloc(sizeof*al->CrTNm1*j*h->ncol_between))) {
        printf("Error: Unable to malloc al->CrTNm1\n");
        return NULL;
        }
    if(!(al->CrTNm1Cr=malloc(sizeof*al->CrTNm1Cr*j*j))) {
        printf("Error: Unable to malloc al->CrTNm1Cr\n");
        return NULL;
        }

    //START151201
    //if(!(al->temp_double=malloc(sizeof*al->temp_double*j*j))) {
    //    printf("fidlError: Unable to malloc al->temp_double\n");
    //    return NULL;
    //    }
    //if(!(al->temp_double2=malloc(sizeof*al->temp_double2*j*j))) {
    //    printf("fidlError: Unable to malloc al->temp_double2\n");
    //    return NULL;
    //    }
    if(!(al->V=malloc(sizeof*al->V*j*j))) {
        printf("fidlError: Unable to malloc al->V\n");
        return NULL;
        }
    if(!(al->Sgsl=malloc(sizeof*al->Sgsl*j))) {
        printf("fidlError: Unable to malloc al->Sgsl\n");
        return NULL;
        }
    if(!(al->work=malloc(sizeof*al->work*j))) {
        printf("fidlError: Unable to malloc al->work\n");
        return NULL;
        }
    #if 0
    //START151208
    //if(!(al->temp_double=malloc(sizeof*al->temp_double*j*h->ncol_between))) {
    //    printf("fidlError: Unable to malloc al->temp_double\n");
    //    return NULL;
    //    }
    //if(!(al->temp_double2=malloc(sizeof*al->temp_double2*j*j))) {
    //    printf("fidlError: Unable to malloc al->temp_double2\n");
    //    return NULL;
    //    }
    if(!(al->V=malloc(sizeof*al->V*h->ncol_between*h->ncol_between))) {
        printf("fidlError: Unable to malloc al->V\n");
        return NULL;
        }
    if(!(al->Sgsl=malloc(sizeof*al->Sgsl*h->ncol_between))) {
        printf("fidlError: Unable to malloc al->Sgsl\n");
        return NULL;
        }
    if(!(al->work=malloc(sizeof*al->work*h->ncol_between))) {
        printf("fidlError: Unable to malloc al->work\n");
        return NULL;
        }
    #endif



    if(!(al->CrCrTNm1Crm1=malloc(sizeof*al->CrCrTNm1Crm1*j*h->ncol_between))) {
        printf("Error: Unable to malloc al->CrCrTNm1Crm1\n");
        return NULL;
        }
    if(!(al->CrCrTNm1Crm1CrT=malloc(sizeof*al->CrCrTNm1Crm1CrT*h->ncol_between*h->ncol_between))) {
        printf("Error: Unable to malloc al->CrCrTNm1Crm1CrT\n");
        return NULL;
        }
    al->gslCrCrTNm1Crm1CrT = gsl_matrix_view_array(al->CrCrTNm1Crm1CrT,h->ncol_between,h->ncol_between);
    if(!(al->scalarAAT=malloc(sizeof*al->scalarAAT*h->ncol*h->ncol))) {
        printf("Error: Unable to malloc al->scalarAAT\n");
        return NULL;
        }
    al->gslscalarAAT = gsl_matrix_view_array(al->scalarAAT,h->ncol,h->ncol);

    if(!(al->Sp=malloc(sizeof*al->Sp*h->ncol*h->ncol))) {
        printf("Error: Unable to malloc al->Sp\n");
        return NULL;
        }
    al->gslSp=gsl_matrix_view_array(al->Sp,h->ncol,h->ncol);
    if(!(al->S=malloc(sizeof*al->S*h->ncol*h->ncol))) {
        printf("Error: Unable to malloc al->S\n");
        return NULL;
        }
    if(!(al->s=malloc(sizeof*al->s*h->ncol*h->ncol))) {
        printf("Error: Unable to malloc al->s\n");
        return NULL;
        }
    if(!(al->vec=malloc(sizeof*al->vec*h->ncol))) {
        printf("Error: Unable to malloc al->vec\n");
        return NULL;
        }
    if(!(al->sstack=malloc(sizeof*al->sstack*h->ncol_between*h->ncol*h->ncol))) {
        printf("Error: Unable to malloc al->sstack\n");
        return NULL;
        }
    if(!(al->ag=malloc(sizeof*al->ag*h->ncol_between))) {
        printf("Error: Unable to malloc al->ag\n");
        return NULL;
        }
    if(!(al->bg=malloc(sizeof*al->bg*h->ncol_between))) {
        printf("Error: Unable to malloc al->bg\n");
        return NULL;
        }
    if(!(al->n=malloc(sizeof*al->n*h->ncol_between))) {
        printf("Error: Unable to malloc al->n\n");
        return NULL;
        }

    if(!(al->nold=malloc(sizeof*al->nold*h->ncol_between))) {
        printf("Error: Unable to malloc al->nold\n");
        return NULL;
        }
    for(j=0;j<h->ncol_between;j++) al->nold[j]=-1;

    if(!(al->nsubpergroup=malloc(sizeof*al->nsubpergroup*h->ncol_between))) {
        printf("Error: Unable to malloc al->nsubpergroup\n");
        exit(-1);
        }
    for(j=0;j<h->ncol_between;j++) al->nsubpergroup[j]=0;
    if(!(al->uhat=malloc(sizeof*al->uhat*ad->withprod))) {
        printf("Error: Unable to malloc al->uhat\n");
        exit(-1);
        }
    if(!(al->uhat_rearranged=malloc(sizeof*al->uhat_rearranged*ad->num_cells_notRANDOM))) {
        printf("Error: Unable to malloc al->uhat\n");
        exit(-1);
        }
    al->gsluhat_rearranged = gsl_vector_view_array(al->uhat_rearranged,ad->num_cells_notRANDOM);

    /*al->gslVecXbarCrCrTNm1Crm1CrTkronprodAAT = gsl_vector_alloc(ad->num_cells_notRANDOM);*/
    /*START56*/
    if(!(al->VecXbarCrCrTNm1Crm1CrTkronprodAAT=malloc(sizeof*al->VecXbarCrCrTNm1Crm1CrTkronprodAAT*ad->num_cells_notRANDOM))) {
        printf("Error: Unable to malloc al->VecXbarCrCrTNm1Crm1CrTkronprodAAT\n");
        exit(-1);
        }
    al->gslVecXbarCrCrTNm1Crm1CrTkronprodAAT = gsl_vector_view_array(al->VecXbarCrCrTNm1Crm1CrTkronprodAAT,ad->num_cells_notRANDOM);

    if(!(al->addressord=malloc(sizeof*al->addressord*ad->Perlman_levels[0]*ad->withprod))) {
        printf("Error: Unable to malloc al->addressord\n");
        exit(-1);
        }
    if(!(al->data=malloc(sizeof*al->data*ad->Perlman_levels[0]*ad->withprod))) {
        printf("Error: Unable to malloc al->data\n");
        exit(-1);
        }
    if(!(alwhichgroupi=malloc(sizeof*alwhichgroupi*ad->Perlman_levels[0]*ad->withprod))) {
        printf("Error: Unable to malloc alwhichgroupi\n");
        exit(-1);
        }
    if(!(aladdress=malloc(sizeof*aladdress*ad->Perlman_levels[0]*ad->withprod))) {
        printf("Error: Unable to malloc aladdress\n");
        exit(-1);
        }
    for(j=0;j<ad->Perlman_levels[0]*ad->withprod;j++) aladdress[j]=-1;
    if(!(alntreatpersub=malloc(sizeof*alntreatpersub*ad->Perlman_levels[0]))) {
        printf("Error: Unable to malloc alntreatpersub\n");
        exit(-1);
        }
    for(j=0;j<ad->Perlman_levels[0];j++) alntreatpersub[j] = 0;
    l=0;
    for(j=0;j<ad->num_factors;j++) ad->level[j] = 0;
    ll=0;
    do {
        do {
            address = offset(ad->level,ad->num_factors,ad->Perlman_levels);
            if(ad->replications[address]) {
                for(k=1,index1=0,j=ad->nbetween;--j>=0;k*=ad->Perlman_levels[ad->betweeni[j]]) index1+=ad->level[ad->betweeni[j]]*k;
                for(k=1,index2=0,j=ad->nwithin;--j>=0;k*=ad->Perlman_levels[ad->withini[j]]) index2+=ad->level[ad->withini[j]]*k;
                index3 = ad->level[(int)RANDOM]*ad->withprod+index2;
                aladdress[index3]=address;
                alwhichgroupi[index3]=index1;
                alntreatpersub[ad->level[(int)RANDOM]]++;
                l++;
                #if 0
                printf("ad->level="); for(j=0;j<ad->num_factors;j++) printf("%d ",ad->level[j]);
                printf("address=%d\n",address);
                printf("    index1=%d index2=%d index3=%d\n",index1,index2,index3);
                printf("    alwhichgroupi[%d]=%d\n",index3,alwhichgroupi[index3]);
                #endif
                }
            } while(nextlevel(ad->level,ad->num_sources-2,0,ad->num_factors,ad->Perlman_levels));
        } while(nextlevel(ad->level,ad->num_sources-2,1,ad->num_factors,ad->Perlman_levels));
    /*printf("aladdress\n");
    for(k=i=0;i<ad->Perlman_levels[0];i++) for(j=0;j<ad->withprod;j++,k++) printf("%d ",aladdress[k]); printf("\n");*/
    if(l!=ad->Perlman_levels[0]*ad->withprod) {
        for(j=0;j<ad->Perlman_levels[0];j++) {
            if(alntreatpersub[j]!=ad->withprod) {
                printf("Subject %d is missing treatments. Will be excluded from the analysis.\n",j+1);
                for(ii=j*ad->withprod,i=0;i<ad->withprod;i++,ii++) aladdress[ii]=-1;
                }
            }
        }
    /*for(k=j=0;j<ad->Perlman_levels[0];j++)
        for(i=0;i<ad->withprod;i++,k++) printf("alwhichgroupi[%d]=%d\n",k,alwhichgroupi[k]);*/
    for(k=j=0;j<ad->Perlman_levels[0];j++) {
        if(aladdress[k]!=-1) {
            for(kk=k++,i=1;i<ad->withprod;i++,k++) {
                if(alwhichgroupi[k]!=alwhichgroupi[kk]) {
                    printf("Error: alwhichgroupi[%d]=%d alwhichgroupi[%d]=%d  Each subject can only belong to one group.\n",
                        k,alwhichgroupi[k],kk,alwhichgroupi[kk]);
                    return NULL;
                    }
                }
            }
        }
//STARTHERE160506
    for(m=l=j=0;j<ad->betprod;j++) {
        for(k=i=0;i<ad->Perlman_levels[0];i++,k+=ad->withprod) {
            if(aladdress[k]!=-1) {
                if(alwhichgroupi[k]==j) {
                    al->nsubpergroup[j]++;
                    for(ll=k,l=0;l<ad->withprod;l++,ll++,m++) al->addressord[m]=aladdress[ll];
                    }
                }
            }
        }
    free(alwhichgroupi);
    free(aladdress);
    free(alntreatpersub);
    return al;
}
int algina(Algina *al,AnovaDesign *ad,Helmert *h){
    int i,j,k,l,m,n,o,kk,ll,nsubjects,count,flag;
    double td1,td2;
    for(i=0;i<h->ncol*h->ncol;i++) al->Sp[i]=al->S[i]=0.;
    for(nsubjects=o=ll=m=i=0;i<ad->betprod;i++) {
        for(j=0;j<h->ncol*h->ncol;j++) al->s[j]=0.;     
        for(j=0;j<ad->withprod;j++) al->uhat[j]=0.;
        for(count=n=j=0;j<al->nsubpergroup[i];j++,m+=ad->withprod) {
            for(kk=m,flag=k=0;k<ad->withprod;k++,kk++) if(ad->data[al->addressord[kk]]==(float)UNSAMPLED_VOXEL) {flag=1; break;}
            if(!flag) {
                for(kk=m,k=0;k<ad->withprod;k++,kk++,n++) al->uhat[k]+=al->data[n]=(double)ad->data[al->addressord[kk]];
                count++;
                nsubjects++;
                }
            }
        if(count<2) return 1;
        al->n[i]=count;
        for(j=0;j<ad->withprod;j++,o++) {
            al->uhat[j]/=(double)count;
            al->uhat_rearranged[o]=al->uhat[j];
            }
        for(n=j=0;j<count;j++) {
            for(k=0;k<ad->withprod;k++,n++) al->vec[k]=al->data[n]-al->uhat[k];
            for(kk=k=0;k<h->ncol;k++) for(l=0;l<h->ncol;l++,kk++) al->s[kk]+=al->vec[k]*al->vec[l];
            }
        for(kk=j=0;j<ad->withprod;j++) for(k=0;k<ad->withprod;k++,ll++,kk++) al->sstack[ll]=al->s[kk];
        td1=(double)count/(double)(count-1);
        for(j=0;j<h->ncol*h->ncol;j++) {al->Sp[j]+=al->s[j];al->S[j]+=td1*al->s[j];}
        }
    td1=1./(double)(nsubjects-ad->betprod);
    td2=1./(double)(nsubjects);
    for(i=0;i<h->ncol*h->ncol;i++) {al->Sp[i]*=td1;al->S[i]*=td2;}
    return 0;
    }
int algina_guts_anova(AnovaDesign *ad,Helmert *h,int nsubjects,int *nsubpergroup,double *sstack,Algina *al,int lcGstackonly,
    int classical_WCM_GIGA,Anovas *anovas){
    int oo,jj,j,m,pp,o,kk,l1,k,l,i,mm,p,n1,n,ppp,nn,flag_empty_cells=0,start=2,df1start=1,lcalgina_guts=1,ndep,signum,status;
    double NminusG,r,trGSstar,trGSstar2,chatnum,chatden,chat,etahat,deltahat,htildaprimeprime,hhatprimeprime,htilda,dferror,c1,c2,c3,
        td,SSN,trATSpA,cond_norm2; //cond,condmax=10000.
    gsl_matrix_view gslCrT,gslCrTNm1,gslCrTNm1Cr,gslCrCrTNm1Crm1,gslAAT,gslA,gslATs,gslATsA,gsls;
    gsl_vector_view gslcol,gslrow; //gslb,gslx
    gsl_permutation *perm;
    gsl_set_error_handler_off();
    NminusG=(double)(nsubjects-ad->betprod);
    if(lcalgina_guts) {
        for(oo=mm=jj=j=0;j<h->ncol_between;j++,jj+=h->ncol) {
            for(m=0;m<h->ncol;m++,oo++) {
                for(pp=jj,o=0;o<h->ncol;o++,pp++,mm++) {

                    //printf("sstack[%d]=%f nsubpergroup[%d]=%d\n",mm,sstack[mm],j,nsubpergroup[j]);

                    gsl_matrix_set(&al->gslSstar.matrix,oo,pp,(double)(sstack[mm]/(nsubpergroup[j]-1)/nsubpergroup[j]));
                    }
                }
            }
        }



    for(kk=df1start,ppp=l1=k=l=0,i=start;i<ad->num_sources;i+=2,k++,kk++) { /*i=0 is the mean*/
        if(lcalgina_guts) {
            r=(double)h->box_dfeffect_between[k];

            //printf("here0 i=%d h->box_dfeffect_between[%d]=%d r=%f\n",i,k,h->box_dfeffect_between[k],r);fflush(stdout);


            gslCrT = gsl_matrix_view_array(al->CrT,h->box_dfeffect_between[k],h->ncol_between);
            gslCrTNm1 = gsl_matrix_view_array(al->CrTNm1,h->box_dfeffect_between[k],h->ncol_between);
            gslCrTNm1Cr = gsl_matrix_view_array(al->CrTNm1Cr,h->box_dfeffect_between[k],h->box_dfeffect_between[k]);
            gslCrCrTNm1Crm1 = gsl_matrix_view_array(al->CrCrTNm1Crm1,h->box_dfeffect_between[k],h->ncol_between);
            gslAAT = gsl_matrix_view_array(&h->AATstack[k*h->ncol*h->ncol],h->ncol,h->ncol);


            #if 0
            if(ad->within_effects[i]) {
                for(jj=j=0;j<h->box_dfeffect_between[k];j++,l1++,jj+=h->ncol_between) {
                    for(mm=jj,m=0;m<h->ncol_between;m++,mm++) {
                        al->CrT[mm] = al->CrCrTNm1Crm1[mm] = h->Cr[l1][m]*nsubpergroup[m]/nsubjects;
                        al->CrTNm1[mm] = h->Cr[l1][m]/nsubjects;
                        }
                    }
                }
            else {
                for(jj=j=0;j<h->box_dfeffect_between[k];j++,l1++,jj+=h->ncol_between) {
                    for(mm=jj,m=0;m<h->ncol_between;m++,mm++) {
                        al->CrT[mm] = al->CrCrTNm1Crm1[mm] = h->Cr[l1][m];
                        al->CrTNm1[mm] = h->Cr[l1][m]/nsubpergroup[m];
                        }
                    }
                }
            #endif
            //START151209
            #if 1
            if(ad->within_effects[i]) {
                for(mm=j=0;j<h->box_dfeffect_between[k];j++,l1++) {
                    for(m=0;m<h->ncol_between;m++,mm++) {
                        al->CrT[mm] = al->CrCrTNm1Crm1[mm] = h->Cr[l1][m]*nsubpergroup[m]/nsubjects;
                        al->CrTNm1[mm] = h->Cr[l1][m]/nsubjects;
                        }
                    }
                }
            else {
                for(mm=j=0;j<h->box_dfeffect_between[k];j++,l1++) {
                    for(m=0;m<h->ncol_between;m++,mm++) {
                        al->CrT[mm] = al->CrCrTNm1Crm1[mm] = h->Cr[l1][m];
                        al->CrTNm1[mm] = h->Cr[l1][m]/nsubpergroup[m];
                        }
                    }
                }
            #endif

            #if 0
            printf("before al->CrT\n");
            for(mm=j=0;j<h->box_dfeffect_between[k];j++) {
                for(m=0;m<h->ncol_between;m++,mm++)printf("%f ",al->CrT[mm]); 
                printf("\n");
                }
            printf("before gslCrT\n");
            for(j=0;j<h->box_dfeffect_between[k];j++) {
                for(m=0;m<h->ncol_between;m++)printf("%f ",gsl_matrix_get(&gslCrT.matrix,j,m)); 
                printf("\n");
                }
            #endif

            gsl_matrix_view gslCr = gsl_matrix_view_array(al->Cr,h->ncol_between,h->box_dfeffect_between[k]);
            if((status=gsl_matrix_transpose_memcpy(&gslCr.matrix,&gslCrT.matrix))){
                printf("fidlError: gsl_matrix_transpose_memcpy %s\n",gsl_strerror(status));fflush(stdout);
                }

            #if 0
            printf("after al->CrT\n");
            for(mm=j=0;j<h->box_dfeffect_between[k];j++) {
                for(m=0;m<h->ncol_between;m++,mm++)printf("%f ",al->CrT[mm]); 
                printf("\n");
                }
            printf("after gslCrT\n");
            for(j=0;j<h->box_dfeffect_between[k];j++) {
                for(m=0;m<h->ncol_between;m++)printf("%f ",gsl_matrix_get(&gslCrT.matrix,j,m)); 
                printf("\n");
                }
            printf("after al->Cr\n");
            for(mm=m=0;m<h->ncol_between;m++){
                for(j=0;j<h->box_dfeffect_between[k];j++,mm++)printf("%f ",al->Cr[mm]);
                printf("\n");
                }
            #endif


            //printf("m=%d n=%d\n",h->ncol_between,h->box_dfeffect_between[k]);
            gsl_svd_golubreinsch(al->Cr,h->ncol_between,h->box_dfeffect_between[k],0.,al->V,al->Sgsl,(double*)NULL,
                &cond_norm2,&ndep,al->work);
            //printf("al->Sgsl=");for(j=0;j<h->ncol_between;j++)printf("%f ",al->Sgsl[j]);printf("\n");
            //printf("%s ndep=%d\n",ad->fstatfilesptr[i],ndep);
            if(!anovas)printf("%s ndep=%d\n",ad->fstatfilesptr[i],ndep);


            gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,&gslCrTNm1.matrix,&gslCrT.matrix,0.0,&gslCrTNm1Cr.matrix);
            if(h->box_dfeffect_between[k]==1) {  
                gsl_matrix_scale(&gslCrCrTNm1Crm1.matrix,1./al->CrTNm1Cr[0]);
                }
            else {

                #if 0
                gsl_linalg_cholesky_decomp(&gslCrTNm1Cr.matrix);
                for(j=0;j<h->ncol_between;j++) {
                    gslcol = gsl_matrix_column(&gslCrCrTNm1Crm1.matrix,j);
                    gsl_linalg_cholesky_svx(&gslCrTNm1Cr.matrix,&gslcol.vector);
                    }
                #endif
                /*START140502*/
                #if 0
                perm = gsl_permutation_alloc(h->box_dfeffect_between[k]);
                gsl_linalg_LU_decomp(&gslCrTNm1Cr.matrix,perm,&signum);
                for(j=0;j<h->ncol_between;j++) {
                    gslcol = gsl_matrix_column(&gslCrCrTNm1Crm1.matrix,j);
                    gsl_linalg_LU_svx(&gslCrTNm1Cr.matrix,perm,&gslcol.vector);
                    }
                gsl_permutation_free(perm);
                #endif
                #if 1
                //START151201
                perm = gsl_permutation_alloc(h->box_dfeffect_between[k]);
                if((status=gsl_linalg_LU_decomp(&gslCrTNm1Cr.matrix,perm,&signum))){
                    printf("fidlError: gsl_linalg_LU_decomp %s\n",gsl_strerror(status));fflush(stdout);
                    }
                for(j=0;j<h->ncol_between;j++) {
                    gslcol = gsl_matrix_column(&gslCrCrTNm1Crm1.matrix,j);
                    if((status=gsl_linalg_LU_svx(&gslCrTNm1Cr.matrix,perm,&gslcol.vector))){
                        printf("fidlError: gsl_linalg_LU_svx %s\n",gsl_strerror(status));fflush(stdout);
                        }
                    }
                gsl_permutation_free(perm);
                #endif

                #if 0
                //START151201
                perm = gsl_permutation_alloc(h->box_dfeffect_between[k]);
                for(j=0;j<h->box_dfeffect_between[k]*h->box_dfeffect_between[k];j++)
                    al->temp_double2[j]=al->temp_double[j]=al->CrTNm1Cr[j];
                if(cond_norm1(al->temp_double,h->box_dfeffect_between[k],&cond,al->CrTNm1Cr,perm))cond=0.;
                gsl_permutation_free(perm);
                printf("Condition number norm1: %f\n",cond);
                if(cond>condmax||cond==0.){
                    td=gsl_svd_golubreinsch(al->temp_double2,h->box_dfeffect_between[k],h->box_dfeffect_between[k],0.,al->V,al->Sgsl,
                        al->CrTNm1Cr,&cond_norm2,&ndep,al->work);
                    printf("gsl_svd_golubreinsch cond_norm2=%f ndep=%d tol=%g\n",cond_norm2,ndep,td);
                    }
                gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,&gslCrT.matrix,&gslCrTNm1Cr.matrix,0.0,&gslCrCrTNm1Crm1.matrix);
                #endif

                #if 0
                //START151203
                for(j=0;j<h->box_dfeffect_between[k]*h->box_dfeffect_between[k];j++)al->temp_double[j]=al->CrTNm1Cr[j];
                perm=gsl_permutation_alloc(h->box_dfeffect_between[k]);
                if((status=gsl_linalg_LU_decomp(&gslCrTNm1Cr.matrix,perm,&signum))){
                    //printf("fidlError: gsl_linalg_LU_decomp %s\n",gsl_strerror(status));fflush(stdout);
                    }
                else {
                    for(j=0;j<h->ncol_between;j++) {
                        gslcol = gsl_matrix_column(&gslCrCrTNm1Crm1.matrix,j);
                        if((status=gsl_linalg_LU_svx(&gslCrTNm1Cr.matrix,perm,&gslcol.vector))){
                            //printf("fidlError: gsl_linalg_LU_svx %s\n",gsl_strerror(status));fflush(stdout);
                            break;
                            }
                        }
                    }
                if(status){
                    //printf("fidlInfo: Computing singular value decomposition\n");
                    //printf("Computing singular value decomposition\n");
                    gslCrTNm1Cr = gsl_matrix_view_array(al->temp_double,h->box_dfeffect_between[k],h->box_dfeffect_between[k]);
                    gsl_matrix_view gV = gsl_matrix_view_array(al->V,h->box_dfeffect_between[k],h->box_dfeffect_between[k]);
                    gsl_vector_view gS = gsl_vector_view_array(al->Sgsl,h->box_dfeffect_between[k]);
                    gsl_vector_view gwork = gsl_vector_view_array(al->work,h->box_dfeffect_between[k]);
                    if((status=gsl_linalg_SV_decomp(&gslCrTNm1Cr.matrix,&gV.matrix,&gS.vector,&gwork.vector))){
                        //printf("fidlError: gsl_linalg_SV_decomp %s\n",gsl_strerror(status));fflush(stdout);
                        }
                    else{
             
                        //printf("al->Sgsl=");for(j=0;j<h->box_dfeffect_between[k];j++)printf("%f ",al->Sgsl[j]);printf("\n");

                        for(j=0;j<h->ncol_between;j++) {
                            gslb = gsl_matrix_column(&gslCrT.matrix,j);
                            gslx = gsl_matrix_column(&gslCrCrTNm1Crm1.matrix,j);
                            if((status=gsl_linalg_SV_solve(&gslCrTNm1Cr.matrix,&gV.matrix,&gS.vector,&gslb.vector,&gslx.vector))){
                                //printf("fidlError: gsl_linalg_SV_solve %s\n",gsl_strerror(status));fflush(stdout);
                                break;
                                }
                            }
                        }
                    }
                gsl_permutation_free(perm);
                //printf("status=%d\n",status);
                #endif


                }

            gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,&gslCrCrTNm1Crm1.matrix,&gslCrT.matrix,0.0,&al->gslCrCrTNm1Crm1CrT.matrix);
            for(n=jj=j=0;j<h->ncol_between;j++,jj+=h->ncol) {
                for(mm=m=0;m<h->ncol_between;m++,mm+=h->ncol,n++) {
                    gsl_matrix_memcpy(&al->gslscalarAAT.matrix,&gslAAT.matrix);
                    gsl_matrix_scale(&al->gslscalarAAT.matrix,al->CrCrTNm1Crm1CrT[n]);
                    /*printf("al->CrCrTNm1Crm1CrT[%d]=%f\n",n,al->CrCrTNm1Crm1CrT[n]);*/
                    for(oo=jj,nn=o=0;o<h->ncol;o++,oo++) {
                        for(pp=mm,p=0;p<h->ncol;p++,pp++,ppp++,nn++) {
                            gsl_matrix_set(&al->gslG.matrix,oo,pp,al->scalarAAT[nn]);
                            //printf("al->gslG[%d][%d]=%f\n",oo,pp,gsl_matrix_get(&al->gslG.matrix,oo,pp));
                            }
                        }
                    }
                }
            if(!lcGstackonly) {
                gslA = gsl_matrix_view_array(&h->contrasts[l],h->box_dfeffect[k],h->ncol);
                gslATs = gsl_matrix_view_array(al->ATs,h->box_dfeffect[k],h->ncol);
                gslATsA = gsl_matrix_view_array(al->ATsA,h->box_dfeffect[k],h->box_dfeffect[k]);
                #if 0
                printf("gslA\n");
                for(m=0;m<h->box_dfeffect[k];m++) {
                    for(n=0;n<h->ncol;n++) printf("%f ",gsl_matrix_get(&gslA.matrix,m,n)); 
                    printf("\n");
                    }
                #endif

                gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,&al->gslG.matrix,&al->gslSstar.matrix,0.0,&al->gslGSstar.matrix);
                for(trGSstar=trGSstar2=0.,j=0;j<ad->num_cells_notRANDOM;j++) {
                    trGSstar+=gsl_matrix_get(&al->gslGSstar.matrix,j,j);
                    gslrow = gsl_matrix_row(&al->gslGSstar.matrix,j);
                    gslcol = gsl_matrix_column(&al->gslGSstar.matrix,j);                
                    gsl_blas_ddot(&gslrow.vector,&gslcol.vector,&td);
                    trGSstar2+=td;
                    }
                chatnum=trGSstar*NminusG;
                //printf("here1 trGSstar=%f NminusG=%f chatnum=%f\n",trGSstar,NminusG,chatnum);
                for(etahat=deltahat=chatden=0.,n1=j=0;j<ad->betprod;j++,n1+=h->ncol*h->ncol) {
                    gsls = gsl_matrix_view_array(&sstack[n1],h->ncol,h->ncol);
                    gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,&gslA.matrix,&gsls.matrix,0.0,&gslATs.matrix);
                    gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,&gslATs.matrix,&gslA.matrix,0.0,&gslATsA.matrix);
                    #if 0
                    printf("gsls\n");
                    for(m=0;m<h->ncol;m++) {
                        for(n=0;n<h->ncol;n++) printf("%f ",gsl_matrix_get(&gsls.matrix,m,n)); 
                        printf("\n");
                        }
                    printf("gslATsA\n");
                    for(m=0;m<h->box_dfeffect[k];m++) {
                        for(n=0;n<h->box_dfeffect[k];n++) printf("%f ",gsl_matrix_get(&gslATsA.matrix,m,n)); 
                        printf("\n");
                        }
                    #endif
                    for(al->ag[j]=al->bg[j]=0.,m=0;m<h->box_dfeffect[k];m++) {
                        al->ag[j]+=gsl_matrix_get(&gslATsA.matrix,m,m);
                        gslrow = gsl_matrix_row(&gslATsA.matrix,m);
                        gslcol = gsl_matrix_column(&gslATsA.matrix,m);
                        gsl_blas_ddot(&gslrow.vector,&gslcol.vector,&td);
                        al->bg[j]+=td;
                        }
                    /*printf("al->ag[%d]=%f\n",j,al->ag[j]);*/
                    chatden+=al->ag[j];
                    c1=1./(((double)nsubpergroup[j]+1.)*((double)nsubpergroup[j]-2.));
                    c2=(double)nsubpergroup[j]-1.;
                    c3=al->ag[j]*al->ag[j];
                    etahat+=c1*c2*((double)nsubpergroup[j]*c3-2.*al->bg[j]);
                    deltahat+=c1*(c2*al->bg[j]-c3);
                    }
                chatden*=r;
                //printf("here2 chatden=%f\n",chatden);
                chat=chatnum/chatden;
                //printf("chatnum=%f chatden=%f chat=%f r=%f\n",chatnum,chatden,chat,r);
                for(c1=0.,j=0;j<ad->betprod-1;j++) for(m=j+1;m<ad->betprod;m++) c1+=al->ag[j]*al->ag[m];
                etahat+=2.*c1;
                /*printf("etahat=%f deltahat=%f\n",etahat,deltahat);
                printf("h->box_dfeffect[%d]=%d NminusG=%f\n",k,h->box_dfeffect[k],NminusG);*/
                if(classical_WCM_GIGA==2) {
                    if((htilda=etahat/deltahat)>(dferror=(double)(h->box_dfeffect[k]*NminusG))) {
                        /*if(fprn) printf("htilda=%f > dferror=%f  htilda will be set to dferror\n",htilda,dferror);*/
                        htilda=dferror;
                        }
                    }
                else {
                    for(c1=c2=0.,j=0;j<ad->betprod;j++) {
                        c1+=al->ag[j];
                        c2+=al->bg[j]/(double)(nsubpergroup[j]-1);
                        }
                    htilda=c1*c1/c2;
                    }
                if(h->box_dfeffect[k]==1&&h->box_dfeffect_between[k]==1) {
                    htildaprimeprime=1.;
                    }
                else {
                    hhatprimeprime=trGSstar*trGSstar/trGSstar2;
                    htildaprimeprime = hhatprimeprime;
                    if(classical_WCM_GIGA==2) {
                        if(NminusG*r>h->box_dfeffect[k]) {
                            if((htildaprimeprime=r*((NminusG+1.)*hhatprimeprime-2.*r)/(NminusG*r-hhatprimeprime))>
                                h->box_dfeffect[k]) {
                                htildaprimeprime=h->box_dfeffect[k];
                                }
                            }
                        }
                    }
                ad->chat[k]=chat;
                ad->htildaprimeprime[k]=htildaprimeprime;
                ad->htilda[k]=htilda;
                /*printf("chat=%f ad->chat[%d]=%f htildaprimeprime=%f htilda=%f\n",chat,k,ad->chat[k],htildaprimeprime,htilda);*/
                }
            } 
        if(anovas) {
            r=(double)h->box_dfeffect_between[k];
            gsl_blas_dgemv(CblasNoTrans,1.0,&al->gslG.matrix,&al->gsluhat_rearranged.vector,0.0,
                &al->gslVecXbarCrCrTNm1Crm1CrTkronprodAAT.vector);
            gsl_blas_ddot(&al->gslVecXbarCrCrTNm1Crm1CrTkronprodAAT.vector,&al->gsluhat_rearranged.vector,&SSN);
            gslA = gsl_matrix_view_array(&h->contrasts[l],h->box_dfeffect[k],h->ncol);
            gslATs = gsl_matrix_view_array(al->ATs,h->box_dfeffect[k],h->ncol);
            gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,&gslA.matrix,&al->gslSp.matrix,0.0,&gslATs.matrix);

            #if 0 
            printf("h->contrasts[%d]\n",l);
            for(jj=j=0;j<h->box_dfeffect[k];j++) {
                for(m=0;m<h->ncol;m++,jj++) printf("%f ",h->contrasts[l+jj]);
                printf("\n");
                }
            printf("gslA\n");
            for(m=0;m<h->box_dfeffect[k];m++) {
                for(n=0;n<h->ncol;n++) printf("%f ",gsl_matrix_get(&gslA.matrix,m,n));
                printf("\n");
                }
            printf("al->ATs\n");
            for(jj=j=0;j<h->box_dfeffect[k];j++) {
                for(m=0;m<h->ncol;m++,jj++) printf("%f ",al->ATs[jj]);
                printf("\n");
                }
            printf("\n");fflush(stdout);
            #endif

            for(trATSpA=0.,p=l,jj=j=0;j<h->box_dfeffect[k];j++,jj+=h->ncol) {
                for(m=0;m<h->ncol;m++) trATSpA+=al->ATs[jj+m]*h->contrasts[p+jj+m];
                }
            ad->fstat[k]=SSN/r/trATSpA;
            /*printf("r=%f trATSpA=%f SSN=%f ad->fstat[%d]=%f\n",r,trATSpA,SSN,k,ad->fstat[k]);fflush(stdout);*/
            ad->sseffect[k]=SSN;
            ad->mseffect[k] = ad->sseffect[k] / (double)ad->dfeffect[kk];
            ad->sserror[k]=NminusG*trATSpA;

            /*printf("here1 ad->sserror[%d]=%f NminusG=%f trATSpA=%f\n",k,ad->sserror[k],NminusG,trATSpA);fflush(stdout);*/

            ad->mserror[k] = trATSpA / (double)ad->dfeffect[kk];

            /*START140509*/
            /*l+=h->box_dfeffect[k]*h->ncol;*/

            }

        /*START140509*/
        l+=h->box_dfeffect[k]*h->ncol;


        }
    if(anovas) {
        if(anovas->fprn) {
            anova_fprn(anovas->fprn,anovas->goose,ad,classical_WCM_GIGA,anovas->cmm,flag_empty_cells,df1start,start,
                anovas->dferror,anovas->epsilon1,anovas->epsilon2,anovas->lcGIGA,!lcGstackonly?ad->chat:anovas->chatv);
            }
        }
    return 0;
}
/*END*/

void anova_fprn(FILE *fprn,int goose,AnovaDesign *ad,int classical_WCM_GIGA,CellMeansModel *cmm,int flag_empty_cells,int df1start,
    int start,double *dferror,double *epsilon1,double *epsilon2,int lcGIGA,double *chat) {
    char string[MAXNAME],zstat_box_corrected_str[6],p_box_corrected_str[11],p_uncorrected_str[50];
    int i,k,kk,leaveblank=0,status=0,work; /* max_length,len,lenmax=0 */
    size_t k1,max_length,len,lenmax=0;
    double df1,df2,p_uncorrected,zstat_uncorrected,zstat_box_corrected,p_box_corrected,fstat;
    if(!ad->datafiles&&classical_WCM_GIGA==0&&lcGIGA==1) leaveblank=1;
    if(!flag_empty_cells) {
        for(lenmax=k=0,i=start;i<ad->num_sources;i+=2,k++) { /*i=0 is the mean*/
            sprintf(string,"%.4f",ad->sseffect[k]);
            if((len=strlen(string))>lenmax) lenmax=len;
            sprintf(string,"%.4f",ad->sserror[k]);
            if((len=strlen(string))>lenmax) lenmax=len;
            }
        }
    if(classical_WCM_GIGA==0) strcpy(string,"classical");
    else if(classical_WCM_GIGA==1) strcpy(string,"weighted cell means");
    else if(classical_WCM_GIGA==2) strcpy(string,"Algina");
    else strcpy(string,"Algina (design matrix)");
    max_length=ad->max_length;
    if((len=strlen(string))>ad->max_length) max_length=len;
    if(goose) fprintf(fprn,"%*s%s\n",(int)(max_length+lenmax+4+lenmax+8+5+12),classical_WCM_GIGA==2?"corrected":"uncorrected",
        ad->datafiles&&classical_WCM_GIGA!=2?"    SPH corrected":"");
    if(classical_WCM_GIGA==3) {
        fprintf(fprn,"%-*s %*s   df %*s        F            p        z%s\n",(int)max_length,string,(int)lenmax,"SS",(int)lenmax,"MS",
            "        p       z");
        }
    else {
        fprintf(fprn,"%-*s %*s   df %*s        F            p        z%s\n",(int)max_length,string,(int)lenmax,"SS",(int)lenmax,"MS",
            cmm||!ad->datafiles?"":"        p       z");
        }
    for(k1=0;k1<ad->max_length;k1++) fprintf(fprn,"=");
    fprintf(fprn,"=============================================%s%s\n",cmm&&classical_WCM_GIGA==1?"":"=================",
        classical_WCM_GIGA!=2?"":"===============");
    if(flag_empty_cells) {
        fprintf(fprn,"Empty cells!\n");
        }
    else {
        for(kk=df1start,k=0,i=start;i<ad->num_sources;i+=2,k++,kk++) { /*i=0 is the mean*/
            if(isnan(ad->fstat[k])) {
                fprintf(fprn,"%s%*.4f %4d %8.4f %8.4f\n",ad->fstatfilesptr[i],(int)(ad->max_length-ad->fstatfilesl[k]+9),
                    ad->sseffect[k],ad->dfeffect[kk],ad->mseffect[k],ad->fstat[k]);
                }
            else {
                if(classical_WCM_GIGA==2) {
                    df1 = ad->htildaprimeprime[k];
                    df2 = ad->htilda[k];
                    fstat = ad->fstat[k]/chat[k];
                    printf("*************************************GIGA**************************************\n");
                    printf("    df1=%f df2=%f chat=%f fstat=%f\n",df1,df2,chat[k],fstat);
                    }
                else {
                    df1 = (double)ad->dfeffect[kk];
                    df2 = dferror[kk];
                    fstat = ad->fstat[k];
                    }
                p_uncorrected=gsl_cdf_fdist_Q(fstat,df1,df2);
                f_to_z(&fstat,&zstat_uncorrected,1,&df1,&df2,&work);
                zstat_box_corrected_str[0]=p_box_corrected_str[0]=0;
                if(p_uncorrected == (double)UNSAMPLED_VOXEL) {
                    sprintf(p_uncorrected_str,"%12s",gsl_strerror(status));
                    }
                else {
                    sprintf(p_uncorrected_str,"%12g",p_uncorrected);
                    }
                if(classical_WCM_GIGA==2) {
                    fprintf(fprn,"%-*s %*.4f %4d %*.4f %8.4f %s\n",(int)max_length,ad->fstatfilesptr[i],(int)lenmax,
                        ad->sseffect[k],ad->dfeffect[kk],(int)lenmax,ad->mseffect[k],fstat,p_uncorrected_str);
                    }
                else {
                    if(ad->datafiles) {
                        fprintf(fprn,"epsilon1[%d]=%f epsilon2[%d]=%f",kk,epsilon1[kk],kk,epsilon2[kk]);
                        if(classical_WCM_GIGA==3) fprintf(fprn," scalar[%d]=%f",kk,chat[kk]);
                        fprintf(fprn,"\n");
                        }
                    if(epsilon1[kk]==(double)UNSAMPLED_VOXEL||epsilon2[kk]==(double)UNSAMPLED_VOXEL) {
                        leaveblank=1;
                        }
                    else {
                        df1 *= epsilon1[kk];
                        df2 *= epsilon2[kk];
                        if(classical_WCM_GIGA==3) fstat/=chat[kk];
                        f_to_z(&fstat,&zstat_box_corrected,1,&df1,&df2,&work);
                        p_box_corrected=gsl_cdf_fdist_Q(fstat,df1,df2);
                        if(p_box_corrected == (double)UNSAMPLED_VOXEL) {
                            sprintf(p_uncorrected_str,"%10s",gsl_strerror(status));
                            }
                        else if(zstat_box_corrected>zstat_uncorrected && chat[kk]>=1.) {
                            strcpy(zstat_box_corrected_str," ----");
                            strcpy(p_box_corrected_str,"  --------");
                            }
                        else {
                            sprintf(zstat_box_corrected_str,"%7.6f",zstat_box_corrected);
                            sprintf(p_box_corrected_str,"%10.6g",p_box_corrected);
                            }
                        }
                    fprintf(fprn,"%-*s %*.4f %4d %*.4f %8.4f %s %7.6f %s %s",(int)max_length,ad->fstatfilesptr[i],(int)lenmax,
                        ad->sseffect[k],ad->dfeffect[kk],(int)lenmax,ad->mseffect[k],ad->fstat[k],p_uncorrected_str,zstat_uncorrected,
                        !p_box_corrected_str[0]?"":p_box_corrected_str,!zstat_box_corrected_str[0]?"":zstat_box_corrected_str);
                    if(classical_WCM_GIGA==2) fprintf(fprn," %.2f  %.4f",ad->htildaprimeprime[k],1./chat[k]);
                    fprintf(fprn,"\n");
                    }
                }
            fprintf(fprn,"%-*s %*.4f %4d %*.4f",(int)max_length,cmm&&classical_WCM_GIGA<2?"error":ad->fstatfilesptr[i+1],(int)lenmax,
                ad->sserror[k],(int)dferror[kk],(int)lenmax,ad->mserror[k]);
            fprintf(fprn,"\n\n");
            }
        }
    fflush(fprn);
    }
Treats *get_treatment_str(AnovaDesign *ad,int nidx,int *idx){
    char *strptr;
    int i,j,k,l,index,value,len,*treat,*driver_table;
    Treats *treats=NULL;
    if(!(treat=malloc(sizeof*treat*(nidx+1)))) {
        printf("fidlError: get_treatment_str Unable to malloc treat\n");
        return 0;
        }
    treat[nidx]=1;
    treat[nidx-1]=ad->Perlman_levels[idx[nidx-1]];
    for(i=nidx-1;--i>=0;) treat[i] = treat[i+1]*ad->Perlman_levels[idx[i]];
    //for(i=0;i<nidx+1;i++) printf("treat[%d]=%d\n",i,treat[i]);
    if(!(driver_table=malloc(sizeof*driver_table*treat[0]*nidx))) {
        printf("fidlError: get_treatment_str Unable to malloc driver_table\n");
        return 0;
        }
    for(i=nidx;--i>=0;) {
        for(index=i,value=j=0;j<treat[0]/treat[i+1];j++) {
            for(k=0;k<treat[i+1];k++,index+=nidx) driver_table[index] = value;
            if((++value)>(ad->Perlman_levels[idx[i]]-1)) value=0;
            } 
        }
    /*for(i=0;i<treat[0]*nidx;i++) printf("driver_table[%d]=%d\n",i,driver_table[i]);*/
    for(len=k=i=0;i<treat[0];i++) {
        for(j=0;j<nidx;j++,k++) {
            for(index=l=0;l<idx[j];l++) index+=ad->Perlman_levels[l];
            len += strlen(ad->fnptr[idx[j]])+1+strlen(ad->lnptr[index+driver_table[k]])+1;
            }
        }
    if(!(treats=malloc(sizeof*treats))) {
        printf("fidlError: get_treatment_str Unable to malloc treats\n");
        return NULL;
        }
    treats->n=treat[0];
    if(!(treats->tptr=malloc(sizeof*treats->tptr*treat[0]))) {
        printf("fidlError: Unable to malloc treats->tptr\n");
        return NULL;
        }
    if(!(treats->t=malloc(sizeof*treats->t*len))) {
        printf("fidlError: get_treatment_str Unable to malloc treats->t\n");
        return NULL;
        }
    *treats->t=0;
    for(strptr=treats->t,k=i=0;i<treat[0];i++) {
        treats->tptr[i]=strptr;
        for(j=0;j<nidx;j++,k++) {
            for(index=l=0;l<idx[j];l++) index+=ad->Perlman_levels[l]; 
            strcat(strptr,ad->fnptr[idx[j]]);
            strcat(strptr,".");
            strcat(strptr,ad->lnptr[index+driver_table[k]]);
            strcat(strptr,j<nidx-1?" ":"\0");
            }
        if(i<(treat[0]-1))strptr++;
        }
    free(driver_table);
    free(treat);
    return treats;
    }
void free_algina(Algina *al){
    free(al->data);
    free(al->addressord);
    free(al->VecXbarCrCrTNm1Crm1CrTkronprodAAT);
    free(al->uhat_rearranged);
    free(al->uhat);
    free(al->nsubpergroup);
    free(al->nold);
    free(al->n);
    free(al->bg);
    free(al->ag);
    free(al->sstack);
    free(al->vec); 
    free(al->s);
    free(al->S);
    free(al->Sp);
    free(al->scalarAAT);
    free(al->CrCrTNm1Crm1CrT);
    free(al->CrCrTNm1Crm1);

    //START151201
    free(al->work);
    free(al->Sgsl);
    free(al->V);
    free(al->temp_double2);
    free(al->temp_double);


    free(al->CrTNm1Cr);
    free(al->CrTNm1);
    free(al->CrT);
    free(al->ATsA);
    free(al->ATs);
    free(al->G);
    free(al->Sstar);
    free(al->GSstar);
    free(al);
    }
int statview_guts(int lenvol,Algina *al,AnovaDesign *ad,char *regional_anova_name){
    int i,j,k,l,m,n,ii,jj,kk,ll,mm;
    int maxnsubpergroup,withprod_before,withprod_after,restn,jump,regionfidli,regionfidln=0,*betprodptr=NULL,lnptri;
    double *temp_double; //td
    Memory_Map *map;
    FILE *fp;
    if(lenvol!=1) {
        printf("Error: Currently only set up for lenvol equal to 1.\n");
        return 0;
        }
    for(maxnsubpergroup=i=0;i<ad->betprod;i++) if(al->nsubpergroup[i]>maxnsubpergroup) maxnsubpergroup=al->nsubpergroup[i]; 
    if(!(temp_double=malloc(sizeof*temp_double*ad->num_cells_notRANDOM*maxnsubpergroup))) {
        printf("Error: Unable to malloc temp_double\n");
        exit(-1);
        }
    for(i=0;i<ad->num_cells_notRANDOM*maxnsubpergroup;i++) temp_double[i] = (double)UNSAMPLED_VOXEL; 
    gsl_matrix_view gsltemp_double = gsl_matrix_view_array(temp_double,maxnsubpergroup,ad->num_cells_notRANDOM);

    /*if(!(fp = fopen_sub("fidl_check.dat","w"))) exit(-1);*/
    for(i=0;i<ad->num_datafiles;i++) {

        #if 0
        if(!(map = map_disk(ad->datafiles[i],lenvol,0,sizeof(float)))) exit(-1);
        td=(double)map->ptr[0];
        #ifdef __sun__
          if(IsNANorINF(td)) {
        #else
          if(isnan(td)||isinf(td)) {
        #endif
            ad->data[ad->address[i]]=(float)UNSAMPLED_VOXEL;
            }
        else {
            ad->data[ad->address[i]] = map->ptr[0];
            }
        unmap_disk(map);
        #endif
        //START161028
        if(ad->NA[i])ad->data[ad->address[i]]=(float)UNSAMPLED_VOXEL;
        else{
            if(!(map=map_disk(ad->datafiles[i],lenvol,0,sizeof(float))))return 0; 
            ad->data[ad->address[i]]=!isfinite(map->ptr[0])?(float)UNSAMPLED_VOXEL:map->ptr[0];
            unmap_disk(map);
            }

        }

    /*fclose(fp);*/
    /*printf("\n");*/
    /*if(!(fp = fopen_sub("fidl_check2.dat","w"))) exit(-1);*/
    for(n=m=i=0;i<ad->betprod;i++) {
        for(j=0;j<al->nsubpergroup[i];j++,m+=ad->withprod) {
            for(kk=m,k=0;k<ad->withprod;k++,kk++,n++) {
                al->data[n] = ad->data[al->addressord[kk]]==(float)UNSAMPLED_VOXEL ? (double)UNSAMPLED_VOXEL : 
                    (double)ad->data[al->addressord[kk]];
                /*fprintf(fp,"i=%d j=%d k=%d %f\n",i,j,k,al->data[n]);*/
                /*printf("al->data[%d]=%f ",n,al->data[n]);*/
                }
            }
        }
    /*fclose(fp);*/
    /*printf("\n");*/
    for(lnptri=ad->Perlman_levels[0],regionfidli=-1,withprod_after=withprod_before=i=1;i<ad->num_factors;i++) {
        if(!ad->within_between[i]) {
            if(!strcmp("regionfidl",ad->fnptr[i])) {
                regionfidli = i;
                regionfidln = ad->Perlman_levels[i];
                }
            else if(regionfidli==-1) {
                withprod_before *= ad->Perlman_levels[i];
                lnptri+=ad->Perlman_levels[i];
                }
            else {
                withprod_after *= ad->Perlman_levels[i];
                }
            }
        }
    if(regionfidli==-1) {
        printf("Error: regionfidl not found.\n");
        return 0;
        }
    restn = ad->withprod/regionfidln;

    jump = withprod_after*regionfidln;
    /*printf("withprod_after=%d withprod_before=%d jump=%d ad->betprod=%d\n",withprod_after,withprod_before,jump,ad->betprod);*/
    for(jj=ii=i=0;i<ad->betprod;i++,ii+=ad->withprod) {
        for(j=0;j<al->nsubpergroup[i];j++,jj+=ad->withprod) {
            for(mm=kk=k=0;k<regionfidln;k++) {
                for(ll=l=0;l<withprod_before;l++,ll+=jump) {
                    for(m=0;m<withprod_after;m++,mm++) {
                        gsl_matrix_set(&gsltemp_double.matrix,j,ii+mm,al->data[jj+kk+ll+mm]);
                        }
                    }
                }
            }
        }
    if(!(fp = fopen_sub(regional_anova_name,"w"))) exit(-1);
    if(ad->nbetween) {
        if(!(betprodptr=malloc(sizeof*betprodptr*ad->betprod))) {
            printf("Error: Unable to malloc betprodptr\n");
            return 0;
            }
        for(m=i=0;i<ad->nbetween;i++) {
            for(l=j=0;j<ad->betweeni[i];j++) l+=ad->Perlman_levels[j];
            for(j=0;j<ad->Perlman_levels[ad->betweeni[i]];j++,m++) betprodptr[m] = l+j;
            }
        for(k=0;k<ad->betprod;k++) for(i=0;i<ad->withprod;i++) fprintf(fp,"%s\t",ad->lnptr[betprodptr[k]]); 
        fprintf(fp,"\n");
        }
    for(k=0;k<ad->betprod;k++) for(i=0;i<regionfidln;i++) for(j=0;j<restn;j++) fprintf(fp,"%s\t",ad->lnptr[lnptri+i]);
    fprintf(fp,"\n");
    for(kk=ad->Perlman_levels[0],k=1;k<ad->num_factors;kk+=ad->Perlman_levels[k++]) {
        for(ii=j=1;j<k;j++) ii *= ad->Perlman_levels[j];
        if(k==regionfidli || ad->betweenf[k]) continue;
        for(jj=1,j=k+1;j<ad->num_factors;j++) {
            if(j==regionfidli || ad->betweenf[j]) continue;
            jj *= ad->Perlman_levels[j];
            }
        for(m=0;m<ad->betprod;m++) {
            for(l=0;l<ii;l++) for(j=0;j<ad->Perlman_levels[k];j++) for(i=0;i<jj;i++) fprintf(fp,"%s\t",ad->lnptr[kk+j]);
            }
        fprintf(fp,"\n");
        }

    for(k=i=0;i<maxnsubpergroup;i++) {
        for(j=0;j<ad->num_cells_notRANDOM;j++,k++) {
            temp_double[k]==(double)UNSAMPLED_VOXEL?fprintf(fp,"\t"):fprintf(fp,"%f\t",temp_double[k]);
            }
        fprintf(fp,"\n");
        }
    fclose(fp);
    printf("Statview file written to %s\n",regional_anova_name);
    if(ad->nbetween) free(betprodptr);
    free(temp_double);
    return 1;
}
SS *init_ss(Helmert *h){
    int i,max;
    SS *ss;
    for(max=i=0;i<h->aa1;i++) if(h->box_dfeffect[i]>max) max=h->box_dfeffect[i];
    if(!(ss=malloc(sizeof*ss))) {
        printf("Error: Unable to malloc ss\n");
        return NULL;
        }
    if(!(ss->ybar=malloc(sizeof*ss->ybar*max))) {
        printf("Error: Unable to malloc ss->ybar\n");
        return NULL;
        }
    if(!(ss->Sinvybar=malloc(sizeof*ss->Sinvybar*max))) {
        printf("Error: Unable to malloc ss->ybar\n");
        return NULL;
        }
    return ss;
    }
LME *init_lme(int adaa0,char *scratchdir,char *output){
    LME *lme;
    char *driver="rdriver.dat",*script="rscript.r",*data0="rdata0.dat",*data="rdata.dat",*datao="rdata.4dfp.img";

    //size_t len=strlen(scratchdir)+strlen(output);
    //START180130
    size_t len=strlen(scratchdir)+(output?strlen(output):0);

    if(!(lme=malloc(sizeof*lme))) {
        printf("fidlError: Unable to malloc lme\n");
        return NULL;
        }
    if(!(lme->rdriver=malloc(sizeof*lme->rdriver*(len+strlen(driver)+1)))) {
        printf("fidlError: Unable to malloc lme->rdriver\n");
        return NULL;
        }
    sprintf(lme->rdriver,"%s%s%s",scratchdir,output?output:"",driver);
    //printf("lme->rdriver=%s\n",lme->rdriver);
    if(!(lme->rscript=malloc(sizeof*lme->rscript*(len+strlen(script)+1)))) {
        printf("fidlError: Unable to malloc lme->rscript\n");
        return NULL;
        }
    sprintf(lme->rscript,"%s%s%s",scratchdir,output?output:"",script);
    //printf("lme->rscript=%s\n",lme->rscript);
    if(!(lme->rdata0=malloc(sizeof*lme->rdata0*(len+strlen(data0)+1)))) {
        printf("fidlError: Unable to malloc lme->rdata0\n");
        return NULL;
        }
    sprintf(lme->rdata0,"%s%s%s",scratchdir,output?output:"",data0);
    if(!(lme->rdata=malloc(sizeof*lme->rdata*(len+strlen(data)+1)))) {
        printf("fidlError: Unable to malloc lme->rdata\n");
        return NULL;
        }
    sprintf(lme->rdata,"%s%s%s",scratchdir,output?output:"",data);
    if(!(lme->rdatao=malloc(sizeof*lme->rdatao*(len+strlen(datao)+1)))) {
        printf("fidlError: Unable to malloc lme->rdatao\n");
        return NULL;
        }
    sprintf(lme->rdatao,"%s%s%s",scratchdir,output?output:"",datao);
    if(adaa0){
        if(!(lme->lmerp=malloc(sizeof*lme->lmerp*(adaa0+1)))){
            printf("fidlError: Unable to malloc lme->lmerp\n");
            return NULL;
            }
        if(!(lme->lmerl=malloc(sizeof*lme->lmerl*(adaa0+1)))){
            printf("fidlError: Unable to malloc lme->lmerl\n");
            return NULL;
            }
        }
    return lme;
    }

#if 0
int get_rdriver(char *driver_file,char *rdriver,int nstr) {
    int i1,nlines=0,nstr0;
    char line[MAXNAME],*write_back_ptr,*strptr;
    FILE *fp,*op;
    if(!(fp=fopen_sub(driver_file,"r"))) return 0;
    if(!(op=fopen_sub(rdriver,"w"))) return 0;
    for(;fgets(line,sizeof(line),fp);) {
        for(nstr0=0,strptr=line;strtok_r(strptr," 	",&strptr);nstr0++);
        if(nstr0>=nstr){
            for(strptr=line,i1=0;i1<nstr;i1++,strptr++) {
                write_back_ptr=strtok_r(strptr," 	",&strptr);
                fprintf(op,"%s%s",write_back_ptr,(i1<nstr-1)?"\t":"\n");
                }
            nlines++;
            } 
        }
    fclose(op);
    fclose(fp);
    return nlines;
    }
#endif
//START170405
int get_rdriver(char *driver_file,char *rdriver,int nstr,int *notfactor,int *numeric){
    int i1,nlines=0,nstr0;
    char line[MAXNAME],*write_back_ptr,*strptr;
    FILE *fp,*op;
    if(!(fp=fopen_sub(driver_file,"r"))) return 0;
    if(!(op=fopen_sub(rdriver,"w"))) return 0;
    for(;fgets(line,sizeof(line),fp);){
        for(nstr0=0,strptr=line;strtok_r(strptr," \t",&strptr);nstr0++);
        if(nstr0>=nstr){
            for(strptr=line,i1=0;i1<nstr;i1++,strptr++){
                if(!(notfactor[i1]&&!numeric[i1])){
                    write_back_ptr=strtok_r(strptr," \t",&strptr);
                    fprintf(op,"%s%s",write_back_ptr,(i1<nstr-1)?"\t":"\n");
                    }
                else if(i1==nstr-1)fprintf(op,"\n");
                }
            nlines++;
            }
        }
    fclose(op);
    fclose(fp);
    return nlines;
    }

char **get_lmef(AnovaDesign *ad,int *lmerl,int nlmef){
    int i,j,k,n;
    size_t cnt;
    char string[1000],*ptr,*strptr,*write_back_ptr,**lmef;
    for(j=2,cnt=0,i=1;i<=ad->aa1;i++,j+=2)if(lmerl[i])cnt+=ad->fstatfilesl[j];

    //for(j=ad->aa1+1,i=0;i<ad->nnotfactor;i++,j++)if(lmerl[j])cnt+=strlen(ad->fnptr[ad->notfactori[i]])+1;
    //START160408
    for(j=ad->aa1+1,i=0;i<ad->nnotfactor;i++,j++)if(lmerl[j])cnt+=strlen(ad->fnptr0[ad->notfactori[i]])+1;

    if(!(lmef=malloc(sizeof*lmef*nlmef))){
        printf("Error: Unable to malloc lmef\n");
        return NULL;
        }
    if(!(lmef[0]=calloc(cnt,sizeof*lmef[0]))){
        printf("Error: Unable to malloc lmef[0]\n");
        return NULL;
        }
    for(lmef[0][0]=0,ptr=lmef[0],j=2,n=0,i=1;i<=ad->aa1;i++,j+=2)if(lmerl[i]){
        strcpy(string,ad->fstatfilesptr[j]);
        for(strptr=string;;) {
            if(!(write_back_ptr=strtok_r(strptr,":",&strptr)))break;
            strcat(ptr,write_back_ptr);
            strcat(ptr,"_");
            }
        lmef[n++]=ptr;
        ptr+=ad->fstatfilesl[j]-1;
        *ptr++=0;
        }

    for(k=ad->aa1+1,i=0;i<ad->nnotfactor;i++,k++)if(lmerl[k]){

        //strcat(ptr,ad->fnptr[ad->notfactori[i]]);
        //START160408
        strcat(ptr,ad->fnptr0[ad->notfactori[i]]);

        lmef[n++]=ptr;

        //ptr+=strlen(ad->fnptr[ad->notfactori[i]])+1;
        //START160408
        ptr+=strlen(ad->fnptr0[ad->notfactori[i]])+1;

        }
    return lmef;
    }
void get_lme_models(FILE *fp,AnovaDesign *ad,int *lmerl,int indent){
    int i,j,k,flag;
    for(i=1;i<=ad->aa1;i++){
        if(lmerl[i]) {
            fprintf(fp,"%sfidl.model%d=lmer(bold~",indent?"    ":"",i);
            for(flag=0,k=1,j=2;j<ad->num_sources;j+=2,k++) {
                if(k!=i) {
                    fprintf(fp,"%s%s",!flag?"":"+",ad->fstatfilesptr[j]);
                    flag=1;
                    }
                }

            //for(j=0;j<ad->nnotfactor;j++)fprintf(fp,"+%s",ad->fnptr[ad->notfactori[j]]);
            //START160408
            for(j=0;j<ad->nnotfactor;j++)fprintf(fp,"+%s",ad->fnptr0[ad->notfactori[j]]);

            #if 0
            KEEP - can only compute if you have replications
            if(ad->nwithin) for(j=0;j<ad->nwithin;j++) fprintf(fp,"+(%s|%s)",ad->fnptr[ad->withini[j]],ad->fnptr[RANDOM]);
            fprintf(fp,",data=fidl.list,REML=FALSE)\n");
            #endif
            if(ad->nwithin)fprintf(fp,"+(1|%s)",ad->fnptr[RANDOM]);
            fprintf(fp,",data=fidl.list,REML=FALSE)\n");
            }
        }
    for(k=ad->aa1+1,i=0;i<ad->nnotfactor;i++,k++){
        if(lmerl[k]) {
            fprintf(fp,"%sfidl.model%d=lmer(bold~%s",indent?"    ":"",k,ad->fstatfilesptr[2]);
            for(j=4;j<ad->num_sources;j+=2)fprintf(fp,"+%s",ad->fstatfilesptr[j]);

            //for(j=0;j<ad->nnotfactor;j++)if(j!=i)fprintf(fp,"+%s",ad->fnptr[ad->notfactori[j]]);
            //START16048
            for(j=0;j<ad->nnotfactor;j++)if(j!=i)fprintf(fp,"+%s",ad->fnptr0[ad->notfactori[j]]);

            #if 0
            KEEP - can only compute if you have replications
            if(ad->nwithin) for(j=0;j<ad->nwithin;k++) fprintf(fp,"+(%s|%s)",ad->fnptr[ad->withini[j]],ad->fnptr[RANDOM]);
            fprintf(fp,",data=fidl.list,REML=FALSE)\n");
            #endif
            if(ad->nwithin)fprintf(fp,"+(1|%s)",ad->fnptr[RANDOM]);
            fprintf(fp,",data=fidl.list,REML=FALSE)\n");
            }
        }
    }
int get_rscript_guts(char *file,AnovaDesign *ad,LME *lme,int regionalanalysis){
    int i,superbird;
    FILE *fp;
    if(!(fp=fopen_sub(file,"w")))return 0;
    fprintf(fp,"#!/usr/bin/env Rscript\nsuppressMessages(library(lme4))\n");

    #if 0
    fprintf(fp,"fidl.ind=read.delim(\"%s\",nrows=%d,colClasses=c(\"%s\"",lme->rdriver,lme->nlines,
        !ad->notfactor[0]?"factor":"numeric");
    for(i=1;i<ad->num_factors0;i++)fprintf(fp,",\"%s\"",!ad->notfactor[i]?"factor":"numeric");
    #endif
    //START170405
    fprintf(fp,"fidl.ind=read.delim(\"%s\",nrows=%d,colClasses=c(",lme->rdriver,lme->nlines);
    #if 0
    for(i=0;i<ad->num_factors0;i++)if(!(ad->notfactor[i]&&!ad->numeric0[i]))
        fprintf(fp,"\"%s\"%s",!ad->notfactor[i]?"factor":"numeric",i<ad->num_factors0-1?",":"");
    #endif
    for(superbird=i=0;i<ad->num_factors0;i++)if(!(ad->notfactor[i]&&!ad->numeric0[i]))
        {fprintf(fp,"%s\"%s\"",superbird?",":"",!ad->notfactor[i]?"factor":"numeric");superbird=1;}

    fprintf(fp,"))\nfidl.dep=read.delim(\"%s\",nrows=%d)\n",lme->rdata0,lme->nlines);
    fprintf(fp,"fidl.list=cbind(fidl.ind,fidl.dep)\n");
    sprintf(lme->model,"fidl.model0=lmer(bold~%s",ad->fstatfilesptr[2]);
    for(i=4;i<ad->num_sources;i+=2){strcat(lme->model,"+");strcat(lme->model,ad->fstatfilesptr[i]);}

    //for(i=0;i<ad->nnotfactor;i++){strcat(lme->model,"+");strcat(lme->model,ad->fnptr0[ad->notfactori[i]]);}
    //START170405
    for(i=0;i<ad->nnotfactor;i++)
        if(ad->numeric0[ad->notfactori[i]]){strcat(lme->model,"+");strcat(lme->model,ad->fnptr0[ad->notfactori[i]]);}

    if(ad->nwithin){strcat(lme->model,"+(1|");strcat(lme->model,ad->fnptr[RANDOM]);strcat(lme->model,")");}
    strcat(lme->model,",data=fidl.list,REML=FALSE)");
    fprintf(fp,"%s\n",lme->model);
    #if 0
    KEEP - can only compute if you have replications
    if(ad->nwithin) for(i=0;i<ad->nwithin;i++) fprintf(fp,"+(%s|%s)",ad->fnptr[ad->withini[i]],ad->fnptr[RANDOM]);
    fprintf(fp,",data=fidl.list,REML=FALSE)\n");
    #endif
    get_lme_models(fp,ad,lme->lmerl,0);
    for(i=0;i<=ad->aa0;i++)if(lme->lmerl[i])fprintf(fp,"cat(df.residual(fidl.model%d),\"\\n\")\n",i);
    if(regionalanalysis){
        for(i=0;i<=ad->aa0;i++)if(lme->lmerl[i])fprintf(fp,"cat(unname(getME(fidl.model%d,\"p\")),\"\\n\")\n",i);
        for(i=0;i<=ad->aa0;i++)if(lme->lmerl[i])fprintf(fp,"cat(names(getME(fidl.model%d,\"fixef\")),\"\\n\")\n",i);
        }
    fclose(fp);
    return 1;
    }
int get_rscript_guts2(AnovaDesign *ad,LME *lme,int regionalanalysis){
    int i,flag;
    FILE *fp;
    if(!(fp=fopen_sub(lme->rscript,"w"))) return 0;
    fprintf(fp,"#!/usr/bin/env Rscript\nsuppressMessages(library(lme4))\nargs=commandArgs(TRUE)\n");

    #if 0
    fprintf(fp,"fidl.ind=read.delim(\"%s\",nrows=%d,colClasses=c(\"%s\"",lme->rdriver,lme->nlines,
        !ad->notfactor[0]?"factor":"numeric");
    for(i=1;i<ad->num_factors0;i++)fprintf(fp,",\"%s\"",!ad->notfactor[i]?"factor":"numeric");
    #endif
    //START170405
    fprintf(fp,"fidl.ind=read.delim(\"%s\",nrows=%d,colClasses=c(",lme->rdriver,lme->nlines);
    #if 0
    for(i=0;i<ad->num_factors0;i++)if(!(ad->notfactor[i]&&!ad->numeric0[i]))
        fprintf(fp,"\"%s\"%s",!ad->notfactor[i]?"factor":"numeric",i<ad->num_factors0-1?",":"");
    #endif
    for(flag=i=0;i<ad->num_factors0;i++)if(!(ad->notfactor[i]&&!ad->numeric0[i]))
        {fprintf(fp,"%s\"%s\"",flag?",":"",!ad->notfactor[i]?"factor":"numeric");flag=1;}



    fprintf(fp,"))\nfidl.dep=double(%d)\nfidl.out=double(%d*as.integer(args[2]))\nfp=file(\"%s\",\"rb\")\n",lme->nlines-1,
        lme->nlmef+lme->nbeta*2,lme->rdata);
    fprintf(fp,"idx=0\n");
    fprintf(fp,"for(i in 1:args[2]){\n    fidl.dep=readBin(fp,double(),%d)\n",lme->nlines-1);
    fprintf(fp,"    fidl.list=cbind(fidl.ind,data.frame(matrix(fidl.dep,nrow=%d,dimnames=list(NULL,\"bold\"))))\n",lme->nlines-1);
    fprintf(fp,"    %s\n",lme->model);
    get_lme_models(fp,ad,lme->lmerl,1);
    fprintf(fp,"    fidl.out[(idx+1):(idx+%d)]=c(",lme->nlmef);
    for(flag=0,i=1;i<=ad->aa0;i++)if(lme->lmerl[i])fprintf(fp,"anova(fidl.model%d,fidl.model0)$Pr[2]%s",i,(++flag)<lme->nlmef?",":")");
    fprintf(fp,"\n    idx=idx+%d\n",lme->nlmef);
    if(regionalanalysis){
        fprintf(fp,"    fidl.out[(idx+1):(idx+%d)]=c(",lme->nbeta);
        for(flag=0,i=0;i<=ad->aa0;i++)if(lme->lmerl[i])fprintf(fp,"getME(fidl.model%d,\"beta\")%s",i,(++flag)<lme->nlmef+1?",":")");
        fprintf(fp,"\n    idx=idx+%d\n    fidl.out[(idx+1):(idx+%d)]=c(",lme->nbeta,lme->nbeta);
        for(flag=0,i=0;i<=ad->aa0;i++)if(lme->lmerl[i])fprintf(fp,"unname(coef(summary(fidl.model%d))[,\"Std. Error\"])%s",
            i,(++flag)<lme->nlmef+1?",":")");
        fprintf(fp,"\n    idx=idx+%d\n",lme->nbeta);
        }
    fprintf(fp,"    }\nclose(fp)\nfp=file(\"%s\",\"wb\")\nwriteBin(fidl.out,fp)\nclose(fp)\n",lme->rdatao);
    fclose(fp);
    return 1;
    }
int get_rscript(AnovaDesign *ad,char *scratchdir,LME *lme,int chunk,int regionalanalysis,int lc_names_only){
    char string[10000],*strptr,*strptr2;  
    int i,j,k,l,nhdr;
    long df0=0;
    FILE *fp;
    fpos_t pos;
    size_t *hdrl;
    sprintf(string,"%srscriptdf.r",scratchdir);
    for(i=0;i<=ad->aa0;i++)lme->lmerl[i]=1;
    if(!get_rscript_guts(string,ad,lme,regionalanalysis))return 0;
    if(!(fp=fopen_sub(lme->rdata0,"w"))) return 0;
    fprintf(fp,"bold\n");
    const gsl_rng_type *T = gsl_rng_mt19937;
    gsl_rng *r = gsl_rng_alloc(T);
    gsl_rng_set(r,0);
    for(i=0;i<chunk;i++)fprintf(fp,"%f\n",gsl_ran_gaussian_ziggurat(r,1.));
    gsl_rng_free(r);
    fclose(fp);
    if(!lc_names_only)printf("Dummy data written to %s\n",lme->rdata0);
    sprintf(string,"Rscript --vanilla %srscriptdf.r > %srscriptdf.out",scratchdir,scratchdir);
    if(!lc_names_only)printf("%s\n",string);
    if(system(string)==-1){printf("fidlError: %s\n",string);fflush(stdout);return 0;}
    sprintf(string,"%srscriptdf.out",scratchdir);
    if(!lc_names_only)printf("Reading %s\n",string);fflush(stdout);
    if(!(fp=fopen_sub(string,"r"))) return 0;
    for(lme->nlmef=0,i=0;i<=ad->aa0;i++){
        fgets(string,sizeof(string),fp);
        if(!i){df0=strtol(string,NULL,10);}else{lme->nlmef+=lme->lmerl[i]=(int)((strtol(string,NULL,10)-df0)?1:0);}
        }
    for(nhdr=j=i=0;i<=ad->aa0;i++){
        fgets(string,sizeof(string),fp);
        if(lme->lmerl[i]){nhdr+=lme->lmerp[j++]=(int)strtol(string,NULL,10);}
        }
    if(regionalanalysis){
        if(!(hdrl=malloc(sizeof*hdrl*nhdr))) {
            printf("fidlError: get_rscript Unable to malloc hdrl\n");
            return 0;
            }
        for(fgetpos(fp,&pos),l=j=i=0;i<=ad->aa0;i++){
            fgets(string,sizeof(string),fp);
            if(lme->lmerl[i]){
                for(strptr=string,k=0;k<lme->lmerp[j];k++,l++){
                    strptr2=strtok_r(strptr," ",&strptr);
                    hdrl[l]=strlen(strptr2)+1;
                    }
                j++;
                }
            }
        if(!(lme->namesfixef=d2charvar(nhdr,hdrl)))return 0;
        for(fsetpos(fp,&pos),l=j=i=0;i<=ad->aa0;i++){
            fgets(string,sizeof(string),fp);
            if(lme->lmerl[i]){
                for(strptr=string,k=0;k<lme->lmerp[j];k++,l++){
                    strptr2=strtok_r(strptr," ",&strptr);
                    strcpy(lme->namesfixef[l],strptr2);
                    }
                j++;
                }
            }
        free(hdrl);
        }
    fclose(fp);
    lme->nbeta=regionalanalysis?nhdr:0;
    #if 0
    printf("lme->nlmef=%d ad->aa0=%d\n",lme->nlmef,ad->aa0);
    printf("lme->lmerl=");for(i=0;i<=ad->aa0;i++)printf("%d ",lme->lmerl[i]);printf("\n");fflush(stdout);
    if(regionalanalysis){
        printf("lme->lmerp=");for(i=0;i<=lme->nlmef;i++)printf("%d ",lme->lmerp[i]);printf("\n");fflush(stdout);
        for(k=i=0;i<=lme->nlmef;i++)for(j=0;j<lme->lmerp[i];j++,k++)printf("lme->namesfixef[%d]=%s\n",k,lme->namesfixef[k]);
        }
    #endif
    if(!(get_rscript_guts2(ad,lme,regionalanalysis))) return 0;
    return 1;
    }
int get_rscript_guts_lmerTest(char *file,AnovaDesign *ad,LME *lme,int regionalanalysis){
    int i,superbird;
    FILE *fp;
    if(!(fp=fopen_sub(file,"w")))return 0;
    fprintf(fp,"#!/usr/bin/env Rscript\noptions(warn=1)\nsuppressMessages(library(lmerTest))\n");

    #if 0
    fprintf(fp,"fidl.ind=read.delim(\"%s\",nrows=%d,colClasses=c(\"%s\"",lme->rdriver,lme->nlines,
        !ad->notfactor[0]?"factor":"numeric");
    for(i=1;i<ad->num_factors0;i++)fprintf(fp,",\"%s\"",!ad->notfactor[i]?"factor":"numeric");
    #endif
    //START170405
    fprintf(fp,"fidl.ind=read.delim(\"%s\",nrows=%d,colClasses=c(",lme->rdriver,lme->nlines);
    #if 0
    for(i=0;i<ad->num_factors0;i++)if(!(ad->notfactor[i]&&!ad->numeric0[i]))
        fprintf(fp,"\"%s\"%s",!ad->notfactor[i]?"factor":"numeric",i<ad->num_factors0-1?",":"");
    #endif
    for(superbird=i=0;i<ad->num_factors0;i++)if(!(ad->notfactor[i]&&!ad->numeric0[i]))
        {fprintf(fp,"%s\"%s\"",superbird?",":"",!ad->notfactor[i]?"factor":"numeric");superbird=1;}



    fprintf(fp,"))\nfidl.dep=read.delim(\"%s\",nrows=%d)\n",lme->rdata0,lme->nlines);
    fprintf(fp,"fidl.list=cbind(fidl.ind,fidl.dep)\n");
    sprintf(lme->model,"fidl.model0=lmer(bold~");

    #if 0
    for(i=0;i<ad->num_factors0;i++)if(i!=(int)RANDOM){
        strcat(lme->model,ad->fnptr0[i]);
        if(i<ad->num_factors0-1)strcat(lme->model,"*");
        }
    #endif
    //START170405
    for(i=0;i<ad->num_factors0;i++)if(i!=(int)RANDOM){
        if(!(ad->notfactor[i]&&!ad->numeric0[i])){
            strcat(lme->model,ad->fnptr0[i]);
            if(i<ad->num_factors0-1)strcat(lme->model,"*");
            }
        }

    if(ad->nwithin){strcat(lme->model,"+(1|");strcat(lme->model,ad->fnptr[RANDOM]);strcat(lme->model,")");}
    strcat(lme->model,",data=fidl.list,REML=FALSE)");
    fprintf(fp,"%s\ncat(labels(anova(fidl.model0))[1][[1]],\"\\n\")\n",lme->model);
    if(regionalanalysis){
        fprintf(fp,"cat(unname(getME(fidl.model0,\"p\")),\"\\n\")\n");
        fprintf(fp,"cat(names(getME(fidl.model0,\"fixef\")),\"\\n\")\n");
        }
    fclose(fp);
    return 1;
    }
int get_rscript_guts2_lmerTest(AnovaDesign *ad,LME *lme,int regionalanalysis){
    int i,superbird;
    FILE *fp;
    if(!(fp=fopen_sub(lme->rscript,"w"))) return 0;
    fprintf(fp,"#!/usr/bin/env Rscript\nsuppressMessages(library(lmerTest))\nargs=commandArgs(TRUE)\n");

    #if 0
    fprintf(fp,"fidl.ind=read.delim(\"%s\",nrows=%d,colClasses=c(\"%s\"",lme->rdriver,lme->nlines,
        !ad->notfactor[0]?"factor":"numeric");
    for(i=1;i<ad->num_factors0;i++)fprintf(fp,",\"%s\"",!ad->notfactor[i]?"factor":"numeric");
    #endif
    //START170405
    fprintf(fp,"fidl.ind=read.delim(\"%s\",nrows=%d,colClasses=c(",lme->rdriver,lme->nlines);
    #if 0
    for(i=0;i<ad->num_factors0;i++)if(!(ad->notfactor[i]&&!ad->numeric0[i]))
        fprintf(fp,"\"%s\"%s",!ad->notfactor[i]?"factor":"numeric",i<ad->num_factors0-1?",":"");
    #endif
    for(superbird=i=0;i<ad->num_factors0;i++)if(!(ad->notfactor[i]&&!ad->numeric0[i]))
        {fprintf(fp,"%s\"%s\"",superbird?",":"",!ad->notfactor[i]?"factor":"numeric");superbird=1;}



    fprintf(fp,"))\nfidl.dep=double(%d)\nfidl.out=double(%d*as.integer(args[2]))\nfp=file(\"%s\",\"rb\")\n",lme->nlines-1,
        lme->nlmef*4+lme->nbeta*3,lme->rdata);
    fprintf(fp,"idx=0\n");
    fprintf(fp,"for(i in 1:args[2]){\n    fidl.dep=readBin(fp,double(),%d)\n",lme->nlines-1);
    fprintf(fp,"    fidl.list=cbind(fidl.ind,data.frame(matrix(fidl.dep,nrow=%d,dimnames=list(NULL,\"bold\"))))\n",lme->nlines-1);
    fprintf(fp,"    %s\n",lme->model);
    fprintf(fp,"    x=anova(fidl.model0)\n    fidl.out[(idx+1):(idx+%d)]=c(x$Pr,x$F,x$NumDF,x$DenDF)\n",lme->nlmef*4);
    fprintf(fp,"    idx=idx+%d\n",lme->nlmef*4);
    if(regionalanalysis){
        fprintf(fp,"    fidl.out[(idx+1):(idx+%d)]=c(getME(fidl.model0,\"beta\"))",lme->nbeta);
        fprintf(fp,"\n    idx=idx+%d\n    fidl.out[(idx+1):(idx+%d)]=c(unname(coef(summary(fidl.model0))[,\"Std. Error\"]))",
            lme->nbeta,lme->nbeta);
        fprintf(fp,"\n    idx=idx+%d\n    fidl.out[(idx+1):(idx+%d)]=c(unname(coef(summary(fidl.model0))[,\"Pr(>|t|)\"]))",
            lme->nbeta,lme->nbeta);
        fprintf(fp,"\n    idx=idx+%d\n",lme->nbeta);
        }
    fprintf(fp,"    }\nclose(fp)\nfp=file(\"%s\",\"wb\")\nwriteBin(fidl.out,fp)\nclose(fp)\n",lme->rdatao);
    fclose(fp);
    return 1;
    }
char** get_rscript_lmerTest(AnovaDesign *ad,char *scratchdir,LME *lme,int chunk,int regionalanalysis,int lc_names_only,char *rpath,
    char *output){
    char string[10000],line[LINE_MAX],string2[LINE_MAX],*strptr,*strptr2,**lmef,*ptr,last_char,first_char,*write_back_ptr;
    int i,k,cnt;
    FILE *fp;
    fpos_t pos;
    sprintf(string2,"%s%srscriptlab.r",scratchdir,output?output:"");
    sprintf(line,"%s%srscriptlab.out",scratchdir,output?output:"");
    if(!get_rscript_guts_lmerTest(string2,ad,lme,regionalanalysis))return NULL;
    if(!(fp=fopen_sub(lme->rdata0,"w")))return NULL;
    fprintf(fp,"bold\n");
    const gsl_rng_type *T = gsl_rng_mt19937;
    gsl_rng *r = gsl_rng_alloc(T);
    gsl_rng_set(r,0);
    for(i=0;i<chunk;i++)fprintf(fp,"%f\n",gsl_ran_gaussian_ziggurat(r,1.));
    gsl_rng_free(r);
    fclose(fp);


    #if 0
    if(!lc_names_only){printf("Dummy data written to %s\n",lme->rdata0);fflush(stdout);}
    sprintf(string,"%s/Rscript --vanilla %s > %s",rpath,string2,line);
    if(!lc_names_only){printf("%s\n",string);fflush(stdout);}
    if(system(string)==-1){printf("fidlError: %s\n",string);fflush(stdout);return NULL;}
    if(!lc_names_only){printf("Reading %s\n",line);fflush(stdout);}
    #endif
    //START160421
    fprintf(!lc_names_only?stdout:stderr,"Dummy data written to %s\n",lme->rdata0);fflush(stdout);
    sprintf(string,"%s/Rscript --vanilla %s > %s",rpath,string2,line);
    fprintf(!lc_names_only?stdout:stderr,"%s\n",string);fflush(stdout);
    if(system(string)==-1){printf("fidlError: %s\n",string);fflush(stdout);return NULL;}
    fprintf(!lc_names_only?stdout:stderr,"Reading %s\n",line);fflush(stdout);


    if(!(fp=fopen_sub(line,"r")))return NULL;
    fgets(line,LINE_MAX,fp);
    if(!(lme->nlmef=count_strings_new3(line,string,' ',' ',&last_char,&first_char,0,' '))){
        printf("fidlError: get_rscript_lmerTest lme->nlmef=%d\n",lme->nlmef);fflush(stdout);return NULL;}
    cnt=(int)strlen(string)+1;
    if(!(lmef=malloc(sizeof*lmef*lme->nlmef))){
        printf("fidlError: get_rscript_lmerTest Unable to malloc lmef\n");
        return NULL;
        }
    if(!(lmef[0]=calloc(cnt,sizeof*lmef[0]))){
        printf("fidlError: get_rscript_lmerTest Unable to malloc lmef[0]\n");
        return NULL;
        }
    for(ptr=lmef[0],strptr=string,i=0;i<lme->nlmef;i++){
        strptr2=strtok_r(strptr," ",&strptr);
        for(;;){
            if(!(write_back_ptr=strtok_r(strptr2,":",&strptr2)))break;
            strcat(ptr,write_back_ptr);
            strcat(ptr,"_");
            }
        lmef[i]=ptr;
        ptr+=strlen(ptr)-1;
        *ptr++=0;
        }
    if(regionalanalysis){
        if(!(lme->lmerp=malloc(sizeof*lme->lmerp))){
            printf("fidlError: get_rscript_lmerTest Unable to malloc lme->lmerp\n");
            return NULL;
            }
        fgets(string,sizeof(string),fp);
        lme->lmerp[0]=(int)strtol(string,NULL,10);
        fgetpos(fp,&pos);
        fgets(string,sizeof(string),fp);
        for(strptr=string,cnt=k=0;k<lme->lmerp[0];k++){
            strptr2=strtok_r(strptr," ",&strptr);
            cnt+=(int)strlen(strptr2)+1;
            }
        if(!(lme->namesfixef=malloc(sizeof*lme->namesfixef*lme->lmerp[0]))){
            printf("fidlError: get_rscript_lmerTest Unable to malloc lme->namesfixef\n");
            return NULL;
            }
        if(!(lme->namesfixef[0]=calloc(cnt,sizeof*lme->namesfixef[0]))){
            printf("fidlError: get_rscript_lmerTest Unable to malloc lme->namesfixef[0]\n");
            return NULL;
            }
        fsetpos(fp,&pos);
        fgets(string,sizeof(string),fp);
        for(ptr=lme->namesfixef[0],strptr=string,k=0;k<lme->lmerp[0];k++){
            ptr=strtok_r(strptr," ",&strptr);
            lme->namesfixef[k]=ptr;
            ptr+=strlen(ptr)+1;
            }
        }
    fclose(fp);
    lme->nbeta=regionalanalysis?lme->lmerp[0]:0;
    if(!(get_rscript_guts2_lmerTest(ad,lme,regionalanalysis)))return NULL;
    return lmef;
    }

//START160307
char* get_rpath(){
    char *home,*hostname,dotfidl[1000],line[1000],write_back[1000],*p,*rpath=NULL,*rpathd="/usr/local/pkg/R/bin",last_char,first_char,
        *strptr,*p1;
    int nstrings,lcdefault=0;
    size_t l;
    fpos_t pos;
    FILE *fp;
    if(!(home=getenv("HOME"))) {
        printf("fidlError: get_rpath Unable to get environment variable HOME\n");
        return NULL;
        }
    if(!(hostname=getenv("HOSTNAME"))) {
        printf("fidlError: get_rpath Unable to get environment variable HOSTNAME\n");
        return NULL;
        }
    sprintf(dotfidl,"%s/.fidl",home);
    if(!(fp=fopen_sub(dotfidl,"r")))return NULL;
    for(;fgets(line,sizeof(line),fp);){

        //printf("here0 line=%s\n",line);

        if((nstrings=count_strings_new3(line,write_back,' ',' ',&last_char,&first_char,0,' '))>=3){
            strptr=write_back;
            p=strtok_r(strptr," ",&strptr);

            //printf("here1 p=%s\n",p);

            //for(;*p;++p)*p=tolower(*p); 
            //START160308
            for(p1=p;*p1;++p1)*p1=tolower(*p1); 

            //printf("here2 p=%s\n",p);

            if(!strcmp("rpath",p)){
                p=strtok_r(strptr," ",&strptr);

                //printf("here3 p=%s\n",p);

                if(!strcmp(hostname,p)){
                    p=strtok_r(strptr," \n",&strptr);
                    l=strlen(p);
                    if(*(p+l-1)!='/')l++;
                    if(!(rpath=malloc(sizeof*rpath*l))){
                        printf("fidlError: get_rpath Unable to malloc rpath\n");
                        exit(-1);
                        }
                    strncpy(rpath,p,l);
                    if(rpath[l-1]=='/')rpath[l-1]=0;
                    break;
                    }
                else if(!strcmp("default",p)){
                    fgetpos(fp,&pos);
                    lcdefault=1;
                    }
                }
            }
        }
    if(!rpath&&lcdefault){
        fsetpos(fp,&pos);
        p=strtok_r(strptr," \n",&strptr);
        l=strlen(p);
        if(*(p+l-1)!='/')l++;
        if(!(rpath=malloc(sizeof*rpath*l))){
            printf("fidlError: get_rpath Unable to malloc rpath\n");
            exit(-1);
            }
        strncpy(rpath,p,l);
        if(rpath[l-1]=='/')rpath[l-1]=0;
        }
    fclose(fp);
    if(!rpath){
        printf("fidlWarning: No Rpath found for %s or default path listed in %s.\n",hostname,dotfidl);
        printf("fidlWarning: We will use %s.\n",rpathd);
        if(!(rpath=malloc(sizeof*rpath*(strlen(rpathd)+1)))){
            printf("fidlError: get_rpath Unable to malloc rpath\n");
            exit(-1);
            }
        strcpy(rpath,rpathd);
        }
    return rpath;
    }
