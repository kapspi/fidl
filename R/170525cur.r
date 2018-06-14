library(rCUR)

p.vox=5765
p.sub=233
p.Af="../233subjectsXvox.4dfp.img"
#p.Af="../233subjectsXvox_double.4dfp.img"
p.n=0.05

Av=double(p.vox*p.sub)
Av=readBin(p.Af,double(),p.vox*p.sub,4)
#Av=readBin(p.Af,double(),p.vox*p.sub)

##A=matrix(Av,nrow=p.sub,ncol=p.vox,byrow=TRUE)
##Abig=matrix(Av,nrow=p.sub,ncol=p.vox,byrow=TRUE)
#Abig=matrix(Av,nrow=p.sub,ncol=p.vox,byrow=TRUE,dimnames=list(c(rep('HRP',36),rep('HRN',131),rep('LRN',66)),NULL))
#A=Abig[1:10,]
Abig=matrix(Av,nrow=p.vox,ncol=p.sub,dimnames=list(NULL,c(rep('HRP',36),rep('HRN',131),rep('LRN',66))))
#p0.sub=10
p0.sub=p.sub
A=Abig[,1:p0.sub]

q.n=round(p.vox*p.n)

#res=CUR(A,c=q.n)
#res=CUR(A,q.n,10)
res=CUR(A,r=q.n)

plotLeverage(res,C=FALSE,top.n=q.n,xlab='voxel',las=1,top.col='black',top.pch=16,ul.col='black',ul.lty=2,col='grey')

PCA01=prcomp(A,scale=TRUE)

#top.idx=topLeverage(res,C=TRUE,top.n=q.n,sort=FALSE)
#top.A=A[,top.idx]
top.idx=topLeverage(res,C=FALSE,top.n=q.n,sort=FALSE)
top.A=A[top.idx,]

PCA02=prcomp(top.A,scale=TRUE)

#PCAs=rbind(PCA01$rotation[c(1,2),],PCA02$rotation[c(1,2),])
PCAs=rbind(PCA01$rotation[,c(1,2)],PCA02$rotation[,c(1,2)])
group=rownames(PCAs)
pchs=ifelse(group=='HRP',16,0)
pchs=ifelse(group=='HRN',17,pchs)
pchs=ifelse(group=='LRN',18,pchs)
PCAs=data.frame(PC1=PCAs[,1],PC2=PCAs[,2],lab=c(rep('all',p0.sub),rep('selected',p0.sub)),group,pchs)
xyplot(PC2~PC1|lab,data=PCAs,pch=pchs,cex=1.0,col='black',fill='black',key=list(space="bottom",text=list(levels(PCAs$group)),
    points=TRUE,pch=c(16,17,18),fill='black',columns=3,just=1))

