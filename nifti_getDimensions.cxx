/* Copyright 6/23/15 Washington University.  All Rights Reserved.
   nifti_getDimensions.cxx  $Revision: 1.3 $*/
//#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "CiftiException.h"
#include "NiftiIO.h"
#include <cstdio>
#include <iostream>
using namespace std;
using namespace cifti;
#include "nifti_getDimensions.h"
int nifti_getDimensions(char *file,int64_t *xdim,int64_t *ydim,int64_t *zdim,int64_t *tdim)
{
    try
    {
        NiftiIO inputFile;
        inputFile.openRead(file);
        int nc;
        if((nc=inputFile.getNumComponents())!=1){ 
            cerr<<"fidlError: inputFile.getNumComponents()="<<nc<<" Only one is allowed."<<endl;
            return 0;
            }
        if(inputFile.getDimensions().size()<3||inputFile.getDimensions().size()>4){
            cerr << "fidlError: "<<file<<" Not a 3D or 4D volume" << endl;
            return 0;
            }
        *xdim = inputFile.getDimensions()[0];
        *ydim = inputFile.getDimensions()[1];
        *zdim = inputFile.getDimensions()[2];

        //*tdim = inputFile.getDimensions()[3];
        //START170620
        *tdim=inputFile.getDimensions().size()>3?inputFile.getDimensions()[3]:1;

        inputFile.close();
    } catch (CiftiException& e) {
        cerr << "Caught CiftiException: " + AString_to_std_string(e.whatString()) << endl;
        return 0;
    }
    return 1;
}
int _nifti_getDimensions(int argc,char **argv)
{
    if(argc<5)
    {
        cerr << "_nifti_getDimensions requires five arguments: an input nifti file, int64_t xdim, int64_t ydim, int64_t zdim, int64_t tdim" << endl;
        return 0;
    }
    char *file=argv[0];
    int64_t *xdim = (intptr_t*)argv[1];
    int64_t *ydim = (intptr_t*)argv[2];
    int64_t *zdim = (intptr_t*)argv[3];
    int64_t *tdim = (intptr_t*)argv[4];
    if(!nifti_getDimensions(file,xdim,ydim,zdim,tdim)) return 0;

    //printf("xdim=" "%"PRId64"\tydim=" "%"PRId64"\tzdim=" "%"PRId64"\ttdim=" "%"PRId64"\n",*xdim,*ydim,*zdim,*tdim);fflush(stdout);
    //START150629
    cout<<"xdim="<<*xdim<<" ydim="<<*ydim<<" zdim="<<*zdim<<" tdim="<<*tdim<<endl;

    return 1;
}

#if 0
/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx: In function âint nifti_getDimensions(char*, int64_t*, int64_t*, int64_t*,
int64_t*)â:
/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx:41: error: redefinition of âint nifti_getDimensions(char*, int64_t*,
int64_t*, int64_t*, int64_t*)â
/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx:17: error: âint nifti_getDimensions(char*, int64_t*, int64_t*, int64_t*,
int64_t*)â previously defined here
make[3]: *** [CMakeFiles/fidlcifti.dir/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx.o] Error 1
make[2]: *** [CMakeFiles/fidlcifti.dir/all] Error 2
make[1]: *** [CMakeFiles/fidlcifti.dir/rule] Error 2
make: *** [fidlcifti] Error 2
#endif
