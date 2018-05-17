/* Copyright 6/23/15 Washington University.  All Rights Reserved.
   nifti_gethdr.cxx  $Revision: 1.7 $*/
//#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "CiftiException.h"
#include "NiftiIO.h"
#include "VolumeSpace.h"
#include <cstdio>
#include <iostream>
#include <vector>
#include "nifti_gethdr.h"
using namespace std;
using namespace cifti;

int nifti_gethdr(char *file,int64_t *xdim,int64_t *ydim,int64_t *zdim,int64_t *tdim,float *center,float *mmppix,int *c_orient){
    //cout<<"nifti_gethdr top"<<endl;
    try{
        NiftiIO inputFile;
        inputFile.openRead(file);
        int nc;
        if((nc=inputFile.getNumComponents())!=1){
            cerr<<"fidlError: "<<file<<" getNumComponents()="<<nc<<" Only one is allowed."<<endl;
            return 0;
            }
        if(inputFile.getDimensions().size()<3||inputFile.getDimensions().size()>4){
            cerr<<"fidlError: "<<file<<" Not a 3D or 4D volume"<<endl;
            cerr<<"fidlError: dims= ";
            vector<int64_t> dims=inputFile.getDimensions();
            for(vector<int64_t>::iterator i=dims.begin();i!=dims.end();++i)cerr<<*i<<' ';
            cerr<<endl;

            return 0;
            }

        //cout << "nifti_gethdr inputFile.getDimensions().size()=" << inputFile.getDimensions().size() << endl;


        const NiftiHeader& myHeader=inputFile.getHeader();
        VolumeSpace myVolSpace(myHeader.getDimensions().data(),myHeader.getSForm());

        *xdim = inputFile.getDimensions()[0];
        *ydim = inputFile.getDimensions()[1];
        *zdim = inputFile.getDimensions()[2];
        *tdim=inputFile.getDimensions().size()>3?inputFile.getDimensions()[3]:1;
        VolumeSpace::OrientTypes orient[3];
        if(!myVolSpace.isPlumb()){
            cerr<<"fidlInfo: "<<file<<" oblique volume"<<endl;
            cerr<<"fidlInfo: "<<*xdim<<" "<<*ydim<<" "<<*zdim<<endl;
            myVolSpace.getOrientation(orient);
            }
        else{
            myVolSpace.getOrientAndSpacingForPlumb(orient,mmppix,center);
            }
        if(c_orient){
            c_orient[0]=(intptr_t)orient[0];
            c_orient[1]=(intptr_t)orient[1];
            c_orient[2]=(intptr_t)orient[2];
            }




        //cout<<file<<endl;
        //if(c_orient){cout<<"orient[0]="<<orient[0]<<" orient[1]="<<orient[1]<<" orient[2]="<<orient[2]<<endl;}
        //cout<<"mmppix[0]="<<mmppix[0]<<" mmppix[1]="<<mmppix[1]<<" mmppix[2]="<<mmppix[2]<<endl;
        //cout<<"center[0]="<<center[0]<<" center[1]="<<center[1]<<" center[2]="<<center[2]<<endl;

        //START150708
        #if 0
        vector<vector<float> > sform=myHeader.getSForm();
        for(vector<vector<float> >::iterator i=sform.begin();i!=sform.end();++i){
            for(vector<float>::iterator j=i->begin();j!=i->end();++j)cout<<*j<<' ';
            cout<<endl;
            }

        //START150702
        size_t k=0;
        for(vector<vector<float> >::iterator i=sform.begin();i!=sform.end()-1;++i){
            for(vector<float>::iterator j=i->begin();j!=i->end();++j,++k)c_sform[k]=*j;
            }
        #endif

        #if 0
        //KEEP
        vector<vector<float> > sform=myHeader.getSForm();
        cout<<"sform"<<endl;
        for(size_t i=0;i<sform.size();++i){
            for(size_t j=0;j<sform[i].size();++j)cout<<sform[i][j]<<' ';
            cout<<endl;
            }
        #endif


        //START170620
        //cout << "nifti_gethdr inputFile.numBytesPerElem()=" << inputFile.numBytesPerElem() << endl;
        //cout << "nifti_gethdr myHeader.getDataType()=" << myHeader.getDataType() << endl;
        #if 0
        switch (myHeader.getDataType())
        {
            case NIFTI_TYPE_UINT8:
                cout << "NIFTI_TYPE_UINT8" << endl;
                break;
            case NIFTI_TYPE_RGB24:
                cout << "NIFTI_TYPE_RGB24" << endl;
                break;
            case NIFTI_TYPE_INT8:
                cout << "NIFTI_TYPE_INT8" << endl;
                break;
            case NIFTI_TYPE_UINT16:
                cout << "NIFTI_TYPE_UINT16" << endl;
                break;
            case NIFTI_TYPE_INT16:
                cout << "NIFTI_TYPE_INT16" << endl;
                break;
            case NIFTI_TYPE_UINT32:
                cout << "NIFTI_TYPE_UINT32" << endl;
                break;
            case NIFTI_TYPE_INT32:
                cout << "NIFTI_TYPE_UINT32" << endl;
                break;
            case NIFTI_TYPE_UINT64:
                cout << "NIFTI_TYPE_UINT64" << endl;
                break;
            case NIFTI_TYPE_INT64:
                cout << "NIFTI_TYPE_INT64" << endl;
                break;
            case NIFTI_TYPE_FLOAT32:
                cout << "NIFTI_TYPE_FLOAT32" << endl;
                break;
            case NIFTI_TYPE_COMPLEX64:
                cout << "NIFTI_TYPE_COMPLEX64" << endl;
                break;
            case NIFTI_TYPE_FLOAT64:
                cout << "NIFTI_TYPE_FLOAT64" << endl;
                break;
            case NIFTI_TYPE_COMPLEX128:
                cout << "NIFTI_TYPE_COMPLEX128" << endl;
                break;
            case NIFTI_TYPE_FLOAT128:
                cout << "NIFTI_TYPE_FLOAT128" << endl;
                break;
            case NIFTI_TYPE_COMPLEX256:
                cout << "NIFTI_TYPE_COMPLEX256" << endl;
                break;
            default:
                CiftiAssert(0);
                throw CiftiException("internal error, tell the developers what you just tried to do");
        }
        #endif


        //cout<<"nifti_gethdr xdim="<<*xdim<<" ydim="<<*ydim<<" zdim="<<*zdim<<" tdim="<<*tdim<<endl;

        inputFile.close();
    } catch (CiftiException& e) {
        cerr << "Caught CiftiException: " + AString_to_std_string(e.whatString()) << endl;
        return 0;
    }

    //cout<<"nifti_gethdr bottom"<<endl;

    return 1;
}
int _nifti_gethdr(int argc,char **argv){

    //cout<<"_nifti_gethdr top"<<endl;

    if(argc<8){
        cerr<<"fidlError: _nifti_gethdr requires eight arguments: char *file,int64_t *xdim,int64_t *ydim,int64_t *zdim,"<<endl;
        cerr<<"    int64_t *tdim,float *center,float *mmppix,int *c_orient"<<endl;
        return 0;
        }
    char *file=argv[0];
    int64_t *xdim=(intptr_t*)argv[1];
    int64_t *ydim=(intptr_t*)argv[2];
    int64_t *zdim=(intptr_t*)argv[3];
    int64_t *tdim=(intptr_t*)argv[4];
    float *center=(float*)argv[5];
    float *mmppix=(float*)argv[6];
    int *c_orient=(int*)argv[7];
    if(!nifti_gethdr(file,xdim,ydim,zdim,tdim,center,mmppix,c_orient)) return 0;

    //cout<<file<<endl;
    //cout<<"xdim="<<*xdim<<" ydim="<<*ydim<<" zdim="<<*zdim<<" tdim="<<*tdim<<endl;
    //cout<<"center[0]="<<center[0]<<" center[1]="<<center[1]<<" center[2]="<<center[2]<<endl;
    //cout<<"mmppix[0]="<<mmppix[0]<<" mmppix[1]="<<mmppix[1]<<" mmppix[2]="<<mmppix[2]<<endl;
    //cout<<"c_orient[0]="<<c_orient[0]<<" c_orient[1]="<<c_orient[1]<<" c_orient[2]="<<c_orient[2]<<endl;

    //cout<<"_nifti_gethdr bottom"<<endl;

    return 1;
    }
