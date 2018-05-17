/* Copyright 2/10/12 Washington University.  All Rights Reserved.
   read_driver2.c  $Revision: 1.21 $ */
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//START170405
#include <errno.h>

#include "fidl.h"
#include "anova_header_new.h"
#include "subs_util.h"
#include "read_driver2.h"

//START180116
#include "make_timestr2.h"

#if 0
static char rcsid[] = "$Header: /home/hannah/mcavoy/idl/clib/RCS/read_driver2.c,v 1.21 2017/04/06 19:18:56 mcavoy Exp $";
#endif

//AnovaDesign *read_driver2(char *driver_file,int nreg,int not_num_factors,int lclevelall,char separator) {
//START170207
AnovaDesign *read_driver2(char *driver_file,int nreg,int not_num_factors,int lclevelall,char separator,int dontcheck) {

    char line[MAXNAME],write_back[MAXNAME],*write_back_ptr=NULL,*strptr,string[MAXNAME],timestr[23], //timestr[16],
        last_char,first_char,*saveptr,*saveptr2,*strptr2=NULL; /* string2[MAXNAME],*strptrfstat */
    int i,j,k,l,m,nstrings,nfiles,nlines=0,addr,lc_data,address_check,*replications_check,*goose,*flagarr,len,len2,nhdr,*factori,
        lcchunks=0; /* notfactort=0,index,nfact */
    size_t *hdrl,kk; /* ii,jj,ll */
    AnovaDesign *ad;
    FILE *fp,**fpf;
    if(!(ad=malloc(sizeof*ad))) {
        printf("fidlError: read_driver2 Unable to malloc ad\n");fflush(stdout);return NULL;
        }
    ad->num_datafiles = 0;
    if(!(fp = fopen_sub(driver_file,"r"))) return NULL;
    if(!fgets(line,sizeof(line),fp)) {
	printf("fidlError: read_driver2 %s is empty.\n",driver_file);fflush(stdout);return NULL;
	}
    for(nhdr=0,strptr=line;strtok_r(strptr," 	\n",&strptr);nhdr++);
    if((ad->num_factors0 = nhdr - not_num_factors) < 1) {
        printf("fidlError: read_driver2 List the factor names on the first line of %s.\n",driver_file);fflush(stdout);return NULL;
        }
    else if(ad->num_factors0<2) {
        printf("fidlError: read_driver2 You need to run a T test instead.\n");fflush(stdout);return NULL;
        }
    if(!(hdrl=malloc(sizeof*hdrl*nhdr))) {
        printf("fidlError: read_driver2 Unable to malloc hdrl\n");fflush(stdout);return NULL;
        }
    for(strptr=line,i=0;i<nhdr;i++,strptr++) {
        write_back_ptr=strtok_r(strptr," 	\n",&strptr);
        hdrl[i] = strlen(write_back_ptr)+1;
        }


    #if 0
    if(!(ad->fnptr=d2charvar(nhdr,hdrl)))return NULL;
    for(lc_data=lcchunks=0,ad->chunks=1,strptr=line,i=0;i<nhdr;i++,strptr++) {
        write_back_ptr=strtok_r(strptr," 	\n",&strptr);
        strcpy(ad->fnptr[i],write_back_ptr);
        if(strstr(write_back_ptr,"CHUNKS")) {
            ad->chunks=(int)strtol(write_back_ptr+6,NULL,10);
            lcchunks=1;
            }
        else if(!strcmp(write_back_ptr,"DATA"))lc_data=1;
        }
    #endif
    #if 0
    //START160407
    if(!(ad->fnptr0=d2charvar(nhdr,hdrl)))return NULL;
    for(lc_data=lcchunks=0,ad->chunks=1,strptr=line,i=0;i<nhdr;i++,strptr++) {
        write_back_ptr=strtok_r(strptr," 	\n",&strptr);
        strcpy(ad->fnptr0[i],write_back_ptr);
        if(strstr(write_back_ptr,"CHUNKS")) {
            ad->chunks=(int)strtol(write_back_ptr+6,NULL,10);
            lcchunks=1;
            }
        else if(!strcmp(write_back_ptr,"DATA"))lc_data=1;
        }
    #endif
    //START180130
    if(!(ad->fnptr0=d2charvar(nhdr,hdrl)))return NULL;
    for(lc_data=lcchunks=0,ad->chunks=1,strptr=line,i=0;i<nhdr;i++,strptr++) {
        write_back_ptr=strtok_r(strptr," 	\n",&strptr);
        strcpy(ad->fnptr0[i],write_back_ptr);
        if(!strcmp(write_back_ptr,"CHUNKS1")){
            printf("write_back_ptr=%s\n",write_back_ptr);fflush(stdout);
            }
        else if(strstr(write_back_ptr,"CHUNKS")) {
            ad->chunks=(int)strtol(write_back_ptr+6,NULL,10);
            lcchunks=1;
            }
        else if(!strcmp(write_back_ptr,"DATA"))lc_data=1;
        }

    free(hdrl);
    if(!(ad->level=malloc(sizeof*ad->level*ad->num_factors0))) {
        printf("fidlError: read_driver2 Unable to malloc ad->level\n");fflush(stdout);return NULL;
        }


    #if 0
    if(!(ad->Perlman_levels=malloc(sizeof*ad->Perlman_levels*ad->num_factors0))) {
        printf("fidlError: read_driver2 Unable to malloc ad->Perlman_levels\n");fflush(stdout);return NULL;
        }
    for(i=0;i<ad->num_factors0;i++)ad->Perlman_levels[i]=0;
    #endif
    //START160408
    if(!(ad->Perlman_levels0=malloc(sizeof*ad->Perlman_levels0*ad->num_factors0))) {
        printf("fidlError: read_driver2 Unable to malloc ad->Perlman_levels0\n");fflush(stdout);return NULL;
        }
    for(i=0;i<ad->num_factors0;i++)ad->Perlman_levels0[i]=0;


    if(!(fpf=malloc(sizeof*fpf*ad->num_factors0))) {
        printf("fidlError: read_driver2 Unable to malloc fpf\n");fflush(stdout);return NULL;
        }
    make_timestr2(timestr);
    for(i=0;i<ad->num_factors0;i++) {
        sprintf(line,".fidl%d_%s.dat",i+1,timestr);
        if(!(fpf[i]=fopen_sub(line,"w+"))) return NULL;
        }
    for(len=0;fgets(line,sizeof(line),fp);) {
        if((nstrings=count_strings_new3(line,write_back,' ',' ',&last_char,&first_char,0,' '))>1) {
            if((nfiles = nstrings - ad->num_factors0 - not_num_factors+1) < 1) {
                printf("fidlError: read_driver2 %s need to list the subject number and file name.\n",driver_file);
                fflush(stdout);return NULL;
                }
            else if(nfiles>1) {
                printf("fidlError: read_driver2 nfiles=%d nstrings=%d ad->num_factors0=%d  Not set up to handle more than 1 file.\n",
                    nfiles,nstrings,ad->num_factors0);
                printf("fidlError: write_back = %s\n",write_back);
                fflush(stdout);return NULL;
                }
            for(strptr=write_back,i=0;i<ad->num_factors0;i++,strptr=NULL) {
                write_back_ptr=strtok_r(strptr," ",&saveptr);
                for(j=0;fgets(string,sizeof(string),fpf[i]);j++) {
                    strptr2=strtok_r(string,"     \n",&saveptr2);
                    if(!strcmp(strptr2,write_back_ptr))break;
                    }

                #if 0
                if(j==ad->Perlman_levels[i]) {
                    ad->Perlman_levels[i]++;
                #endif
                //START160408
                if(j==ad->Perlman_levels0[i]) {
                    ad->Perlman_levels0[i]++;

                    len+=strlen(write_back_ptr)+1;
                    sprintf(line,"%s\n",write_back_ptr);
                    fputs(line,fpf[i]);
                    }
                rewind(fpf[i]);
                }
            }
        }

    #if 0
    for(j=0,i=0;i<ad->num_factors0;i++)j+=ad->Perlman_levels[i];
    if(!(ad->lnptr=malloc(sizeof*ad->lnptr*j))) {
        printf("fidlError: read_driver2 Unable to malloc ad->lnptr\n");fflush(stdout);return NULL;
        }
    #endif
    //START160408
    for(j=0,i=0;i<ad->num_factors0;i++)j+=ad->Perlman_levels0[i];
    if(!(ad->lnptr0=malloc(sizeof*ad->lnptr0*j))) {
        printf("fidlError: read_driver2 Unable to malloc ad->lnptr0\n");fflush(stdout);return NULL;
        }

    //printf("here98 ad->Perlman_levels0=");for(i=0;i<ad->num_factors0;i++)printf("%d ",ad->Perlman_levels0[i]);printf("\n");

    if(!(ad->ln=malloc(sizeof*ad->ln*len))) {
        printf("fidlError: read_driver2 Unable to malloc ad->ln\n");fflush(stdout);return NULL;
        }

    #if 0
    for(strptr=ad->ln,j=i=0;i<ad->num_factors0;i++) {
        for(;fgets(line,sizeof(line),fpf[i]);) {
            saveptr2=strtok_r(line," 	\n",&saveptr);
            strcpy(strptr,saveptr2);
            ad->lnptr0[j++] = strptr;
            strptr+=strlen(saveptr2)+1;
            }
        fclose(fpf[i]);
        }
    #endif
    //START170405
    if(!(ad->numeric0=malloc(sizeof*ad->numeric0*ad->num_factors0)))
        {printf("fidlError: read_driver2 Unable to malloc ad->numeric0\n");fflush(stdout);return NULL;}
    for(i=0;i<ad->num_factors0;i++)ad->numeric0[i]=0;
    for(strptr=ad->ln,j=i=0;i<ad->num_factors0;i++) {
        for(;fgets(line,sizeof(line),fpf[i]);) {
            saveptr2=strtok_r(line," \t\n",&saveptr);
            strcpy(strptr,saveptr2);
            ad->lnptr0[j++]=strptr;

            errno=0;
            strptr2=strptr;
            strtod(strptr,&strptr2); 
            if(errno!=0); //conversion failed (EINVAL,ERANGE)
            else if(strptr==strptr2); //conversion failed (no characters consumed) 
            else if(*strptr2!=0); //conversion failed (trailing data)
            else{ad->numeric0[i]=1;}

            strptr+=strlen(saveptr2)+1;
            }
        fclose(fpf[i]);
        }

    //printf("here99 ad->numeric0=");for(i=0;i<ad->num_factors0;i++)printf("%d ",ad->numeric0[i]);printf("\n");

    sprintf(string,"rm -f .fidl*_%s.dat",timestr);
    if(system(string)==-1)printf("fidError: read_driver2 Unable to %s\n",string);

    //for(ad->num_cells_notRANDOM=1,i=1;i<ad->num_factors0;i++)ad->num_cells_notRANDOM*=ad->Perlman_levels[i];
    //START160408
    for(ad->num_cells_notRANDOM=1,i=1;i<ad->num_factors0;i++)ad->num_cells_notRANDOM*=ad->Perlman_levels0[i];


    if(!(replications_check=malloc(sizeof*replications_check*ad->num_cells_notRANDOM))){
        printf("fidlError: read_driver2 Unable to malloc replications_check\n");fflush(stdout);return NULL;
        }
    if(!(ad->notfactor=malloc(sizeof*ad->notfactor*ad->num_factors0))){
        printf("fidlError: read_driver2 Unable to malloc ad->sumcount\n");fflush(stdout);return NULL;
        }
    for(i=0;i<ad->num_factors0;i++)ad->notfactor[i]=0;
    if(!(goose=malloc(sizeof*goose*(ad->num_factors0+1)))) {
        printf("fidlError: read_driver2 Unable to malloc goose\n");fflush(stdout);return NULL;
        }


    #if 0
    //KEEP
    for(m=0;m<ad->num_factors0;m++){
        for(ad->num_cells_notRANDOM=1,i=1;i<ad->num_factors0;i++)if(!ad->notfactor[i])ad->num_cells_notRANDOM*=ad->Perlman_levels[i];
        for(i=0;i<ad->num_cells_notRANDOM;i++)replications_check[i]=0;
        for(rewind(fp),fgets(line,sizeof(line),fp);fgets(line,sizeof(line),fp);) {
            if((nfiles=count_strings_new3(line,write_back,' ',' ',&last_char,&first_char,0,' ')-ad->num_factors0)>0) {
                get_level(write_back,ad);
                address_check=offset2(&ad->level[1],ad->num_factors0-1,&ad->Perlman_levels[1],&ad->notfactor[1]);
                replications_check[address_check] += nfiles;
                }
            }
        for(i=0;i<ad->num_factors0+1;i++)goose[i]=1;
        for(nfact=0,index=i=ad->num_factors0;--i>=0;)if(!ad->notfactor[i]){
            goose[index-1]=goose[index]*ad->Perlman_levels[i];
            index--;nfact++;
            }
        for(i=0;i<ad->num_cells_notRANDOM;i++){
            if(!replications_check[i]){
                for(j=0;j<ad->num_factors0;j++)ad->level[j]=0;
                for(index=i,j=nfact;index>goose[--j];);
                for(l=j,k=j+1;k<nfact;k++,l++) {
                    ad->level[l]=index/goose[k];
                    index-=ad->level[l]*goose[k];
                    }
                ad->notfactor[l]=1;
                }
            }
        }
    #endif
    #if 0
    //START160408
    for(m=1;m<ad->num_factors0;m++){
        for(ad->num_cells_notRANDOM=1,i=1;i<=m;i++)if(!ad->notfactor[i])ad->num_cells_notRANDOM*=ad->Perlman_levels0[i];
        for(i=0;i<ad->num_cells_notRANDOM;i++)replications_check[i]=0;
        for(rewind(fp),fgets(line,sizeof(line),fp);fgets(line,sizeof(line),fp);) {
            if((nfiles=count_strings_new3(line,write_back,' ',' ',&last_char,&first_char,0,' ')-ad->num_factors0)>0) {
                get_level2(write_back,ad,m+1);
                address_check=offset2(&ad->level[1],m,&ad->Perlman_levels0[1],&ad->notfactor[1]);
                replications_check[address_check] += nfiles;
                }
            }
        //for(i=0;i<ad->num_cells_notRANDOM;i++)if(!replications_check[i]){ad->notfactor[m]=1;break;}
        //START160506
        for(i=0;i<ad->num_cells_notRANDOM;i++){
            //printf("m=%d replications_check[%d]=%d\n",m,i,replications_check[i]);

            //if(replications_check[i]<2){ad->notfactor[m]=1;break;}
            //START170302
            if(replications_check[i]<2&&ad->Perlman_levels0[0]>1){ad->notfactor[m]=1;break;}

            }
        }
    #endif
    //START170405
    for(m=1;m<ad->num_factors0;m++){
        for(ad->num_cells_notRANDOM=1,i=1;i<=m;i++)if(!ad->notfactor[i])ad->num_cells_notRANDOM*=ad->Perlman_levels0[i];
        for(i=0;i<ad->num_cells_notRANDOM;i++)replications_check[i]=0;
        for(rewind(fp),fgets(line,sizeof(line),fp);fgets(line,sizeof(line),fp);) {
            if((nfiles=count_strings_new3(line,write_back,' ',' ',&last_char,&first_char,0,' ')-ad->num_factors0)>0) {
                get_level2(write_back,ad,m+1);
                address_check=offset2(&ad->level[1],m,&ad->Perlman_levels0[1],&ad->notfactor[1]);
                replications_check[address_check] += nfiles;
                }
            }
        for(i=0;i<ad->num_cells_notRANDOM;i++){
            if(replications_check[i]<2&&ad->Perlman_levels0[0]>1){ad->notfactor[m]=1;break;}
            }
        }

    
    free(goose);
    free(replications_check);
    for(ad->nnotfactor=0,i=0;i<ad->num_factors0;ad->nnotfactor+=ad->notfactor[i++]);
    ad->num_factors=ad->num_factors0-ad->nnotfactor;
    ad->num_sources = 1 << ad->num_factors;
    ad->aa = ad->num_sources/2;
    ad->aa1 = ad->num_sources/2-1;
    ad->aa0=ad->num_sources/2-1+ad->nnotfactor;


    //START160408
    if(!(ad->Perlman_levels=malloc(sizeof*ad->Perlman_levels*ad->num_factors))) {
        printf("fidlError: read_driver2 Unable to malloc ad->Perlman_levels\n");fflush(stdout);return NULL;
        }
    for(j=0,i=0;i<ad->num_factors0;i++)if(!ad->notfactor[i])ad->Perlman_levels[j++]=ad->Perlman_levels0[i];

    #if 0
    for(ad->num_cells_notRANDOM=1,j=0,i=1;i<ad->num_factors0;i++)if(!ad->notfactor[i]){
        ad->num_cells_notRANDOM*=ad->Perlman_levels[j++]=ad->Perlman_levels0[i];
        }
    #endif
    for(ad->num_cells_notRANDOM=1,i=1;i<ad->num_factors;i++)ad->num_cells_notRANDOM*=ad->Perlman_levels[i];


    if(!(ad->fnptr=malloc(sizeof*ad->fnptr*ad->num_factors))){
        printf("fidlError: read_driver2 Unable to malloc ad->fnptr\n");
        return NULL;
        }

    #if 0
    for(ad->num_cells_notRANDOM=1,k=j=0,i=1;i<ad->num_factors0;i++)if(!ad->notfactor[i]){
        ad->num_cells_notRANDOM*=ad->Perlman_levels[j]=ad->Perlman_levels0[i];
        ad->fnptr[j++]=ad->fnptr0[i];
        k+=ad->Perlman_levels0[i];
        }
    #endif
    for(k=j=0,i=0;i<ad->num_factors0;i++)if(!ad->notfactor[i]){
        ad->fnptr[j++]=ad->fnptr0[i];
        k+=ad->Perlman_levels0[i];
        }

    #if 1
    if(!(ad->lnptr=malloc(sizeof*ad->lnptr*k))) {
        printf("fidlError: read_driver2 Unable to malloc ad->lnptr\n");fflush(stdout);return NULL;
        }
    for(l=k=i=0;i<ad->num_factors0;k+=ad->Perlman_levels0[i++])if(!ad->notfactor[i]){
        for(j=0;j<ad->Perlman_levels0[i];j++)ad->lnptr[l++]=ad->lnptr0[k+j];
        }
    #endif


    if(!(ad->notfactori=malloc(sizeof*ad->notfactori*ad->nnotfactor))){
        printf("fidlError: read_driver2 Unable to malloc ad->notfactori\n");
        return NULL;
        }
    if(!(factori=malloc(sizeof*factori*ad->num_factors0))){
        printf("fidlError: read_driver2 Unable to malloc factori\n");
        return NULL;
        }
    for(k=j=i=0;i<ad->num_factors0;i++){
        if(!ad->notfactor[i])factori[j++]=i;else ad->notfactori[k++]=i;
        }

    #if 0
    printf("ad->num_factors=%d ad->num_factors0=%d\n",ad->num_factors,ad->num_factors0);
    printf("factori=");for(i=0;i<ad->num_factors0;i++)printf("%d ",factori[i]);printf("\n");
    #endif

    for(rewind(fp),fgets(line,sizeof(line),fp),nlines=len2=0;fgets(line,sizeof(line),fp);){
        if((nstrings=count_strings_new3(line,write_back,' ',' ',&last_char,&first_char,0,' '))>1){
            ++nlines;
            ad->num_datafiles+=nstrings-ad->num_factors0-not_num_factors+1;
            for(strptr=write_back,j=0;j<ad->num_factors0+1;j++,strptr=NULL)write_back_ptr=strtok_r(strptr," ",&saveptr);

            //if(ad->chunks==1&&!lc_data)len2+=strlen(write_back_ptr)+1;
            //START160309
            if(!lcchunks&&!lc_data)len2+=strlen(write_back_ptr)+1;

            }

        //else if(nstrings==1&&ad->chunks>1){
        //START160309 
        else if(nstrings==1&&lcchunks){

            len2+=strlen(write_back)+1;
            }
        }
    if(!nlines) {
        printf("%s need to list the factor levels, subject, and data file.\n",driver_file);
        return NULL;
        }
    ad->num_datafiles*=ad->chunks;
    if(lc_data) {
        ad->datafiles=(char**)NULL;
        if(!(ad->driver_data=malloc(sizeof*ad->driver_data*nlines))){
            printf("fidlError: read_driver2 Unable to malloc ad->driver_data\n");
            return NULL;
            }
        }
    else {
        if(!(ad->datafiles=malloc(sizeof*ad->datafiles*ad->num_datafiles))){
            printf("fidlError: read_driver2 Unable to malloc ad->datafiles\n");
            return NULL;
            }
        if(!(ad->datafilesstack=malloc(sizeof*ad->datafilesstack*len2))){
            printf("fidlError: read_driver2 Unable to malloc ad->datafilesstack\n");
            return NULL;
            }
        }
    /* Check for more than a single measurement per cell.  This is multiple lines with the same factor and subject strings. */
    for(ad->num_cells=1,i=0;i<ad->num_factors;i++)ad->num_cells*=ad->Perlman_levels[i];
    if(lclevelall){if(!(ad->levelall=d2int(nlines,ad->num_factors)))return NULL;}
    if(!(ad->address=malloc(sizeof*ad->address*nlines))){
        printf("fidlError: read_driver2 Unable to malloc ad->address\n");
        return NULL;
        }
    if(!(ad->replications=malloc(sizeof*ad->replications*ad->num_cells))){
        printf("fidlError: read_driver2 Unable to malloc ad->replications\n");
        return NULL;
        }


    for(i=0;i<ad->num_cells;i++)ad->replications[i]=0;
    for(rewind(fp),fgets(line,sizeof(line),fp),strptr=ad->datafilesstack,k=i=0;fgets(line,sizeof(line),fp);) {
        if((nfiles=(nstrings=count_strings_new3(line,write_back,' ',' ',&last_char,&first_char,0,' '))-ad->num_factors0)>0) {
            strptr2=get_level(write_back,ad);

            //ad->address[i]=offset2(ad->level,ad->num_factors0,ad->Perlman_levels,ad->notfactor);
            //START160409
            ad->address[i]=offset2(ad->level,ad->num_factors0,ad->Perlman_levels0,ad->notfactor);

//address_check=offset2(&ad->level[1],m,&ad->Perlman_levels0[1],&ad->notfactor[1]);

            ad->replications[ad->address[i]]+=nfiles;
            if(lclevelall)for(j=0;j<ad->num_factors0;j++)if(!ad->notfactor[j])ad->levelall[i][j]=ad->level[j];

            #if 0
            if(ad->chunks==1){
                if(!lc_data){
                    write_back_ptr=strtok_r(strptr2," ",&saveptr);
                    strcpy(strptr,write_back_ptr);
                    ad->datafiles[i] = strptr;
                    strptr+=strlen(write_back_ptr)+1;
                    }
                else {
                    ad->driver_data[i]=strtof(write_back_ptr,NULL);
                    }
                }
            #endif
            //START160309
            if(lc_data){
                ad->driver_data[i]=strtof(write_back_ptr,NULL);
                }
            else if(!lcchunks){
                write_back_ptr=strtok_r(strptr2," ",&saveptr);
                strcpy(strptr,write_back_ptr);
                ad->datafiles[i] = strptr;
                strptr+=strlen(write_back_ptr)+1;
                }


            i++;
            }

        //else if(nstrings==1 && ad->chunks>1){
        //START160309
        else if(nstrings==1&&lcchunks){

            strcpy(strptr,write_back);
            ad->datafiles[k++]=strptr;
            strptr+=strlen(write_back)+1;
            }
        }


    //START160127
    #if 0
    //PRINT
    #ifndef MONTE_CARLO
        printf("ad->chunks=%d nlines=%d\n",ad->chunks,nlines);
        printf("ad->num_factors=%d ad->num_sources=%d ad->aa=%d ad->aa1=%d ad->num_datafiles=%d ad->num_factors0=%d\n",
            ad->num_factors,ad->num_sources,ad->aa,ad->aa1,ad->num_datafiles,ad->num_factors0);
        printf("ad->Perlman_levels=");for(i=0;i<ad->num_factors;i++)printf("%d ",ad->Perlman_levels[i]);printf("\n");
        printf("ad->notfactor=");for(i=0;i<ad->num_factors0;i++)printf("%d ",ad->notfactor[i]);printf("\n");
        printf("ad->nnotfactor=%d ",ad->nnotfactor); 
        printf("ad->notfactori=");for(i=0;i<ad->nnotfactor;i++)printf("%d ",ad->notfactor[i]);printf("\n");
    #endif
    #endif


    //printf("ad->replications=");for(i=0;i<ad->num_cells;i++)printf("%d ",ad->replications[i]);printf("\n");


    #if 0
    /*KEEP*/
    ad->num_factors_n = ad->num_factors + 1;
    if(!(level_lines=malloc(sizeof*level_lines*ad->num_factors_n*nlines))) {
        printf("Error: Unable to malloc level_lines\n");
        return NULL;
        }
    for(ii=0;ii<ad->num_factors_n*nlines;ii++) level_lines[ii]=0;
    for(rewind(fp),fgets(line,sizeof(line),fp),k=i=0;fgets(line,sizeof (line),fp);) {
        if((nfiles=count_strings_new3(line,write_back,' ',' ',&last_char,&first_char,0,' ')-ad->num_factors) > 0) {
            write_back_ptr = write_back;
            for(j=0;j<ad->num_factors;j++) {
                write_back_ptr = (char *)grab_string(write_back_ptr,line);
                while(strcmp(ad->level_names[j][level_lines[k+j]],line)) level_lines[k+j]++;
                }
            ad->address[i] = offset(&level_lines[k],ad->num_factors,ad->Perlman_levels);
            printf("level_lines="); for(j=0;j<ad->num_factors;j++) printf("%d ",level_lines[k+j]);
            printf(" ad->address[%d]=%d\n",i,ad->address[i]);
            level_lines[k+ad->num_factors] = ad->replications[ad->address[i]];
            ad->replications[ad->address[i]] += nfiles;
            i++;
            }
        }
    free(level_lines);
    #endif


    if(!(ad->data=malloc(sizeof*ad->data*ad->num_cells))){
        printf("fidlError: read_driver2 Unable to malloc ad->data\n");fflush(stdout);return NULL;
        }
    if(!(ad->bracket=malloc(sizeof*ad->bracket*ad->num_sources))){
        printf("fidlError: read_driver2 Unable to malloc ad->bracket\n");fflush(stdout);return NULL;
        }
    if(!(ad->sumcount=malloc(sizeof*ad->sumcount*ad->num_factors))){
        printf("fidlError: read_driver2 Unable to malloc ad->sumcount\n");fflush(stdout);return NULL;
        }
    if(!(ad->fstat=malloc(sizeof*ad->fstat*ad->num_sources))){ /*Just to be safe in case we want to look at the mean.*/
        printf("fidlError: read_driver2 Unable to malloc ad->fstat\n");fflush(stdout);return NULL;
        }
    if(!(ad->chat=malloc(sizeof*ad->chat*ad->num_sources))){ /*Just to be safe in case we want to look at the mean.*/
        printf("fidlError: read_driver2 Unable to malloc ad->chat\n");fflush(stdout);return NULL;
        }
    if(!(ad->htildaprimeprime=malloc(sizeof*ad->htildaprimeprime*ad->num_sources))){
        /*Just to be safe in case we want to look at the mean.*/
        printf("fidlError: read_driver2 Unable to malloc ad->htildaprimeprime\n");fflush(stdout);return NULL;
        }
    if(!(ad->htilda=malloc(sizeof*ad->htilda*ad->num_sources))){ /*Just to be safe in case we want to look at the mean.*/
        printf("fidlError: read_driver2 Unable to malloc ad->htilda\n");fflush(stdout);return NULL;
        }
    if(!(ad->fstatfilesl=malloc(sizeof*ad->fstatfilesl*ad->num_sources))) {
        printf("fidlError: read_driver2 Unable to malloc fstatfilesl\n");fflush(stdout);return NULL;
        }


    for(ad->max_length=kk=ad->fstatfilesl[0]=5,i=1;i<ad->num_sources;i++){ /*i=0 source is mean*/
        write_back[0] = 0;
        for(j=0;j<ad->num_factors;j++) {
            if(member(j,i)) {

                //strcat(write_back,ad->fnptr[factori[j]]);
                //START160408
                strcat(write_back,ad->fnptr[j]);

                strcat(write_back,&separator);
                }
            }
        *strrchr(write_back,separator)=0;
        kk += ad->fstatfilesl[i] = strlen(write_back)+1;
        if(ad->fstatfilesl[i]>ad->max_length) ad->max_length=ad->fstatfilesl[i];
        }


    if(!(ad->fstatfilesptr=d2charvar(ad->num_sources,ad->fstatfilesl)))return NULL;
    for(strcpy(ad->fstatfilesptr[0],"mean"),i=1;i<ad->num_sources;i++) { /*i=0 source is mean*/
        write_back[0]=0;
        for(j=0;j<ad->num_factors;j++) {
            if(member(j,i)) {

                //strcat(write_back,ad->fnptr[factori[j]]);
                //START160408
                strcat(write_back,ad->fnptr[j]);

                strcat(write_back,&separator);
                }
            }
        *strrchr(write_back,separator)=0;
        strcpy(ad->fstatfilesptr[i],write_back);
        }

    #if 0
    printf("ad->num_factors=%d ad->num_factors0=%d ad->num_sources=%d ad->aa=%d ad->nnotfactor=%d\n",ad->num_factors,ad->num_factors0,
        ad->num_sources,ad->aa,ad->nnotfactor);
    #endif


    #if 0
    if(!(ad->dfeffect=malloc(sizeof*ad->dfeffect*ad->aa))) {
        printf("fidlError: read_driver2 Unable to malloc ad->dfeffect\n");fflush(stdout);return NULL;
        }
    for(k=0,i=0;i<ad->num_sources;i+=2,k++){ /*i=0 source is mean*/
        ad->dfeffect[k]=1;
        for(j=1;j<ad->num_factors;j++) if(member(j,i)) ad->dfeffect[k]*=ad->Perlman_levels[j]-1;
        if(ad->dfeffect[k]<=0) {
            printf("fidlError: read_driver2 Invalid dfeffect: %d  Do you have a factor at only 1 level?\n",ad->dfeffect[k]);
            fflush(stdout);return NULL;
            }
        }
    #endif
    //START170207
    if(!dontcheck){
        if(!(ad->dfeffect=malloc(sizeof*ad->dfeffect*ad->aa))) {
            printf("fidlError: read_driver2 Unable to malloc ad->dfeffect\n");fflush(stdout);return NULL;
            }
        for(k=0,i=0;i<ad->num_sources;i+=2,k++){ /*i=0 source is mean*/
            ad->dfeffect[k]=1;
            for(j=1;j<ad->num_factors;j++) if(member(j,i)) ad->dfeffect[k]*=ad->Perlman_levels[j]-1;
            if(ad->dfeffect[k]<=0) {
                printf("fidlError: read_driver2 Invalid dfeffect: %d  Do you have a factor at only 1 level?\n",ad->dfeffect[k]);
                fflush(stdout);return NULL;
                }
            }
        }



    if(!(flagarr=malloc(sizeof*flagarr*ad->num_sources))) {
        printf("fidlError: read_driver2 Unable to malloc flagarr\n");fflush(stdout);return NULL;
        }
    if(!(ad->nterms=malloc(sizeof*ad->nterms*ad->num_sources))) {
        printf("fidlError: read_driver2 Unable to malloc ad->nterms\n");fflush(stdout);return NULL;
        } 
    for(ad->between=i=0;i<ad->num_sources;i++){
        ad->nterms[i]=setsize(i,ad->num_factors);
        if(ad->nterms[i]==2&&member(RANDOM,i)){
            for(j=0;j<ad->num_factors;j++)ad->level[j]=0;
            do{
                k=0;
                do{
                    addr=offset3(ad->level,ad->num_factors,ad->Perlman_levels);
                    if(ad->replications[addr])k++;
                    }while(nextlevel(ad->level,i,0,ad->num_factors,ad->Perlman_levels));
                if(!k){
                    ad->between=join(ad->between,i-1);
                    flagarr[i]=1;
                    }
                }while(nextlevel(ad->level,i,1,ad->num_factors,ad->Perlman_levels));
            }

        }
    #if 0
    for(ad->within=i=0;i<ad->num_sources;i++){
        if(ad->nterms[i]==2&&member(RANDOM,i)){
            for(j=0;j<ad->num_factors;j++)ad->level[j]=0;
            do{
                do{
                    addr=offset3(ad->level,ad->num_factors,ad->Perlman_levels);
                    }while(nextlevel(ad->level,i,0,ad->num_factors,ad->Perlman_levels));
                if(!flagarr[i]){
                    ad->within=join(ad->within,i-1);
                    }
                }while(nextlevel(ad->level,i,1,ad->num_factors,ad->Perlman_levels));
            }
        }
    #endif
    for(ad->within=i=0;i<ad->num_sources;i++)if(ad->nterms[i]==2&&member(RANDOM,i)&&!flagarr[i])ad->within=join(ad->within,i-1);



    free(flagarr);
    if(!(ad->within_effects=malloc(sizeof*ad->within_effects*ad->num_sources))) {
        printf("fidlError: read_driver2 Unable to malloc ad->within_effects\n");fflush(stdout);return NULL;
        }
    for(i=0;i<ad->num_sources;i++)ad->within_effects[i]=0; 
    for(ad->within_neffects=i=0;i<ad->num_sources;i+=2) {
        if((ad->within&i)&&!(ad->between&i)) {
            ad->within_neffects++;
            ad->within_effects[i]=1; 
            }
        }
    if(!(ad->within_between=malloc(sizeof*ad->within_between*ad->num_factors))) {
        printf("fidlError: read_driver2 Unable to malloc ad->within_between\n");fflush(stdout);return NULL;
        }
    if(!(ad->betweeni=malloc(sizeof*ad->betweeni*ad->num_factors))) {
        printf("fidlError: read_driver2 Unable to malloc ad->betweeni\n");fflush(stdout);return NULL;
        }
    if(!(ad->betweenf=malloc(sizeof*ad->betweenf*ad->num_factors))) {
        printf("fidlError: read_driver2 Unable to malloc ad->betweenf\n");fflush(stdout);return NULL;
        }
    for(i=0;i<ad->num_factors;i++) ad->betweenf[i]=0;
    if(!(ad->withini=malloc(sizeof*ad->withini*ad->num_factors))) {
        printf("fidlError: read_driver2 Unable to malloc ad->withini\n");fflush(stdout);return NULL;
        }
    for(ad->betprod=ad->withprod=1,ad->nwithin=ad->nbetween=i=0;i<ad->num_factors;i++) {
        if((ad->within_between[i]=member(i,ad->between))){

            //ad->betprod*=ad->Perlman_levels[factori[i]];
            //START160408
            ad->betprod*=ad->Perlman_levels[i];

            ad->betweeni[ad->nbetween++]=i;
            ad->betweenf[i]=1;
            }
        else if(i){

            //ad->withprod*=ad->Perlman_levels[factori[i]];
            //START160408
            ad->withprod*=ad->Perlman_levels[i];

            ad->withini[ad->nwithin++]=i;
            }
        }


    if(!(ad->dferror_within=malloc(sizeof*ad->dferror_within*ad->aa))) {
        printf("fidlError: read_driver2 Unable to malloc ad->dferror_within\n");fflush(stdout);return NULL;
        }
    if(!(ad->dferror=malloc(sizeof*ad->dferror*ad->aa))) {
        printf("fidlError: read_driver2 Unable to malloc ad->dferror\n");fflush(stdout);return NULL;
        }

    #if 0
    for(k=i=0;i<ad->num_sources;i+=2,k++) { /*i=0 source is mean*/
        ad->dferror[k] = ad->Perlman_levels[RANDOM] - ad->betprod;
        ad->dferror_within[k] = 1;
        for(j=1;j<ad->num_factors;j++)
            if(member(j,i) && !member(j,ad->between)) ad->dferror_within[k] *= ad->Perlman_levels[j] - 1;
        ad->dferror[k] *= ad->dferror_within[k];
        if(ad->dferror_within[k] <= 0) {
            printf("fidlError: read_driver2 Invalid ad->dferror_within[%d] = %d\n",k,ad->dferror_within[k]);fflush(stdout);return NULL;
            }
        }
    #endif
    //START170207
    if(!dontcheck){
        for(k=i=0;i<ad->num_sources;i+=2,k++) { /*i=0 source is mean*/
            ad->dferror[k] = ad->Perlman_levels[RANDOM] - ad->betprod;
            ad->dferror_within[k] = 1;
            for(j=1;j<ad->num_factors;j++)
                if(member(j,i) && !member(j,ad->between)) ad->dferror_within[k] *= ad->Perlman_levels[j] - 1;
            ad->dferror[k] *= ad->dferror_within[k];
            if(ad->dferror_within[k] <= 0) {
                printf("fidlError: read_driver2 Invalid ad->dferror_within[%d] = %d\n",k,ad->dferror_within[k]);
                fflush(stdout);return NULL;
                }
            }
        }

    if(!(ad->dfeffectSPH=malloc(sizeof*ad->dfeffectSPH*ad->aa*nreg))) {
        printf("Error: Unable to malloc ad->dfeffectSPH\n");
        return NULL;
        }
    if(!(ad->dferrorSPH=malloc(sizeof*ad->dferrorSPH*ad->aa*nreg))) {
        printf("Error: Unable to malloc ad->dferrorSPH\n");
        return NULL;
        }
    if(!(ad->sseffect=malloc(sizeof*ad->sseffect*ad->aa))) {
        printf("Error: Unable to malloc ad->sseffect\n");
        return NULL;
        }
    if(!(ad->mseffect=malloc(sizeof*ad->mseffect*ad->aa))) {
        printf("Error: Unable to malloc ad->mseffect\n");
        return NULL;
        }
    if(!(ad->sserror=malloc(sizeof*ad->sserror*ad->aa))) {
        printf("Error: Unable to malloc ad->sserror\n");
        return NULL;
        }
    if(!(ad->mserror=malloc(sizeof*ad->mserror*ad->aa))) {
        printf("Error: Unable to malloc ad->mserror\n");
        return NULL;
        }

    //printf("here101 ad->num_datafiles=%d\n",ad->num_datafiles);fflush(stdout);
    //for(i=0;i<ad->num_datafiles;i++){printf("%s\n",ad->datafiles[i]);fflush(stdout);}

    if(!(ad->NA=malloc(sizeof*ad->NA*ad->num_datafiles))){
        printf("fidlError: read_driver2 Unable to malloc ad->NA\n");
        return NULL;
        }
    for(i=0;i<ad->num_datafiles;i++)ad->NA[i]=!strcmp(ad->datafiles[i],"NA")?1:0;
    free(factori);
    return ad;
}

/* return unique index for each combination factor levels */
int offset(int *level,int num_factors,int *num_levels)
{
    int i,aindex,coeff=1;

    aindex = level[num_factors-1];
    for(i=num_factors-1; --i>=0;) {
        coeff *= num_levels[i+1];
        aindex += coeff * level[i];
        }
    return aindex;
}

/*START150908*/
/* return unique index for each combination factor levels */
int offset2(int *level,int num_factors,int *num_levels,int *notfactor){
    int i,aindex=0,coeff=1;
    for(i=num_factors;--i>=0;){
        if(!notfactor[i]){
            aindex += coeff*level[i];
            coeff *= num_levels[i];
            }
        }
    return aindex;
    }
//START160407
int offset3(int *level,int num_factors,int *num_levels){
    int i,aindex=0,coeff=1;
    for(i=num_factors;--i>=0;){
        aindex += coeff*level[i];
        coeff *= num_levels[i];
        }
    return aindex;
    }

int setsize(int set,int maxsize)
{
    int         size = 0;
    unsigned    bit;

    /*for(bit=0;bit<maxsize;bit++) {*/
    /*START150526*/
    for(bit=0;bit<(unsigned)maxsize;bit++) {

	/*printf("bit=%u 1<<bit=%u 1<<bit & set=%u %d set=%d member=%d\n",
                bit,1<<bit,(1<<bit)&set,(1<<bit)&set,set,member (bit, set));*/
        if(member (bit, set)) size++;
        }
    return size;
}


/* simulate a counting system based on ad->Perlman_levels[factors] 
   returns whether there are more levels
   source:     bit array of factors to (not) increment
   sourceflag: incr source factor if == 1, else non-source */
int nextlevel(int *level,int source,int sourceflag,int num_factors,int *num_levels) 
{
    int i;

    for(i=num_factors;--i>= 0;) {
        if(sourceflag == member(i, source)){
	    //printf("nextlevel: level[%d]=%d num_levels[%d]=%d\n",i,level[i],i,num_levels[i]);
            if(++level[i] < num_levels[i]) 
                return 1;
            else 
                level[i] = 0; /* go to next `decimal' place */
            }
        }
    return 0;
}

/*START150909*/
/* simulate a counting system based on ad->Perlman_levels[factors]
   returns whether there are more levels
   source:     bit array of factors to (not) increment
   sourceflag: incr source factor if == 1, else non-source */
int nextlevel2(int *level,int source,int sourceflag,int num_factors,int *num_levels,int *factori)
{
    int i;
    for(i=num_factors;--i>= 0;){
        if(sourceflag==member(i,source)){
            if(++level[i] < num_levels[factori[i]])
                return 1;
            else
                level[i] = 0; /* go to next `decimal' place */
            }
        }
    return 0;
}


#if 0
/*START150908*/
char* get_level(char *write_back,AnovaDesign *ad){
    char *strptr,*write_back_ptr=NULL,*saveptr;
    int i,j;

    for(strptr=write_back,j=i=0;i<ad->num_factors0;j+=ad->Perlman_levels[i++],strptr=NULL){
    //for(strptr=write_back,j=i=0;i<ad->num_factors0;j+=ad->Perlman_levels[i++],strptr++){

        /*START150910*/
        write_back_ptr=strtok_r(strptr," ",&saveptr);

        if(!ad->notfactor[i]){

            /*START150910*/
            #if 0
            write_back_ptr=strtok_r(strptr," ",&saveptr);
            //write_back_ptr=strtok_r(strptr," ",&strptr);
            #endif

            ad->level[i]=0;
            while(strcmp(ad->lnptr[j+ad->level[i]],write_back_ptr))ad->level[i]++;
            }
        }

    //printf("get_level strptr=%s\n",strptr);
    //printf("get_level write_back_ptr=%s\n",write_back_ptr);
    //printf("get_level write_back=%s\n",write_back);
    //printf("get_level saveptr=%s\n",saveptr);

    //return write_back_ptr;
    return saveptr;
    //return strptr;
    }
#endif
//START160407
char* get_level(char *write_back,AnovaDesign *ad){
    char *strptr,*write_back_ptr=NULL,*saveptr=NULL;
    int i,j;

    //for(strptr=write_back,j=i=0;i<ad->num_factors0;j+=ad->Perlman_levels[i++],strptr=NULL){
    //START160408
    for(strptr=write_back,j=i=0;i<ad->num_factors0;j+=ad->Perlman_levels0[i++],strptr=NULL){

        write_back_ptr=strtok_r(strptr," ",&saveptr);
        if(!ad->notfactor[i]){
            ad->level[i]=0;

            //while(strcmp(ad->lnptr[j+ad->level[i]],write_back_ptr))ad->level[i]++;
            //START160408
            while(strcmp(ad->lnptr0[j+ad->level[i]],write_back_ptr))ad->level[i]++;

            }
        }
    return saveptr;
    }

//START160408
char* get_level2(char *write_back,AnovaDesign *ad,int num_factors){
    char *strptr,*write_back_ptr=NULL,*saveptr=NULL;
    int i,j;
    for(strptr=write_back,j=i=0;i<num_factors;j+=ad->Perlman_levels0[i++],strptr=NULL){
        write_back_ptr=strtok_r(strptr," ",&saveptr);
        if(!ad->notfactor[i]){
            ad->level[i]=0;
            while(strcmp(ad->lnptr0[j+ad->level[i]],write_back_ptr))ad->level[i]++;
            }
        }
    return saveptr;
    }





