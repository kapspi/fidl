/* Copyright 12/04/15 Washington University.  All Rights Reserved.
   read_tc_string_new.h  $Revision: 1.2 $*/
#ifndef __READ_TC_STRING_NEW_H__
    #define __READ_TC_STRING_NEW_H__
    typedef struct {
        int     num_tc,total,*tcstacki,*total_each,*total_eachi;
        int     *each;
        int     **num_tc_to_sum; /*num_tc_to_sum[num_tc][num_glm_files]*/
        double  ***tc;  /*tc[num_tc][num_glm_files][num_tc_to_sum] changed to double to accomodate tc_weights*/
        } TC;

    #ifdef __cplusplus
        extern "C" {
    #endif

    double ***d3doublevar(int dim1,int *dim2,int **dim3);
    TC *read_tc_string_new(int num_tc,int *num_glm_files,int argc_tc,char **argv);
    TC *read_tc_string_new2(int num_tc,int argc_tc,char **argv,int lctotal_each);
    typedef struct {
        int     num_tc,total,*each,*num_tc_to_sum,*eachi; /*num_tc_to_sum[num_tc][num_glm_files]*/
        double *tc; /*tc[num_tc][num_glm_files][num_tc_to_sum] changed to double to accomodate tc_weights*/
        } TCnew;
    TCnew *read_tc_string_TCnew(int num_tc,int *num_glm_files,int argc_tc,char **argv,char plusorminus);

    #ifdef __cplusplus
        }//extern
    #endif
#endif
