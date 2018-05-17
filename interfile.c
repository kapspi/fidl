/* Copyright 12/31/99 Washington University.  All Rights Reserved.
   interfile.c  $Revision: 12.192 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fidl.h"
#include "subs_util.h"
Interfile_header *read_ifh(char *filename,Interfile_header *ifh1) {
    Interfile_header *ifh=NULL;
    char string[MAXNAME],*strptr;
    FILE *fp;
    strptr = strcpy(string,filename);
    strptr += strlen(string)-9;
    if(!strcmp(strptr,".4dfp.ifh")) {
        /* do nothing*/
        }
    else if(!strcmp(strptr,".4dfp.img")) {
        *(strptr+5) = 0;
        strcat(string,".ifh");
        }

    #if 0
    else {
        printf("Warning: %s is not a 4dfp.\n",string);
        return ifh;
        }
    #endif
    //START180316
    else {
        strptr+=5;
        if(!strcmp(strptr,".nii")){
            *strptr=0;
            strcat(string,".ifh");
            }
        else{
            printf("Warning: %s is not a 4dfp.\n",string);
            return ifh;
            }
        }


    if((fp=fopen_sub(string,"r"))) {
        ifh = get_ifh(fp,ifh1);
        fclose(fp);
        }
    return ifh;
    }
Interfile_header *get_ifh(FILE *fp,Interfile_header *ifh1){
Interfile_header *ifh;
char    line[MAXNAME],junk[MAXNAME];
size_t  *tstv=NULL,*glm_F_names_len=NULL,*glm_F_names2_len=NULL;
int	tst,i,iF=0,iF2=0,jlab=0,jlen=0,jcol=0,scrap,len;
if(!(ifh=init_ifh0(ifh1))) return NULL;
while(fgets(line,sizeof(line),fp)) {
    get_line_wnull(line,junk,":=",&len);
    if(!len) continue;
    if(!strcmp(line,"image modality")) {
	if(!(ifh->image_modality=malloc(sizeof*ifh->image_modality*(len+1)))) {
            printf("Error: Unable to malloc ifh->image_modality\n");
            return NULL;
            }
        strcpy(ifh->image_modality,junk);
        }
    else if(!strcmp(line,"originating system")) {
	if(!(ifh->orig_system=malloc(sizeof*ifh->orig_system*(len+1)))) {
            printf("Error: Unable to malloc ifh->orig_system\n");
            return NULL;
            }
        strcpy(ifh->orig_system,junk);
        }
    else if(!strcmp(line,"program version")) {
	if(!(ifh->prog_version=malloc(sizeof*ifh->prog_version*(len+1)))) {
            printf("Error: Unable to malloc ifh->prog_version\n");
            return NULL;
            }
        strcpy(ifh->prog_version,junk);
        }
    else if(!strcmp(line,"program date")) {
	if(!(ifh->prog_date=malloc(sizeof*ifh->prog_date*(len+1)))) {
            printf("Error: Unable to malloc ifh->prog_date\n");
            return NULL;
            }
        strcpy(ifh->prog_date,junk);
        }
    else if(!strcmp(line,"original institution")) {
	if(!(ifh->orig_inst=malloc(sizeof*ifh->orig_inst*(len+1)))) {
            printf("Error: Unable to malloc ifh->orig_inst\n");
            return NULL;
            }
        strcpy(ifh->orig_inst,junk);
        }
    else if(!strcmp(line,"name of data file")) {
	if(!(ifh->file_name=malloc(sizeof*ifh->file_name*(len+1)))) {
            printf("Error: Unable to malloc ifh->file_name\n");
            return NULL;
            }
        strcpy(ifh->file_name,junk);
        }
    else if(!strcmp(line,"patient ID")) {
	if(!(ifh->patient_id=malloc(sizeof*ifh->patient_id*(len+1)))) {
            printf("Error: Unable to malloc ifh->patient_id\n");
            return NULL;
            }
        strcpy(ifh->patient_id,junk);
        }
    else if(!strcmp(line,"study date")) {
	if(!(ifh->study_date=malloc(sizeof*ifh->study_date*(len+1)))) {
            printf("Error: Unable to malloc ifh->study_date\n");
            return NULL;
            }
        strcpy(ifh->study_date,junk);
        }
    else if(!strcmp(line,"mri parameter file name")) {
	if(!(ifh->mri_param_file=malloc(sizeof*ifh->mri_param_file*(len+1)))) {
            printf("Error: Unable to malloc ifh->mri_param_file\n");
            return NULL;
            }
        strcpy(ifh->mri_param_file,junk);
        }
    else if(!strcmp(line,"mri sequence file name")) {
	if(!(ifh->mri_seq_file=malloc(sizeof*ifh->mri_seq_file*(len+1)))) {
            printf("Error: Unable to malloc ifh->mri_seq_file\n");
            return NULL;
            }
        strcpy(ifh->mri_seq_file,junk);
        }

    else if(!strcmp(line,"mri sequence description")) {
	if(!(ifh->mri_seq_desc=malloc(sizeof*ifh->mri_seq_desc*(len+1)))) {
            printf("Error: Unable to malloc ifh->mri_seq_desc\n");
            return NULL;
            }
        strcpy(ifh->mri_seq_desc,junk);
        }
    else if(!strcmp(line,"paradigm format")) {
	if(!(ifh->paradigm_format=malloc(sizeof*ifh->paradigm_format*(len+1)))) {
            printf("Error: Unable to malloc ifh->paradigm_format\n");
            return NULL;
            }
        strcpy(ifh->paradigm_format,junk);
        }
    else if(!strcmp(line,"global minimum"))
        ifh->global_min = (float)atof(junk);
    else if(!strcmp(line,"global maximum"))
        ifh->global_max = (float)atof(junk);
    else if(!strcmp(line,"Gaussian field smoothness"))
        ifh->smoothness = (float)atof(junk);
    else if(!strcmp(line,"time series flag"))
        ifh->time_series_flag = atoi(junk);
    else if(!strcmp(line,"slice thickness (mm/pixel)"))
        ifh->slice_thickness = (float)atof(junk);
    else if(!strcmp(line,"number of dimensions"))
        ifh->ndim = atoi(junk);
    else if(!strcmp(line,"matrix size [1]"))
        ifh->dim1 = atoi(junk);
    else if(!strcmp(line,"matrix size [2]"))
        ifh->dim2 = atoi(junk);
    else if(!strcmp(line,"matrix size [3]"))
        ifh->dim3 = atoi(junk);
    else if(!strcmp(line,"matrix size [4]"))
        ifh->dim4 = atoi(junk);
    else if(!strcmp(line,"scaling factor (mm/pixel) [1]"))
        ifh->voxel_size_1 = (float)atof(junk);
    else if(!strcmp(line,"scaling factor (mm/pixel) [2]"))
        ifh->voxel_size_2 = (float)atof(junk);
    else if(!strcmp(line,"scaling factor (mm/pixel) [3]"))
        ifh->voxel_size_3 = (float)atof(junk);
    else if(!strcmp(line,"number format")) {
        if(strstr(junk,"byte"))
	    ifh->number_format = (int)BYTE_IF;
        else if(strstr(junk,"short"))
	    ifh->number_format = (int)SHORT_IF;
        else if(strstr(junk,"float"))
	    ifh->number_format = (int)FLOAT_IF;
        else if(strstr(junk,"double"))
	    ifh->number_format = (int)DOUBLE_IF;
	else
	    ifh->number_format = 0;
        }
    else if(!strcmp(line,"orientation"))
        ifh->orientation = atoi(junk);
    else if(!strcmp(line,"number of bytes per pixel"))
        ifh->bytes_per_pix = atoi(junk);
    else if(!strcmp(line,"matrix initial element [1]")) {
        if(!(ifh->origin1=malloc(sizeof*ifh->origin1*(len+1)))) {
            printf("Error: Unable to malloc ifh->origin1\n");
            return NULL;
            }
        strcpy(ifh->origin1,junk);
        }
    else if(!strcmp(line,"matrix initial element [2]")) {
        if(!(ifh->origin2=malloc(sizeof*ifh->origin2*(len+1)))) {
            printf("Error: Unable to malloc ifh->origin2\n");
            return NULL;
            }
        strcpy(ifh->origin2,junk);
        }
    else if(!strcmp(line,"matrix initial element [3]")) {
        if(!(ifh->origin3=malloc(sizeof*ifh->origin3*(len+1)))) {
            printf("Error: Unable to malloc ifh->origin3\n");
            return NULL;
            }
        strcpy(ifh->origin3,junk);
        }
    else if(!strcmp(line,"imagedata byte order")) { /*Assumed to be bigendian*/
        if(strcmp(junk,"bigendian")) ifh->bigendian=0;
        }
    else if(!strcmp(line,"fwhm in voxels"))
        ifh->fwhm = (float)atof(junk);
    else if(!strcmp(line,"region names"))
        ifh->nregions++;
    else if(!strcmp(line,"behavior names"))
        ifh->nbehav++;
    else if(!strcmp(line,"degrees of freedom condition"))
        ifh->df1 = atof(junk);
    else if(!strcmp(line,"degrees of freedom error"))
        ifh->df2 = atof(junk);
    else if(!strcmp(line,"box correction"))
        ifh->box_correction = atof(junk);
    else if(!strcmp(line,"region file"))
        ifh->nregfiles++;
    else if(!strcmp(line,"data description")) {
        if(!(ifh->datades=malloc(sizeof*ifh->datades*(len+1)))) {
            printf("Error: Unable to malloc ifh->datades\n");
            return NULL;
            }
        strcpy(ifh->datades,junk);
        }
    else if(!strcmp(line,"mask file")) {
        if(!(ifh->mask=malloc(sizeof*ifh->mask*(len+1)))) {
            printf("Error: Unable to malloc ifh->mask\n");
            return NULL;
            }
        strcpy(ifh->mask,junk);
        }
    else if(!strcmp(line,"glm revision number"))
        ifh->glm_rev = atoi(junk);
    else if(!strcmp(line,"glm total number of estimates"))
        ifh->glm_Mcol = atoi(junk);    
    else if(!strcmp(line,"glm estimates of interest"))    
        ifh->glm_M_interest = atoi(junk);
    else if(!strcmp(line,"glm column dimension of estimates"))    
        ifh->glm_xdim = atoi(junk);
    else if(!strcmp(line,"glm row dimension of estimates"))    
        ifh->glm_ydim = atoi(junk);
    else if(!strcmp(line,"glm axial dimension of estimates"))    
        ifh->glm_zdim = atoi(junk);
    else if(!strcmp(line,"glm number of frames in raw"))    
        ifh->glm_tdim = atoi(junk);
    else if(!strcmp(line,"glm number of usable frames"))
        ifh->glm_Nrow = atoi(junk);
    else if(!strcmp(line,"glm degrees of freedom"))
        ifh->glm_df = (float)atof(junk);
    else if(!strcmp(line,"glm number of contrasts"))
        ifh->glm_nc = atoi(junk);
    else if(!strcmp(line,"glm BOLD response duration"))
        ifh->glm_period = atoi(junk);
    else if(!strcmp(line,"glm number of trials"))
        ifh->glm_num_trials = atoi(junk); 
    else if(!strcmp(line,"glm temporal sampling rate"))
        ifh->glm_TR = (float)atof(junk);
    else if(!strcmp(line,"glm number of effects of interest")) {
        ifh->glm_tot_eff = atoi(junk);
        if(!(ifh->glm_funclen=malloc(sizeof*ifh->glm_funclen*ifh->glm_tot_eff))) {
            printf("Error: Unable to malloc ifh->glm_funclen\n");
            return NULL;
            }
        for(i=0;i<ifh->glm_tot_eff;i++) ifh->glm_funclen[i]=0;
        if(!(ifh->glm_functype=malloc(sizeof*ifh->glm_functype*ifh->glm_tot_eff))) {
            printf("Error: Unable to malloc ifh->glm_functype\n");
            return NULL;
            }
        for(i=0;i<ifh->glm_tot_eff;i++) ifh->glm_functype[i]=0;
        if(!(ifh->glm_effect_TR=malloc(sizeof*ifh->glm_effect_TR*ifh->glm_tot_eff))) {
            printf("Error: Unable to malloc ifh->glm_effect_TR\n");
            return NULL;
            }
        for(i=0;i<ifh->glm_tot_eff;i++) ifh->glm_effect_TR[i]=0.;
        if(!(ifh->glm_effect_shift_TR=malloc(sizeof*ifh->glm_effect_shift_TR*ifh->glm_tot_eff))) {
            printf("Error: Unable to malloc ifh->glm_effect_shift_TR\n");
            return NULL;
            }
        for(i=0;i<ifh->glm_tot_eff;i++) ifh->glm_effect_shift_TR[i]=0.;
        }

    /*START150320*/
    #if 0
    else if(!strcmp(line,"glm input data space")) {
        if(!(ifh->glm_input_data_space=malloc(sizeof*ifh->glm_input_data_space*(len+1)))) {
            printf("Error: Unable to malloc ifh->glm_input_data_space\n");
            return NULL;
            }
        strcpy(ifh->glm_input_data_space,junk);
        }
    #endif

    else if(!strcmp(line,"glm total number of effects")) {
        ifh->glm_all_eff = atoi(junk);
        if(!(ifh->glm_effect_length=malloc(sizeof*ifh->glm_effect_length*ifh->glm_all_eff))) {
            printf("Error: Unable to malloc ifh->glm_effect_length\n");
            return NULL;
            }
        if(!(ifh->glm_effect_column=malloc(sizeof*ifh->glm_effect_column*ifh->glm_all_eff))) {
            printf("Error: Unable to malloc ifh->glm_effect_column\n");
            return NULL;
            }
        if(!(ifh->glm_leffect_label=malloc(sizeof*ifh->glm_leffect_label*ifh->glm_all_eff))) { 
            printf("Error: Unable to malloc ifh->glm_leffect_label\n");
            return NULL;
            }
        }
    else if(!strcmp(line,"glm effect label"))
        ifh->glm_leffect_label[jlab++] = len+1;
    else if(!strcmp(line,"glm effect length"))
        ifh->glm_effect_length[jlen++] = atoi(junk); 
    else if(!strcmp(line,"glm effect column")) 
        ifh->glm_effect_column[jcol++] = atoi(junk); 
    else if(!strcmp(line,"glm length of encoded function"))
        ifh->glm_funclen[jlab-1] = atoi(junk);
    else if(!strcmp(line,"glm type of encoded function"))
        ifh->glm_functype[jlab-1] = atoi(junk);
    else if(!strcmp(line,"glm effective TR"))
        ifh->glm_effect_TR[jlab-1] = atof(junk);
    else if(!strcmp(line,"glm shift TR"))
        ifh->glm_effect_shift_TR[jlab-1] = atof(junk);
    else if(!strcmp(line,"glm number of bold files"))
        ifh->glm_num_files = atoi(junk);
    else if(!strcmp(line,"glm fwhm in voxels"))
        ifh->glm_fwhm = (float)atof(junk);
    else if(!strcmp(line,"glm transverse voxel dimension"))
        ifh->glm_dxdy = (float)atof(junk);
    else if(!strcmp(line,"glm axial voxel dimension"))
        ifh->glm_dz = (float)atof(junk);
    else if(!strcmp(line,"glm number of F statistics")) {
        ifh->glm_nF = (float)atoi(junk);
        if(!(glm_F_names_len=malloc(sizeof*glm_F_names_len*ifh->glm_nF))) {
            printf("fidlError: Unable to malloc glm_F_names_len\n");
            return NULL;
            }
        if(!(glm_F_names2_len=malloc(sizeof*glm_F_names2_len*ifh->glm_nF))) {
            printf("fidlError: Unable to malloc glm_F_names2_len\n");
            return NULL;
            }
        }
    else if(!strcmp(line,"glm description of F statistic"))
        glm_F_names_len[iF++] = len+1;
    else if(!strcmp(line,"glm name of F statistic"))
        glm_F_names2_len[iF2++] = len+1;
    else if(!strcmp(line,"glm event file")) {
        if(!(ifh->glm_event_file=malloc(sizeof*ifh->glm_event_file*(len+1)))) {
            printf("Error: Unable to malloc ifh->glm_event_file\n");
            return NULL;
            }
        strcpy(ifh->glm_event_file,junk);
        }
    else if(!strcmp(line,"glm mask file")) {
        if(!(ifh->glm_mask_file=malloc(sizeof*ifh->glm_mask_file*(len+1)))) {
            printf("Error: Unable to malloc ifh->glm_mask_file\n");
            return NULL;
            }
        strcpy(ifh->glm_mask_file,junk);
        }

    //START160906
    else if(!strcmp(line,"glm is masked"))
        ifh->glm_masked=atoi(junk);


    else if(!strcmp(line,"glm t4 file")) {
        ifh->glm_nt4files++;
        }

    //START160906
    #if 0
    else if(!strcmp(line,"glm format")) { /*Assumed to be uncompressed*/
        if(!strcmp(junk,"uncompressed")) { /*do nothing*/ }
        else if(!strcmp(junk,"compressed")) ifh->glm_compressed=1;
        }
    #endif

    else if(!strcmp(line,"glm cifti xml number of bytes"))
        ifh->glm_cifti_xmlsize = (unsigned int)strtoul(junk,NULL,10);
    else if(!strcmp(line,"glm bold type")) {
        if(ifh->glm_boldtype)free(ifh->glm_boldtype);
        if(!(ifh->glm_boldtype=malloc(sizeof*ifh->glm_boldtype*(len+1)))) {
            printf("fidlError: Unable to malloc ifh->glm_boldypte\n");
            return NULL;
            }
        strcpy(ifh->glm_boldtype,junk);
        }

    else if(!strcmp(line,"center")) /*Must follow "glm effect label" just in case someone uses center as a condition.*/
        sscanf(junk,"%f %f %f",&ifh->center[0],&ifh->center[1],&ifh->center[2]);
    else if(!strcmp(line,"mmppix"))
        sscanf(junk,"%f %f %f",&ifh->mmppix[0],&ifh->mmppix[1],&ifh->mmppix[2]);
    else if(!strcmp(line,"next seed")) {/*Must follow "glm effect label" just in case someone uses seed as a condition.*/
        if(!(ifh->nextseed=malloc(sizeof*ifh->nextseed*3))) {
            printf("Error: Unable to malloc ifh->nextseed\n");
            return NULL;
            }
        sscanf(junk,"%hu %hu %hu",&ifh->nextseed[0],&ifh->nextseed[1],&ifh->nextseed[2]);
        }
    else if(strstr(line,"START_BINARY")) 
    break;
    }

tst=ifh->nbehav>ifh->nregions?ifh->nbehav:ifh->nregions;
if(ifh->nregfiles>tst) tst=ifh->nregfiles;
if(ifh->glm_all_eff>tst) tst=ifh->glm_all_eff;
if(ifh->glm_nF>tst) tst=ifh->glm_nF;
if(ifh->glm_nt4files>tst) tst=ifh->glm_nt4files;
if(tst) {
    if(!(tstv=malloc(sizeof*tstv*tst))) {
        printf("Error: Unable to malloc tstv\n");
        return NULL;
        }
    }
if(ifh->nbehav) {
    for(i=0,rewind(fp);fgets(line,(int)MAXNAME,fp);) {
        if(strstr(line,"behavior names")) {
            get_line(line,junk,":=",&len);
            tstv[i++] = len+1;
            }
        }
    if(!(ifh->behavior_names=d2charvar(ifh->nbehav,tstv))) return NULL;
    for(i=0,rewind(fp);fgets(line,(int)MAXNAME,fp);) {
        if(strstr(line,"behavior names")) {
            get_line(line,junk,":=",&len);
            strcpy(ifh->behavior_names[i++],junk);
            }
        }
    }
if(ifh->nregions) {
    for(i=0,rewind(fp);fgets(line,(int)MAXNAME,fp);) {
        if(strstr(line,"region names")) {
            get_line(line,junk,":=",&len);
            tstv[i++] = len+1;
            }
        }
    if(!(ifh->region_names=d2charvar(ifh->nregions,tstv))) return NULL;
    for(i=0,rewind(fp);fgets(line,(int)MAXNAME,fp);) {
        if(strstr(line,"region names")) {
            get_line(line,junk,":=",&len);
            strcpy(ifh->region_names[i++],junk);
            }
        }
    }
if(ifh->nregfiles) {
    for(i=0,rewind(fp);fgets(line,(int)MAXNAME,fp);) {
        if(strstr(line,"region file")) {
            get_line(line,junk,":=",&len);
            tstv[i++] = len+1;
            }
        }
    if(!(ifh->regfiles=d2charvar(ifh->nregfiles,tstv))) return NULL;
    for(i=0,rewind(fp);fgets(line,(int)MAXNAME,fp);) {
        if(strstr(line,"region file")) {
            get_line(line,junk,":=",&len);
            strcpy(ifh->regfiles[i++],junk);
            }
        }
    }
if(ifh->glm_tot_eff) for(i=0;i<ifh->glm_tot_eff;i++) if(!ifh->glm_effect_TR[i]) ifh->glm_effect_TR[i] = ifh->glm_TR;
if(jcol) {
    if(ifh->glm_effect_column[0]) {
        for(scrap=i=0;i<jcol;i++) scrap += ifh->glm_effect_column[i];
        if(scrap/jcol == ifh->glm_effect_column[0]) {
            fprintf(stderr,"Warning: ifh->glm_effect_column coded incorrectly. It shall be fixed.\n");
            fprintf(stderr,"Information: scrap=%d jcol=%d ifh->glm_effect_column[0]=%d\n",scrap,jcol,ifh->glm_effect_column[0]);
            ifh->glm_effect_column[0] = 0;
            for(i=1;i<jcol;i++) ifh->glm_effect_column[i] = ifh->glm_effect_column[i-1] + ifh->glm_effect_length[i-1];
            }
        }
    } 
if(jlab) {
    for(i=0;i<ifh->glm_all_eff;i++) tstv[i]=ifh->glm_leffect_label[i];
    if(!(ifh->glm_effect_label=d2charvar(ifh->glm_all_eff,tstv))) return NULL;
    for(jlab=0,rewind(fp);fgets(line,(int)MAXNAME,fp);) {
        if(strstr(line,"glm effect label")) {
            get_line(line,junk,":=",&len);
            strcpy(ifh->glm_effect_label[jlab++],junk);
            }
        else if(strstr(line,"START_BINARY")) {
            break;
            }
        }
    }
if(iF) {
    if(!(ifh->glm_F_names=d2charvar(ifh->glm_nF,glm_F_names_len))) return NULL;
    for(iF=0,rewind(fp);fgets(line,(int)MAXNAME,fp);) {
        if(strstr(line,"glm description of F statistic")) {
            get_line(line,junk,":=",&len);
            strcpy(ifh->glm_F_names[iF++],junk);
            }
        else if(strstr(line,"START_BINARY")) {
            break;
            }
        }
    free(glm_F_names_len);

    /*START140826*/
    if(iF2) {
        if(!(ifh->glm_F_names2=d2charvar(ifh->glm_nF,glm_F_names2_len))) return NULL;
        for(iF=0,rewind(fp);fgets(line,(int)MAXNAME,fp);) {
            if(strstr(line,"glm name of F statistic")) {
                get_line(line,junk,":=",&len);
                strcpy(ifh->glm_F_names2[iF++],junk);
                }
            else if(strstr(line,"START_BINARY")) {
                break;
                }
            }
        }
    free(glm_F_names2_len);

    }
if(ifh->glm_nt4files) {
    for(i=0,rewind(fp);fgets(line,(int)MAXNAME,fp);) {
        if(strstr(line,"glm t4 file")) {
            get_line(line,junk,":=",&len);
            tstv[i++] = len+1;
            }
        }
    if(!(ifh->glm_t4files=d2charvar(ifh->glm_nt4files,tstv))) return NULL;
    for(i=0,rewind(fp);fgets(line,(int)MAXNAME,fp);) {
        if(strstr(line,"glm t4 file")) {
            get_line(line,junk,":=",&len);
            strcpy(ifh->glm_t4files[i++],junk);
            }
        else if(strstr(line,"START_BINARY")) {
            break;
            }

        }
    }
if(tst) free(tstv);
return ifh;
}

int write_ifh(char *filename,Interfile_header *ifh,int glm_text_hdr){
    char    hdrfil[MAXNAME];
    FILE    *fp;

    strcpy(hdrfil,filename);
    *strrchr(hdrfil,'.') = 0;
    strcat(hdrfil,".ifh");
    if(!(fp=fopen(hdrfil,"w"))) {
        fprintf(stdout,"Error: Could not open %s in write_ifh\n",filename);
        return 0;
        }
    put_ifh(fp,ifh,glm_text_hdr);
    fclose(fp);
    return 1;
    }
void put_ifh(FILE *fp,Interfile_header *ifh,int glm_text_hdr)
{                                            /* 1:print glm hdr*/
    char *string=NULL;
    int	i;
    float	x;
    fprintf(fp,"INTERFILE                         :=\n");
    fprintf(fp,"version of keys                   := 3.3\n");
    if(ifh->image_modality != NULL)
        fprintf(fp,"image modality                    := %s\n",ifh->image_modality);
    if(ifh->orig_system != NULL)
        fprintf(fp,"originating system                := %s\n",ifh->orig_system);
    if(ifh->prog_version != NULL)
        fprintf(fp,"program version                   := %s\n",ifh->prog_version);
    if(ifh->orig_inst != NULL)
        fprintf(fp,"original institution              := %s\n",ifh->orig_inst);
    if(ifh->file_name != NULL)
        fprintf(fp,"name of data file                 := %s\n",ifh->file_name);
    if(ifh->patient_id != NULL)
        fprintf(fp,"patient ID                        := %s\n",ifh->patient_id);
    if(ifh->study_date != NULL)
        fprintf(fp,"study date                        := %s\n",ifh->study_date);
    switch(ifh->number_format) {
        case (int)BYTE_IF:
            string = "byte";
	    break;
        case (int)SHORT_IF:
            string = "short";
	    break;
        case (int)FLOAT_IF:
            string = "float";
	    break;
        case (int)DOUBLE_IF:
            string = "double";
	    break;
        default:
            fprintf(stderr,"Invalid number format in put_ifh\tifh->number_format= %d\n",ifh->number_format);
        }
    fprintf(fp,"number format                     := %s\n",string);
    fprintf(fp,"number of bytes per pixel         := %d\n",ifh->bytes_per_pix);
    fprintf(fp,"orientation                       := %d\n",ifh->orientation);
    fprintf(fp,"number of dimensions              := %d\n",ifh->ndim);
    fprintf(fp,"matrix size [1]                   := %d\n",ifh->dim1);
    fprintf(fp,"matrix size [2]                   := %d\n",ifh->dim2);
    fprintf(fp,"matrix size [3]                   := %d\n",ifh->dim3);
    fprintf(fp,"matrix size [4]                   := %d\n",ifh->dim4);
    fprintf(fp,"scaling factor (mm/pixel) [1]     := %f\n",ifh->voxel_size_1);
    fprintf(fp,"scaling factor (mm/pixel) [2]     := %f\n",ifh->voxel_size_2);
    fprintf(fp,"scaling factor (mm/pixel) [3]     := %f\n",ifh->voxel_size_3);
    if(ifh->origin1 != NULL)
        fprintf(fp,"matrix initial element [1]        := %s\n",ifh->origin1);
    if(ifh->origin2 != NULL)
        fprintf(fp,"matrix initial element [2]        := %s\n",ifh->origin2);
    if(ifh->origin3 != NULL)
        fprintf(fp,"matrix initial element [3]        := %s\n",ifh->origin3);
    fprintf(fp,"imagedata byte order              := %s\n",!ifh->bigendian?"littleendian":"bigendian");
    if(ifh->global_min || ifh->global_max) {
        if(ifh->global_min != (float)UNSAMPLED_VOXEL)
            fprintf(fp,"global minimum                    := %f\n",ifh->global_min);
        if(ifh->global_max != (float)UNSAMPLED_VOXEL)
            fprintf(fp,"global maximum                    := %f\n",ifh->global_max);
        }
    if(ifh->mri_param_file)
        fprintf(fp,"mri parameter file name           := %s\n",ifh->mri_param_file);
    if(ifh->mri_seq_file)
        fprintf(fp,"mri sequence file name            := %s\n",ifh->mri_seq_file);
    if(ifh->mri_seq_desc)
        fprintf(fp,"mri sequence description          := %s\n",ifh->mri_seq_desc);
    if(ifh->paradigm_format)
        fprintf(fp,"paradigm format                   := %s\n",ifh->paradigm_format);
    for(i=0,x=0.;i<3;i++)
        x += ifh->center[i]*ifh->center[i];
    if(x > 0.) 
        fprintf(fp,"center                            := %f %f %f\n",ifh->center[0],ifh->center[1],ifh->center[2]);
    for(i=0,x=0.;i<3;i++)
        x += ifh->mmppix[i]*ifh->mmppix[i];
    if(x > 0.) 
        fprintf(fp,"mmppix                            := %f %f %f\n",ifh->mmppix[0],ifh->mmppix[1],ifh->mmppix[2]);
    if(ifh->df1)
        fprintf(fp,"degrees of freedom condition      := %f\n",ifh->df1);
    if(ifh->df2)
        fprintf(fp,"degrees of freedom error          := %f\n",ifh->df2);
    if(ifh->box_correction != (float)UNSAMPLED_VOXEL)
        fprintf(fp,"box correction                    := %f\n",ifh->box_correction);
    fprintf(fp,"fwhm in voxels                    := %f\n",ifh->fwhm);
    if(ifh->datades) fprintf(fp,"data description                  := %s\n",ifh->datades);
    if(ifh->mask) fprintf(fp,"mask file                         := %s\n",ifh->mask);
    if(ifh->nextseed) fprintf(fp,"next seed                         := %hu %hu %hu\n",ifh->nextseed[0],ifh->nextseed[1],
        ifh->nextseed[2]); 
    if(glm_text_hdr) {
        if(ifh->glm_rev < 0) {
            if(ifh->glm_rev)
                fprintf(fp,"glm revision number               := %d\n",ifh->glm_rev);
            if(ifh->glm_Mcol)
                fprintf(fp,"glm total number of estimates     := %d\n",ifh->glm_Mcol);
            if(ifh->glm_M_interest)
                fprintf(fp,"glm estimates of interest         := %d\n",ifh->glm_M_interest);
            if(ifh->glm_xdim)
                fprintf(fp,"glm column dimension of estimates := %d\n",ifh->glm_xdim);
            if(ifh->glm_ydim)
                fprintf(fp,"glm row dimension of estimates    := %d\n",ifh->glm_ydim);
            if(ifh->glm_zdim)
                fprintf(fp,"glm axial dimension of estimates  := %d\n",ifh->glm_zdim);
            if(ifh->glm_tdim)
                fprintf(fp,"glm number of frames in raw       := %d\n",ifh->glm_tdim);
            if(ifh->glm_Nrow)
                fprintf(fp,"glm number of usable frames       := %d\n",ifh->glm_Nrow);
            if(ifh->glm_df)
                fprintf(fp,"glm degrees of freedom            := %g\n",ifh->glm_df);
            if(ifh->glm_nc)
                fprintf(fp,"glm number of contrasts           := %d\n",ifh->glm_nc);
            if(ifh->glm_period)
                fprintf(fp,"glm BOLD response duration        := %d\n",ifh->glm_period);
            if(ifh->glm_num_trials)
                fprintf(fp,"glm number of trials              := %d\n",ifh->glm_num_trials);
            if(ifh->glm_TR)
                fprintf(fp,"glm temporal sampling rate        := %f\n",ifh->glm_TR);
            fprintf(fp,"glm number of effects of interest := %d\n",ifh->glm_tot_eff); 
            fprintf(fp,"glm fwhm in voxels                := %f\n",ifh->glm_fwhm);
            if(ifh->glm_all_eff) {
                fprintf(fp,"glm total number of effects       := %d\n",ifh->glm_all_eff);
                for(i=0;i<ifh->glm_all_eff;i++) {
                    fprintf(fp,"glm effect label                  := %s\n",ifh->glm_effect_label[i]);
                    fprintf(fp,"glm effect length                 := %d\n",ifh->glm_effect_length[i]);
                    fprintf(fp,"glm effect column                 := %d\n",ifh->glm_effect_column[i]);
                    if(i < ifh->glm_tot_eff) {
                        if(ifh->glm_funclen[i]) {
                            fprintf(fp,"glm length of encoded function    := %d\n",ifh->glm_funclen[i]);
                            fprintf(fp,"glm type of encoded function      := %d\n",ifh->glm_functype[i]);
                            }
                        if(ifh->glm_effect_TR[i]) {
                            fprintf(fp,"glm effective TR                  := %f\n",ifh->glm_effect_TR[i]);
                            fprintf(fp,"glm shift TR                      := %f\n",ifh->glm_effect_shift_TR[i]);
                            }
                        }
                    }
                }
            if(ifh->glm_num_files) fprintf(fp,"glm number of bold files          := %d\n",ifh->glm_num_files);
            if(ifh->glm_W) fprintf(fp,"glm random field smoothness       := %f\n",ifh->glm_W);
            if(ifh->glm_dxdy) fprintf(fp,"glm transverse voxel dimension    := %f\n",ifh->glm_dxdy);
            if(ifh->glm_dz) fprintf(fp,"glm axial voxel dimension         := %f\n",ifh->glm_dz);
            if(ifh->glm_nF) {
                fprintf(fp,"glm number of F statistics        := %d\n",ifh->glm_nF);
                for(i=0;i<ifh->glm_nF;i++) fprintf(fp,"glm description of F statistic    := %s\n",ifh->glm_F_names[i]);
                if(ifh->glm_F_names2)
                    for(i=0;i<ifh->glm_nF;i++) fprintf(fp,"glm name of F statistic           := %s\n",ifh->glm_F_names2[i]);
                }
	    if(ifh->glm_event_file)fprintf(fp,"glm event file                    := %s\n",ifh->glm_event_file);
	    if(ifh->glm_mask_file) fprintf(fp,"glm mask file                     := %s\n",ifh->glm_mask_file);

            //START160906
	    if(ifh->glm_masked)    fprintf(fp,"glm is masked                     := %d\n",ifh->glm_masked);

            for(i=0;i<ifh->glm_nt4files;i++) fprintf(fp,"glm t4 file                       := %s\n",ifh->glm_t4files[i]);

            //START160908
            //fprintf(fp,"glm format                        := %s\n",!ifh->glm_compressed?"uncompressed":"compressed");

	    if(ifh->glm_boldtype) fprintf(fp,"glm bold type                     := %s\n",ifh->glm_boldtype);
            if(ifh->glm_cifti_xmlsize) fprintf(fp,"glm cifti xml number of bytes     := %u\n",ifh->glm_cifti_xmlsize);
            }
        }
    for(i=0;i<ifh->nregions;i++) fprintf(fp,"region names                     := %s\n",ifh->region_names[i]);
    for(i=0;i<ifh->nbehav;i++)   fprintf(fp,"behavior names                   := %s\n",ifh->behavior_names[i]);
    for(i=0;i<ifh->nregfiles;i++)fprintf(fp,"region file                      := %s\n",ifh->regfiles[i]);
    }

//Interfile_header *init_ifh(int ndim,int dim1,int dim2,int dim3,int dim4,float dx1,float dx2,float dx3,int bigendian)
//START161104
Interfile_header *init_ifh(int ndim,int dim1,int dim2,int dim3,int dim4,float dx1,float dx2,float dx3,int bigendian,float *center,
    float *mmppix)

{
    Interfile_header *ifh;
    if(!(ifh=init_ifh0((Interfile_header*)NULL))) return NULL;
    ifh->number_format = FLOAT_IF;
    ifh->bytes_per_pix = 4;
    ifh->orientation = 2;
    ifh->time_series_flag = 0;
    ifh->ndim = ndim;
    ifh->dim1 = dim1;
    ifh->dim2 = dim2;
    ifh->dim3 = dim3;
    ifh->dim4 = dim4;
    ifh->voxel_size_1 = dx1;
    ifh->voxel_size_2 = dx2;
    ifh->voxel_size_3 = dx3;
    ifh->bigendian = bigendian;

    //START161104
    if(center){ifh->center[0]=center[0];ifh->center[1]=center[1];ifh->center[2]=center[2];}
    if(mmppix){ifh->mmppix[0]=mmppix[0];ifh->mmppix[1]=mmppix[1];ifh->mmppix[2]=mmppix[2];}

    if(!(ifh->origin1=malloc(sizeof*ifh->origin1*6))) {
        printf("Error: Unable to malloc ifh->origin1\n");
        return NULL;
        }
    strcpy(ifh->origin1,"right");
    if(!(ifh->origin2=malloc(sizeof*ifh->origin2*10))) {
        printf("Error: Unable to malloc origin2\n");
        return NULL;
        }
    strcpy(ifh->origin2,"posterior");
    if(!(ifh->origin3=malloc(sizeof*ifh->origin3*9))) {
        printf("Error: Unable to malloc origin3\n");
        return NULL;
        }
    strcpy(ifh->origin3,"inferior");
    return(ifh);
}
void free_ifh(Interfile_header *ifh,int dontfreeifh)
{
    if(ifh->image_modality)free(ifh->image_modality);
    if(ifh->orig_system)free(ifh->orig_system);
    if(ifh->prog_version)free(ifh->prog_version);
    if(ifh->prog_date)free(ifh->prog_date);
    if(ifh->orig_inst)free(ifh->orig_inst);
    if(ifh->file_name)free(ifh->file_name);
    if(ifh->patient_id)free(ifh->patient_id);
    if(ifh->study_date)free(ifh->study_date);
    if(ifh->mri_param_file)free(ifh->mri_param_file);
    if(ifh->mri_seq_file)free(ifh->mri_seq_file);
    if(ifh->mri_seq_desc)free(ifh->mri_seq_desc);
    if(ifh->paradigm_format)free(ifh->paradigm_format);
    if(ifh->origin1)free(ifh->origin1);
    if(ifh->origin2)free(ifh->origin2);
    if(ifh->origin3)free(ifh->origin3);
    if(ifh->datades) free(ifh->datades);
    if(ifh->mask) free(ifh->mask);
    if(ifh->glm_all_eff) {
        if(ifh->glm_effect_length)free(ifh->glm_effect_length);
        if(ifh->glm_effect_column)free(ifh->glm_effect_column);
        if(ifh->glm_effect_label) {
            free(ifh->glm_effect_label[0]);
            free(ifh->glm_effect_label);
            }
        if(ifh->glm_leffect_label)free(ifh->glm_leffect_label);
        if(ifh->glm_F_names) {
            free(ifh->glm_F_names[0]);
            free(ifh->glm_F_names);
            }
        if(ifh->glm_F_names2) {
            free(ifh->glm_F_names2[0]);
            free(ifh->glm_F_names2);
            }
        if(ifh->glm_effect_TR)free(ifh->glm_effect_TR);
        if(ifh->glm_effect_shift_TR)free(ifh->glm_effect_shift_TR);
        if(ifh->glm_funclen)free(ifh->glm_funclen);
        if(ifh->glm_functype)free(ifh->glm_functype);
        if(ifh->glm_event_file)free(ifh->glm_event_file);
        if(ifh->glm_mask_file) free(ifh->glm_mask_file);
        if(ifh->glm_nt4files) {
            free(ifh->glm_t4files[0]);
            free(ifh->glm_t4files);
            ifh->glm_nt4files = 0;
            }
        if(ifh->glm_boldtype)free(ifh->glm_boldtype);
        }
    if(ifh->nbehav) {
        free(ifh->behavior_names[0]);
        free(ifh->behavior_names);
        }
    if(ifh->nregfiles) {
        free(ifh->regfiles[0]);
        free(ifh->regfiles);
        ifh->nregfiles = 0;
        }
    if(ifh->nregions) free_ifhregnames(ifh); 
    if(ifh->nextseed) free(ifh->nextseed);
    if(!dontfreeifh) free(ifh);
}
void free_ifhregnames(Interfile_header *ifh)
{
    free(ifh->region_names[0]);
    free(ifh->region_names); /*Bus error with compute_region_clusters with only two regions to cluster.*/
    ifh->nregions = 0;
}
Interfile_header *init_ifh0(Interfile_header *ifh1)
{
    Interfile_header *ifh;
    if(!(ifh = ifh1? ifh1 : malloc(sizeof*ifh))) {
        printf("Error: Unable to malloc ifh\n");
        return NULL;
        }
    ifh->glm_nF = 0;
    ifh->mmppix[0] = 0.;
    ifh->mmppix[1] = 0.;
    ifh->mmppix[2] = 0.;
    ifh->center[0] = 0.;
    ifh->center[1] = 0.;
    ifh->center[2] = 0.;
    ifh->fwhm=-1.;
    ifh->glm_fwhm = 0;
    ifh->glm_fwhmx = 0.;
    ifh->glm_fwhmy = 0.;
    ifh->glm_fwhmz = 0.;
    ifh->df1 = 0.;
    ifh->df2 = 0.;
    ifh->box_correction = (float)UNSAMPLED_VOXEL;
    ifh->glm_tot_eff = 0;
    ifh->image_modality = (char *)NULL;
    ifh->orig_system = (char *)NULL;
    ifh->prog_version = (char *)NULL;
    ifh->prog_date = (char *)NULL;
    ifh->orig_inst = (char *)NULL;
    ifh->file_name = (char *)NULL;
    ifh->patient_id = (char *)NULL;
    ifh->study_date = (char *)NULL;
    ifh->mri_param_file = (char *)NULL;
    ifh->mri_seq_file = (char *)NULL;
    ifh->mri_seq_desc = (char *)NULL;
    ifh->paradigm_format = (char *)NULL;
    ifh->origin1 = (char *)NULL;
    ifh->origin2 = (char *)NULL;
    ifh->origin3 = (char *)NULL;
    ifh->smoothness=(float)UNSAMPLED_VOXEL;
    ifh->bigendian = 1;
    ifh->glm_event_file = (char *)NULL;
    ifh->glm_mask_file = (char *)NULL;

    //START160906
    ifh->glm_masked=0;

    ifh->glm_effect_column = (int *)NULL;
    ifh->glm_all_eff = 0;
    ifh->glm_nc = 0;
    ifh->nregions = 0;
    ifh->region_names = (char**)NULL;
    ifh->nbehav = 0;
    ifh->nregfiles = 0;
    ifh->datades = (char*)NULL;
    ifh->mask = (char*)NULL;
    ifh->nextseed = NULL;
    ifh->glm_nt4files = 0;

    //START160908
    //ifh->glm_compressed = 0;

    ifh->global_min = 0.;
    ifh->global_max = 0.;
    ifh->glm_F_names2 = NULL;
    ifh->glm_cifti_xmlsize=0;
    if(!(ifh->glm_boldtype=malloc(sizeof*ifh->glm_boldtype*4))) {
        printf("fidlError: init_ifh0 Unable to malloc ifh->glm_boldypte\n");
        return NULL;
        }
    strcpy(ifh->glm_boldtype,"img");
    return ifh;
}
