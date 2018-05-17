/* Copyright 12/31/99 Washington University.  All Rights Reserved.
   twoA_or_twoB.c  $Revision: 1.15 $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "fidl.h"
//START170802
#include "twoA_or_twoB.h"

void _twoA_or_twoB(int argc,char **argv){
    char *t4_filename;
    int A_or_B_or_U;
    float *twoAis0_twoBis1_unknown2;
    t4_filename = (char *)argv[0];
    twoAis0_twoBis1_unknown2 = (float *)argv[1];
    A_or_B_or_U = twoA_or_twoB(t4_filename);
    if(A_or_B_or_U == 0)
        *twoAis0_twoBis1_unknown2 = 0.;
    else if(A_or_B_or_U == 1)
        *twoAis0_twoBis1_unknown2 = 1.; 
    else
        *twoAis0_twoBis1_unknown2 = 2.;
    }
int twoA_or_twoB(char *t4_filename){
    int count=0,A_or_B_or_U=2;
    if(strstr(t4_filename,"2A")) {
        count++;
        A_or_B_or_U = 0;
        } 
    if(strstr(t4_filename,"anat_ave_")) {
        if(strstr(t4_filename,"_to_711-2B_t4") || strstr(t4_filename,"_to_711-2C_t4") || 
            strstr(t4_filename,"_to_711-2Y_t4") || strstr(t4_filename,"_to_711-2L_t4") ||
            strstr(t4_filename,"_to_711-2F_t4") || strstr(t4_filename,"_to_TRIO_KY_NDC_t4")) {
            count++;
            A_or_B_or_U = 1;
            }
        } 
    if(count==2) {
        fprintf(stderr,"Error: t4 filename contains both 2A and 2B. Illegal!\n");
        A_or_B_or_U = 2;
        }
    else if(!count) {
        A_or_B_or_U = 1;
        }
    return A_or_B_or_U;
    }
