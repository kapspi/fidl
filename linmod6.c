/* Copyright 9/06/16 Washington University.  All Rights Reserved.
   linmod6.c  $Revision: 1.6 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_cdf.h>
#include "d2double.h"
#include "fidl.h"
#include "gauss_smoth2.h"
#include "subs_mask.h"
#include "subs_cifti.h"
#include "dim_param2.h"
#include "cond_norm1.h"
#include "subs_nifti.h"
#include "shouldiswap.h"
#include "nifti_gethdr.h"
#include "write_glm.h"
#include "filetype.h"
#include "subs_util.h"
#include "minmax.h"
#include "get_atlas_param.h"
#include "t4_atlas.h"
#include "gsl_svd.h"

//START170324
#include "nifti_flipslice.h"

static char rcsid[] = "$Header: /home/hannah/mcavoy/idl/clib/RCS/linmod6.c,v 1.6 2017/03/25 00:49:27 mcavoy Exp $";
int linmod6(
           Files_Struct *file_names,
           char *in_glm_file,   /* File containing glm info (*.glm).    */
           char *out_glm_file_in,       /* File glm info written to (*.glm).    */
           Files_Struct *xform_file,    /* File defining atlas transformation (2-2-2). NULL=none. */
           char  *mask_file,    /* Mask defining regions of atlas containing brain. */
           int   mode,          /* bit 0: smooth.
                            bit 1: tranforms to 2-2-2 space,
                            bit 2: Compute smoothness estimate.
                            the following values hold:
                            0: Unsmoothed data in subject space.
                            1: smooth only,
                            2: transform only,
                            3: smooth and transform.
                            4: Estimate smoothness (assume preprocessing)
                            7: Smooth, transform and estimate smoothness. */
           float fwhm,           /* FWHM of smoothing filter. */
           unsigned short *seed, /* If seed, then generate white noise data with the seed. */
           int *t4_identify,int num_regions,Files_Struct *region_files,int *roi,char *atlas,int SunOS_Linux,int bigendian,
           int lcdetrend,char *scratchdir,int lccompressb,int lcvoxels,int lcR2)

           /* Syntax: -F label1 computes an F statistic for label1 where label1 is a label in the
                    event file.  -F label1&label2 computes and F statistic over the effects corresponding 
                    label1 and label2. Label1 and label2 might be levels of a main effect, so this would
                     compute an F for the main effect. */
{
    double *temp_double,*stat,traceR=0,traceRR=0,*A=NULL,*ATy,cond,regress_msq,*atlasimage,*smothimage,*procimage,*df1=NULL, 
        *df2=NULL,*yTy=NULL,**ATAm1,devmsq,b0,**ATAm1AT,*V,*S,cond_norm2,*work,y0,**ATA,**voxATA=NULL,*rowA=NULL,td,*y=NULL,
         condmax=10000.;
    float *temp_float,*t4=NULL,*mask=NULL,lpxy,lpz,*fptr;
    size_t l1,iii,jj1,jj2,iF1,*idx=NULL,*offset=NULL,*tstv; /* i,j,k,l,m,n,iF,ioff,lenbrain */
    long seekstack;
    int	found,*A_or_B_or_U=NULL,nt4,*frames,*Fdef=NULL,*Fcol=NULL,vol=0,lenraw,xdim_in,ydim_in,zdim_in,*brnidx,*brnidx1,
        orientation,ndep,mcol,swapbytesout,imean,eff,lccompressed=0,*status,lccheckunsampled=1,lcscratch=0,
        lcgetmask=1,*work1=NULL,lenbrain,i,j,k,l,m,n,iF,ioff,num_regions1,c_orient[3],cf_flip=-1,lcflip=0; 
    char *separator,string[MAXNAME],string2[MAXNAME],*out_glm_file,*strptr=NULL,concname[MAXNAME],glmroot[MAXNAME];
        /* char *filetype[]={"img","cifti","nifti"};*s1="r+",*s2="a" */
    FILE *fp=NULL,*op=NULL;
    LinearModel	*glm;
    Regions **reg;
    Regions_By_File *rbf=NULL,*fbf;
    Dim_Param2 *dp;
    Meancol *meancol;
    Atlas_Param *ap=NULL;
    Interfile_header *ifh=NULL;
    Mask_Struct *ms=NULL;
    gauss_smoth2_struct *gs=NULL;
    gsl_matrix_view growA,gATAm1,gA;
    XmlS *xmls;
    void* niftiPtr=NULL;
    int64_t xdim64,ydim64,zdim64,tdim64,i64;
    if(!lccompressb) print_version_number(rcsid,stdout);
    gsl_set_error_handler_off();
    if(!(dp=dim_param2(file_names->nfiles,file_names->files,SunOS_Linux))) return 0;
    if(dp->filetypeall==-1){printf("fidlError: All files must be a single type. Abort!\n");fflush(stdout);return 0;}
    if(dp->volall==-1){printf("fidlError: All files must be the same size. Abort!\n");fflush(stdout);return 0;}
    out_glm_file = out_glm_file_in;
    if(!(glm=read_glm(in_glm_file,1,SunOS_Linux))) {
        printf("fidlError reading %s  Abort!\n",in_glm_file);
        return 0;
        }
    if(!glm->ifh->glm_all_eff) {
        printf("fidlError: glm->ifh->glm_all_eff=%d  GLM does not contain any effects. Abort!\n",glm->ifh->glm_all_eff);
        return 0;
        }
    if(dp->tdim_total<glm->ifh->glm_tdim) {
        printf("fidlError: dp->tdim_total=%d glm->ifh->glm_tdim=%d\n",dp->tdim_total,glm->ifh->glm_tdim);
        printf("fidlError: Looks like you are missing some bolds. Abort! Abort! Abort!\n");
        return 0;
        }
    if(glm->ifh->glm_boldtype)free(glm->ifh->glm_boldtype);
    if(!(glm->ifh->glm_boldtype=malloc(sizeof*glm->ifh->glm_boldtype*(strlen(Filetype[dp->filetypeall])+1)))){
        printf("fidlError: Unable to malloc glm->ifh->glm_boldtype\n");fflush(stdout);return 0;}
    strcpy(glm->ifh->glm_boldtype,Filetype[dp->filetypeall]);
    strcpy(glmroot,out_glm_file);
    if(!get_tail_sans_ext(glmroot)) exit(-1);

    //START160906
    //glm->ifh->glm_compressed = lccompress;

    glm->ifh->number_format=(int)FLOAT_IF;
    if(!seed&&!lccompressb) if(glm->ifh->glm_event_file) printf("event file = %s\n",glm->ifh->glm_event_file);
    if(mode&1) glm->ifh->glm_fwhm = fwhm;

    //glm->ifh->bigendian = !lccompress ? bigendian : (SunOS_Linux?0:1);
    //swapbytesout = !lccompress ? shouldiswap(SunOS_Linux,bigendian) : 0;
    //START160906
    glm->ifh->bigendian=bigendian;
    swapbytesout=shouldiswap(SunOS_Linux,bigendian);

    //START160906
    //if(glm->sd) free(glm->sd);

    if(glm->var)free(glm->var);
    if(glm->fstat)free(glm->fstat);
    if(glm->fzstat)free(glm->fzstat);
    lpxy = glm->ifh->voxel_size_1;
    lpz = glm->ifh->voxel_size_3;
    orientation = glm->ifh->orientation;
    xdim_in = glm->ifh->dim1;
    ydim_in = glm->ifh->dim2;
    zdim_in = glm->ifh->dim3;
    lenraw = xdim_in*ydim_in*zdim_in;
    nt4 = (mode & 2) ? xform_file->nfiles : 1;
    if(!(mode&2)){
        if(((vol=glm->ifh->glm_xdim*glm->ifh->glm_ydim*glm->ifh->glm_zdim)>lenraw&&mask_file)||
            (glm->ifh->dim2==1&&glm->ifh->dim3==1&&glm->ifh->dim4==1)){
            lccompressed=1;
            }
        else {
            glm->ifh->glm_xdim = xdim_in;
            glm->ifh->glm_ydim = ydim_in;
            glm->ifh->glm_zdim = zdim_in;
            glm->ifh->glm_dxdy = lpxy;
            glm->ifh->glm_dz = lpz;
            vol = glm->ifh->glm_xdim*glm->ifh->glm_ydim*glm->ifh->glm_zdim;
            }
        get_atlas(vol,atlas);
        }
    if(dp->filetypeall==(int)NIFTI){

        #if 0
        if(!(nifti_gethdr(file_names->files[0],&xdim64,&ydim64,&zdim64,&tdim64,glm->ifh->center,glm->ifh->mmppix,(int*)NULL)))
            return 0;
        printf("glm->ifh->center=%f %f %f\nglm->ifh->mmppix=%f %f %f\n",glm->ifh->center[0],glm->ifh->center[1],glm->ifh->center[2],
            glm->ifh->mmppix[0],glm->ifh->mmppix[1],glm->ifh->mmppix[2]);
        #endif
        //START170324
        if(!(nifti_gethdr(file_names->files[0],&xdim64,&ydim64,&zdim64,&tdim64,glm->ifh->center,glm->ifh->mmppix,c_orient)))
            return 0;
        printf("glm->ifh->center=%f %f %f\nglm->ifh->mmppix=%f %f %f c_orient=%d %d %d\n",glm->ifh->center[0],glm->ifh->center[1],
            glm->ifh->center[2],glm->ifh->mmppix[0],glm->ifh->mmppix[1],glm->ifh->mmppix[2],c_orient[0],c_orient[1],c_orient[2]);


        }
    else{

        //if(!(ap=get_atlas_param(atlas,(Interfile_header*)NULL))) return 0;
        //START161214
        if(!(ap=get_atlas_param(atlas,!strstr(atlas,"MNI")?(Interfile_header*)NULL:glm->ifh))) return 0;

        glm->ifh->mmppix[0] = ap->mmppix[0];
        glm->ifh->mmppix[1] = ap->mmppix[1];
        glm->ifh->mmppix[2] = ap->mmppix[2];
        glm->ifh->center[0] = ap->center[0];
        glm->ifh->center[1] = ap->center[1];
        glm->ifh->center[2] = ap->center[2];
        }
    if(mode&2)  {
        if(!(t4=malloc(sizeof*t4*(size_t)T4SIZE*nt4))) {
            printf("fidlError: Unable to malloc t4\n");
            return 0;
            }
        if(!(A_or_B_or_U=malloc(sizeof*A_or_B_or_U*nt4))) {
            printf("fidlError: Unable to malloc A_or_B_or_U\n");
            return 0;
            }
        for(i=0;i<nt4;i++) {
            if(!read_xform(xform_file->files[i],&t4[i*(int)T4SIZE])) return 0;
            if((A_or_B_or_U[i]=twoA_or_twoB(xform_file->files[i])) == 2) return 0;
            }
        #if 0
        printf("Checking t4s\n");
        for(l=i=0;i<nt4;i++) {
            printf("    %s\n",xform_file->files[i]);
            for(j=0;j<4;j++) {
                printf("        ");
                for(k=0;k<4;k++,l++) printf("%f ",t4[l]);
                printf("\n");
                }
            }
        #endif
        if(!lccompressb) {
            printf("Checking t4s\n");
            for(l=i=0;i<nt4;i++) {
                printf("    %s\n",xform_file->files[i]);
                for(j=0;j<4;j++) {
                    printf("        ");
                    for(k=0;k<4;k++,l++) printf("%f ",t4[l]);
                    printf("\n");
                    }
                }
            }
        glm->ifh->glm_xdim = ap->xdim;
        glm->ifh->glm_ydim = ap->ydim;
        glm->ifh->glm_zdim = ap->zdim;
        glm->ifh->glm_dxdy = ap->voxel_size[0];
        glm->ifh->glm_dz = ap->voxel_size[2];
        vol = glm->ifh->glm_xdim*glm->ifh->glm_ydim*glm->ifh->glm_zdim;
        }


    if(!lccompressb) {

        #if 0
        printf("lccompressed=%d lccompress=%d lccompressb=%d lcdetrend=%d lcvoxels=%d\n",lccompressed,lccompress,
            lccompressb,lcdetrend,lcvoxels); 
        #endif
        //START160906
        printf("lccompressed=%d lccompressb=%d lcdetrend=%d lcvoxels=%d\n",lccompressed,lccompressb,lcdetrend,lcvoxels); 

        fflush(stdout);
        }

    //if(dp->filetypeall==(int)CIFTI){    
    //START160923
    if(dp->filetypeall==(int)CIFTI||dp->filetypeall==(int)CIFTI_DTSERIES){    

        lccheckunsampled=0;
        if(!(xmls=cifti_getxml(file_names->files[0]))) return 0;
        glm->ifh->glm_cifti_xmlsize = xmls->size;
        glm->cifti_xmldata = xmls->data;
        }
    else if(dp->filetypeall==(int)NIFTI){
        lccheckunsampled=0;
        }
    i = vol>lenraw ? vol : lenraw;
    if(i<glm->ifh->glm_Mcol) i=glm->ifh->glm_Mcol;
    if(i<num_regions) i=num_regions;
    j=i;

    //if((dp->filetypeall==(int)CIFTI)&&(i<dp->lenvol_max)) i=dp->lenvol_max;
    //START160923
    if((dp->filetypeall==(int)CIFTI||dp->filetypeall==(int)CIFTI_DTSERIES)&&(i<dp->lenvol_max)) i=dp->lenvol_max;

    if(!(temp_float=malloc(sizeof*temp_float*i))) {
        printf("fidlError: Unable to malloc temp_float\n");
        return 0;
        }
    if(j<(glm->ifh->glm_Mcol*glm->ifh->glm_Mcol)) j=glm->ifh->glm_Mcol*glm->ifh->glm_Mcol;
    if(!(temp_double=malloc(sizeof*temp_double*j))) {
        printf("fidlError: Unable to malloc temp_double\n");
        return 0;
        }
    if(mode&2) {
        if(!(atlasimage=malloc(sizeof*atlasimage*vol))) {
            printf("fidlError: Unable to malloc atlasimage\n");
            return 0;
            }
        for(i=0;i<vol;i++)atlasimage[i]=0.;
        }
    else {
        atlasimage = temp_double;
        }
    if(glm->ifh->glm_nF) {
        if(!(Fdef=malloc(sizeof*Fdef*glm->ifh->glm_nF*glm->ifh->glm_Mcol))) {
            printf("fidlError: Unable to malloc Fdef\n");
            return 0;
            }
        if(!(Fcol=malloc(sizeof*Fcol*glm->ifh->glm_nF))) {
            printf("fidlError: Unable to malloc Fcol\n");
            return 0;
            }
        for(i=0;i<glm->ifh->glm_nF*glm->ifh->glm_Mcol;i++) Fdef[i] = 0;
        for(iF=0;iF<glm->ifh->glm_nF;iF++) { /* Parse string defining F tests. */
            found = 0;
            l = iF * glm->ifh->glm_Mcol;
            strcpy(string,glm->ifh->glm_F_names[iF]);
            while((separator = strrchr(string,'&'))) {
                if(separator+1) {   /* Found an "&" in the string. */
                    strptr = separator + 1;
                    for(i=k=0;i<glm->ifh->glm_all_eff;k+=glm->ifh->glm_effect_length[i++]) { 
                        if(!strcmp(strptr,glm->ifh->glm_effect_label[i])) {
                            found = 1;
                            for(j=k;j<k+glm->ifh->glm_effect_length[i];j++) Fdef[l+j] = 1;
                            }
                        }
                    *separator = 0;
                    if((separator=strrchr(string,'\\'))) *separator = 0;
                    }
                }
            for(i=k=0;i<glm->ifh->glm_all_eff;k+=glm->ifh->glm_effect_length[i++]) { /*Convert the last or only label.*/
                if(!strcmp(string,glm->ifh->glm_effect_label[i])) {
                    found = 1;
                    for(j=k;j<k+glm->ifh->glm_effect_length[i];j++) Fdef[l+j] = 1;
                    }
                }
            if(!strcmp(string,"Omnibus")) { /* Omnibus F-test. */
                found = 1;
                for(i=0;i<glm->ifh->glm_M_interest;i++) Fdef[l+i] = 1;
                }
            for(i=0,Fcol[iF]=0.;i<glm->ifh->glm_Mcol;i++) if(Fdef[l+i]) ++Fcol[iF];
            if(!found) {
                printf("Could not match label specifying F test: %s\n",strptr);
                fflush(stdout);
                return 0;
                }
            if(!glm->ifh->nregions) {
                sprintf(string,"%s_%s_fzstat.4dfp.img",glmroot,glm->ifh->glm_F_names2[iF]);
                if((k=strlen(string))>MAXNAME_CSH) {
                    printf("F statistic %d: Name too long, %d characters\n    %s\n",iF+1,k,string);
                    for(j=0,i=0;i<glm->ifh->glm_M_interest;i++) if(Fdef[l+i]==1) j++;
                    if(j==glm->ifh->glm_M_interest&&(l=(int)strlen(glm->ifh->glm_F_names2[iF]))>=(int)strlen("Omnibus"))
                        strcpy(glm->ifh->glm_F_names2[iF],"Omnibus");
                    else sprintf(glm->ifh->glm_F_names2[iF],"%d",iF+1);
                    sprintf(string,"%s_%s_fzstat.4dfp.img",glmroot,glm->ifh->glm_F_names2[iF]);
                    printf("    Name changed to %s\n",string);
                    }
                }
            #if 0
            printf("%s Fcol[%d]=%d\n    ",glm->ifh->glm_F_names[iF],iF,Fcol[iF]);
            for(i=0;i<glm->ifh->glm_Mcol;i++) printf("%d ",Fdef[l+i]); printf("\n");
            #endif
            }
        }
    if(!(frames=malloc(sizeof*frames*glm->ifh->glm_Nrow))) {
        printf("fidlError: Unable to malloc frames\n");
        return 0;
        }
    for(j=i=0;i<glm->ifh->glm_tdim;i++) if(glm->valid_frms[i] > 0.) frames[j++]=i; 
    if(j>glm->ifh->glm_Nrow) {
        printf("fidlError: j=%lu should equal glm->ifh->glm_Nrow=%d <= glm->ifh->glm_tdim=%d\n",(unsigned long)j,glm->ifh->glm_Nrow,
            glm->ifh->glm_tdim);
        printf("fidlError: glm->ifh->glm_Nrow was probably misspecified in the point and click\n");
        return 0;
        }
    if(!(fbf=find_regions_by_file(file_names->nfiles,glm->ifh->glm_Nrow,dp->tdim,frames))) return 0;
    if(!(meancol=get_meancol(glm->ifh,lccompressb?0:1))) {
        if(!lccompressb) printf("fidlInfo: meancol is NULL. Grand mean will be 0.\n");
        }

    /*printf("meancol->ncol=%d ",meancol->ncol);for(i=0;i<meancol->ncol;i++)printf("%d ",meancol->col[i]);printf("\n");
    printf("meancol->ncoltrend=%d ",meancol->ncoltrend);for(i=0;i<meancol->ncoltrend;i++)printf("%d ",meancol->coltrend[i]);
    printf("\n");fflush(stdout);*/
    lenbrain = (size_t)vol;
    if(!region_files) {
        if(!lccompressed) {
            if(seed) lccheckunsampled=0; 
            }
        else {
            lccheckunsampled=0; 
            }
        num_regions = vol;
        }
    else {
        if(!check_dimensions(region_files->nfiles,region_files->files,vol)) return 0;
        if(!(reg=malloc(sizeof*reg*region_files->nfiles))) {
            printf("fidlError: Unable to malloc reg\n");
            return 0;
            } 
        for(j=0;j<(int)region_files->nfiles;j++) {
            if(!(reg[j]=extract_regions(region_files->files[j],0,0,(float*)NULL,0,SunOS_Linux,(char**)NULL))) exit(-1);
            }
        if(!num_regions) for(i=0;i<(int)region_files->nfiles;i++) num_regions += reg[i]->nregions;
        if(!roi) {
            printf("Warning: -regions_of_interest unspecified.  Will use all %d regions.\n",num_regions);
            if(!(roi=malloc(sizeof*roi*num_regions))) {
                printf("fidlError: Unable to malloc roi\n");
                return 0;
                }
            for(i=0;i<num_regions;i++) roi[i]=i;
            }
        if(!(rbf=find_regions_by_file_cover(region_files->nfiles,num_regions,reg,roi))) return 0;
        glm->ifh->glm_ydim = 1;
        glm->ifh->glm_zdim = 1;
        if(!lcvoxels) {
            lenbrain = glm->ifh->glm_xdim = glm->ifh->nregions = num_regions;
            if(!(tstv=malloc(sizeof*tstv*num_regions))) {
                printf("fidlError: Unable to malloc tstv\n");
                return 0;
                }
            for(i=0;i<num_regions;i++) {
                sprintf(string,"%d %s %d",i,rbf->region_names_ptr[i],rbf->nvoxels_region[i]);
                tstv[i] = strlen(string) + 1;
                }
            if(!(glm->ifh->region_names=d2charvar(num_regions,tstv))) return 0;
            for(i=0;i<num_regions;i++) 
                sprintf(glm->ifh->region_names[i],"%d %s %d",i,rbf->region_names_ptr[i],rbf->nvoxels_region[i]);
            free(tstv);
            if(!(glm->ifh->regfiles=d2charvar(region_files->nfiles,region_files->strlen_files))) return 0;
            for(i=0;i<(int)region_files->nfiles;i++) sprintf(glm->ifh->regfiles[i],"%s",region_files->files[i]);
            glm->ifh->nregfiles = region_files->nfiles;
            lccheckunsampled=lcgetmask=0;
            }
        else {
            glm->ifh->glm_xdim=rbf->nvoxels;
            lenbrain=rbf->nvoxels;
            }
        }
    if(mode&1) if(!(gs=gauss_smoth2_init(glm->ifh->glm_xdim,glm->ifh->glm_ydim,glm->ifh->glm_zdim,glm->ifh->glm_fwhm,
        glm->ifh->glm_fwhm))) return 0;
    /*If more than one t4 is used (ie multiple sessions), then the mask over multiple sessions will not be the same as the mask for
      the individual sessions. Thus, the stacks from the individual sessions will not diff with the stacks over the multiple
      sessions. This can also affect the value of the regional regressors (eg global signal), in that the regional regressors from
      the individual sessions will not be the same as the regional regressors of multiple sessions.*/
    if(!(mask=malloc(sizeof*mask*vol))) {
        printf("fidlError: Unable to malloc mask\n");
        return 0;
        }
    for(i=0;i<vol;i++) mask[i]=0.;
    glm->ifh->glm_masked=0;
    if(lcgetmask) {
        if(!(ms=get_mask_struct(mask_file,!region_files?vol:rbf->nindices_uniqsort,
            !region_files?(int*)NULL:rbf->indices_uniqsort,SunOS_Linux,(LinearModel*)NULL,vol))) return 0;
        for(i=0;i<(lenbrain=ms->lenbrain);i++) mask[ms->brnidx[i]]=1.;
        if(!lccompressed&&!strcmp(atlas,"222"))glm->ifh->glm_masked=1;

        //START170324
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

#if 0
            hdr.ifh.center[0]=-hdr.ifh.center[0]
            hdr.ifh.center[1]=-hdr.ifh.center[1]-36

        sform[3][3]=1.0f;
        sform[0][0]=mmppix[0];sform[0][3]=center[0];

        sform[1][1]=mmppix[1];sform[1][3]=center[1];
        //START170324
        //sform[1][1]=-mmppix[1];sform[1][3]=center[1];

        sform[2][2]=mmppix[2];sform[2][3]=center[2];
#endif

//glm->ifh->center,glm->ifh->mmppix
            }

        }
    if(!region_files&&lccompressed) if((lenbrain=ms->lenbrain)!=lenraw) { 
        printf("lenraw=%d lenbrain=%d Must be equal. Abort!\n",lenraw,lenbrain);fflush(stdout);return 0;}
    if(lccheckunsampled) {
        if((mode&2)&&scratchdir) lcscratch=1;
        if(!lcdetrend) printf("Checking bolds for unsampled voxels.\n");
        for(j=0;j<(int)file_names->nfiles;j++) {
            if(fbf->num_regions_by_file[j]) {
                if(!(fp=fopen_sub(file_names->files[j],"r"))) return 0;
                if(lcscratch) {
                    strcpy(string2,file_names->files[j]);
                    if(!(strptr=get_tail_sans_ext(string2))) return 0;
                    sprintf(string,"%s%s_%s.4dfp.img",scratchdir?scratchdir:"",strptr,lcdetrend?"detrend":ap->str);
                    if(!(op=fopen_sub(string,"w"))) return 0;
                    }
                for(i=0;i<fbf->num_regions_by_file[j];i++) {

                    #if 0
                    if(do_fseek(file_names->files[j],fp,(long)(fbf->roi_by_file[j][i]*lenraw*sizeof(float)),(int)SEEK_SET))
                       return 0;
                    #endif
                    /*START150805*/
                    if(fseek(fp,(long)(fbf->roi_by_file[j][i]*lenraw*sizeof(float)),(int)SEEK_SET)){
                        printf("fidlError: fseek %ld\n",(long)(fbf->roi_by_file[j][i]*lenraw*sizeof(float)));return 0;}
                         

                    if(!fread_sub(temp_float,sizeof(float),(size_t)lenraw,fp,dp->swapbytes[j],file_names->files[j])) return 0;
                    if(mode&2) {
                        for(k=0;k<lenraw;k++) temp_double[k] = temp_float[k]!=(float)UNSAMPLED_VOXEL ?
                            (double)temp_float[k] : (double)UNSAMPLED_VOXEL;
                        if(!t4_atlas(temp_double,atlasimage,&t4[t4_identify[j]*(int)T4SIZE],xdim_in,ydim_in,zdim_in,lpxy,
                            lpz,A_or_B_or_U[t4_identify[j]],orientation,ap,(double*)NULL)) return 0;
                        for(k=0;k<ms->lenbrain;k++) {
                            if(fabs(atlasimage[ms->brnidx[k]])<=(double)UNSAMPLED_VOXEL) mask[ms->brnidx[k]]=0.;
                            }
                        if(lcscratch) {
                            for(k=0;k<ms->lenbrain;k++) temp_double[k] = atlasimage[ms->brnidx[k]];
                            if(!(fwrite_sub(temp_double,sizeof(double),(size_t)ms->lenbrain,op,0))) {
                                printf("fidlError: Could not write to %s\n",string);
                                return 0;
                                }
                            }
                        }
                    else
                        for(k=0;k<lenraw;k++) if(temp_float[k]==(float)UNSAMPLED_VOXEL) mask[k]=0.;
                    }
                fclose(fp);
                if(lcscratch) {
                    fclose(op); 
                    if(!lcdetrend){printf("    Output written to %s\n",string);fflush(stdout);}
                    }

                }
            }
        /*printf("Finished checking bolds for unsampled voxels.\n");fflush(stdout);*/
        }
    if(!lcdetrend) {

        //for(j=i=0;i<vol;i++) if(mask[i]==1.)j++;
        //START160906
        for(j=i=0;i<vol;i++)if(mask[i])j++;

        printf("%d voxels will be processed\n",j);fflush(stdout);
        }
    if(!(brnidx=malloc(sizeof*brnidx*lenbrain))) {
        printf("fidlError: Unable to malloc brnidx\n");
        return 0;
        }
    if(lccheckunsampled) {
        for(lenbrain=0,i=0;i<ms->lenbrain;i++) if(mask[ms->brnidx[i]]>0.) brnidx[lenbrain++] = ms->brnidx[i];
        }
    else if(!region_files&&!lccompressed) {
        for(lenbrain=0,i=0;i<vol;i++) if(mask[i]>0.) brnidx[lenbrain++] = i;
        }
    else {
        for(i=0;i<lenbrain;i++) brnidx[i] = i;
        }
    if(lccompressed){brnidx1=ms->brnidx;num_regions1=num_regions;}
    else if(!glm->ifh->glm_masked){brnidx1=brnidx;num_regions1=num_regions;}
    else{
        if(!(brnidx1=malloc(sizeof*brnidx1*lenbrain))) {
            printf("fidlError: Unable to malloc brnidx1\n");
            return 0;
            }
        for(i=0;i<lenbrain;i++)brnidx1[i]=i;
        num_regions1=lenbrain;
        glm->mask=mask;
        glm->nmaski=lenbrain;
        glm->maski=brnidx;
        }
    if(!lcdetrend) printf("lenraw=%d vol=%d lenbrain=%d num_regions=%d lcscratch=%d\n",lenraw,vol,lenbrain,num_regions,lcscratch); 
    fflush(stdout);
    if(!(stat=malloc(sizeof*stat*(lenbrain>
        (glm->ifh->glm_Mcol*glm->ifh->glm_Mcol)?lenbrain:(glm->ifh->glm_Mcol*glm->ifh->glm_Mcol))))) {
        printf("fidlError: Unable to malloc stat\n");
        return 0;
        }

    //printf("lenraw=%d vol=%d lenbrain=%d\n",lenraw,vol,lenbrain);fflush(stdout);

    if(mode&1) {
        if(!(smothimage=malloc(sizeof*smothimage*(lenraw>vol?lenraw:vol)))) {
            printf("fidlError: Unable to malloc smothimage\n");
            return 0;
            }
        }
    else {
        smothimage = atlasimage;
        }
    if(region_files&&!lcvoxels) {
        if(!(procimage=malloc(sizeof*procimage*lenbrain))) {
            printf("fidlError: Unable to malloc procimage\n");
            return 0;
            }
        }
    else {
        procimage = smothimage;
        }
    if(glm->ifh->glm_nF&&!lcdetrend) {
        if(!(df1=malloc(sizeof*df1*lenbrain))) {
            printf("fidlError: Unable to malloc df1\n");
            return 0;
            }
        if(!(df2=malloc(sizeof*df2*lenbrain))) {
            printf("fidlError: Unable to malloc df2\n");
            return 0;
            }
        if(!(work1=malloc(sizeof*work1*lenbrain))) {
            printf("fidlError: Unable to malloc work1\n");
            return 0;
            }
        }
    if(!(ATy=malloc(sizeof*ATy*glm->ifh->glm_Mcol*lenbrain))) {
        printf("fidlError: Unable to malloc ATy\n");
        return 0;
        }
    for(i=0;i<glm->ifh->glm_Mcol*lenbrain;i++) ATy[i]=0.;

    //if(!lccompress&&!lcdetrend) {
    //START160906
    if(!lcdetrend) {
        if(!(yTy=malloc(sizeof*yTy*lenbrain))) {
            printf("fidlError: Unable to malloc yTy\n");
            return 0;
            }
        for(i=0;i<lenbrain;i++) yTy[i] = 0.;
        }
    if(!(ATA=d2double(glm->ifh->glm_Mcol,glm->ifh->glm_Mcol))) return 0;
    gsl_matrix_view gATA = gsl_matrix_view_array(ATA[0],glm->ifh->glm_Mcol,glm->ifh->glm_Mcol);
    if(!(ATAm1=d2double(glm->ifh->glm_Mcol,glm->ifh->glm_Mcol))) return 0;
    if(!(V=malloc(sizeof*V*glm->ifh->glm_Mcol*glm->ifh->glm_Mcol))) {
        printf("fidlError: Unable to malloc V\n");
        return 0;
        }
    if(!(S=malloc(sizeof*S*glm->ifh->glm_Mcol))) {
        printf("fidlError: Unable to malloc S\n");
        return 0;
        }
    if(!(work=malloc(sizeof*work*glm->ifh->glm_Mcol))) {
        printf("fidlError: Unable to malloc work\n");
        return 0;
        }
    gsl_permutation *perm = gsl_permutation_alloc(glm->ifh->glm_Mcol);
    if(!lccompressed) {
        if(!(glm->ATAm1=d2double(glm->ifh->glm_Mcol,glm->ifh->glm_Mcol))) return 0;
        glm->ATAm1vox=(float**)NULL;
        if(!(A=malloc(sizeof*A*glm->ifh->glm_Nrow*glm->ifh->glm_Mcol))) {
            printf("fidlError: Unable to malloc A\n");
            return 0;
            }
        for(k=i=0;i<glm->ifh->glm_Nrow;i++) for(j=0;j<glm->ifh->glm_Mcol;j++,k++) A[k] = (double)glm->AT[j][i];
        gA = gsl_matrix_view_array(A,glm->ifh->glm_Nrow,glm->ifh->glm_Mcol);
        gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,&gA.matrix,&gA.matrix,0.0,&gATA.matrix);
        for(k=i=0;i<glm->ifh->glm_Mcol;i++) for(j=0;j<glm->ifh->glm_Mcol;j++,k++) temp_double[k] = ATA[i][j];
        gATAm1 = gsl_matrix_view_array(glm->ATAm1[0],glm->ifh->glm_Mcol,glm->ifh->glm_Mcol);
        if(cond_norm1(temp_double,glm->ifh->glm_Mcol,&cond,glm->ATAm1[0],perm))cond=0.;
        if(!seed&&!lcdetrend) printf("Condition number norm1: %f\n",cond);
        if(cond>condmax||cond==0.) {
            if(!seed) {
                if(cond>condmax) printf("Condition number greater than %f.\n",condmax);
                else printf("Design matrix not invertible.\n");
                printf("Computing singular value decomposition.\n");
                }
            for(k=i=0;i<glm->ifh->glm_Mcol;i++)for(j=0;j<glm->ifh->glm_Mcol;j++,k++)temp_double[k]=ATA[i][j];
            td=gsl_svd_golubreinsch(temp_double,glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,0.,V,S,glm->ATAm1[0],&cond_norm2,&ndep,work);
            printf("gsl_svd_golubreinsch cond_norm2=%f ndep=%d tol=%g\n",cond_norm2,ndep,td);
            }
        glm->df=(float*)NULL;
        }
    else {
        glm->ATAm1=(double**)NULL;
        for(j=0,i=glm->ifh->glm_Mcol;i>=1;i--) j+=i; 
        if(!(glm->ATAm1vox=d2float(j,vol))) return 0;
        if(!(voxATA=d2double(lenbrain,j))) return 0;
        if(!(rowA=malloc(sizeof*rowA*glm->ifh->glm_Mcol))) {
            printf("fidlError: Unable to malloc rowA\n");
            return 0;
            }
        growA = gsl_matrix_view_array(rowA,glm->ifh->glm_Mcol,1);
        if(!(glm->df=malloc(sizeof*glm->df*vol))) {
            printf("fidlError: Unable to malloc glm->df\n");
            return 0;
            }
        for(i=0;i<vol;i++) glm->df[i]=0.;
        }
    if(lcdetrend) {
        /*printf("fbf->num_regions_by_file_max=%d\n",fbf->num_regions_by_file_max);fflush(stdout);*/
        if(!(y=malloc(sizeof*y*lenbrain*fbf->num_regions_by_file_max))) {
            printf("fidlError: Unable to malloc y\n");
            return 0;
            }
        strcpy(string2,in_glm_file);
        if(!(strptr=get_tail_sans_ext(string2))) return 0;
        sprintf(concname,"%s%s.conc",scratchdir?scratchdir:"",strptr);
        if(!(op=fopen_sub(concname,"w"))) return 0;
        }
    seekstack=lcscratch?(ms->lenbrain*sizeof(double)):(lenraw*sizeof(float));
    for(jj1=l1=l=j=0;j<(int)file_names->nfiles;j++) {
        if(fbf->num_regions_by_file[j]) {
            if(lcscratch) {
                strcpy(string2,file_names->files[j]);
                if(!(strptr=get_tail_sans_ext(string2))) return 0;
                sprintf(string,"%s%s_%s.4dfp.img",scratchdir?scratchdir:"",strptr,lcdetrend?"detrend":ap->str);
                strptr = string;
                }
            else 
               strptr = file_names->files[j];
            if(dp->filetypeall==(int)IMG){if(!(fp=fopen_sub(strptr,"r")))return 0;}
            else if(dp->filetypeall==(int)CIFTI||dp->filetypeall==(int)CIFTI_DTSERIES){if(!cifti_getstack(strptr,temp_float))return 0;}
            else if(!(niftiPtr=nifti_openRead(strptr)))return 0;
            if(!lcdetrend) printf("Processing %s\n",strptr);
            for(m=i=0;i<fbf->num_regions_by_file[j];i++,l++) {
                if(!seed) {
                    if(dp->filetypeall==(int)IMG){
                        if(fseek(fp,(long)((lcscratch?i:fbf->roi_by_file[j][i])*seekstack),(int)SEEK_SET)){
                            printf("fidlError: fseek %ld\n",(long)((lcscratch?i:fbf->roi_by_file[j][i])*seekstack));return 0;}
                        if(!fread_sub(lcscratch?(void*)temp_double:(void*)temp_float,lcscratch?sizeof(double):sizeof(float),
                            (size_t)(lcscratch?ms->lenbrain:lenraw),fp,lcscratch?0:dp->swapbytes[j],strptr)) return 0;
                        if(!lcscratch) for(k=0;k<lenraw;k++) 
                            temp_double[k] = temp_float[k]!=(float)UNSAMPLED_VOXEL ? (double)temp_float[k] : (double)UNSAMPLED_VOXEL;
                        else for(k=0;k<ms->lenbrain;k++) atlasimage[ms->brnidx[k]] = temp_double[k];
                        }
                    else if(dp->filetypeall==(int)CIFTI||dp->filetypeall==(int)CIFTI_DTSERIES){
                        for(ioff=fbf->roi_by_file[j][i],k=0;k<dp->vol[j];k++,ioff+=dp->tdim[j]) {
                            temp_double[k] = temp_float[ioff]!=(float)UNSAMPLED_VOXEL?(double)temp_float[ioff]:(double)UNSAMPLED_VOXEL;
                            }
                        }
                    else{
//STARTHERE
                        if(!nifti_getvol(niftiPtr,(int64_t)fbf->roi_by_file[j][i],temp_float))return 0;
       
                        //START170324
                        if(lcflip){
                            //printf("flipping slices\n");
                            for(fptr=temp_float,i64=0;i64<zdim64;i64++,fptr+=xdim64*ydim64){
                                if(!nifti_flipslice(xdim64,ydim64,cf_flip,fptr))return 0;
                                }
                            //printf("finished flipping slices\n");fflush(stdout);
                            }
                        


                        if(!lcscratch) for(k=0;k<lenraw;k++) 
                            temp_double[k] = temp_float[k]!=(float)UNSAMPLED_VOXEL ? (double)temp_float[k] : (double)UNSAMPLED_VOXEL;
                        else for(k=0;k<ms->lenbrain;k++) atlasimage[ms->brnidx[k]] = temp_double[k];
                        }
                    }
                else {
                    nrmlrv_doublestack(temp_double,lenraw,seed);
                    }
                if(!lcscratch&&(mode&2)) {
                    if(!t4_atlas(temp_double,atlasimage,&t4[t4_identify[j]*(int)T4SIZE],xdim_in,ydim_in,zdim_in,lpxy,lpz,
                        A_or_B_or_U[t4_identify[j]],orientation,ap,(double*)NULL)) return 0;
                    }
                if(mode&1) gauss_smoth2(atlasimage,smothimage,gs);
                if(region_files&&!lcvoxels) crs(smothimage,procimage,rbf,(char*)NULL);
                if(!lccompressed) {

                    //if(!lccompress&&!lcdetrend) {
                    //START160908
                    if(!lcdetrend) {

                        for(k=0;k<lenbrain;k++) yTy[k] += procimage[brnidx[k]]*procimage[brnidx[k]];
                        } 
                    for(ioff=k=0;k<lenbrain;k++) { 
                        for(eff=0;eff<glm->ifh->glm_Mcol;eff++,ioff++) ATy[ioff] += (double)glm->AT[eff][l]*procimage[brnidx[k]];
                        }
                    }
                else {
                    for(eff=0;eff<glm->ifh->glm_Mcol;eff++) rowA[eff] = (double)glm->AT[eff][l];
                    gsl_blas_dsyrk(CblasUpper,CblasNoTrans,1.0,&growA.matrix,0.,&gATA.matrix);
                    for(ioff=k=0;k<lenbrain;k++) {
                        if(procimage[brnidx[k]]!=(double)UNSAMPLED_VOXEL) {
                            yTy[k] += procimage[brnidx[k]]*procimage[brnidx[k]];

                            //for(eff=0;eff<glm->ifh->glm_Mcol;eff++,ioff++) ATy[ioff] += rowA[eff]*procimage[brnidx[k]];
                            //START161214
                            for(eff=0;eff<glm->ifh->glm_Mcol;eff++,ioff++){
                                ATy[ioff]+=rowA[eff]*procimage[brnidx[k]];
                                //printf("rowA[%d]=%f procimage[%d]=%f ATy[%d]=%f\n",eff,rowA[eff],brnidx[k],procimage[brnidx[k]],
                                //    ioff,ATy[ioff]);fflush(stdout);
                                }

                            /*for(eff=0;eff<glm->ifh->glm_Mcol;eff++,ioff++) {
                                ATy[ioff] += rowA[eff]*procimage[brnidx[k]];
                                if(k==10000)printf("j=%d k=%d rowA[%d]=%f procimage[%d]=%f ATy[%d]=%f\n",j,k,eff,rowA[eff],
                                    brnidx[k],procimage[brnidx[k]],ioff,ATy[ioff]);
                                }*/

                            for(iii=m=0;m<glm->ifh->glm_Mcol;m++) for(n=m;n<glm->ifh->glm_Mcol;n++,iii++) voxATA[k][iii]+=ATA[m][n];
                            glm->df[ms->brnidx[k]]++;
                            }
                        else {
                            ioff+=glm->ifh->glm_Mcol;
                            }
                        }
                    }
                if(lcdetrend) for(k=0;k<lenbrain;k++,m++) y[m] = procimage[brnidx[k]]; 
                }
            if(dp->filetypeall==(int)IMG)fclose(fp);
            if(lcdetrend) {
                if(lccompressb) {
                    for(i=0;i<ms->lenbrain;i++) temp_double[i]=(double)UNSAMPLED_VOXEL;
                    }
                else {
                    for(i=0;i<num_regions;i++) temp_float[i] = 0.;
                    for(i=0;i<ms->lenbrain;i++) temp_float[ms->brnidx[i]]=(float)UNSAMPLED_VOXEL;
                    }
                strcpy(string2,file_names->files[j]);
                if(!(strptr=get_tail_sans_ext(string2))) return 0;
                sprintf(string,"%s%s_detrend.4dfp.img",scratchdir?scratchdir:"",strptr);
                if(!(fp=fopen_sub(string,"w"))) return 0;
                if(!(ifh=read_ifh(file_names->files[j],(Interfile_header*)NULL))) return 0;
                for(m=i=0;i<fbf->num_regions_by_file[j];i++,l1++) {
                    for(ioff=k=0;k<lenbrain;k++,ioff+=glm->ifh->glm_Mcol,m++) {
                        for(y0=0.,eff=0;eff<meancol->ncoltrend;eff++) {
                            for(b0=0.,n=0;n<glm->ifh->glm_Mcol;n++) b0 += glm->ATAm1[meancol->coltrend[eff]][n]*ATy[n+ioff];
                            y0 += (double)glm->AT[meancol->coltrend[eff]][l1]*b0;
                            }
                        y[m] -= y0;
                        }
                    }
                if(lccompressb) {
                    for(m=i=0;i<fbf->num_regions_by_file[j];i++) {

                        for(k=0;k<lenbrain;k++,m++) temp_double[ms->maskidx[brnidx[k]]] = y[m];
                        #if 0
                        for(k=0;k<lenbrain;k++,m++) {
                            printf("%f ",y[m]);
                            temp_double[ms->maskidx[brnidx[k]]] = y[m]; 
                            }
                        printf("\n");
                        #endif
                        /*printf("ms->lenbrain=%d lenbrain=%d\n",ms->lenbrain,lenbrain);*/

                        if(!(fwrite_sub(temp_double,sizeof(double),(size_t)ms->lenbrain,fp,0))) {
                            printf("Could not write to %s\n",string);
                            return 0;
                            }
                        }
                    ifh->dim1 = ms->lenbrain;
                    ifh->dim2 = 1;
                    ifh->dim3 = 1;
                    ifh->dim4 = fbf->num_regions_by_file[j];
                    ifh->mmppix[0] = 0;
                    ifh->mmppix[1] = 0;
                    ifh->mmppix[2] = 0;
                    ifh->center[0] = 0;
                    ifh->center[1] = 0;
                    ifh->center[2] = 0;
                    ifh->number_format = (int)DOUBLE_IF;
                    ifh->bytes_per_pix = 8;
                    }
                else {
                    for(m=i=0;i<ifh->dim4;i++,jj1++) {
                        if(glm->valid_frms[jj1]>0.) {
                            for(k=0;k<lenbrain;k++,m++) temp_float[brnidx[k]] = (float)y[m];
                            }
                        else {
                            for(k=0;k<lenbrain;k++) temp_float[brnidx[k]] = 0.;
                            }
                        if(!(fwrite_sub(temp_float,sizeof(float),(size_t)num_regions,fp,0))) {
                            printf("Could not write to %s\n",string);
                            return 0;
                            }
                        }
                    ifh->dim1 = glm->ifh->glm_xdim;
                    ifh->dim2 = glm->ifh->glm_ydim;
                    ifh->dim3 = glm->ifh->glm_zdim;
                    ifh->voxel_size_1 = ifh->voxel_size_2 = glm->ifh->glm_dxdy;
                    ifh->voxel_size_3 = glm->ifh->glm_dz;
                    }
                fclose(fp);
                ifh->bigendian = !SunOS_Linux ? 1 : 0;
                if(!write_ifh(string,ifh,0)) exit(-1);
                free_ifh(ifh,0);
                fprintf(op,"%s\n",string);
                /*printf("    Output written to %s\n",string);*/
                printf("Output written to %s\n",string);
                }
            }
        }
    if(lcdetrend) {
        printf("Conc written to %s\n",concname);fflush(stdout);
        return 1;
        }

    #if 0
    if(!lcdetrend) {
        if(!(glm->sd=malloc(sizeof*glm->sd*num_regions))) {
            printf("fidlError: Unable to malloc glm->sd\n");
            return 0;
            }
        for(i=0;i<num_regions;i++) glm->sd[i]=0.;
        if(!(glm->var=malloc(sizeof*glm->var*num_regions))) {
            printf("fidlError: Unable to malloc glm->var\n");
            return 0;
            }
        for(i=0;i<num_regions;i++) glm->var[i]=0.;
        if(glm->ifh->glm_nF) {
            if(!(glm->fstat=malloc(sizeof*glm->fstat*glm->ifh->glm_nF*num_regions))) {
                printf("fidlError: Unable to malloc glm->fstat\n");
                return 0;
                }
            for(i=0;i<glm->ifh->glm_nF*num_regions;i++) glm->fstat[i]=0.;
            if(!(glm->fzstat=malloc(sizeof*glm->fzstat*glm->ifh->glm_nF*num_regions))) {
                printf("fidlError: Unable to malloc glm->fzstat\n");
                return 0;
                }
            for(i=0;i<glm->ifh->glm_nF*num_regions;i++) glm->fzstat[i]=0.;
            }
        }
    if(glm->ifh->glm_nF) {
        if(!(idx=malloc(sizeof*idx*glm->ifh->glm_Mcol))) {
            printf("fidlError: Unable to malloc idx\n");
            return 0;
            }
        if(glm->ifh->nregions) {
            if(!(status=malloc(sizeof*status*glm->ifh->glm_nF*num_regions))) {
                printf("fidlError: Unable to malloc status\n");
                return 0;
                }
            }
        }
    #endif
    //START160907
    if(!(glm->var=malloc(sizeof*glm->var*num_regions1))){
        printf("fidlError: Unable to malloc glm->var\n");
        return 0;
        }
    for(i=0;i<num_regions1;i++)glm->var[i]=0.;
    if(glm->ifh->glm_nF){
        if(!(glm->fstat=malloc(sizeof*glm->fstat*glm->ifh->glm_nF*num_regions1))){
            printf("fidlError: Unable to malloc glm->fstat\n");
            return 0;
            }
        for(i=0;i<glm->ifh->glm_nF*num_regions1;i++)glm->fstat[i]=0.;
        if(!(glm->fzstat=malloc(sizeof*glm->fzstat*glm->ifh->glm_nF*num_regions1))){
            printf("fidlError: Unable to malloc glm->fzstat\n");
            return 0;
            }
        for(i=0;i<glm->ifh->glm_nF*num_regions1;i++)glm->fzstat[i]=0.;
        if(!(idx=malloc(sizeof*idx*glm->ifh->glm_Mcol))) {
            printf("fidlError: Unable to malloc idx\n");
            return 0;
            }
        if(glm->ifh->nregions) {
            if(!(status=malloc(sizeof*status*glm->ifh->glm_nF*num_regions))) {
                printf("fidlError: Unable to malloc status\n");
                return 0;
                }
            }
        }




    if(!lccompressed) {
        if(!(ATAm1AT=d2double(glm->ifh->glm_Mcol,glm->ifh->glm_Nrow))) return 0;
        gsl_matrix_view gATAm1AT = gsl_matrix_view_array(ATAm1AT[0],glm->ifh->glm_Mcol,glm->ifh->glm_Nrow);
        gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,&gATAm1.matrix,&gA.matrix,0.0,&gATAm1AT.matrix);
        for(traceR=traceRR=0.,i=0;i<glm->ifh->glm_Nrow;i++) {
            for(j=0;j<glm->ifh->glm_Nrow;j++) {
                for(td=0.,k=0;k<glm->ifh->glm_Mcol;k++) td += (double)glm->AT[k][i]*ATAm1AT[k][j];
                if(i==j) {
                    td = 1.-td;
                    traceR += td;
                    }
                traceRR += td*td;
                }
            }
        glm->ifh->glm_df = (float)(traceR*traceR/traceRR);
        }
    else {
        glm->ifh->glm_df = (float)UNSAMPLED_VOXEL;
        for(i=0;i<lenbrain;i++) glm->df[ms->brnidx[i]]-=glm->ifh->glm_Mcol;
        if(glm->ifh->glm_nF) {
            if(!(offset=malloc(sizeof*offset*glm->ifh->glm_Mcol))) {
                printf("fidlError: Unable to malloc offset\n");
                return 0;
                }
            for(offset[0]=0,i=1;i<glm->ifh->glm_Mcol;i++) offset[i]=i+offset[i-1];

            /*printf("offset=");for(i=0;i<glm->ifh->glm_Mcol;i++)printf("%d ",offset[i]);printf("\n");fflush(stdout);*/

            }
        }

    //START160907
    //if(!lcdetrend) {
        if((!lccompressed&&traceR>0.&&lcR2)||(lccompressed&&glm->ifh->glm_nF)) {

            #if 0
            if(!(ifh=init_ifh(4,glm->ifh->glm_xdim,glm->ifh->glm_ydim,glm->ifh->glm_zdim,1,glm->ifh->glm_dxdy,glm->ifh->glm_dxdy,
                glm->ifh->glm_dxdy,glm->ifh->bigendian))) return 0;
            #endif
            //START161214
            //if(!(ifh=init_ifh(4,glm->ifh->glm_xdim,glm->ifh->glm_ydim,glm->ifh->glm_zdim,1,glm->ifh->glm_dxdy,glm->ifh->glm_dxdy,
            //    glm->ifh->glm_dxdy,glm->ifh->bigendian,glm->ifh->center,glm->ifh->mmppix)))return 0;
            //START170720
            if(!(ifh=init_ifh(4,glm->ifh->glm_xdim,glm->ifh->glm_ydim,glm->ifh->glm_zdim,1,glm->ifh->glm_dxdy,glm->ifh->glm_dxdy,
                glm->ifh->glm_dz,glm->ifh->bigendian,glm->ifh->center,glm->ifh->mmppix)))return 0;

            if(!(ifh->file_name=malloc(sizeof*ifh->file_name*(strlen(out_glm_file)+1)))) {
                printf("Error: Unable to malloc ifh->file_name\n");
                return 0;
                }
            strcpy(ifh->file_name,out_glm_file);
            ifh->fwhm = glm->ifh->glm_fwhm;
            } 
        if(!lccompressed) {
            if(!seed) {
                printf("glm->ifh->glm_Nrow=%d glm->ifh->glm_Mcol=%d traceR=%f traceRR=%f\nNumber of degrees of freedom: %f\n",
                    glm->ifh->glm_Nrow,glm->ifh->glm_Mcol,traceR,traceRR,glm->ifh->glm_df);fflush(stdout);
                }
            gsl_permutation_free(perm);
            if(traceR>0.) {
                for(ioff=i=0;i<lenbrain;i++,ioff+=glm->ifh->glm_Mcol) {
                    for(devmsq=yTy[i],m=0;m<glm->ifh->glm_Mcol;m++) {
                        devmsq -= glm->ATAm1[m][m]*ATy[m+ioff]*ATy[m+ioff];
                        for(n=m+1;n<glm->ifh->glm_Mcol;n++) devmsq -= 2*glm->ATAm1[m][n]*ATy[n+ioff]*ATy[m+ioff];
                        }

                    #if 0
                    glm->var[brnidx[i]] = devmsq/traceR;
                    glm->sd[brnidx[i]] = (float)sqrt(glm->var[brnidx[i]]);
                    #endif
                    //START160709
                    glm->var[brnidx1[i]]=devmsq/traceR;


                    }
                if(glm->ifh->glm_nF) {
                    printf("Computing F statistics\n");
                    for(i=0;i<lenbrain;i++) df2[i] = traceR;
                    if(glm->ifh->glm_nF>1)for(i=0;i<num_regions;i++)temp_float[i]=0.;

                    //for(iii=iF1=iF=0;iF<glm->ifh->glm_nF;iF++,iF1+=num_regions,iii+=glm->ifh->glm_Mcol) {
                    //START160908
                    for(iii=iF1=iF=0;iF<glm->ifh->glm_nF;iF++,iF1+=num_regions1,iii+=glm->ifh->glm_Mcol){

                        if(!Fcol[iF])continue;
                        if(!(mcol=glm->ifh->glm_Mcol-Fcol[iF])){

                            //for(i=0;i<lenbrain;i++) glm->fstat[iF1+brnidx[i]]=glm->fzstat[iF1+brnidx[i]]=(float)UNSAMPLED_VOXEL;
                            //START160908
                            for(i=0;i<lenbrain;i++)glm->fstat[iF1+brnidx1[i]]=glm->fzstat[iF1+brnidx1[i]]=(float)UNSAMPLED_VOXEL;

                            }
                        else {
                            for(jj1=iii,jj2=j=0;j<glm->ifh->glm_Mcol;j++,jj1++) {
                                if(!Fdef[jj1]) {
                                    for(l=jj2,k=i=0;i<glm->ifh->glm_Nrow;i++,l+=mcol) A[l] = (double)glm->AT[j][i];
                                    idx[jj2++]=j;
                                    }
                                }
                            gA = gsl_matrix_view_array(A,glm->ifh->glm_Nrow,mcol);
                            gsl_matrix_view gATA = gsl_matrix_view_array(ATA[0],mcol,mcol);
                            gATAm1 = gsl_matrix_view_array(ATAm1[0],mcol,mcol);
                            for(j=1;j<mcol;j++) {ATA[j]=ATA[j-1]+mcol;ATAm1[j]=ATAm1[j-1]+mcol;}
                            gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,&gA.matrix,&gA.matrix,0.0,&gATA.matrix);
                            gsl_permutation *perm = gsl_permutation_alloc(mcol);
                            for(k=i=0;i<mcol;i++) for(j=0;j<mcol;j++,k++) temp_double[k] = ATA[i][j];
                            if(cond_norm1(temp_double,mcol,&cond,ATAm1[0],perm))cond=0.;
                            if(cond>condmax||cond==0.) gsl_svd_golubreinsch(ATA[0],mcol,mcol,0.,V,S,ATAm1[0],&cond_norm2,&ndep,work);
                            gsl_permutation_free(perm);
                            for(ioff=i=0;i<lenbrain;i++,ioff+=glm->ifh->glm_Mcol) {
                                temp_double[i]=0.;

                                #if 0
                                if(glm->var[brnidx[i]]>0.) {
                                    for(stat[i]=yTy[i],m=0;m<mcol;m++) for(n=0;n<mcol;n++)
                                        stat[i] -= ATAm1[m][n]*ATy[idx[n]+ioff]*ATy[idx[m]+ioff];
                                    temp_double[i]=(stat[i]-glm->var[brnidx[i]]*traceR)/((double)Fcol[iF]*glm->var[brnidx[i]]);
                                    }
                                glm->fstat[iF1+brnidx[i]]=(float)temp_double[i];
                                #endif
                                //START160907
                                if(glm->var[brnidx1[i]]>0.) {
                                    for(stat[i]=yTy[i],m=0;m<mcol;m++) for(n=0;n<mcol;n++)
                                        stat[i] -= ATAm1[m][n]*ATy[idx[n]+ioff]*ATy[idx[m]+ioff];
                                    temp_double[i]=(stat[i]-glm->var[brnidx1[i]]*traceR)/((double)Fcol[iF]*glm->var[brnidx1[i]]);
                                    }
                                glm->fstat[iF1+brnidx1[i]]=(float)temp_double[i];


                                df1[i] = (double)Fcol[iF];
                                }
                            if(lcR2&&!glm->ifh->nregions) {
                                for(i=0;i<lenbrain;i++) {

                                    //stat[i]=1.-glm->var[brnidx[i]]*traceR/stat[i];
                                    //START160907
                                    //stat[i]=1.-glm->var[brnidx1[i]]*traceR/stat[i];
                                    //START180403
                                    stat[i]=glm->var[brnidx1[i]]>0.?1.-glm->var[brnidx1[i]]*traceR/stat[i]:(double)UNSAMPLED_VOXEL;

                                    temp_float[brnidx[i]]=(float)stat[i];
                                    }
                                sprintf(string,"%s_%s_R2.4dfp.img",glmroot,glm->ifh->glm_F_names2[iF]);
                                if(!writestack(string,temp_float,sizeof*glm->fzstat,(size_t)num_regions,swapbytesout)) return 0;
                                min_and_max_init(&ifh->global_min,&ifh->global_max);
                                min_and_max_doublestack(stat,lenbrain,&ifh->global_min,&ifh->global_max);
                                if(!write_ifh(string,ifh,0)) return 0;
                                printf("R2 written to %s\n",string);
                                }
                            f_to_z(temp_double,stat,lenbrain,df1,df2,work1);

                            //for(i=0;i<lenbrain;i++) glm->fzstat[iF1+brnidx[i]] = (float)stat[i];
                            //START160908
                            for(i=0;i<lenbrain;i++)temp_float[brnidx[i]]=glm->fzstat[iF1+brnidx1[i]]=(float)stat[i];

                            if(glm->ifh->glm_nF>1&&!glm->ifh->nregions) {
                                sprintf(string,"%s_%s_fzstat.4dfp.img",glmroot,glm->ifh->glm_F_names2[iF]);

                                //if(!writestack(string,&glm->fzstat[iF1],sizeof*glm->fzstat,(size_t)num_regions,swapbytesout))
                                //    return 0;
                                //START160908
                                if(!writestack(string,&temp_float,sizeof*temp_float,(size_t)num_regions,swapbytesout))return 0;

                                min_and_max_init(&ifh->global_min,&ifh->global_max);
                                min_and_max_doublestack(stat,lenbrain,&ifh->global_min,&ifh->global_max);
                                if(!write_ifh(string,ifh,0)) return 0;

                                //printf("F statistic written to %s\n",string);
                                //START160908
                                printf("Gaussianized F statistic written to %s\n",string);

                                }
                            }
                        }
                    if(rbf) {
                        printf("    %*s\tF\t\tZ\n",rbf->max_length,"region");
                        for(iF=0;iF<glm->ifh->glm_nF;iF++) {
                            printf("%s\n",glm->ifh->glm_F_names2[iF]);
                            for(i=0;i<lenbrain;i++)
                                printf("    %*s\t%f\t%.7g\n",rbf->max_length,rbf->region_names_ptr[i],
                                    glm->fstat[iF*num_regions+brnidx[i]],glm->fzstat[iF*num_regions+brnidx[i]]);
                            }
                        }
                    if(glm->ifh->nregions) {
                        sprintf(string,"%s_fzstat.txt",glmroot);
                        if(!(fp=fopen_sub(string,"w"))) return 0;
                        fprintf(fp,"region\tF\tp\tZ\n");
                        for(iF=0;iF<glm->ifh->glm_nF;iF++) {
                            fprintf(fp,"%s\n",glm->ifh->glm_F_names2[iF]);
                            for(i=0;i<lenbrain;i++) {
                                fprintf(fp,"%s\t%f\t",glm->ifh->region_names[i],glm->fstat[iF*num_regions+brnidx[i]]);
                                if(glm->fzstat[iF*num_regions+brnidx[i]]==(float)UNSAMPLED_VOXEL)
                                    fprintf(fp,"not computable\n");
                                else
                                    fprintf(fp,"%g\t%g\n",gsl_cdf_fdist_Q((double)glm->fstat[iF*num_regions+brnidx[i]],
                                        (double)Fcol[iF],df2[i]),glm->fzstat[iF*num_regions+brnidx[i]]);
                                }
                            }
                        fclose(fp);fflush(fp);
                        printf("F statistics written to %s\n",string);fflush(stdout);
                        }
                    }
                }
            }
        else {
            for(ioff=i=0;i<lenbrain;i++,ioff+=glm->ifh->glm_Mcol) {
                if(glm->df[ms->brnidx[i]]>0.) {
                    for(l=j=0;j<glm->ifh->glm_Mcol;j++) for(k=j;k<glm->ifh->glm_Mcol;k++,l++) 
                        temp_double[j*glm->ifh->glm_Mcol+k]=temp_double[k*glm->ifh->glm_Mcol+j]=ATA[j][k]=ATA[k][j]=voxATA[i][l];
                    #if 0
                    printf("voxATA\n");
                    for(l=j=0;j<glm->ifh->glm_Mcol;j++) {
                        for(k=j;k<glm->ifh->glm_Mcol;k++,l++) printf("%f ",voxATA[i][l]);
                        printf("\n");
                        }
                    #endif
                    if(cond_norm1(temp_double,glm->ifh->glm_Mcol,&cond,ATAm1[0],perm))cond=0.;
                    if(cond>condmax||cond==0.)
                        gsl_svd_golubreinsch(ATA[0],glm->ifh->glm_Mcol,glm->ifh->glm_Mcol,0.,V,S,ATAm1[0],&cond_norm2,&ndep,work);
                    for(l=j=0;j<glm->ifh->glm_Mcol;j++) for(k=j;k<glm->ifh->glm_Mcol;k++,l++) 
                        glm->ATAm1vox[l][ms->brnidx[i]]=(float)ATAm1[j][k];
                    for(devmsq=yTy[i],j=0;j<glm->ifh->glm_Mcol;j++) {
                        devmsq -= ATAm1[j][j]*ATy[j+ioff]*ATy[j+ioff];
                        for(k=j+1;k<glm->ifh->glm_Mcol;k++) devmsq -= 2*ATAm1[j][k]*ATy[k+ioff]*ATy[j+ioff];
                        }

                    //glm->var[ms->brnidx[i]] = devmsq/glm->df[ms->brnidx[i]];
                    //START160919
                    glm->var[ms->brnidx[i]]=devmsq/(double)glm->df[ms->brnidx[i]];

                    //START160906
                    //glm->sd[ms->brnidx[i]] = (float)sqrt(glm->var[ms->brnidx[i]]);

                    }
                }
            gsl_permutation_free(perm);
            if(glm->ifh->glm_nF) {
                printf("Computing F statistics\n");
                for(i=0;i<lenbrain;i++) df2[i] = (double)glm->df[ms->brnidx[i]];
                for(iii=iF1=iF=0;iF<glm->ifh->glm_nF;iF++,iF1+=num_regions,iii+=glm->ifh->glm_Mcol) {
                    if(!Fcol[iF]) continue;
                    /*printf("glm->ifh->glm_Mcol=%d Fcol[%lu]=%d\n",glm->ifh->glm_Mcol,(unsigned long)iF,Fcol[iF]);fflush(stdout);*/
                    if(!(mcol=glm->ifh->glm_Mcol-Fcol[iF])) {
                        for(i=0;i<lenbrain;i++) glm->fstat[iF1+ms->brnidx[i]]=glm->fzstat[iF1+ms->brnidx[i]]=(float)UNSAMPLED_VOXEL;
                        }
                    else {
                        for(jj1=iii,jj2=j=0;j<glm->ifh->glm_Mcol;j++,jj1++) if(!Fdef[jj1]) idx[jj2++]=j;
                        gATAm1 = gsl_matrix_view_array(ATAm1[0],mcol,mcol);
                        gsl_permutation *perm = gsl_permutation_alloc(mcol);
                        for(j=1;j<mcol;j++) {ATA[j]=ATA[j-1]+mcol;ATAm1[j]=ATAm1[j-1]+mcol;}
                        for(ioff=i=0;i<lenbrain;i++,ioff+=glm->ifh->glm_Mcol) {
                            temp_double[i]=0.;
                            if(glm->var[ms->brnidx[i]]>0.) {
                                for(j=0;j<mcol;j++) for(k=j;k<mcol;k++) {
                                    stat[j*mcol+k]=stat[k*mcol+j]=ATA[j][k]=ATA[k][j]=
                                        voxATA[i][idx[j]*glm->ifh->glm_Mcol+idx[k]-offset[idx[j]]];
                                    }
                                if(cond_norm1(stat,mcol,&cond,ATAm1[0],perm)){
                                    cond=0.;
                                    printf("i=%lu stat\n",(unsigned long)i);
                                    for(l=j=0;j<mcol;j++) {
                                        for(k=0;k<mcol;k++,l++) printf("%f ",stat[l]);
                                        printf("\n");
                                        }
                                    }
                                if(cond>condmax||cond==0.) 
                                    gsl_svd_golubreinsch(ATA[0],mcol,mcol,0.,V,S,ATAm1[0],&cond_norm2,&ndep,work);
                                for(regress_msq=yTy[i],m=0;m<mcol;m++) for(n=0;n<mcol;n++)
                                    regress_msq -= ATAm1[m][n]*ATy[idx[n]+ioff]*ATy[idx[m]+ioff];
                                temp_double[i]=(regress_msq-glm->var[ms->brnidx[i]]*(double)glm->df[ms->brnidx[i]])/
                                    ((double)Fcol[iF]*glm->var[ms->brnidx[i]]);
                                }
                            glm->fstat[iF1+ms->brnidx[i]]=(float)temp_double[i];
                            df1[i] = (double)Fcol[iF];
                            }
                        if(!glm->ifh->nregions) {
                            f_to_z(temp_double,stat,lenbrain,df1,df2,work1);
                            for(i=0;i<lenbrain;i++) glm->fzstat[iF1+ms->brnidx[i]] = (float)stat[i];
                            gsl_permutation_free(perm);

                            /*sprintf(string,"%s_%s_fzstat.4dfp.img",glmroot,glm->ifh->glm_F_names[iF]);*/
                            /*START140826*/
                            sprintf(string,"%s_%s_fzstat.4dfp.img",glmroot,glm->ifh->glm_F_names2[iF]);

                            if(!writestack(string,&glm->fzstat[iF1],sizeof*glm->fzstat,(size_t)num_regions,swapbytesout)) return 0;
                            if(!write_ifh(string,ifh,0)) return 0;
                            printf("F statistic written to %s\n",string);
                            }
                        }
                    }
                }
            }
    //START160907
    #if 0
        }
    else { 
        glm->ifh->glm_nc=0;
        }
    #endif


    glm->ifh->glm_W = 0.;
    if(mask_file) {
        if(glm->ifh->glm_mask_file) free(glm->ifh->glm_mask_file); 
        if(!(glm->ifh->glm_mask_file=malloc(sizeof*ifh->glm_mask_file*(strlen(mask_file)+1)))) {
            printf("fidlError: Unable to malloc glm->ifh->glm_mask_file\n");
            return 0;
            }
        strcpy(glm->ifh->glm_mask_file,mask_file);
        }

    //STARTHERE


    if(xform_file) {
        if(!(glm->ifh->glm_t4files=d2charvar(xform_file->nfiles,xform_file->strlen_files))) return 0;
        for(i=0;i<(int)xform_file->nfiles;i++) sprintf(glm->ifh->glm_t4files[i],"%s",xform_file->files[i]);
        glm->ifh->glm_nt4files = xform_file->nfiles;
        }
    if(!write_glm(out_glm_file,glm,(int)WRITE_GLM_THRU_FZSTAT,num_regions,swapbytesout)) {
        printf("fidlError: writing WRITE_GLM_THRU_FZSTAT to %s\n",out_glm_file);
        return 0;
        }

    #if 0
    if(!lccompress) {
        if(!(glm->grand_mean=malloc(sizeof*glm->grand_mean*num_regions))) {
            printf("fidlError: Unable to malloc glm->grand_mean\n");
            return 0;
            }
        for(i=0;i<num_regions;i++) glm->grand_mean[i] = 0.;
        }
    if(!(fp=fopen_sub(out_glm_file,!lccompress?s1:s2))) return 0;
    if(!lccompress) if(fseek(fp,(long)(num_regions*sizeof(float)),(int)SEEK_END)){
        printf("fidlError: fseek %ld\n",(long)(num_regions*sizeof(float)));return 0;}
    if(lccompressed) for(i=1;i<glm->ifh->glm_Mcol;i++) ATAm1[i]=ATAm1[i-1]+glm->ifh->glm_Mcol;
    brnidx1=!lccompressed?brnidx:ms->brnidx;
    for(i=0;i<num_regions;i++) temp_float[i] = (float)UNSAMPLED_VOXEL;
    if(!lccompressed){if(ATAm1)free(ATAm1);ATAm1=glm->ATAm1;}
    #endif
    //START160907
    #if 0
    if(lccompressed){brnidx1=ms->brnidx;num_regions1=num_regions;}
    else if(!glm->ifh->glm_masked){brnidx1=brnidx;num_regions1=num_regions;}
    else{
        if(!(brnidx1=malloc(sizeof*brnidx1*lenbrain))) {
            printf("fidlError: Unable to malloc brnidx1\n");
            return 0;
            }
        for(i=0;i<lenbrain;i++)brnidx1[i]=i;
        num_regions1=lenbrain;
        }
    #endif
    if(!(glm->grand_mean=malloc(sizeof*glm->grand_mean*num_regions1))){
        printf("fidlError: Unable to malloc glm->grand_mean\n");
        return 0;
        }
    for(i=0;i<num_regions1;i++) glm->grand_mean[i] = 0.;
    if(!(fp=fopen_sub(out_glm_file,"r+"))) return 0;
    if(fseek(fp,(long)(num_regions1*sizeof(float)),(int)SEEK_END)){
        printf("fidlError: fseek %ld\n",(long)(num_regions1*sizeof(float)));return 0;}
    for(i=0;i<num_regions1;i++)temp_float[i]=(float)UNSAMPLED_VOXEL;
    if(!lccompressed){if(ATAm1)free(ATAm1);ATAm1=glm->ATAm1;}
    else for(i=1;i<glm->ifh->glm_Mcol;i++)ATAm1[i]=ATAm1[i-1]+glm->ifh->glm_Mcol;




    for(imean=eff=0;eff<glm->ifh->glm_Mcol;eff++) {
        for(ioff=i=0;i<lenbrain;i++) {
            if(lccompressed){for(l=j=0;j<glm->ifh->glm_Mcol;j++) for(k=j;k<glm->ifh->glm_Mcol;k++,l++)
                ATAm1[j][k]=ATAm1[k][j]=(double)glm->ATAm1vox[l][ms->brnidx[i]];}

            //for(b0=0.,m=0;m<glm->ifh->glm_Mcol;m++,ioff++) b0 += ATAm1[eff][m]*ATy[ioff];
            //START161214
            for(b0=0.,m=0;m<glm->ifh->glm_Mcol;m++,ioff++){
                b0+=ATAm1[eff][m]*ATy[ioff];
                //printf("ATAm1[%d][%d]=%f ATy[%d]=%f\n",eff,m,ATAm1[eff][m],ioff,ATy[ioff]);
                }

            temp_float[brnidx1[i]] = (float)b0;


            //START161214
            //if(!isfinite(b0)){printf("b0=%f\n",b0);fflush(stdout);}

            }

        #if 0
        if(meancol&&!lccompress) {
            if(eff == meancol->col[imean]) {
                for(i=0;i<lenbrain;i++) glm->grand_mean[brnidx1[i]] += temp_float[brnidx[i]];
                if(imean<(meancol->ncol-1)) ++imean;
                }
            }
        if(!(fwrite_sub(temp_float,sizeof(float),(size_t)num_regions,fp,swapbytesout))) {
            printf("Could not write b to %s in linmod. eff=%d\n",out_glm_file,eff);
            return 0;
            }
        #endif
        //START160907
        if(meancol){
            if(eff==meancol->col[imean]){
                for(i=0;i<lenbrain;i++)glm->grand_mean[brnidx1[i]]+=temp_float[brnidx1[i]];
                if(imean<(meancol->ncol-1))++imean;
                }
            }
        if(!(fwrite_sub(temp_float,sizeof(float),(size_t)num_regions1,fp,swapbytesout))){
            printf("Could not write b to %s in linmod. eff=%d\n",out_glm_file,eff);
            return 0;
            }


        }
    fclose(fp);


    #if 0
    if(!lccompress) {
        if(meancol) for(i=0;i<num_regions;i++) glm->grand_mean[i] /= meancol->ncol;
        if(!write_glm_grand_mean(out_glm_file,glm,num_regions,(long)(-num_regions*(glm->ifh->glm_Mcol+1)*sizeof(float)),swapbytesout))
            return 0;
        }
    #endif
    //START160907
    if(meancol)for(i=0;i<lenbrain;i++)glm->grand_mean[brnidx1[i]]/=meancol->ncol;
    if(!write_glm_grand_mean(out_glm_file,glm,num_regions1,(long)(-num_regions1*(glm->ifh->glm_Mcol+1)*sizeof(float)),swapbytesout))
        return 0;


    printf("Output written to %s\n",out_glm_file);
    return 1;
    }
