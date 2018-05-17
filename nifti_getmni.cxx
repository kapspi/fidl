/* Copyright 7/2/15 Washington University.  All Rights Reserved.
   nifti_getmni.cxx  $Revision: 1.4 $*/
//#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <cstdio>
#include <iostream>
#include "CiftiException.h"
#include "NiftiIO.h"
#include "VolumeSpace.h"
#include "nifti_getmni.h" 
using namespace std;
using namespace cifti;
int nifti_getmni(char *file,float *indices,float *coor){
    try{
        NiftiIO inputFile;
        inputFile.openRead(file);
        const NiftiHeader& myHeader=inputFile.getHeader();
        VolumeSpace myVolSpace(myHeader.getDimensions().data(),myHeader.getSForm());
        myVolSpace.indexToSpace(indices,coor);
        inputFile.close();
        }
    catch(CiftiException& e){
        cerr << "Caught CiftiException: " + AString_to_std_string(e.whatString()) << endl;
        return 0;
        }
    return 1;
    }
int _nifti_getmni(int argc,char **argv){

    //cout<<"_nifti_getmni top"<<endl;

    if(argc<3){
        cerr<<"fidlError: _nifti_getmni requires three arguments: char *file,float *indices,float *coor"<<endl;
        return 0;
        }
    char *file=argv[0];
    float *indices=(float*)argv[1];
    float *coor=(float*)argv[2];
    if(!nifti_getmni(file,indices,coor))return 0;

    //cout<<file<<endl;
    //cout<<"indices[0]="<<indices[0]<<" indices[1]="<<indices[1]<<" indices[2]="<<indices[2]<<endl;
    //cout<<"coor[0]="<<coor[0]<<" coor[1]="<<coor[1]<<" coor[2]="<<coor[2]<<endl;

    //cout<<"_nifti_getmni bottom"<<endl;

    return 1;
    }
