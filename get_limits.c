/* Copyright 11/18/11 Washington University.  All Rights Reserved.
   get_limits.c  $Revision: 1.8 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#ifndef __sun__
    #include <stdint.h>
#endif

//#include "fidl.h"
//START171207
#include "get_limits.h"
#include "make_timestr2.h"

Limits get_limits(){
    int fildes;
    char fn[31],timestr[23];
    Limits limits;
    limits.name_max=0;limits.path_max=0;
    sprintf(fn,"fidl%s.txt",make_timestr2(timestr));
    if((fildes=creat(fn,S_IRUSR))<0)
        perror("creat() error");
    else {
        errno=0;
        if((limits.name_max=fpathconf(fildes,_PC_NAME_MAX))==-1) {
            if(errno==0)
                puts("There is no limit to NAME_MAX.");
            else {
                perror("fpathconf() error");
                limits.name_max = 255;
                }
            }
        else {
            /*printf("NAME_MAX is %ld\n",name_max);*/
            }
        errno=0;
        if((limits.path_max=fpathconf(fildes,_PC_PATH_MAX))==-1) {
            if(errno==0)
                puts("There is no limit to PATH_MAX.");
            else {
                perror("fpathconf() error");
                limits.path_max = 1024;
                }
            }
        else {
            /*printf("PATH_MAX is %ld\n",path_max);*/
            }
        close(fildes);
        unlink(fn);
        }
    return limits;
    }
int _get_limits(int argc,char **argv){
    #ifdef __sun__
        int *name_max=(int*)argv[0],*path_max=(int*)argv[1];
    #else
        intptr_t *name_max=(intptr_t*)argv[0],*path_max=(intptr_t*)argv[1];
    #endif
    Limits limits;
    limits=get_limits();
    *name_max = (int)limits.name_max;
    *path_max = (int)limits.path_max;
    return 1;
    }
