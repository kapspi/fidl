;Copyright 12/31/99 Washington University.  All Rights Reserved.
;read_header.pro  $Revision: 12.106 $

;function read_header,filename,filetype
;START180430
function read_header,filename,filetype,quiet
if n_elements(quite) eq 0 then quiet=1

;if filetype eq 'img' then begin
;START170703
if filetype eq 'img' or filetype eq 'glm' then begin

    ifh = read_mri_ifh(filename)
    if ifh.msg ne 'OK' then return,rtn={msg:ifh.msg}
    space=get_space(ifh.matrix_size_1,ifh.matrix_size_2,ifh.matrix_size_3)
    if space eq !SPACE_MNI222 or space eq !SPACE_MNI333 then begin
        c_orient=lonarr(3)
        stat=call_external(!SHARE_LIBCIFTI,'_nifti_getorient',ifh.center,ifh.mmppix,c_orient,VALUE=[0,0,0])
        if stat eq 0L then begin
            print,'Error in _nifti_getorient
            return,rtn={msg:'ERROR'}
        endif
        ifh.c_orient=ptr_new(c_orient)
        ifh.cf_flip=long(strjoin(trim(c_orient),/SINGLE))
    endif
    ;print,filename
    ;if ptr_valid(ifh.c_orient) then $ 
    ;    print,'    read_header *ifh.c_orient=',*ifh.c_orient,' ifh.cf_flip=',ifh.cf_flip $
    ;else $
    ;    print,'    read_header ifh.cf_flip=',ifh.cf_flip
endif else if filetype eq 'dtseries' or filetype eq 'dscalar' then begin 
    rowsize=0LL
    colsize=0LL
    stat=0L
    stat=call_external(!SHARE_LIBCIFTI,'_cifti_getDimensions',filename,rowsize,colsize,VALUE=[1,0,0])
    if stat eq 0L then begin
        print,'Error in _cifti_getDimensions'
        return,rtn={msg:'EXIT'}
    endif
    ifh = {InterFile_Header}
    ifh.matrix_size_1=long(colsize)
    ifh.matrix_size_2=1L
    ifh.matrix_size_3=1L
    ifh.matrix_size_4=long(rowsize)
    ifh.scale_1=1.
    ifh.scale_3=1.
endif else if filetype eq 'nii' then begin 


    ;xdim=0LL
    ;ydim=0LL
    ;zdim=0LL
    ;tdim=0LL
    ;c_orient=lonarr(3)
    ;ifh = {InterFile_Header}
    ;stat=call_external(!SHARE_LIBCIFTI,'_nifti_gethdr',filename,xdim,ydim,zdim,tdim,ifh.center,ifh.mmppix,c_orient, $ 
    ;    VALUE=[1,0,0,0,0,0,0,0])
    ;if stat eq 0L then begin
    ;    print,'Error in _nifti_gethdr'
    ;    return,rtn={msg:'EXIT'}
    ;endif
    ;ifh.matrix_size_1=long(xdim)
    ;ifh.matrix_size_2=long(ydim)
    ;ifh.matrix_size_3=long(zdim)
    ;ifh.matrix_size_4=long(tdim)
    ;ifh.scale_1=abs(ifh.mmppix[0])
    ;ifh.scale_3=abs(ifh.mmppix[2])
    ;ifh.c_orient=ptr_new(c_orient)
    ;ifh.cf_flip=long(strjoin(trim(c_orient),/SINGLE))
    ;undefine,xdim,ydim,zdim,tdim
    ;START180119
    xdim=0LL
    ydim=0LL
    zdim=0LL
    tdim=0LL
    center=fltarr(3)
    mmppix=fltarr(3)
    c_orient=lonarr(3)
    ifh = {InterFile_Header}
    stat=call_external(!SHARE_LIBCIFTI,'_nifti_gethdr',filename,xdim,ydim,zdim,tdim,center,mmppix,c_orient, $
        VALUE=[1,0,0,0,0,0,0,0])
    if stat eq 0L then begin
        print,'Error in _nifti_gethdr'
        return,rtn={msg:'EXIT'}
    endif
    ifh.matrix_size_1=long(xdim)
    ifh.matrix_size_2=long(ydim)
    ifh.matrix_size_3=long(zdim)
    ifh.matrix_size_4=long(tdim)
    ifh.scale_1=abs(mmppix[0])
    ifh.scale_3=abs(mmppix[2])
    ifh.center=center
    ifh.mmppix=mmppix
    ifh.c_orient=ptr_new(c_orient)
    ifh.cf_flip=long(strjoin(trim(c_orient),/SINGLE))
    undefine,mmppix,center,tdim,zdim,ydim,xdim


    ;print,'read_header '+filename
    ;print,'            ifh.cf_flip=',ifh.cf_flip 
    ;print,'            ifh.center=',ifh.center
    ;print,'            ifh.mmppix=',ifh.mmppix
    ;START180430
    if quiet eq 0 then begin
        print,'read_header '+filename
        print,'            ifh.cf_flip=',ifh.cf_flip 
        print,'            ifh.center=',ifh.center
        print,'            ifh.mmppix=',ifh.mmppix
    endif


endif else begin
    lizard=filename+string(10B)+'File type:'+filetype+' Unknown file type!'
    print,lizard
    scrap = get_button(['Go back','Exit'],TITLE=lizard,BASE_TITLE='Please select') 
    if scrap eq 0 then return,rtn={msg:'GOBACK'} else return,rtn={msg:'EXIT'}
endelse
return,rtn={msg:'OK',ifh:ifh}
end
