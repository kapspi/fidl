/* Copyright 3/15/18 Washington University.  All Rights Reserved.
   atlas.cxx  $Revision: 1.1 $*/
#include <iostream>
#include "atlas.h"
using namespace std;

void getxyz(int index,int xdim,int ydim,float *xyz){
    //slice,col_row,row,col must be integers
    int area=xdim*ydim;
    int slice =index/area;
    int col_row = index-slice*area;
    int row = col_row/xdim;
    int col = col_row - row*xdim;

    //cout<<"index= "<<index<<" slice= "<<slice<<" area= "<<area<<" col_row= "<<col_row<<endl;

    xyz[0]=(float)col-1.;xyz[1]=(float)row;xyz[2]=(float)slice;
    }

#if 0
void col_row_slice(int n,int *index,double *dcol,double *drow,double *dslice,Atlas_Param *ap){
    int i,slice,col_row,col,row; /*Must be integers.*/
    for(i=0;i<n;i++) {
        slice = index[i]/ap->area;
        col_row = index[i]-slice*ap->area;
        row = col_row/ap->xdim;
        col = col_row - row*ap->xdim;
        dcol[i] = (double)col;
        drow[i] = (double)row;
        dslice[i] = (double)slice;
        }
    }
#endif

