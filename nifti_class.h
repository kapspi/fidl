/* Copyright 6/19/17 Washington University.  All Rights Reserved.
   nifti_class.h  $Revision: 1.1 $ */
#ifndef __NIFTI_CLASS_H__
    #define __NIFTI_CLASS_H__
    class niftiStack{
        int64_t xdim64,ydim64,zdim64,tdim64;
        float center[3],mmppix[3];
        int c_orient[3],cf_flip,lcflip;
        void *niftiPtr;
        public:
            int niftiStack0(char* file);
            ~niftiStack();

            //int niftiVol(int64_t volidx,float *temp_float);
            //START170620
            int niftiVol(int64_t volidx,float *temp_float,int flipit);

        };
#endif
