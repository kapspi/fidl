/* Copyright 11/24/14 Washington University.  All Rights Reserved.
   read_data.c  $Revision: 1.7 $*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fidl.h"
#include "read_data.h"
#include "subs_util.h"
#include "count_tab.h"
#include "d2double.h"
Data *read_data(char *datafile,int label,int nskiplines,int nreadcol,int counttab)
{
    char line[10000],write_back[10000],*strptr,*write_back_ptr,last_char,first_char;
    int i,j,m,nstrings,firstlinelabels;
    FILE *fp;
    Data *data;
    if(!(data=malloc(sizeof*data))) {
        printf("Error: Unable to malloc data in read_data\n");
        return NULL;
        }
    data->subjects=(char**)NULL;
    data->colptr=(char**)NULL;
    if(!(fp=fopen_sub(datafile,"r"))) return NULL;
    for(i=0;i<nskiplines;i++) fgets(line,sizeof(line),fp);
    do{fgets(line,sizeof(line),fp);}while(line[0]=='#');
    for(firstlinelabels=0,strptr=line;*strptr;strptr++) if(isalpha(*strptr)) {firstlinelabels=1;break;}
    for(rewind(fp),i=0;i<nskiplines;i++) fgets(line,sizeof(line),fp);
    if(firstlinelabels) {
        do{fgets(line,sizeof(line),fp);}while(line[0]=='#');
        if(!counttab) {
            if((data->ncol = count_strings_new3(line,write_back,' ',' ',&last_char,&first_char,0,' ')) < 1) {
                printf("List the column headers on the first line of %s.\n",datafile);
                return NULL;
                }
            }
        else {

            /*if((data->ncol = count_tab(line,write_back)) < 1) {*/
            /*START141215*/
            if((data->ncol = count_tab(line,write_back,' ')) < 1) {

                printf("List the column headers on the first line of %s.\n",datafile);
                return NULL;
                }
            }
        if(!(data->colptr=malloc(sizeof*data->colptr*data->ncol))) {
            printf("Error: Unable to malloc data->colptr\n");
            return NULL;
            }
        for(write_back_ptr=write_back,i=j=0;j<data->ncol;j++) {
            write_back_ptr = grab_string_new(write_back_ptr,line,&m);
            i += m+1;
            }
        if(!(data->col=malloc(sizeof*data->col*i))) {
            printf("Error: Unable to malloc data->col\n");
            return NULL;
            }
        for(strptr=data->col,write_back_ptr=write_back,j=0;j<data->ncol;j++) {
            write_back_ptr = grab_string_new(write_back_ptr,line,&m);
            strcpy(strptr,line);
            data->colptr[j] = strptr;
            strptr+=m+1;
            }
        }
    if(!counttab) 
        for(data->nsubjects=data->npoints=0,i=1;fgets(line,sizeof(line),fp);i++) {
            if(line[0]=='#') continue;
            if((nstrings=count_strings_new3(line,write_back,' ',' ',&last_char,&first_char,0,' '))) {
                if(nstrings>data->npoints) data->npoints = nstrings;
                data->nsubjects++;
                }
            }
    else
        for(data->nsubjects=data->npoints=0,i=1;fgets(line,sizeof(line),fp);i++) {
            if(line[0]=='#') continue;

            /*if((nstrings=count_tab(line,write_back))) {*/
            /*START141215*/
            if((nstrings=count_tab(line,write_back,' '))) {

                if(nstrings>data->npoints) data->npoints = nstrings;
                data->nsubjects++;
                }
            }
    if(nreadcol) {
        if(nreadcol<data->npoints) data->npoints=nreadcol;
        }
    data->npoints-=label;
    if(!data->nsubjects) {
        printf("Error: %s appears to be empty. Abort!\n",datafile);
        return NULL;
        }
    if(!data->npoints) {
        printf("Error: %s appears not to have any data points. Abort!\n",datafile);
        return NULL;
        }
    if(!(data->x=d2double(data->nsubjects,data->npoints))) exit(-1);
    if(!(data->npoints_per_line=malloc(sizeof*data->npoints_per_line*data->nsubjects))) {
        printf("Error: Unable to malloc data->npoints_per_line\n");
        return NULL;
        }
    for(rewind(fp),i=0;i<nskiplines;i++) fgets(line,sizeof(line),fp);
    if(firstlinelabels) do{fgets(line,sizeof(line),fp);}while(line[0]=='#'); 
    if(!counttab)
        for(data->total_npoints_per_line=j=i=0;fgets(line,sizeof(line),fp);) {
            if(line[0]=='#') continue;
            if((nstrings=count_strings_new3(line,write_back,' ',' ',&last_char,&first_char,0,' '))) {
                data->total_npoints_per_line+=data->npoints_per_line[i] = nstrings-label;
                if(label) {
                    strptr = grab_string_new(write_back,line,&m);
                    j += m+1;
                    }
                else {
                    strptr = write_back;
                    }
                strings_to_double(strptr,data->x[i++],data->npoints);
                }
            }
    else
        for(data->total_npoints_per_line=j=i=0;fgets(line,sizeof(line),fp);) {
            if(line[0]=='#') continue;

            /*if((nstrings=count_tab(line,write_back))) {*/
            /*START141215*/
            if((nstrings=count_tab(line,write_back,' '))) {

                data->total_npoints_per_line+=data->npoints_per_line[i] = nstrings-label;
                if(label) {
                    strptr = grab_string_new(write_back,line,&m);
                    j += m+1;
                    }
                else {
                    strptr = write_back;
                    }
                strings_to_double(strptr,data->x[i++],data->npoints);
                }
            }
    if(label) {
        if(!(data->subjectsstack=malloc(sizeof*data->subjectsstack*j))) {
            printf("Error: Unable to malloc data->subjectsstack\n");
            return NULL;
            }
        if(!(data->subjects=malloc(sizeof*data->subjects*data->nsubjects))) {
            printf("Error: Unable to malloc data->subjects\n");
            return NULL;
            }
        for(rewind(fp),i=0;i<nskiplines;i++) fgets(line,sizeof(line),fp);
        if(firstlinelabels) do{fgets(line,sizeof(line),fp);}while(line[0]=='#'); 
        if(!counttab)
            for(strptr=data->subjectsstack,i=0;fgets(line,sizeof(line),fp);) {
                if(line[0]=='#') continue;
                if((nstrings=count_strings_new3(line,write_back,' ',' ',&last_char,&first_char,0,' '))) {
                    grab_string_new(write_back,line,&m);
                    strcpy(strptr,line);
                    data->subjects[i++] = strptr;
                    strptr+=m+1;
                    }
                }
        else
            for(strptr=data->subjectsstack,i=0;fgets(line,sizeof(line),fp);) {
                if(line[0]=='#') continue;
                if((nstrings=count_strings_new3(line,write_back,' ',' ',&last_char,&first_char,0,' '))) {
                    grab_string_new(write_back,line,&m);
                    strcpy(strptr,line);
                    data->subjects[i++] = strptr;
                    strptr+=m+1;
                    }
                }

        }
    return data;
}
void free_data(Data *data)
{
    if(data->subjects) {
        free(data->subjects);
        free(data->subjectsstack);
        }
    if(data->colptr) {
        free(data->colptr);
        free(data->col);
        }
    free(data->npoints_per_line);
    free_d2double(data->x);
    free(data);
}
