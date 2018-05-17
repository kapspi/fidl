/* Copyright 9/17/04 Washington University.  All Rights Reserved.
   get_atlas.c  $Revision: 1.14 $ */
#include <string.h>

//#include "fidl.h"       
//START170802
#include "get_atlas.h"       
#include "constants.h"       

                      /*char atlas[7]="";*/
char *get_atlas(int vol,char *atlas){

    //char *lizard[]={"111","222","333","222MNI","333MNI"};
    //START170616
    char *lizard[]={"111","222","333","222MNI","333MNI","111MNI"};

    size_t i;
    if(vol==(int)VOL_111)i=0;
    else if(vol==(int)VOL_222)i=1;
    else if(vol==(int)VOL_333)i=2;
    else if(vol==(int)VOL_222MNI)i=3;
    else if(vol==(int)VOL_333MNI)i=4;

    //START170616
    else if(vol==(int)VOL_111MNI)i=5;

    else{*atlas=0;return NULL;}
    strcpy(atlas,lizard[i]);
    return atlas;
    }
