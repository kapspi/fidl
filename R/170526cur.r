# R
library(rCUR)
library(scatterplot3d)

p.vox=5765
p.sub=233
p.Af="../233subjectsXvox.4dfp.img"

Av=double(p.vox*p.sub)
Av=readBin(p.Af,double(),p.vox*p.sub,4)
A=matrix(Av,nrow=p.vox,ncol=p.sub,dimnames=list(NULL,c(rep('HRP',36),rep('HRN',131),rep('LRN',66))))

##p.n=0.05
##q.n=round(p.vox*p.n)
##res=CUR(A,r=q.n)
##plotLeverage(res,C=FALSE,top.n=q.n,xlab='voxel',las=1,top.col='black',top.pch=16,ul.col='black',ul.lty=2,col='grey')
##top.idx=topLeverage(res,C=FALSE,top.n=q.n,sort=FALSE)
##top.A=A[top.idx,]
##PCA02=prcomp(top.A,scale=TRUE)
##PCA01=prcomp(A,scale=TRUE)
##PCAs=rbind(PCA01$rotation[,c(1,2)],PCA02$rotation[,c(1,2)])
##group=rownames(PCAs)
##pchs=ifelse(group=='HRP',16,0)
##pchs=ifelse(group=='HRN',17,pchs)
##pchs=ifelse(group=='LRN',18,pchs)
###PCAs=data.frame(PC1=PCAs[,1],PC2=PCAs[,2],lab=c(rep('all',p.sub),rep('selected',p.sub)),group,pchs)
##dev.new()
##print(xyplot(PC2~PC1|lab,data=PCAs,pch=pchs,cex=1.0,col='black',fill='black',key=list(space="bottom",
##    text=list(levels(PCAs$group)),points=TRUE,pch=c(16,17,18),fill='black',columns=3,just=1)))

#PCA01=prcomp(A,scale=TRUE)
#PCA01.rot=PCA01$rotation[,c(1,2,3)]
#group=rownames(PCA01.rot)
#pchs=ifelse(group=='HRP',16,0)
#pchs=ifelse(group=='HRN',17,pchs)
#pchs=ifelse(group=='LRN',18,pchs)
#cols=ifelse(group=='HRP','blue',0)
#cols=ifelse(group=='HRN','red',cols)
#cols=ifelse(group=='LRN','green',cols)
#dev.new()
#scatterplot3d(PCA01.rot,pch=pchs,color=cols)

p.n=0.1
q.n=round(p.vox*p.n)
#res=CUR(A,r=q.n)
res=CUR(A,c=p.sub,r=q.n,method="ortho.top.scores",alpha=2)
dev.new()
plotLeverage(res,C=FALSE,top.n=q.n,xlab='voxel',las=1,top.col='black',top.pch=16,ul.col='black',ul.lty=2,col='grey')
#top.idx=topLeverage(res,C=FALSE,top.n=q.n,sort=FALSE)
#top.A=A[top.idx,]
top.A=getR(res)
PCA02=prcomp(top.A,scale=TRUE)
PCA02.rot=PCA02$rotation[,c(1,2,3)]
group=rownames(PCA02.rot)
pchs=ifelse(group=='HRP',16,0)
pchs=ifelse(group=='HRN',17,pchs)
pchs=ifelse(group=='LRN',18,pchs)
cols=ifelse(group=='HRP','blue',0)
cols=ifelse(group=='HRN','red',cols)
cols=ifelse(group=='LRN','green',cols)
dev.new()
scatterplot3d(PCA02.rot,pch=pchs,color=cols)

p.n=0.05
q.n=round(p.vox*p.n)
#res=CUR(A,r=q.n)
res=CUR(A,c=p.sub,r=q.n,method="ortho.top.scores",alpha=2)
dev.new()
plotLeverage(res,C=FALSE,top.n=q.n,xlab='voxel',las=1,top.col='black',top.pch=16,ul.col='black',ul.lty=2,col='grey')
#top.idx=topLeverage(res,C=FALSE,top.n=q.n,sort=FALSE)
#top.A=A[top.idx,]
top.A=getR(res)
PCA02=prcomp(top.A,scale=TRUE)
PCA02.rot=PCA02$rotation[,c(1,2,3)]
group=rownames(PCA02.rot)
pchs=ifelse(group=='HRP',16,0)
pchs=ifelse(group=='HRN',17,pchs)
pchs=ifelse(group=='LRN',18,pchs)
cols=ifelse(group=='HRP','blue',0)
cols=ifelse(group=='HRN','red',cols)
cols=ifelse(group=='LRN','green',cols)
dev.new()
scatterplot3d(PCA02.rot,pch=pchs,color=cols)

p.n=0.01
q.n=round(p.vox*p.n)
#res=CUR(A,r=q.n)
res=CUR(A,c=p.sub,r=q.n,method="ortho.top.scores",alpha=2)
dev.new()
plotLeverage(res,C=FALSE,top.n=q.n,xlab='voxel',las=1,top.col='black',top.pch=16,ul.col='black',ul.lty=2,col='grey')
#top.idx=topLeverage(res,C=FALSE,top.n=q.n,sort=FALSE)
#top.A=A[top.idx,]
top.A=getR(res)
PCA02=prcomp(top.A,scale=TRUE)
PCA02.rot=PCA02$rotation[,c(1,2,3)]
group=rownames(PCA02.rot)
pchs=ifelse(group=='HRP',16,0)
pchs=ifelse(group=='HRN',17,pchs)
pchs=ifelse(group=='LRN',18,pchs)
cols=ifelse(group=='HRP','blue',0)
cols=ifelse(group=='HRN','red',cols)
cols=ifelse(group=='LRN','green',cols)
dev.new()
scatterplot3d(PCA02.rot,pch=pchs,color=cols)

