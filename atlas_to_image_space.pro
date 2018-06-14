;Copyright 12/31/99 Washington University.  All Rights Reserved.
;atlas_to_image_space.pro  $Revision: 1.5 $
function atlas_to_image_space,hdr,x_atl,y_atl,z_atl,view,space,XYZDIM=xyzdim
if not keyword_set(XYZDIM) then begin
    xdim = hdr.xdim
    ydim = hdr.ydim
    zdim = hdr.zdim 
endif else begin
    xdim = xyzdim[0] 
    ydim = xyzdim[1] 
    zdim = xyzdim[2] 
endelse
atlas_pix = fltarr(3)
atlas_off = fltarr(3)
if space eq !SPACE_111 then begin
    str = '111:'
    atlas_pix[0] = !MMPPIX_X_111
    atlas_pix[1] = !MMPPIX_Y_111
    atlas_pix[2] = !MMPPIX_Z_111
    atlas_off[0] = !CENTER_X_111
    atlas_off[1] = !CENTER_Y_111
    atlas_off[2] = !CENTER_Z_111

    ;START180119
    x_img = -(x_atl - atlas_off[0])/atlas_pix[0]
    y_img =  (y_atl + atlas_off[1])/atlas_pix[1]
    z_img =  (z_atl + atlas_off[2])/atlas_pix[2]

    ;START180122
    case view of
        !TRANSVERSE: v = [x_img-1,y_img-1,zdim-z_img]
        !SAGITTAL:    v = [y_img-1,zdim-z_img,x_img-1]
        !CORONAL:    v = [x_img-1,z_img,y_img-1]
        !VIEW_COLOR_SCALE:   v = [0,0,0]
        else: begin
            print,'Unknown image orientation.'
            return,-1
        end
    endcase


endif else if space eq !SPACE_222 then begin
    str = '222:'
    atlas_pix[0] = !MMPPIX_X_222
    atlas_pix[1] = !MMPPIX_Y_222
    atlas_pix[2] = !MMPPIX_Z_222
    atlas_off[0] = !CENTER_X_222
    atlas_off[1] = !CENTER_Y_222
    atlas_off[2] = !CENTER_Z_222

    ;START180119
    x_img = -(x_atl - atlas_off[0])/atlas_pix[0]
    y_img =  (y_atl + atlas_off[1])/atlas_pix[1]
    z_img =  (z_atl + atlas_off[2])/atlas_pix[2]

    ;START180122
    case view of
        !TRANSVERSE: v = [x_img-1,y_img-1,zdim-z_img]
        !SAGITTAL:    v = [y_img-1,zdim-z_img,x_img-1]
        !CORONAL:    v = [x_img-1,z_img,y_img-1]
        !VIEW_COLOR_SCALE:   v = [0,0,0]
        else: begin
            print,'Unknown image orientation.'
            return,-1
        end
    endcase


endif else if space eq !SPACE_333 then begin
    str = '333:'
    atlas_pix[0] = !MMPPIX_X_333
    atlas_pix[1] = !MMPPIX_Y_333
    atlas_pix[2] = !MMPPIX_Z_333
    atlas_off[0] = !CENTER_X_333
    atlas_off[1] = !CENTER_Y_333
    atlas_off[2] = !CENTER_Z_333

    ;START180119
    x_img = -(x_atl - atlas_off[0])/atlas_pix[0]
    y_img =  (y_atl + atlas_off[1])/atlas_pix[1]
    z_img =  (z_atl + atlas_off[2])/atlas_pix[2]

    ;START180122
    case view of
        !TRANSVERSE: v = [x_img-1,y_img-1,zdim-z_img]
        !SAGITTAL:    v = [y_img-1,zdim-z_img,x_img-1]
        !CORONAL:    v = [x_img-1,z_img,y_img-1]
        !VIEW_COLOR_SCALE:   v = [0,0,0]
        else: begin
            print,'Unknown image orientation.'
            return,-1
        end
    endcase


endif else if space eq !SPACE_MNI222 then begin
    str = '222:'
    atlas_pix[0] = !MMPPIX_X_MNI222
    atlas_pix[1] = !MMPPIX_Y_MNI222
    atlas_pix[2] = !MMPPIX_Z_MNI222
    atlas_off[0] = !CENTER_X_MNI222
    atlas_off[1] = !CENTER_Y_MNI222
    atlas_off[2] = !CENTER_Z_MNI222

    ;START180119
    x_img = -(x_atl - atlas_off[0])/atlas_pix[0]
    y_img =  (y_atl + atlas_off[1])/atlas_pix[1]
    z_img =  (z_atl + atlas_off[2])/atlas_pix[2]

    ;START180122
    case view of
        !TRANSVERSE: v = [x_img-1,y_img-1,zdim-z_img]
        !SAGITTAL:    v = [y_img-1,zdim-z_img,x_img-1]
        !CORONAL:    v = [x_img-1,z_img,y_img-1]
        !VIEW_COLOR_SCALE:   v = [0,0,0]
        else: begin
            print,'Unknown image orientation.'
            return,-1
        end
    endcase


;START180119
endif else if space eq !SPACE_MNI111 then begin
    str = '111:'

    ;print,'here500 x_atl=',x_atl,' y_atl=',y_atl,' z_atl=',z_atl

    x_img = (x_atl + hdr.ifh.center[0])/hdr.ifh.mmppix[0]
    y_img = -(y_atl + hdr.ifh.center[1])/hdr.ifh.mmppix[1]
    z_img = -(-z_atl + hdr.ifh.center[2])/hdr.ifh.mmppix[2]

    ;START180122
    case view of

        ;!TRANSVERSE: v = [x_img-1,y_img-1,zdim-z_img]
        !TRANSVERSE: v = [x_img-1,y_img-1,z_img]

        !SAGITTAL:    v = [y_img-1,zdim-z_img,x_img-1]
        !CORONAL:    v = [x_img-1,z_img,y_img-1]
        !VIEW_COLOR_SCALE:   v = [0,0,0]
        else: begin
            print,'Unknown image orientation.'
            return,-1
        end
    endcase

    ;print,'here501 v=',v


endif else if space eq !SPACE_DATA then begin
    str = 'DS:'
    v = [0,0,0]
    return,v
endif else begin
    return,-1
endelse

;START180119
;x_img = -(x_atl - atlas_off[0])/atlas_pix[0]
;y_img =  (y_atl + atlas_off[1])/atlas_pix[1]
;z_img =  (z_atl + atlas_off[2])/atlas_pix[2]

;START180122
;case view of
;    !TRANSVERSE: v = [x_img-1,y_img-1,zdim-z_img]
;    !SAGITTAL:    v = [y_img-1,zdim-z_img,x_img-1]
;    !CORONAL:    v = [x_img-1,z_img,y_img-1]
;    !VIEW_COLOR_SCALE:   v = [0,0,0]
;    else: begin
;        print,'Unknown image orientation.'
;        return,-1
;    end
;endcase


return,v

;defsysv,'!MMPPIX_X_222',2.
;defsysv,'!MMPPIX_Y_222',-2.
;defsysv,'!MMPPIX_Z_222',-2.
;defsysv,'!CENTER_X_222',129.
;defsysv,'!CENTER_Y_222',-129.
;defsysv,'!CENTER_Z_222',-82.


;                v = get_atlas_coords(ev.x,dsp[fi.cw].dsizey-ev.y-1,dsp[fi.cw].plane-1,0,0,fi.view[fi.n],fi.space[fi.n],0,0, $
;                    dsp[fi.cw].zoom,!FALSE,dsp[fi.cw].orientation,dsp[fi.cw].xdim,dsp[fi.cw].ydim,fi,stc,dsp,IMAGE=dsp_image)

end
