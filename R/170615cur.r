# R
library(rCUR)
library(scatterplot3d)

p.vox=5765
p.sub=233
p.Af="../233subjectsXvox.4dfp.img"

Av=double(p.vox*p.sub)
Av=readBin(p.Af,double(),p.vox*p.sub,4)
A=matrix(Av,nrow=p.vox,ncol=p.sub,dimnames=list(NULL,c(rep('HRP',36),rep('HRN',131),rep('LRN',66))))

PCA01=prcomp(A)
PCA01.rot=PCA01$rotation[,c(1,2,3)]
group=rownames(PCA01.rot)
pchs=ifelse(group=='HRP',16,0)
pchs=ifelse(group=='HRN',17,pchs)
pchs=ifelse(group=='LRN',18,pchs)
cols=ifelse(group=='HRP','blue',0)
cols=ifelse(group=='HRN','green',cols)
cols=ifelse(group=='LRN','red',cols)
dev.new()
scatterplot3d(PCA01.rot,pch=pchs,color=cols)
