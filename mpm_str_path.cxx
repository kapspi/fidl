/* Copyright 11/13/17 Washington University.  All Rights Reserved.
   mpm_str_path.cxx  $Revision: 1.1 $ */
#include <iostream>
#include <cstring>
#include "fidl.h" 
using namespace std;
int main(int argc,char **argv)
{
char *dir;
if(argc!=2){
    printf("mpm_str_path <string>\n");
    printf("    Ex. mpm_str_path /data/nil-bluearc/raichle/mcavoy/midnight/ds000224_R1.0.2_sub07-10/sub-MSC10/ses-func01/func/sub-MSC10_ses-func01_task-rest_bold.nii.gz\n");
    //printf("        returns /data/nil-bluearc/raichle/mcavoy/midnight/ds000224_R1.0.2_sub07-10/sub-MSC10/ses-func01/func/\n");
    printf("        returns /data/nil-bluearc/raichle/mcavoy/midnight/ds000224_R1.0.2_sub07-10/sub-MSC10/ses-func01/func\n");
    exit(-1);
    }
//cout << get_dir(argv[1]) << endl;
dir=get_dir(argv[1]);
*(dir+strlen(dir)-1)=0;
cout << dir << endl;
}
