/* Copyright 10/10/06 Washington University.  All Rights Reserved.
   fidl_chisquare_to_z.c  $Revision: 1.4 $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <errno.h> KEEP
//#include <math.h>
//#include "fidl.h"

//START171220
#include "constants.h"
#include "ifh.h"
#include "checkOS.h"
#include "x2_to_z.h"
#include "shouldiswap.h"
#include "subs_util.h"
#include "minmax.h"

int main(int argc,char **argv){
char *chisquaref=NULL,string[MAXNAME],*strptr,*dfs=NULL;
int i,SunOS_Linux,vol,swapbytes;
float *temp_float;
double *chisquare,*z,*df,x0,z0,df0;
Interfile_header *ifh;

if(argc < 3) {

    //fprintf(stderr,"    -chisquare: Chisquare image.\n");
    //START171219
    printf("    -chisquare: Chisquare image or number.\n");
    printf("    -df: If number, then need to specify the degrees of freedom.\n");

    exit(-1);
    }

for(i=1;i<argc;i++) {

    #if 0
    if(!strcmp(argv[i],"-chisquare") && argc > i+1)
        chisquaref = argv[++i];
    #endif
    //START171220
    if(!strcmp(argv[i],"-chisquare") && argc > i+1){
        //errno=0;
        x0=strtod(chisquaref=argv[++i],&strptr);
        if(strptr!=chisquaref){ //a conversion was performed
            if(*strptr){
                printf("Found data '%s' at end of %s. Assuming it's a filename.\n",strptr,chisquaref);
                }
            else{
                //*chisquaref=0;
                }
            }
        else{ //trouble
            printf("%s doesn't start with a number. Assuming it's a filename.\n",chisquaref); 
            }
        }
    if(!strcmp(argv[i],"-df") && argc > i+1){
        //errno=0;
        df0=strtod(dfs=argv[++i],&strptr);
        if(strptr!=dfs){ //a conversion was performed
            if(*strptr){
                printf("fidlError: -dfs Found data '%s' at end of %s. Abort.\n",strptr,dfs);exit(-1);
                }
            else{
                }
            }
        else{ //trouble
            printf("fidlError: -dfs %s doesn't start with a number. Abort!\n",dfs);exit(-1);
            }
        }


    }
if(!chisquaref) {
    printf("Error: Need to specify -chisquare\n");
    exit(-1);
    }
if((SunOS_Linux=checkOS())==-1) exit(-1);


#if 0
if(!(ifh=read_ifh(chisquaref))) exit(-1);
vol = ifh->dim1*ifh->dim2*ifh->dim3;
swapbytes=shouldiswap(SunOS_Linux,ifh->bigendian);
if(!(temp_float=malloc(sizeof*temp_float*vol))) {
    printf("Error: Unable to malloc temp_float\n");
    exit(-1);
    }
if(!(chisquare=malloc(sizeof*chisquare*vol))) {
    printf("Error: Unable to malloc chisquare\n");
    exit(-1);
    }
if(!(z=malloc(sizeof*z*vol))) {
    printf("Error: Unable to malloc z\n");
    exit(-1);
    }
if(!(df=malloc(sizeof*df*vol))) {
    printf("Error: Unable to malloc df\n");
    exit(-1);
    }
if(!readstack(chisquaref,(float*)temp_float,sizeof(float),(size_t)vol,SunOS_Linux)) exit(-1);
for(i=0;i<vol;i++) chisquare[i] = temp_float[i]==(float)UNSAMPLED_VOXEL ? (double)UNSAMPLED_VOXEL : (double)temp_float[i]; 
printf("ifh->dof_condition=%f\n",ifh->dof_condition);
df[0] = (double)ifh->dof_condition;
for(i=1;i<vol;i++) df[i] = df[0];
if(!x2_to_z(chisquare,z,vol,df,(int*)NULL,(Atlas_Param*)NULL)) exit(-1);
for(i=0;i<vol;i++) temp_float[i] = (float)z[i];
strcpy(string,chisquaref);
if(!(strptr=strstr(string,"chisquare"))) {
    printf("Error: chisquare not found in %s\n",string);
    exit(-1);
    }
*strptr = 0;
strcat(string,"zstat");
strcat(string,strptr+9);
if(!writestack(string,temp_float,sizeof(float),(size_t)vol,swapbytes)) exit(-1);
min_and_max_init(&ifh->global_min,&ifh->global_max);
min_and_max_doublestack(z,vol,&ifh->global_min,&ifh->global_max);
free(ifh->file_name);
if(!(ifh->file_name=malloc(sizeof*ifh->file_name*(strlen(chisquaref)+1)))) exit(-1);
strcpy(ifh->file_name,chisquaref);
if(!write_ifh(string,ifh,(int)FALSE)) exit(-1);
printf("Z statistic written to %s\n",string);
#endif
//START171220
if(dfs){
    x2_to_z(&x0,&z0,1,&df0);
    printf("x2 = %f df = %f -> z = %f\n",x0,df0,z0);
    }
else{
    if(!(ifh=read_ifh(chisquaref,(Interfile_header*)NULL)))exit(-1);
    vol = ifh->dim1*ifh->dim2*ifh->dim3;
    swapbytes=shouldiswap(SunOS_Linux,ifh->bigendian);
    if(!(temp_float=malloc(sizeof*temp_float*vol))) {
        printf("Error: Unable to malloc temp_float\n");
        exit(-1);
        }
    if(!(chisquare=malloc(sizeof*chisquare*vol))) {
        printf("Error: Unable to malloc chisquare\n");
        exit(-1);
        }
    if(!(z=malloc(sizeof*z*vol))) {
        printf("Error: Unable to malloc z\n");
        exit(-1);
        }
    if(!(df=malloc(sizeof*df*vol))) {
        printf("Error: Unable to malloc df\n");
        exit(-1);
        }
    if(!readstack(chisquaref,(float*)temp_float,sizeof(float),(size_t)vol,SunOS_Linux,ifh->bigendian))exit(-1);
    for(i=0;i<vol;i++) chisquare[i] = temp_float[i]==(float)UNSAMPLED_VOXEL ? (double)UNSAMPLED_VOXEL : (double)temp_float[i];
    printf("ifh->df1=%f\n",ifh->df1);
    df[0] = (double)ifh->df1;
    for(i=1;i<vol;i++) df[i] = df[0];
    x2_to_z(chisquare,z,vol,df);
    for(i=0;i<vol;i++) temp_float[i] = (float)z[i];
    strcpy(string,chisquaref);
    if(!(strptr=strstr(string,"chisquare"))) {
        printf("Error: chisquare not found in %s\n",string);
        exit(-1);
        }
    *strptr = 0;
    strcat(string,"zstat");
    strcat(string,strptr+9);
    if(!writestack(string,temp_float,sizeof(float),(size_t)vol,swapbytes)) exit(-1);
    min_and_max_init(&ifh->global_min,&ifh->global_max);
    min_and_max_doublestack(z,vol,&ifh->global_min,&ifh->global_max);
    free(ifh->file_name);
    if(!(ifh->file_name=malloc(sizeof*ifh->file_name*(strlen(chisquaref)+1)))) exit(-1);
    strcpy(ifh->file_name,chisquaref);
    if(!write_ifh(string,ifh,0))exit(-1);
    printf("Z statistic written to %s\n",string);
    } 

}
