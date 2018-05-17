/* Copyright 7/13/17 Washington University.  All Rights Reserved.
   header.h  $Revision: 1.1 $ */
#ifndef __HEADER_H__
    #define __HEADER_H__ 

    //START180223
    #include <inttypes.h>

    class header{
        char *file;
        int filetype,dim[4],c_orient[3],bigendian,vol;

        //START182023
        int64_t dim64[4];

        float voxel_size[3],mmppixf[3],centerf[3];
        public:
            int header0(char* file0);
            int getfiletype(){return filetype;}
            void assign(int& xdim,int& ydim,int& zdim);
            void assign(int& xdim,int& ydim,int& zdim,float& voxel_size_1,float& voxel_size_2,float& voxel_size_3,
                float *center,float *mmppix);
            int getvol(float *temp_float,int SunOS_Linux);

            #if 0
            int getvol();
            int* get_c_orient();
            int64_t* get_dim64();
            #endif
            //START180319
            int getvol(){return vol;}
            int* get_c_orient(){return c_orient;}
            int64_t* get_dim64(){return dim64;}

        };
#endif
