/* Copyright 4/17/03 Washington University.  All Rights Reserved.
   d2float.c  $Revision: 1.4 $*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "d2float.h"
float **d2float(int dim1,int dim2)
{
    int i;
    float **array;
    if(!(array=(float **)malloc((size_t)(dim1*sizeof(float*))))) {
        printf("allocation failure 1 in d2float\n");
        return (float **)NULL;
        }
    if(!(array[0]=(float *)calloc((size_t)(dim1*dim2),sizeof(float)))) {
        printf("allocation failure 2 in d2float\n");
        return (float **)NULL;
        }
    for(i=1;i<dim1;i++) array[i]=array[i-1]+dim2;
    return array;
}
void free_d2float(float **array)
{
    free(array[0]);
    free(array);
}
