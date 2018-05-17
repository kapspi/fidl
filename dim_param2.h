/* Copyright 7/13/15 Washington University.  All Rights Reserved.
   dim_param2.h  $Revision: 1.8 $*/
#ifndef __DIM_PARAM2_H__
    #define __DIM_PARAM2_H__
    typedef struct{
        int *filetype,filetypeall,*swapbytes,*vol,volall,*tdim,tdimall,tdim_total,tdim_max,lenvol_max,*bigendian,bigendianall,
            *xdim,*ydim,*zdim,*orientation;
        float *dxdy,*dz;
        }Dim_Param2;

    //START170619
    #ifdef __cplusplus
        extern "C" {
    #endif

    //START170714
    //int get_filetype(char *file);

    Dim_Param2 *dim_param2(size_t nfiles,char **files,int SunOS_Linux);
    void free_dim_param2(Dim_Param2 *dp);

    //START170619
    #ifdef __cplusplus
        }//extern
    #endif

#endif
