/* Copyright 8/13/15 Washington University.  All Rights Reserved.
   write1.h  $Revision: 1.6 $ */
typedef struct{
    char *cifti_xmldata,*file_hdr;
    int filetype,lenbrain,how_many,swapbytes,dontminmax;
    unsigned int xmlsize;
    int64_t *dims,tdim;
    float *temp_float,*center,*mmppix;
    double *temp_double;
    Interfile_header *ifh;
    }W1;

#ifdef __cplusplus
    extern "C" {
#endif

W1 *write1_init();
int write1(char *file,W1 *w1);
void* open2(char *file,W1 *w1);

#ifdef __cplusplus
    }//extern
#endif
