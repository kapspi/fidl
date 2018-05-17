/* Copyright 8/02/17 Washington University.  All Rights Reserved.
   strutil.h  $Revision: 1.1 $ */
#ifndef __STRUTIL_H__
    #define __STRUTIL_H__

    #ifdef __cplusplus
        extern "C" {
    #endif

    char *grab_string_new(char *line,char *returnstr,int *returnstrlen);
    float *strings_to_float(char *line,float *ptr,int how_many);
    double *strings_to_double(char *line,double *ptr,int how_many);
    int count_strings_new3(char *line,char *write_back,char wildcard1,char wildcard2,char *last_char,char *first_char,
        int no_first_char_to_write_back,char terminator);
    char *get_line_wnull(char *line,char *returnstr,char *searchstr,int *returnstrlen);

    #ifdef __cplusplus
        }//extern
    #endif

#endif

