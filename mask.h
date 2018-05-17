/* Copyright 11/1/17 Washington University.  All Rights Reserved.
   mask.h  $Revision: 1.1 $ */
#ifndef __MASK_H__
    #define __MASK_H__
    #include "subs_glm.h"
    #include "header.h"
    class mask : header {
        int vol,lenbrain,*brnidx,*maskidx,nuns,*unsi;
        public:
            int read_mask(char *mask_file,int SunOS_Linux,LinearModel *glm_in);
            int get_mask(char *mask_file,int vol,int *indices,int SunOS_Linux,LinearModel *glm,int mslenvol);
            ~mask();

            int* get_brnidx(int& lenbrain);
            int* get_brnidx(int& lenbrain,int& vol);
            int* get_brnidx(int& lenbrain,int& xdim,int& ydim,int& zdim);
            int* get_brnidx(int& lenbrain,int& vol,int& xdim,int& ydim,int& zdim);

            int _read_mask(int argc,char **argv);
        };
#endif
