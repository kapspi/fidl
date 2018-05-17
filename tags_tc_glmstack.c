/* Copyright 8/13/09 Washington University.  All Rights Reserved.
   tags_tc_glmstack.c  $Revision: 1.6 $*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tags_tc_glmstack.h"

int get_tags_tc_guts_glmstack(int i,int *ii,TC *tcs,LinearModel **glmstack,TC *tc_weights,char *string,char *string3,
    Regions_By_File *fbf,char **stringsptr,int *integers)
{
    int j,k,m,n,p,glmi,tci;
    char string2[MAXNAME];
    string[0] = string3[0] = 0;
    for(n=p=1,j=0;j<tcs->num_tc_to_sum[i][0];j++,(*ii)++) {
        glmi = fbf->file_index[*ii];
        tci = fbf->roi_index[*ii];
        for(m=k=0;m<=tci;m+=glmstack[glmi]->ifh->glm_effect_length[k++]);k--;
        stringsptr[j]=glmstack[glmi]->ifh->glm_effect_label[k];
        integers[j] = tci - m+1 + glmstack[glmi]->ifh->glm_effect_length[k];
        if(j) {
            for(m=0;m<j;m++) {
                n = strcmp(glmstack[glmi]->ifh->glm_effect_label[k],stringsptr[m]) ? 1 : 0;
                if(!n) break;
                }
            for(m=0;m<j;m++) {
                p = integers[j] != integers[m] ? 1 : 0;
                if(!p) break;
                }
            }
        if(n) {
            if(j) strcat(string,"+");
            strcat(string,glmstack[glmi]->ifh->glm_effect_label[k]);
            }

        #if 0
        if(p) {
            sprintf(string2,"%d+",integers[j]);
            strcat(string3,string2);
            }
        #endif
        /*START121009*/
        if(p) {
            if(j) strcat(string3,"+");
            sprintf(string2,"%d",integers[j]);
            strcat(string3,string2);
            }


        if(tc_weights) sprintf(string,"%s_%.3f",string,tc_weights->tc[i][0][j]);
        }

    /*START121009*/
    /*if(!tc_weights) *strrchr(string3,'+') = 0;*/


    return 1;
}

Tags_Tc_Struct2 *get_tags_tc_glmstack(TC *tcs,int nglmfiles,LinearModel **glmstack,TC *tc_weights,Regions_By_File *fbf)
{
    char **stringsptr,string[MAXNAME],string3[MAXNAME]; /* *strings */
    int i,j,ii,len,nmax,total_tags_tc_len,total_tags_tc_effects_len,*integers;
    Tags_Tc_Struct2 *tags_tcs;
    if(!(tags_tcs=malloc(sizeof*tags_tcs))) {
        printf("Error: Unable to malloc tags_tcs in get_tags_tc\n");
        return NULL;
        }
    for(len=i=0;i<nglmfiles;i++) for(j=0;j<glmstack[i]->ifh->glm_all_eff;j++) len += glmstack[i]->ifh->glm_leffect_label[j]+1;
    for(nmax=i=0;i<tcs->num_tc;i++) if(tcs->num_tc_to_sum[i][0]>nmax) nmax=tcs->num_tc_to_sum[i][0]; 
    if(!(stringsptr=malloc(sizeof*stringsptr*nmax))) {
        printf("Error: Unable to malloc stringsptr in get_tags_tc_glmstack\n");
        return NULL;
        }
    if(!(integers=malloc(sizeof*integers*nmax))) {
        printf("Error: Unable to malloc integers in get_tags_tc_glmstack\n");
        return 0;
        }
    for(total_tags_tc_len=total_tags_tc_effects_len=ii=i=0;i<tcs->num_tc;i++) {
        if(!get_tags_tc_guts_glmstack(i,&ii,tcs,glmstack,tc_weights,string,string3,fbf,stringsptr,integers)) return NULL;
        total_tags_tc_len += strlen(string)+1;
        total_tags_tc_effects_len += strlen(string3)+1;
        }
    if(!(tags_tcs->tags_tc=malloc(sizeof*tags_tcs->tags_tc*total_tags_tc_len))) {
        printf("Error: Unable to malloc tags_tcs->tags_tc in get_tags_tc\n");
        return NULL;
        }
    if(!(tags_tcs->tags_tcptr=malloc(sizeof*tags_tcs->tags_tcptr*tcs->num_tc))) {
        printf("Error: Unable to malloc tags_tcs->tags_tc in get_tags_tc\n");
        return NULL;
        }
    if(total_tags_tc_effects_len==tcs->num_tc) {
        tags_tcs->tags_tc_effects=NULL;
        }
    else {
        if(!(tags_tcs->tags_tc_effects=malloc(sizeof*tags_tcs->tags_tc_effects*total_tags_tc_effects_len))) {
            printf("Error: Unable to malloc tags_tcs->tags_tc in get_tags_tc\n");
            return NULL;
            }
        if(!(tags_tcs->tags_tc_effectsptr=malloc(sizeof*tags_tcs->tags_tc_effectsptr*tcs->num_tc))) {
            printf("Error: Unable to malloc tags_tcs->tags_tc in get_tags_tc\n");
            return NULL;
            }
        }
    for(ii=i=0;i<tcs->num_tc;i++) {
        if(!get_tags_tc_guts_glmstack(i,&ii,tcs,glmstack,tc_weights,string,string3,fbf,stringsptr,integers)) return NULL;
        tags_tcs->tags_tcptr[i]=strcpy(tags_tcs->tags_tc,string);
        tags_tcs->tags_tc+=strlen(tags_tcs->tags_tc)+1;
        if(tags_tcs->tags_tc_effects) {
            tags_tcs->tags_tc_effectsptr[i]=strcpy(tags_tcs->tags_tc_effects,string3);
            tags_tcs->tags_tc_effects+=strlen(tags_tcs->tags_tc_effects)+1;
            }
        }
    free(integers);
    free(stringsptr);
    return tags_tcs;
}
