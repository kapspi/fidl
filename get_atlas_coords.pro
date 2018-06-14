;Copyright 12/31/99 Washington University.  All Rights Reserved.
;get_atlas_coords.pro  $Revision: 1.20 $

;function get_atlas_coords,xin,yin,zin,x0,y0,view,space,xpad,ypad,zoom,sagittal_face_left,orientation,xdim,ydim,fi,stc,dsp, $
;    IMAGE=image,SECOND_IMAGE=second_image,RETURN_XY=return_xy
;START180308
function get_atlas_coords,xin,yin,zin,x0,y0,view,space,xpad,ypad,zoom,sagittal_face_left,orientation,xdim,ydim,fi,stc,dsp, $
    IMAGE=image,SECOND_IMAGE=second_image,RETURN_XY=return_xy,DSPYDIM=dspydim

;Voxel coordinate system is zero at the upper left corner of the image at the base of the brain.

;print,'get_atlas_coords top  yin=',yin,' y0=',y0,' ypad=',ypad,' ydim=',ydim,' zoom=',zoom
;print,'get_atlas_coords top  xin=',xin,' x0=',x0,' xpad=',xpad,' xdim=',xdim,' zoom=',zoom

hdr=*fi.hdr_ptr[fi.n]

;print,'get_atlas_coords 0 zoom=',zoom

case view of
    !TRANSVERSE: begin

        ;print,'TRANSVERSE'

        ;if orientation eq !RADIOL then $
        ;    x = (xin - x0)/zoom $
        ;else $
        ;    x = xdim - 1 - (xin - x0)/zoom
        ;y = yin/zoom
        ;xx = x + xpad
        ;yy = y + ypad
        ;zz = zin
        ;     
        ;;START150708
        ;xx_mni=(xin - x0)/zoom + xpad
        ;;START150709
        ;xx_mni=(xin - x0)/zoom -1 + xpad
        ;
        ;yy_mni=ydim - 1 - (yin - y0)/zoom + ypad
        ;START150709
        if orientation eq !RADIOL then begin 
            x = (xin - x0)/zoom 
            ;print,'get_atlas_coords RADIOL'
        endif else begin 

            x = xdim - 1 - (xin - x0)/zoom
            ;START180309
            ;x = xdim - 1. - (xin - x0)/zoom + xpad

            ;print,'get_atlas_coords NEUROL'
        endelse

        ;y = (yin-y0)/zoom  ;orig
        ;START180303
        ;y = yin - ypad
        ;START180413
        y = yin/zoom


        if space eq !SPACE_MNI111 or space eq !SPACE_MNI222 or space eq !SPACE_MNI333 then begin
            if hdr.ifh.cf_flip eq 12 then begin

                xx=(xin - x0)/zoom -1 + xpad

                ;yy=ydim - (yin - y0)/zoom -1 + ypad
                ;START180416
                yy=ydim - yin/zoom -1 + ypad

                ;print,'    xx=',xx,' yy=',yy 

            endif else if hdr.ifh.cf_flip eq 412 then begin
                xx=x +1 + xpad ;
                yy=ydim - 1 - (yin - y0)/zoom + ypad
            endif else if hdr.ifh.cf_flip eq 52 then begin
                ;xx = x + xpad
                xx=(xin - x0)/zoom -1 + xpad
                ;yy = y + ypad
                yy=(yin - y0)/zoom + ypad
                ;print,'x+xpad=',x+xpad,' y+ypad=',y+ypad
            endif else begin
                ;xx = x + xpad
                ;yy = y + ypad
                ;START170413
                xx=(xin - x0)/zoom -1 + xpad
                yy=(yin - y0)/zoom + ypad
            endelse

            ;print,'get_atlas_coords 2 xx=',xx,' yy=',yy

            ;print,'here2 xx=',xx,' yy=',yy
        endif else begin
            xx = x + xpad
            yy = y + ypad
        endelse
        zz = zin

    end
    !SAGITTAL: begin
        if sagittal_face_left eq !FALSE then begin
            ;print,'here face left'
            x = xdim - 1 - (xin - x0)/zoom
        endif else begin 
            ;print,'here face right'
            x = (xin - x0)/zoom
        endelse

        ;y = ydim - 1 - yin/zoom
        ;z = zin
        ;xx = z
        ;yy = x + xpad
        ;zz = y + ypad
        ;xi = x
        ;yi = yin/zoom 

        y = yin/zoom
        xx = zin
        yy = x + xpad
        zz = ydim - 1 - y + ypad
    end
    !CORONAL: begin
        if(orientation eq !NEURO) then $
            x = xdim - 1 - (xin - x0)/zoom $
        else $
            x = (xin - x0)/zoom

        ;y = ydim - 1 - yin/zoom
        ;z = zin
        ;xx = x + xpad
        ;yy = z
        ;zz = y + ypad
        ;xi = x
        ;yi = yin/zoom

        y = yin/zoom
        xx = x + xpad
        yy = zin
        zz = ydim - 1 - y + ypad
    end
    else: begin
      xx = 0
      yy = 0
      zz = 0
    end
endcase

;print,'get_atlas_coords xx=',xx,' yy=',yy,' zz=',zz
;print,'get_atlas_coords xin=',xin,' yin=',yin,' x=',x,' y=',y
;print,'size(image)=',size(image)
;print,'size(second_image)=',size(second_image)
;index = where(image eq 6,count)
;if count eq 0 then $
;    print,'NONE' $
;else begin
;    print,'count=',count
;    print,'index=',index
;    for i=0,114 do begin
;        for j=0,74 do begin
;            if image[i,j] eq 6 then begin
;                print,'i=',i,' j=',j
;                ;goto,goose
;            endif
;        endfor
;    endfor
;endelse 
;;goose:
;print,'xpad=',xpad,' ypad=',ypad,' xin=',xin,' yin=',yin

;START150709
;START150708
;c_mni=0.

if space eq !SPACE_DATA or space eq !UNKNOWN then begin
    v = [xx,yy,zz]
endif else begin

    ;if filetype[0] eq 'nii' then begin
    ;    v=fltarr(3)
    ;    if hdr.array_type eq !STITCH_ARRAY then begin
    ;        t_to_file = *stc[fi.n].t_to_file
    ;        i_file = t_to_file[(hdr.zdim*(dsp[fi.cw].frame-1)+dsp[fi.cw].plane-1)/hdr.zdim]
    ;        filnam = *stc[fi.n].filnam
    ;        if strpos(filnam[i_file],!SLASH) lt 0 then file=fi.paths[filnum]+!SLASH+filnam[i_file] else file=filnam[i_file]
    ;    endif else file=fi.names[fi.n]
    ;    stat=call_external(!SHARE_LIBCIFTI,'_nifti_getmni',file,[xx,yy,zz],v,VALUE=[1,0,0])
    ;endif else $
    ;    v = image_to_atlas_space(space,xx,yy,zz,!TRANSVERSE,hdr.ifh)
    ;START150731
    ;if filetype[0] eq 'nii' then begin
    ;START170407
    ;if space eq !SPACE_MNI222 or space eq !SPACE_MNI333 then begin
    ;START171218
    if space eq !SPACE_MNI111 or space eq !SPACE_MNI222 or space eq !SPACE_MNI333 then begin


        v=fltarr(3)
        if hdr.array_type eq !LINEAR_MODEL then begin 
            ;print,'get_atlas_coords xdim=',xdim,' ydim=',ydim,' hdr.xdim=',hdr.xdim,' hdr.ydim=',hdr.ydim,' hdr.zdim=',hdr.zdim

            ;hdr.ifh.center[0]=-hdr.ifh.center[0]
            ;hdr.ifh.center[1]=-hdr.ifh.center[1]
            ;hdr.ifh.center[1]=-hdr.ifh.center[1]-36
            ;print,'hdr.ifh.center=',hdr.ifh.center
            ;print,'hdr.ifh.mmppix=',hdr.ifh.mmppix

            stat=call_external(!SHARE_LIBCIFTI,'_nifti_getmni2',hdr.ifh.center,hdr.ifh.mmppix,$
                [long64(hdr.xdim),long64(hdr.ydim),long64(hdr.zdim)],[xx,yy,zz],v,VALUE=[0,0,0,0,0])
        endif else begin 
            if hdr.array_type eq !STITCH_ARRAY then begin

                ;print,'get_atlas_coords 4a'

                ;t_to_file = *stc[fi.n].t_to_file
                ;i_file = t_to_file[(hdr.zdim*(dsp[fi.cw].frame-1)+dsp[fi.cw].plane-1)/hdr.zdim]
                ;filnam = *stc[fi.n].filnam
                ;if strpos(filnam[i_file],!SLASH) lt 0 then file=fi.paths[filnum]+!SLASH+filnam[i_file] else file=filnam[i_file]
                ;START190119
                if stc[fi.n].n gt 1 then begin
                   t_to_file = *stc[fi.n].t_to_file
                   i_file = t_to_file[(hdr.zdim*(dsp[fi.cw].frame-1)+dsp[fi.cw].plane-1)/hdr.zdim]
                   filnam = *stc[fi.n].filnam
                   if strpos(filnam[i_file],!SLASH) lt 0 then file=fi.paths[filnum]+!SLASH+filnam[i_file] else file=filnam[i_file]
                endif else $
                   file=fi.names[fi.n]

            endif else begin 

                ;print,'get_atlas_coords 4b1'

                file=fi.names[fi.n]

                ;print,'get_atlas_coords 4b2 fi.n=',fi.n
                ;print,'get_atlas_coords 4b2 fi.names[fi.n]=',fi.names[fi.n]
            endelse

            ;stat=call_external(!SHARE_LIBCIFTI,'_nifti_getmni',file,[xx,yy,zz],v,VALUE=[1,0,0])
            ;START170411
            filetype=*stc[fi.n].filetype
            if filetype[0] eq 'nii' then begin 

                ;stat=call_external(!SHARE_LIBCIFTI,'_nifti_getmni',file,[xx,yy,zz],v,VALUE=[1,0,0])
                ;START180220
                stat=call_external(!SHARE_LIBCIFTI,'_nifti_getmni',file,float([xx,yy,zz]),v,VALUE=[1,0,0])

                ;print,'get_atlas_coords 5a file=',file
                ;print,'get_atlas_coords 5a     xx=',xx,' yy=',yy,' zz=',zz
                ;print,'get_atlas_coords 5a      v=',v

            endif else begin 

                ;v=image_to_atlas_space(space,xx,yy,zz,!TRANSVERSE,hdr.ifh)
                ;START170413
                stat=call_external(!SHARE_LIBCIFTI,'_nifti_getmni2',hdr.ifh.center,hdr.ifh.mmppix,$
                    [long64(hdr.xdim),long64(hdr.ydim),long64(hdr.zdim)],[xx,yy,zz],v,VALUE=[0,0,0,0,0])

                ;print,'get_atlas_coords 5b'

            endelse
        endelse
    endif else $
        v = image_to_atlas_space(space,xx,yy,zz,!TRANSVERSE,hdr.ifh)

endelse

;print,'get_atlas_coords image[0,0]=',image[0,0]

;x=31
;y=26.5
;x=xin
;y=yin
;print,'    x=',x,' y=',y

;if keyword_set(IMAGE) then v = [v,image[x,y]]
if keyword_set(IMAGE) then begin

    ;print,'get_atlas_coords size(image)=',size(image),' x=',x,' y=',y

    ;print,'get_atlas_coords x=',x,' y=',y,' image[x,y]=',image[x,y]
    v = [v,image[x,y]]
endif


;if keyword_set(SECOND_IMAGE) then v = [v,second_image[x,y]]
if keyword_set(SECOND_IMAGE) then begin

    ;print,'get_atlas_coords size(second_image)=',size(second_image)

    v = [v,second_image[x,y]]
endif

if keyword_set(RETURN_XY) then v = [v,x,y]

;print,'image[0,0]=',image[0,0]
;print,'xx=',xx,' yy=',yy,' zz=',zz
;print,'get_atlas_coords bottom v=',v


;START150709
return,v
;START150708
;return,rtn={v:v,c_mni:c_mni}

end
