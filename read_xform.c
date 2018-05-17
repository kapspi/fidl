/* Copyright 12/31/99 Washington University.  All Rights Reserved.
   read_xform.c  $Revision: 12.90 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//#include "fidl.h"
//START170802
#include "read_xform.h"
#include "constants.h"
#include "strutil.h"

#include "subs_util.h"
int _read_xform(int argc,char **argv){
    char *filename;
    float *t4;
    filename = (char *)argv[0];
    t4 = (float *)argv[1];
    if(!(read_xform(filename,t4))) return 0; 
    return 1;
    }
int read_xform(char *xform_file,float *t4){
    char string[MAXNAME],write_back[MAXNAME],last_char,first_char;
    int index;
    FILE *fp;
    if(!(fp = fopen_sub(xform_file,"r"))) return 0; 
    for(index=0;fgets(string,sizeof(string),fp);) {
        if(count_strings_new3(string,write_back,' ',' ',&last_char,&first_char,0,0) == 4) {
            if(isdigit(write_back[0]) || (write_back[0]=='-' && isdigit(write_back[1]))) {
                strings_to_float(write_back,t4+index,4);
                index += 4;
                }
            }
        }
    fclose(fp);
    return 1;
    }
