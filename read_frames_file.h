/* Copyright 4/22/15 Washington University.  All Rights Reserved.
   read_frames_file.h  $Revision: 1.2 $ */
typedef struct {
    int ntc,nframes,nlines,*frames_per_line,*num_frames_to_sum,*frames,*frames_cond,nfactornames,nfactors,**factors,classi,
        subjecti,runi,conditioni;
    char **factornames;
    } FS;

//START170619
#ifdef __cplusplus
    extern "C" {
#endif

FS *read_frames_file(char *filename,int lccond_and_frames,int starti,int readthismany);

//START170619
#ifdef __cplusplus
    }//extern
#endif

