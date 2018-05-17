/* Copyright 4/18/18 Washington University.  All Rights Reserved.
   fidl_logreg2.cxx  $Revision: 1.1 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_combination.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_permutation.h>
#include "constants.h"
#include "find_regions_by_file_cover.h"
#include "get_atlas_param.h"
#include "dim_param2.h"
#include "map_disk.h"
#include "mask.h"
#include "read_frames_file.h"
#include "files_struct.h"
#include "read_tc_string_new.h"
#include "checkOS.h"
#include "read_data.h"
#include "read_xform.h"
#include "twoA_or_twoB.h"
#include "get_atlas.h"
#include "fidl.h"
#include "subs_util.h"
#include "minmax.h"
#include "x2_to_z.h"
#include "make_timestr2.h"
#include "subs_cifti.h"
#include "write1.h"
#include "filetype.h"
#include "get_limits.h"
#include "cond_norm1.h"
#include "gsl_svd.h"
#include "check_dimensions.h"
#include "region.h"
#include "logreg.h"

#if 0
#define MAXINC 5
#define MAXIT 5 
#define TOL 1e-3
#endif
//START180112
#define MAXINC 1 
#define MAXIT 10 
#define TOL 0.1 

#define CONDMAX 10000.0

typedef struct {
    int uns_count,small;
    } Scratch;

void assignXandy(LRy *lry,double *indvar,int nindvar_per_line,int j,int lcconstant,double *y,double *X,int lengthX,
    gsl_combination *c,int k);
int assignXtrblockgsl(Data *data,int *num_regions_by_file,int num_regions,int num_wfiles,int goose,int j,int p,int *sindex,
    FS *fs,double *X,Scratch *s,int lengthX);
double logreg_guts(int nlines,int lengthX,double *w,double *X,double *AX,double *y,double *z,double *wTX,double *tau,double *residual,
    double llold);

class logreg{
    double *w,*wTX,*AX,*Az,*XAXT,*XAXTcopy,*XAXTinv,*XAz,*V,*S,*work;
    public:
        int logreg0(size_t lengthX0,size_t nlines);
        double logreg_guts(int nlines,int lengthX,double *X,double *y,double ll_old,double lambda);

        //START180228
        double* get_w();

    };
int logreg::logreg0(size_t lengthX0,size_t nlines){
    if(!(w=(double*)malloc(sizeof*w*lengthX0))) {
        printf("fidlError: Unable to malloc w\n");
        return 0;
        }
    if(!(wTX=(double*)malloc(sizeof*wTX*nlines))) {
        printf("fidlError: Unable to malloc wTX\n");
        return 0;
        }
    if(!(AX=(double*)malloc(sizeof*AX*lengthX0*nlines))) {
        printf("fidlError: Unable to malloc AX\n");
        return 0;
        }
    if(!(Az=(double*)malloc(sizeof*Az*nlines))) {
        printf("fidlError: Unable to malloc Az\n");
        return 0;
        }
    if(!(XAXT=(double*)malloc(sizeof*XAXT*lengthX0*lengthX0))) {
        printf("fidlError: Unable to malloc XAXT\n");
        return 0;
        }
    if(!(XAXTcopy=(double*)malloc(sizeof*XAXTcopy*lengthX0*lengthX0))) {
        printf("fidlError: Unable to malloc XAXTcopy\n");
        return 0;
        }
    if(!(XAXTinv=(double*)malloc(sizeof*XAXTinv*lengthX0*lengthX0))) {
        printf("fidlError: Unable to malloc XAXTinv\n");
        return 0;
        }
    if(!(XAz=(double*)malloc(sizeof*XAz*lengthX0))){
        printf("fidlError: Unable to malloc XAz\n");
        return 0;
        }
    if(!(V=(double*)malloc(sizeof*V*lengthX0*lengthX0))){
        printf("fidlError: Unable to malloc V\n");
        return 0;
        }
    if(!(S=(double*)malloc(sizeof*S*lengthX0))){
        printf("fidlError: Unable to malloc S\n");
        return 0;
        }
    if(!(work=(double*)malloc(sizeof*work*lengthX0))){
        printf("fidlError: Unable to malloc work\n");
        return 0;
        }
    return 1;
    }
double logreg::logreg_guts(int nlines,int lengthX,double *X,double *y,double ll_old,double lambda){
    int i,k,kk,increment,iterations,ndep;
    double ll,temp,A,cond,cond2;

    gsl_matrix_view gX = gsl_matrix_view_array(X,nlines,lengthX);
    gsl_vector_view gw = gsl_vector_view_array(w,lengthX);
    gsl_vector_view gwTX = gsl_vector_view_array(wTX,nlines);
    gsl_matrix_view gAX = gsl_matrix_view_array(AX,nlines,lengthX);
    gsl_matrix_view gXAXT = gsl_matrix_view_array(XAXT,lengthX,lengthX);
    gsl_vector_view gAz = gsl_vector_view_array(Az,nlines);
    gsl_vector_view gXAz = gsl_vector_view_array(XAz,lengthX);
    gsl_matrix_view gXAXTinv = gsl_matrix_view_array(XAXTinv,lengthX,lengthX);

    //alllocate this once in an init routine, make sure it gives the same answer
    gsl_permutation *perm = gsl_permutation_alloc(lengthX);

    gsl_vector_set_zero(&gw.vector);


    #if 0
    printf("X\n");
    for(kk=i=0;i<nlines;i++){
        for(k=0;k<lengthX;k++,kk++)printf("%f ",X[kk]);
        printf("\n");
        }
    printf("\n");
    #endif

    //printf("w\n");for(k=0;k<lengthX;k++)printf("%f ",w[k]);printf("\n");


    for(increment=iterations=0;;){
        gsl_blas_dgemv(CblasNoTrans,1.0,&gX.matrix,&gw.vector,0.0,&gwTX.vector);
        for(kk=0,ll=0.,i=0;i<nlines;i++) {
            temp = 1./(1.+exp(wTX[i]));
            A = temp*(1-temp);
            for(k=0;k<lengthX;k++,kk++) AX[kk] = X[kk]*A;
            temp = 1. + exp(-y[i]*wTX[i]);
            Az[i] = A*wTX[i] + (1.-1./temp)*y[i];
            ll += log(temp);
            }

        #if 0
        printf("AX\n");
        for(kk=i=0;i<nlines;i++){
            for(k=0;k<lengthX;k++,kk++)printf("%f ",AX[kk]);
            printf("\n");
            }
        printf("\n");
        #endif


        if((increment = fabs(ll_old-ll)<=(double)TOL ? increment+1 : 0)<(int)MAXINC && ++iterations<(int)MAXIT){
            gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,&gX.matrix,&gAX.matrix,0.0,&gXAXT.matrix);
            if(lambda>0.)for(kk=i=0;i<lengthX;i++,kk+=lengthX+1)XAXT[kk]+=lambda;

            #if 0
            printf("XAXT\n");
            for(kk=i=0;i<lengthX;i++){
                for(k=0;k<lengthX;k++,kk++)printf("%f ",XAXT[kk]);
                printf("\n");
                }
            printf("\n");
            #endif

            memcpy(XAXTcopy,XAXT,lengthX*lengthX*sizeof*XAXT);
            if(cond_norm1(XAXT,lengthX,&cond,XAXTinv,perm))cond=0.;
            if(cond>CONDMAX||cond==0.)gsl_svd_golubreinsch(XAXTcopy,lengthX,lengthX,0.,V,S,XAXTinv,&cond2,&ndep,work);
            gsl_blas_dgemv(CblasTrans,1.0,&gX.matrix,&gAz.vector,0.0,&gXAz.vector);

            //gsl_blas_dsymv(CblasUpper,1.0,&gXAXTinv.matrix,&gXAz.vector,0.0,&gw.vector);
            gsl_blas_dgemv(CblasNoTrans,1.0,&gXAXTinv.matrix,&gXAz.vector,0.0,&gw.vector);

            ll_old = ll;

            //printf("w\n");for(k=0;k<lengthX;k++)printf("%f ",w[k]);printf("\n");

            }
        else{
            break;
            }

        //printf("ll=%f increment=%d iterations=%d\n",ll,increment,iterations);

        }

    //printf("end w\n");for(k=0;k<lengthX;k++)printf("%f ",w[k]);printf("\n");

    gsl_permutation_free(perm);

    return ll;
    }

//START180228
double* logreg::get_w(){
    return w;
    } 


//START171107
char const* Fileext[]={"",".4dfp.img",".dscalar.nii",".nii",".dtseries.nii"};

int main(int argc,char **argv)
{
char *regional_name="fidl_logreg_ss.txt",*xform_file=NULL,filename[MAXNAME],*mask_file=NULL,timestr[23],
     *scratchdir=NULL,*root=NULL,*frames_file=NULL,*logregwts_file=NULL,*driver_file=NULL,string[MAXNAME],atlas[7]="";

int i,j,k,l,m,o,m1,num_regions=0,num_region_files=0,num_tc_files=0,*roi=NULL,nframes_cond=0,*frames_cond=NULL,num_tc_names=0,
    num_tc_cond=0,A_or_B_or_U,lcscratch=1,*temp_int,n0,n1,SunOS_Linux,*nframes,*sindex=NULL,half=0,
    *lengthX,argc_tc=0,lcconstant=0,modulo=20000,bigendian=1,lcreducedmodels=0,nindvar=0,nmodels,*df,tlengthX,
    endloop,lrsinc,wstackinc,lcROCarea=0,lccleanup=0,*brnidx=NULL,oldguts=0,*superbird,indonly=0,*m0;
 
float *t4,*temp_float,*pval; //*logregwts_stack

double *temp_double,temp,ll,ll_old=0,*wstack=NULL,*lrs,ll_null=0.,ll_null_new,*ROCarea,*dfstack,log2,*X,*w=NULL,*wTX=NULL,*y,*z=NULL,
    *tau=NULL,*residual=NULL,*AX=NULL,*temp_double2,*LL0,*LLm,lambda=0.,*X0,*w0,*w01,*dptr;

size_t *cptr,*indx,l0,ck,i0,j0;

FILE *fp;
Regions **reg;
Regions_By_File *rbf=NULL,*fbf;
Atlas_Param *ap;
Dim_Param2 *dp;
Interfile_header *ifh=NULL;
mask ms;
FS *fs;
Files_Struct *tc_names=NULL,*tc_files=NULL,*region_files=NULL; //*wfiles=NULL
TC *tcs=NULL;
XmlS *xmls;
W1 *w1=NULL;
int64_t dims[3];
Scratch *s=NULL;
Data *data=NULL;
LRy *lry=NULL;
gsl_combination *c;
gsl_matrix_view wstackgsl;
Limits limits;
logreg lr;

log2 = log(2);
if(argc < 5) {
    fprintf(stderr,"        -tc_files:            4dfp timecourse files.\n");
    fprintf(stderr,"        -region_file:         *.4dfp.img file(s) that specifies regions of interest.\n");
    fprintf(stderr,"                              Timecourses are averaged over the region.\n");
    fprintf(stderr,"        -regions_of_interest: Compute timecourses for selected regions in the region file(s).\n");
    fprintf(stderr,"                              First region is one.\n");
    fprintf(stderr,"        -frames:              Frames to extract. First frame is 1.\n");
    fprintf(stderr,"        -frames_cond:         Identifier for each frame from second column of event file.\n");
    fprintf(stderr,"        -regional_name:       Output filename for regional results. Default is fidl_logreg_ss.txt\n");
    fprintf(stderr,"        -tc_names:            Timcourse identifiers.\n");
    fprintf(stderr,"        -tc_cond:             Identifier from second column of event file.\n");
    fprintf(stderr,"        -xform_file:          Name of 2A or 2B t4 file defining the transform to atlas space.\n");
    fprintf(stderr,"        -mask:                Only voxels in the mask are analyzed.\n");
    fprintf(stderr,"        -scratchdir:          Location of precomputed scratchfiles. Include the backslash at the end.\n");
    fprintf(stderr,"        -clean_up             Delete scratch directory.\n");
    fprintf(stderr,"        -logregwts_file:      Compute ROCs only.\n");
    fprintf(stderr,"        -constant             Include a constant term in the model.\n");
    fprintf(stderr,"        -driver:              File that specifies the behavioral measures (independent variables).\n");
    fprintf(stderr,"        -littleendian         Write output in little endian. Big endian is the default.\n");
    fprintf(stderr,"        -reducedmodels        Evaluate the statistical significance of the contribution of each independent variable.\n");

    //START180110
    fprintf(stderr,"        -root                 Output root.\n");
    fprintf(stderr,"        -lambda               Regularization. Try 0.01. Default is 0 (no regularization).\n");
    fprintf(stderr,"        -oldguts              Run the old logreg_guts algorithm.\n");

    //START180418
    fprintf(stderr,"        -indonly              Only run individual variable models.\n");



    exit(-1);
    }
for(i=1;i<argc;i++) {
    if(!strcmp(argv[i],"-tc_files") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_tc_files;
        if(!(tc_files=get_files(num_tc_files,&argv[i+1]))) exit(-1);
        i += num_tc_files;
        }
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
    if(!strcmp(argv[i],"-frames") && argc > i+1)
        frames_file = argv[++i];
    if(!strcmp(argv[i],"-frames_cond") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++nframes_cond;
        if(!(frames_cond=(int*)malloc(sizeof*frames_cond*nframes_cond))) {
            printf("fidlError: Unable to malloc frames_cond\n");
            exit(-1);
            }
        for(j=0;j<nframes_cond;j++) frames_cond[j] = atoi(argv[++i]);
        }
    if(!strcmp(argv[i],"-regional_name") && argc > i+1)
        regional_name = argv[++i];
    if(!strcmp(argv[i],"-tc_names") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_tc_names;
        if(!(tc_names=get_files(num_tc_names,&argv[i+1]))) exit(-1);
        i += num_tc_names;
        }
    if(!strcmp(argv[i],"-tc_cond") && argc > i+1) {
        for(j=1; i+j < argc && strchr(argv[i+j],'-') != argv[i+j]; j++) ++num_tc_cond;
        argc_tc = i+1;
        i += num_tc_cond;
        }
    if(!strcmp(argv[i],"-xform_file") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        xform_file = argv[++i];
    if(!strcmp(argv[i],"-mask") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        mask_file = argv[++i];
    if(!strcmp(argv[i],"-scratchdir") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1]) {
        scratchdir = argv[++i];
        lcscratch = 0;
        }
    if(!strcmp(argv[i],"-clean_up"))
        lccleanup = 1;
    if(!strcmp(argv[i],"-logregwts_file") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        logregwts_file = argv[++i];
    if(!strcmp(argv[i],"-root") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        root = argv[++i];
    if(!strcmp(argv[i],"-ROCarea"))
        lcROCarea = 1;
    if(!strcmp(argv[i],"-constant"))
        lcconstant = 1;
    if(!strcmp(argv[i],"-driver") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        driver_file = argv[++i];
    if(!strcmp(argv[i],"-littleendian"))
        bigendian = 0;
    if(!strcmp(argv[i],"-reducedmodels"))
        lcreducedmodels = 1;

    //START180110
    if(!strcmp(argv[i],"-lambda") && argc > i+1 && strchr(argv[i+1],'-') != argv[i+1])
        lambda=strtod(argv[++i],NULL);
    if(!strcmp(argv[i],"-oldguts"))
        oldguts = 1;

    //START180418
    if(!strcmp(argv[i],"-indonly"))
        indonly = 1;


    }
if((SunOS_Linux=checkOS())==-1) exit(-1);
printf("SunOS_Linux=%d\n",SunOS_Linux);
if(!num_tc_files) {
    printf("fidlError: No timecourse files. Abort!\n");
    exit(-1);
    }
if(num_tc_cond%2) {
    printf("fidlError: num_tc_cond=%d Must have an even number of tc_cond. Abort!\n",num_tc_cond); 
    exit(-1);
    }
if(num_tc_names > 2) {
    printf("fidlError: Only two levels may be analyzed with a logistic regression. Abort!\n");
    exit(-1);
    }
if(!frames_file && !driver_file) {
    if(!frames_file) {
        printf("fidlError: Need to specify frames file with -frames option. Abort!\n");
        exit(-1);
        }
    if(!driver_file) {
        printf("fidlError: Need to specify driver file with -driver option. Abort!\n");
        exit(-1);
        }
    }
if(!num_region_files && !logregwts_file) {
    if(!root) {
        printf("fidlError: Need to specify the output root with -root\n");
        exit(-1);
        }
    }

//START171207
limits=get_limits();
printf("limits.name_max=%ld lambda=%f oldguts=%d\n",limits.name_max,lambda,oldguts);

//START170714
#if 0
if(lcscratch) if(!(scratchdir=make_scratchdir())) exit(-1);
printf("scratchdir=%s\n",scratchdir);
#endif

if(num_tc_cond) {
    if(!(tcs=read_tc_string_new(num_tc_cond,(int*)NULL,argc_tc,argv))) exit(-1);
    #if 0
        printf("num_tc_cond=%d\n",num_tc_cond);
        printf("tcs->num_tc=%d\n",tcs->num_tc);
        printf("tcs->num_tc_to_sum="); for(i=0;i<tcs->num_tc;i++) printf("%d ",tcs->num_tc_to_sum[i][0]); printf("\n");
        printf("tcs->tc=\n");
        for(i=0;i<tcs->num_tc;i++) {
            for(k=0;k<tcs->each[i];k++) {
                for(j=0;j<tcs->num_tc_to_sum[i][k];j++) printf("%d ",(int)tcs->tc[i][k][j]);
                printf("    ");
                }
            printf("\n");
            }
    #endif
    }

if(driver_file){
    if(!(data=read_data(driver_file,1,0,0,0)))exit(-1);
    printf("data->nsubjects=%d data->npoints_per_line[0]=%d data->total_npoints_per_line=%d data->ncol=%d data->npoints=%d\n",
        data->nsubjects,data->npoints_per_line[0],data->total_npoints_per_line,data->ncol,data->npoints); 
    printf("data->colptr=");for(i=0;i<data->ncol;i++)printf("%s ",data->colptr[i]);printf("\n");

    if(!(frames_file=(char*)malloc(sizeof*frames_file*MAXNAME))) {
        printf("fidlError: Unable to malloc frames_file\n");
        exit(-1);
        }
    sprintf(frames_file,"%s%sframes.dat",scratchdir,make_timestr2(timestr));
    if(!(fp=fopen_sub(frames_file,"w"))) exit(-1);
    for(j=0;j<data->nsubjects;j++) fprintf(fp,"%d\n",j+1);
    fclose(fp);

    nindvar=data->npoints;
    }

if(!(fs=read_frames_file(frames_file,0,0,0))) exit(-1);
if(!driver_file) {
    if(fs->nlines != nframes_cond) {
        printf("Error: Number of lines in %s does not match the number of frames given by -frames_cond. Abort!\n",frames_file);
        printf("Error: fs->nlines=%d fs->nframes_cond=%d\n",fs->nlines,nframes_cond);
        exit(-1);
        }
    if(!(nframes=(int*)malloc(sizeof*nframes*num_tc_cond))) {
        printf("fidlError: Unable to malloc nframes\n");
        exit(-1);
        }
    for(i=0;i<num_tc_cond;i++) nframes[i]=0;
    for(i=0;i<fs->nlines;i++) {
        for(j=0;j<tcs->num_tc;j++) {
            for(k=0;k<tcs->each[j];k++) {
                if(frames_cond[i] == tcs->tc[j][k][0]) {
                    if(nframes[j] < fs->frames_per_line[i]) nframes[j] = fs->frames_per_line[i];
                    }
                }
            }
        }
    half = num_tc_cond/2;

    for(nindvar=i=0;i<half;i++) {
        if(nframes[i] != nframes[i+half]) {
            printf("Error: tc_cond %s has %d timepoints. Its pair, tc_cond %s has %d timepoints. Must be equal. Abort!\n",
                argv[argc_tc+i],nframes[i],argv[argc_tc+i+half],nframes[i+half]);
            exit(-1);
            }
        nindvar += nframes[i];
        }

    printf("num_tc_cond=%d half=%d\n",num_tc_cond,half);
    printf("tc_cond="); for(i=0;i<num_tc_cond;i++) printf("%s ",argv[argc_tc+i]); printf("\n");
    printf("nframes="); for(i=0;i<num_tc_cond;i++) printf("%d ",nframes[i]); printf("\n");
    if(!(sindex=(int*)malloc(sizeof*sindex*nframes_cond))) {
        printf("fidlError: Unable to malloc sindex\n");
        exit(-1);
        }
    for(i=0;i<nframes_cond;i++) sindex[i]=0;
    if(!(temp_int=(int*)malloc(sizeof*temp_int*num_tc_cond))) {
        printf("fidlError: Unable to malloc temp_int\n");
        exit(-1);
        }
    for(i=0;i<num_tc_cond;i++) temp_int[i]=0;
    for(i=0;i<nframes_cond;i++) {
        for(l=j=0;j<half;j++) {
            for(k=0;k<tcs->each[j];k++) {
                if(frames_cond[i] == tcs->tc[j][k][0]) {
                    l = 1;
                    break;
                    }
                }
            if(l) {
                temp_int[j]++;
                break;
                }
            else {
                sindex[i] += nframes[j];
                }
            }
        if(!l) {
            sindex[i] = 0;
            for(j=half;j<num_tc_cond;j++) {
                for(k=0;k<tcs->each[j];k++) {
                    if(frames_cond[i] == tcs->tc[j][k][0]) {
                        l = 1;
                        break;
                        }
                    }
                if(l) {
                    temp_int[j]++;
                    break;
                    }
                else {
                    sindex[i] += nframes[j];
                    }
                }
            }
        if(!l) {
            printf("Error: l is zero. Should be 1. Abort!\n");
            exit(-1);
            }
        }
    for(j=0;j<num_tc_cond;j++) printf("tc_cond=%s %d trials\n",argv[argc_tc+j],temp_int[j]);
    free(temp_int);
    /*printf("sindex="); for(i=0;i<nframes_cond;i++) printf("%d ",sindex[i]); printf("\n");*/
    }


//nmodels = !lcreducedmodels ? 1 : 1+nindvar;
//START180416
nmodels = !lcreducedmodels ? 1 : 1+2*nindvar;


if(!(lengthX=(int*)malloc(sizeof*lengthX*nmodels))) {
    printf("fidlError: Unable to malloc lengthX\n");
    exit(-1);
    }
if(!(df=(int*)malloc(sizeof*df*nmodels))) {
    printf("fidlError: Unable to malloc df\n");
    exit(-1);
    }


//START180416
endloop = nmodels==1 ? 1 : 3;
if(!(superbird=(int*)malloc(sizeof*superbird*endloop))) {
    printf("fidlError: Unable to malloc superbird\n");
    exit(-1);
    }
superbird[0]=nindvar;

//START180418
if(!(m0=(int*)malloc(sizeof*m0*endloop))) {
    printf("fidlError: Unable to malloc m0\n");
    exit(-1);
    }
m0[0]=0;


tlengthX=lengthX[0]=(df[0]=nindvar)+lcconstant;


//for(i=1;i<nmodels;i++) tlengthX+=lengthX[i]=(df[i]=nindvar-1)+lcconstant;
//START180416
if(nmodels>1){
    superbird[1]=nindvar-1;superbird[2]=1;
    m0[1]=1;m0[2]=nindvar+1;
    for(i=1;i<=nindvar;i++) tlengthX+=lengthX[i]=(df[i]=nindvar-1)+lcconstant;
    for(;i<=nmodels;i++) tlengthX+=lengthX[i]=(df[i]=1)+lcconstant;
    }
printf("nmodels=%d tlengthX=%d\n",nmodels,tlengthX);
printf("lengthX=");for(i=0;i<nmodels;i++)printf("%d ",lengthX[i]);printf("\n");
printf("df=");for(i=0;i<nmodels;i++)printf("%d ",df[i]);printf("\n");
printf("superbird=");for(i=0;i<endloop;i++)printf("%d ",superbird[i]);printf("\n");
printf("indonly=%d\n",indonly);
printf("m0=");for(i=0;i<endloop;i++)printf("%d ",m0[i]);printf("\n");
//exit(-1);


if(!(dp=dim_param2(tc_files->nfiles,tc_files->files,SunOS_Linux)))exit(-1);
if(dp->volall==-1){printf("fidlError: All files must be the same size. Abort!\n");exit(-1);}
if(xform_file) {
    if(!(t4=(float*)malloc(sizeof*t4*(size_t)T4SIZE))) {
        printf("fidlError: Unable to malloc t4\n");
        exit(-1);
        }
    if(!read_xform(xform_file,t4)) exit(-1);
    if((A_or_B_or_U=twoA_or_twoB(xform_file)) == 2) {
        printf("fidlError: Illegally named t4 file.\n");
        exit(-1);
        }
    strcpy(atlas,"222");
    }
else {
    get_atlas(dp->volall,atlas);
    }
if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL,tc_files->files[0]))) exit(-1);
if(!xform_file)ap->vol=dp->volall;
if(!atlas[0]){
    ap->xdim = dp->xdim[0];
    ap->ydim = dp->ydim[0];
    ap->zdim = dp->zdim[0];
    ap->voxel_size[0] = dp->dxdy[0];
    ap->voxel_size[1] = dp->dxdy[0];
    ap->voxel_size[2] = dp->dz[0];
    }
if(num_region_files) {
    if(!check_dimensions(num_region_files,region_files->files,ap->vol)) exit(-1);
    if(!(reg=(Regions**)malloc(sizeof*reg*num_region_files))) {
        printf("fidlError: Unable to malloc reg\n");
        exit(-1);
        }

    //for(i=0;i<num_region_files;i++) if(!(reg[i] = extract_regions(region_files->files[i],0,0,(float*)NULL,0,SunOS_Linux,
    //    (char**)NULL))) exit(-1);
    //START180416
    for(i=0;i<num_region_files;i++)if(!(reg[i]=get_reg(region_files->files[i],0,0,(float*)NULL,0,SunOS_Linux,(char**)NULL)))exit(-1);

    if(!num_regions) for(m=0;m<num_region_files;m++) num_regions += reg[m]->nregions;
    if(!(rbf=find_regions_by_file_cover(num_region_files,num_regions,reg,roi))) exit(-1);
    }
else {
    if(!(ms.get_mask(mask_file,ap->vol,(int*)NULL,SunOS_Linux,(LinearModel*)NULL,ap->vol)))exit(-1);
    brnidx=ms.get_brnidx(num_regions);
    //printf("brnidx\n");for(i=0;i<num_regions;i++)printf("%d ",brnidx[i]);printf("\n");
    }
printf("num_regions=%d ap->vol=%d fs->nlines=%d dp->volall=%d dp->filetypeall=%d\n",num_regions,ap->vol,fs->nlines,dp->volall,
    dp->filetypeall);

if(!(temp_float=(float*)malloc(sizeof*temp_float*ap->vol))) {
    printf("fidlError: Unable to malloc temp_float\n");
    exit(-1);
    }
for(i=0;i<ap->vol;i++) temp_float[i] = 0.;
if(!(temp_double=(double*)malloc(sizeof*temp_double*ap->vol))) {
    printf("fidlError: Unable to malloc temp_double\n");
    exit(-1);
    }
if(!(temp_double2=(double*)malloc(sizeof*temp_double2*ap->vol))) {
    printf("fidlError: Unable to malloc temp_double2\n");
    exit(-1);
    }
if(!(dfstack=(double*)malloc(sizeof*dfstack*num_regions))) {
    printf("fidlError: Unable to malloc dfstack\n");
    exit(-1);
    }
if(!(fbf=find_regions_by_file(num_tc_files,fs->nframes,dp->tdim,fs->frames))) exit(-1);
if(!num_region_files) {
    if(!(ifh=init_ifh(4,ap->xdim,ap->ydim,ap->zdim,1,ap->voxel_size[0],ap->voxel_size[1],ap->voxel_size[2],bigendian,
        ap->centerf,ap->mmppixf))) exit(-1);
    } 
if(lcROCarea || num_region_files) {
    if(!(wstack=(double*)malloc(sizeof*wstack*tlengthX*num_regions))) {
        printf("fidlError: Unable to malloc wstack\n");
        exit(-1);
        }
    /*for(i=0;i<tlengthX*num_regions;i++) wstack[i]=0.;*/
    if(!(indx=(size_t*)malloc(sizeof*indx*fs->nlines))) {
        printf("fidlError: Unable to malloc indx\n");
        exit(-1);
        }
    if(!(pval=(float*)malloc(sizeof*pval*num_regions))) {
        printf("fidlError: Unable to malloc pval\n");
        exit(-1);
        }
    }    
if(lcROCarea) {
    if(!(ROCarea=(double*)malloc(sizeof*ROCarea*num_regions))) {
        printf("fidlError: Unable to malloc ROCarea\n");
        exit(-1);
        }
    }
if(!(lrs=(double*)malloc(sizeof*lrs*nmodels*num_regions))) {
    printf("fidlError: Unable to malloc lrs\n");
    exit(-1);
    }
for(i=0;i<nmodels*num_regions;i++) lrs[i]=(double)UNSAMPLED_VOXEL;


//START180108
if(!(LL0=(double*)malloc(sizeof*LL0*nmodels*num_regions))) {
    printf("fidlError: Unable to malloc LL0\n");
    exit(-1);
    }
for(i=0;i<nmodels*num_regions;i++) LL0[i]=(double)UNSAMPLED_VOXEL;
if(!(LLm=(double*)malloc(sizeof*LLm*nmodels*num_regions))) {
    printf("fidlError: Unable to malloc LLm\n");
    exit(-1);
    }
for(i=0;i<nmodels*num_regions;i++) LLm[i]=(double)UNSAMPLED_VOXEL;

//START180228
if(!(w0=(double*)malloc(sizeof*w0*lengthX[0]*num_regions))){
    printf("fidlError: Unable to malloc w0\n");
    exit(-1);
    }
for(i=0;i<lengthX[0]*num_regions;i++)w0[i]=(double)UNSAMPLED_VOXEL;

//START180416
if(!(w01=(double*)malloc(sizeof*w01*2*num_regions))){
    printf("fidlError: Unable to malloc w01\n");
    exit(-1);
    }
for(i=0;i<2*num_regions;i++)w01[i]=(double)UNSAMPLED_VOXEL;


if(!(X=(double*)malloc(sizeof*X*lengthX[0]*fs->nlines))){
    printf("fidlError: Unable to malloc X\n");
    exit(-1);
    }

//START180112
if(!(X0=(double*)malloc(sizeof*X*fs->nlines))){
    printf("fidlError: Unable to malloc X0\n");
    exit(-1);
    }
for(i=0;i<fs->nlines;i++)X0[i]=1.;

if(!(y=(double*)malloc(sizeof*y*fs->nlines))){
    printf("fidlError: Unable to malloc y\n");
    exit(-1);
    }
if(!oldguts){
    if(!lr.logreg0((size_t)lengthX[0],(size_t)fs->nlines))exit(-1);
    }
else{
    if(!(AX=(double*)malloc(sizeof*AX*lengthX[0]*fs->nlines))) {
        printf("fidlError: Unable to malloc AX\n");
        exit(-1);
        }
    if(!(w=(double*)malloc(sizeof*w*lengthX[0]))) {
        printf("fidlError: Unable to malloc w\n");
        exit(-1);
        }
    if(!(wTX=(double*)malloc(sizeof*wTX*fs->nlines))) {
        printf("fidlError: Unable to malloc wTX\n");
        exit(-1);
        }
    if(!(z=(double*)malloc(sizeof*z*fs->nlines))) {
        printf("fidlError: Unable to malloc z\n");
        exit(-1);
        }
    if(!(tau=(double*)malloc(sizeof*tau*lengthX[0]))) {
        printf("fidlError: Unable to malloc tau\n");
        exit(-1);
        }
    if(!(residual=(double*)malloc(sizeof*residual*fs->nlines))) {
        printf("fidlError: Unable to malloc residual\n");
        exit(-1);
        }
    }


if(!driver_file) {
    for(n0=n1=i=0;i<fs->nlines;i++) {
        for(temp=-1.,n0++,j=0;j<half;j++) {
            for(k=0;k<tcs->each[j];k++) {
                if(frames_cond[i] == tcs->tc[j][k][0]) {
                    temp = 1.;
                    n0--;
                    n1++;
                    break;
                    }
                }
            }
        y[i] = temp;
        }
    printf("%s mapped to 1->p(1)=1    %d trials\n%s mapped to -1->p(-1)=0.    %d trials\n",
        tc_names->files[0],n1,tc_names->files[1],n0);
    ll_null = log2*(double)fs->nlines;
    }
else {
    if(!(lry=create_LRy(num_regions,data->nsubjects))) exit(-1);
    }

if(!(w1=write1_init()))exit(-1);
w1->filetype=dp->filetypeall;
w1->swapbytes=0;
w1->temp_float=temp_float;
if(dp->filetypeall==(int)IMG){
    w1->lenbrain=num_regions;
    w1->how_many=ap->vol;
    if(!(ifh=read_ifh(tc_files->files[0],(Interfile_header*)NULL)))exit(-1);
    ifh->dim1=ap->xdim;ifh->dim2=ap->ydim;ifh->dim3=ap->zdim;ifh->dim4=1;
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
    w1->tdim=(int64_t)1;
    }
else if(dp->filetypeall==(int)NIFTI){
    w1->file_hdr=tc_files->files[0];
    }

if(!logregwts_file) {

    //START180416
    //endloop = nmodels==1 ? nindvar : nindvar-1;

    if(driver_file)if(!(assign_LRy(data,fbf->num_regions_by_file,num_regions,lry)))exit(-1);
    printf("nindvar=%d endloop=%d\n",nindvar,endloop);fflush(stdout);


    #if 0
    for(lrsinc=wstackinc=m=m1=0,o=nindvar;o>=endloop;o--) {
        c = gsl_combination_alloc(nindvar,o);
    #endif
    //START180416
    //for(lrsinc=wstackinc=m=m1=0,o=0;o<endloop;o++) {
    //START180418
    for(lrsinc=wstackinc=m=m1=0,o=(nindvar>1&&indonly)?2:0;o<endloop;o++) {
        m=m0[o];

        c = gsl_combination_alloc(nindvar,superbird[o]);

        gsl_combination_init_last(c);
        do {

            printf("{");
            gsl_combination_fprintf(stdout,c," %u");
            printf(" }\n");
            printf("df[%d]=%d lengthX[%d]=%d\n",m,df[m],m,lengthX[m]);
            fflush(stdout);



            if(wstack) wstackgsl = gsl_matrix_view_array(&wstack[wstackinc],lengthX[m],num_regions);
            if(!driver_file) {
                for(j=0;j<num_regions;j++,m1++) {
                    if(!(j%modulo)) fprintf(stdout,"Processing voxel %d\n",j);
                    if(!(assignXtrblockgsl(data,fbf->num_regions_by_file,num_regions,1,lcconstant,j,0,sindex,fs,X,s,lengthX[m])))
                        exit(-1);
                    if(!s->uns_count) {
                        ll_old = ll_null_new = ll_null;
                        ll=logreg_guts(fs->nlines,lengthX[m],w,X,AX,y,z,wTX,tau,residual,ll_old);
                        if(lcconstant) for(ll_null_new=0.,i=0;i<fs->nlines;i++) ll_null_new += log(1.+exp(-y[i]*w[0]));
                        lrs[m1] = 2.*(-ll + ll_null_new);
                        if(wstack) for(i=0;i<lengthX[m];i++) gsl_matrix_set(&wstackgsl.matrix,i,j,w[i]); 
                        }
                    }
                }
            else {

                //printf("here0 m1=%d lrsinc=%d\n",m1,lrsinc);

                for(j=0;j<num_regions;j++,m1++) {
                    if(!(j%modulo)) fprintf(stdout,"Processing voxel %d\n",j);
                    //printf("lry->n0[%d]=%d lry->n1[%d]=%d\n",j,lry->n0[j],j,lry->n1[j]);
                    if(lry->n0[j] && lry->n1[j]) {

                        //assignXandy(lry,data->x[0],data->npoints,j,lcconstant,y,X,lengthX[m],c,o);
                        //START180416
                        assignXandy(lry,data->x[0],data->npoints,j,lcconstant,y,X,lengthX[m],c,lengthX[m]-1);
                            //This might break if lcconstant=0

                        //START180110
                        //printf("computing ll_null\n");
                        if(lcconstant){
                            ll_old=ll_null=oldguts?logreg_guts(lry->nindex[j],1,w,X0,AX,y,z,wTX,tau,residual,ll_old)
                                                  :lr.logreg_guts(lry->nindex[j],1,X0,y,ll_old,lambda);
                            }
                        else{
                            ll_old = ll_null = log2*(double)lry->nindex[j];
                            }
                        //printf("computing ll\n");
                        ll=oldguts?logreg_guts(lry->nindex[j],lengthX[m],w,X,AX,y,z,wTX,tau,residual,ll_old)
                                  :lr.logreg_guts(lry->nindex[j],lengthX[m],X,y,ll_old,lambda);
                        LL0[m1]=ll_null;LLm[m1]=ll;

                        //printf("here0 ll_null=%f ll=%f\n",ll_null,ll);

                        if(ll<ll_null){ //ll less positive than ll_null (intercept only model) since we aren't taking negatives
                            lrs[m1] = 2.*(-ll+ll_null);

                            //START180228
                            //if(o==nindvar){
                            //START180416
                            if(o==0||o==2){
                                w=lr.get_w();
                                dptr=!o?w0:w01; 
                                for(k=j,i=0;i<lengthX[m];i++,k+=num_regions)dptr[k]=w[i];
                                }

                            }
                        else{//something went wrong
                            lrs[m1] = (double)UNSAMPLED_VOXEL;
                            }

                        //START180112
                        //printf("j=%d lry->n0[%d]=%d lry->n1[%d]=%d ll_null=%f ll=%f 2.*(-ll+ll_null)=%f\n",j,j,lry->n0[j],j,
                        //    lry->n1[j],ll_null,ll,2.*(-ll+ll_null));
                        //exit(-1);
 

                        //if(lrs[m1]>10.3){printf("ll=%f ll_null=%f lrs[%d]=%f\n",ll,ll_null,m1,lrs[m1]);fflush(stdout);}
                        //if(m1==32033){printf("lengthX[%d]=%d w[0]=%f ll=%f ll_null=%f lrs[%d]=%f\n",
                        //    m,lengthX[m],w[0],ll,ll_null,m1,lrs[m1]);fflush(stdout);exit(-1);}
                        //printf("lengthX[%d]=%d w[0]=%f ll=%f ll_null=%f lrs[%d]=%f\n",
                        //    m,lengthX[m],w[0],ll,ll_null,m1,lrs[m1]);fflush(stdout);

                        if(wstack) for(i=0;i<lengthX[m];i++) gsl_matrix_set(&wstackgsl.matrix,i,j,w[i]); 
                        }
                    }
                }
            if(!num_region_files) {
                strcpy(string,root);

                //if(o==nindvar)strcat(string,"FULL");
                //START180416
                if(!o)strcat(string,"FULL");

                cptr = gsl_combination_data(c);


                #if 0
                for(j=0;j<o;j++) {
                    strcat(string,"_");
                    strcat(string,data->colptr[cptr[j]+1]);
                    }
                #endif
                //START180417
                ck = gsl_combination_k(c);
                for(j0=0;j0<ck;j0++) {
                    strcat(string,"_");
                    strcat(string,data->colptr[cptr[j0]+1]);
                    }


                if((l0=strlen(string))>(size_t)limits.name_max){
                    printf("fidlWarning: string = %s\n    has %zd characters which exceeds the limit of %ld",string,l0,
                        limits.name_max);
                    }

                //printf("here1 m1=%d lrsinc=%d\n",m1,lrsinc);


                //KEEP
                #if 0
                sprintf(filename,"%s_lrs%s%s",string,ap->str,Fileext[dp->filetypeall]);
                for(j=0;j<num_regions;j++) temp_float[brnidx[j]] = (float)lrs[lrsinc+j];
                if(dp->filetypeall==(int)IMG){
                    w1->temp_double=&lrs[lrsinc];
                    ifh->dim4 = 1;
                    ifh->df1 = (float)df[m];
                    ifh->bigendian=0;
                    }
                if(!write1(filename,w1))exit(-1);
                printf("Likelihood ratio statistic written to %s\n",filename);
                sprintf(filename,"%s_LL0%s%s",string,ap->str,Fileext[dp->filetypeall]);
                for(j=0;j<num_regions;j++) temp_float[brnidx[j]] = (float)LL0[lrsinc+j];
                if(dp->filetypeall==(int)IMG){
                    w1->temp_double=&LL0[lrsinc];
                    ifh->dim4 = 1;
                    ifh->df1 = (float)df[m];
                    ifh->bigendian=0;
                    }
                if(!write1(filename,w1))exit(-1);
                printf("LL0 written to %s\n",filename);
                sprintf(filename,"%s_LLm%s%s",string,ap->str,Fileext[dp->filetypeall]);
                for(j=0;j<num_regions;j++) temp_float[brnidx[j]] = (float)LLm[lrsinc+j];
                if(dp->filetypeall==(int)IMG){
                    w1->temp_double=&LLm[lrsinc];
                    ifh->dim4 = 1;
                    ifh->df1 = (float)df[m];
                    ifh->bigendian=0;
                    }
                if(!write1(filename,w1))exit(-1);
                printf("LLm written to %s\n",filename);
                #endif


                sprintf(filename,"%s_lrszstat%s%s",string,ap->str,Fileext[dp->filetypeall]);
                for(j=0;j<num_regions;j++) dfstack[j] = (double)df[m];
                x2_to_z_asym(&lrs[lrsinc],temp_double,num_regions,dfstack);
                for(j=0;j<num_regions;j++) temp_float[brnidx[j]] = (float)temp_double[j];
                if(dp->filetypeall==(int)IMG)w1->temp_double=temp_double;
                if(!write1(filename,w1))exit(-1);
                printf("Z statistic written to %s\n",filename);


                //START180416
                //if(o==2){
                //START180417
                if(o==2||nindvar==1){
                    dptr=!o?w0:w01;

                    sprintf(filename,"%s_lrszstat_signed%s%s",string,ap->str,Fileext[dp->filetypeall]);

                    #if 0
                    lrsinc = num_regions;
                    for(j=0;j<num_regions;j++)temp_float[brnidx[j]]=
                        temp_double[j]==(double)UNSAMPLED_VOXEL?(float)UNSAMPLED_VOXEL:
                        (float)(dptr[lrsinc+j]<0.?(temp_double[j]=-temp_double[j]):temp_double[j]);
                    #endif
                    //START180418
                    for(i=num_regions,j=0;j<num_regions;j++,i++)temp_float[brnidx[j]]=
                        temp_double[j]==(double)UNSAMPLED_VOXEL?(float)UNSAMPLED_VOXEL:
                        (float)(dptr[i]<0.?(temp_double[j]=-temp_double[j]):temp_double[j]);

                    if(dp->filetypeall==(int)IMG)w1->temp_double=temp_double;
                    if(!write1(filename,w1))exit(-1);
                    printf("Z statistic written to %s\n",filename);
                    }


                #if 0
                //START180228
                if(o==nindvar){
                    sprintf(filename,"%sFULL_constant_wts%s%s",root,ap->str,Fileext[dp->filetypeall]);
                    for(k=j=0;j<num_regions;j++,k++) temp_float[brnidx[j]] = (float)w0[k];
                    if(dp->filetypeall==(int)IMG){
                        w1->temp_double=w0;
                        ifh->dim4 = 1;
                        ifh->df1 = (float)df[m];
                        ifh->bigendian=0;
                        }
                    if(!write1(filename,w1))exit(-1);
                    printf("Weight written to %s\n",filename);
                    cptr = gsl_combination_data(c);
                    for(i=0;i<o;i++) {
                        sprintf(filename,"%sFULL_%s_wts%s%s",root,data->colptr[cptr[i]+1],ap->str,Fileext[dp->filetypeall]);
                        for(j=0;j<num_regions;j++,k++) temp_float[brnidx[j]] = (float)w0[k];
                        if(dp->filetypeall==(int)IMG)w1->temp_double=&w0[(i+1)*num_regions];
                        if(!write1(filename,w1))exit(-1);
                        printf("Weight written to %s\n",filename);
                        }
                    }
                #endif
                //START180416
                if(o==0||o==2){
                    dptr=!o?w0:w01;

                    //sprintf(filename,"%s%s_constant_wts%s%s",root,o==0?"FULL":"",ap->str,Fileext[dp->filetypeall]);
                    //START180418
                    cptr = gsl_combination_data(c);
                    sprintf(filename,"%s%s_constant_wts%s%s",root,o==0?"FULL":data->colptr[cptr[0]+1],ap->str,
                        Fileext[dp->filetypeall]);

                    for(k=j=0;j<num_regions;j++,k++) temp_float[brnidx[j]] = (float)dptr[k];
                    if(dp->filetypeall==(int)IMG){
                        w1->temp_double=dptr;
                        ifh->dim4 = 1;
                        ifh->df1 = (float)df[m];
                        ifh->bigendian=0;
                        }
                    if(!write1(filename,w1))exit(-1);
                    printf("Weight written to %s\n",filename);

                    //START180418
                    //cptr = gsl_combination_data(c);

                    #if 0
                    for(i=0;i<df[m];i++) {
                        sprintf(filename,"%s%s_%s_wts%s%s",root,o==0?"FULL":"",data->colptr[cptr[i]+1],ap->str,
                            Fileext[dp->filetypeall]);
                        for(j=0;j<num_regions;j++,k++) temp_float[brnidx[j]] = (float)dptr[k];
                        if(dp->filetypeall==(int)IMG)w1->temp_double=&dptr[(i+1)*num_regions];
                        if(!write1(filename,w1))exit(-1);
                        printf("Weight written to %s\n",filename);
                        }
                    #endif
                    //START180417
                    ck = gsl_combination_k(c);
                    for(i0=0;i0<ck;i0++) {

                        //sprintf(filename,"%s%s_%s_wts%s%s",root,o==0?"FULL":"",data->colptr[cptr[i0]+1],ap->str,
                        //    Fileext[dp->filetypeall]);
                        //START180418
                        sprintf(filename,"%s%s_%s_wts%s%s",root,o==0?"FULL":data->colptr[cptr[0]+1],data->colptr[cptr[i0]+1],ap->str,
                            Fileext[dp->filetypeall]);

                        for(j=0;j<num_regions;j++,k++) temp_float[brnidx[j]] = (float)dptr[k];
                        if(dp->filetypeall==(int)IMG)w1->temp_double=&dptr[(i0+1)*num_regions];
                        if(!write1(filename,w1))exit(-1);
                        printf("Weight written to %s\n",filename);
                        }


                    }




                #if 0
                if(wstack) {
                    sprintf(filename,"%s_wts%s.4dfp.img",string,ap->str);
                    if(!(fp = fopen_sub(filename,"w"))) exit(-1);
                    for(j=0;j<lengthX[m];j++) {
                        for(n=wstackinc,i=0;i<num_regions;i++,n++) temp_float[ms->brnidx[i]] = (float)wstack[n];
                        if(!(fwrite_sub(temp_float,sizeof(float),(size_t)ap->vol,fp,swapbytesout))) {
                            printf("Error writing to %s. Are you out of memory? Abort!\n",filename);
                            exit(-1);
                            }
                        }
                    fclose(fp);
                    printf("Logistic regression weights written to %s\n",filename);
                    min_and_max_init(&ifh->global_min,&ifh->global_max);
                    min_and_max_doublestack(&wstack[wstackinc],lengthX[m]*num_regions,&ifh->global_min,&ifh->global_max);
                    ifh->dim4 = lengthX[m];
                    if(!write_ifh(filename,ifh,(int)FALSE)) exit(-1);
                    }
                #endif
                }



            lrsinc+=num_regions;
            wstackinc+=lengthX[m++]*num_regions; //******* m incremented HERE **********
            } while(gsl_combination_prev(c)==GSL_SUCCESS);
        gsl_combination_free(c);

        } //FOR


//endloop = nmodels==1 ? nindvar : nindvar-1;

    //if(nindvar>1) {
    //START180419
    if(nindvar>1&&!indonly) {

        //c = gsl_combination_alloc(nindvar,endloop);
        //START180417
        c = gsl_combination_alloc(nindvar,nindvar-1);

        gsl_combination_init_last(c);
        lrsinc = num_regions;
        i=0;
        do {

            //for(j=0;j<num_regions;j++) temp_double[j] = lrs[j] - lrs[lrsinc+j];
            //START180108
            #if 0
            for(j=0;j<num_regions;j++){
                if(lrs[j]==(double)UNSAMPLED_VOXEL||lrs[lrsinc+j]==(double)UNSAMPLED_VOXEL)temp_double[j]=(double)UNSAMPLED_VOXEL;
                else temp_double[j] = lrs[j] - lrs[lrsinc+j];
                }
            #endif
            //START180302
            for(j=0;j<num_regions;j++)
                temp_double[j]=(lrs[j]==(double)UNSAMPLED_VOXEL||lrs[lrsinc+j]==(double)UNSAMPLED_VOXEL)?(double)UNSAMPLED_VOXEL:
                    lrs[j] - lrs[lrsinc+j];

            //START180417
            //cptr = gsl_combination_data(c);

            sprintf(string,"%s_%s",root,data->colptr[i+1]);
            if(dp->filetypeall==(int)IMG){
                w1->temp_double=temp_double;
                ifh->dim4 = 1;
                ifh->df1=1.;
                ifh->bigendian=0;
                }

            //KEEP
            #if 0
            sprintf(filename,"%s_wrtFULLlrs%s%s",string,ap->str,Fileext[dp->filetypeall]);
            for(j=0;j<num_regions;j++)temp_float[brnidx[j]]=(float)temp_double[j];
            if(!write1(filename,w1))exit(-1);
            printf("Likelihood ratio statistic written to %s\n",filename);
            #endif

            sprintf(filename,"%s_wrtFULLlrszstat%s%s",string,ap->str,Fileext[dp->filetypeall]);
            for(j=0;j<num_regions;j++) dfstack[j] = 1.;
            x2_to_z_asym(temp_double,temp_double2,num_regions,dfstack);
            for(j=0;j<num_regions;j++)temp_float[brnidx[j]]=(float)temp_double2[j];
            if(dp->filetypeall==(int)IMG)w1->temp_double=temp_double2;
            if(!write1(filename,w1))exit(-1);
            printf("Z statistic written to %s\n",filename);

            //START180302
            sprintf(filename,"%s_wrtFULLlrszstat_signed%s%s",string,ap->str,Fileext[dp->filetypeall]);
            for(j=0;j<num_regions;j++)temp_float[brnidx[j]]=
                temp_double[j]==(double)UNSAMPLED_VOXEL?(float)UNSAMPLED_VOXEL:
                (w0[lrsinc+j]<0.?(float)(temp_double2[j]=-temp_double2[j]):(float)temp_double2[j]);
            if(!write1(filename,w1))exit(-1);
            printf("Z statistic written to %s\n",filename);

            lrsinc += num_regions;
            i++;
            } while(gsl_combination_prev(c)==GSL_SUCCESS);
        gsl_combination_free(c);
        }
    }

/*START*/
#if 0
/*if(ROCarea_name || num_region_files) {*/
if(lcROCarea || num_region_files) {

    if(logregwts_file) {
        if(!(logregwts_ifh = read_ifh(logregwts_file))) exit(-1);
        if((j=logregwts_ifh->dim1*logregwts_ifh->dim2*logregwts_ifh->dim3) != ap->vol) {
            printf("Error: %s vol=%d  ap->vol=%d  Must match. Abort!\n",logregwts_file,j,ap->vol);
            exit(-1);
            }
        if(logregwts_ifh->dim4 != lengthX[0]) {
            printf("Error: %s logregwts_ifh->dim4=%d lengthX[0]=%d  Must match. Abort!\n",logregwts_file,logregwts_ifh->dim4,
                lengthX[0]);
            exit(-1);
            }
        if(!(logregwts_stack=malloc(sizeof*logregwts_stack*ap->vol*lengthX[0]))) {
            printf("Error: Unable to malloc logregwts_stack\n");
            exit(-1);
            }
        if(!readstack(logregwts_file,(float*)logregwts_stack,sizeof(float),(size_t)(ap->vol*lengthX[0]),SunOS_Linux)) exit(-1);
        for(k=i=n=0;n<lengthX[0];n++,i+=ap->vol) {

            //for(j=0;j<num_regions;j++,k++) wstack[k] = (double)logregwts_stack[i+ms->brnidx[j]];
            //START171106
            for(j=0;j<num_regions;j++,k++) wstack[k] = (double)logregwts_stack[i+brnidx[j]];

            }
        printf("Starting computation of lry\n");
        if(!(assign_LRy(num_tc_files,fbf->num_regions_by_file,sn,num_regions,lcconstant,lry,td->ncov[0]))) exit(-1);
        }
    printf("Starting computation of ROC. Please be patient.\n");
    if(num_region_files) {
        ll = (double)(df[0])/2.;
        if(!(fp = fopen_sub(regional_name,"w"))) exit(-1);
        fprintf(fp,"%s mapped to 1->p(1)=1    %d trials\n%s mapped to -1->p(-1)=0.    %d trials\n\n",tc_names->files[0],n1,
            tc_names->files[1],n0);
        }
    temp = (double)(n0*n1);
    nlines = fs->nlines;
    if(!(indx=malloc(sizeof*indx*nlines))) {
        printf("Error: Unable to malloc indx\n");
        exit(-1);
        }
    endloop = nmodels==1 ? nindvar : nindvar-1;
    if(driver_file) if(!(assign_LRy(num_tc_files,fbf->num_regions_by_file,sn,num_regions,lcconstant,lry,td->ncov[0]))) exit(-1);
    for(lrsinc=wstackinc=m=m1=0,o=nindvar;o>=endloop;o--) {
        c = gsl_combination_alloc(nindvar,o);
        for(gsl_combination_init_last(c);gsl_combination_next(c)==GSL_SUCCESS;lrsinc+=num_regions,
            wstackinc+=lengthX[m++]*num_regions) {
            wstackgsl = gsl_matrix_view_array(&wstack[wstackinc],lengthX[m],num_regions);
            for(j=0;j<num_regions;j++,m1++) {
                ROCarea[j] = (double)UNSAMPLED_VOXEL;
                if(!driver_file) {
                    assignXtrblockgsl(num_tc_files,fbf->num_regions_by_file,sn,num_regions,1,lcconstant,j,0,sindex,fs,X,s,lengthX[m]);
                    if(s->uns_count) continue;
                    }
                else {
                    if(!lry->n0[j] || !lry->n1[j]) continue; 
                    assignXandy(lry,indvar,td->ncov_per_line[0],j,lcconstant,y,X,lengthX[m],c,o);
                    nlines = lry->nindex[j];
                    temp = (double)(lry->n0[j]*lry->n1[j]);
                    }
                gsl_matrix_view Xgsl = gsl_matrix_view_array(X,nlines,lengthX[m]);
                gsl_vector_view wgsl = gsl_matrix_column(&wstackgsl.matrix,j);
                gsl_vector_view wTXgsl = gsl_vector_view_array(wTX,nlines);
                gsl_blas_dgemv(CblasNoTrans,1.0,&Xgsl.matrix,&wgsl.vector,0.0,&wTXgsl.vector);
                for(i=0;i<nlines;i++) z[i] = 1./(1.+exp(-y[i]*wTX[i]));
                gsl_sort_index(indx,wTX,1,nlines);
                for(nn=n=0,i=nlines;--i>=0;) {
                    if((int)y[indx[i]]==1) {
                        n++;
                        }
                    else {
                        nn += n;
                        }
                    }
                ROCarea[j] = (double)nn/temp;
                if(num_region_files) {
                    pval[j] = !lrs[m1] ? 1.0 : (float)gammq(ll,lrs[m1]/2.);
                    fprintf(fp,"    REGION : %s %d\n",rbf->region_names_ptr[j],rbf->nvoxels_region[j]);
                    fprintf(fp,"        likelihood ratio statistic %f\n",lrs[m1]);
                    fprintf(fp,"        significance probability   %.10f\n",pval[j]);
                    fprintf(fp,"        area under ROC             %.4f\n",ROCarea[j]);
                    fprintf(fp,"        weights       "); for(i=0;i<lengthX[m];i++) fprintf(fp,"%f ",gsl_vector_get(&wgsl.vector,i)); 
                    fprintf(fp,"\n");
                    fprintf(fp,"trial trialtype data = inner product p(y|data,weights)\n");
                    for(i=fs->nlines;--i>=0;) {
                        fprintf(fp,"%d %d\t",indx[i],(int)y[indx[i]]);
                        for(k=0;k<lengthX[m];k++) fprintf(fp,"%f ",gsl_matrix_get(&Xgsl.matrix,indx[i],k));
                        fprintf(fp,"= %f %f\n",wTX[i],z[indx[i]]);
                        }
                    }
                }
            }
        }
    if(num_region_files) {
        fclose(fp);
        printf("Output written to %s\n",regional_name);
        }
    }

#if 0
/*START CHECK*/
y[1] = -1;
y[2] = -1;
y[3] = 1;
y[4] = -1;
y[5] = -1;
y[6] = -1;
y[7] = 1;
y[8] = 1;
y[9] = -1;
y[10] = 1;
n0 = 6;
n1 = 4;
temp = 0.;
for(l=n0,i=0;i<10;i++) {
    for(m=n=0,k=i+1;k<=10;k++) y[k] == -1 ? m++ : n++;
    if(m != l) temp += (double)(l-m)/(double)n0 * (double)n/(double)n1;
    l = m;
    }
printf("temp=%f\n",temp);
exit(-1);
/*END CHECK*/
#endif

if(!num_region_files) {
    #if 0
    if(ROCarea_name) {
        if((str_ptr=strstr(ROCarea_name,".4dfp.img"))) *str_ptr = 0;
        sprintf(filename,"%s%s.4dfp.img",ROCarea_name,ap->str);
    #endif
    if(lcROCarea) {
        sprintf(filename,"%s_ROC%s.4dfp.img",root,ap->str);


        if(!(fp = fopen_sub(filename,"w"))) exit(-1);

        //for(j=0;j<num_regions;j++) temp_float[ms->brnidx[j]] = (float)ROCarea[j];
        //START171106
        for(j=0;j<num_regions;j++) temp_float[brnidx[j]] = (float)ROCarea[j];

        if(!(fwrite_sub(temp_float,sizeof(float),(size_t)ap->vol,fp,swapbytesout))) {
            printf("Error writing to %s. Are you out of memory? Abort!\n",filename);
            exit(-1);
            }
        fclose(fp);
        printf("Area under ROC written to %s\n",filename);
        min_and_max_init(&ifh->global_min,&ifh->global_max);
        min_and_max_doublestack(ROCarea,num_regions,&ifh->global_min,&ifh->global_max);
        ifh->dim4 = 1;
        if(!write_ifh(filename,ifh,(int)FALSE)) exit(-1);
        }
    }
#endif
/*END*/


if(lccleanup) {
    sprintf(filename,"rm -rf %s",scratchdir);
    if(system(filename) == -1) printf("Error: unable to %s\n",filename);
    }
}


void assignXandy(LRy *lry,double *indvar,int nindvar_per_line,int j,int lcconstant,double *y,double *X,int lengthX,
    gsl_combination *c,int k){
    int m,mm,i,ii,iii;
    gsl_matrix_view Xgsl = gsl_matrix_view_array(X,lry->nindex[j],lengthX);
    size_t *cptr = gsl_combination_data(c);
    for(mm=lry->starti[j],m=0;m<lry->nindex[j];m++,mm++) {
        y[m] = lry->y[mm];
        for(iii=lry->index[mm]*nindvar_per_line,ii=lcconstant,i=0;i<k;i++,ii++) gsl_matrix_set(&Xgsl.matrix,m,ii,indvar[iii+cptr[i]]);
        }
    if(lcconstant) for(m=0;m<lry->nindex[j];m++) gsl_matrix_set(&Xgsl.matrix,m,0,1.);
    }


#if 0
double logreg_guts(int nlines,int lengthX,double *w,double *X,double *AX,double *y,double *z,double *wTX,double *tau,double *residual,
    double ll_old)
{
    int i,k,kk,increment,iterations;
    double ll,temp,A;

    //START170810
    //gsl_vector_view row;

    gsl_matrix_view Xgsl = gsl_matrix_view_array(X,nlines,lengthX);
    gsl_matrix_view AXgsl = gsl_matrix_view_array(AX,nlines,lengthX);
    gsl_vector_view wgsl = gsl_vector_view_array(w,lengthX);
    gsl_vector_view wTXgsl = gsl_vector_view_array(wTX,nlines);
    gsl_vector_view zgsl = gsl_vector_view_array(z,nlines);
    gsl_vector_view taugsl = gsl_vector_view_array(tau,lengthX);
    gsl_vector_view residualgsl = gsl_vector_view_array(residual,nlines);
    gsl_vector_set_zero(&wgsl.vector);
    for(i=0;i<lengthX;i++) w[i]=0.;
    for(increment=iterations=0;increment<(int)MAXINC && iterations<(int)MAXIT;iterations++) {

        /*printf("begin w="); for(i=0;i<lengthX;i++) printf("%f ",w[i]); printf("\n");*/

        gsl_blas_dgemv(CblasNoTrans,1.0,&Xgsl.matrix,&wgsl.vector,0.0,&wTXgsl.vector);
        for(kk=ll=0.,i=0;i<nlines;i++) {
            temp = 1./(1.+exp(wTX[i]));
            A = temp*(1-temp);

            /*printf("temp=%f A=%f ",temp,A);*/

            temp = 1. + exp(-y[i]*wTX[i]);

            /*printf("temp=%f ",temp);*/

            ll += log(temp);
            z[i] = A*wTX[i] + (1.-1./temp)*y[i];

            /*printf("z[%d]=%f\n",i,z[i]);*/

            for(k=0;k<lengthX;k++,kk++) AX[kk] = X[kk]*A;            
            }

        printf("logreg_guts ll=%f\n",ll);

        gsl_linalg_QR_decomp(&AXgsl.matrix,&taugsl.vector);
        gsl_linalg_QR_lssolve(&AXgsl.matrix,&taugsl.vector,&zgsl.vector,&wgsl.vector,&residualgsl.vector);

        /*printf("w="); for(i=0;i<lengthX;i++) printf("%f ",w[i]); printf("\n");*/

        //increment = fabs(ll-ll_old)<=(double)TOL ? ++increment : 0;
        //START170810
        increment = fabs(ll-ll_old)<=(double)TOL ? increment+1 : 0;

        ll_old = ll;
        }
    /*printf("bottom w="); for(i=0;i<lengthX;i++) printf("%f ",w[i]); printf("\n");
    exit(-1);*/

    return ll;
}
#endif
//START170821
double logreg_guts(int nlines,int lengthX,double *w,double *X,double *AX,double *y,double *z,double *wTX,double *tau,double *residual,
    double ll_old){
    int i,k,kk,increment,iterations;
    double ll,temp,A;
    gsl_matrix_view Xgsl = gsl_matrix_view_array(X,nlines,lengthX);
    gsl_matrix_view AXgsl = gsl_matrix_view_array(AX,nlines,lengthX);
    gsl_vector_view wgsl = gsl_vector_view_array(w,lengthX);
    gsl_vector_view wTXgsl = gsl_vector_view_array(wTX,nlines);
    gsl_vector_view zgsl = gsl_vector_view_array(z,nlines);
    gsl_vector_view taugsl = gsl_vector_view_array(tau,lengthX);
    gsl_vector_view residualgsl = gsl_vector_view_array(residual,nlines);

    gsl_vector_set_zero(&wgsl.vector);
    //for(i=0;i<lengthX;i++) w[i]=0.;

    #if 0
    printf("nlines=%d lengthX=%d\n",nlines,lengthX);
    printf("X\n");
    for(kk=i=0;i<nlines;i++){
        printf("i=%d\n",i);
        for(k=0;k<lengthX;k++,kk++)printf("%f ",X[kk]);
        printf("\n");
        }
    printf("\n");
    #endif

    //printf("w\n");for(k=0;k<lengthX;k++)printf("%f ",w[k]);printf("\n");

    for(increment=iterations=0;;){
        gsl_blas_dgemv(CblasNoTrans,1.0,&Xgsl.matrix,&wgsl.vector,0.0,&wTXgsl.vector);
        for(kk=ll=0.,i=0;i<nlines;i++) {
            temp = 1./(1.+exp(wTX[i]));
            A = temp*(1-temp);
            temp = 1. + exp(-y[i]*wTX[i]);
            ll += log(temp);
            z[i] = A*wTX[i] + (1.-1./temp)*y[i];
            for(k=0;k<lengthX;k++,kk++) AX[kk] = X[kk]*A;
            }

        #if 0
        printf("AX\n");
        for(kk=i=0;i<nlines;i++){
            for(k=0;k<lengthX;k++,kk++)printf("%f ",AX[kk]);
            printf("\n");
            }
        printf("\n");
        #endif

        //if((increment = ll_old-ll<=(double)TOL ? increment+1 : 0)<(int)MAXINC && ++iterations<(int)MAXIT){
        //START180112
        if((increment = fabs(ll_old-ll)<=(double)TOL ? increment+1 : 0)<(int)MAXINC && ++iterations<(int)MAXIT){

            gsl_linalg_QR_decomp(&AXgsl.matrix,&taugsl.vector);
            gsl_linalg_QR_lssolve(&AXgsl.matrix,&taugsl.vector,&zgsl.vector,&wgsl.vector,&residualgsl.vector);
            ll_old = ll;

            //printf("w\n");for(k=0;k<lengthX;k++)printf("%f ",w[k]);printf("\n");

            }
        else{
            break;
            }

        //printf("ll=%f increment=%d iterations=%d\n",ll,increment,iterations);

        }

    //printf("end w\n");for(k=0;k<lengthX;k++)printf("%f ",w[k]);printf("\n");

    return ll;
    }

//START170808
#if 0
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
#endif
//START170808
int assignXtrblockgsl(Data *data,int *num_regions_by_file,int num_regions,int num_wfiles,int goose,int j,int p,int *sindex,
    FS *fs,double *X,Scratch *s,int lengthX) {
    int i,k,m,q,uns_count,small,index,i2;
    Memory_Map *mm;
    gsl_vector_view col;
    gsl_matrix_view Xgsl = gsl_matrix_view_array(X,fs->nlines,lengthX);
    if(goose==1) {
        col = gsl_matrix_column(&Xgsl.matrix,0);
        gsl_vector_set_all(&col.vector,1.0);
        }
    for(q=0,small=uns_count=m=0;m<data->nsubjects&&!uns_count;m++) {
        if(num_regions_by_file[m]) {
            if(!(mm=map_disk(data->subjects[m],num_regions*num_regions_by_file[m]*num_wfiles,0,sizeof(float))))return 0;
            for(k=goose,i=0;i<num_regions_by_file[m]&&!uns_count;i++) {
                index = sindex[q]+k;
                i2 = num_regions*(p+num_wfiles*i)+j;
                gsl_matrix_set(&Xgsl.matrix,q,index,(double)mm->ptr[i2]);
                if(mm->ptr[i2]==(float)UNSAMPLED_VOXEL)uns_count++;
                if(fabsf(mm->ptr[i2])<(float)UNSAMPLED_VOXEL)small++;
                if(++k==(fs->frames_per_line[q]+goose)) {
                    k = goose;
                    q++;
                    }
                }
            if(!unmap_disk(mm))return 0;
            }
        }
    s->uns_count = uns_count;
    s->small = small;
    return 1;
    }
