/* Copyright 3/16/18 Washington University.  All Rights Reserved.
   get_indbase.c  $Revision: 1.1 $*/
#include <math.h>
#include "read_tc_string_new.h"
#include "find_regions_by_file_cover.h"
#include "subs_glm.h"

int get_indbase(TC *tcs,Regions_By_File *fbf,LinearModel **glmstack,Meancol **meancol_stack,int *indbase_col,int subi){
    int i,j,k,l,kk,glmi,tci;
    for(kk=k=0;k<tcs->num_tc;k++) {
        for(i=0;i<tcs->num_tc_to_sum[k][subi];i++,kk++) {
            indbase_col[kk] = -1;
            glmi = fbf->file_index[kk];
            tci = fbf->roi_index[kk];
            for(j=0;j<glmstack[glmi]->ifh->glm_Nrow;j++) {
                if(fabs(glmstack[glmi]->AT[tci][j]) > (double)UNSAMPLED_VOXEL) {
                    for(l=0;l<meancol_stack[glmi]->ncol;l++) {
                        if(glmstack[glmi]->AT[meancol_stack[glmi]->col[l]][j] > (float)UNSAMPLED_VOXEL) {
                            if(indbase_col[kk]==-1) {
                                indbase_col[kk] = meancol_stack[glmi]->col[l];
                                }
                            else {
                                printf("Error: get_indbase More than one baseline found!\n");
                                return 0;
                                }
                            }
                        }
                    if(indbase_col[kk]==-1) {
                        printf("Error: get_indbase Baseline not found!\n");
                        return 0;
                        }
                    break;
                    }
                }
            }
        }
    return 1;
    }

