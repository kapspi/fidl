;Copyright 12/31/99 Washington University.  All Rights Reserved.
;stats_exec.pro  $Revision: 12.484 $
;**********************
pro stats_exec_event,ev
;**********************
common stats_comm,wd,wd1,fi,pr,dsp,fl,st,stc,cstm,bltimg,glm,dsp_image,help,pref,first_pass,xm1,ym1,vw

common profiles,x1,x2,y1,y2,x1m1,x2m1,y1m1,y2m1,row,col,profdat,profmask,n_clicks

common linear_fit,fit_name,slope,intcpt,trend

common color_table_index,table_indx

common fidl_batch,macro_commands,num_macro_cmd,lcbatch,batch_base, $
        batch_id,batch_name,macro_descriptor,num_macro_dsc,batch_pro

type = tag_names(ev,/STRUCTURE)
on_error,2

for i=0,!MAX_WORKING_WINDOWS-1 do begin
    if ev.id eq wd.draw[i] then begin
        cwm1 = fi.cw
        fi.cw = i
        if fi.cw ne cwm1 then begin
            dsp_image = update_image(fi,dsp,wd,stc,pref)
        endif
    endif
endfor

if wd.draw[fi.cw] gt 0 then begin
    widget_control,GET_VALUE=i,wd.draw[fi.cw]
    if dsp[fi.cw].image_index[fi.cw] ne i then return
endif

if first_pass eq !TRUE then first_pass = !FALSE
error_status=0
catch,error_status
if error_status ne 0 then begin
    stat=widget_message('*** Procedure aborted due to an error.  ***',/ERROR)
    print,'Error index: ',error_status
    print,'Error message: ',!ERR_STRING
    pr.prof_on = !PROF_NO
    wset,dsp[fi.cw].image_index[fi.cw]
    return
endif

; Recover from failure to complete a time-profile.
if pr.mode eq !DRAWING then begin
    pr.mode = !DRAW_OFF
    pr.time_type = pr.last_type
    device,SET_GRAPHICS=3,/cursor_crosshair     ;Resume normal graphics
    if(dsp[fi.cw].draw_mode eq !DRAW_LOCATION) then $
        widget_control,wd.draw[fi.cw],DRAW_MOTION_EVENTS=0
    widget_control,wd.draw[fi.cw],EVENT_PRO='stats_exec_event'
endif

if ev.id eq wd.draw[fi.cw] then begin ;This has to stay up here for profiles to work correctly.
    if(n_elements(dsp[fi.cw].xdim) eq 0) then return ;No images loaded.
    if(pr.prof_on ne !PROF_NO) then begin
        if(ev.type eq !RELEASE) then return ; Only process button presses.
        if(ev.press eq 2) then begin
            pr.prof_on = !PROF_NO
            pr.roiprof = !FALSE
            widget_control,wd.draw[fi.cw],/DRAW_MOTION_EVENTS ;ADDED 7/24/01
        endif else begin
            widget_control,wd.error,SET_VALUE=string('Exit profile mode with center button.')
            if(pr.prof_on eq !PROF_REG) then $
                ev.id = wd.roiprof $
            else $
                ev.id = wd.oprof
        endelse
    endif else begin
        if(ev.press eq 4) then begin ;Generate event from pulldown menu.
            ev.id = draw_pulldown(wd,wd1,pr,fi,glm,help,pref,dsp)
            if(ev.id lt 0) then return
        endif
    endelse
endif

if(ev.id eq wd.repeat_action) then begin
    ev.id = st.last_evid
endif else begin
    if(ev.id eq wd.repeat_action_m1) then begin
        ev.id = st.second_last_evid
    endif
endelse
if((ev.id ne wd.draw[fi.cw]) and (ev.id ne wd.files) and $
   (ev.id ne wd.nextpln) and (ev.id ne wd.lastpln) and $
   (ev.id ne wd.nextfrm) and (ev.id ne wd.lastfrm)) then begin
    st.second_last_evid = st.last_evid
    st.last_evid = ev.id
endif

report_math_errors = !FALSE
case ev.id of
    wd.exit: begin
        widget_control,wd.leader,/DESTROY
        return
    end
    wd.draw[fi.cw]: begin
        if n_elements(dsp_image) gt 0 then begin 
            if dsp_image[0] ne -1 then begin

                ;print,'here0 stats_exec dsp[fi.cw].xdim=',dsp[fi.cw].xdim,' dsp[fi.cw].ydim=',dsp[fi.cw].ydim
                ;print,'here0 stats_exec ev.x=',ev.x,' dsp[fi.cw].dsizey-ev.y-1=',dsp[fi.cw].dsizey-ev.y-1, $
                ;    ' dsp[fi.cw].plane-1=',dsp[fi.cw].plane-1 
                ;print,'here0 stats_exec dsp[fi.cw].orientation=',dsp[fi.cw].orientation,' dsp[fi.cw].zoom=',dsp[fi.cw].zoom
                ;print,'here0 stats_exec fi.view[fi.n]=',fi.view[fi.n],' fi.space[fi.n]=',fi.space[fi.n]

                ;print,'stats_exec dsp[fi.cw].dsizey-ev.y-1=',dsp[fi.cw].dsizey-ev.y-1,' dsp[fi.cw].dsizey=',dsp[fi.cw].dsizey, $
                ;    ' ev.y=',ev.y

                v = get_atlas_coords(ev.x,dsp[fi.cw].dsizey-ev.y-1,dsp[fi.cw].plane-1,0,0,fi.view[fi.n],fi.space[fi.n],0,0, $
                    dsp[fi.cw].zoom,!FALSE,dsp[fi.cw].orientation,dsp[fi.cw].xdim,dsp[fi.cw].ydim,fi,stc,dsp,IMAGE=dsp_image)

                widget_control,wd.error,SET_VALUE=get_space_str(fi.space[fi.n],v)
                st.error = !ERROR
            endif
        endif
    end

    ;*********** File I/O Functions.  *************
    wd.ldmri_analyze: begin
        stat = load_data(fi,st,dsp,wd,stc,!MRI_ANALYZE,help,glm)
        if(stat eq !OK) then begin
            dsp_image = update_image(fi,dsp,wd,stc,pref)
            device,SET_FONT='Helvetica Bold',/TT_FONT
        endif
    end
    wd.ldmri_4dfp: begin

        ;if load_data(fi,st,dsp,wd,stc,!MRI_4DFP,help,glm,/MULTIPLE_FILES) eq !OK then dsp_image = update_image(fi,dsp,wd,stc,pref)
        ;START171020
        ;if load_data(fi,st,dsp,wd,stc,!MRI_4DFP,help,glm,/MULTIPLE_FILES,FILTER=['*img','*nii','*nii.gz','*.conc']) eq !OK then $
        ;    dsp_image = update_image(fi,dsp,wd,stc,pref)
        ;START180306
        if load_data(fi,st,dsp,wd,stc,!MRI_4DFP,help,glm,/MULTIPLE_FILES,FILTER=['*nii','*img','*nii.gz','*.conc']) eq !OK then $
            dsp_image = update_image(fi,dsp,wd,stc,pref)

    end

    ;START160112
    wd.ld_list: begin
        if load_data(fi,st,dsp,wd,stc,!LIST,help,glm) eq !OK then dsp_image = update_image(fi,dsp,wd,stc,pref)
    end


    wd.ld_gif: begin
        if load_data(fi,st,dsp,wd,stc,!GIF,help,glm) eq !OK then begin
            dsp_image = update_image(fi,dsp,wd,stc,pref)
        endif
    end
    wd.ld_tif: begin
        if load_data(fi,st,dsp,wd,stc,!TIFF,help,glm) eq !OK then begin
            dsp_image = update_image(fi,dsp,wd,stc,pref)
        endif
    end
    wd.ld_xwd: begin
        if load_data(fi,st,dsp,wd,stc,!XWD,help,glm) eq !OK then begin
            dsp_image = update_image(fi,dsp,wd,stc,pref)
        endif
    end
    wd.ldraw: begin
        if load_data(fi,st,dsp,wd,stc,!RAW,help,glm) eq !OK then begin
            dsp_image = update_image(fi,dsp,wd,stc,pref)
        endif
    end

    ;wd.ld_t88: begin
    ;    if load_data(fi,st,dsp,wd,stc,!ECAT_DATA,help,glm) eq !OK then begin
    ;        dsp_image = update_image(fi,dsp,wd,stc,pref)
    ;    endif
    ;end
    ;wd.ld_matrix7v: begin
    ;    if load_data(fi,st,dsp,wd,stc,!MATRIX7V,help,glm) eq !OK then begin
    ;        dsp_image = update_image(fi,dsp,wd,stc,pref)
    ;    endif
    ;end

    wd.stitch: begin
        if load_data(fi,st,dsp,wd,stc,!STITCH,help,glm) eq !OK then dsp_image = update_image(fi,dsp,wd,stc,pref)
    end

    ;START150807
    ;wd1.load_conc: begin
    ;    ;stat = wd1_load_conc(fi,dsp,wd,help,stc,pref)
    ;    ;dsp_image = update_image(fi,dsp,wd,stc,pref)
    ;    ;START150129
    ;    if load_data(fi,st,dsp,wd,stc,!MRI_4DFP,help,glm,FILTER='*.conc',/MULTIPLE_FILES, $
    ;        TITLE='Please select concatenated set(s).') eq !OK then dsp_image = update_image(fi,dsp,wd,stc,pref)
    ;end

    wd1.save_conc: begin
        save_conc,fi,stc
    end

    ;START150114
    ;wd1.generate_conc: begin
    ;    generate_conc,fi,dsp,wd,help,stc
    ;end
    ;wd1.fidl_randskew: begin
    ;    fidl_randskew,fi,dsp,wd,help,stc
    ;end
    ;wd1.fidl_genconc: begin
    ;    stat=fidl_genconc(fi,dsp,wd,glm,help,stc,pref,glmfile,concfile)
    ;end

    wd1.load_atlas: begin
        load_atlas,fi,st,dsp,wd,stc,help,glm,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.load_mask: begin
        load_mask,fi,st,dsp,wd,stc,help,glm,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.delete: begin
        delete_image,wd,fi,dsp,glm,stc,fi.n
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.delete_all: begin
        delete_image,wd,fi,dsp,glm,stc,fi.n,/ALL
    end

    wd.wrtgif: begin
        stat = write_data(fi,st,dsp,wd,stc,!GIF,help,cstm,pr)
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.wrttif: begin
        stat = write_data(fi,st,dsp,wd,stc,!TIFF,help,cstm,pr)
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.wrt4dfp: begin
        stat = write_data(fi,st,dsp,wd,stc,!MRI_4DFP,help,cstm,pr)
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.printany: begin
        print_window,fi,dsp,cstm,pr,pref
    end
    wd1.import_spm: begin
        if load_data(fi,st,dsp,wd,stc,!SPM_MAT,help,glm) eq !OK then begin
            dsp_image = update_image(fi,dsp,wd,stc,pref)
        endif
    end
    wd1.link_image: begin
    ;   Manually restore link from processed file to original data file
        if(help.enable eq !TRUE) then begin
            value=string('Manually restore link from processed file to original data file')
            widget_control,help.id,SET_VALUE=value
        endif
        labels = strarr(fi.nfiles)
        labels(0:fi.nfiles-1) = fi.list(0:fi.nfiles-1)
        idxmap = get_button(labels,TITLE='File to be linked (e.g., statistical map).')
        hdr = *fi.hdr_ptr(idxmap)
        idxdata = get_button(labels,TITLE='Data file.')
        hdr = *fi.hdr_ptr(idxmap)
        ptr_free,fi.hdr_ptr(idxmap)
        hdr.mother = idxdata
        fi.hdr_ptr(idxmap) = ptr_new(hdr)
    end
    wd1.link_model: begin
    ;   Link data file to a model.
        if(help.enable eq !TRUE) then begin
            value=string('Manually restore link from processed file to original data file')
            widget_control,help.id,SET_VALUE=value
        endif
        labels = strarr(fi.nfiles)
        labels(0:fi.nfiles-1) = fi.list(0:fi.nfiles-1)
        idxmodel = get_button(labels,TITLE='Model to be linked (e.g., *.glm file).')
        hdr = *fi.hdr_ptr(idxmodel)
        idxdata = get_button(labels,TITLE='Data file.')
        if(hdr.array_type eq !LINEAR_MODEL) then begin ; Link data file to model.
            hdr = *fi.hdr_ptr(idxdata)
            ptr_free,fi.hdr_ptr(idxdata)
            hdr.model = idxmodel
            fi.hdr_ptr(idxdata) = ptr_new(hdr)
        endif
    end
    wd1.preferences: begin
        get_pref
    end
    wd1.save_prefs: begin
        write_pref,pref
    end
    wd.files: begin
        dsp_image = update_image(fi,dsp,wd,stc,pref)
        if ev.clicks eq 2 then begin
            newname = get_str(1,"New list name: ",fi.list[fi.n],WIDTH=50)
            fi.list[fi.n] = newname[0]
            fi.tails[fi.n] = newname[0]
            widget_control,wd.files,SET_VALUE=fi.list,SET_LIST_SELECT=fi.n
        endif
        if fi.n ge fi.nfiles then return
        fi.name = fi.names[fi.n]
        fi.path = fi.paths[fi.n]
        fi.tail = fi.tails[fi.n]
        dsp[fi.cw].zoom = fi.zoom[fi.n]
        widget_control,wd.sld_pln,SET_SLIDER_MAX=dsp[fi.cw].zdim
        widget_control,wd.sld_frm,SET_SLIDER_MAX=dsp[fi.cw].tdim
        ;if lcbatch eq !TRUE then batch_files,fi,stc
    end

    ;*********** Display Functions.  **************
    wd.adjust_colortable: begin
        if(dsp[fi.cw].adjust_color_top eq 0) then $
            adjust_colortable $
        else $
            widget_control,dsp[fi.cw].adjust_color_top,/SHOW
    end
    wd.cine: begin
        if(help.enable eq !TRUE) then begin
            value='Displays difference between each frame and the first frame in cine mode.'+'Images are smoothed with a 3x3 boxcar filter.'
            widget_control,help.id,SET_VALUE=value
        endif
        widget_control,/HOURGLASS
        frame = dsp[fi.cw].frame
        image1 = get_image(dsp[fi.cw].plane-1,fi,stc)
        x = fltarr(512,512)
    ;;;    for j=0,1 do begin
            for i=2,dsp[fi.cw].tdim do begin
            dsp[fi.cw].frame = i
                image2 = get_image(dsp[fi.cw].zdim*(dsp[fi.cw].frame-1)+dsp[fi.cw].plane-1,fi,stc)
                diff = smooth(image2 - image1,3)
            image2 = smooth(x,7)
                refresh_display,fi,dsp,stc,wd,pref,IMAGE=diff
                widget_control,wd.error,SET_VALUE=string(dsp[fi.cw].frame)
            endfor
    ;;;    endfor
        dsp[fi.cw].frame = frame
        refresh_display,fi,dsp,stc,wd,pref,IMAGE=dsp_image
        widget_control,HOURGLASS=0
    end
    wd.nextpln: begin
        dsp[fi.cw].plane = dsp[fi.cw].plane + 1
        if(dsp[fi.cw].plane gt dsp[fi.cw].zdim) then dsp[fi.cw].plane = dsp[fi.cw].zdim
        widget_control,wd.sld_pln,SET_VALUE=dsp[fi.cw].plane
        dsp_image = update_image(fi,dsp,wd,stc,pref)
        report_math_errors = !FALSE
    end
    wd.lastpln: begin
        dsp[fi.cw].plane = dsp[fi.cw].plane - 1
        if(dsp[fi.cw].plane lt 1) then dsp[fi.cw].plane = 1
        widget_control,wd.sld_pln,SET_VALUE=dsp[fi.cw].plane
        dsp_image = update_image(fi,dsp,wd,stc,pref)
        report_math_errors = !FALSE
    end
    wd.nextfrm: begin
        dsp[fi.cw].frame = dsp[fi.cw].frame + 1
        if(dsp[fi.cw].frame gt dsp[fi.cw].tdim) then dsp[fi.cw].frame = dsp[fi.cw].tdim
        widget_control,wd.sld_frm,SET_VALUE=dsp[fi.cw].frame
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.lastfrm: begin
        dsp[fi.cw].frame = dsp[fi.cw].frame - 1
        if(dsp[fi.cw].frame lt 1) then dsp[fi.cw].frame = 1
        widget_control,wd.sld_frm,SET_VALUE=dsp[fi.cw].frame
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    
    wd.scale: begin
        case ev.value of
            0: begin
                if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='Image is scaled to the maximum over the volume.'
                dsp[fi.cw].scale = !VOLUME
            end
            1: begin
                if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='Image is scaled to the maximum over the slice.'
                dsp[fi.cw].scale = !SLICE
            end
        endcase
        if !D.WINDOW gt -1 then refresh_display,fi,dsp,stc,wd,pref,IMAGE=dsp_image
    end

    wd.coneqoff: begin
        dsp[fi.cw].conadj = 0
        widget_control,wd.coneqoff,SENSITIVE=0
        widget_control,wd.coneqon,/SENSITIVE
        if !D.WINDOW gt -1 then refresh_display,fi,dsp,stc,wd,pref,IMAGE=dsp_image
    end
    wd.coneqon: begin
        dsp[fi.cw].conadj = 1
        widget_control,wd.coneqoff,/SENSITIVE
        widget_control,wd.coneqon,SENSITIVE=0
        if !D.WINDOW gt -1 then refresh_display,fi,dsp,stc,wd,pref,IMAGE=dsp_image
    end

    wd.sld_pln: begin
        widget_control,wd.sld_pln,GET_VALUE=i
        dsp[fi.cw].plane = i
        if(dsp[fi.cw].plane lt 1) then dsp[fi.cw].plane = 1
        if(dsp[fi.cw].plane gt dsp[fi.cw].zdim) then dsp[fi.cw].plane = dsp[fi.cw].zdim
        dsp_image = update_image(fi,dsp,wd,stc,pref)
        report_math_errors = !FALSE
    end
    wd.sld_frm: begin
        widget_control,wd.sld_frm,GET_VALUE=i
        dsp[fi.cw].frame = i
        if(dsp[fi.cw].frame lt 1) then dsp[fi.cw].frame = 1
        if(dsp[fi.cw].frame gt dsp[fi.cw].tdim) then dsp[fi.cw].frame = dsp[fi.cw].tdim
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.zoomin: begin
        fi.zoom(fi.n) = 2*dsp[fi.cw].zoom
        dsp[fi.cw].zoom = fi.zoom(fi.n)
        refresh_display,fi,dsp,stc,wd,pref,IMAGE=dsp_image
    end
    wd.zoomout: begin
        fi.zoom(fi.n) = .5*dsp[fi.cw].zoom
        dsp[fi.cw].zoom = fi.zoom(fi.n)
        refresh_display,fi,dsp,stc,wd,pref,IMAGE=dsp_image
    end
    wd.zoomtype: begin
        dsp[fi.cw].zoomtype = get_button(['Nearest neighbor','Bilinear','Cubic spline'],TITLE='Select interpolation method.')
        dsp[fi.cw].zoomtype = dsp[fi.cw].zoomtype + 1
        refresh_display,fi,dsp,stc,wd,pref,IMAGE=dsp_image
    end
    wd1.zoom: begin
        zoom,fi,wd,dsp,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    
    wd.local_scl: begin
        if(help.enable eq !TRUE) then begin
            value='Each image is scaled to its own maximum.'
            widget_control,help.id,SET_VALUE=value
        endif
        dsp[fi.cw].scale = !LOCAL
        refresh_display,fi,dsp,stc,wd,pref,IMAGE=dsp_image
        widget_control,wd.local_scl,SENSITIVE=0
        widget_control,wd.global_scl,/SENSITIVE
        widget_control,wd.fixed_scl,/SENSITIVE
        widget_control,wd.fixed_glbl_scl,/SENSITIVE
    end
    wd.global_scl: begin
        if help.enable eq !TRUE then widget_control,help.id, $
            SET_VALUE='Images are scaled to the maximum over all displayed images.'
        dsp[fi.cw].scale = !GLOBAL
        refresh_display,fi,dsp,stc,wd,pref,IMAGE=dsp_image
        widget_control,wd.local_scl,/SENSITIVE
        widget_control,wd.global_scl,SENSITIVE=0
        widget_control,wd.fixed_scl,/SENSITIVE
        widget_control,wd.fixed_glbl_scl,/SENSITIVE
    end
    wd.fixed_scl: begin
        if(help.enable eq !TRUE) then begin
            value='Images are scaled to (max - min)/NUMBER_OF_COLORS where max and min are specified by the user.  ' $
                +'Overflows and underflows are set to 0 and NUMBER_OF_COLORS respectively'
            widget_control,help.id,SET_VALUE=value
        endif
        dsp[fi.cw].scale = !FIXED
        dsp_image = get_image(dsp[fi.cw].zdim*(dsp[fi.cw].frame-1) + dsp[fi.cw].plane-1,fi,stc)
        max = max(dsp_image)
        min = min(dsp_image)
        if(abs(max) gt abs(min)) then begin
        dsp[fi.cw].fixed_min = -abs(max)
            dsp[fi.cw].fixed_max =  abs(max)
        endif else begin
        dsp[fi.cw].fixed_min = -abs(min)
            dsp[fi.cw].fixed_max =  abs(min)
        endelse
        value = strarr(2)
        label = strarr(2)
        value(0) = string(dsp[fi.cw].fixed_max)
        value(1) = string(dsp[fi.cw].fixed_min)
        label(0) = string("Maximum: ")
        label(1) = string("Minimum: ")
        minmax = get_str(2,label,value)
        dsp[fi.cw].fixed_max = float(minmax(0))
        dsp[fi.cw].fixed_min = float(minmax(1))
        refresh_display,fi,dsp,stc,wd,pref,IMAGE=dsp_image
        widget_control,wd.local_scl,/SENSITIVE
        widget_control,wd.global_scl,/SENSITIVE
        widget_control,wd.fixed_scl,/SENSITIVE
        widget_control,wd.fixed_glbl_scl,/SENSITIVE
    end
    wd.fixed_glbl_scl: begin
        fixed_glbl_scl,dsp,help,fi,stc
        refresh_display,fi,dsp,stc,wd,pref,IMAGE=dsp_image
        widget_control,wd.local_scl,/SENSITIVE
        widget_control,wd.global_scl,/SENSITIVE
        widget_control,wd.fixed_scl,/SENSITIVE
        widget_control,wd.fixed_glbl_scl,SENSITIVE=0
    end
    wd.radview: begin
        dsp[fi.cw].orientation = !RADIOL
        refresh_display,fi,dsp,stc,wd,pref,IMAGE=dsp_image
        widget_control,wd.neuroview,/SENSITIVE
        widget_control,wd.radview,SENSITIVE=0
    end
    wd.neuroview: begin
        dsp[fi.cw].orientation = !NEURO
        refresh_display,fi,dsp,stc,wd,pref,IMAGE=dsp_image
        widget_control,wd.radview,/SENSITIVE
        widget_control,wd.neuroview,SENSITIVE=0
    end

    wd.crop_custom: begin
        crop,cstm,dsp
    end
    wd.build_2x2: begin
        cstm.cur_window = cstm.nwindows
        cstm.nwindows = cstm.nwindows + 1
        title = string(cstm.cur_window,FORMAT='("Custom window ",i3)')
        custbase = widget_base(TITLE=title,/COLUMN)
        wd.build_draw = widget_draw(custbase,xsize= !CUSTOM_XSZ, $
        ysize= !CUSTOM_XSZ,RETAIN=2,KILL_NOTIFY=string('custom_dead'))
        widget_control,custbase,/REALIZE
        widget_control,GET_VALUE=custom_id,wd.build_draw
        cstm.id(cstm.cur_window) = custom_id
        cstm.draw(cstm.cur_window) = wd.build_draw
        cstm.base(cstm.cur_window) = custbase
        cstm.init = !TRUE
        hdr = *fi.hdr_ptr(fi.n)
        if(cstm.crop(1) gt 0) then begin
            xdim = cstm.crop(1) - cstm.crop(0) + 1
            ydim = cstm.crop(3) - cstm.crop(2) + 1
        endif else begin
            xdim = dsp[fi.cw].xdim
            ydim = dsp[fi.cw].ydim
        endelse
        bltimg = fltarr(xdim,ydim,!MAX_BLTIMG)
        cstm.nblt = 0
        wset,dsp[fi.cw].image_index[fi.cw]
        widget_control,wd.build_addimg,/SENSITIVE
        widget_control,wd.build_annotate,/SENSITIVE
    end
    wd.build_custom: begin
        value = strarr(2)
        label = strarr(2)
        value(0) = string('512')
        value(1) = string('512')
        label(0) = string("X dimension: ")
        label(1) = string("Y dimension: ")
        sizes = get_str(2,label,value)
        xsize = fix(sizes(0))
        ysize = fix(sizes(1))
        cstm.cur_window = cstm.nwindows
        cstm.nwindows = cstm.nwindows + 1
        title = string(cstm.cur_window,FORMAT='("Custom window ",i3)')
        custbase = widget_base(TITLE=title,/COLUMN)
        wd.build_draw = widget_draw(custbase,xsize=xsize, $
            ysize=ysize,RETAIN=2,KILL_NOTIFY=string('custom_dead'))
        widget_control,custbase,/REALIZE
        widget_control,GET_VALUE=custom_id,wd.build_draw
        cstm.id(cstm.cur_window) = custom_id
        cstm.init = !TRUE
        hdr = *fi.hdr_ptr(fi.n)
        if(cstm.crop(1) gt 0) then begin
            xdim = cstm.crop(1) - cstm.crop(0) + 1
            ydim = cstm.crop(3) - cstm.crop(2) + 1
        endif else begin
            xdim = dsp[fi.cw].xdim
            ydim = dsp[fi.cw].ydim
        endelse
        bltimg = fltarr(xdim,ydim,!MAX_BLTIMG)
        cstm.nblt = 0
        wset,dsp[fi.cw].image_index[fi.cw]
        widget_control,wd.build_addimg,/SENSITIVE
        widget_control,wd.build_annotate,/SENSITIVE
    end
    wd.build_addimg: begin
        add_custom_img
    end
    wd.build_annotate: begin
        wset,cstm.id(cstm.cur_window)
        annotate,WINDOW=cstm.id(cstm.cur_window)
        wset,dsp[fi.cw].image_index[fi.cw]
    end
    wd.select_custom: begin
        if(help.enable eq !TRUE) then begin
            value='Selects window that will be stored with the "Write GIF image" button.'
            widget_control,help.id,SET_VALUE=value
        endif
        labels = strarr(cstm.nwindows)
        for i=0,cstm.nwindows-1 do $
        labels(i) = strcompress(string(i),/REMOVE_ALL)
        index = get_button(labels)
        cstm.cur_window = index
    end
    wd.overlay_actmap: begin
        overlay_actmap,fi,dsp,wd,st,stc,help,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end

    wd.atlas_transform: begin
        dsp_image = update_image(fi,dsp,wd,stc,pref)
        atlas_transform,fi,dsp,wd,stc,help,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end

    wd.atlas_transform_c: begin
        atlas_transform_c,fi,dsp,wd,stc,help
    end
    wd1.threshold_image: begin
        threshold_display
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end


    ;wd1.threshold_cluster: begin
    ;    threshold_cluster,fi,wd,dsp,help,pref,dsp_image,stc
    ;    dsp_image = update_image(fi,dsp,wd,stc,pref)
    ;end
    ;wd1.fidl_threshold_cluster: begin
    ;    fidl_threshold_cluster,fi,wd,dsp,help,pref,dsp_image,stc
    ;    dsp_image = update_image(fi,dsp,wd,stc,pref)
    ;end
    ;wd1.fidl_threshold_cluster2: begin
    ;    fidl_threshold_cluster,fi,wd,dsp,help,pref,dsp_image,stc,/MULTIPLE
    ;    dsp_image = update_image(fi,dsp,wd,stc,pref)
    ;end
    ;START140825
    wd1.fidl_threshclus_idl: begin
        fidl_threshclus,fi,wd,dsp,help,pref,dsp_image,stc,/IDL
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.fidl_threshclus: begin
        fidl_threshclus,fi,wd,dsp,help,pref,dsp_image,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end




    wd1.subtract_images: begin

        ;subtract_images,fi,wd,dsp,help,stc
        ;START150112
        subtract_images,fi,st,dsp,wd,stc,help,glm,pref

        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.divide_images: begin
        divide_images,fi,wd,dsp,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.multiply_images: begin
        multiply_images,fi,st,dsp,wd,stc,help,glm,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.scale_images: begin
        scale_images,fi,wd,dsp,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    ;wd1.power_spectrum: begin
    ;    power_spectrum,fi,wd,dsp,help,pr,stc
    ;    dsp_image = update_image(fi,dsp,wd,stc,pref)
    ;end

    ;START150326
    ;wd1.compute_pearson: begin
    ;    compute_pearson,fi,wd,dsp,help,pr,stc,pref
    ;end

    wd1.compute_boynton: begin
        compute_boynton,fi,wd,dsp,help,stc,pref
    end

    ;START160614
    wd1.fidl_flipglm: begin
        fidl_flipglm,fi,help,pref
    end


    wd1.fidl_flip: begin
        fidl_flip,fi,wd,dsp,help,pr,stc,pref
    end
    wd1.fidl_flipnew: begin
        fidl_flipnew,fi,wd,dsp,help,pr,stc,pref
    end
    wd1.fidl_psd: begin
        fidl_psd,fi,wd,dsp,help,stc,pref
    end
    wd1.fidl_histogram: begin
        fidl_histogram,fi,wd,dsp,help,stc,pref
    end
    wd1.fidl_hemitxt: begin
        fidl_hemitxt,fi,wd,dsp,help,stc,pref
    end
    wd1.fidl_circor: begin
        fidl_circor,fi,wd,dsp,help,stc,pref
    end
    wd1.add_scalar: begin
        add_scalar,fi,wd,dsp,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.clip_image: begin
        clip_image,fi,wd,dsp,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.fidl_avg: begin

        ;fidl_avg,fi,wd,dsp,help,stc
        ;START140421
        fidl_avg,fi,wd,dsp,help,stc,pref

        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end

    ;START120720
    wd1.fidl_zeromean: begin
        fidl_zeromean,fi,wd,dsp,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end

    wd1.fidl_minmax: begin
        fidl_minmax,fi,wd,dsp,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.fidl_lminusrdivlplusr: begin
        fidl_lminusrdivlplusr,fi,wd,dsp,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end

    ;START170404
    ;wd1.fidl_sumconcpair: begin
    ;    wd1.fidl_sumconcpair,fi,wd,dsp,help,stc
    ;    dsp_image = update_image(fi,dsp,wd,stc,pref)
    ;end

    wd1.low_bandwidth: begin
        dsp[fi.cw].low_bandwidth = !TRUE
        widget_control,wd1.low_bandwidth,SENSITIVE=0
        widget_control,wd1.high_bandwidth,/SENSITIVE
    end
    wd1.high_bandwidth: begin
        dsp[fi.cw].low_bandwidth = !FALSE
        widget_control,wd1.low_bandwidth,/SENSITIVE
        widget_control,wd1.high_bandwidth,SENSITIVE=0
    end

    wd1.t_to_z_IDL: begin
        t_to_z,fi,dsp,wd,stc,help,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.fidl_t_to_z: begin
        fidl_t_to_z,fi,dsp,wd,stc,help,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.f_to_z_IDL: begin
        f_to_z,fi,dsp,wd,stc,help
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.f_to_z_C: begin
        transform_f_to_z,fi,wd,dsp,help,pref,dsp_image,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end

    wd1.fidl_r_to_z: begin
        fidl_r_to_z,fi,wd,dsp,help,pref,dsp_image,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.fidl_r_to_zC: begin
        fidl_r_to_z,fi,wd,dsp,help,pref,dsp_image,stc,/CVERSION
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end

    wd1.fidl_p_to_z: begin
        fidl_p_to_z,fi,wd,dsp,help,pr,stc,pref
    end

    wd1.sum_images: begin

        ;sum_images,fi,wd,dsp,help,stc
        ;START110920
        sum_images,fi,wd,dsp,help,stc,pref

        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.view: begin
        reslice,fi,wd,dsp,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.color_scale: begin
        showcolors,'Main',wd,dsp,fi
    end
    wd.display_all: begin
        display_all,dsp,fi,help,stc,pref
    end
    wd.view_images: begin
        if vw.num_windows eq 0 then $
            view_images $
        else $
            create_new_view_window,vw,fi
    end
    wd.view26d4: begin
        if vw.num_windows eq 0 then begin
            view26d4 
        endif else begin
            create_new_view_window,vw,fi
        endelse
    end
    wd1.display_comp: begin
        display_comp,dsp,fi,help,stc,pref
    end
    wd.new_wrkg_wdw: begin
        for i=!MAX_WORKING_WINDOWS-1,0,-1 do begin
            if wd.draw[i] lt 0 then fi.cw = i
        endfor
        dsp[fi.cw].file_num = fi.n
        fi.current_colors = fi.n
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    ;wd.view3d: begin
    ;    idx_list = get_bool_list(fi.tails(0:fi.nfiles-1),TITLE='Select files')
    ;    nf = fix(total(idx_list))
    ;    slicer_ptr = ptrarr(nf)
    ;    slicer_lab = strarr(nf)
    ;    jf = 0
    ;    for idx=0,fi.nfiles-1 do begin
    ;        if(idx_list[idx] ne 0) then begin
    ;            hdr = *fi.hdr_ptr[idx]
    ;            img = fltarr(hdr.xdim,hdr.ydim,hdr.zdim)
    ;            for z=0,hdr.zdim-1 do $
    ;                img(*,*,z) = get_image(z,fi,stc,FILNUM=idx+1)
    ;            slicer_ptr[jf] = ptr_new(img)
    ;            slicer_lab[jf] = fi.tails[idx]
    ;            jf = jf + 1
    ;         endif
    ;    endfor
    ;    slicer3_jmo,slicer_ptr,DATA_NAMES=slicer_lab,/DETACH,GROUP=wd.leader,/MODAL
    ;    load_colortable,fi,dsp
    ;end
    wd.print_all: begin
        if(help.enable eq !TRUE) then begin
            value=string("Prints all images in current frame to ",pref.printer_name)
            widget_control,help.id,SET_VALUE=value
        endif
        print_all,dsp,fi,pref
    end
    wd.refresh: begin
        erase
        low_bandwidth = dsp[fi.cw].low_bandwidth
        dsp[fi.cw].low_bandwidth = !FALSE  ; Always display when manually commanded.
        refresh_display,fi,dsp,stc,wd,pref,IMAGE=dsp_image
        dsp[fi.cw].low_bandwidth = low_bandwidth
    end
    wd.help_info: begin
        help_base = widget_base(/COLUMN,GROUP_LEADER=!FMRI_LEADER)
        help.id = widget_text(help_base,KILL_NOTIFY=string('help_dead'),/WRAP, $
        VALUE=string("Context-sensitive help messages will appear here.  "), $
        XSIZE=80,YSIZE=20,/SCROLL)
        widget_control,help_base,/REALIZE
        xmanager,'stats_exec',help_base
        help.enable = !TRUE
    end
    wd.help: begin
        text = strarr(1)
        text(0)="Send all questions to /dev/null@npg, or equivalently, to systems@npg."
        i = widget_message(text,/ERROR)
    end
    ;********** Processing functions. ***************
    wd.plnstats: begin
        if n_elements(dsp_image) gt 0 then print_stats,dsp_image,dsp,wd,fi,stc
    end
    wd.fidl_swap: begin
        fidl_swap,fi,dsp,wd,help,pref
    end
    wd.voxel_values: begin
        compute_voxel_value
    end
    wd.fidl_4dfp_to_text: begin
        fidl_4dfp_to_text,fi,dsp,wd,help,stc,pref
    end
    wd.fidl_text_to_4dfp: begin
        fidl_text_to_4dfp,fi,dsp,wd,help,pref
    end
    wd.fidl_txt2conc_subcol: begin
        fidl_txt2conc_subcol,fi,dsp,wd,help,pref
    end
    wd.fidl_collatecol: begin
        fidl_collatecol,fi,dsp,wd,glm,help,stc,pref
    end

    wd1.comp_linmod: begin
        linmod_c,fi,dsp,wd,glm,help,stc,pref
    end
    wd1.compute_residuals: begin
        stat=compute_residuals(fi,dsp,wd,glm,help,stc,pref)
    end
    ;wd1.fidl_autocorrelation: begin
    ;    fidl_autocorrelation,fi,dsp,wd,glm,help,stc,pref
    ;end

    ;START150325
    wd1.fidl_checkglm: begin
        fidl_checkglm,fi,dsp,wd,glm,help,stc,pref
    end
    wd1.fidl_glmcond: begin
        fidl_glmcond,fi,dsp,wd,glm,help,stc,pref
    end


    wd1.fidl_rename_effects: begin
        fidl_rename_effects,fi,dsp,wd,glm,help,stc,pref
    end
    wd1.fidl_rename_paths: begin
        fidl_rename_paths,fi,dsp,wd,glm,help,stc,pref
    end
    wd1.fidl_eog: begin
        fidl_eog,fi,dsp,wd,glm,help,stc,pref
    end

    ;START141001
    wd1.fidl_motionreg: begin
        fidl_motionreg,fi,dsp,wd,glm,help,stc,pref
    end

    ;START140911
    ;wd1.fidl_motion: begin
    ;    fidl_motion,fi,dsp,wd,glm,help,stc,pref
    ;end



    wd1.fidl_anova2: begin

        ;fidl_anova2,fi,dsp,wd,glm,help,stc,pref
        ;START151210
        ;fidl_anova3,fi,dsp,wd,glm,help,stc,pref
        ;START160120
        fidl_anova4,fi,dsp,wd,glm,help,stc,pref

    end
    wd1.compute_avg_zstat: begin
        lizard=compute_avg_zstat(fi,dsp,wd,glm,help,stc,pref)
    end
    wd1.fidl_2ndlevelmodel: begin
        fidl_2ndlevelmodel,fi,dsp,wd,glm,help,stc,pref
    end
    wd1.fidl_slopesintercepts: begin
        fidl_slopesintercepts,fi,dsp,wd,glm,help,stc,pref
    end
    wd1.fidl_lmerr: begin

        ;fidl_anova2,fi,dsp,wd,glm,help,stc,pref,/LMERR
        ;START151124
        ;fidl_anova3,fi,dsp,wd,glm,help,stc,pref,/LMERR
        ;START160114
        fidl_anova4,fi,dsp,wd,glm,help,stc,pref,/LMERR

    end
    wd1.fidl_mvpa2: fidl_mvpa,fi,dsp,wd,glm,help,stc,pref
    wd1.fidl_anova_ss: begin
        fidl_anova_ss,fi,dsp,wd,glm,help,stc,pref
    end

    ;wd1.compute_epsilon: begin
    ;    compute_epsilon,fi,dsp,wd,glm,help,stc,pref
    ;end
    ;wd1.glm_adjusted_df: begin
    ;    glm_adjusted_df,fi,dsp,wd,glm,help,stc
    ;end

    wd1.fidl_ttest: begin
        rtn=fidl_ttest(fi,dsp,wd,glm,help,stc,pref)
    end

    wd1.fidl_ftest_ss: begin

        ;fidl_anova2,fi,dsp,wd,glm,help,stc,pref,/FTEST_SS
        ;START151210
        ;fidl_anova3,fi,dsp,wd,glm,help,stc,pref,/FTEST_SS
        ;START160120
        fidl_anova4,fi,dsp,wd,glm,help,stc,pref,/FTEST_SS

    end

    ;START140311
    wd1.fidl_ttest_ss: begin
        rtn=fidl_ttest(fi,dsp,wd,glm,help,stc,pref,/SS)
    end


    ;wd1.correlation_analysis: begin
    ;    across_subject_t_test,fi,dsp,wd,glm,help,stc,/CORRELATION
    ;end
    ;wd1.scatter_plots: begin
    ;    correlation_scatter_plots,fi
    ;end

    wd1.extract_mag: begin
        if fi.nfiles eq 0 then wd1_load_linmod,fi,dsp,wd,glm,help
        if fi.nfiles gt 0 then begin
            dsp_image = update_image(fi,dsp,wd,stc,pref)
            extract_mag,fi,dsp,wd,glm,help,stc,pref
            dsp_image = update_image(fi,dsp,wd,stc,pref)
        endif
    end
    wd1.fidl_mvpa: fidl_mvpa,fi,dsp,wd,glm,help,stc,pref

    ;START170614
    wd1.fidl_logreg_ss2:stat=fidl_logreg_ss(fi,dsp,wd,glm,help,stc,pref)

    wd1.compute_zstat: begin
        if fi.nfiles eq 0 then wd1_load_linmod,fi,dsp,wd,glm,help
        if fi.nfiles gt 0 then begin
            dsp_image = update_image(fi,dsp,wd,stc,pref)
            compute_zstat,fi,dsp,wd,glm,help,stc,pref
            dsp_image = update_image(fi,dsp,wd,stc,pref)
        endif
    end
    wd1.apply_linmod: begin
        if fi.nfiles eq 0 then wd1_load_linmod,fi,dsp,wd,glm,help
        if fi.nfiles gt 0 then begin
            dsp_image = update_image(fi,dsp,wd,stc,pref)
            apply_linmod_lag,fi,dsp,wd,glm,help,stc,pref,/APPLY_LINMOD
            dsp_image = update_image(fi,dsp,wd,stc,pref)
        endif
    end
    wd1.apply_linmod_lag: begin
        if fi.nfiles eq 0 then wd1_load_linmod,fi,dsp,wd,glm,help
        if fi.nfiles gt 0 then begin
            dsp_image = update_image(fi,dsp,wd,stc,pref)
            apply_linmod_lag,fi,dsp,wd,glm,help,stc,pref
            dsp_image = update_image(fi,dsp,wd,stc,pref)
        endif
    end
    wd1.extract_timecourse: begin
        ;compute_avg_zstat,fi,dsp,wd,glm,help,stc,pref,/TC
        ;START130118
        lizard=compute_avg_zstat(fi,dsp,wd,glm,help,stc,pref,/TC)
    end
    wd1.compute_tc_max_deflect: begin
        compute_tc_max_deflect,fi,dsp,wd,glm,help,stc,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.compute_avg_zstat_boy: begin
        ;compute_avg_zstat,fi,dsp,wd,glm,help,stc,pref,/BOY
        ;START130118
        lizard=compute_avg_zstat(fi,dsp,wd,glm,help,stc,pref,/BOY)
    end
    wd1.fidl_tc_ss: begin
        fidl_tc_ss,fi,dsp,wd,glm,help,stc,pref
    end
    wd1.fidl_logreg_ss: begin
        stat=fidl_logreg_ss(fi,dsp,wd,glm,help,stc,pref)
    end
    wd1.fidl_cc: begin
        fidl_cc,fi,dsp,wd,glm,help,stc,pref
    end
    wd1.fidl_fano: begin
        fidl_tc_ss,fi,dsp,wd,glm,help,stc,pref,/FANO
    end
    wd1.extract_mean: begin
        extract_mean,fi,dsp,wd,glm,help,stc,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.extract_sdev: begin
        hd = *fi.hdr_ptr(fi.n)
        mom = hd.mother
        hdr = *fi.hdr_ptr(mom)
        if(ptr_valid(glm[mom].grand_mean)) then $
            grand_mean = *glm[mom].grand_mean $
        else $
            stat = widget_message('Mean has not been calculated, expressing as raw MR values.')
        if(ptr_valid(glm[mom].sd)) then begin
            sd = *glm[mom].sd
            name = strcompress(string(fi.tails(mom),FORMAT='(a,"_sd")'),/REMOVE_ALL)

            ;put_image,sd,fi,wd,dsp,name,hdr.xdim,hdr.ydim,hdr.zdim,1,!FLOAT,hdr.dxdy,hdr.dz,hdr.scl,!FLOAT_ARRAY,mom,-1,MODEL=mom,IFH=hdr.ifh
            ;START150731
            put_image,sd,fi,wd,dsp,name,hdr.xdim,hdr.ydim,hdr.zdim,1,!FLOAT,hdr.dxdy,hdr.dz,hdr.scl,!FLOAT_ARRAY,mom,MODEL=mom,IFH=hdr.ifh

            dsp_image = update_image(fi,dsp,wd,stc,pref)
            print,'Standard deviation expressed in raw mr units.'
        endif else begin
            stat = widget_message('Standard deviation was not calculated.')
        endelse
    end
    wd1.extract_trend: begin
        extract_trend,fi,dsp,wd,glm,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.extract_cov: begin
        hdr = *fi.hdr_ptr(fi.n)
    ;;;    if(ptr_valid(glm[fi.n].ATAm1)) then begin
    ;;;        ATAm1 = *glm[fi.n].ATAm1
            lcmain = get_boolean('Extract main effects only?',['Yes','No'])
            if(ptr_valid(glm[fi.n].A)) then A = *glm[fi.n].A
            widget_control,/HOURGLASS
            ATA = transpose(A)#A
            widget_control,/HOURGLASS
            ATAm1 = float(pinv(ATA,0.))
            if(lcmain eq !TRUE) then $
                xdim = glm[fi.n].n_interest $
            else $
                xdim = n_elements(ATAm1[*,0])
            ydim = xdim
            output = fltarr(xdim,ydim,3)
            img = fltarr(xdim,ydim)
            output[*,*,2] = ATA[0:xdim-1,0:ydim-1]
            output[*,*,1] = ATAm1[0:xdim-1,0:ydim-1]
            prof = fltarr(xdim)
            widget_control,/HOURGLASS
            for i=0,xdim-1 do begin
                for j=0,ydim-1 do $
                    img[i,j] = ATAm1[i,j]/sqrt(ATAm1[i,i]*ATAm1[j,j])
                prof[i] = ATAm1[i,i]
            endfor
            output[*,*,0] = img
            name = strcompress(string(fi.tails(fi.n),FORMAT='(a,"_cov")'),/REMOVE_ALL)

            ;put_image,output,fi,wd,dsp,name,xdim,ydim,3,1,!FLOAT,1.,1.,1.,!FLOAT_ARRAY,fi.n,-1,MODEL=fi.n,IFH=hdr.ifh
            ;START150731
            put_image,output,fi,wd,dsp,name,xdim,ydim,3,1,!FLOAT,1.,1.,1.,!FLOAT_ARRAY,fi.n,MODEL=fi.n,IFH=hdr.ifh

            dsp_image = update_image(fi,dsp,wd,stc,pref)
    ;;;    endif else begin
    ;;;        stat = widget_message('Covariance has not been calculated.')
    ;;;        A = *glm[fi.n].A
    ;;;        ATA = transpose(A)#A
    ;;;        dim = n_elements(ATA[*,0])
    ;;;        name = strcompress(string(fi.tails(fi.n),FORMAT='(a,"_ATA")'),/REMOVE_ALL)
    ;;;        put_image,ATA,fi,wd,dsp,name,dim,dim,1,1,!FLOAT,1.,1.,1.,!FLOAT_ARRAY,fi.n,-1,MODEL=fi.n,IFH=hdr.ifh
    ;;;    endelse
    end
    wd1.fidl_glmsavecorr: fidl_glmsavecorr,fi,wd,dsp,help,stc 

    wd1.fix_grand_mean: fix_grand_mean,fi,wd,dsp,help,stc

    wd1.fidl_ac: stat=compute_residuals(fi,dsp,wd,glm,help,stc,pref,/AUTOCORR)
    wd1.fidl_crosscorr: stat=compute_residuals(fi,dsp,wd,glm,help,stc,pref,/CROSSCORR)
    wd1.fidl_crosscov: stat=compute_residuals(fi,dsp,wd,glm,help,stc,pref,/CROSSCOV)
    wd1.fidl_cov: stat=compute_residuals(fi,dsp,wd,glm,help,stc,pref,/COV)
    wd1.fidl_pca: stat=compute_residuals(fi,dsp,wd,glm,help,stc,pref,/PCA)

    ;START170208
    wd1.fidl_pcat:fidl_pcat,fi,dsp,wd,glm,help,stc,pref 
    
    ;START120118
    wd1.fidl_levene: begin 
        fidl_levene,glm,dsp,fi,pref,wd,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end


    wd1.extract_fzstat: begin
        if fi.nfiles eq 0 then wd1_load_linmod,fi,dsp,wd,glm,help
        if fi.nfiles gt 0 then begin
            dsp_image = update_image(fi,dsp,wd,stc,pref)
            extract_fzstat,fi,dsp,wd,glm,help
            dsp_image = update_image(fi,dsp,wd,stc,pref)
        endif
    end
    wd1.fidl_extract_fzstat: begin
        fidl_extract_fzstat,fi,dsp,wd,stc,glm,help
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end

    ;START120224
    ;wd1.fidl_checkglm: begin
    ;    fidl_checkglm,fi,dsp,wd,glm,help,stc,pref
    ;end
    
    wd1.save_linmod: begin
        hdr = *fi.hdr_ptr(fi.n)
        filename = save_linmod(fi,dsp,wd,glm,help,stc,pref,fi.n,hdr.model)
    end
    wd1.load_linmod: begin
        wd1_load_linmod,fi,dsp,wd,glm,help,/MULTIPLE_FILES
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    
    wd1.load_image_filter: begin
        str = get_str(1,'Default image filter: ','*4dfp.img',TITLE='Enter default filter')
        fi.image_filter = str[0]
    end
    
    wd1.load_glm_filter: begin
        str = get_str(1,'Default glm filter: ','*.glm',TITLE='Enter default filter')
        fi.glm_filter = str[0]
    end
    wd1.show_linmod: begin
        show_dsgn_matrix,fi,dsp,wd,glm,help,cstm
    end
    
    ;wd1.glm_simulate: begin
    ;    hdr = *fi.hdr_ptr(fi.n)
    ;    glm_simulate,glm(hdr.model),dsp,fi,wd,pr,help,pref
    ;end
    ;wd1.glm_sim_paradigm: begin
    ;    hd = *fi.hdr_ptr(fi.n)
    ;    sngl_trial = glm_sim_paradigm(help,pref)
    ;    fi.paradigm_single(hd.mother) = ptr_new(sngl_trial)
    ;end

    ;wd1.def_single_linmod_new: begin
    ;    stat = define_single_dsgn_matrix_new(params,glm,dsp,fi,pref,wd,help,stc)
    ;end
    ;START170130
    wd1.def_single_linmod_new: begin
        stat=define_single_dsgn_matrix_new2(params,glm,dsp,fi,pref,wd,help,stc)
    end

    ;START170109
    wd1.def_single_linmod_new_trials:begin
        stat=define_single_dsgn_matrix_new2(params,glm,dsp,fi,pref,wd,help,stc,/TRIALS)
    end

    wd1.def_block_linmod: begin
        if define_block_dsgn_matrix(dsp,fi,stc,pref,wd,help,glm) eq 0 then dsp_image=update_image(fi,dsp,wd,stc,pref)
    end
    wd1.export_dsgn_matrix: begin
        export_dsgn_matrix,fi,dsp,wd,glm,help,stc
    end
    wd1.import_dsgn_matrix: begin
        labels = strarr(fi.nfiles)
        index = intarr(fi.nfiles)
        nglm = 0
        for idx=0,fi.nfiles-1 do begin
            if(ptr_valid(glm(idx).A)) then begin
                labels[nglm] = fi.tails[idx]
                index[nglm] = idx
                nglm = nglm + 1
            endif
        endfor
        if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='Select that glm file that was used to export the ' $
            +'ancestor of the design matrix being imported.'
        labels = labels[0:nglm-1]
        idx = get_button(labels,TITLE='Select glm template.')
        idx = index[idx]
        import_dsgn_matrix,fi,dsp,wd,glm,idx,help,stc
        define_contrast,glm,idx,fi,help,dsp,wd,stc,pref
    end
    wd1.dsgn_matrix_metrics: begin
        glm_metrics,glm,fi,dsp,wd,help,stc,pref
    end
    wd1.fidl_dsgn_matrix_metrics: begin
        fidl_glm_metrics,glm,fi,dsp,wd,help,stc,pref
    end
    wd1.def_contrast: begin
        goto,def_contrast
    end
    wd1.def_contrast1: begin
        def_contrast:
        define_contrast,glm,model,fi,help,dsp,wd,stc,pref
        if fi.nfiles gt 0 then dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.glm_boynton_mod: begin
        hdr = *fi.hdr_ptr(fi.n)
        nc = glm(hdr.model).nc
        if(ptr_valid(glm(hdr.model).delay)) then $
            delay = *glm(hdr.model).delay $
        else $
            delay = fltarr(nc)
        if(ptr_valid(glm(hdr.model).stimlen)) then $
            stimlen = *glm(hdr.model).stimlen $
        else $
            stimlen = fltarr(nc)
        labrow = strarr(nc)
        for i=0,nc-1 do $
            labrow[i] = strcompress(string(i+1),/REMOVE_ALL)
        values = fltarr(nc,2)
        values[*,0] = delay
        values[*,1] = stimlen
        vals = get_array(nc,2,labrow,['Contrast','Delay','Duration'],values)
        delay = float(vals[*,0])
        stimlen = float(vals[*,1])
        if(ptr_valid(glm(hdr.model).delay)) then $
            ptr_free,glm(hdr.model).delay
        glm(hdr.model).delay = ptr_new(delay)
        if(ptr_valid(glm(hdr.model).stimlen)) then $
            ptr_free,glm(hdr.model).stimlen
        glm(hdr.model).stimlen = ptr_new(stimlen)
    end
    
    ;wd1.edit_linmod: begin
    ;    hdr = *fi.hdr_ptr(fi.n)
    ;    edit_dsgn_matrix,fi,glm
    ;    show_dsgn_matrix,glm(hdr.model),dsp,cstm,fi.tails[fi.n]
    ;end
    
    wd.actmap: begin
        actmap,fi,help
    end

    ;START160226
    ;wd.define_stimulus: begin
    ;    hd = *fi.hdr_ptr(fi.n)
    ;    hdr = *fi.hdr_ptr(hd.mother)
    ;    if hdr.array_type eq !STITCH_ARRAY then begin
    ;        tdim = stc[hd.mother].tdim_all
    ;        tdim_max = max(*stc[hd.mother].tdim_file)
    ;        nfile = stc[hd.mother].n
    ;    endif else begin
    ;        tdim = hdr.tdim
    ;        tdim_max = tdim
    ;        nfile = 1
    ;    endelse
    ;    if hdr.tdim le 2 then begin
    ;        stat=dialog_message('Paradigm can only be defined for time-course data.',/ERROR)
    ;        return
    ;    endif
    ;    rtn = define_paradigm(tdim_max,pref)
    ;    if rtn.status eq !OK then begin
    ;        lenpar = n_elements(rtn.paradigm)
    ;        if hdr.array_type eq !STITCH_ARRAY then begin
    ;            paradigm = fltarr(tdim_all)
    ;            if lenpar eq tdim_max+1 then begin
    ;                tdim_sum = *stc[hd.mother].tdim_sum
    ;                tdim_file = *stc[hd.mother].tdim_file
    ;                for i=0,nfile-1 do begin
    ;                    t0 = tdim_sum[i]
    ;                    t1 = t0 + tdim_file[i] - 1
    ;                    paradigm[t0:t1] = rtn.paradigm[0:tdim_file[i]-1]
    ;                    fi.paradigm(hd.mother) = ptr_new(paradigm)
    ;                    fi.default_paradigm = ptr_new(paradigm)
    ;                endfor
    ;            endif else begin
    ;                paradigm[0:lenpar-1] = rtn.paradigm[0:lenpar-1]
    ;                fi.paradigm(hd.mother) = ptr_new(paradigm)
    ;                fi.default_paradigm = ptr_new(paradigm)
    ;            endelse
    ;        endif else begin
    ;            paradigm = rtn.paradigm(0:lenpar-1)
    ;            fi.paradigm(hd.mother) = ptr_new(paradigm)
    ;            fi.default_paradigm = ptr_new(paradigm)
    ;        endelse
    ;        fi.paradigm_code(hd.mother) =  $
    ;        encode_paradigm(paradigm[0:hdr.tdim-1],hdr.tdim)
    ;        fi.paradigm_loaded(hd.mother) = !TRUE
    ;    endif else begin
    ;        fi.paradigm(hd.mother) = ptr_new()
    ;        stat=widget_message(rtn.msg,/ERROR)
    ;    endelse
    ;end
    ;wd.edit_stimulus: begin
    ;    hd = *fi.hdr_ptr(fi.n)
    ;    hdr = *fi.hdr_ptr(hd.mother)
    ;    if(hdr.tdim le 2) then begin
    ;        stat=widget_message('Paradigm can only be defined for time-course data. ',/ERROR)
    ;        return
    ;    endif
    ;    if(ptr_valid(fi.paradigm(hd.mother))) then begin
    ;        paradigm = *fi.paradigm(hdr.mother)
    ;    endif else begin
    ;        paradigm = fltarr(hdr.tdim)
    ;        paradigm[*] = 1
    ;        paradigm[0:4] = 0
    ;    endelse
    ;    rtn = edit_paradigm(paradigm)
    ;    paradigm = rtn.paradigm(0:hdr.tdim-1)
    ;    fi.paradigm(hd.mother) = ptr_new(paradigm)
    ;    fi.paradigm_code(hd.mother) = encode_paradigm(rtn.paradigm,hdr.tdim)
    ;    fi.paradigm_loaded(hd.mother) = !TRUE
    ;end
    ;wd.show_stimulus: begin
    ;    plot_stimulus,fi,dsp
    ;end
    ;wd.save_stimulus: begin
    ;    stat = save_paradigm(fi)
    ;end

    ;wd.covariance: begin
    ;    get_cov_ui,fi,pr,dsp,wd,stc,help,pref,glm
    ;    dsp_image = update_image(fi,dsp,wd,stc,pref)
    ;    dsp_image = get_image(dsp[fi.cw].zdim*(dsp[fi.cw].frame-1) + dsp[fi.cw].plane-1,fi,stc)
    ;end
    ;wd.analyze_cc: begin
    ;    analyze_cc,fi,pr,dsp,wd,stc,help,pref,glm
    ;end
    ;wd.pca_cc: begin
    ;    pca_cc,fi,pr,dsp,fi,wd,stc,help,pref,glm
    ;end
    ;wd.cov_butter: begin
    ;    butter_cc,fi,pr,dsp,wd,stc,help,pref,glm
    ;end
    ;wd.create_cc_src: begin
    ;    create_cc_src,fi,pr,dsp,wd,stc,help,pref
    ;end
    ;wd.time_corr: begin
    ;    get_time_corr,fi,pr,dsp,wd,stc,help,pref
    ;end

    ;START150326
    ;wd.mult_comp_monte_carlo: begin
    ;    mult_comp_monte_carlo,fi,wd,dsp,help,pref,dsp_image,stc
    ;    dsp_image = update_image(fi,dsp,wd,stc,pref)
    ;end

    wd.mult_comp_monte_carloC: begin
        mult_comp_monte_carlo,fi,wd,dsp,help,pref,dsp_image,stc,/CVERSION
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end

    ;START140711
    wd.fidl_fdr: begin
        fidl_fdr,fi,wd,dsp,help,pref,dsp_image,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end

    wd.generate_mask: begin

        ;generate_mask,fi,wd,dsp,help,stc,pref
        ;START151016
        generate_mask,fi,st,dsp,wd,stc,help,glm,pref

        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.brain_mask: begin
        brain_mask,fi,wd,dsp,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.logical_and: begin
        logical_and,fi,wd,dsp,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.fidl_and: begin
        fidl_and,fi,wd,dsp,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.logical_xor: begin
        logical_xor,fi,wd,dsp,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.logical_not: begin
        logical_not,fi,wd,dsp,help,stc,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.logical_or: begin
        logical_or,fi,wd,dsp,help,stc,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.apply_mask: begin
        apply_mask,fi,wd,dsp,help,stc,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end

    ;START130312
    wd.fidl_mask: begin
        fidl_mask,fi,wd,dsp,help,stc,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end


    wd.fidl_logic: begin
        fidl_logic,fi,dsp,wd,stc,help,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end

    ;START111121
    wd.fidl_split: begin
        fidl_split,fi,dsp,wd,stc,help,pref
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    
    wd1.define_regions: begin
        define_regions,fi,wd,dsp,help,stc,pref,dsp_image
        ;For some reason this call must be in define_regions for the image to be updated.
        ;dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    
    wd1.count_mask: begin
        count_mask,fi,wd,dsp,help,stc
    end
    wd1.fidl_reg_ass: begin
        fidl_reg_ass,fi,wd,dsp,help
    end
    wd1.convert_af3d_to_fidl: begin
        convert_af3d_to_fidl,fi,wd,dsp,help,stc
    end
    wd1.fidl_labelfile: begin
        fidl_labelfile,fi,wd,dsp,help,stc
    end
    wd1.compute_region_stats: begin
        compute_region_stats,fi,wd,dsp,help,stc,pref
    end
    wd1.compute_region_clusters: begin
        compute_region_clusters,fi,wd,dsp,help,stc
    end
    wd1.delete_regions: begin
        delete_regions,fi,wd,dsp,help,pref,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd1.fidl_flipreg: begin
        fidl_flipreg,fi,wd,dsp,help,stc
        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end

    ;START150326
    ;wd1.grow_regions: begin
    ;    grow_regions,fi,wd,dsp,help
    ;end

    wd1.compute_tc_reg_stats: begin
        compute_tc_reg_stats,fi,wd,dsp,help,stc,pref
    end
    wd1.fix_region_file: begin
        fix_region_file,fi,wd,dsp,help,stc,pref
    end

    ;START170719
    wd1.fidl_fix_region_file:begin
        fidl_fix_region_file,fi,help,pref
    end

    wd.smobox: begin
        smobox,fi,dsp,wd
    end
    wd.smogauss: begin
        smogauss,fi,st,dsp,wd,stc,help,glm,pref
    end
    wd.fidl_gauss: begin

        ;fidl_gauss,fi,wd,dsp,help,stc 
        ;START170206
        fidl_gauss,fi,wd,dsp,help,stc,pref

        dsp_image = update_image(fi,dsp,wd,stc,pref)
    end
    wd.smobutter: begin
        smobutter,fi,dsp,wd,help,stc,pref
    end

    ;********** Profile functions. ***************
    
    wd.hprof: begin
        pr.roiprof = !FALSE
        pr.oprof = !FALSE
        pr.horzprof = !TRUE
        pr.prof_on = !PROF_HV
        widget_control,wd.profsave,/SENSITIVE
        pr.mode = !DRAW_OFF
        widget_control,wd.draw[fi.cw],EVENT_PRO='get_profile'
    end
    
    wd.vprof: begin
        pr.roiprof = !FALSE
        pr.oprof = !FALSE
        pr.horzprof = !FALSE
        pr.prof_on = !PROF_HV
        widget_control,wd.profsave,/SENSITIVE
        pr.mode = !DRAW_OFF
        widget_control,wd.draw[fi.cw],EVENT_PRO='get_profile'
    end
    
    wd.zprof: begin
        hdr = *fi.hdr_ptr[fi.n]
        hd = *fi.hdr_ptr[hdr.mother]
        pr.last_type = pr.time_type
        if(ptr_valid(hdr.ifh.region_names)) then $
            pr.regional = !TRUE $  ; Use regional values if current image is a region file.
        else $
            pr.regional = !FALSE
        if(ptr_valid(hd.ifh.behavior_names)) then $
            pr.scatter_plot = !TRUE $  ; Scatter plot from correlation analysis.
        else $
            pr.scatter_plot = !FALSE
        pr.roiprof = !FALSE
        pr.prof_on = !PROF_NO ;ADDED 4/30/01
        widget_control,wd.draw[fi.cw],EVENT_PRO='get_time_prof'
    end
    
    wd.roiprof: begin
        pr.prof_on = !PROF_REG
        pr.roiprof = !TRUE
        pr.last_type = pr.time_type
        widget_control,wd.draw[fi.cw],EVENT_PRO='get_time_prof'
        if(pr.mode eq !DRAW_DONE) then begin
            widget_control,wd.draw[fi.cw],GET_UVALUE=prof
        endif
    end
    
    wd.oprof: begin
    ;   Determine profile to plot.
        case pr.prof_on of
        !PROF_NO: return
        !PROF_HV: begin
    ;;;        prof = get_profile(row,fi,pr,dsp,wd,st,stc,help)
    ;;;        name = strcompress(string(fi.tail,"_r",row),/REMOVE_ALL)
    ;;;            overplot,prof,row,name,profdat,pr,dsp,fi
            pr.oprof = !TRUE
            pr.mode = !DRAW_OFF
            widget_control,wd.draw[fi.cw],EVENT_PRO='get_profile'
        end
        !PROF_T: begin
                widget_control,wd.draw[fi.cw],EVENT_PRO='get_time_prof'
                if(pr.mode eq !DRAW_DONE) then begin
                    widget_control,wd.draw[fi.cw],GET_UVALUE=prof
                endif
        end
        endcase
    end
    
    wd.profsave: begin
        if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='Save profiles to disk in an ASCII file.' 
        get_dialog_pickfile,'*.txt',path,'Please select file.',plotfile,rtn_nfiles,path,FILE="profile.txt"
        if plotfile eq 'GOBACK' or plotfile eq 'EXIT' then return
        pr.path = path
        prd = *pr.profcmds(pr.cur_window)
        openw,luprof,plotfile(0),/GET_LUN
        x = fltarr(prd.nprof+1)
        for i=0,n_elements(prd.profdat(*,0))-1 do begin
            for j=0,prd.nprof do x(j) = prd.profdat(i,j)
            printf,luprof,x,FORMAT='(10g)'
        endfor
        close,luprof
        free_lun,luprof
    end
    wd.profwidth: begin
        if(help.enable eq !TRUE) then begin
            value='Profile will be computed by integrating over a line perpendicular to the profile for the width specified here.'
            widget_control,help.id,SET_VALUE=value
        endif
        labels = strarr(13)
        labels(0) = string('Width: 1')
        labels(1) = string('Width: 3')
        labels(2) = string('Width: 5')
        labels(3) = string('Width: 7')
        labels(4) = string('Width: 9')
        labels(5) = string('Width: 11')
        labels(6) = string('Width: 13')
        labels(7) = string('Width: 15')
        labels(8) = string('Width: 21')
        labels(9) = string('Width: 31')
        labels(10) = string('Width: 41')
        labels(11) = string('Width: 51')
        labels(12) = string('Width: 61')
        index = get_button(labels)
        if(index lt 8) then $
            pr.width = 2*index + 1 $
        else $
        pr.width = 10*(index-6) + 1
    end
    wd.profhght: begin
        if(help.enable eq !TRUE) then begin
            value='Profile will be computed by integrating over a line perpendicular to the profile for the height specified here.'
            widget_control,help.id,SET_VALUE=value
        endif
        labels = strarr(13)
        labels(0) = string('Height: 1')
        labels(1) = string('Height: 3')
        labels(2) = string('Height: 5')
        labels(3) = string('Height: 7')
        labels(4) = string('Height: 9')
        labels(5) = string('Height: 11')
        labels(6) = string('Height: 13')
        labels(7) = string('Height: 15')
        labels(8) = string('Width: 21')
        labels(9) = string('Width: 31')
        labels(10) = string('Width: 41')
        labels(11) = string('Width: 51')
        labels(12) = string('Width: 61')
        index = get_button(labels)
        if(index lt 8) then $
            pr.height = 2*index + 1 $
        else $
        pr.height = 10*(index-6) + 1
    end
    wd.t_box_size: begin
        if(help.enable eq !TRUE) then begin
            value='Time profile will be computed by either by integrating over the specified box at each frame or by summing pixels above a specified threshold.'
            widget_control,help.id,SET_VALUE=value
        endif
        labels = strarr(14)
        labels(0) = string('Height: 1x1')
        labels(1) = string('Height: 3x3')
        labels(2) = string('Height: 5x5')
        labels(3) = string('Height: 7x7')
        labels(4) = string('Height: 9x9')
        labels(5) = string('Height: 11x11')
        labels(6) = string('Height: 13x13')
        labels(7) = string('Height: 15x15')
        labels(8) = string('Width: 21x21')
        labels(9) = string('Width: 31x31')
        labels(10) = string('Width: 41x41')
        labels(11) = string('Width: 51x51')
        labels(12) = string('Width: 61x61')
        labels(13) = string('Other')
        index = get_button(labels)
        if(index ne 13) then begin
            if(index lt 8) then begin
                pr.height = 2*index + 1
                pr.width = 2*index + 1
            endif else begin
                pr.height = 10*(index-6) + 1
                pr.width = 10*(index-6) + 1
            endelse
        endif else begin
            str = get_str(1,'XY dimensions','1')
            pr.height = fix(str[0])
            pr.width = fix(str[0])
        endelse
    end

    ;wd.prof_fft: begin
    ;    prof_fft,pr,dsp,fi,help
    ;end
    
    wd.print_prof: begin
        if(help.enable eq !TRUE) then begin
            value='Print most recently plotted profile to the default line printer.'
            widget_control,help.id,SET_VALUE=value
        endif
        print_prof,pr,fi,dsp,pref
        widget_control,wd.error,SET_VALUE=string('Plot printed to ',pref.printer_name)
    end
    
    wd.select_prof: begin
        if(help.enable eq !TRUE) then begin
            value='Select a profile.  This can be used to plot additonal profiles.'
            widget_control,help.id,SET_VALUE=value
        endif
        n = 0
        for i=0,pr.nwindows-1 do begin
            if(pr.id(i) gt 0) then n = n + 1
        end
        labels = strarr(n)
        j = 0
        for i=0,pr.nwindows-1 do begin
            if(pr.id(i) ge 0) then begin
            labels(j) = strcompress(string(i),/REMOVE_ALL)
            j = j + 1
        endif
        end
        index = get_button(labels)
        pr.cur_window = index
    end

    wd.kill_all_prof: begin
        n = 0
        for i=0,pr.nwindows-1 do begin
            if(ptr_valid(pr.profcmds(i))) then $
                ptr_free,pr.profcmds(i)
            if(pr.base(i) gt 0) then begin
                widget_control,pr.base(i),/DESTROY
                pr.id(i) = -1
                pr.draw(i) = -1
                pr.base(i) = -1
            endif
        end
        pr.nwindows = 0
    end
    
    wd.profsym: begin
        if(help.enable eq !TRUE) then begin
            value='Profiles plot using symbols only.'
            widget_control,help.id,SET_VALUE=value
        endif
        pr.symtype = !SYMBOL
        widget_control,wd.profsym,SENSITIVE=0
        widget_control,wd.profline,/SENSITIVE
        widget_control,wd.profsymline,/SENSITIVE
    end
    
    wd.profline: begin
        if(help.enable eq !TRUE) then begin
            value='Profiles plot using lines only.'
            widget_control,help.id,SET_VALUE=value
        endif
        pr.symtype = !LINE
        widget_control,wd.profline,SENSITIVE=0
        widget_control,wd.profsym,/SENSITIVE
        widget_control,wd.profsymline,/SENSITIVE
    end

    wd.profsymline: begin
        if(help.enable eq !TRUE) then begin
            value='Profiles plot using symbols and lines.'
            widget_control,help.id,SET_VALUE=value
        endif
        pr.symtype = !SYMLINE
        widget_control,wd.profline,/SENSITIVE
        widget_control,wd.profsym,/SENSITIVE
        widget_control,wd.profsymline,SENSITIVE=0
    end
    
    wd.t_prof_plain: begin
        if(help.enable eq !TRUE) then begin
            value='Integrate over specified box at each frame.'
            widget_control,help.id,SET_VALUE=value
        endif
        pr.time_type = !PLAIN_TPROF
        widget_control,wd.t_prof_max_pix,/SENSITIVE
        widget_control,wd.t_prof_plain,SENSITIVE=0
        widget_control,wd.t_gang,/SENSITIVE
        widget_control,wd.phisto,/SENSITIVE
        widget_control,wd.t_prof_ext_thresh,/SENSITIVE
        widget_control,wd.scatter_plot,/SENSITIVE
    end

    wd.t_prof_ext_thresh: begin
        if(help.enable eq !TRUE) then begin
            value='Apply mask generated under Process|Generate_mask to the image before summing over a region.'
            widget_control,help.id,SET_VALUE=value
        endif
        labels = strarr(fi.nfiles)
        labels(0:fi.nfiles-1) = fi.tails(0:fi.nfiles-1)
        idx = get_button(labels,TITLE='Mask.')
        hdr = *fi.hdr_ptr(idx)
        profmask = fltarr(hdr.xdim,hdr.ydim,hdr.zdim)
        for z=0,hdr.zdim-1 do $
            profmask(*,*,z) = get_image(z,fi,stc,FILNUM=idx+1)
        pr.time_type = !EXT_THRESH_TPROF
        widget_control,wd.t_prof_max_pix,/SENSITIVE
        widget_control,wd.t_prof_ext_thresh,SENSITIVE=0
        widget_control,wd.t_gang,/SENSITIVE
        widget_control,wd.phisto,/SENSITIVE
        widget_control,wd.t_prof_plain,/SENSITIVE
        widget_control,wd.scatter_plot,/SENSITIVE
    end
    
    wd.t_gang: begin
        pr.time_type = !GANG_TPROF
        widget_control,wd.t_prof_max_pix,/SENSITIVE
        widget_control,wd.t_prof_plain,/SENSITIVE
        widget_control,wd.gang_dim,/SENSITIVE
        widget_control,wd.gang_subdim,/SENSITIVE
        widget_control,wd.phisto,/SENSITIVE
        widget_control,wd.t_gang,SENSITIVE=0
        widget_control,wd.t_prof_ext_thresh,/SENSITIVE
        widget_control,wd.scatter_plot,/SENSITIVE
    end

    wd.phisto: begin
        pr.time_type = !HISTO_TPROF
        widget_control,wd.t_prof_max_pix,/SENSITIVE
        widget_control,wd.t_prof_plain,/SENSITIVE
        widget_control,wd.gang_dim,/SENSITIVE
        widget_control,wd.gang_subdim,/SENSITIVE
        widget_control,wd.t_gang,/SENSITIVE
        widget_control,wd.phisto,SENSITIVE=0
        widget_control,wd.t_prof_ext_thresh,/SENSITIVE
        widget_control,wd.scatter_plot,/SENSITIVE
    end
    
    wd.gang_dim: begin
        labels = strarr(2)
        labels(0) = string('3x3')
        labels(1) = string('5x5')
        index = get_button(labels,TITLE='Enter profile array size')
        pr.gang_dim = (index+1)*2 + 1
    end
    
    wd.gang_subdim: begin
        labels = strarr(5)
        labels(0) = string('1')
        labels(1) = string('2')
        labels(2) = string('3')
        labels(3) = string('4')
        labels(4) = string('5')
        index = get_button(labels,TITLE='Number of voxels to average')
        pr.gang_subdim = index + 1
    end
    
    wd.t_prof_max_pix: begin
        if(help.enable eq !TRUE) then begin
            value='Time profiles are computed by summing over voxels inside the box with values above the threshold specified by the "Set Threshold" button.'+'  The threshold is computed as the specified fractional threshold times the maximum voxel within'
            widget_control,help.id,SET_VALUE=value
        endif
        pr.time_type = !THRESH_TPROF
        widget_control,wd.t_prof_max_pix,SENSITIVE=0
        widget_control,wd.t_prof_plain,/SENSITIVE
        widget_control,wd.t_gang,/SENSITIVE
        widget_control,wd.scatter_plot,/SENSITIVE
    end

    wd.scatter_plot: begin
        pr.time_type = !SCATTER_PLOT
        widget_control,wd.scatter_plot,SENSITIVE=0
        widget_control,wd.t_prof_max_pix,/SENSITIVE
        widget_control,wd.t_prof_plain,/SENSITIVE
        widget_control,wd.t_gang,/SENSITIVE
    end
    
    wd.thresh_pct: begin
        if(help.enable eq !TRUE) then begin
            value='Set the threshold used by the time profile command in the "Average over Thresholded Region" mode.  The threshold is expressed as a percent of the maximum value in the region.'+'  Set threshold to a negative value to threshold negative vo'
            widget_control,help.id,SET_VALUE=value
        endif
        pr.time_thresh_type = !PERCENT
        if(abs(pr.time_thresh) gt 1.) then $
            pr.time_thresh = 0.5
        value = string(pr.time_thresh*100.)
        label = 'Threshold (+/-) as percent of maximum/minimum.'
        thresh = get_str(1,label,value)
        pr.time_thresh = float(thresh(0))/100.
    end
    
    wd.thresh_sig: begin
        if(help.enable eq !TRUE) then begin
            value='Set the threshold used by the time profile command in the "Average over Thresholded Region" mode.' + '  The threshold should be the minimum log-significance-probability.' + ' The absolute value of the significance probability will be tes'
            widget_control,help.id,SET_VALUE=value
        endif
        pr.time_thresh_type = !SIGNIFICANCE
        if(abs(pr.time_thresh) lt 1.) then $
        pr.time_thresh = 3
        value = string(pr.time_thresh)
        label = 'Threshold as log-probability/t/z. Negative for deactivations'
        thresh = get_str(1,label,value)
        pr.time_thresh = float(thresh(0))
    end
    
    wd.subtract_trend: begin
        if(help.enable eq !TRUE) then begin
            value='Subtract linear trend from time profile before plotting.'
            widget_control,help.id,SET_VALUE=value
        endif
        hdr = *fi.hdr_ptr(fi.n)
        if(ptr_valid(trend.slope_ptr(hdr.mother))) then $
            get_trend,hdr.mother,fi,pr,dsp,wd,stc,help,pref
        pr.subtract_trend = !TRUE
        widget_control,wd.no_trend,/SENSITIVE
        widget_control,wd.subtract_trend,SENSITIVE=0
    end
    
    wd.no_trend: begin
        if(help.enable eq !TRUE) then begin
            value='Dont subtract linear trend from time profile before plotting.'
            widget_control,help.id,SET_VALUE=value
        endif
        pr.subtract_trend = !FALSE
        widget_control,wd.no_trend,SENSITIVE=0
        widget_control,wd.subtract_trend,/SENSITIVE
    end
    wd1.timeover_no: begin
        pr.time_overplot = !PLOT_NOTHING
        widget_control,wd1.timeover_no,SENSITIVE=0
        widget_control,wd1.timeover_para,/SENSITIVE
        widget_control,wd1.timeover_glm,/SENSITIVE
        widget_control,wd1.timeover_resid,/SENSITIVE
        widget_control,wd1.timeover_hrf,/SENSITIVE
        widget_control,wd1.timeover_pca,/SENSITIVE
        widget_control,wd1.timeover_stderr,/SENSITIVE
    end
    
    wd1.timeover_para: begin
        pr.time_overplot = !PLOT_PARADIGM
        widget_control,wd1.timeover_no,/SENSITIVE
        widget_control,wd1.timeover_para,SENSITIVE=0
        widget_control,wd1.timeover_glm,/SENSITIVE
        widget_control,wd1.timeover_resid,/SENSITIVE
        widget_control,wd1.timeover_hrf,/SENSITIVE
        widget_control,wd1.timeover_pca,/SENSITIVE
        widget_control,wd1.timeover_stderr,/SENSITIVE
    end
    
    wd1.timeover_glm: begin
        pr.time_overplot = !PLOT_GLM
        widget_control,wd1.timeover_no,/SENSITIVE
        widget_control,wd1.timeover_para,/SENSITIVE
        widget_control,wd1.timeover_glm,SENSITIVE=0
        widget_control,wd1.timeover_resid,/SENSITIVE
        widget_control,wd1.timeover_hrf,/SENSITIVE
        widget_control,wd1.timeover_pca,/SENSITIVE
        widget_control,wd1.timeover_stderr,/SENSITIVE
    end
    
    wd1.timeover_resid: begin
        pr.time_overplot = !PLOT_RESIDUALS
        widget_control,wd1.timeover_no,/SENSITIVE
        widget_control,wd1.timeover_para,/SENSITIVE
        widget_control,wd1.timeover_glm,/SENSITIVE
        widget_control,wd1.timeover_resid,SENSITIVE=0
        widget_control,wd1.timeover_hrf,/SENSITIVE
        widget_control,wd1.timeover_pca,/SENSITIVE
        widget_control,wd1.timeover_stderr,/SENSITIVE
    end
    
    wd1.timeover_hrf: begin
        pr.time_overplot = !PLOT_HRF
        widget_control,wd1.timeover_no,/SENSITIVE
        widget_control,wd1.timeover_para,/SENSITIVE
        widget_control,wd1.timeover_glm,/SENSITIVE
        widget_control,wd1.timeover_resid,/SENSITIVE
        widget_control,wd1.timeover_hrf,SENSITIVE=0
        widget_control,wd1.timeover_pca,/SENSITIVE
        widget_control,wd1.timeover_stderr,/SENSITIVE
    end
    
    wd1.timeover_pca: begin
        pr.time_overplot = !PLOT_PCA
        widget_control,wd1.timeover_no,/SENSITIVE
        widget_control,wd1.timeover_para,/SENSITIVE
        widget_control,wd1.timeover_glm,/SENSITIVE
        widget_control,wd1.timeover_resid,/SENSITIVE
        widget_control,wd1.timeover_hrf,/SENSITIVE
        widget_control,wd1.timeover_pca,SENSITIVE=0
        widget_control,wd1.timeover_pca,/SENSITIVE
        widget_control,wd1.timeover_stderr,/SENSITIVE
    end
    
    wd1.timeover_stderr: begin
        pr.time_overplot = !PLOT_STDERR
        widget_control,wd1.timeover_no,/SENSITIVE
        widget_control,wd1.timeover_para,/SENSITIVE
        widget_control,wd1.timeover_glm,/SENSITIVE
        widget_control,wd1.timeover_resid,/SENSITIVE
        widget_control,wd1.timeover_hrf,/SENSITIVE
        widget_control,wd1.timeover_pca,/SENSITIVE
        widget_control,wd1.timeover_stderr,SENSITIVE=0
    end
    
    wd.true_scaling: begin
        pr.true_scl = !TRUE
        widget_control,wd.true_scaling,SENSITIVE=0
        widget_control,wd.prop_scaling,/SENSITIVE
    end
    
    wd.prop_scaling: begin
        pr.true_scl = !FALSE
        widget_control,wd.prop_scaling,SENSITIVE=0
        widget_control,wd.true_scaling,/SENSITIVE
    end
    
    wd1.prof_proc_nothing: begin
        pr.prof_proc = !PROF_PROC_NOTHING
        widget_control,wd1.prof_proc_nothing,SENSITIVE=0
        widget_control,wd1.prof_proc_pct,/SENSITIVE
    end

    wd1.prof_proc_pct: begin
        pr.prof_proc = !PROF_PROC_PCT
        widget_control,wd1.prof_proc_nothing,/SENSITIVE
        widget_control,wd1.prof_proc_pct,SENSITIVE=0
    end
    
    wd1.frames_to_skip: begin
        str = get_str(1,'Number of frames to skip',pr.frames_to_skip)
        pr.frames_to_skip = fix(str[0])
    end
    
    wd.prof_wht: pr.color = dsp[fi.cw].white
    wd.prof_ylw: pr.color = dsp[fi.cw].yellow
    wd.prof_red: pr.color = dsp[fi.cw].red
    wd.prof_grn: pr.color = dsp[fi.cw].green
    wd.prof_blu: pr.color = dsp[fi.cw].blue
    
    wd1.prgauss_smooth: begin
        if(help.enable eq !TRUE) then begin
            value='Smooth profile with Gaussian filter.'
            widget_control,help.id,SET_VALUE=value
        endif
        value = string(1.41)
        label = 'FWHM of Gaussian kernel'
        str = float(get_str(1,label,value))
        pr.krnl_width = float(str(0))
        pr.krnl_type = !GAUSSIAN
        widget_control,wd.prbox_smooth0,/SENSITIVE
        widget_control,wd.prbox_smooth3,/SENSITIVE
        widget_control,wd.prbox_smooth5,/SENSITIVE
    end
    
    wd.prbox_smooth0: begin
        if(help.enable eq !TRUE) then begin
            value='Dont smooth time profiles.'
            widget_control,help.id,SET_VALUE=value
        endif
        pr.krnl_width = 0
        pr.krnl_type = !BOXCAR
        widget_control,wd1.prgauss_smooth,/SENSITIVE
        widget_control,wd.prbox_smooth0,SENSITIVE=0
        widget_control,wd.prbox_smooth3,/SENSITIVE
        widget_control,wd.prbox_smooth5,/SENSITIVE
    end
    
    wd.prbox_smooth3: begin
        if(help.enable eq !TRUE) then begin
            value='Smooth time profiles with a 3 point boxcar filter.'
            widget_control,help.id,SET_VALUE=value
        endif
        pr.krnl_width = 3
        pr.krnl_type = !BOXCAR
        widget_control,wd1.prgauss_smooth,/SENSITIVE
        widget_control,wd.prbox_smooth0,/SENSITIVE
        widget_control,wd.prbox_smooth3,SENSITIVE=0
        widget_control,wd.prbox_smooth5,/SENSITIVE
    end
    
    wd.prbox_smooth5: begin
        if(help.enable eq !TRUE) then begin
            value='Smooth time profiles with a 5 point boxcar filter.'
            widget_control,help.id,SET_VALUE=value
        endif
        pr.krnl_width = 5
        pr.krnl_type = !BOXCAR
        widget_control,wd1.prgauss_smooth,/SENSITIVE
        widget_control,wd.prbox_smooth0,/SENSITIVE
        widget_control,wd.prbox_smooth3,/SENSITIVE
        widget_control,wd.prbox_smooth5,SENSITIVE=0
    end
    
    ;wd1.build_macro: begin
    ;    init_macro,wd,wd1
    ;    widget_control,wd1.finish_macro,/SENSITIVE
    ;    widget_control,wd1.macro_comment,/SENSITIVE
    ;    widget_control,wd1.build_macro,SENSITIVE=0
    ;    widget_control,wd1.compile_macro,/SENSITIVE
    ;end
    ;wd1.macro_comment: begin
    ;    macro_comment,wd,wd1
    ;end
    ;wd1.finish_macro: begin
    ;    finish_macro,wd,wd1
    ;    widget_control,wd1.finish_macro,SENSITIVE=0
    ;    widget_control,wd1.build_macro,/SENSITIVE
    ;end
    ;wd1.compile_macro: begin
    ;    slash = rstrpos(batch_pro,"/");
    ;    len = strlen(batch_pro)
    ;    pro_name = strmid(batch_pro,slash+1,len-slash-1)
    ;    compile_macro,pro_name,/FIDL
    ;    widget_control,wd1.compile_macro,SENSITIVE=0
    ;end
    ;wd.user1: begin
    ;    user1,fi,pr,dsp,wd,stc,help,pref
    ;end
    ;wd.user2: begin
    ;    user2,fi,pr,dsp,wd,stc,help,pref
    ;end
    ;wd.user3: begin
    ;    user3,fi,pr,dsp,wd,stc,help,pref
    ;end
    ;wd.user4: begin
    ;    user4,fi,pr,dsp,wd,stc,help,pref
    ;end
    ;wd.user5: begin
    ;    user5,fi,pr,dsp,wd,stc,help,pref
    ;end
    ;wd.user6: begin
    ;    user6,fi,pr,dsp,wd,stc,help,pref
    ;end
    ;wd.user7: begin
    ;    user7,fi,pr,dsp,wd,stc,help,pref
    ;end
    ;wd.user8: begin
    ;    user8,fi,pr,dsp,wd,stc,help,pref
    ;end
    ;wd.user9: begin
    ;    user9,fi,pr,dsp,wd,stc,help,pref
    ;end
    ;wd.user10: begin
    ;    user10,fi,pr,dsp,wd,stc,help,pref
    ;end
    else: begin
        print,'*** Unknown event encountered in stats_exec_event. ***
    end
endcase

if(lcbatch eq !TRUE and batch_base gt 0 and ev.id ne wd1.build_macro) then begin
    if(num_macro_dsc gt 0) then begin
        cmd = macro_descriptor(0:num_macro_dsc-1)
        widget_control,batch_id,SET_VALUE=cmd
    endif
endif

check_math_errors,report_math_errors
return
end


;*************
pro stats_exec
;*************
common stats_comm
common linear_fit
common fidl_batch
CD,GETENV('PWD') ;gets around a new feature in idl 6.0 and later, see  http://www.rsinc.com/services/techtip.asp?ttid=3844


;defsysv,'!MAC_OSX',getenv('MAC_OSX')
;if !MAC_OSX eq 'TRUE' then begin
;    print,'Using TRUE_COLOR=24'
;    set_plot,'X'
;    device,TRUE_COLOR=24
;    device,DECOMPOSED=0
;endif else begin
;    if !VERSION.OS_FAMILY ne 'Windows' then begin
;        device,PSEUDO_COLOR=8  ; Use pseudo-color on ultra creators.
;        set_plot,'X'
;    endif
;endelse
;START110412
defsysv,'!FIDLCOLORDEPTH',getenv('FIDLCOLORDEPTH')
if !FIDLCOLORDEPTH eq 24 then begin
    print,'Using TRUE_COLOR=24'
    set_plot,'X'
    device,TRUE_COLOR=24
    device,DECOMPOSED=0
endif else if !FIDLCOLORDEPTH eq 16 then begin
    print,'Using TRUE_COLOR=16'
    set_plot,'X'
    device,TRUE_COLOR=16
    device,DECOMPOSED=0
endif else begin
    if !VERSION.OS_FAMILY ne 'Windows' then begin
        device,PSEUDO_COLOR=8  ; Use pseudo-color on ultra creators.
        set_plot,'X'
    endif
endelse


@stats.h
@stats_init.h
fi.image_filter = '*.4dfp.img'
fi.glm_filter = '*.glm'
read_pref,pref
xm1 = [0,0,0,0]
first_pass = !TRUE
n_clicks = 0

;START150121
;print,'FIDL '+!FIDL_REV
;START150413
print,'FIDL '+trim(!FIDL_REV)

; Next two lines force !D.TABLE_SIZE to be correct.
tvlct,red,green,blue,/GET
tvlct,red,green,blue

;ncolors = !MAX_NUM_COLORS < !D.TABLE_SIZE
ncolors = !D.TABLE_SIZE
;ncolors = 136 < !D.TABLE_SIZE
min_primary = 20
max_secondary = 100
if ncolors lt 55 then begin
    stat=dialog_message('Only '+strtrim(!D.TABLE_SIZE,2)+' colors are available from the X11 server.' $
        +' Fidl needs at least 55.'+string(10B)+string(10B)+'Open a second fidl, ' $
        +'then close this one.'+string(10B)+'The new fidl should have enough colors.',/ERROR)
endif else if ncolors lt !NUM_LINECOLORS+min_primary+max_secondary then begin
    len_colortab1 = min_primary
    len_colortab2 = ncolors - !NUM_LINECOLORS - len_colortab1
endif else begin
    len_colortab2 = max_secondary
    len_colortab1 = ncolors - !NUM_LINECOLORS - len_colortab2
endelse
defsysv,'!LEN_COLORTAB1',len_colortab1
defsysv,'!LEN_COLORTAB2',len_colortab2
defsysv,'!LEN_COLORTAB',(!LEN_COLORTAB1+!LEN_COLORTAB2)
dsp[fi.cw].num_colors = !LEN_COLORTAB1 + !LEN_COLORTAB2 + !NUM_LINECOLORS
defsysv,'!NUM_COLORS',dsp[fi.cw].num_colors
print,'!D.TABLE_SIZE=',!D.TABLE_SIZE,' !NUM_COLORS=',!NUM_COLORS
print,'Number of colors in the primary color table:   '+ strtrim(!LEN_COLORTAB1,2)
print,'Number of colors in the secondary color table: '+ strtrim(!LEN_COLORTAB2,2)
print,'Number of line colors:                         '+ strtrim(!NUM_LINECOLORS,2)
print,'!LEN_COLORTAB = '+ strtrim(!LEN_COLORTAB,2)

;dsp[fi.cw].scale = !VOLUME
dsp[fi.cw].scale = !SLICE
dsp[fi.cw].conadj = 0

stats_init,dsp,fi,pr,fl,st,wd,pref
fi.n = 0
fi.color_scale1[*] = !GRAY_SCALE
fi.color_min1[*] = 0
fi.color_max1[*] = 100
fi.color_gamma1[*] = -2.
fi.color_scale2[*] = !NO_SCALE
fi.color_min2[*] = 0
fi.color_max2[*] = 100
fi.color_gamma2[*] = 1.
load_colortable,fi,dsp
lcbatch = !FALSE
rtn=checkos()
defsysv,'!SunOS_Linux',rtn.SunOS_Linux
defsysv,'!bit32_64',rtn.bit32_64
print,'!SunOS_Linux=',!SunOS_Linux
print,'!bit32_64=',!bit32_64
if !SunOS_Linux eq 0 then begin
    defsysv,'!SHARE_LIB',getenv('FIDL_SHAREABLE_OBJ_LIB')
    defsysv,'!BINEXECUTE',getenv('FIDL_BIN_PATH')
endif else if !SunOS_Linux eq 1 then begin
    if !bit32_64 eq 32 then begin
        defsysv,'!SHARE_LIB',getenv('FIDL_SHAREABLE_OBJ_LIB_LINUX')
        defsysv,'!BINEXECUTE',getenv('FIDL_BIN_PATH_LINUX')
    endif else begin

        ;defsysv,'!SHARE_LIB',getenv('FIDL_SHAREABLE_OBJ_LIB_LINUX64')
        ;START150423
        ;spawn,'ldd --version',result
        ;print,result[0]
        ;result=strsplit(result[0],'.',/EXTRACT)
        ;if float(result[n_elements(result)-1]) lt 7 then $ ;/lib64/libc.so.6: version `GLIBC_2.7'
        ;    defsysv,'!SHARE_LIB',getenv('FIDL_SHAREABLE_OBJ_LIB_LINUX64') $
        ;else $
        ;    defsysv,'!SHARE_LIB',getenv('FIDL_SHAREABLE_OBJ_LIB_LINUX64_NEW')
        ;START150424
        defsysv,'!SHARE_LIB',getenv('FIDL_SHAREABLE_OBJ_LIB_LINUX64_NEW')

        defsysv,'!SHARE_LIBCIFTI',getenv('FIDL_SO64CIFTI')
        print,'!SHARE_LIBCIFITI=',!SHARE_LIBCIFTI
        defsysv,'!BINEXECUTE',getenv('FIDL_BIN_PATH_LINUX64')
    endelse
endif
print,'!SHARE_LIB=',!SHARE_LIB
print,'!BINEXECUTE=',!BINEXECUTE







;defsysv,'!FIDL_ARCANE',!TRUE

wd.leader        = widget_base(title=string(!FIDL_REV,FORMAT='("fidl version",f6.2)'),/COLUMN)
defsysv,'!FMRI_LEADER',wd.leader
menrow      = widget_base(wd.leader,/ROW)
butrow      = widget_base(wd.leader,/ROW)
;START0
butrow1 = widget_base(wd.leader,/ROW)
;;;roirow      = widget_base(wd.leader,/ROW)
filerow     = widget_base(wd.leader,/COLUMN)

wd1.mload       = widget_button(menrow,value='Files',MENU=3)
wd1.mproc       = widget_button(menrow,value='Process',MENU=3)
wd1.mlinmod     = widget_button(menrow,value='GLM',MENU=3)
wd1.mdisp       = widget_button(menrow,value='Display',MENU=3)
wd1.mprof       = widget_button(menrow,value='Profiles',MENU=3)
;;;wd1.mroi            = widget_button(menrow,value='ROI',MENU=3)
wd1.mhelp       = widget_button(menrow,value='Help',MENU=3)
wd.help_info= widget_button(wd1.mhelp,value='Display help window')
;wd.help     = widget_button(wd1.mhelp,value='Support')


mldmri=widget_button(wd1.mload,value='*img, *nii, *nii.gz, *.conc',MENU=3)
wd.ldmri_4dfp = widget_button(mldmri,value='Load one or more')
wd.wrt4dfp  = widget_button(mldmri,value='Write 4dfp image(s)')
wd.stitch    = widget_button(mldmri,value='Concatenate images',/SEPARATOR)
wd1.save_conc = widget_button(mldmri,value='Save concatenated set')
wd1.load_atlas = widget_button(mldmri,value='Load atlas',/SEPARATOR)
wd1.load_mask = widget_button(mldmri,value='Load mask')
;wd.ldmri_analyze = widget_button(mldmri,value='Load Analyze x-y-t')

;START160112
wd.ld_list = widget_button(wd1.mload,value='Load list (or conc) - individual files are listed')

mgif = widget_button(wd1.mload,value='GIF',MENU=3)
wd.ld_gif = widget_button(mgif,value='Load GIF')
wd.wrtgif   = widget_button(mgif,value='Save')
mtiff = widget_button(wd1.mload,value='TIFF',MENU=3)
wd.ld_tif = widget_button(mtiff,value='Load')
wd.wrttif   = widget_button(mtiff,value='Save')

;mecat = widget_button(wd1.mload,value='ECAT',MENU=3)
;wd.ld_t88   = widget_button(mecat,value='Load Rev 6 (953b)')
;wd.ld_matrix7v = widget_button(mecat,value='Load Rev 7 (Exact)')

wd.ld_xwd = widget_button(wd1.mload,value='Load XWD')
wd.ldraw    = widget_button(wd1.mload,value='Load Unformatted')
wd1.load_image_filter    = widget_button(wd1.mload,value='File name filter')
;;;wd.ld_optical    = widget_button(wd1.mload,value='Load optical data')
wd.delete = widget_button(wd1.mload,value='Delete image',/SEPARATOR)
wd.delete_all = widget_button(wd1.mload,value='Delete all images')
wd.printany = widget_button(wd1.mload,value='Print any window',/SEPARATOR)
if !FIDL_ARCANE eq !TRUE then wd1.import_spm = widget_button(wd1.mload,value='Import SPM')
wd1.link_image = widget_button(wd1.mload,value='Link image')

;mpref    = widget_button(wd1.mload,value='Preferences',MENU=3)
;START120130
mpref = widget_button(wd1.mload,value='Preferences',MENU=3,/SEPARATOR)

wd1.preferences = widget_button(mpref,value='Modify')
wd1.save_prefs = widget_button(mpref,value='Save')
wd.exit     = widget_button(wd1.mload,value='Exit',/SEPARATOR)

wd.hprof    = widget_button(wd1.mprof,value='Horizontal Profile')
wd.vprof    = widget_button(wd1.mprof,value='Vertical Profile')
wd.zprof    = widget_button(wd1.mprof,value='Time Profile')
wd.roiprof  = widget_button(wd1.mprof,value='Profile region')
profsize    = widget_button(wd1.mprof,value='Size',MENU=3)
wd.profwidth= widget_button(profsize,value='Width')
wd.profhght = widget_button(profsize,value='Height')
wd.t_box_size = widget_button(profsize,value='Square')

mscaling = widget_button(wd1.mprof,value=string("Scaling"),MENU=3)
wd.true_scaling = widget_button(mscaling,value=string("Unscaled"))
wd.prop_scaling = widget_button(mscaling,value=string("Rescaled"))
wd.profsave = widget_button(wd1.mprof,value='Save Profiles')
;wd.gaussfit  = widget_button(wd1.mprof,value='Fit Gaussian')
wd.select_prof = widget_button(wd1.mprof,value='Select profile')
wd.kill_all_prof = widget_button(wd1.mprof,value='Kill all profiles')
wd.print_prof = widget_button(wd1.mprof,value='Print profile')
wsymbol     = widget_button(wd1.mprof,value='Format',MENU=3)
wd.profsym   = widget_button(wsymbol,value='Symbols Only')
wd.profline  = widget_button(wsymbol,value='Lines Only')
wd.profsymline = widget_button(wsymbol,value='Lines and Symbols')
wpr_tproftyp  = widget_button(wd1.mprof,value='Time profile mode',MENU=3)
wd.t_prof_plain  = widget_button(wpr_tproftyp,value='Mean over region')
widget_control,wd.t_prof_plain,SENSITIVE=0
wd.t_prof_max_pix= widget_button(wpr_tproftyp,value='Average over thresholded region')
threshold = widget_button(wpr_tproftyp,value='Set threshold',MENU=3)
wd.thresh_pct = widget_button(threshold,value='Percent of Maximum')
wd.thresh_sig = widget_button(threshold,value='Significance')
wd.t_prof_ext_thresh  = widget_button(wpr_tproftyp,value='Average over masked region')
wd.scatter_plot = widget_button(wpr_tproftyp,value='Scatter plot of correlation')
wd.phisto = widget_button(wpr_tproftyp,value='Histogram')
wd.t_gang  = widget_button(wpr_tproftyp,value='Ganged array')
wd.gang_dim  = widget_button(wpr_tproftyp,value='Ganged Dimension ')
wd.gang_subdim  = widget_button(wpr_tproftyp,value='Ganged Pixel Size')
wd.subtract_trend = widget_button(wpr_tproftyp,value='Subtract trend',/SEPARATOR)
wd.no_trend = widget_button(wpr_tproftyp,value=string("Don't subtract trend"))
widget_control,wd.no_trend,SENSITIVE=0
overplot = widget_button(wpr_tproftyp,value=string("Overplot"),MENU=3,/SEPARATOR)
wd1.timeover_no = widget_button(overplot,value=string("Nothing"))
wd1.timeover_para = widget_button(overplot,value=string("Stimulus Paradigm"))
wd1.timeover_glm = widget_button(overplot,value=string("Linear Model"))
wd1.timeover_resid = widget_button(overplot,value=string("Residuals"))
wd1.timeover_hrf = widget_button(overplot,value=string("Assumed hemodynamic response"))
wd1.timeover_stderr = widget_button(overplot,value=string("Time-course standard error"))
wd1.timeover_PCA = widget_button(overplot,value=string("Principal Components"))
widget_control,wd.prop_scaling,SENSITIVE=0
pr.true_scl = !FALSE
widget_control,wd1.timeover_para,SENSITIVE=0
widget_control,wd.gang_dim,SENSITIVE=0
widget_control,wd.gang_subdim,SENSITIVE=0
mprof_proc = widget_button(wpr_tproftyp,value=string("Profile region processing"),MENU=3)
wd1.prof_proc_nothing = widget_button(mprof_proc,value=string("None"))
wd1.prof_proc_pct = widget_button(mprof_proc,value=string("Percentile threshold"))
wd1.frames_to_skip = widget_button(wpr_tproftyp,value=string("Frames to skip"))
widget_control,wd1.prof_proc_nothing,SENSITIVE=0
wpr_color     = widget_button(wd1.mprof,value='Color',MENU=3)
wd.prof_wht   = widget_button(wpr_color,value='White')
wd.prof_ylw   = widget_button(wpr_color,value='Yellow')
wd.prof_red   = widget_button(wpr_color,value='Red')
wd.prof_grn   = widget_button(wpr_color,value='Green')
wd.prof_blu   = widget_button(wpr_color,value='Blue')
wprbox_smooth = widget_button(wd1.mprof,value='Smooth',MENU=3)
wd.prbox_smooth0 = widget_button(wprbox_smooth,value='None')
wd1.prgauss_smooth = widget_button(wprbox_smooth,value='Gaussian Smooth')
wd.prbox_smooth3 = widget_button(wprbox_smooth,value='Boxcar 3')
wd.prbox_smooth5 = widget_button(wprbox_smooth,value='Boxcar 5')
;if !FIDL_ARCANE eq !TRUE then wd.prof_fft   = widget_button(wd1.mprof,value='Profile FFT')

wd.plnstats = widget_button(wd1.mproc,value='Image stats')
wd.fidl_swap = widget_button(wd1.mproc,value='Swapbytes',/SEPARATOR)
wd.voxel_values = widget_button(wd1.mproc,value='Voxel values',/SEPARATOR)
wd.fidl_4dfp_to_text = widget_button(wd1.mproc,value='4dfp to text')



;wd.fidl_text_to_4dfp = widget_button(wd1.mproc,value='text to 4dfp')
;START140217
mtxt24dfp = widget_button(wd1.mproc,value='Text to 4dfp',MENU=3,/SEPARATOR)
wd.fidl_text_to_4dfp = widget_button(mtxt24dfp,value='Text to 4dfp')
wd.fidl_txt2conc_subcol = widget_button(mtxt24dfp,value='Text to conc, subtract columns')

wd.fidl_collatecol =widget_button(wd1.mproc,value='collate columns')

;;;wd.sinecorr = widget_button(wd1.mproc,value='Harmonics')
;wd.meanvar = widget_button(wd1.mproc,value='Mean and Variance')
;if(!FIDL_ARCANE eq !TRUE) then begin
;    wd.linfit = widget_button(wd1.mproc,value='Fit linear trend')
;    wd.kolmo_smrnov = widget_button(wd1.mproc,value='Compute K-S Statistic',/SEPARATOR)
;    wd.regress = widget_button(wd1.mproc,value='Regression analysis')
;    wd.kendalls_tau = widget_button(wd1.mproc,value='Compute Kendalls tau')
;    wd.diffimg  = widget_button(wd1.mproc,value='Difference Images')
;    wd.wilcoxon = widget_button(wd1.mproc,value='Compute Wilcoxon Statistic')
;endif else begin
;    wd.linfit = 0
;    wd.kolmo_smrnov = 0
;    wd.regress = 0
;    wd.kendalls_tau = 0
;    wd.diffimg = 0
;    wd.wilcoxon = 0
;endelse




mmultcomp = widget_button(wd1.mproc,value='Correct for multiple comparisons',MENU=3,/SEPARATOR)
mmultcomp0 = widget_button(mmultcomp,VALUE='Controls familywise error rate',MENU=3)

;wd.mult_comp_monte_carlo = widget_button(mmultcomp0,value='Z-map smoothed-Monte Carlo (IDL)' + string(10B) $
;    + '        Z-map must be in atlas space.'+string(10B)+'        See C version below for multiple files.')

wd.mult_comp_monte_carloC = widget_button(mmultcomp0,value='Z-map smoothed-Monte Carlo (C) - accepts multiple files',/SEPARATOR)
wd.fidl_fdr = widget_button(mmultcomp,VALUE='False discovery rate')
wd1.threshold_image= widget_button(wd1.mproc,value='Threshold image')


mthreshold_cluster = widget_button(wd1.mproc,value='Threshold and cluster image',MENU=3,/SEPARATOR)
;wd1.threshold_cluster= widget_button(mthreshold_cluster,value='IDL')
;wd1.fidl_threshold_cluster= widget_button(mthreshold_cluster,value='C')
;wd1.fidl_threshold_cluster2= widget_button(mthreshold_cluster,value='C - accepts multiple files')
;START140825
wd1.fidl_threshclus_idl=widget_button(mthreshold_cluster,value='IDL',/SEPARATOR)
wd1.fidl_threshclus=widget_button(mthreshold_cluster,value='C')




;;;wd.time_corr  = widget_button(wd1.mproc,value='Estimate time correlation')

;mcnlapps = widget_button(wd1.mproc,value='CNL applications',MENU=3,/SEPARATOR)
;wd.checkbold = widget_button(mcnlapps,value='Checkbold')
;wd.checkmprage = widget_button(mcnlapps,value='Checkmprage')
;wd.checkmprage111 = widget_button(mcnlapps,value='Checkmprage111')
;wd.anat222 = widget_button(mcnlapps,value='Anat222')
;wd.anat333 = widget_button(mcnlapps,value='Anat333')
;wd.check_glm = widget_button(mcnlapps,value='Check GLM')
;wd.glm_4dfp = widget_button(mcnlapps,value='GLM 4dfp')
;wd.fidl_avg = widget_button(mcnlapps,value='Mean and SEM 4dfps')

mlogical = widget_button(wd1.mproc,value='Logical Operations',MENU=3,/SEPARATOR)
wd.generate_mask  = widget_button(mlogical,value='Convert to mask')
wd1.brain_mask  = widget_button(mlogical,value='Compute brain mask')
wd.logical_not  = widget_button(mlogical,value='Logical inverse')
wd.logical_and  = widget_button(mlogical,value='Logical AND (IDL)',/SEPARATOR)
wd.fidl_and  = widget_button(mlogical,value='Logical AND (C)')
wd.logical_or  = widget_button(mlogical,value='Logical OR',/SEPARATOR)
wd.logical_xor  = widget_button(mlogical,value='Exclusive or')

;wd.apply_mask  = widget_button(mlogical,value='Apply mask')
;START130312
wd.apply_mask  = widget_button(mlogical,value='Apply mask',/SEPARATOR)
wd.fidl_mask  = widget_button(mlogical,value='Apply mask (C)')

wd.fidl_logic = widget_button(mlogical,value='3 different ANDs (C)',/SEPARATOR)
wd.fidl_split = widget_button(mlogical,value='Split (C)')
wd1.malgebra = widget_button(wd1.mproc,value='Algebraic Operations',MENU=3)
wd1.sum_images = widget_button(wd1.malgebra,value='Average or sum images')
wd1.subtract_images = widget_button(wd1.malgebra,value='Subtract images')
wd1.multiply_images = widget_button(wd1.malgebra,value='Multiply images')
wd1.divide_images = widget_button(wd1.malgebra,value='Divide images')
wd1.scale_images = widget_button(wd1.malgebra,value='Scale images')
wd1.add_scalar = widget_button(wd1.malgebra,value='Add constant to image')
wd1.clip_image = widget_button(wd1.malgebra,value='Clip image')
wd1.fidl_avg = widget_button(wd1.malgebra,value='Mean, SEM, Sum, MaxOverlap, Anat ave (C)',/SEPARATOR)
wd1.fidl_zeromean = widget_button(wd1.malgebra,value='Zero mean time series (C)')
wd1.fidl_minmax = widget_button(wd1.malgebra,value='Min and max (C)')
wd1.fidl_lminusrdivlplusr=widget_button(wd1.malgebra,value='L-R / L+R (C)')

;START170404
;wd1.fidl_sumconcpair=widget_button(wd1.malgebra,value='Sum conc pairs')

mregions = widget_button(wd1.mproc,value='Regions of Interest',MENU=3,/SEPARATOR)
wd1.fidl_reg_ass = widget_button(mregions,value='Assign values to regions')
wd1.convert_af3d_to_fidl = widget_button(mregions,value='Convert af3d or text file to fidl region file')
wd1.fidl_labelfile = widget_button(mregions,value='Convert label file to fidl region file')
wd1.compute_region_clusters = widget_button(mregions,value='Cluster regions')
wd1.define_regions = widget_button(mregions,value='Define regions')
wd1.delete_regions = widget_button(mregions,value='Delete regions')
wd1.fidl_flipreg = widget_button(mregions,value='Flip regions')

;START150326
;wd1.grow_regions = widget_button(mregions,value='Grow regions')

wd1.compute_region_stats = widget_button(mregions,value='Region center of mass and peak activation')
wd1.compute_tc_reg_stats = widget_button(mregions,value='Statistics')

;wd1.count_mask = widget_button(mregions,value='Sum regions')
wd1.count_mask = 0 

wd1.fix_region_file = widget_button(mregions,value='Fix region file',/SEPARATOR)

;START170719
wd1.fidl_fix_region_file=widget_button(mregions,value='Fix region file (new)')

;START160226
;wd1.mstim      = widget_button(wd1.mproc,value='Stimulus time-course',MENU=3,/SEPARATOR)
;wd.define_stimulus  = widget_button(wd1.mstim,value='Define')
;wd.edit_stimulus  = widget_button(wd1.mstim,value='Edit')
;wd.show_stimulus  = widget_button(wd1.mstim,value='Display')
;wd.save_stimulus  = widget_button(wd1.mstim,value='Save to ifh header')



wd1.msmoth      = widget_button(wd1.mproc,value='Smooth',MENU=3)
wd.smobox   = widget_button(wd1.msmoth,value='Boxcar filter')

;wd.smogauss   = widget_button(wd1.msmoth,value='Gaussian filter')
wd1.msmoth_gauss = widget_button(wd1.msmoth,value='Gaussian filter',MENU=3)
wd.smogauss = widget_button(wd1.msmoth_gauss,value='IDL')
wd.fidl_gauss = widget_button(wd1.msmoth_gauss,value='C (recommended for BOLD runs)')

wd.smobutter = widget_button(wd1.msmoth,value='Butterworth filter')
wd.overlay_actmap= widget_button(wd1.mproc,value='Overlay activation map')

matlas = widget_button(wd1.mproc,value='Transform to Atlas Space',MENU=3)
wd.atlas_transform = widget_button(matlas,value='Transform to Atlas Space (IDL)')
wd.atlas_transform_c = widget_button(matlas,value='Transform to Atlas Space (C)')

wd1.view = widget_button(wd1.mproc,value='Reslice images')
wd1.zoom  = widget_button(wd1.mproc,value='Zoom images')
;;;wd1.single_trial_resp = widget_button(wd1.mproc,value='Single trial response')

mconvert = widget_button(wd1.mproc,value='Convert',MENU=3)
wd1.t_to_z_IDL = widget_button(mconvert,value='T to Z statistic (IDL)')
wd1.fidl_t_to_z = widget_button(mconvert,value='T to Z statistic (C)')
wd1.f_to_z_IDL = widget_button(mconvert,value='F to Z statistic (IDL)',/SEPARATOR)
wd1.f_to_z_C = widget_button(mconvert,value='F to Z statistic (C)')
wd1.fidl_r_to_z = widget_button(mconvert,value='r to Z statistic (IDL)',/SEPARATOR)
wd1.fidl_r_to_zC = widget_button(mconvert,value='r to Z statistic (C)')
wd1.fidl_p_to_z = widget_button(mconvert,value='p to Z and Z to p',/SEPARATOR)

;if(!FIDL_ARCANE eq !TRUE) then begin
;    wd1.power_spectrum  = widget_button(wd1.mproc,value='Power spectral density')
;    mcorrel= widget_button(wd1.mproc,value='Correlation Analysis',MENU=3)
;    wd.covariance  = widget_button(mcorrel,value='Compute Correlations')
;    wd.analyze_cc  = widget_button(mcorrel,value='Analyze Correlations')
;    wd.create_cc_src  = widget_button(mcorrel,value='Create source voxel data file')
;    wd.pca_cc  = widget_button(mcorrel,value='Compute PCA for correlated regions')
;    wd.cov_butter  = widget_button(mcorrel,value='Compute Butterworth filtered correlations')
;endif

;START150326
;wd1.compute_pearson  = widget_button(wd1.mproc,value='Pearson product moment correlation coefficient')

wd1.compute_boynton  = widget_button(wd1.mproc,value='Boynton HRF')



mflip = widget_button(wd1.mproc,value='Flipper',MENU=3)

;wd1.fidl_flip = widget_button(mflip,value='Old')
;wd1.fidl_flipnew = widget_button(mflip,value='New')
;START160614
wd1.fidl_flipglm = widget_button(mflip,value='Flip GLM')
wd1.fidl_flip = widget_button(mflip,value='z-score flipper Old')
wd1.fidl_flipnew = widget_button(mflip,value='z-score flipper New')

wd1.fidl_histogram = widget_button(wd1.mproc,value='Histogram')


;wd1.fidl_hemitxt = widget_button(wd1.mproc,value='Hemisphere text')
;START110725
mhemi = widget_button(wd1.mproc,value='Hemisphere',MENU=3)
wd1.fidl_hemitxt = widget_button(mhemi,value='text')



;wd1.fidl_psd = widget_button(wd1.mproc,value='Power spectral density (under development)')
wd1.fidl_psd = 0 

;wd1.fidl_circor = widget_button(wd1.mproc,value='Circular correlation')
;wd1.fidl_circor = widget_button(wd1.mproc,value='Under development')
wd1.fidl_circor=0

;user_buttons,wd1.mproc,wd

;START150325
;wd1.fidl_checkglm = widget_button(wd1.mlinmod,value='Check linear model',/SEPARATOR)

mcompglm      = widget_button(wd1.mlinmod,value='Compute model',MENU=3)
wd1.comp_linmod  = widget_button(mcompglm,value='Compute linear model')
wd1.compute_residuals = widget_button(mcompglm,value='Compute residuals',/SEPARATOR)
;wd1.fidl_autocorrelation = widget_button(mcompglm,value='Compute AR(1)',/SEPARATOR)
wd1.fidl_autocorrelation=-1

;START150325
wd1.fidl_checkglm = widget_button(mcompglm,value='Check glm',/SEPARATOR)
wd1.fidl_glmcond = widget_button(mcompglm,value='Check condtion number')

wd1.fidl_rename_effects = widget_button(mcompglm,value='Rename effects',/SEPARATOR)
wd1.fidl_rename_paths = widget_button(mcompglm,value='Rename paths')
wd1.fidl_eog = widget_button(mcompglm,value='Create EOG regressors',/SEPARATOR)

;START141001
;wd1.fidl_motionreg = widget_button(mcompglm,value='Create motion regressors')
;START141113
wd1.fidl_motionreg = widget_button(mcompglm,value='Create motion regressors and scrub files')

;START140911
;wd1.fidl_motion = widget_button(mcompglm,value='Create scrub files')

;if(!FIDL_ARCANE eq !TRUE) then begin
;    wd1.comp_linmod_old  = widget_button(mcompglm,value='Compute linear model (Obsolete)')
;    wd1.glm_adjusted_df  = widget_button(mcompglm,value='Compute degrees of freedom')
;endif
mcompglmz   = widget_button(wd1.mlinmod,value='Compute Z statistics',MENU=3)
wd1.compute_zstat  = widget_button(mcompglmz,value='Linear contrasts w/wo delay (C)')
wd1.apply_linmod  = widget_button(mcompglmz,value='Linear contrasts (IDL)',/SEPARATOR)
wd1.apply_linmod_lag = widget_button(mcompglmz,value='Linear contrasts with delay (IDL)')
wd1.def_contrast1 = widget_button(mcompglmz,value='Contrast manager',/SEPARATOR)

wd1.extract_fzstat = widget_button(mcompglmz,value='Extract F statistics (IDL)',/SEPARATOR)
wd1.fidl_extract_fzstat = widget_button(mcompglmz,value='Extract F statistics (C)')

mcomptc = widget_button(wd1.mlinmod,value='Time courses',MENU=3)

;wd1.extract_timecourse_c = widget_button(mcomptc,value='Extract (C)')
;wd1.extract_timecourse = widget_button(mcomptc,value='Extract (IDL)')
;START120719
wd1.extract_timecourse = widget_button(mcomptc,value='Extract')

wd1.compute_tc_max_deflect = widget_button(mcomptc,value='Maximum deflection',/SEPARATOR)
wd1.compute_avg_zstat_boy = widget_button(mcomptc,value='Boynton fit')
wd1.fidl_tc_ss = widget_button(mcomptc,value='Widely spaced',/SEPARATOR)
wd1.fidl_logreg_ss = widget_button(mcomptc,value='Logistic regression')
wd1.fidl_cc = widget_button(mcomptc,value='Canonical Correlation')
wd1.fidl_fano  = widget_button(mcomptc,value='Fano factor/Coefficient of variation')

mcompmag = widget_button(wd1.mlinmod,value='Magnitudes',MENU=3)
wd1.extract_mag = widget_button(mcompmag,value='Extract (C)')
wd1.fidl_mvpa = widget_button(mcompmag,value='Multivoxel Pattern Analysis',/SEPARATOR)


mcompdsgn   = widget_button(wd1.mlinmod,value='Design matrix',MENU=3)
wd1.show_linmod  = widget_button(mcompdsgn,value='Show design matrix')
wd1.def_single_linmod_new  = widget_button(mcompdsgn,value='Define single trial design and model',/SEPARATOR)

;wd1.def_block_linmod  = widget_button(mcompdsgn,value='Define block design')
;START170109
wd1.def_single_linmod_new_trials=widget_button(mcompdsgn,value='Rapid event, trial estimates')
wd1.def_block_linmod=widget_button(mcompdsgn,value='Define block design',/SEPARATOR)


wd1.def_contrast  = widget_button(mcompdsgn,value='Contrast manager')

;START72
;wd1.copy_contrasts = widget_button(mcompdsgn,value='Copy contrasts')

;wd1.define_pet_linmod  = widget_button(mcompdsgn,value='Define PET design')
;if(!FIDL_ARCANE eq !TRUE) then begin
;    wd1.define_manual_linmod  = widget_button(mcompdsgn,value='Manually define design')
;;;;    wd1.define_optical_linmod  = widget_button(mcompdsgn,value='Define optical design')
;    wd1.define_cov_linmod  = widget_button(mcompdsgn,value='Define artifact removal design')
;    wd1.event_file_synopsis  = widget_button(mcompdsgn,value='Create synopsis of event file.')
;endif
;wd1.define_cov_linmod  = widget_button(mcompdsgn,value='Define artifact removal design')
wd1.export_dsgn_matrix  = widget_button(mcompdsgn,value='Export design matrix')
wd1.import_dsgn_matrix  = widget_button(mcompdsgn,value='Import design matrix')

mdsgn_matrix_metrics = widget_button(mcompdsgn,value='Print design matrix metrics',MENU=3)
wd1.dsgn_matrix_metrics  = widget_button(mdsgn_matrix_metrics,value='IDL')
wd1.fidl_dsgn_matrix_metrics  = widget_button(mdsgn_matrix_metrics,value='C')

;;;wd1.glm_sim_paradigm  = widget_button(wd1.mlinmod,value='Generate simulated paradigm')
wd1.save_linmod  = widget_button(wd1.mlinmod,value='Save linear model',/SEPARATOR)
wd1.load_linmod  = widget_button(wd1.mlinmod,value='Load linear model(s)')
wd1.load_glm_filter    = widget_button(wd1.mlinmod,value='File name filter')
;;;wd1.glm_simulate  = widget_button(wd1.mlinmod,value='Simulate GLM')
;;;wd.svd = widget_button(wd1.mlinmod,value='Principal Components')
;;;wd1.cross_corr  = widget_button(wd1.mlinmod,value='Cross-correlate time-courses')
;;;wd1.edit_linmod  = widget_button(wd1.mlinmod,value='Edit design matrix')

;wd1.fidl_anova = widget_button(wd1.mlinmod,value='Repeated Measures ANOVA',/SEPARATOR)
wd1.fidl_anova2 = widget_button(wd1.mlinmod,value='Repeated Measures ANOVA',/SEPARATOR)
wd1.fidl_ttest = widget_button(wd1.mlinmod,value='Compute T-test')
wd1.compute_avg_zstat = widget_button(wd1.mlinmod,value='Average statistics')

m2ndlevel   = widget_button(wd1.mlinmod,value='Second level',MENU=3)
wd1.fidl_2ndlevelmodel = widget_button(m2ndlevel,value='Model')
wd1.fidl_slopesintercepts = widget_button(m2ndlevel,value='Slopes and intercepts')
wd1.fidl_mvpa2 = widget_button(wd1.mlinmod,value='Multivoxel Pattern Analysis',/SEPARATOR)

;START170614
wd1.fidl_logreg_ss2=widget_button(wd1.mlinmod,value='Logistic regression')

mlme = widget_button(m2ndlevel,value='Linear mixed effects',/MENU)

;wd1.fidl_lmerr = widget_button(mlme,value='lmer from lme4 (R version)')
;START160218
wd1.fidl_lmerr = widget_button(mlme,value='lmer and anova from lmerTest (R version)')

wd1.fidl_anova_ss = widget_button(wd1.mlinmod,value='Single subject ANOVA'+string(10B)+'    Completely Randomized Design', $
    /SEPARATOR)
wd1.fidl_ftest_ss = widget_button(wd1.mlinmod,value='Single subject F test')
wd1.fidl_ttest_ss = widget_button(wd1.mlinmod,value='Single subject T test')

;mcorrelation_analysis = widget_button(wd1.mlinmod,value='Correlation analysis',MENU=3)
;wd1.correlation_analysis = widget_button(mcorrelation_analysis,value='Compute')
;wd1.scatter_plots = widget_button(mcorrelation_analysis,value='Extract scatter plots')
wd1.extract_mean  = widget_button(wd1.mlinmod,value='Extract mean',/SEPARATOR)
;wd1.extract_sdev  = widget_button(wd1.mlinmod,value='Extract std. dev.',/SEPARATOR)
wd1.extract_sdev  = widget_button(wd1.mlinmod,value='Extract std. dev.')
wd1.extract_trend  = widget_button(wd1.mlinmod,value='Extract trend')



;wd1.extract_cov  = widget_button(wd1.mlinmod,value='Extract correlation matrix')
;START70
mextract_cov = widget_button(wd1.mlinmod,value='Correlation matrix',MENU=3)
wd1.extract_cov = widget_button(mextract_cov,value='Extract')
wd1.fidl_glmsavecorr = widget_button(mextract_cov,value='Save as text')




;wd1.compute_epsilon = widget_button(wd1.mlinmod,value='Compute epsilon')

;wd1.fix_grand_mean  = widget_button(wd1.mlinmod,value='Fix mean',/SEPARATOR)
wd1.fix_grand_mean = 0
if !FIDL_ARCANE eq !TRUE then wd1.fix_grand_mean  = widget_button(wd1.mlinmod,value='Fix mean',/SEPARATOR)


wd1.fidl_ac = widget_button(wd1.mlinmod,value='Autocorrelation',/SEPARATOR)
wd1.fidl_crosscorr = widget_button(wd1.mlinmod,value='Cross correlation')
wd1.fidl_crosscov = widget_button(wd1.mlinmod,value='Cross covariance')
wd1.fidl_cov = widget_button(wd1.mlinmod,value='Variance-covariance')


;wd1.fidl_pca = widget_button(wd1.mlinmod,value='Principal components analysis')
;START170127
mfidl_pca=widget_button(wd1.mlinmod,value='Principal component analysis',MENU=3)
wd1.fidl_pca = widget_button(mfidl_pca,value='Extract spatial features')
wd1.fidl_pcat = widget_button(mfidl_pca,value='Extract temporal features')


;START120118
wd1.fidl_levene = widget_button(wd1.mlinmod,value="Levene's test")

wd.refresh  = widget_button(wd1.mdisp,value='Refresh')
;;;wd.loadct   = widget_button(wd1.mdisp,value='Color Tables') ; IDL version
wd.adjust_colortable   = widget_button(wd1.mdisp,value='Color Tables')
wd.zoomin = widget_button(wd1.mdisp,value='Zoom in')
wd.zoomout = widget_button(wd1.mdisp,value='Zoom out')
wd.zoomtype = widget_button(wd1.mdisp,value='Type of zoom')
wd.display_all = widget_button(wd1.mdisp,value='Display all planes')
wd1.display_comp = widget_button(wd1.mdisp,value='Compare all planes')
wd.new_wrkg_wdw = widget_button(wd1.mdisp,value='New working window')
wd.cine = widget_button(wd1.mdisp,value='Cine')

;wd.view_images = widget_button(wd1.mdisp,value='view25d4')
wd.view_images = widget_button(wd1.mdisp,value='view25d4',/SEPARATOR)
;wd.view26d4 = widget_button(wd1.mdisp,value='view26d4 (under development)')
wd.view26d4 = 0

;wd.view3d = widget_button(wd1.mdisp,value='Display in 3D')

;wd.print_all= widget_button(wd1.mdisp,value='Print all planes')
wd.print_all= widget_button(wd1.mdisp,value='Print all planes',/SEPARATOR)

wd.color_scale= widget_button(wd1.mdisp,value='Display color bar')


mcon = widget_button(wd1.mdisp,value='Contrast equalization',MENU=3)
wd.coneqoff = widget_button(mcon,value='Off')
wd.coneqon = widget_button(mcon,value='On')
widget_control,wd.coneqoff,SENSITIVE=0



mscale      = widget_button(wd1.mdisp,value='Scaling',MENU=3)
wd.local_scl= widget_button(mscale,value='Local')
wd.global_scl= widget_button(mscale,value='Global')
widget_control,wd.global_scl,SENSITIVE=0
wd.fixed_scl= widget_button(mscale,value='Fixed')
wd.fixed_glbl_scl= widget_button(mscale,value='Fixed global')




morient      = widget_button(wd1.mdisp,value='Display Orientation',MENU=3)
wd.radview= widget_button(morient,value='Radiological (right is left)')
wd.neuroview= widget_button(morient,value='Neuropsych (right is right)')
if(dsp[fi.cw].orientation eq !NEURO) then begin
    widget_control,wd.neuroview,SENSITIVE=0
    widget_control,wd.radview,/SENSITIVE
endif else begin
    widget_control,wd.radview,SENSITIVE=0
    widget_control,wd.neuroview,/SENSITIVE
endelse
mcustom= widget_button(wd1.mdisp,value='Custom Display',MENU=3)
wd.build_2x2= widget_button(mcustom,value='Build 2x2')
wd.build_custom= widget_button(mcustom,value='Build custom size')
wd.build_addimg= widget_button(mcustom,value='Add image')
wd.build_annotate= widget_button(mcustom,value='Annotate display')
wd.select_custom= widget_button(mcustom,value='Select custom display')
wd.crop_custom= widget_button(mcustom,value='Crop images')

;matlas= widget_button(wd1.mdisp,value='Atlas readout',MENU=3)
;wd1.atlas_2a = widget_button(matlas,value='Default')
;wd1.atlas_2b = widget_button(matlas,value='Used 2a but display as if 2b')
;widget_control,wd1.atlas_2a,SENSITIVE=0

mbandwidth= widget_button(wd1.mdisp,value='Network Speed',MENU=3)
wd1.low_bandwidth = widget_button(mbandwidth,value='Slow (e.g., modem)')
wd1.high_bandwidth = widget_button(mbandwidth,value='Fast (e.g., ethernet)')




wd.nextpln   = widget_button(butrow,value='Next Slice')
wd.lastpln    = widget_button(butrow,value='Prev Slice')
wd.nextfrm   = widget_button(butrow,value='Next Frame')
wd.lastfrm    = widget_button(butrow,value='Prev Frame')

;;;mbatch       = widget_button(butrow,value='Batch',MENU=3)
;;;wd1.build_macro = widget_button(mbatch,value='Build batch job macro')
;;;wd1.macro_comment = widget_button(mbatch,value='Insert comment')
;;;wd1.finish_macro = widget_button(mbatch,value='Exit macro build')
;;;wd1.compile_macro = widget_button(mbatch,value='Compile batch job macro')
;;;widget_control,wd1.finish_macro,SENSITIVE=0
;;;widget_control,wd1.macro_comment,SENSITIVE=0
;;;widget_control,wd1.compile_macro,SENSITIVE=0
;;;widget_control,wd1.build_macro,/SENSITIVE

wd.repeat_action = widget_button(butrow,value='Rpt-1')
wd.repeat_action_m1 = widget_button(butrow,value='Rpt-2')

;START4
;wd.scale = cw_bgroup(butrow1,['volume','slice'],SET_VALUE=0,FRAME=2,/EXCLUSIVE,ROW=1,/NO_RELEASE,LABEL_LEFT='Scale intensity')
wd.scale = 0
;wd.conadj = cw_bgroup(butrow1,['Off','On'],SET_VALUE=0,FRAME=2,/EXCLUSIVE,ROW=1,/NO_RELEASE,LABEL_LEFT='Adjust contrast')







;;;wd.roinum   = widget_label(roirow,value='Region # -1')
;;;wd.roivol   = cw_field(roirow,TITLE='Vol: ',VALUE='-1',/ALL_EVENTS,XSIZE=4)
;;;wd.nxtvol = widget_button(roirow,value='Next vol #')

wd.sld_pln = widget_slider(filerow,Title='Slice Number',SCROLL=1,FRAME=5,XSIZE=450,SENSITIVE=0,MINIMUM=1)
wd.sld_frm = widget_slider(filerow,Title='Frame Number',SCROLL=1,FRAME=5,XSIZE=450,SENSITIVE=0,MINIMUM=1)

;wd.sld_pln = widget_slider(filerow,Title='Slice Number',SCROLL=1,FRAME=5,SCR_XSIZE=100,SENSITIVE=0,MINIMUM=1)
;wd.sld_frm = widget_slider(filerow,Title='Frame Number',SCROLL=1,FRAME=5,SCR_XSIZE=100,SENSITIVE=0,MINIMUM=1)

;wd.sld_pln = widget_slider(filerow,Title='Slice Number',SCROLL=1,FRAME=5,XSIZE=500,SENSITIVE=0,MINIMUM=1)
;wd.sld_frm = widget_slider(filerow,Title='Frame Number',SCROLL=1,FRAME=5,XSIZE=500,SENSITIVE=0,MINIMUM=1)

;wd.sld_frm = widget_slider(filerow,Title='Frame Number',SCROLL=1,FRAME=5,XSIZE=500,SCR_XSIZE=480,SENSITIVE=0,MINIMUM=1)
;wd.sld_frm = widget_slider(filerow,Title='Frame Number',SCROLL=1,FRAME=5,SENSITIVE=0,MINIMUM=1)
;wd.sld_frm = widget_slider(filerow,Title='Frame Number',SCROLL=1,FRAME=5,SENSITIVE=0,MINIMUM=1,SCR_XSIZE=480)
;wd.sld_frm = widget_slider(filerow,Title='Frame Number',SCROLL=1,FRAME=5,SENSITIVE=0,MINIMUM=1,XOFFSET=10)

wd.error   = widget_text(filerow,VALUE=' ',YSIZE=1)
wd.files   = widget_list(filerow,VALUE=fi.list,SCR_YSIZE=300,SCR_XSIZE=500,/MULTIPLE)

widget_control,wd.leader,/REALIZE

xmanager,'stats_exec',wd.leader

end
