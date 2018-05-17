/* Copyright 12/04/15 Washington University.  All Rights Reserved.
   read_tc_string_new.c  $Revision: 1.2 $*/
#define _POSIX_C_SOURCE 200112L
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "d2intvar.h"
#include "constants.h"
#include "strutil.h"
#include "read_tc_string_new.h"

//START170810
double ***d3doublevar(int dim1,int *dim2,int **dim3)
{
    int i,j,length,length2,length3;
    double ***array;

    if(!(array=(double ***)malloc((size_t)(dim1*sizeof(double**))))) {
        printf("allocation failure 1 in d3doublevar\n");
        return (double ***)NULL;
        }
    for(length=i=0;i<dim1;i++) length += dim2[i];
    if(!(array[0]=(double **)malloc((size_t)(length*sizeof(double*))))) {
        printf("allocation failure 2 in d3doublevar\n");
        return (double ***)NULL;
        }
    for(length=i=0;i<dim1;i++) for(j=0;j<dim2[i];j++) length += dim3[i][j];
    if(!(array[0][0]=(double *)calloc((size_t)(length),sizeof(double)))) {
        printf("allocation failure 3 in d3doublevar\n");
        return (double ***)NULL;
        }
    for(length=length2=i=0;i<dim1;i++,length+=dim2[i]) {
        array[i] = array[0] + length;
        array[i][0] = array[0][0] + length2;
        /*fprintf(stderr,"d3doublevar length=%d length2=%d\n",length,length2);*/

        /*for(length3=j=0;j<dim2[i];j++,length2+=dim3[i][j],length3+=dim3[i][j]) { length2 not incremented properly
            array[i][j] = array[i][0] + length3;
            fprintf(stderr,"d3doublevar length3=%d\n",length3);
            }*/

        for(length3=j=0;j<dim2[i];j++) {
            array[i][j] = array[i][0] + length3;
            /*fprintf(stderr,"d3doublevar length3=%d\n",length3);*/
            length2+=dim3[i][j];
            length3+=dim3[i][j];
            }

        }
    return array;
}

#include "read_tc_string_new.h"
TC *read_tc_string_new(int num_tc,int *num_glm_files,int argc_tc,char **argv){
    char *string,last_char,dummy,line[MAXNAME],write_back[MAXNAME],*strptr,first_char; //*write_back_ptr
    int i,j,k,l,m,n,*each,ti,len;
    TC *tcs;
    if(!(tcs=malloc(sizeof*tcs))) {
        printf("Error: Unable to malloc tcs\n");
        return NULL;
        }
    for(ti=i=0;i<num_tc;i++) if((k=strlen(argv[argc_tc+i])+1) > ti) ti = k;
    if(!(string=malloc(sizeof*string*ti))) {
        printf("Error: Unable to malloc string\n");
        return NULL;
        }
    for(tcs->num_tc=i=0;i<num_tc;i++) {

        //count_strings_new(argv[argc_tc+i],string,0,&last_char);
        //START170810
        count_strings_new3(argv[argc_tc+i],string,0,0,&last_char,&first_char,0,' ');

        if(!last_char) tcs->num_tc++;
        }
    if(!(tcs->each=malloc(sizeof*tcs->each*tcs->num_tc))) {
        printf("Error: Unable to malloc tcs->each\n");
        return NULL;
        }
    for(i=0;i<tcs->num_tc;i++) tcs->each[i]=0;
    for(j=i=0;i<num_tc;i++) {

        //tcs->each[j] += count_strings_new(argv[argc_tc+i],string,0,&last_char);
        //START170810
        tcs->each[j]+=count_strings_new3(argv[argc_tc+i],string,0,0,&last_char,&first_char,0,' ');

        if(!last_char) j++;
        }
    each = !num_glm_files ? tcs->each : num_glm_files;
    if(!(tcs->num_tc_to_sum = d2intvar(tcs->num_tc,each))) return (TC*)NULL;
    for(tcs->total=k=l=i=0;i<num_tc;i++) {

        //m = count_strings_new(argv[argc_tc+i],string,0,&last_char);
        //STARRT170810
        m=count_strings_new3(argv[argc_tc+i],string,0,0,&last_char,&first_char,0,' ');

        for(strptr=string,j=0;j<m;j++) {

            //strptr = grab_string(strptr,line);
            //tcs->total += tcs->num_tc_to_sum[k][l++] = count_strings_new(line,write_back,'+',&dummy);
            //START170810
            strptr = grab_string_new(strptr,line,&len);
            tcs->total += tcs->num_tc_to_sum[k][l++] = count_strings_new3(line,write_back,'+',0,&dummy,&first_char,0,' ');

            /*printf("tcs->num_tc_to_sum[%d][%d]=%d\n",k,l-1,tcs->num_tc_to_sum[k][l-1]);*/
            }
        if(!last_char) {
            k++;
            l = 0;
            }
        }

    #if 0
    printf("num_tc=%d tcs->num_tc=%d\n",num_tc,tcs->num_tc);
    printf("tcs->each="); for(i=0;i<num_tc;i++) printf("%d ",tcs->each[i]); printf("\n");
    printf("each="); for(i=0;i<num_tc;i++) printf("%d ",each[i]); printf("\n");
    printf("tcs->num_tc_to_sum\n");
    for(i=0;i<tcs->num_tc;i++) {
        for(j=0;j<each[i];j++) printf("%d ",tcs->num_tc_to_sum[i][j]);
        printf("\n");
        }
    #endif
    if(!(tcs->tc = d3doublevar(tcs->num_tc,each,tcs->num_tc_to_sum))) return NULL;

    for(k=l=i=0;i<num_tc;i++) {

        //m = count_strings_new(argv[argc_tc+i],string,0,&last_char);
        //START151204
        m = count_strings_new3(argv[argc_tc+i],string,0,0,&last_char,&first_char,0,' ');

        for(strptr=string,j=0;j<m;j++) {

            //strptr = grab_string(strptr,line);
            //n = count_strings_new(line,write_back,'+',&dummy);
            //START151204
            strptr = grab_string_new(strptr,line,&len);
            n = count_strings_new3(line,write_back,'+','+',&dummy,&first_char,0,' ');

            strings_to_double(write_back,tcs->tc[k][l++],n);
            }
        if(!last_char) {
            k++;
            l = 0;
            }
        }


    if(!(tcs->tcstacki=malloc(sizeof*tcs->tcstacki*tcs->total))) {
        printf("Error: Unable to malloc tcs->tcstacki\n");
        return NULL;
        }
    for(l=i=0;i<tcs->num_tc;i++) {
        for(j=0;j<each[i];j++) {
            for(k=0;k<tcs->num_tc_to_sum[i][j];k++,l++) {
                tcs->tcstacki[l]=(int)tcs->tc[i][j][k]-1;
                /*printf("i=%d j=%d k=%d l=%d\n",i,j,k,l);
                fflush(stdout);*/
                }
            }
        }
    if(num_glm_files) {
        for(i=1;i<tcs->num_tc;i++) if(each[i]!=each[0]) break;
        if(i!=tcs->num_tc) {
            printf("Error: each[%d]=%d each[0]=%d Must be equal.\n",i,each[i],each[0]);
            exit(-1);
            }
        else {
            if(!(tcs->total_each=malloc(sizeof*tcs->total_each*each[0]))) {
                printf("Error: Unable to malloc tcs->total_each\n");
                return NULL;
                }
            if(!(tcs->total_eachi=malloc(sizeof*tcs->total_eachi*tcs->total))) {
                printf("Error: Unable to malloc tcs->total_eachi\n");
                return NULL;
                }
            for(l=j=0;j<each[0];j++) {
                for(tcs->total_each[j]=i=0;i<tcs->num_tc;i++) {
                    tcs->total_each[j]+=tcs->num_tc_to_sum[i][j];
                    for(k=0;k<tcs->num_tc_to_sum[i][j];k++,l++) tcs->total_eachi[l]=(int)tcs->tc[i][j][k]-1;
                    }
                }
            }
        }
    free(string);
    return tcs;
}
TC *read_tc_string_new2(int num_tc,int argc_tc,char **argv,int lctotal_each){
    char *string,last_char,dummy,line[MAXNAME],write_back[MAXNAME],*strptr,first_char;
    int i,j,k,l,m,n,ti,len;
    TC *tcs;
    if(!(tcs=malloc(sizeof*tcs))) {
        printf("Error: Unable to malloc tcs\n");
        return NULL;
        }
    for(ti=i=0;i<num_tc;i++) if((k=strlen(argv[argc_tc+i])+1) > ti) ti = k;
    if(!(string=malloc(sizeof*string*ti))) {
        printf("Error: Unable to malloc string\n");
        return NULL;
        }
    for(tcs->num_tc=i=0;i<num_tc;i++) {

        //count_strings_new(argv[argc_tc+i],string,0,&last_char);
        //START170810
        count_strings_new3(argv[argc_tc+i],string,0,0,&last_char,&first_char,0,' ');

        if(!last_char) tcs->num_tc++;
        }

    /*printf("num_tc=%d tcs->num_tc=%d\n",num_tc,tcs->num_tc);fflush(stdout);*/

    if(!(tcs->each=malloc(sizeof*tcs->each*tcs->num_tc))) {
        printf("Error: Unable to malloc tcs->each\n");
        return NULL;
        }
    for(i=0;i<tcs->num_tc;i++) tcs->each[i]=0;
    for(j=i=0;i<num_tc;i++) {

        //tcs->each[j] += count_strings_new(argv[argc_tc+i],string,0,&last_char);
        //START170810
        tcs->each[j] += count_strings_new3(argv[argc_tc+i],string,0,0,&last_char,&first_char,0,' ');

        if(!last_char) j++;
        }

    /*printf("tcs->each="); for(i=0;i<tcs->num_tc;i++) printf("%d ",tcs->each[i]); printf("\n");fflush(stdout);*/

    if(!(tcs->num_tc_to_sum = d2intvar(tcs->num_tc,tcs->each))) return (TC*)NULL;
    for(tcs->total=k=l=i=0;i<num_tc;i++) {

        //m = count_strings_new(argv[argc_tc+i],string,0,&last_char);
        //START170810
        m = count_strings_new3(argv[argc_tc+i],string,0,0,&last_char,&first_char,0,' ');

        for(strptr=string,j=0;j<m;j++) {

            //strptr = grab_string(strptr,line);
            //tcs->total += tcs->num_tc_to_sum[k][l++] = count_strings_new(line,write_back,'+',&dummy);
            //START170810
            strptr = grab_string_new(strptr,line,&len);
            tcs->total += tcs->num_tc_to_sum[k][l++] = count_strings_new3(line,write_back,'+',0,&dummy,&first_char,0,' ');

            /*printf("tcs->num_tc_to_sum[%d][%d]=%d\n",k,l-1,tcs->num_tc_to_sum[k][l-1]);*/
            }
        if(!last_char) {
            k++;
            l = 0;
            }
        }
    #if 0
    printf("num_tc=%d tcs->num_tc=%d\n",num_tc,tcs->num_tc);
    printf("tcs->each="); for(i=0;i<tcs->num_tc;i++) printf("%d ",tcs->each[i]); printf("\n");
    printf("tcs->num_tc_to_sum\n");
    for(i=0;i<tcs->num_tc;i++) {
        for(j=0;j<tcs->each[i];j++) printf("%d ",tcs->num_tc_to_sum[i][j]);
        printf("\n");
        }
    #endif
    if(!(tcs->tc = d3doublevar(tcs->num_tc,tcs->each,tcs->num_tc_to_sum))) return NULL;
    for(k=l=i=0;i<num_tc;i++) {

        //m = count_strings_new(argv[argc_tc+i],string,0,&last_char);
        //START170810
        m = count_strings_new3(argv[argc_tc+i],string,0,0,&last_char,&first_char,0,' ');

        for(strptr=string,j=0;j<m;j++) {

            //strptr = grab_string(strptr,line);
            //n = count_strings_new(line,write_back,'+',&dummy);
            //START170810
            strptr = grab_string_new(strptr,line,&len);
            n = count_strings_new3(line,write_back,'+',0,&dummy,&first_char,0,' ');

            strings_to_double(write_back,tcs->tc[k][l++],n);
            }
        if(!last_char) {
            k++;
            l = 0;
            }
        }
    if(!(tcs->tcstacki=malloc(sizeof*tcs->tcstacki*tcs->total))) {
        printf("Error: Unable to malloc tcs->tcstacki\n");
        return NULL;
        }
    for(l=i=0;i<tcs->num_tc;i++) {
        for(j=0;j<tcs->each[i];j++) {
            for(k=0;k<tcs->num_tc_to_sum[i][j];k++,l++) {
                tcs->tcstacki[l]=(int)tcs->tc[i][j][k]-1;
                /*printf("i=%d j=%d k=%d l=%d\n",i,j,k,l);
                fflush(stdout);*/
                }
            }
        }

    if(lctotal_each) {
        for(i=1;i<tcs->num_tc;i++) if(tcs->each[i]!=tcs->each[0]) break;
        if(i!=tcs->num_tc) {
            printf("fidlError: tcs->each[%d]=%d tcs->each[0]=%d Must be equal.\n",i,tcs->each[i],tcs->each[0]);
            return NULL;
            }
        if(!(tcs->total_each=malloc(sizeof*tcs->total_each*tcs->each[0]))) {
            printf("fidlError: Unable to malloc tcs->total_each\n");
            return NULL;
            }
        if(!(tcs->total_eachi=malloc(sizeof*tcs->total_eachi*tcs->total))) {
            printf("fidlError: Unable to malloc tcs->total_eachi\n");
            return NULL;
            }
        for(l=j=0;j<tcs->each[0];j++) {
            for(tcs->total_each[j]=i=0;i<tcs->num_tc;i++) {
                tcs->total_each[j]+=tcs->num_tc_to_sum[i][j];
                for(k=0;k<tcs->num_tc_to_sum[i][j];k++,l++) tcs->total_eachi[l]=(int)tcs->tc[i][j][k]-1;
                }
            }
        /*for(j=0;j<tcs->each[0];j++) printf("tcs->total_each[%d]=%d\n",j,tcs->total_each[j]);
        printf("tcs->total_eachi= ");for(j=0;j<tcs->total;j++)printf("%d ",tcs->total_eachi[j]);printf("\n");fflush(stdout);*/
        }


    free(string);
    return tcs;
    }
TCnew *read_tc_string_TCnew(int num_tc,int *num_glm_files,int argc_tc,char **argv,char plusordash){
    char *string,last_char,dummy,line[MAXNAME],write_back[MAXNAME],*strptr,first_char;
    int i,j,k,m,n,*each,ti,len;
    TCnew *tcs;
    double *dp;

    if(!(tcs=malloc(sizeof*tcs))) {
        printf("Error: Unable to malloc tcs\n");
        return NULL;
        }
    if(!(tcs->each=malloc(sizeof*tcs->each*num_tc))) {
        printf("Error: Unable to malloc tcs->each\n");
        return NULL;
        }
    for(ti=i=0;i<num_tc;i++) if((k=strlen(argv[argc_tc+i])+1) > ti) ti = k;
    if(!(string=malloc(sizeof*string*ti))) {
        printf("Error: Unable to malloc string\n");
        return NULL;
        }
    for(k=i=0;i<num_tc;i++) {

        //tcs->each[i] = count_strings_new(argv[argc_tc+i],string,0,&last_char);
        //START170810
        tcs->each[i]=count_strings_new3(argv[argc_tc+i],string,0,0,&last_char,&first_char,0,' ');

        if(!last_char) k++;
        }
    tcs->num_tc = k;
    each = !num_glm_files ? tcs->each : num_glm_files;
    for(k=i=0;i<num_tc;i++) k += each[i];
    if(!(tcs->num_tc_to_sum=malloc(sizeof*tcs->num_tc_to_sum*k))) {
        printf("Error: Unable to malloc tcs->num_tc_to_sum\n");
        return NULL;
        }
    for(tcs->total=k=i=0;i<num_tc;i++) {

        //m = count_strings_new(argv[argc_tc+i],string,0,&last_char);
        //START170810
        m = count_strings_new3(argv[argc_tc+i],string,0,0,&last_char,&first_char,0,' ');

        for(strptr=string,j=0;j<m;j++) {

            //strptr = grab_string(strptr,line);
            //tcs->total += tcs->num_tc_to_sum[k++] = count_strings_new(line,write_back,plusordash,&dummy);
            //START170810
            strptr = grab_string_new(strptr,line,&len);
            tcs->total += tcs->num_tc_to_sum[k++] = count_strings_new3(line,write_back,plusordash,0,&dummy,&first_char,0,' ');

            }
        }
    for(k=i=0;i<tcs->num_tc;i++) for(j=0;j<each[i];j++) k += tcs->num_tc_to_sum[k];
    if(!(tcs->tc=malloc(sizeof*tcs->tc*k))) {
        printf("Error: Unable to malloc tcs->tc\n");
        return NULL;
        }
    dp = tcs->tc;
    for(i=0;i<num_tc;i++) {

        //m = count_strings_new(argv[argc_tc+i],string,0,&last_char);
        //START170810
        m=count_strings_new3(argv[argc_tc+i],string,0,0,&last_char,&first_char,0,' ');

        for(strptr=string,j=0;j<m;j++) {

            //strptr = grab_string(strptr,line);
            //n = count_strings_new(line,write_back,plusordash,&dummy);
            //START170810
            strptr = grab_string_new(strptr,line,&len);
            n=count_strings_new3(line,write_back,plusordash,0,&dummy,&first_char,0,' ');

            dp = strings_to_double(write_back,dp,n);
            }
        }
    free(string);
    if(!(tcs->eachi=malloc(sizeof*tcs->eachi*num_tc))) {
        printf("Error: Unable to malloc tcs->eachi\n");
        return NULL;
        }
    tcs->eachi[0] = 0;
    for(i=0;i<num_tc-1;i++) tcs->eachi[i+1] = tcs->eachi[i] + tcs->each[i];

    #if 0
    printf("num_tc=%d tcs->num_tc=%d\n",num_tc,tcs->num_tc);
    printf("tcs->each="); for(i=0;i<num_tc;i++) printf("%d ",tcs->each[i]); printf("\n");
    printf("each="); for(i=0;i<num_tc;i++) printf("%d ",each[i]); printf("\n");
    printf("tcs->num_tc_to_sum\n");
    for(k=i=0;i<tcs->num_tc;i++) {
        for(j=0;j<each[i];j++,k++) printf("%d ",tcs->num_tc_to_sum[k]);
        printf("\n");
        }
    printf("tcs->tc\n");
    for(m=k=i=0;i<tcs->num_tc;i++) {
        for(j=0;j<each[i];j++,k++) {
            for(l=0;l<tcs->num_tc_to_sum[k];l++,m++) printf("%f ",tcs->tc[m]); 
            printf("\n");
            } 
        }
    printf("tcs->eachi="); for(i=0;i<num_tc;i++) printf("%d ",tcs->eachi[i]); printf("\n");
    #endif
 
    return tcs;
    }
