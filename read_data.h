/* Copyright 3/23/15 Washington University.  All Rights Reserved.
   read_data.h  $Revision: 1.2 $ */

#ifndef __READ_DATA_H__
    #define __READ_DATA_H__

    typedef struct {
        int nsubjects,npoints,*npoints_per_line,total_npoints_per_line,ncol;
        char *subjectsstack,**subjects,*col,**colptr;
        double **x;
        } Data;

    #ifdef __cplusplus
        extern "C" {
    #endif

    Data *read_data(char *datafile,int label,int nskiplines,int nreadcol,int counttab);
    void free_data(Data *data);

    #ifdef __cplusplus
        }//extern
    #endif
#endif
