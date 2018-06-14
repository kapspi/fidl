;Copyright 12/31/99 Washington University.  All Rights Reserved.
;print_stats.pro  $Revision: 12.85 $

;**************************************
pro print_stats,dsp_image,dsp,wd,fi,stc
;**************************************

min = min(dsp_image(*,*))
max = max(dsp_image(*,*))
sum = total(dsp_image(*,*))
mean = sum/(dsp[fi.cw].xdim*dsp[fi.cw].ydim)
sdim = strcompress(string(dsp[fi.cw].xdim,dsp[fi.cw].ydim,FORMAT='(i4,"x",i4)'),/REMOVE_ALL)
print,'File: ',fi.tails[fi.n],'; ',sdim
out = strcompress(string('Pln: ',dsp[fi.cw].plane,' Frm: ',dsp[fi.cw].frame,'  Min: ',min,' Max: ',max,' Mean: ',mean))
print,out
widget_control,wd.error,SET_VALUE=out
img = fltarr(dsp[fi.cw].xdim,dsp[fi.cw].ydim,dsp[fi.cw].zdim)
for z=0,dsp[fi.cw].zdim-1 do img(*,*,z) = get_image(dsp[fi.cw].zdim*(dsp[fi.cw].frame-1) + z,fi,stc)
min = min(img,MAX=max)
sum = total(img)
mean = sum/dsp[fi.cw].xdim/dsp[fi.cw].ydim/dsp[fi.cw].zdim
non_zero = total(img ne 0.)
out = strcompress(string('Volume stats: ',' Frm: ',dsp[fi.cw].frame,'  Min: ',min,' Max: ',max,' Mean: ',mean,' Nonzero: ',non_zero))
print,out

index = where(abs(img) gt float(!UNSAMPLED_VOXEL),count)

;if count ne 0 then print,'min nonzero =',min(img[index])
;START180424
if count ne 0 then print,'UNSAMPLED: min = ',min(img[index]),' max = ',max(img[index]),' nonzero = ',count

end
