/* Copyright 12/07/17 Washington University.  All Rights Reserved.
   get_limits.h  $Revision: 1.1 $ */

#ifndef __GET_LIMITS_H__
    #define __GET_LIMITS_H__

    typedef struct {
        long name_max,path_max;
        } Limits;
    
    #ifdef __cplusplus
        extern "C" {
    #endif
    
    Limits get_limits();
    
    #ifdef __cplusplus
        }//extern
    #endif

#endif
