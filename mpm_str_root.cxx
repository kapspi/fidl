/* Copyright 11/13/17 Washington University.  All Rights Reserved.
   mpm_str_root.cxx  $Revision: 1.1 $ */
#include <iostream>
#include "fidl.h"
using namespace std;
int main(int argc,char **argv)
{
if(argc!=2){
    printf("mpm_str_root <string>\n");
    printf("    Ex. mpm_str_root /data/nil-bluearc/raichle/mcavoy/midnight/ds000224_R1.0.2_sub07-10/sub-MSC10/ses-func01/func/sub-MSC10_ses-func01_task-rest_bold.nii.gz\n");
    printf("        returns sub-MSC10_ses-func01_task-rest_bold\n");
    exit(-1);
    }
cout << get_tail_sans_ext(argv[1]) << endl;
}
