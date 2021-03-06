# R
library(rCUR)

p.vox=5803
p.sub=102
p.Af="../102subjectsXvox.4dfp.img"

Av=double(p.vox*p.sub)
Av=readBin(p.Af,double(),p.vox*p.sub,4)
A=matrix(Av,nrow=p.vox,ncol=p.sub,dimnames=list(NULL,c(rep('HRP',36),rep('LRN',66))))
#AT=matrix(Av,nrow=p.sub,ncol=p.vox,dimnames=list(c(rep('HRP',36),rep('LRN',66)),NULL),byrow=TRUE)

PCA01=prcomp(A)
PCA01.rot=PCA01$rotation[,c(1,2)]
group=rownames(PCA01.rot)
pchs=ifelse(group=='HRP',16,0)
pchs=ifelse(group=='HRN',17,pchs)
pchs=ifelse(group=='LRN',18,pchs)
cols=ifelse(group=='HRP','blue',0)
cols=ifelse(group=='HRN','red',cols)
cols=ifelse(group=='LRN','green',cols)
dev.new()
PCAs=data.frame(PC1=PCA01.rot[,1],PC2=PCA01.rot[,2],group,pchs)
print(xyplot(PC2~PC1,data=PCAs,pch=pchs,col=cols,main="PCA no scaling"))

PCA01=prcomp(A,scale=TRUE)
PCA01.rot=PCA01$rotation[,c(1,2)]
group=rownames(PCA01.rot)
pchs=ifelse(group=='HRP',16,0)
pchs=ifelse(group=='HRN',17,pchs)
pchs=ifelse(group=='LRN',18,pchs)
cols=ifelse(group=='HRP','blue',0)
cols=ifelse(group=='HRN','red',cols)
cols=ifelse(group=='LRN','green',cols)
dev.new()
PCAs=data.frame(PC1=PCA01.rot[,1],PC2=PCA01.rot[,2],group,pchs)
print(xyplot(PC2~PC1,data=PCAs,pch=pchs,col=cols,main="PCA with scaling"))

p.n=0.3
q.n=round(p.vox*p.n)
res=CUR(A,r=q.n)
dev.new()
plotLeverage(res,C=FALSE,top.n=q.n,xlab='voxel',las=1,top.col='black',top.pch=16,ul.col='black',ul.lty=2,col='grey')
top.idx=topLeverage(res,C=FALSE,top.n=q.n,sort=FALSE)
top.A=A[top.idx,]
PCA02=prcomp(top.A,scale=TRUE)
PCA02.rot=PCA02$rotation[,c(1,2)]
group=rownames(PCA02.rot)
pchs=ifelse(group=='HRP',16,0)
pchs=ifelse(group=='HRN',17,pchs)
pchs=ifelse(group=='LRN',18,pchs)
cols=ifelse(group=='HRP','blue',0)
cols=ifelse(group=='HRN','red',cols)
cols=ifelse(group=='LRN','green',cols)
dev.new()
PCAs=data.frame(PC1=PCA02.rot[,1],PC2=PCA02.rot[,2],group,pchs)
title=c("PCA with scaling",q.n,"topLeverage voxels")
print(xyplot(PC2~PC1,data=PCAs,pch=pchs,col=cols,main=title))
PCA02=prcomp(top.A)
PCA02.rot=PCA02$rotation[,c(1,2)]
group=rownames(PCA02.rot)
pchs=ifelse(group=='HRP',16,0)
pchs=ifelse(group=='HRN',17,pchs)
pchs=ifelse(group=='LRN',18,pchs)
cols=ifelse(group=='HRP','blue',0)
cols=ifelse(group=='HRN','red',cols)
cols=ifelse(group=='LRN','green',cols)
dev.new()
PCAs=data.frame(PC1=PCA02.rot[,1],PC2=PCA02.rot[,2],group,pchs)
title=c("PCA no scaling",q.n,"topLeverage voxels")
print(xyplot(PC2~PC1,data=PCAs,pch=pchs,col=cols,main=title))

#p.n=0.2
#q.n=round(p.vox*p.n)
#res=CUR(A,r=q.n)
#dev.new()
#plotLeverage(res,C=FALSE,top.n=q.n,xlab='voxel',las=1,top.col='black',top.pch=16,ul.col='black',ul.lty=2,col='grey')
#top.idx=topLeverage(res,C=FALSE,top.n=q.n,sort=FALSE)
#top.A=A[top.idx,]
#PCA02=prcomp(top.A,scale=TRUE)
#PCA02.rot=PCA02$rotation[,c(1,2)]
#group=rownames(PCA02.rot)
#pchs=ifelse(group=='HRP',16,0)
#pchs=ifelse(group=='HRN',17,pchs)
#pchs=ifelse(group=='LRN',18,pchs)
#cols=ifelse(group=='HRP','blue',0)
#cols=ifelse(group=='HRN','red',cols)
#cols=ifelse(group=='LRN','green',cols)
#dev.new()
#PCAs=data.frame(PC1=PCA02.rot[,1],PC2=PCA02.rot[,2],group,pchs)
#title=c("PCA with scaling",q.n,"topLeverage voxels")
#print(xyplot(PC2~PC1,data=PCAs,pch=pchs,col=cols,main=title))
#PCA02=prcomp(top.A)
#PCA02.rot=PCA02$rotation[,c(1,2)]
#group=rownames(PCA02.rot)
#pchs=ifelse(group=='HRP',16,0)
#pchs=ifelse(group=='HRN',17,pchs)
#pchs=ifelse(group=='LRN',18,pchs)
#cols=ifelse(group=='HRP','blue',0)
#cols=ifelse(group=='HRN','red',cols)
#cols=ifelse(group=='LRN','green',cols)
#dev.new()
#PCAs=data.frame(PC1=PCA02.rot[,1],PC2=PCA02.rot[,2],group,pchs)
#title=c("PCA no scaling",q.n,"topLeverage voxels")
#print(xyplot(PC2~PC1,data=PCAs,pch=pchs,col=cols,main=title))

#p.n=0.1
#q.n=round(p.vox*p.n)
#res=CUR(A,r=q.n)
#dev.new()
#plotLeverage(res,C=FALSE,top.n=q.n,xlab='voxel',las=1,top.col='black',top.pch=16,ul.col='black',ul.lty=2,col='grey')
#top.idx=topLeverage(res,C=FALSE,top.n=q.n,sort=FALSE)
#top.A=A[top.idx,]
#PCA02=prcomp(top.A,scale=TRUE)
#PCA02.rot=PCA02$rotation[,c(1,2)]
#group=rownames(PCA02.rot)
#pchs=ifelse(group=='HRP',16,0)
#pchs=ifelse(group=='HRN',17,pchs)
#pchs=ifelse(group=='LRN',18,pchs)
#cols=ifelse(group=='HRP','blue',0)
#cols=ifelse(group=='HRN','red',cols)
#cols=ifelse(group=='LRN','green',cols)
#dev.new()
#PCAs=data.frame(PC1=PCA02.rot[,1],PC2=PCA02.rot[,2],group,pchs)
#title=c("PCA with scaling",q.n,"topLeverage voxels")
#print(xyplot(PC2~PC1,data=PCAs,pch=pchs,col=cols,main=title))
#PCA02=prcomp(top.A)
#PCA02.rot=PCA02$rotation[,c(1,2)]
#group=rownames(PCA02.rot)
#pchs=ifelse(group=='HRP',16,0)
#pchs=ifelse(group=='HRN',17,pchs)
#pchs=ifelse(group=='LRN',18,pchs)
#cols=ifelse(group=='HRP','blue',0)
#cols=ifelse(group=='HRN','red',cols)
#cols=ifelse(group=='LRN','green',cols)
#dev.new()
#PCAs=data.frame(PC1=PCA02.rot[,1],PC2=PCA02.rot[,2],group,pchs)
#title=c("PCA no scaling",q.n,"topLeverage voxels")
#print(xyplot(PC2~PC1,data=PCAs,pch=pchs,col=cols,main=title))

#p.n=0.05
#q.n=round(p.vox*p.n)
#res=CUR(A,r=q.n)
#dev.new()
#plotLeverage(res,C=FALSE,top.n=q.n,xlab='voxel',las=1,top.col='black',top.pch=16,ul.col='black',ul.lty=2,col='grey')
#top.idx=topLeverage(res,C=FALSE,top.n=q.n,sort=FALSE)
#top.A=A[top.idx,]
#PCA02=prcomp(top.A,scale=TRUE)
#PCA02.rot=PCA02$rotation[,c(1,2)]
#group=rownames(PCA02.rot)
#pchs=ifelse(group=='HRP',16,0)
#pchs=ifelse(group=='HRN',17,pchs)
#pchs=ifelse(group=='LRN',18,pchs)
#cols=ifelse(group=='HRP','blue',0)
#cols=ifelse(group=='HRN','red',cols)
#cols=ifelse(group=='LRN','green',cols)
#dev.new()
#PCAs=data.frame(PC1=PCA02.rot[,1],PC2=PCA02.rot[,2],group,pchs)
#title=c("PCA with scaling",q.n,"topLeverage voxels")
#print(xyplot(PC2~PC1,data=PCAs,pch=pchs,col=cols,main=title))
#PCA02=prcomp(top.A)
#PCA02.rot=PCA02$rotation[,c(1,2)]
#group=rownames(PCA02.rot)
#pchs=ifelse(group=='HRP',16,0)
#pchs=ifelse(group=='HRN',17,pchs)
#pchs=ifelse(group=='LRN',18,pchs)
#cols=ifelse(group=='HRP','blue',0)
#cols=ifelse(group=='HRN','red',cols)
#cols=ifelse(group=='LRN','green',cols)
#dev.new()
#PCAs=data.frame(PC1=PCA02.rot[,1],PC2=PCA02.rot[,2],group,pchs)
#title=c("PCA no scaling",q.n,"topLeverage voxels")
#print(xyplot(PC2~PC1,data=PCAs,pch=pchs,col=cols,main=title))

p.n=0.01
q.n=round(p.vox*p.n)
res=CUR(A,r=q.n)
dev.new()
plotLeverage(res,C=FALSE,top.n=q.n,xlab='voxel',las=1,top.col='black',top.pch=16,ul.col='black',ul.lty=2,col='grey')
top.idx=topLeverage(res,C=FALSE,top.n=q.n,sort=FALSE)
top.A=A[top.idx,]
PCA02=prcomp(top.A,scale=TRUE)
PCA02.rot=PCA02$rotation[,c(1,2)]
group=rownames(PCA02.rot)
pchs=ifelse(group=='HRP',16,0)
pchs=ifelse(group=='HRN',17,pchs)
pchs=ifelse(group=='LRN',18,pchs)
cols=ifelse(group=='HRP','blue',0)
cols=ifelse(group=='HRN','red',cols)
cols=ifelse(group=='LRN','green',cols)
dev.new()
PCAs=data.frame(PC1=PCA02.rot[,1],PC2=PCA02.rot[,2],group,pchs)
title=c("PCA with scaling",q.n,"topLeverage voxels")
print(xyplot(PC2~PC1,data=PCAs,pch=pchs,col=cols,main=title))
PCA02=prcomp(top.A)
PCA02.rot=PCA02$rotation[,c(1,2)]
group=rownames(PCA02.rot)
pchs=ifelse(group=='HRP',16,0)
pchs=ifelse(group=='HRN',17,pchs)
pchs=ifelse(group=='LRN',18,pchs)
cols=ifelse(group=='HRP','blue',0)
cols=ifelse(group=='HRN','red',cols)
cols=ifelse(group=='LRN','green',cols)
dev.new()
PCAs=data.frame(PC1=PCA02.rot[,1],PC2=PCA02.rot[,2],group,pchs)
title=c("PCA no scaling",q.n,"topLeverage voxels")
print(xyplot(PC2~PC1,data=PCAs,pch=pchs,col=cols,main=title))

top.AT=t(top.A)
PCA02=prcomp(top.AT,scale=TRUE,retx=TRUE)
PCA02.rot=PCA02$x[,c(1,2)]
group=rownames(PCA02.rot)
pchs=ifelse(group=='HRP',16,0)
pchs=ifelse(group=='HRN',17,pchs)
pchs=ifelse(group=='LRN',18,pchs)
cols=ifelse(group=='HRP','blue',0)
cols=ifelse(group=='HRN','red',cols)
cols=ifelse(group=='LRN','green',cols)
dev.new()
PCAs=data.frame(X1=PCA02.rot[,1],X2=PCA02.rot[,2],group,pchs)
title=c("PCA with scaling",q.n,"topLeverage voxels")
print(xyplot(X2~X1,data=PCAs,pch=pchs,col=cols,main=title))

top.AT=t(top.A)
PCA02=prcomp(top.AT,retx=TRUE)
PCA02.rot=PCA02$x[,c(1,2)]
group=rownames(PCA02.rot)
pchs=ifelse(group=='HRP',16,0)
pchs=ifelse(group=='HRN',17,pchs)
pchs=ifelse(group=='LRN',18,pchs)
cols=ifelse(group=='HRP','blue',0)
cols=ifelse(group=='HRN','red',cols)
cols=ifelse(group=='LRN','green',cols)
dev.new()
PCAs=data.frame(X1=PCA02.rot[,1],X2=PCA02.rot[,2],group,pchs)
title=c("PCA no scaling",q.n,"topLeverage voxels")
print(xyplot(X2~X1,data=PCAs,pch=pchs,col=cols,main=title))
