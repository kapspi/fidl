;Copyright 12/31/99 Washington University.  All Rights Reserved.
;get_image.pro  $Revision: 12.99 $

;function get_image,index,fi,stc,FILNUM=filnum,SUBTRACT_TREND=subtract_trend
;START160915
function get_image,index,fi,stc,plane,frame,FILNUM=filnum,SUBTRACT_TREND=subtract_trend

if keyword_set(filnum) then filnum = filnum - 1 else filnum = fi.n
hdr = *fi.hdr_ptr[filnum]
if hdr.array_type eq !STITCH_ARRAY then begin
    t_to_file = *stc[filnum].t_to_file
    tdim_sum = *stc[filnum].tdim_sum
    filnam = *stc[filnum].filnam
    t_total = index/hdr.zdim       
    i_file = t_to_file[t_total]
    i1 = index - tdim_sum[i_file]*hdr.zdim
    if strpos(filnam[i_file],!SLASH) lt 0 then file_name=fi.paths[filnum]+!SLASH+filnam[i_file] else file_name=filnam[i_file]
    filetype=*stc[filnum].filetype
    if filetype[i_file] eq 'img' then begin
        openr,lun,file_name,/GET_LUN
        if hdr.type eq !FLOAT then $
            img = assoc(lun,fltarr(hdr.xdim,hdr.ydim,/NOZERO),0) $
        else $
            img = assoc(lun,dblarr(hdr.xdim,hdr.ydim,/NOZERO),0)
        image = img[*,*,i1]
        close,lun
        free_lun,lun
    endif else if filetype[i_file] eq 'dtseries' or filetype[i_file] eq 'dscalar' then begin
        image=fltarr(hdr.xdim,hdr.ydim)
        t = long64(t_total - tdim_sum[i_file])
        stat=0L
        stat=call_external(!SHARE_LIBCIFTI,'_cifti_getColumn',file_name,t,image,VALUE=[1,1,0])
        if stat eq 0L then begin
            print,'Error in _cifti_getColumn'
        endif
        ;print,'get_image size(image)=',size(image)
    endif else if filetype[i_file] eq 'nii' then begin 
        z = long64(index - t_total*hdr.zdim)
        t = long64(t_total - tdim_sum[i_file])
        image=fltarr(hdr.xdim,hdr.ydim)
        stat=0L
        stat=call_external(!SHARE_LIBCIFTI,'_nifti_getslice',file_name,z,t,image,VALUE=[1,1,1,0])
        if stat eq 0L then print,'Error in _nifti_getslice'

        ;print,'get_image hdr.ifh.cf_flip=',trim(hdr.ifh.cf_flip),' hdr.xdim=',trim(hdr.xdim),' hdr.ydim=',trim(hdr.ydim), $
        ;    ' hdr.zdim=',trim(hdr.zdim),' hdr.tdim=',trim(hdr.tdim)

        ;stat=call_external(!SHARE_LIBCIFTI,'_nifti_flipslice',hdr.xdim,hdr.ydim,hdr.ifh.cf_flip,image,VALUE=[1,1,1,0])
        ;START170411
        if hdr.ifh.cf_flip ne 52 then $
            stat=call_external(!SHARE_LIBCIFTI,'_nifti_flipslice',hdr.xdim,hdr.ydim,hdr.ifh.cf_flip,image,VALUE=[1,1,1,0])

    endif else begin
        print,'Unknown filetype'
        image=0.
    endelse
endif else begin
    if not ptr_valid(fi.data_ptr[filnum]) then return,-1
    img = *fi.data_ptr[filnum]


    ;if hdr.ifh.glm_boldtype eq 'cifti' then begin
    ;    image=fltarr(hdr.xdim*hdr.xdim)
    ;    image[0:hdr.ifh.matrix_size_1-1]=img[*,index]
    ;    image=reform(image,hdr.xdim,hdr.xdim,/OVERWRITE)
    ;endif else begin
    ;    image = img[*,*,index]
    ;    if hdr.ifh.glm_boldtype eq 'nifti' then begin
    ;        stat=0L
    ;        stat=call_external(!SHARE_LIBCIFTI,'_nifti_flipslice',hdr.xdim,hdr.ydim,hdr.ifh.cf_flip,image,VALUE=[1,1,1,0])
    ;    endif
    ;endelse
    ;START160914
    if hdr.ifh.glm_boldtype eq 'cifti' then begin
        image=fltarr(hdr.xdim*hdr.xdim)
        image[0:hdr.ifh.matrix_size_1-1]=img[*,index]
        image=reform(image,hdr.xdim,hdr.xdim,/OVERWRITE)
    endif else if hdr.array_type eq !LINEAR_MODEL and hdr.ifh.glm_masked eq 1 then begin

        ;START160915
        image=fltarr(hdr.xdim*hdr.ydim*hdr.zdim)
        image[hdr.uval]=img[*,frame]
        image=reform(image,hdr.xdim,hdr.ydim,hdr.zdim,/OVERWRITE)
        image=image[*,*,plane]

        ;print,'get_image hdr.uval=',hdr.uval
        ;print,'get_image size(hdr.uval)=',size(hdr.uval),' hdr.array_type=',hdr.array_type,' frame=',frame,' plane=',plane

    endif else begin
        image = img[*,*,index]

        ;if hdr.ifh.glm_boldtype eq 'nifti' then begin
        ;START170324
        if hdr.ifh.glm_boldtype eq 'nifti' and hdr.ifh.cf_flip ne 52 then begin

            stat=0L
            stat=call_external(!SHARE_LIBCIFTI,'_nifti_flipslice',hdr.xdim,hdr.ydim,hdr.ifh.cf_flip,image,VALUE=[1,1,1,0])
        endif
    endelse



endelse
if !VERSION.OS_FAMILY eq 'Windows' then begin
    if size(img,/FILE_LUN) ne 0 then image = swap_endian(image)
endif else if shouldiswap(hdr.ifh.bigendian) then begin 
    if size(img,/FILE_LUN) ne 0 then image = swap_endian(image)
endif
if hdr.scl ne 1. then image = hdr.scl*image
sz = size(image)
if(sz[n_elements(sz)-2] eq 6) then image = abs(image) ;Complex data.

;print,'get_image bottom'

return,image
end
