/* Copyright 11/10/05 Washington University.  All Rights Reserved.
   subs_util.c  $Revision: 1.47 $*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "subs_util.h"
#include "shouldiswap.h"
#include "constants.h"
#include "ifh.h"
FILE *fopen_sub(char *filename,const char *mode)
{
    char *stringr="reading",*stringw="writing",*stringr2="read",*stringw2="write",

        //*stringr3="\nError: Perhaps the file does not exist or has been moved. Please check your directories.\n",
        //START160803
        *stringr3="\nfidlError: Perhaps the file does not exist or has been moved. Please check your directories.\n",

        *stringw3=" Are you out of memory?\n",*str_ptr,*str_ptr2,*str_ptr3;
    FILE *fp;
    if(strchr(mode,'r')) {
        str_ptr = stringr;
        str_ptr2 = stringr2;
        str_ptr3 = stringr3;
        }
    else {
        str_ptr = stringw;
        str_ptr2 = stringw2;
        str_ptr3 = stringw3;
        }
    if(!(fp=fopen(filename,mode))) {
        printf("fidlError: Could not open %s for %s.\n",filename,str_ptr);

        //printf("fidlError: Do you have %s permission in this directory?%s\n",str_ptr2,str_ptr3);
        //START160803
        printf("fidlError: Do you have %s permission in this directory?%s",str_ptr2,str_ptr3);

        }

    return fp;
}
int fwrite_sub(void *ptr,size_t size,size_t nitems,FILE *fp,int swapbytes){
    unsigned char *buf;
    size_t len;
    if(swapbytes) {
        if(!(buf=malloc(sizeof*buf*(len=size*nitems)))) {
            printf("fwrite_sub size=%zd nitems=%zd len=%zd\n",size,nitems,len);
            printf("Error: fwrite_sub Unable to malloc buf\n");
            return 0;
            }
        if(!memcpy(buf,ptr,len)) return 0;
        swap_bytes(buf,size,nitems);
        }
    else {
        buf = (unsigned char *)ptr;
        }
    if((len=fwrite(buf,size,nitems,fp))!=nitems){
        printf("Error: fwrite_sub failed. Wrote %zd  expecting to write %zd Abort!\n",len,nitems);
        printf("Error: Are you out of memory?\n");
        return 0;
        }
    if(swapbytes) free(buf);
    return 1;
    }
int writestack(char *filename,void *x,size_t size,size_t nitems,int swapbytes){
    FILE *fp;
    if(!(fp = fopen_sub(filename,"w"))) return 0;
    if(!(fwrite_sub(x,size,nitems,fp,swapbytes))) {
        printf("fidlError: Could not write to %s\n",filename);
        return 0;
        }
    fclose(fp);
    return 1;
    }
void swap_bytes(unsigned char *buf,size_t size,size_t nitems){
    unsigned char temp;
    int k;
    size_t i,j,len;
    len = size*nitems;
    for(i=0;i<len;i+=size) {
        for(k=size-1,j=0;j<size/2;j++,k--) {
            temp = buf[i+j];
            buf[i+j] = buf[i+k];
            buf[i+k] = temp;
            }
        }
    }

//START170505
void swap_bytes_idx(unsigned char *buf,size_t size,int lenbrain,int *brnidx){
    unsigned char temp,*p;
    int i,k;
    size_t j1;
    for(i=0;i<lenbrain;i++){
        p=&buf[brnidx[i]*size];
        for(k=(int)size-1,j1=0;j1<size/2;j1++,k--){
            temp = p[j1];
            p[j1] = p[k];
            p[k] = temp;
            }
        }
    }


int fread_sub(void *ptr,size_t size,size_t nitems,FILE *fp,int swapbytes,char *filename)
{
    unsigned char *buf;
    size_t nread;
    buf = (unsigned char *)ptr;
    if((nread=fread(buf,size,nitems,fp)) != nitems) {
        printf("feof(fp)=%d ferror(fp)=%d\n",feof(fp),ferror(fp));
        printf("Error: Please check %s\n",filename);
        if(feof(fp)) {
            printf("Error: File should have %zd voxels. Instead it has %zd.\n",nitems,nread);
            printf("Error: Do a more on the ifh. Multiply the voxel dimensions.\n");
            printf("Error: Do an ls -l on the img. Divide the size by 4.\n");
            printf("Error: If the above two numbers do not match, then fidl is not broken. Your file is. Abort!\n");
            }
        else if(ferror(fp)) {
            printf("Error: File is corrupt. Abort!\n");
            }
        else {
            printf("Error: Unknown problem with file. Abort!\n");
            }
        return 0;
        }
    if(swapbytes) swap_bytes(buf,size,nitems);
    return 1;
}
int readstack(char *infile,void *x,size_t size,size_t nitems,int SunOS_Linux,int bigendian)
{                                                                            /* -1 to read_ifh */
    char filename[MAXNAME];
    int swapbytes,bigendian1;
    FILE *fp;
    Interfile_header *ifh;
    strcpy(filename,infile);
    *strrchr(filename,'.') = 0;
    strcat(filename,".img");
    if((bigendian1=bigendian)==-1) {
        if(!(ifh=read_ifh(filename,(Interfile_header*)NULL))) return 0;
        bigendian1=ifh->bigendian;
        free_ifh(ifh,0);
        }
    swapbytes=shouldiswap(SunOS_Linux,bigendian1);
    if(!(fp=fopen_sub(filename,"r"))) return 0;
    if(!(fread_sub(x,size,nitems,fp,swapbytes,infile))) return 0;
    fclose(fp);
    return 1;
}
