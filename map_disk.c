/* Copyright 12/31/99 Washington University.  All Rights Reserved.
   map_disk.c  $Revision: 12.92 $ */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "map_disk.h"



Memory_Map *map_disk(char *filnam,int length,int offset,size_t size) {
    int fd;
    Memory_Map *map;
    if(!(map=malloc(sizeof*map))) {
        printf("fidlError: Unable to malloc map in map_disk\n");
        return NULL;
        }
    map->offset = offset;
    map->length = length;
    map->size = size;
    map->vptr = MAP_FAILED;
    if((fd=open(filnam,O_RDONLY)) == -1) {
        printf("fidlError: Unable to open %s in map_disk\n",filnam);
        return NULL;
        }

    #if 0
    else if((map->vptr=mmap((caddr_t)0,(size_t)(length*size),(int)PROT_READ,(int)MAP_SHARED,(int)fd,
            (off_t)(offset*size))) == MAP_FAILED) {
    #endif
    /*START140729*/
    else if((map->vptr=mmap(0,(size_t)(length*size),(int)PROT_READ,(int)MAP_SHARED,(int)fd,
            (off_t)(offset*size))) == MAP_FAILED) {


        printf("fidlError: mapping %s\n",filnam);
        }
    if(fd!=-1)close(fd);
    if(map->vptr==MAP_FAILED){free(map);map=NULL;} 
    else if(size==sizeof(float)) map->ptr = (float*)map->vptr;
    else map->dptr = (double*)map->vptr;
    return map;
    }
int unmap_disk(Memory_Map *map) {
    int status=1;

    #if 0
    status += munmap((caddr_t)map->vptr,(size_t)(map->length*map->size));
    #endif
    /*START140729*/
    status += munmap(map->vptr,(size_t)(map->length*map->size));

    if(status) free(map); else printf("fidlError: unmap_disk. Abort!\n");
    return status;
    }
