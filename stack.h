/* Copyright 2/21/18 Washington University.  All Rights Reserved.
   stack.h  $Revision: 1.1 $ */
#ifndef __STACK_H__
    #define __STACK_H__
    #include "header.h"
    class stack : header {
        float *f;
        int vol;
        public:
            float* stack0(char *file,int SunOS_Linux);
            int stack1(char *file,int SunOS_Linux,float *f);

            int getvol();

            //START180223
            //float* flip();
            //float* flip(float* f0);
            int flip(float* f0);

        };
#endif
