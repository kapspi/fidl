/* Copyright 7/14/15 Washington University.  All Rights Reserved.
   files_struct.h  $Revision: 1.3 $*/
#ifndef __FILES_STRUCT_H__
    #define __FILES_STRUCT_H__ 
    typedef struct {
        char **files,**identify;
        size_t nfiles,nstrings,max_length,*strlen_files,*strlen_identify;
        } Files_Struct;

    #ifdef __cplusplus
        extern "C" {
    #endif

    Files_Struct *read_files(int nfiles,char **argv);
    Files_Struct *get_files(int num_region_files,char **argv);
    Files_Struct *init_FS(size_t nfiles);
    Files_Struct *read_conc(int nfile,char **file);
    void free_files_struct(Files_Struct *fi);

    #ifdef __cplusplus
        }//extern
    #endif
#endif
