/* Copyright 4/23/15 Washington University.  All Rights Reserved.
   make_scratchdir.c  $Revision: 1.3 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fidl.h"
#include "make_scratchdir.h"
#include "make_timestr2.h"
char *make_scratchdir(int noPWD,char *label){
    char *scratchdir,cmd[1000],path[1000]="",*pwd,timestr[23]; 
    if(!noPWD) {
        if(!(pwd=getenv("PWD"))) {
            printf("fidlError: Unable to get environment variable PWD\n");
            return NULL;
            }
        sprintf(path,"%s/",pwd);
        }
    make_timestr2(timestr);
    if(!(scratchdir=(char*)malloc(sizeof*scratchdir*(strlen(path)+strlen(label)+strlen(timestr)+3)))) {
        printf("fidlError: Unable to malloc scratchdir\n");
        return NULL;
        }
    sprintf(scratchdir,"%s%s_%s/",noPWD?"":path,label,timestr);
    sprintf(cmd,"mkdir %s",scratchdir);
    if(!system(cmd) == -1) {
        printf("fidlError: Unable to %s\n",cmd);
        return NULL;
        }
    return scratchdir;
    }
