/* Copyright 5/18/05 Washington University.  All Rights Reserved.
   d2intvar.c  $Revision: 1.4 $*/
#include <stdlib.h>
#include <stdio.h>
#include "d2intvar.h"
int **d2intvar(int dim1,int *dim2)
{
    size_t i,length;
    int **array;
    if(!(array=malloc((size_t)(dim1*sizeof(int*))))) {
        printf("allocation failure 1 in d2intvar\n");
        return NULL;
        }
    for(length=i=0;i<(size_t)dim1;i++) length += dim2[i];
    if(!(array[0]=calloc((size_t)length,sizeof(int)))) {
        printf("allocation failure 2 in d2intvar\n");
        return NULL;
        }
    for(i=1;i<(size_t)dim1;i++) array[i]=array[i-1]+dim2[i-1];
    return array;
}
