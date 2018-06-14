;Copyright 12/31/99 Washington University.  All Rights Reserved.
;load_linmod.pro  $Revision: 12.141 $
function load_linmod,fi,dsp,wd,glm,help,ifh,FILENAME=filename,GET_CNORM=get_cnorm,DONT_PUT_IMAGE=dont_put_image,$
    STOPATCNORM=stopatcnorm,STOPATFSTAT=stopatfstat
if keyword_set(FILENAME) then begin
    filename = filename
    lcbatch = !TRUE
endif else begin
    get_dialog_pickfile,fi.glm_filter,fi.path,'Please select GLM.',filename,rtn_nfiles,rtn_path
    if filename eq 'EXIT' or filename eq 'GOBACK' then return,rtn={msg:filename}
    fi.path = rtn_path
    lcbatch = !FALSE
endelse
if !BATCH eq !FALSE then widget_control,/HOURGLASS
xdim = 0L
ydim = 0L
zdim = 0L
xdimdisp=0L
ydimdisp=0L
zdimdisp=0L
tdim = 0L
dxdy = 0.
dz = 0.
t_valid = 0L
df = 0.
nc = 0L
N = 0L
rev = 0
dum = 0
n_interest = 0L
period = 0L
num_trials = 0L
tot_eff = 0L
TR = 0.
n_files = 0L
funclen = 1
functype = !BOYNTON
W_smoothness = 0.
fwhmx = 0.
fwhmy = 0.
fwhmz = 0.
effect_TR = 0.
effect_shift_TR = 0.
contrast_labels = ''
stimlen = 0.
cnorm = 0.
swapbytes=0
cifti_xmlsize=0L

;START160909
glm_masked=0

openr,lu,filename,/GET_LUN
s = string("         ")
readu_swap,lu,s
if strpos(s,"INTERFILE") ge 0 then begin
    lc_interfile = !TRUE
    get_mri_ifh,lu,ifh
    swapbytes=shouldiswap(ifh.bigendian)
    rev = ifh.glm_rev
    N = ifh.glm_N
    n_interest = ifh.glm_n_interest
    xdim = ifh.glm_xdim
    ydim = ifh.glm_ydim
    zdim = ifh.glm_zdim
    tdim = ifh.glm_tdim
    t_valid = ifh.glm_t_valid
    df = ifh.glm_df
    nc = ifh.glm_nc
    period = ifh.glm_period
    num_trials = ifh.glm_num_trials
    TR = ifh.glm_TR
    tot_eff = ifh.glm_tot_eff
    all_eff = ifh.glm_all_eff
    effect_label = ifh.glm_effect_label
    effect_length = ifh.glm_effect_length
    effect_column = ifh.glm_effect_column
    n_files = ifh.glm_n_files
    funclen = ifh.glm_funclen
    functype = ifh.glm_functype
    dxdy = ifh.glm_dxdy
    dz = ifh.glm_dz
    effect_TR = ifh.glm_effect_TR
    effect_shift_TR = ifh.glm_effect_shift_TR
    nF = ifh.glm_nF
    if ptr_valid(ifh.glm_F_names) then begin
        F_names = *ifh.glm_F_names
        if ptr_valid(ifh.glm_F_names2) then F_names2 = *ifh.glm_F_names2 else F_names2 = F_names
    endif else $ 
        nF = 0
    if keyword_set(STOPATFSTAT) then begin
        close,lu
        free_lun,lu
        if ptr_valid(ifh.glm_F_names) then rtn={nF:nF,F_names:F_names,F_names2:F_names2} else rtn={nF:0}
        return,rtn
    endif
    cifti_xmlsize=ifh.glm_cifti_xmlsize

    ;START160909
    glm_masked=ifh.glm_masked

endif else begin
    lc_interfile = !FALSE
    point_lun,lu,0  
    nF = 1
    F_names = 'Omnibus'
    F_names2 = 'Omnibus'
endelse
if rev gt -17 then readu_swap,lu,rev,SWAPBYTES=swapbytes
if rev ge 0 then begin
    ;File written prior to inclusion of rev number.
    rev = 0L
    point_lun,lu,0
    readu_swap,lu,N,n_interest,xdim,ydim,zdim,tdim,t_valid,df,nc,period,SWAPBYTES=swapbytes
endif else begin
    if rev eq -10 then begin
        readu_swap,lu,dum,SWAPBYTES=swapbytes
        readu_swap,lu,N,n_interest,xdim,ydim,zdim,tdim,t_valid,df,nc,period,num_trials,TR,SWAPBYTES=swapbytes
        tot_eff = fix(N/period)
        n_files = 1
    endif
    if(rev eq -11) or (rev eq -12) then begin
        readu_swap,lu,N,n_interest,xdim,ydim,zdim,tdim,t_valid,df,nc,period,num_trials,TR,tot_eff,n_files,SWAPBYTES=swapbytes
    endif
    if(rev eq -13) or (rev eq -14) then begin
        readu_swap,lu,N,n_interest,xdim,ydim,zdim,tdim,t_valid,df,nc,period,num_trials,TR,tot_eff,n_files,funclen,functype, $
            SWAPBYTES=swapbytes
    endif
    if rev eq -15 then begin
        readu_swap,lu,N,n_interest,xdim,ydim,zdim,tdim,t_valid,df,nc,period,num_trials,TR,tot_eff,n_files,funclen,functype, $
            W_smoothness,SWAPBYTES=swapbytes
    endif
    if rev eq -16 then begin
        readu_swap,lu,N,n_interest,xdim,ydim,zdim,tdim,t_valid,df,nc,period,num_trials,TR,tot_eff,n_files,funclen,functype, $
            W_smoothness,dxdy,dz,SWAPBYTES=swapbytes
    endif
endelse

;START160909
;len1=0
;for i=1,N do len1=len1+i
;START160914
;len1=0L
;for i=1,N do len1=len1+i

;print,'here0 len1=',len1
;print,'here0 size(len1)l=',size(len1)

if n_files eq 0 then n_files = 4


;START160909
;if df ne !UNSAMPLED_VOXEL then $
;    ATAm1=dblarr(N,N) $
;else begin
;    len1=0
;    for i=1,N do len1=len1+i
;    ATAm1vox=fltarr(xdim,ydim,zdim,len1)
;    dfvox=fltarr(xdim,ydim,zdim)
;endelse


if nc gt 0 then begin
    c = fltarr(N,nc)
    cnorm = fltarr(tot_eff,nc)
endif
start_data = 0L
if rev gt -16 then mean = fltarr(xdim,ydim,zdim,n_files)
if xdim gt 0 then begin
    sd = fltarr(xdim,ydim,zdim)
    grand_mean = fltarr(xdim,ydim,zdim)
    stat = fltarr(xdim,ydim,zdim)
endif
if rev le -18 then begin 
    if tot_eff gt 0 then begin
        delay = make_array(tot_eff,/FLOAT,VALUE=!HRF_DELTA)
        stimlen = fltarr(tot_eff)
    endif
endif else begin 
    delay = make_array(nc,/FLOAT,VALUE=!HRF_DELTA)
    stimlen = fltarr(nc)
endelse
if tot_eff gt 0 then begin
    lcfunc = intarr(tot_eff)
    effect_group = intarr(tot_eff)
    if rev lt 0 and rev ge -22 then Ysim = fltarr(tdim,tot_eff)
endif
if nc gt 0 then contrast_labels = strarr(nc)
if tdim gt 0 then valid_frms = fltarr(tdim)
if cifti_xmlsize gt 0L then begin
    cifti_xmldata = bytarr(cifti_xmlsize)
    readu_swap,lu,cifti_xmldata,SWAPBYTES=swapbytes
endif

lablen = 0
if rev le -14 then nnc = nc else nnc = tot_eff
for i=0,nnc-1 do begin
    readu_swap,lu,lablen,SWAPBYTES=swapbytes
    if lablen gt 0 then begin
        labs = string(replicate(32B,lablen))
        readu_swap,lu,labs,SWAPBYTES=swapbytes
    endif else begin
        labs = ''
    endelse
    contrast_labels[i] = labs
endfor
if t_valid gt 0L then begin
    A=fltarr(t_valid,N)
    readu_swap,lu,A,SWAPBYTES=swapbytes
endif
if nc gt 0 then readu_swap,lu,c,SWAPBYTES=swapbytes
if rev le -13 then if nc gt 0 then readu_swap,lu,cnorm,SWAPBYTES=swapbytes
if keyword_set(STOPATCNORM) then begin
    close,lu
    free_lun,lu
    return,rtn={nc:nc,contrast_labels:contrast_labels,c:c,cnorm:cnorm}
endif
readu_swap,lu,valid_frms,SWAPBYTES=swapbytes
if rev lt 0 and rev ge -22 then readu_swap,lu,Ysim,SWAPBYTES=swapbytes
if rev le -14 then begin
    if tot_eff gt 0 then readu_swap,lu,delay,SWAPBYTES=swapbytes
endif
if rev le -11 then begin
    if tot_eff gt 0 then readu_swap,lu,stimlen,SWAPBYTES=swapbytes
endif
if keyword_set(GET_CNORM) then begin
    close,lu
    free_lun,lu
    return,rtn = {msg:'OK',ifh:ifh,contrast_labels:contrast_labels,cnorm:cnorm,stimlen:stimlen,A:A}
endif
if rev le -11 and rev ge -22 then readu_swap,lu,effect_group,SWAPBYTES=swapbytes
if rev le -12 then begin
    if tot_eff gt 0 then readu_swap,lu,lcfunc,SWAPBYTES=swapbytes
endif
if rev le -18 then readu_swap,lu,start_data,SWAPBYTES=swapbytes
if not eof(lu) then begin
    dsgn_only = !FALSE
    if rev le -18 then point_lun,lu,start_data

    ;START160909
    ;if glm_masked eq 1 then begin
    ;    mask=fltarr(xdim*ydim*zdim)       
    ;    readu_swap,lu,mask,SWAPBYTES=swapbytes
    ;    maski=where(mask eq 1.,nmaski)
    ;    if nmaski eq 0 then begin
    ;        print,'nmaski=',trim(nmaski),' Should not be zero.'
    ;        return,rtn={msg:'ERROR'}
    ;    endif 
    ;    mask=reform(mask,xdim,ydim,zdim,/OVERWRITE)
    ;endif
    ;START160914
    if glm_masked eq 1 then begin
        maskimg=fltarr(xdim*ydim*zdim)
        readu_swap,lu,maskimg,SWAPBYTES=swapbytes
        maski=where(maskimg eq 1.,nmaski)

        ;print,'here100 maski=',maski
        ;print,'here100 size(maski)=',size(maski)
        ;print,'here100 nmaski=',nmaski

        if nmaski eq 0 then begin
            print,'nmaski=',trim(nmaski),' Should not be zero.'
            return,rtn={msg:'ERROR'}
        endif
    endif

    ;START160915
    len1=0L
    for i=1,N do len1=len1+i

    ;if df ne !UNSAMPLED_VOXEL then $
    ;    readu_swap,lu,ATAm1,SWAPBYTES=swapbytes $
    ;else begin 
    ;START160909
    if df ne !UNSAMPLED_VOXEL then begin 
        ATAm1=dblarr(N,N)
        if rev gt -27 then begin 
            readu_swap,lu,ATAm1,SWAPBYTES=swapbytes
        endif else begin
            temp_float=fltarr(len1)
            readu_swap,lu,temp_float,SWAPBYTES=swapbytes
            i=0L
            for j=0,N-1 do begin
                for k=j,N-1 do begin
                    ;print,'here4 i=',i,' len1=',len1,' j=',j,' k=',k,' N=',N,' temp_float[i]=',temp_float[i]
                    ATAm1[j,k]=double(temp_float[i])
                    ATAm1[k,j]=ATAm1[j,k]
                    i=i+1L
                endfor
            endfor
            undefine,temp_float 
        endelse


    endif else begin 
 
        ;START160909
        ATAm1vox=fltarr(xdim,ydim,zdim,len1)
        dfvox=fltarr(xdim,ydim,zdim)


        for i=0,len1-1 do begin
            readu_swap,lu,stat,SWAPBYTES=swapbytes
            ATAm1vox[*,*,*,i] = stat
        endfor
        readu_swap,lu,dfvox,SWAPBYTES=swapbytes
    endelse


    if rev gt -17 then begin
        fs = fstat(lu) ; Get current pointer position.
        b_offset = fs.cur_ptr
        point_lun,lu,fs.cur_ptr + 4L*xdim*ydim*zdim*N
        readu_swap,lu,sd,SWAPBYTES=swapbytes
    endif else begin


	;readu_swap,lu,sd,SWAPBYTES=swapbytes
        ;if rev le -25 then begin
        ;    var = dblarr(xdim,ydim,zdim)
        ;    readu_swap,lu,var,SWAPBYTES=swapbytes
        ;endif
        ;if rev le -26 then begin
        ;    if nF ne 0 then begin
        ;        ffstat = fltarr(xdim,ydim,zdim,nF)
        ;        for i=0,nF-1 do begin
   	;            readu_swap,lu,stat,SWAPBYTES=swapbytes
   	;            ffstat[*,*,*,i] = stat
        ;        endfor
        ;    endif
        ;endif
        ;if nF ne 0 then begin
        ;    fzstat = fltarr(xdim,ydim,zdim,nF)
        ;    for i=0,nF-1 do begin
   	;         readu_swap,lu,stat,SWAPBYTES=swapbytes
   	;         fzstat[*,*,*,i] = stat
        ;    endfor
        ;endif
        ;readu_swap,lu,grand_mean,SWAPBYTES=swapbytes
        ;START160909
        if rev gt -27 then readu_swap,lu,sd,SWAPBYTES=swapbytes
        if rev eq -25 or rev eq -26 then begin 
            var=dblarr(xdim,ydim,zdim)
            readu_swap,lu,var,SWAPBYTES=swapbytes
        endif else if rev le -27 then begin
            if glm_masked eq 0 then begin
                temp_float=fltarr(xdim,ydim,zdim)
                readu_swap,lu,temp_float,SWAPBYTES=swapbytes
                var=double(temp_float)
                undefine,temp_float
            endif else begin
                temp_float=fltarr(nmaski)
                readu_swap,lu,temp_float,SWAPBYTES=swapbytes
                var=dblarr(xdim*ydim*zdim)
                var[maski]=double(temp_float)
                var=reform(var,xdim,ydim,zdim,/OVERWRITE)
            endelse
            sd=sqrt(var)
        endif
        if nF ne 0 then begin
            if rev le -26 then begin
                ffstat=fltarr(xdim,ydim,zdim,nF)
                if glm_masked eq 0 then begin
                    for i=0,nF-1 do begin
                        readu_swap,lu,stat,SWAPBYTES=swapbytes
                        ffstat[*,*,*,i] = stat
                    endfor
                endif else begin
                    stat1=fltarr(xdim*ydim*zdim)
                    for i=0,nF-1 do begin
                        readu_swap,lu,temp_float,SWAPBYTES=swapbytes
                        stat1[maski]=temp_float
                        stat=reform(stat1,xdim,ydim,zdim)
                        ffstat[*,*,*,i]=stat
                    endfor
                endelse
            endif
            fzstat=fltarr(xdim,ydim,zdim,nF)
            if glm_masked eq 0 then begin
                for i=0,nF-1 do begin
                    readu_swap,lu,stat,SWAPBYTES=swapbytes
                    fzstat[*,*,*,i]=stat
                endfor
            endif else begin
                for i=0,nF-1 do begin
                    readu_swap,lu,temp_float,SWAPBYTES=swapbytes
                    stat1[maski]=temp_float
                    stat=reform(stat1,xdim,ydim,zdim)
                    fzstat[*,*,*,i]=stat
                endfor
            endelse
        endif
        if glm_masked eq 0 then $
            readu_swap,lu,grand_mean,SWAPBYTES=swapbytes $
        else begin
            readu_swap,lu,temp_float,SWAPBYTES=swapbytes
            stat1[maski]=temp_float

            ;print,temp_float
            ;print,'n_elements(temp_float)=',n_elements(temp_float)

            grand_mean=reform(stat1,xdim,ydim,zdim)

            ;print,grand_mean
            ;print,'n_elements(grand_mean)=',n_elements(grand_mean)
            ;print,'max(grand_mean)=',max(grand_mean)
            ;print,'max(temp_float)=',max(temp_float)
            ;print,'size(grand_mean)=',size(grand_mean)

            undefine,stat1,temp_float

            ;print,'after max(grand_mean)=',max(grand_mean)
        endelse


	fs = fstat(lu) ; Get current pointer position.
        b_offset = fs.cur_ptr
    endelse
    if not eof(lu) then begin

        ;if cifti_xmlsize gt 0L then begin
        ;    xdimdisp=ceil(sqrt(xdim))
        ;    ydimdisp=xdimdisp
        ;    zdimdisp=zdim
        ;    b=assoc(lu,fltarr(xdim*ydim*zdim),b_offset)
        ;endif else begin
        ;    xdimdisp=xdim
        ;    ydimdisp=ydim
        ;    zdimdisp=zdim
        ;    b = assoc(lu,fltarr(xdim,ydim,/NOZERO),b_offset)
        ;endelse
        ;START160914
        if cifti_xmlsize gt 0L then begin
            xdimdisp=ceil(sqrt(xdim))
            ydimdisp=xdimdisp
            zdimdisp=zdim
            b=assoc(lu,fltarr(xdim*ydim*zdim),b_offset)
        endif else if glm_masked eq 0 then begin
            xdimdisp=xdim
            ydimdisp=ydim
            zdimdisp=zdim
            b=assoc(lu,fltarr(xdim,ydim,/NOZERO),b_offset)
        endif else begin
            xdimdisp=xdim
            ydimdisp=ydim
            zdimdisp=zdim
            b=assoc(lu,fltarr(nmaski,/NOZERO),b_offset)
        endelse


        fi.lun[fi.nfiles] = lu
        if(rev le -11) and (rev gt -16) then readu_swap,lu,mean,SWAPBYTES=swapbytes
        if(rev le -13) and (rev gt -17) then begin
            fzstat = fltarr(xdim,ydim,zdim,nF)
            readu_swap,lu,fzstat,SWAPBYTES=swapbytes
        endif
    endif
endif else begin
    dsgn_only = !TRUE
endelse
if lc_interfile eq !FALSE then begin
    if(lcbatch eq !FALSE) then begin
        slabels = strarr(fi.nfiles)
        slabels(0:fi.nfiles-1) = fi.tails(0:fi.nfiles-1)
        idxdata = get_button(slabels,TITLE='Data file to be linked to.')
     endif else begin
        idxdata = fi.n
    endelse
    hdr = *fi.hdr_ptr(idxdata)
    ptr_free,fi.hdr_ptr(idxdata)
    hdr.model = fi.nfiles
    fi.hdr_ptr(idxdata) = ptr_new(hdr)
    if(dxdy eq 0.) then $
        dxdy = hdr.ifh.scale_1
    if(dz eq 0.) then $
        dz = hdr.ifh.scale_3
    mom =  idxdata
    ifh = hdr.ifh
endif else begin
    mom = fi.n
endelse
scl = 1.
name = filename

;print,'load_linmod xdim=',xdim,' ydim=',ydim,' zdim=',zdim
;print,'load_linmod xdimdisp=',xdimdisp,' ydimdisp=',ydimdisp,' zdimdisp=',zdimdisp

;if ifh.glm_boldtype eq 'nifti' then begin
;START170407
space=get_space(xdim,ydim,zdim)
if space eq !SPACE_MNI222 or space eq !SPACE_MNI333 then begin 

    c_orient=lonarr(3)
    stat=call_external(!SHARE_LIBCIFTI,'_nifti_getorient',ifh.center,ifh.mmppix,c_orient, $
        VALUE=[0,0,0])
    if stat eq 0L then begin
        print,'Error in _nifti_getorient'
        return,rtn={msg:'ERROR'}
    endif
    ifh.c_orient=ptr_new(c_orient)
    ifh.cf_flip=long(strjoin(trim(c_orient),/SINGLE))

    ;print,'load_linmod *ifh.c_orient=',*ifh.c_orient,' ifh.cf_flip=',ifh.cf_flip

endif

;if not keyword_set(DONT_PUT_IMAGE) then put_image,b,fi,wd,dsp,name,xdimdisp,ydimdisp,zdimdisp,N,!FLOAT,dxdy,dz,scl, $
;    !LINEAR_MODEL,fi.nfiles,MODEL=fi.nfiles,IFH=ifh,/DONTCHANGEIFH
;START160914
if glm_masked eq 0 then uvalue=0 else uvalue=maski
if not keyword_set(DONT_PUT_IMAGE) then put_image,b,fi,wd,dsp,name,xdimdisp,ydimdisp,zdimdisp,N,!FLOAT,dxdy,dz,scl, $
    !LINEAR_MODEL,fi.nfiles,MODEL=fi.nfiles,IFH=ifh,/DONTCHANGEIFH,UVALUE=uvalue

glm[fi.n].nc = nc
glm[fi.n].N = N
glm[fi.n].n_interest = n_interest
glm[fi.n].df = df
glm[fi.n].xdim = xdim
glm[fi.n].ydim = ydim
glm[fi.n].zdim = zdim
glm[fi.n].tdim = tdim
glm[fi.n].dxdy = dxdy
glm[fi.n].dz = dz
glm[fi.n].nF = nF
if nF ne 0 then begin
    glm[fi.n].F_names = ptr_new(F_names)
    glm[fi.n].F_names2 = ptr_new(F_names2)
    undefine,F_names,F_names2
endif
glm[fi.n].t_valid = t_valid
glm[fi.n].period = period
glm[fi.n].tot_eff = tot_eff
if(rev le -17) then begin
    glm[fi.n].all_eff = all_eff
    glm[fi.n].effect_label = effect_label
    glm[fi.n].effect_length = effect_length
    glm[fi.n].effect_column = effect_column
endif else begin
    glm[fi.n].all_eff = tot_eff
    effect_length = intarr(tot_eff)
    effect_column = intarr(tot_eff)
    effect_label = strarr(tot_eff)
    for i=0,tot_eff-1 do begin
        effect_length[i] = period
        effect_column[i] = i*period
        effect_label[i] = contrast_labels[i]
    endfor
    glm[fi.n].effect_label = ptr_new(effect_label)
    glm[fi.n].effect_length = ptr_new(effect_length)
    glm[fi.n].effect_column = ptr_new(effect_length)
endelse
if rev le -20 then begin
    glm[fi.n].funclen = funclen
    glm[fi.n].functype = functype
endif else begin
    funclen = *funclen
    functype = *functype
    effect_length = *effect_length
    for i=0,tot_eff-1 do begin
        if lcfunc[i] eq !TRUE or effect_length[i] eq 1 then begin
            funclen[i] = funclen[tot_eff-1] > 1
            functype[i] = functype[tot_eff-1]
        endif else begin
            funclen[i] = 0 
            functype[i] = 0 
        endelse
    endfor
    glm[fi.n].funclen = ptr_new(funclen)
    glm[fi.n].functype = ptr_new(functype)
endelse
glm[fi.n].effect_TR = effect_TR
glm[fi.n].effect_shift_TR = effect_shift_TR
glm[fi.n].W = W_smoothness
glm[fi.n].fwhmx = fwhmx
glm[fi.n].fwhmy = fwhmy
glm[fi.n].fwhmz = fwhmz
glm[fi.n].n_files = n_files
glm[fi.n].start_data = start_data
if n_files eq 0 then n_files = 4
if(rev lt 0) then begin
    glm(fi.n).num_trials = num_trials
    glm(fi.n).TR = TR
    glm.rev = rev
endif
glm(fi.n).A = ptr_new(A)
if cifti_xmlsize gt 0L then glm[fi.n].cifti_xmldata = ptr_new(cifti_xmldata) 
glm(fi.n).c = ptr_new(c)
glm(fi.n).cnorm = ptr_new(cnorm)
glm(fi.n).delay = ptr_new(delay)
if rev le -11 then glm[fi.n].stimlen = ptr_new(stimlen)
if rev le -11 and rev ge -22 then glm[fi.n].effect_group = ptr_new(effect_group)
glm(fi.n).lcfunc = ptr_new(lcfunc)
glm(fi.n).contrast_labels = ptr_new(contrast_labels)
glm(fi.n).valid_frms = ptr_new(valid_frms)
if rev lt 0 and rev ge -22 then glm(fi.n).Ysim = ptr_new(Ysim)
if dsgn_only eq !FALSE then begin

    ;START160914
    if glm_masked eq 1 then begin
        glm[fi.n].maskimg=ptr_new(maskimg)
        glm[fi.n].maski=ptr_new(maski)
        glm[fi.n].nmaski=nmaski
    endif


    if df ne !UNSAMPLED_VOXEL then $
        glm[fi.n].ATAm1 = ptr_new(ATAm1) $
    else begin
        glm[fi.n].ATAm1vox = ptr_new(ATAm1vox)
        glm[fi.n].dfvox = ptr_new(dfvox)
    endelse
    sz = size(b)
    if(sz[0] ne 0) then begin
        glm(fi.n).b = ptr_new(b)
    endif else begin
        if(ptr_valid(glm(fi.n).b)) then $
            ptr_free,glm(fi.n).b
    endelse
    glm(fi.n).sd = ptr_new(sd)
    if rev ge -24 then var = double(sd^2)
    glm[fi.n].var = ptr_new(var)
    if((rev le -11) and (rev gt -16)) then glm(fi.n).mean = ptr_new(mean)
    if rev le -26 and nF ne 0 then glm[fi.n].fstat = ptr_new(ffstat)
    if rev le -13 and nF ne 0 then glm[fi.n].fzstat = ptr_new(fzstat)

    ;if rev le -17 then begin
    ;    glm(fi.n).grand_mean = ptr_new(grand_mean)
    ;START160914
    if rev le -17 then begin
        glm[fi.n].grand_mean = ptr_new(grand_mean)
        ;print,'here99 max(*glm[fi.n].grand_mean)=',max(*glm[fi.n].grand_mean)

    endif else begin
        grand_mean = fltarr(xdim,ydim,zdim)
        if((glm[fi.n].rev le -16) or (glm[fi.n].rev eq 0)) then begin
            slpcol = glm[fi.n].N-2
            for ifile=0,glm[fi.n].n_files-1 do begin
                for z=0,zdim-1 do begin
                    grand_mean[*,*,z] = grand_mean[*,*,z] + b[*,*,slpcol*zdim+z]
                endfor
                slpcol = slpcol - 2
            endfor
        endif else begin
            for ifile=0,glm[fi.n].n_files-1 do $
                grand_mean = grand_mean + mean[*,*,*,ifile]
        endelse
        grand_mean = grand_mean/glm[fi.n].n_files
        glm[fi.n].grand_mean = ptr_new(grand_mean)
    endelse
endif
if !BATCH eq !FALSE then widget_control,HOURGLASS=0
paradigm = valid_frms[0:tdim-1]
if mom eq -1 then mom = 0 ;ADDED
fi.paradigm[mom] = ptr_new(paradigm)
fi.paradigm_loaded[mom] = !TRUE
return,rtn={msg:'OK',filename:filename,lu:lu}
end
