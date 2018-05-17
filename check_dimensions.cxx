/* Copyright 3/19/18 Washington University.  All Rights Reserved.
   check_dimensions.cxx  $Revision: 1.1 $*/
#include <iostream>
#include <string.h>
#include "header.h"
#include "check_dimensions.h"
using namespace std;
int check_dimensions(int nfiles,char **files_ptr,int vol){
    int i,vol0;
    header h;
    for(i=0;i<nfiles;i++){
        if(strcmp(files_ptr[i],"NA")){
            if(!(vol0=h.header0(files_ptr[i])))return 0;
            if(vol0!=vol){
                cout<<"fidlError: Dimensions of images are not equal. Problem file: "<<files_ptr[i]<<endl;
                cout<<"fidlError: Should be "<<vol<<" instead of "<<vol0<<endl;
                return 0;
                }
            }
        }
    return 1;
    }
