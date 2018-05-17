/* Copyright 4/23/07 Washington University.  All Rights Reserved.
   checkOS.c  $Revision: 1.7 $*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "subs_util.h"
#include "make_timestr2.h"
#include "checkOS.h"

/*START150615*/
#if 0
KEEP
int _checkOS(int argc,char **argv) {
    return checkOS();
    }
#endif

int checkOS(){
    char os[6],timestr[23],lizard[39],fish[47];
    int SunOS_Linux;
    FILE *fp;
    sprintf(lizard,"fidl_checkOS%s.dat",make_timestr2(timestr));
    sprintf(fish,"uname > %s",lizard);
    if(system(fish) == -1) {
        printf("fidlError: system call 1 checkOS. Abort!\n");
        return -1;
        }
    if(!(fp = fopen_sub(lizard,"r"))) return -1;
    fscanf(fp,"%s",os);
    fclose(fp);
    sprintf(fish,"rm %s",lizard);
    if(system(fish) == -1) {
        printf("fidlError: system call 2 checkOS. Abort!\n");
        return -1;
        }
    if(strstr(os,"SunOS")) {
        SunOS_Linux = 0;
        }
    else if(strstr(os,"Linux")) {
        SunOS_Linux = 1;
        }
    else {
        printf("fidlError: os = %s    Unknown operating system. Abort!\n",os);
        printf("       This error will appear erroneously if you are out of disk space.\n");
        SunOS_Linux = -1;
        }
    return SunOS_Linux;
    }
