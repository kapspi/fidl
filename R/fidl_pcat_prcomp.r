#!/usr/bin/env Rscript

#This is an example of an Rscript written by fidl_pcat.c.
#It was used internally as a check for the C code.
#The data consists of 134755 voxels with four stages of non-REM sleep: wake, N1, N2, N3.
#The principal component analysis is performed by prcomp. 
#Principal components are returned in *rotation* and the projected data in *transpose*. 

#parameters
p.ifile="fidlrdata170502:15:41:11:313637.dat"
p.byrow=TRUE
p.row=134755
p.col=4
p.ofile0="fidlrdata170502:15:41:11:313637_rotation.4dfp.img"
p.ofile1="fidlrdata170502:15:41:11:313637_transpose.4dfp.img"

#read in data
Av=double(p.row*p.col)
Av=readBin(p.ifile,double(),p.row*p.col)
A=matrix(Av,nrow=p.row,ncol=p.col,byrow=p.byrow)

#principal component analysis
PCA01=prcomp(A,retx=TRUE)

#output eigenvectors
fp=file(p.ofile0,"wb")
writeBin(as.vector(PCA01$rotation),fp)
close(fp)

#output projected data
fp=file(p.ofile1,"wb")
writeBin(as.vector(PCA01$x),fp)
close(fp)
