/* Copyright 8/30/05 Washington University.  All Rights Reserved.
   strutil.c  $Revision: 1.18 $*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//#include "fidl.h"
//START170802
#include "strutil.h"

/*ROUTINES NEEDED FOR SLIB THAT ARE NOT DIRECTLY ACCESSED BY CALL_EXTERNAL*/
char *grab_string_new(char *line,char *returnstr,int *returnstrlen)
{
    while(isspace(*line)) line++;
    for(*returnstrlen=0;!isspace(*line) && *line != 0;(*returnstrlen)++) *returnstr++ = *line++;
    *returnstr = 0;
    return line;
}
float *strings_to_float(char *line,float *ptr,int how_many)
{
    size_t i;
    char *strptr;
    for(i=0;i<(size_t)how_many;i++) {
        *ptr++=(float)strtod(line,&strptr);
        line = ++strptr;
        }
    return ptr;
}
double *strings_to_double(char *line,double *ptr,int how_many)
{
    size_t i;
    char *strptr;
    for(i=0;i<(size_t)how_many;i++) {
        *ptr++=strtod(line,&strptr);
        line = ++strptr;
        }
    return ptr;
}
int count_strings_new3(char *line,char *write_back,char wildcard1,char wildcard2,char *last_char,char *first_char,
    int no_first_char_to_write_back,char terminator)
{
    int num_strings = 0,length,count_length=0;
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
            *write_back++ = terminator;
            ++num_strings;
            }
        if(count_length >= length) break; /*This was added for argv.*/
        }
    *(--write_back) = 0;
    return num_strings;
}

/*START150610*/
char *get_line_wnull(char *line,char *returnstr,char *searchstr,int *returnstrlen){
    char *strptr;
    if((line=strstr(line,searchstr))) {
        for(strptr=line;isspace(*(--strptr)););*(++strptr)=0;
        if((strptr=strstr(line,"\n"))) *strptr = 0;
        line += strlen(searchstr);

        /*while(isspace(*line)) line++;*/
        /*START150610*/
        for(;isspace(*line);)line++;

        for(*returnstrlen=0;*line != 0;(*returnstrlen)++) *returnstr++ = *line++;
        *returnstr = 0;
        }
    return line;
    }
