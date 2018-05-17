/* Copyright 3/6/14 Washington University.  All Rights Reserved.
   make_timestr2.c  $Revision: 1.6 $*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "make_timestr2.h"
char *make_timestr2(char *timestr){ /*min 23 elements, eg timestr[23]*/
    char tstr[16];
    struct timeval tv;struct tm *tm;
    gettimeofday(&tv,NULL);
    tm=localtime(&tv.tv_sec);
    strftime(tstr,16,"%y%m%d:%H:%M:%S",tm);
    sprintf(timestr,"%s:%06ld",tstr,tv.tv_usec);
    return timestr;
    }
