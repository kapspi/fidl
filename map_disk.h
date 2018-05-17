/* Copyright 7/20/17 Washington University.  All Rights Reserved.
   map_disk.h  $Revision: 1.1 $ */
#ifndef __MAP_DISK_H__
    #define __MAP_DISK_H__ 
    typedef struct {
        void  *vptr;
        double *dptr;float *ptr;
        size_t size;
        int   offset,length;
        } Memory_Map;

    #ifdef __cplusplus
        extern "C" {
    #endif

    Memory_Map *map_disk(char *filnam,int length,int offset,size_t size);
    int unmap_disk(Memory_Map *map);

    #ifdef __cplusplus
        }//extern
    #endif
#endif
