;Copyright 12/31/99 Washington University.  All Rights Reserved.
;view_images.pro  $Revision: 1.171 $
;***********************
pro view_images_event,ev
;***********************
common stats_comm,wd,wd1,fi,pr,dsp,fl,st,stc,cstm,bltimg,glm,dsp_image,help,pref,first_pass,xm1,ym1,vw

common view_images_comm_shared,lc_refreshed,prf,win,lc_color_bar_once,wd_hidecolors,wd_showcolors_vert, $
    wd_showcolors_horiz,current_label,current_cells,display_mode,af3dh,lcview,lc_add_label_file

common view_images_comm,view_leader,wd_cut,wd_copy,wd_paste,wd_refresh,wd_exit,wd_reslice_redraw,wd_slice_order, $
    wd_file,last_cell,lc_depress,xd0,yd0,wd_coord,wd_outlines,trans_outlines,show_outlines, $
    trans_outline_table,wd_format,wd_disp,wd_load_pref,wd_load_pref2,wd_save_pref,wd_save_def_pref,wd_slice_def,wd_load, $
    sprf,wd_idxidx,wd_base_file,wd_over_file,wd_view,wd_orient,wd_first,wd_last,wd_increment,wd_sag_face, $
    wd_clear,wd_no_outlines,wd_overlay,wd_top_bot,xxm1,yym1,wd_append_set,wd_erase_set,wd_erase_selected, $
    wd_delete_all,wd_scale_zoom,pref_path,wd_save_pref_path,pref_files,wd_load_pref_list,wd_slice_type, $
    wd_move,wd_deselect,drag_mode,x0,y0,x1,y1,dragm1,moved_cells,wd_reload,wd_slice_def_draw,wd_insert_set, $
    wd_data_set_status,wd_delete_image_set,cells_to_print,wd_print_all,wd_print_file, $
    wd_print_setup,print_destination,preference_file,wd_preference_file,wd_load_slice_pref,wd_wdw_name,cwm1, $
    wd_new_window,wd_print_selected,wd_delete_cell,print_file_name,wd_load_def_pref,wd_tiff,wd_label_font, $
    wd_add_label_file,wd_colortables,wd_tiff_selected,wd_add_label,wd_delete_label,lc_add_label,drag_x0, $
    drag_y0,wd_edit_label,wd_label_color,wd_load_user_pref,lc_cut,cut_cells, $
    image_sets,wd_font_name,wd_font_size,wd_font_weight,wd_font_slant,font_list,font_names,nfonts,font_families, $
    font_sizes,wdw_names,wd_slicer,slicer_zdim,slicer_z,lc_add_mark,wd_add_mark, $
    atlaslabels,atlasfiles,natlas,wd_af3d_loci,wd_points,wd_freeze_cell,wd_reg,wd_reg_ass, $
    over_image,voxval,wd_regname,regnames,regnamesi,maskfi,lc_delete_cells,wd_plot,wd_tiff_all, $
    base_path,over_path

lc_draw = !FALSE
lc_refreshed = !FALSE
event = !WIDGET
set_plot,'X'             ; Recover from errors - make sure you can write to screen.
if fi.current_colors ne !NUM_FILES then begin
    fi.current_colors = !NUM_FILES
    view_images_lct,fi.color_scale1[!NUM_FILES],fi.color_scale2[!NUM_FILES]
endif
for i=0,vw.num_windows-1 do begin
    if(ev.id eq vw.id[i] ) then begin
            vw.cw = i
            if vw.cw ne cwm1 then begin
                cwm1 = vw.cw
                if(ptr_valid(vw.win[vw.cw])) then begin
                    win = *vw.win[vw.cw]
                    sprf = win.sprf[win.idxidx]
                    prf = win.prf
                endif
                update_text,fi,help,vw,win
            endif
            wset,vw.wdw[vw.cw]
            lc_draw = !TRUE
            iwdw = i
        case ev.type of
            0: begin
                case ev.clicks of
                    0:
                    1: begin
                        event = !PRESS
                        lc_depress = !TRUE
                        drag_x0 = ev.x
                        drag_y0 = ev.y
;;;print,'press'
                    end
                    2: begin
                        event = !DOUBLE_CLICK
;;;print,'double_click'
                        lc_depress = !FALSE
                        if(iwdw ne vw.cw) then begin
                            vw.cw = iwdw
                            wset,vw.wdw[vw.cw]
                            win = *vw.win[vw.cw]
                            prf = win.prf
                            sprf = win.sprf[win.idxidx]
                            update_text,fi,help,vw,win
                        endif
                    end
                    else: print,'Invalid value of clicks.'
                endcase
            end
            1: begin
                if(lc_depress eq !TRUE) then begin
                    event = !RELEASE
;;;print,'release'
                    lc_depress = !FALSE
                endif
            end
            2: begin
                if(lc_depress eq !TRUE) then begin
                    if((ev.x-drag_x0)^2+(ev.y-drag_y0)^2 gt 10) then $
                        event = !DRAG  $
                    else $
                        return
;;;print,'drag'
                endif else begin
                    event = !MOTION
;;;print,'motion'
                endelse
            end
            else: print,'Invalid value of ev.type.'
         endcase
    endif
endfor

for i=0,vw.num_windows-1 do begin
    if ev.id eq vw.base[i] then begin
        win.xdim_draw = ev.x
        win.ydim_draw = ev.y
        win.ymax = ev.y

        ;widget_control,vw.base[i],XSIZE=win.xdim_draw,YSIZE=win.ydim_draw
        widget_control,vw.base[i],SCR_XSIZE=win.xdim_draw,SCR_YSIZE=win.ydim_draw 
            ;This eliminates the vertical grey band on the left.

        widget_control,vw.id[i],XSIZE=win.xdim_draw,YSIZE=win.ydim_draw,DRAW_XSIZE=win.xdim_draw
        widget_control,vw.view_leader,/SHOW
        win.lastx = 0
        win.lasty = 0
        reload,fi,dsp,vw
        view_images_refresh,fi,dsp,vw.fnt
        ev.id = -101
    endif
endfor

if(event ne !WIDGET) then begin
    if(win.ncell eq 0) then $
        return
    case event of
        !PRESS: begin
            dragm1 = !FALSE
            if lc_add_label then begin
                lc_add_label = !FALSE
                labels = *win.labels
                labels[current_label].x = ev.x
                labels[current_label].y = win.ydim_draw - ev.y - 1
                if lc_add_label_file eq !TRUE then begin ;BETTER WAY IS TO UPDATE THE LABELS WHEN THE IMAGES ARE UPDATED.
                    name = win.filenamelabels[win.idxidx]
                    lc_add_label_file = !FALSE
                endif else $
                    name = labels[current_label].s 
                xyouts,ev.x,ev.y,name,/DEVICE,COLOR=win.prf.label_color+!LEN_COLORTAB,CHARSIZE=2,WIDTH=width
                labels[current_label].wdth = long(width*win.xdim_draw)
                labels[current_label].hgth = 15
                *win.labels = labels
                outline_label,win,current_label,win.ydim_draw,!GREEN
                device,cursor_standard=34
                checked = *win.label[win.idxidx].checked
                checked[current_label] = 1
                *win.label[win.idxidx].checked = checked
                return
            endif else if(lc_add_mark) then begin
                ;print,'x: ',ev.x,' y: ',ev.y
                ;plot_symbol,ev.x,ev.y,6,!YELLOW,2
                ;plot_symbol,ev.x,ev.y,win.prf.points_shape,win.prf.points_color,win.prf.points_size
                plot_symbol,ev.x,ev.y,win.prf.points_shape,win.prf.points_color+!WHITE,win.prf.points_size
            endif
        end
        !RELEASE: begin
            if(dragm1 eq !TRUE) then begin
                case drag_mode of
                    !MOVE_CELL: begin
                        dragm1 = !FALSE
                        device,SET_GRAPHICS=3,/cursor_crosshair
                        delta_x = ev.x - x0
                        delta_y = y0 - ev.y
                        nc = n_elements(current_cells)
                        for ic=0,nc-1 do begin
                            if((abs(delta_x) gt 2) or (abs(delta_y) gt 2)) then begin
                                xnew = win.cell[moved_cells[ic]].x + delta_x
                                ynew = win.cell[moved_cells[ic]].y + delta_y
                                if(xnew lt 0) then xnew = 0
                                if(xnew+win.cell[moved_cells[ic]].xdim ge win.xdim_draw) then $
                                    xnew = win.xdim_draw - win.cell[moved_cells[ic]].xdim - 1
                                if(ynew lt 0) then ynew = 0
                                if(ynew+win.cell[moved_cells[ic]].ydim ge win.ydim_draw) then $
                                    ynew = win.ydim_draw - win.cell[moved_cells[ic]].ydim - 1
                                win.cell[moved_cells[ic]].x = xnew
                                win.cell[moved_cells[ic]].y = ynew
                                *vw.win[vw.cw] = win
                             endif
                        endfor
                        view_images_refresh,fi,dsp,vw.fnt
                        for ic=0,nc-1 do begin
                             outline_cell,win,current_cells[ic],prf.zoom,win.ydim_draw,display_mode,!GREEN
                        endfor
                    end
                    !MOVE_LABEL: begin
                        dragm1 = !FALSE
                        device,SET_GRAPHICS=3,/cursor_crosshair
                        delta_x = ev.x - x0
                        delta_y = y0 - ev.y
                        labels = *win.labels
                        if((abs(delta_x) gt 2) or (abs(delta_y) gt 2)) then begin
                            xnew = labels[current_label].x + delta_x
                            ynew = labels[current_label].y + delta_y
                            if(xnew lt 0) then xnew = 0
                            if(xnew+labels[current_label].wdth ge win.xdim_draw) then $
                                xnew = win.xdim_draw - labels[current_label].wdth - 1
                            if(ynew lt 0) then ynew = 0
                            if(ynew+labels[current_label].hgth ge win.ydim_draw) then $
                                ynew = win.ydim_draw - labels[current_label].hgth - 1
                            labels[current_label].x = xnew
                            labels[current_label].y = ynew
                            *win.labels = labels
                            *vw.win[vw.cw] = win
                         endif
                         view_images_refresh,fi,dsp,vw.fnt
                    end
                    !DEFINE_BOX: begin
                        device,SET_GRAPHICS=6
                        plots,[x0,xxm1,xxm1,x0,x0],[y0,y0,yym1,yym1,y0],/DEV,/NOCLIP
                        device,SET_GRAPHICS=3
                        for ic=0,n_elements(current_cells)-1 do $
                            outline_cell,win,current_cells[ic],prf.zoom,win.ydim_draw,display_mode,0
                        find_cells_in_box,win,current_cells,x0,win.ydim_draw-y0-1,xxm1,win.ydim_draw-yym1-1,prf.zoom,display_mode
                        for ic=0,n_elements(current_cells)-1 do  begin
                            if(current_cells[ic] ge 0) then $
                                outline_cell,win,current_cells[ic],prf.zoom,win.ydim_draw,display_mode,!GREEN
                        endfor
                    end
                    else: print,'Invalid value of drag_mode.'
                endcase
            endif
        end
        !DOUBLE_CLICK: begin
            x = ev.x
            y = win.ydim_draw - ev.y - 1
            if(ev.clicks eq 2) then begin
                last_cells = current_cells
                nc = n_elements(last_cells)
                find_label_num,current_label,win,x,y
                if(current_label lt 0) then begin
                    fnd = find_cell(win,x,y,prf.zoom,display_mode)
                    current_cells = fnd.itr
                endif else begin
                    current_cells = -1
                endelse
                if(nc gt 0) then $
                    view_images_refresh,fi,dsp,vw.fnt
                if(current_cells ge 0) then $
                    outline_cell,win,current_cells,prf.zoom,win.ydim_draw,display_mode,!GREEN
                if(current_label ge 0) then $
                    outline_label,win,current_label,win.ydim_draw,!GREEN
            endif
            dragm1 = !FALSE
          end
          !MOTION: begin
              y = win.ydim_draw - ev.y - 1
              fnd = find_cell(win,ev.x,y,prf.zoom,display_mode)
              cc = fnd.itr
              lnum = fnd.lnum
              if(cc ge 0) then begin
                cc = win.translation_table[cc]
                if(win.cell[cc].view ne !VIEW_COLOR_SCALE) then begin
                  if(display_mode eq !ANAT_OVER_ACT_MODE) then begin
                      xxx = intarr(16)
                      xxx[*] = 32769
                      xxx[7] = -1
                      xxx[8] = -1
                      if(!VERSION.OS_FAMILY eq 'Windows') then $
                          device,set_graphics=6,/CURSOR_CROSSHAIR $
                      else $
                          device,set_graphics=6,cursor_image=xxx
                      y = ev.y
                      x = ev.x
                      yp = win.ydim_draw - ev.y - 1 - win.cell[cc].y
                      if(yp gt prf.zoom*win.cell[cc].ydim) then $
                          y = y + prf.zoom*win.cell[cc].ydim $
                      else $
                          y = y - prf.zoom*win.cell[cc].ydim 
                      d = 10
                      plots,[xxm1-d,xxm1,xxm1+d,xxm1,xxm1-d],[yym1,yym1+d,yym1,yym1-d,yym1],COLOR=dsp[fi.cw].green,/DEV,/NOCLIP
                      plots,[xxm1-d,xxm1+d],[yym1,yym1],COLOR=dsp[fi.cw].green,/DEV,/NOCLIP
                      plots,[xxm1,xxm1],[yym1-d,yym1+d],COLOR=dsp[fi.cw].green,/DEV,/NOCLIP
                      plots,[x-d,x,x+d,x,x-d],[y,y+d,y,y-d,y],COLOR=dsp[fi.cw].green,/DEV,/NOCLIP
                      plots,[x-d,x+d],[y,y],COLOR=dsp[fi.cw].green,/DEV,/NOCLIP
                      plots,[x,x],[y-d,y+d],COLOR=dsp[fi.cw].green,/DEV,/NOCLIP
                      xxm1 = x
                      yym1 = y
                      device,SET_GRAPHICS=3,/cursor_crosshair ;Resume normal graphics.
                  endif
                  if ptr_valid(win.cell[cc].overlay_image) then begin
                      img = *win.cell[cc].overlay_image_val
                      if ptr_valid(win.cell[cc].base_image) then img2 = *win.cell[cc].base_image_val
                  endif else if ptr_valid(win.cell[cc].base_image) then begin
                      img = *win.cell[cc].base_image_val
                  endif else $
                      img = fltarr(win.cell[cc].xdim,win.cell[cc].ydim)
                  y = win.ydim_draw - ev.y - 1 - win.cell[cc].y

                  if(display_mode eq !ANAT_OVER_ACT_MODE) then begin
                      if(y gt prf.zoom*win.cell[cc].ydim) then $
                          y = y - prf.zoom*win.cell[cc].ydim
                  endif

                  ;print,'view_images y=',y,' win.ydim_draw=',win.ydim_draw,' ev.y=',ev.y,' win.cell[cc].y=',win.cell[cc].y
                  ;print,'win.cell[cc].xpad=',win.cell[cc].xpad,' win.cell[cc].ypad=',win.cell[cc].ypad
                  ;print,'view_images here100 win.xmax=',win.xmax,' win.ymax=',win.ymax
                  ;print,'view_images here100 dsp[fi.cw].xdim=',dsp[fi.cw].xdim,' dsp[fi.cw].ydim=',dsp[fi.cw].ydim


                  ;junki=where(img gt 1.e-37,junkn) 
                  ;print,'img[junki]=',img[junki]
                  ;print,'junkn=',junkn

                  ;if ptr_valid(win.cell[cc].overlay_image) and ptr_valid(win.cell[cc].base_image) then $
                  ;    v = double(get_atlas_coords(ev.x,y,win.cell[cc].slice,win.cell[cc].x,win.cell[cc].y,win.cell[cc].view, $
                  ;        win.atlas_space[win.idxidx],win.cell[cc].xpad,win.cell[cc].ypad,prf.zoom, $
                  ;        win.sprf[win.idxidx].sagittal_face_left,prf.orientation,win.cell[cc].xdim,win.cell[cc].ydim,fi,stc,dsp, $
                  ;        IMAGE=img,SECOND_IMAGE=img2)) $
                  ;else $
                  ;    v = double(get_atlas_coords(ev.x,y,win.cell[cc].slice,win.cell[cc].x,win.cell[cc].y,win.cell[cc].view, $
                  ;        win.atlas_space[win.idxidx],win.cell[cc].xpad,win.cell[cc].ypad,prf.zoom, $
                  ;        win.sprf[win.idxidx].sagittal_face_left,prf.orientation,win.cell[cc].xdim,win.cell[cc].ydim,fi,stc,dsp, $
                  ;        IMAGE=img))
                  ;START180308 Works for 111MN1 cf_flip=12
                  if ptr_valid(win.cell[cc].overlay_image) and ptr_valid(win.cell[cc].base_image) then $ 
                      superbird=img2 $
                  else $
                      superbird=0

                  ;print,'    prf.xgap=',prf.xgap,' prf.ygap=',prf.ygap

                  ;junki=where(superbird eq 791.5,junkn)
                  ;print,'junkn=',junkn
                  ;print,superbird

                  ;v = double(get_atlas_coords(ev.x,y,win.cell[cc].slice,win.cell[cc].x,win.cell[cc].y,win.cell[cc].view, $
                  v = double(get_atlas_coords(ev.x,y,win.cell[cc].slice,win.cell[cc].x,prf.ygap,win.cell[cc].view, $
                      win.atlas_space[win.idxidx],win.cell[cc].xpad,win.cell[cc].ypad,prf.zoom, $
                      win.sprf[win.idxidx].sagittal_face_left,prf.orientation,win.cell[cc].xdim,win.cell[cc].ydim,fi,stc,dsp, $
                      IMAGE=img,SECOND_IMAGE=superbird,DSPYDIM=dsp[fi.cw].ydim))
                      ;IMAGE=superbird,SECOND_IMAGE=superbird,DSPYDIM=dsp[fi.cw].ydim))


                  ;print,'ev.x=',ev.x,' y=',y,' win.cell[cc].slice=',win.cell[cc].slice,' win.cell[cc].x=',win.cell[cc].x
                  ;print,'win.cell[cc].y=',win.cell[cc].y,' win.cell[cc].view=',win.cell[cc].view,' win.atlas_space=', $
                  ;    win.atlas_space
                  ;print,'win.cell[cc].xpad=',win.cell[cc].xpad,' win.cell[cc].ypad

                  case prf.display_units of
                      !DISPLAY_Z:
                      !DISPLAY_P: begin
                         v[3] = t_pdfc(v[3],1.e8)
                      end
                      !DISPLAY_P_ROUNDED: begin
                         scrap = strsplit(string(t_pdfc(v[3],1.e8),FORMAT='(e)'),'e',/EXTRACT)
                         v[3] = 10.^double(fix(scrap[1])+1)
                      end
                      else: print,'Invalid specification of display units.'
                  endcase
                  if ptr_valid(win.cell[cc].overlay_image) and ptr_valid(win.cell[cc].base_image) then begin 
                      widget_control,wd_coord,SET_VALUE=get_space_str(win.atlas_space[win.idxidx],v,LABEL='overlay', $
                          SECOND_LABEL='base',CODING=prf.coding)
                      if win.nreg[win.idxidx] gt 0 and v[3] ge 2 then begin 
                          if regnames[2] ne '' then widget_control,wd_regname,SET_VALUE=regnames[v[3]]
                      endif else begin 
                          widget_control,wd_regname,SET_VALUE=''
                      endelse
                  endif else $
                      widget_control,wd_coord,SET_VALUE=get_space_str(win.atlas_space[win.idxidx],v,CODING=prf.coding)
                endif else begin
                  widget_control,wd_coord,SET_VALUE=''
                endelse
              endif else begin
                  widget_control,wd_coord,SET_VALUE=''
              endelse
              dragm1 = !FALSE
          end
          !DRAG: begin
              if(current_cells[0] ge 0) then begin
                  drag_mode = !MOVE_CELL
                  ncells = n_elements(current_cells)
                  for ic=0,ncells-1 do $
                      outline_cell,win,current_cells[ic],prf.zoom,win.ydim_draw,display_mode,0
                  xxx = intarr(16)
                  if(!VERSION.OS_FAMILY eq 'Windows') then $
                      device,set_graphics=6,/CURSOR_CROSSHAIR $
                  else $
                      device,set_graphics=6,cursor_image=xxx
                  for ic=0,ncells-1 do begin
                      if(dragm1 eq !FALSE) then begin
                          x0 = ev.x
                          y0 = ev.y
                          xxm1 = x0
                          yym1 = y0
                          moved_cells[ic] = win.translation_table[current_cells[ic]]
                      endif else begin
                          delta_x = ev.x - x0
                          delta_y = ev.y - y0
                          outline_drag_cell,win,current_cells[ic],prf.zoom,win.ydim_draw,display_mode,!GREEN,xxm1-x0,y0-yym1
                          outline_drag_cell,win,current_cells[ic],prf.zoom,win.ydim_draw,display_mode,!GREEN,ev.x-x0,y0-ev.y
                      endelse
                  endfor
                  if(dragm1 eq !FALSE) then $
                      dragm1 = !TRUE
              endif else begin
                  if(current_label ge 0) then begin
                      drag_mode = !MOVE_LABEL
                      xxx = intarr(16)
                      if(!VERSION.OS_FAMILY eq 'Windows') then $
                          device,set_graphics=6,/CURSOR_CROSSHAIR $
                      else $
                          device,set_graphics=6,cursor_image=xxx
                      if(dragm1 eq !FALSE) then begin
                          x0 = ev.x
                          y0 = ev.y
                          xxm1 = x0
                          yym1 = y0
                      endif else begin
                          delta_x = ev.x - x0
                          delta_y = ev.y - y0
                          outline_drag_label,win,current_label,win.ydim_draw,!GREEN,xxm1-x0,y0-yym1
                          outline_drag_label,win,current_label,win.ydim_draw,!GREEN,ev.x-x0,y0-ev.y
                      endelse
                      if(dragm1 eq !FALSE) then $
                          dragm1 = !TRUE
                  endif else begin
                      drag_mode = !DEFINE_BOX
;                     Select range of objects.
                      device,set_graphics=6
                      if(dragm1 eq !FALSE) then begin
                          dragm1 = !TRUE
                          x0 = ev.x
                          y0 = ev.y
                          xxm1 = x0
                          yym1 = y0
                      endif else begin
                          plots,[x0,xxm1,xxm1,x0,x0],[y0,y0,yym1,yym1,y0],/DEV,/NOCLIP
                          plots,[x0,x0,ev.x,ev.x,x0],[y0,ev.y,ev.y,y0,y0],/DEV,/NOCLIP
                      endelse
                      device,set_graphics=3
                  endelse
              endelse
              xxm1 = ev.x
              yym1 = ev.y
          end
       else: begin
              print,'Invalid value of event: ',event
              dragm1 = !FALSE
       end
    endcase
endif

case ev.id of
    -101: 
    wd_load: begin
        labels = [atlaslabels,'On disk']
        get_filelist_labels,fi,nn,tails,index
        if nn ne 0 then labels = [tails,labels]
        labels = [labels,'Cancel']
        repeat begin
            idx = get_button(labels,TITLE='Base image.',GROUP_LEADER=vw.view_leader)
        endrep until labels[idx] ne !BLANK
        widget_control,/HOURGLASS
        win.image_idx[win.num_idx] = -1
        if idx lt nn then begin
            win.image_idx[win.num_idx] = index[idx]
        endif else if idx ge nn and idx lt nn+natlas then begin
            for i=0,fi.nfiles-1 do if fi.names[i] eq atlasfiles[idx-nn] then win.image_idx[win.num_idx] = i
            if win.image_idx[win.num_idx] lt 0 then $
                stat = load_data(fi,st,dsp,wd,stc,!MRI_4DFP,help,glm,FILENAME=atlasfiles[idx-nn],/NOLOAD_COLOR)
        endif else if idx eq nn+natlas then begin
            if win.image_idx[win.num_idx] lt 0 then begin 
                fi.path = base_path

                ;stat = load_data(fi,st,dsp,wd,stc,!MRI_4DFP,help,glm,/NOLOAD_COLOR)
                ;START180118
                stat = load_data(fi,st,dsp,wd,stc,!MRI_4DFP,help,glm,/NOLOAD_COLOR,FILTER=['*img','*nii','*nii.gz'])

                base_path = fi.path
            endif
        endif else begin
            goto,wd_load_bottom
        endelse
        if win.image_idx[win.num_idx] lt 0 then begin
            if stat eq !OK then win.image_idx[win.num_idx] = fi.n
        endif
        if win.image_idx[win.num_idx] ne -1 then begin
            hdr = *fi.hdr_ptr[win.image_idx[win.num_idx]]
            win.atlas_space[win.num_idx] = get_space(hdr.xdim,hdr.ydim,hdr.zdim)
            win.mask_idx[win.num_idx] = -1
            mask_file = ''
            case win.atlas_space[win.num_idx] of
                !SPACE_DATA: begin
                    ;do nothing
                end
                !SPACE_111: begin
                    if maskfi[1] eq -1 then begin
                        mask_file = !MASK_FILE_111
                        scrap = 1
                    endif else $
                        win.mask_idx[win.num_idx] = maskfi[1]
                end
                !SPACE_222: begin
                    if maskfi[2] eq -1 then begin
                        mask_file = !MASK_FILE_222
                        scrap = 2
                    endif else $
                        win.mask_idx[win.num_idx] = maskfi[2]
                end
                !SPACE_333: begin
                    if maskfi[3] eq -1 then begin
                        mask_file = !MASK_FILE_333
                        scrap = 3
                    endif else $
                        win.mask_idx[win.num_idx] = maskfi[3]
                end


                ;else: print,'Invalid value of win.atlas_space.'
                ;START180118
                !SPACE_MNI111: begin
                    if maskfi[6] eq -1 then begin
                        mask_file = !mni_icbm152_t1_tal_nlin_sym_09a_mask 
                        scrap = 6
                    endif else $
                        win.mask_idx[win.num_idx] = maskfi[6]
                end
                else: print,'Invalid value of win.atlas_space = ',win.atlas_space[win.num_idx]


            endcase
            if mask_file ne '' then begin
                stat = load_data(fi,st,dsp,wd,stc,!MRI_4DFP,help,glm,FILENAME=mask_file,/NOLOAD_COLOR)
                if stat eq !OK then begin
                    win.mask_idx[win.num_idx] = fi.n
                    maskfi[scrap] = fi.n
                endif else begin
                    print,' *** Error while loading file. ***'
                    return
                endelse
            endif
            labs = strarr(nn+2)
            if nn gt 0 then labs[0:nn-1] = labels[0:nn-1]
            labs[nn] = 'On disk'
            labs[nn+1] = 'None'
            

            ;idx = get_button(labs,TITLE='Overlay image.',GROUP_LEADER=vw.view_leader)
            ;nspider = 1
            ;spider = -1
            ;if idx lt nn then begin
            ;    spider = index[idx]
            ;endif else begin
            ;    if idx eq nn then begin
            ;        nfilesi0 = fi.nfiles
            ;        fi.path = over_path
            ;        stat = load_data(fi,st,dsp,wd,stc,!MRI_4DFP,help,glm,/NOLOAD_COLOR,/MULTIPLE_FILES)
            ;        over_path = fi.path
            ;        if stat eq !OK then begin
            ;            nspider = fi.nfiles - nfilesi0
            ;            spider = indgen(nspider) + nfilesi0
            ;        endif
            ;    endif
            ;endelse
            ;for i=0,nspider-1 do begin
            ;    if i gt 0 then begin
            ;        win.image_idx[win.num_idx] = win.image_idx[win.num_idx-1]
            ;        win.atlas_space[win.num_idx] = win.atlas_space[win.num_idx-1]
            ;        win.mask_idx[win.num_idx] = win.mask_idx[win.num_idx-1]
            ;    endif
            ;    tdim = 1
            ;    win.over_idx[win.num_idx] = spider[i]
            ;    if win.over_idx[win.num_idx] ne -1 then begin
            ;        hdr = *fi.hdr_ptr[win.over_idx[win.num_idx]]
            ;        tdim = hdr.tdim
            ;    endif
            ;    for j=0,tdim-1 do begin
            ;        if j gt 0 then begin
            ;            win.image_idx[win.num_idx] = win.image_idx[win.num_idx-1]
            ;            win.atlas_space[win.num_idx] = win.atlas_space[win.num_idx-1]
            ;            win.mask_idx[win.num_idx] = win.mask_idx[win.num_idx-1]
            ;            win.over_idx[win.num_idx] = spider[i]
            ;        endif
            ;        if win.over_idx[win.num_idx] ne -1 then begin
            ;            win.over_tidx[win.num_idx] = j
            ;            win.nreg[win.num_idx] = hdr.ifh.nreg
            ;            if win.nreg[win.num_idx] gt 0 then win.prf.overlay_zoom_type = !NEAREST_NEIGHBOR
            ;            win.filenamelabels[win.num_idx] = fi.list[win.over_idx[win.num_idx]]
            ;            if ptr_valid(win.labels) then begin
            ;                labels = *win.labels
            ;                labels[0].s = fi.list[win.over_idx[win.num_idx]]
            ;                *win.labels = labels
            ;            endif
            ;            regnames=['','','']
            ;        endif
            ;        win.idxidx = win.num_idx
            ;        win.num_idx = win.num_idx + 1
            ;        if win.idxidx gt 0 then $
            ;            win.sprf[win.idxidx] = win.sprf[win.idxidx-1] $
            ;        else $
            ;            win.sprf[win.idxidx] = sprf
            ;        update_text,fi,help,vw,win
            ;        widget_control,wd_idxidx,SET_SLIDER_MAX=win.num_idx
            ;        widget_control,wd_idxidx,SET_VALUE=win.num_idx
            ;        if win.num_idx gt 1 then widget_control,wd_idxidx,/SENSITIVE
            ;    endfor
            ;endfor
            ;START130327
            rtn = get_bool_list(labs,TITLE='Overlay image.',GROUP_LEADER=vw.view_leader)
            idx = where(rtn.list eq 1,cnt)
            for k=0,cnt-1 do begin
                nspider = 1
                spider = -1
                if idx[k] lt nn then begin
                    spider = index[idx[k]]
                endif else if idx[k] eq nn then begin
                    nfilesi0 = fi.nfiles
                    fi.path = over_path

                    ;stat = load_data(fi,st,dsp,wd,stc,!MRI_4DFP,help,glm,/NOLOAD_COLOR,/MULTIPLE_FILES)
                    ;START180118
                    stat=load_data(fi,st,dsp,wd,stc,!MRI_4DFP,help,glm,/NOLOAD_COLOR,/MULTIPLE_FILES,FILTER=['*img','*nii','*nii.gz'])

                    over_path = fi.path
                    if stat eq !OK then begin
                        nspider = fi.nfiles - nfilesi0
                        spider = indgen(nspider) + nfilesi0        
                    endif
                endif
                for i=0,nspider-1 do begin
                    if i gt 0 or k gt 0 then begin
                        win.image_idx[win.num_idx] = win.image_idx[win.num_idx-1]
                        win.atlas_space[win.num_idx] = win.atlas_space[win.num_idx-1]
                        win.mask_idx[win.num_idx] = win.mask_idx[win.num_idx-1]
                    endif
                    tdim = 1
                    win.over_idx[win.num_idx] = spider[i]
                    if win.over_idx[win.num_idx] ne -1 then begin
                        hdr = *fi.hdr_ptr[win.over_idx[win.num_idx]]
                        tdim = hdr.tdim
                    endif
                    for j=0,tdim-1 do begin
                        if j gt 0 then begin
                            win.image_idx[win.num_idx] = win.image_idx[win.num_idx-1]
                            win.atlas_space[win.num_idx] = win.atlas_space[win.num_idx-1]
                            win.mask_idx[win.num_idx] = win.mask_idx[win.num_idx-1]
                            win.over_idx[win.num_idx] = spider[i]
                        endif
                        if win.over_idx[win.num_idx] ne -1 then begin
                            win.over_tidx[win.num_idx] = j 
                            win.nreg[win.num_idx] = hdr.ifh.nreg
                            if win.nreg[win.num_idx] gt 0 then win.prf.overlay_zoom_type = !NEAREST_NEIGHBOR
                            win.filenamelabels[win.num_idx] = fi.list[win.over_idx[win.num_idx]]
                            if ptr_valid(win.labels) then begin
                                labels = *win.labels
                                labels[0].s = fi.list[win.over_idx[win.num_idx]]
                                *win.labels = labels
                            endif
                            regnames=['','','']
                        endif
                        win.idxidx = win.num_idx
                        win.num_idx = win.num_idx + 1
                        if win.idxidx gt 0 then $
                            win.sprf[win.idxidx] = win.sprf[win.idxidx-1] $
                        else $
                            win.sprf[win.idxidx] = sprf
                        update_text,fi,help,vw,win
                        widget_control,wd_idxidx,SET_SLIDER_MAX=win.num_idx
                        widget_control,wd_idxidx,SET_VALUE=win.num_idx
                        if win.num_idx gt 1 then widget_control,wd_idxidx,/SENSITIVE
                    endfor
                endfor
            endfor



        endif
        wd_load_bottom:
    end
    wd_tiff: begin
         write_tif,fi,dsp,vw.fnt
    end
    wd_tiff_selected: begin
         write_tif,fi,dsp,vw.fnt,/SELECTED
    end
    wd_tiff_all: begin
        for i=0,win.num_idx-1 do begin
            lc_refreshed = !FALSE
            widget_control,wd_idxidx,SET_VALUE=i+1
            win.idxidx = i
            sprfptr = ptr_new(win.sprf[win.idxidx])
            wd_slice_def_draw_guts,sprfptr
            ptr_free,sprfptr
            current_cells = -1
            write_tif,fi,dsp,vw.fnt,DONTASK=1
        endfor
    end
    wd_print_setup: begin
        prfptr = ptr_new(win.prf)
        get_view_print_pref,prfptr,vw.view_leader
        win.prf = *prfptr
        *vw.win[vw.cw] = win
        prf = win.prf
        ptr_free,prfptr
    end
    wd_print_file: begin
        get_dialog_pickfile,'*.ps',fi.path,'Please select file.',print_file_name,rtn_nfiles,path
        if print_file_name eq '' or print_file_name eq 'GOBACK' or print_file_name eq 'EXIT' then begin
            print_file_name = ''
            return
        endif
        fi.path = path
    end

    wd_print_all: begin
        print_destination = !PRINT_PRINTER
        print_view,fi,dsp,vw.fnt,print_file_name,win,prf,current_cells
    end

    wd_print_selected: begin
        print_destination = !PRINT_PRINTER
        print_view,fi,dsp,vw.fnt,print_file_name,win,prf,current_cells,/SELECTED
    end

    wd_colortables: begin
        if dsp[fi.cw].adjust_color_top eq 0then $
            adjust_colortable $
        else $
            widget_control,dsp[fi.cw].adjust_color_top,/SHOW
    end
    wd_showcolors_vert: begin
        prf.color_bar_type = !VERT_COLOR_BAR
        win.prf.color_bar_type = !VERT_COLOR_BAR
        win.color_bar = !TRUE
        view_images_refresh,fi,dsp,vw.fnt
    end
    wd_showcolors_horiz: begin
        prf.color_bar_type = !HORIZ_COLOR_BAR
        win.prf.color_bar_type = !HORIZ_COLOR_BAR
        win.color_bar = !TRUE
        view_images_refresh,fi,dsp,vw.fnt
    end



    wd_hidecolors: begin
        for itr=0,win.ntrans-1 do begin
            ic = win.translation_table[itr]
            if((win.cell[ic].view eq !VIEW_COLOR_SCALE) and (win.cell[ic].valid eq !TRUE)) then begin
                win.cell[ic].paint = !FALSE
                win.cell[ic].valid = !FALSE
                ptr_free,win.cell[ic].base_image
                if itr lt win.ntrans-1 then $
                    win.translation_table[itr:win.ntrans-2] = win.translation_table[itr+1:win.ntrans-1] $
                else $
                    win.translation_table = win.translation_table[0:win.ntrans-2]
                win.ntrans = win.ntrans - 1
                win.ncell = win.ntrans
            endif
        endfor
        erase
        view_images_refresh,fi,dsp,vw.fnt
        win.color_bar = !FALSE
        widget_control,wd_hidecolors,SENSITIVE=0
        widget_control,wd_showcolors_vert,/SENSITIVE
        widget_control,wd_showcolors_horiz,/SENSITIVE
        prf.color_bar_type = !NO_COLOR_BAR
        win.prf.color_bar_type = !NO_COLOR_BAR
        lc_color_bar_once = !FALSE
    end

    ;wd_slicer: begin
    ;    slicer_z = 0
    ;    slicer_zdim = 0
    ;    for itr=0,win.ntrans-1 do begin
    ;        icell = win.translation_table[itr]
    ;        if((win.cell[icell].valid eq !TRUE) and (win.cell[icell].view ne !VIEW_COLOR_SCALE)) then $
    ;            slicer_zdim = slicer_zdim + 1
    ;    endfor
    ;    slicer_img = 1
    ;    for itr=0,win.ntrans-1 do begin
    ;        icell = win.translation_table[itr]
    ;        if(win.cell[icell].view ne !VIEW_COLOR_SCALE) then begin
    ;            if(win.cell[icell].valid eq !TRUE) then begin
    ;                display_view,win.cell[icell],win.prf,win.ydim_draw,OUTPUT_ARRAY=slicer_img
    ;                slicer_z = slicer_z + 1
    ;            endif
    ;        endif
    ;    endfor
    ;    slicer_img = reverse(slicer_img,3)
    ;    xdim = n_elements(slicer_img[*,0,0])
    ;    ydim = n_elements(slicer_img[0,*,0])
    ;    slicer_img_zoom = fltarr(xdim,ydim,slicer_zdim*prf.zoom)
    ;    for x=0,xdim-1 do begin
    ;        jmg = reform(slicer_img[x,*,*],ydim,slicer_zdim)
    ;        slicer_img_zoom[x,*,*] = rebin(jmg,ydim,prf.zoom*slicer_zdim)
    ;    endfor
    ;    slicer_ptr = ptr_new(slicer_img_zoom)
    ;    slicer_lab = 'Image set name goes here'
    ;    slicer3_jmo,slicer_ptr,DATA_NAMES=slicer_lab,GROUP=vw.view_leader
    ;    load_colortable,fi,dsp,CODING=prf.coding
    ;    wset,vw.wdw[vw.cw]
    ;end

    wd_idxidx: begin
        widget_control,wd_idxidx,GET_VALUE=i
        win.idxidx = i-1
        update_text,fi,help,vw,win
        current_cells = -1
    end
    wd_load_pref: begin
        get_dialog_pickfile,'*.v25d',pref_path,'Please select file.',file,rtn_nfiles,path
        if file eq '' or file eq 'GOBACK' or file eq 'EXIT' then return
        pref_path = path
        preference_file = file
        read_view_pref,prf,sprf,file,pref_path,display_mode,reg,/DEFAULTS
        load_pref,fi,dsp,stc,vw,reg
    end
    wd_load_pref2: begin
        get_dialog_pickfile,'*.v25d',pref_path,'Please select file.',file,rtn_nfiles,path
        if file eq '' or file eq 'GOBACK' or file eq 'EXIT' then return
        pref_path = path
        preference_file = file
        read_view_pref,prf,sprf,file,pref_path,display_mode,reg,/DEFAULTS
        rtn = select_files(strtrim(indgen(win.num_idx)+1,2),TITLE='Apply to image sets',MIN_NUM_TO_SELECT=-1,/EXIT)
        if rtn.files[0] ne 'EXIT' then begin
            for i=0,rtn.count-1 do begin
                win.sprf[rtn.index[i]] = sprf
                win.reg[rtn.index[i]] = reg 
            endfor 
        endif
        load_pref,fi,dsp,stc,vw,reg
    end
    wd_load_slice_pref: begin
        get_dialog_pickfile,'*.v25d',pref_path,'Please select file.',file,rtn_nfiles,path
        if file eq '' or file eq 'GOBACK' or file eq 'EXIT' then return
        pref_path = path
        read_view_pref,prf,sprf,file,pref_path,display_mode
        preference_file = file
        win.sprf[win.idxidx] = sprf
        update_text,fi,help,vw,win
        delete_cells,prf,win,win.idxidx,wd_data_set_status,-1,lc_delete_cells
        widget_control,wd_data_set_status,SET_VALUE='Status: Resliced'
        win.resliced[win.idxidx]= !TRUE
        reslice_view,fi,vw,stc,/SHOW
        reload,fi,dsp,vw
        view_images_refresh,fi,dsp,vw.fnt
        init = fltarr(6)
        init[0] = prf.color_min1
        init[1] = prf.color_max1
        init[2] = prf.color_gamma1
        init[3] = prf.color_min2
        init[4] = prf.color_max2
        init[5] = prf.color_gamma2
        fi.color_scale1[!NUM_FILES] = prf.color_table1
        fi.color_scale2[!NUM_FILES] = prf.color_table2
        view_images_lct,fi.color_scale1[!NUM_FILES],fi.color_scale2[!NUM_FILES],INIT=init
        current_cells = -1
    end
    wd_load_user_pref: begin
        home = getenv('HOME')
        pref_path = home
        file = home + '/.view25d4'
        read_view_pref,prf,sprf,file,pref_path,display_mode,reg,/DEFAULTS
        preference_file = 'User defaults'
        load_pref,fi,dsp,stc,vw,reg
    end
    wd_load_def_pref: begin
        hard_defaults,prf,sprf
        preference_file = 'Hard defaults'
        load_pref,fi,dsp,stc,vw
    end
    wd_load_pref_list: begin
        preference_file = pref_files[ev.value-1]
        read_view_pref,prf,sprf,preference_file,pref_path,display_mode
        win.sprf[win.idxidx] = sprf
        win.prf = prf
        fi.color_scale1[!NUM_FILES] = prf.color_table1
        fi.color_scale2[!NUM_FILES] = prf.color_table2
        update_text,fi,help,vw,win
        delete_cells,prf,win,win.idxidx,wd_data_set_status,-1,lc_delete_cells
        widget_control,wd_data_set_status,SET_VALUE='Status: Resliced'
        win.resliced[win.idxidx]= !TRUE
        reslice_view,fi,vw,stc,/SHOW
        reload,fi,dsp,vw
        view_images_refresh,fi,dsp,vw.fnt
        current_cells = -1
    end
    wd_save_pref: begin
        get_dialog_pickfile,'*.v25d',pref_path,'Please select file.',file,rtn_nfiles,path
        if file eq '' or file eq 'GOBACK' or file eq 'EXIT' then return
        pref_path = path
        set_write_view_pref,file,win,fi,pref_path,display_mode,vw.fnt
    end
    wd_save_def_pref: begin
        home = getenv('HOME')
        file = home + '/.view25d4'
        set_write_view_pref,file,win,fi,pref_path,display_mode,vw.fnt
    end
    wd_save_pref_path: begin
        get_dialog_pickfile,'*.v25d',pref_path,'Please select any file.',file,rtn_nfiles,path
        if file eq '' or file eq 'GOBACK' or file eq 'EXIT' then return
        pref_path = path
    end

    wd_format: begin
        win.prf.xdim_draw = win.xdim_draw
        win.prf.ydim_draw = win.ydim_draw
        prfptr = ptr_new(win.prf)
        if get_view_pref(prfptr,vw.view_leader) eq !FALSE then begin
            win.prf = *prfptr
            prf = win.prf
            *vw.win[vw.cw] = win
            process_prefs,vw,fi,dsp
            update_text,fi,help,vw,win
            delete_cells,prf,win,win.idxidx,wd_data_set_status,-1,lc_delete_cells
            current_cells = -1
            widget_control,wd_data_set_status,SET_VALUE='Status: Resliced'
            win.resliced[win.idxidx]= !TRUE
            widget_control,/HOURGLASS
            reslice_view,fi,vw,stc,/SHOW
            if vw.wdw[vw.cw] gt 0 then wset,vw.wdw[vw.cw]
            reload,fi,dsp,vw
            view_images_refresh,fi,dsp,vw.fnt
        endif
        ptr_free,prfptr
    end
    wd_slice_def_draw: begin
        sprfptr = ptr_new(win.sprf[win.idxidx])
        if get_view_slice_pref(sprfptr[0],vw.view_leader) eq !FALSE then wd_slice_def_draw_guts,sprfptr
        ptr_free,sprfptr
    end

    wd_plot: begin
        sprfptr = ptr_new(win.sprf[win.idxidx])
        wd_slice_def_draw_guts,sprfptr
        ptr_free,sprfptr
    end


    wd_scale_zoom: begin
        old_coding = win.prf.coding
        prfptr = ptr_new(win.prf)
        if get_view_scale_pref(prfptr,vw.view_leader) eq !FALSE then begin
            win.prf = *prfptr
            prf = win.prf
            *vw.win[vw.cw] = win
            if vw.wdw[vw.cw] gt 0 then wset,vw.wdw[vw.cw]
            if prf.coding ne old_coding then begin
                if prf.coding eq 1 then begin
                    if win.ntrans gt 0 then begin
                        virtual_cells = where(win.cell[win.translation_table[0:win.ntrans-1]].freeze eq !FALSE,count)
                        if count ne 0 then begin
                            image_sets = win.cell[win.translation_table[virtual_cells]].image_set
                            delete_cells,prf,win,image_sets,wd_data_set_status,virtual_cells,lc_delete_cells
                        endif
                    endif
                    reslice_view,fi,vw,stc,/SHOW
                endif 
            endif
            reload,fi,dsp,vw
            view_images_refresh,fi,dsp,vw.fnt
        endif
        ptr_free,prfptr
    end

 









    wd_reload: begin
        if vw.wdw[vw.cw] gt 0 then wset,vw.wdw[vw.cw]
        reload,fi,dsp,vw
        view_images_refresh,fi,dsp,vw.fnt
        current_cells = -1
    end
 
    wd_refresh: begin
        if vw.wdw[vw.cw] gt 0 then wset,vw.wdw[vw.cw]
        widget_control,wd_idxidx,SET_VALUE=win.idxidxcur+1
        win.idxidx = win.idxidxcur 
        update_text,fi,help,vw,win
        view_images_refresh,fi,dsp,vw.fnt
    end

    wd_new_window: begin
        create_new_view_window,vw,fi
        win = *vw.win[vw.cw]
        sprf = win.sprf[0]
        prf = win.prf
    end
 
    wd_deselect: begin
        for ic=0,n_elements(current_cells)-1 do outline_cell,win,current_cells[ic],prf.zoom,win.ydim_draw,display_mode,0
        current_cells = -1
    end

    wd_erase_selected: begin
        ntrans = win.ntrans
        translation_table = win.translation_table
        win.ntrans = 0
        for itr=0,ntrans-1 do begin
            icell = win.translation_table[itr]
            keep = !TRUE
            for ic=0,n_elements(current_cells)-1 do begin
                if(icell eq current_cells[ic]) then $
                    keep = !FALSE
            endfor
            if(keep eq !TRUE) then begin
                translation_table[win.ntrans] = icell
                win.ntrans = win.ntrans + 1
            endif
        endfor
        win.translation_table = translation_table
        erase
        view_images_refresh,fi,dsp,vw.fnt
        current_cells = -1
    end

    wd_erase_set: begin
        ntrans = win.ntrans
        translation_table = win.translation_table
        win.ntrans = 0
        for itr=0,ntrans-1 do begin
            ic = win.translation_table[itr]
            if(win.cell[ic].image_set ne win.idxidx) then begin
                translation_table[win.ntrans] = ic
                win.ntrans = win.ntrans + 1
            endif
        endfor
        win.translation_table = translation_table
        erase
        view_images_refresh,fi,dsp,vw.fnt
        current_cells = -1
    end

    wd_insert_set: begin
        itr = win.insertion_point
        if win.ntrans gt 0 then begin
            tmp = win.translation_table[itr:win.ntrans-1]
            ntmp = win.ntrans - itr
        endif else $
            ntmp = 0
        for ic=0,win.ncell-1 do begin
            if(win.cell[ic].image_set eq win.idxidx) and (win.cell[ic].valid eq !TRUE) then begin
                win.cell[ic].paint = !TRUE
                win.translation_table[itr] = ic
                itr = itr + 1
                win.ntrans = win.ntrans + 1
            endif
        endfor
        if ntmp gt 0 then win.translation_table[itr:itr+ntmp-1] = tmp
        erase
        reload,fi,dsp,vw
        view_images_refresh,fi,dsp,vw.fnt
        current_cells = -1
    end

    wd_append_set: begin
        for ic=0,win.ncell-1 do begin
            if((win.cell[ic].image_set eq win.idxidx) and (win.cell[ic].valid eq !TRUE)) then begin
                win.cell[ic].paint = !TRUE
                win.translation_table[win.ntrans] = ic
                win.ntrans = win.ntrans + 1
            endif
        endfor
        erase
        reload,fi,dsp,vw
        view_images_refresh,fi,dsp,vw.fnt
        current_cells = -1
    end

    wd_clear: begin
        erase
    end

    wd_delete_all: begin
        delete_cells,prf,win,win.idxidx,wd_data_set_status,-1,lc_delete_cells,/ALL
        view_images_refresh,fi,dsp,vw.fnt
    end

    wd_delete_image_set: begin
        delete_cells,prf,win,win.idxidx,wd_data_set_status,-1,lc_delete_cells
        win.resliced[win.idxidx] = !FALSE
        reload,fi,dsp,vw
        view_images_refresh,fi,dsp,vw.fnt
        current_cells = -1
    end
    wd_delete_cell: begin
        ndel = n_elements(current_cells)
        virtual_cells = lonarr(ndel)
        image_sets = lonarr(ndel)
        for i=0,ndel-1 do begin
            virtual_cells[i] = current_cells[i]
            ic = win.translation_table[current_cells[i]]
            image_sets[i] = win.cell[ic].image_set
            win.cell[ic].freeze = !FALSE
        endfor
        delete_cells,prf,win,image_sets,wd_data_set_status,virtual_cells,lc_delete_cells
        erase
        view_images_refresh,fi,dsp,vw.fnt,/NO_OUTLINES
        current_cells = -1
    end
    wd_freeze_cell: begin
        if current_cells[0] ne -1 then begin
            win.cell[win.translation_table[current_cells]].freeze = !TRUE
            current_cells = -1
        endif
    end






;;;    wd_cut: begin
;;;        vw.nclip = n_elements(current_cells)
;;;        for i=0,vw.nclip-1 do begin
;;;            vw.clipboard[i] = win.cell[win.translation_table[current_cells[i]]]
;;;        endfor
;;;        lc_cut = !TRUE
;;;        cut_cells = current_cells
;;;    end

    wd_copy: begin
        vw.nclip = n_elements(current_cells)
        for i=0,vw.nclip-1 do begin
            vw.clipboard[i] = win.cell[win.translation_table[current_cells[i]]]
        endfor
        lc_cut = !FALSE
    end

    wd_paste: begin
        itr = win.insertion_point
        if(win.ntrans gt 0) then begin
            tmp = win.translation_table[itr:win.ntrans-1]
            ntmp = win.ntrans - itr
        endif else $
            ntmp = 0
        image_sets = lonarr(vw.nclip)
        for ic=0,vw.nclip-1 do begin
            if(vw.clipboard[ic].valid eq !TRUE) then begin
                image_sets[ic] = vw.clipboard[ic].image_set
                win.cell[win.ncell] = vw.clipboard[ic]
                win.cell[win.ncell].image_set = win.idxidx
                if(ptr_valid(vw.clipboard[ic].base_image)) then begin
                    base_img = *vw.clipboard[ic].base_image
                    win.cell[win.ncell].base_image = ptr_new(base_img)
                endif else begin
                    win.cell[win.ncell].base_image = ptr_new()
                endelse
                if(ptr_valid(vw.clipboard[ic].overlay_image)) then begin
                    overlay_image = *vw.clipboard[ic].overlay_image
                    win.cell[win.ncell].overlay_image = ptr_new(overlay_image)
                endif else begin
                    win.cell[win.ncell].overlay_image = ptr_new()
                endelse
                if(ptr_valid(vw.clipboard[ic].labels)) then begin
                    labels = *vw.clipboard[ic].labels
                    win.cell[win.ncell].labels = ptr_new(labels)
                endif else begin
                    win.cell[win.ncell].labels = ptr_new()
                endelse
                win.translation_table[itr] = win.ncell
                itr = itr + 1
                win.ntrans = win.ntrans + 1
                win.ncell = win.ncell + 1
            endif
        endfor
        if(lc_cut eq !TRUE) then begin
            delete_cells,prf,win,image_sets,wd_data_set_status,cut_cells,lc_delete_cells
            lc_cut = !FALSE
        endif
        if(ntmp gt 0) then $
            win.translation_table[itr:itr+ntmp-1] = tmp
        view_images_refresh,fi,dsp,vw.fnt
    end
    wd_add_label: begin
        if n_elements(current_cells) ne 1 then begin
            st = dialog_message('Select an image by double-clicking on it, then try again.',/ERROR)
            return
        endif
        string = get_str(1,'Enter label then click on an image','',GROUP_LEADER=vw.view_leader,WIDTH=50)
        get_label,vw,win,string,current_label
        lc_add_label = !TRUE
    end
    wd_add_label_file: begin

        ;for i=0,!VIEW_MAX_LABELS-1 do begin   
        ;START130220
        for i=0,!NUM_FILES-1 do begin   

            checked = *win.label[i].checked
            checked[0] = 1
            *win.label[i].checked = checked
        end
        get_label,vw,win,'',current_label
        lc_add_label = !TRUE
        lc_add_label_file = !TRUE
    end
    wd_edit_label: begin
        plot_labels
    end
    wd_add_mark: begin
        lc_add_mark = not lc_add_mark
        if(lc_add_mark) then  $
            widget_control,wd_add_mark,SET_VALUE="Don't mark points" $
        else $
            widget_control,wd_add_mark,SET_VALUE="Mark designated points"
    end
    wd_label_font: begin
        if(ptr_valid(win.labels)) then $
            labels= *win.labels $
        else $
            return
        labs = strarr(win.nlab)
        vals = strarr(win.nlab)
        for i=0,win.nlab-1 do begin
            labs[i] = string(i+1,FORMAT='("Label #",i2)')
            vals[i] = labels[i].s
        endfor
        idx_list = get_bool_list(vals,TITLE='Select labels to update')
        idx_list = idx_list.list
        for i=0,win.nlab-1 do begin
            if(idx_list[i] eq 1) then $
                labels[i].font = vw.fnt.current_font
        endfor
        *win.labels = labels
        current_label = -1
        view_images_refresh,fi,dsp,vw.fnt
    end

    wd_label_color: begin
        lab = strarr(15)
        lab[0] = 'white'
        lab[1] = 'yellow5'
        lab[2] = 'red'
        lab[3] = 'green'
        lab[4] = 'blue'
        lab[5] = 'orange'
        lab[6] = 'magenta'
        lab[7] = 'cyan'
        lab[8] = 'brown'
        lab[9] = 'black'
        lab[10] = 'blue3'
        lab[11] = 'orange8'
        lab[12] = 'purple1'
        lab[13] = 'yellow10'
        lab[14] = 'red1'
        scrap = win.prf.label_color
        win.prf.label_color = get_button(lab,TITLE='Please select label color.',DEFAULT=win.prf.label_color,/OK)
        if win.prf.label_color ne scrap then view_images_refresh,fi,dsp,vw.fnt
    end

    wd_af3d_loci: begin
        atlas_space = win.atlas_space[win.idxidx]
        flag = !OK
        if atlas_space ne !SPACE_111 and atlas_space ne !SPACE_222 and atlas_space ne !SPACE_333 then begin
            atlas_space = get_button(['111','222','333','Cancel'],TITLE='Which atlas will your images be in?')
            case atlas_space of
                0: atlas_space = !SPACE_111
                1: atlas_space = !SPACE_222
                2: atlas_space = !SPACE_333
                else: flag = !ERROR
            endcase
        endif
        if flag eq !OK then af3d_loci,atlas_space
    end

    wd_reg: begin
        if regnames[2] eq '' then begin
            if win.nreg[win.idxidx] eq 0 then begin
                doit=0
                applytoall=0
                title='No regions detected.'+string(10B)+string(10B)+'Would you like FIDL to pretend this is a region file?' $
                    +string(10B)+string(10B)+'FIDL is only good at pretending if the blobs are integer valued.'
                if win.num_idx eq 1 then begin   
                    idx = get_button(['Yes','Cancel'],TITLE=title,GROUP_LEADER=vw.view_leader) 
                    if idx eq 0 then doit=1
                endif else begin
                    idx = get_button(['Yes, just this image set.','Yes, apply to all image sets.','Cancel'],TITLE=title, $
                        GROUP_LEADER=vw.view_leader)
                    if idx lt 2 then doit=1
                    if idx eq 1 then applytoall=1 
                endelse
                if doit eq 1 then begin
                    region_names = [0,0]
                    widget_control,/HOURGLASS
                    for i=win.over_min[win.idxidx],win.over_max[win.idxidx] do begin
                        index = where(over_image eq i,count)
                        if count ne 0 then begin
                            win.nreg[win.idxidx] = win.nreg[win.idxidx] + 1
                            region_names = [region_names,i]
                        endif
                    endfor
                    voxval = region_names[2:*]
                    regnames = strtrim(region_names,2) 
                    regnamesi = indgen(win.nreg[win.idxidx])+2
                    if applytoall eq 1 then win.nreg[*] = win.nreg[win.idxidx] 
                endif
            endif
        endif

        ;if regnames[2] ne '' then plot_regions,regnames[regnamesi]
        ;START130219
        if regnames[2] ne '' then plot_regions,regnames[regnamesi],voxval

    end
    wd_reg_ass: begin
        if win.nreg[win.idxidx] eq 0 then $
            stat = dialog_message("No regions detected.",/ERROR) $
        else begin
            hdr = *fi.hdr_ptr[win.over_idx[win.idxidx]]
            ifh = hdr.ifh
            plot_reg_ass,*ifh.region_names
        endelse
    end
    wd_delete_label: begin
        if ptr_valid(win.labels) then labels= *win.labels else return
        labs = labels[0:win.nlab-1].s 
        labs = [labs,'Cancel']
        idx = get_button(labs,TITLE='Label to be deleted.',GROUP_LEADER=vw.view_leader)
        if idx ne win.nlab then begin
            for i=idx,win.nlab-1 do begin
                labels[i].x = labels[i+1].x
                labels[i].y = labels[i+1].y
                labels[i].wdth = labels[i+1].wdth
                labels[i].hgth = labels[i+1].hgth
                labels[i].s = labels[i+1].s
                labels[i].font = labels[i+1].font
            endfor
            win.nlab = win.nlab - 1
            *win.labels = labels
            current_label = -1
            if vw.wdw[vw.cw] gt 0 then wset,vw.wdw[vw.cw]
            view_images_refresh,fi,dsp,vw.fnt
        endif
    end

    wd_exit: begin
        if dsp[fi.cw].image_index[fi.cw] gt 0 then wset,dsp[fi.cw].image_index[fi.cw]
        widget_control,vw.view_leader,/DESTROY
        for i=vw.num_windows-1,0,-1 do begin
            if widget_info(vw.id[i],/VALID) then widget_control,vw.base[i],/DESTROY
        endfor
        vw.num_windows = 0
    end

    wd_outlines: begin
        load_outlines
    end

    wd_no_outlines: begin
        show_outlines = !FALSE 
    end
    wd_points: begin
        prfptr = ptr_new(win.prf)
        get_points_pref,prfptr,vw.view_leader
        win.prf = *prfptr
        *vw.win[vw.cw] = win
        prf = win.prf
        ptr_free,prfptr
    end
    wd_overlay: begin
        display_mode = !OVERLAY_MODE
        widget_control,wd_top_bot,/SENSITIVE
        widget_control,wd_overlay,SENSITIVE=0
        reload,fi,dsp,vw
        view_images_refresh,fi,dsp,vw.fnt
    end
    wd_top_bot: begin
        display_mode = !ANAT_OVER_ACT_MODE
        widget_control,wd_top_bot,SENSITIVE=0
        widget_control,wd_overlay,/SENSITIVE
        reload,fi,dsp,vw
        view_images_refresh,fi,dsp,vw.fnt
    end

    wd_font_name: begin
        font_size_m1 = font_sizes[vw.fnt.current_font_number]
        vw.fnt.family = font_families[ev.index]
        if(vw.fnt.family eq 'symbol') then begin
            widget_control,wd_font_weight,SENSITIVE=0
            widget_control,wd_font_slant,SENSITIVE=0
        endif else begin
            widget_control,wd_font_weight,/SENSITIVE
            widget_control,wd_font_slant,/SENSITIVE
        endelse
        get_font_list,vw.fnt,font_sizes,font_names,nfonts
        font_sizes = string(long(font_sizes)/10)
        vw.fnt.current_font_number = 1
        for i=0,nfonts-1 do begin
            if(font_sizes[i] eq font_size_m1) then $
                vw.fnt.current_font_number = i
        endfor
        widget_control,wd_font_size,SET_VALUE=font_sizes,SET_DROPLIST_SELECT=vw.fnt.current_font_number
        vw.fnt.current_font = font_names[vw.fnt.current_font_number]
        !P.FONT = 0
        device,SET_FONT=vw.fnt.current_font
    end

    wd_font_size: begin
        vw.fnt.current_font_number = ev.index
        !P.FONT = 0
        vw.fnt.current_font = font_names[vw.fnt.current_font_number]
        device,SET_FONT=vw.fnt.current_font
    end

    wd_font_weight: begin
        if(ev.select eq 1) then $
            vw.fnt.bold = !TRUE $
        else $
            vw.fnt.bold = !FALSE
        font_size_m1 = font_sizes[vw.fnt.current_font_number]
        get_font_list,vw.fnt,font_sizes,font_names,nfonts
        font_sizes = string(long(font_sizes)/10)
        vw.fnt.current_font_number = 1
        for i=0,nfonts-1 do begin
            if(font_sizes[i] eq font_size_m1) then $
                vw.fnt.current_font_number = i
        endfor
        widget_control,wd_font_size,SET_VALUE=font_sizes,SET_DROPLIST_SELECT=vw.fnt.current_font_number
        vw.fnt.current_font = font_names[vw.fnt.current_font_number]
        !P.FONT = 0
        device,SET_FONT=vw.fnt.current_font
    end

    wd_font_slant: begin
        if(ev.select eq 1) then $
            vw.fnt.italic = !TRUE $
        else $
            vw.fnt.italic = !FALSE
        font_size_m1 = font_sizes[vw.fnt.current_font_number]
        get_font_list,vw.fnt,font_sizes,font_names,nfonts
        font_sizes = string(long(font_sizes)/10)
        vw.fnt.current_font_number = 1
        for i=0,nfonts-1 do begin
            if(font_sizes[i] eq font_size_m1) then $
                vw.fnt.current_font_number = i
        endfor
        vw.fnt.current_font = font_names[vw.fnt.current_font_number]
        widget_control,wd_font_size,SET_VALUE=font_sizes,SET_DROPLIST_SELECT=vw.fnt.current_font_number
        !P.FONT = 0
        device,SET_FONT=vw.fnt.current_font
;;;print,font_names[vw.fnt.current_font_number]
    end

    ;wd_elf: begin
    ;    elf = abs(elf-1)
    ;    if vw.wdw[vw.cw] gt 0 then wset,vw.wdw[vw.cw]
    ;    widget_control,wd_idxidx,SET_VALUE=win.idxidxcur+1
    ;    win.idxidx = win.idxidxcur
    ;    update_text,fi,help,vw,win
    ;    view_images_refresh,fi,dsp,vw.fnt
    ;end

    else: begin
        cw = -1
        for i=0,vw.num_windows-1 do begin
            if(ev.id eq vw.id[i]) then $
                cw = i
        endfor
        if(cw lt 0) then $
            print,'Invalid event in view_images_event.'
    end
endcase
if(ptr_valid(vw.win[vw.cw])) then *vw.win[vw.cw] = win
end


pro view_images_lct,table1,table2,INIT=init
    common stats_comm
    common view_images_comm_shared
    if not keyword_set(INIT) then init = 0
    if win.prf.coding eq 1 then $
        color_index = indgen(100)+1 $
    else $
        color_index = -1
    load_colortable,fi,dsp,color_index,TABLE1=table1,TABLE2=table2,INIT=init
end
function set_labels,labels,label_color
    common view_images_comm_shared
    labels[*].color = af3dh.color[label_color]
    labels[*].color_index = label_color 
    labels[*].pixmap_file = !PIXMAP_PATH+'/'+'square_'+af3dh.color[label_color]+'_filled.m.pm'
    return,labels
end
pro init_labels,vw
    common view_images_comm_shared
    current_label = 0
    win.nlab = 1
    labels = replicate({View_label},!NUM_FILES)
    labels[*].s = '                                                                               '
    labels[*].font = vw.fnt.current_font
    checked = intarr(!NUM_FILES)
    for i=0,!NUM_FILES-1 do win.label[i].checked = ptr_new(checked)
    win.labels = ptr_new(set_labels(labels,win.prf.label_color))
end
pro set_write_view_pref,file,win,fi,pref_path,display_mode,fnt
    win.prf.color_table1 = fi.color_scale1[!NUM_FILES]
    win.prf.color_table2 = fi.color_scale2[!NUM_FILES]
    win.prf.xdim_draw = win.xdim_draw
    win.prf.ydim_draw = win.ydim_draw
    write_view_pref,file,win,fi,pref_path,display_mode,fnt
end
pro load_pref,fi,dsp,stc,vw,reg
    common view_images_comm_shared
    common view_images_comm
    process_prefs,vw,fi,dsp
    win.sprf[win.idxidx] = sprf


    ;if n_elements(reg) ne 0 then begin
    ;    
    ;    print,'here200 n_elements(reg.checked)=',n_elements(reg.checked)
    ;    print,'here200 reg.checked=',reg.checked
    ;    print,'here200 reg=',reg
    ;
    ;    win.nreg[win.idxidx] = n_elements(reg.checked)
    ;    win.reg[win.idxidx] = reg
    ;endif
    ;START131121
    if n_elements(reg) ne 0 then begin
        if ptr_valid(reg.checked) then begin
            win.nreg[win.idxidx] = n_elements(reg.checked)
            win.reg[win.idxidx] = reg
        endif
    endif


    win.prf = prf
    init = fltarr(6)
    init[0] = prf.color_min1
    init[1] = prf.color_max1
    init[2] = prf.color_gamma1
    init[3] = prf.color_min2
    init[4] = prf.color_max2
    init[5] = prf.color_gamma2
    fi.color_scale1[!NUM_FILES] = prf.color_table1
    fi.color_scale2[!NUM_FILES] = prf.color_table2
    *win.labels = set_labels(*win.labels,win.prf.label_color)
    view_images_lct,fi.color_scale1[!NUM_FILES],fi.color_scale2[!NUM_FILES],INIT=init
    create_new_window,vw
    delete_cells,prf,win,win.idxidx,wd_data_set_status,-1,lc_delete_cells
    if win.num_idx ge 1 then begin
        sprfptr = ptr_new(win.sprf[win.idxidx])
        wd_slice_def_draw_guts,sprfptr
        ptr_free,sprfptr
        current_cells = -1
    endif
end
pro get_label,vw,win,string,current_label
    labels = *win.labels
    if string[0] eq '' then begin 
        current_label = 0 
    endif else begin
        current_label = win.nlab
        win.nlab = win.nlab + 1
        labels[current_label].s = string[0]
        ;print,'get_label labels=',labels
    endelse
    if !D.WINDOW gt -1 then device,cursor_standard=2 ;Needed for Windows->Hummingbird
    *win.labels = labels
end
pro create_new_window,vw
    common view_images_comm_shared
    common view_images_comm
    if widget_info(vw.base[vw.cw],/VALID_ID) then widget_control,vw.base[vw.cw],/DESTROY
    vw.used_name[vw.cw] = !FALSE
    for i=0,!VIEW_WINDOWS_MAX-1 do begin
        if vw.used_name[i] eq !FALSE then begin
            win.name = wdw_names[i]
            vw.used_name[i] = !TRUE
            i = !VIEW_WINDOWS_MAX
        endif
    endfor
    win.ymax = win.ydim_draw
    vw.base[vw.cw] = widget_base(title=win.name,/COLUMN,/TLB_SIZE_EVENTS, $
                X_SCROLL_SIZE=win.xdim_draw,Y_SCROLL_SIZE=win.ydim_draw,GROUP_LEADER=vw.view_leader)
    vw.id[vw.cw] = widget_draw(vw.base[vw.cw],/BUTTON_EVENTS,/MOTION_EVENTS,RETAIN=2, $
                XSIZE=win.xdim_draw,YSIZE=win.ydim_draw, $
                KILL_NOTIFY=string('view_images_dead'),/ALIGN_RIGHT)
    widget_control,vw.base[vw.cw],/REALIZE
    xmanager,'view_images',vw.base[vw.cw]
    widget_control,vw.view_leader,/SHOW
    i = 0L
    widget_control,GET_VALUE=i,vw.id[vw.cw]
    vw.wdw[vw.cw] = i
    if vw.wdw[vw.cw] gt 0 then wset,vw.wdw[vw.cw]

    ;START130329
    ;widget_control,wd_idxidx,SET_SLIDER_MAX=win.num_idx
    ;widget_control,wd_idxidx,SET_VALUE=win.num_idx
    ;widget_control,wd_idxidx,SENSITIVE=win.num_idx
    widget_control,wd_idxidx,SET_SLIDER_MAX=(win.num_idx>1)
    widget_control,wd_idxidx,SET_VALUE=(win.num_idx>1)

    ;widget_control,wd_idxidx,SENSITIVE=win.num_idx
    ;START130509
    if win.num_idx gt 1 then widget_control,wd_idxidx,/SENSITIVE

end
pro process_prefs,vw,fi,dsp
    common view_images_comm_shared
    common view_images_comm
    vw.fnt.family = prf.font_family
    vw.fnt.size = prf.font_size
    vw.fnt.bold = prf.font_bold
    vw.fnt.italic = prf.font_italic
    win.xdim_draw = prf.xdim_draw
    win.ydim_draw = prf.ydim_draw
    win.black_background = prf.black_background
    win.black_background_mask = prf.black_background_mask
    for itr=0,win.ntrans-1 do begin
        icell = win.translation_table[itr]
        if(win.cell[icell].valid eq !TRUE) then begin
            if(win.cell[icell].view eq !VIEW_COLOR_SCALE) then begin
                win.cell[icell].x = prf.color_x
                win.cell[icell].y = prf.color_y
            endif
        endif
    endfor
    ; First get rid of old bar if there is one
    cc = win.ncell
    if win.color_bar eq !TRUE then begin
        win.color_bar = !FALSE
        for itr=0,win.ntrans-1 do begin
            ic = win.translation_table[itr]
            if((win.cell[ic].view eq !VIEW_COLOR_SCALE) and (win.cell[ic].valid eq !TRUE)) then begin
                cc = ic
                win.cell[ic].paint = !FALSE
                win.cell[ic].valid = !FALSE
                win.cell[ic].image_set = -1
                if(ptr_valid(win.cell[ic].base_image)) then ptr_free,win.cell[ic].base_image
                if(ptr_valid(win.cell[ic].labels)) then ptr_free,win.cell[ic].labels
                if(itr lt win.ntrans-1) then begin
                    win.translation_table[itr:win.ntrans-2] = win.translation_table[itr+1:win.ntrans-1]
                    win.ntrans = win.ntrans - 1
                endif
            endif
        endfor
    endif 
    case prf.color_bar_type of
        !HORIZ_COLOR_BAR: begin
            create_view_color_scale,win,prf,vw.fnt,cc,/HORIZONTAL
            widget_control,wd_hidecolors,/SENSITIVE
            widget_control,wd_showcolors_vert,SENSITIVE=0
            widget_control,wd_showcolors_horiz,SENSITIVE=0
            win.color_bar = !TRUE
        end
        !VERT_COLOR_BAR: begin
            create_view_color_scale,win,prf,vw.fnt,cc
            widget_control,wd_hidecolors,/SENSITIVE
            widget_control,wd_showcolors_vert,SENSITIVE=0
            widget_control,wd_showcolors_horiz,SENSITIVE=0
            win.color_bar = !TRUE
        end
        !NO_COLOR_BAR: begin
            widget_control,wd_hidecolors,SENSITIVE=0
            widget_control,wd_showcolors_vert,/SENSITIVE
            widget_control,wd_showcolors_horiz,/SENSITIVE
            win.color_bar = !FALSE
        end
        else: print,'Invalid value of color_bar_type.'
    endcase
    if prf.lcf eq 1 then begin
        for i=0,!NUM_FILES-1 do begin
            checked = *win.label[i].checked
            checked[0] = 1
            *win.label[i].checked = checked
        end
        get_label,vw,win,'',current_label
        labels = *win.labels
        labels[current_label].x = prf.fx
        labels[current_label].y = prf.fy 
        *win.labels = labels
    endif
end

;**********************************************************************************************************************
pro print_view,fi,dsp,font,filename,win,prf,current_cells,PRINTER=printer,PRINT_TO_FILE=print_to_file,SELECTED=selected
;**********************************************************************************************************************
if keyword_set(SELECTED) then $
    draw_to_array,tv_img,fi,dsp,font,nc,/SELECTED_IMGS $
else $
    draw_to_array,tv_img,fi,dsp,font,nc
if nc eq 0 then begin
    print,'No images to display'
    return
endif


;if !MAC_OSX eq 'TRUE' then begin
;    xdim = n_elements(tv_img[0,*,0])
;    ydim = n_elements(tv_img[0,0,*])
;    tvlct,indgen(256),indgen(256),indgen(256)
;endif else begin
;    xdim = n_elements(tv_img[*,0])
;    ydim = n_elements(tv_img[0,*])
;endelse
;START110412
if (!FIDLCOLORDEPTH eq 24) or (!FIDLCOLORDEPTH eq 16) then begin
    xdim = n_elements(tv_img[0,*,0])
    ydim = n_elements(tv_img[0,0,*])
    tvlct,indgen(256),indgen(256),indgen(256)
endif else begin
    xdim = n_elements(tv_img[*,0])
    ydim = n_elements(tv_img[0,*])
endelse


set_plot,'PS'
sclx = float(prf.print_xdim)/float(xdim)
scly = float(prf.print_ydim)/float(ydim)
if(sclx lt scly) then begin
    xsize = prf.print_xdim
    ysize = float(ydim)*float(xsize)/float(xdim)
    font_scale = 2540*float(xsize)/float(xdim)
endif else begin
    ysize = prf.print_ydim
    xsize = float(xdim)*float(ysize)/float(ydim)
    font_scale = 2540*float(ysize)/float(ydim)
endelse
if(prf.print_orientation eq !PRINT_PORTRAIT) then begin
   paper_xsize = !PRINT_X_PAPER_SIZE
   paper_ysize = !PRINT_Y_PAPER_SIZE
endif else begin
   paper_xsize = !PRINT_Y_PAPER_SIZE
   paper_ysize = !PRINT_X_PAPER_SIZE
endelse
dx = paper_xsize - 2*prf.print_x0 - xsize
if(dx gt 0) then $
    xx0 = prf.print_x0 + dx/2 $
else $
    xx0 = prf.print_x0
dy = paper_ysize - 2*prf.print_y0 - ysize
if(dy gt 0) then $
    yy0 = prf.print_y0 + dy/2 $
else $
    yy0 = prf.print_y0
if(prf.print_orientation eq !PRINT_PORTRAIT) then begin
    device,/PORTRAIT
    x0 = xx0
    y0 = yy0
endif else begin
    device,/LANDSCAPE
    x0 = yy0
    y0 = !PRINT_Y_PAPER_SIZE - xx0
endelse
device,/INCHES,XOFFSET=x0,YOFFSET=y0,FILENAME=filename,XSIZE=xsize,YSIZE=ysize
if prf.print_color eq 0 then $ 
    device,COLOR=0,BITS_PER_PIXEL=4 $
else $ 
    device,/COLOR,BITS_PER_PIXEL=8

;if !MAC_OSX eq 'TRUE' then tv,tv_img,/ORDER,TRUE=1 else tv,tv_img,/ORDER
;START110412
if (!FIDLCOLORDEPTH eq 24) or (!FIDLCOLORDEPTH eq 16) then tv,tv_img,/ORDER,TRUE=1 else tv,tv_img,/ORDER

device,/CLOSE_FILE

;if !MAC_OSX eq 'TRUE' then tvlct,dsp.lct[*,0],dsp.lct[*,1],dsp.lct[*,2]
;START110412
if (!FIDLCOLORDEPTH eq 24) or (!FIDLCOLORDEPTH eq 16) then tvlct,dsp.lct[*,0],dsp.lct[*,1],dsp.lct[*,2]

set_plot,'X'
if not keyword_set(PRINT_TO_FILE) then begin
    cmd = get_printer_cmd(prf.printer_name,filename) 
    spawn,cmd
    print,cmd
endif
end

;*******************************************************************
pro find_cells_in_box,win,cell_list,x0,y0,x1,y1,zoom_in,display_mode
;*******************************************************************
cell_list = lonarr(win.ntrans)
il = 0
if(x0 lt x1) then begin
    xx0 = x0
    xx1 = x1
endif else begin
    xx0 = x1
    xx1 = x0
endelse
if(y0 lt y1) then begin
    yy0 = y0
    yy1 = y1
endif else begin
    yy0 = y1
    yy1 = y0
endelse
for itr=0,win.ntrans-1 do begin
    icell = win.translation_table[itr]
    if(win.cell[icell].paint eq !TRUE) then begin
        if(win.cell[icell].view eq !VIEW_COLOR_SCALE) then $
            zoom = 1 $
        else  $
            zoom = zoom_in
        xctr = win.cell[icell].x + zoom_in*win.cell[icell].xdim/2
        yctr = win.cell[icell].y + zoom_in*win.cell[icell].ydim/2
        if((xctr ge xx0) and (xctr le xx1)) then begin
            if((yctr ge yy0) and (yctr le yy1)) then begin
                cell_list[il] = itr
                il = il + 1
            endif
        endif
    endif
endfor

if(il gt 0) then $
    cell_list = cell_list[0:il-1] $
else $
    cell_list = -1
return
end

;***************************************************************************************
pro delete_cells,prf,win,imgset,wd_data_set_status,virtual_cells,lc_delete_cells,ALL=all
;***************************************************************************************
if(win.ntrans le 0) then $
    return
translation_table = win.translation_table
if((virtual_cells[0] lt 0) or keyword_set(ALL)) then begin
    ncells = win.ntrans
    cell_list = lindgen(win.ntrans)
    image_sets = lonarr(win.ntrans)
    if(n_elements(imgset) eq 1) then $
        image_sets[*] = imgset $
    else $
        image_sets = imgset
    if(keyword_set(ALL)) then $
        image_sets[*] = -1
endif else begin
    cell_list = virtual_cells
    ncells = n_elements(cell_list)
    image_sets = imgset
endelse
color_ic = -1
for itr=0,ncells-1 do begin
    ic = win.translation_table[cell_list[itr]]
    if keyword_set(ALL) then image_set = win.cell[ic].image_set
    if win.cell[ic].image_set eq 1000 then color_ic = ic
    if lc_delete_cells eq !TRUE then begin 
        if(win.cell[ic].valid eq !TRUE) then begin
            if((win.cell[ic].image_set eq image_sets[itr]) or (image_sets[itr] lt 0)) then begin
                if win.cell[ic].view ne !VIEW_COLOR_SCALE then begin
                    if ptr_valid(win.cell[ic].base_image) then ptr_free,win.cell[ic].base_image
                    if ptr_valid(win.cell[ic].overlay_image) then ptr_free,win.cell[ic].overlay_image
                    win.cell[ic].valid = !FALSE
                    win.cell[ic].paint = !FALSE
                    win.cell[ic].image_set = -1
                    if ptr_valid(win.cell[ic].labels) then ptr_free,win.cell[ic].labels
                endif
            endif
        endif
    endif
endfor
lc_delete_cells = !TRUE

;Compress cell list.
ntrans = 0
jtr = 0
jc = 0
for itr=0,win.ntrans-1 do begin
    ic = win.translation_table[itr]
    if win.cell[ic].valid eq !TRUE then begin
        if win.cell[ic].view ne !VIEW_COLOR_SCALE then begin
            if(jc lt ic) then begin
                win.cell[jc] = win.cell[ic]
                win.cell[ic].valid = !FALSE
                win.cell[ic].paint = !FALSE
                win.cell[ic].image_set = -1
            endif
            translation_table[jtr] = jc
            jc = jc + 1
            jtr = jtr + 1
            ntrans = ntrans + 1
        endif
    endif
endfor
win.ntrans = ntrans
win.ncell = win.ntrans
win.translation_table = translation_table


widget_control,wd_data_set_status,SET_VALUE='Status: Unsliced'
if(keyword_set(ALL)) then begin
    win.resliced[*] = !FALSE
    if(color_ic gt 0) then begin
        win.ncell = 1
        win.ntrans = 1
        win.cell[0] = win.cell[color_ic]
        win.translation_table[0] = 0
    endif
endif
if(virtual_cells[0] lt 0) then begin
    win.resliced[win.idxidx] = !FALSE
    widget_control,wd_data_set_status,SET_VALUE='Status: Unsliced'
endif

current_cells = -1

return
end


;****************
pro load_outlines
;****************
common view_images_comm_shared
common view_images_comm

;print,'load_outlines top'

file = '/data/fafner/mri/Tailarach/transverse_d_025_fidl.atl'
get_lun,lu
openr,lu,file
img_file = ''
xdim = 0L
ydim = 0L
centerx = 0L
centery = 0L
readf,lu,xdim,FORMAT='(i3)'
readf,lu,ydim,FORMAT='(i3)'
readf,lu,pixsiz,FORMAT='(f5.2)'
readf,lu,centerx,FORMAT='(i3)'
readf,lu,centery,FORMAT='(i3)'
;;;centery = ydim - centery - 1
readf,lu,img_file,FORMAT='(a)'
trans_outline_table = intarr(75,2)
dummy = 0
img_index = 0
atlaz_z = 0
i2 = 0
for i=0,74 do begin
    readf,lu,dummy,img_index,atlas_z,FORMAT='(3(i2,1x))'
    trans_outline_table[i,0] = img_index
    trans_outline_table[i,1] = atlas_z
endfor
close,lu
zdim = max(trans_outline_table[*,0])

imgin = intarr(xdim,ydim,zdim)
openr,lu,img_file
readu,lu,imgin
close,lu
free_lun,lu

imgin = imgin gt 0


;trans_outlines = intarr(win.prf.dx,win.prf.dy,zdim)
;xpad = win.prf.dx/2 - centerx
;ypad = win.prf.dy/2 - centery
;if(xpad lt 0) then xpad = 0
;if(ypad lt 0) then ypad = 0
;if(ypad+ydim gt win.prf.dy) then $
;    ypad = win.prf.dy - ydim
;for z=0,zdim-1 do begin
;    trans_outlines[xpad:xpad+xdim-1,ypad:ypad+ydim-1,z] = imgin[*,*,z]
;endfor

case win.sprf[win.idxidx].view of
    !TRANSVERSE: begin
        dx = win.prf.transverse_width
        dy = win.prf.transverse_height
    end
    !SAGITTAL: begin
        dx = win.prf.sagittal_width
        dy = win.prf.sagittal_height
    end
    !CORONAL: begin
        dx = win.prf.coronal_width
        dy = win.prf.coronal_height
    end
    else: begin
        print,'Invalid value of win.view'
        return
    end
endcase
trans_outlines = intarr(dx,dy,zdim)
xpad = dx/2 - centerx
ypad = dy/2 - centery
if(xpad lt 0) then xpad = 0
if(ypad lt 0) then ypad = 0
if(ypad+ydim gt dy) then $
    ypad = dy - ydim
for z=0,zdim-1 do begin
    trans_outlines[xpad:xpad+xdim-1,ypad:ypad+ydim-1,z] = imgin[*,*,z]
endfor

show_outlines = !TRUE

;print,'load_outlines bottom'
return
end

;***********************************
pro reslice_view,fi,vw,stc,SHOW=show
;***********************************
common view_images_comm_shared
common view_images_comm
;print,'reslice_view top'
if keyword_set(SHOW) then lcshow = !TRUE else lcshow = !FALSE
if win.resliced[win.idxidx] eq !TRUE then delete_cells,prf,win,win.idxidx,wd_data_set_status,-1,lc_delete_cells
if win.image_idx[win.idxidx] lt 0 then return
if vw.wdw[0] gt 0 then wset,vw.wdw[vw.cw]
hdr = *fi.hdr_ptr(win.image_idx[win.idxidx])
if win.sprf[win.idxidx].slice_not_z eq !TRUE then begin
    ;Slices are specified.
    case win.sprf[win.idxidx].view of
        !TRANSVERSE: slc = {name:'',x0:0,x1:0,dx:0,y0:0,y1:0,dy:0,z0:win.sprf[win.idxidx].first_slice, $
            z1:win.sprf[win.idxidx].last_slice,dz:win.sprf[win.idxidx].slice_incr,view:!TRANSVERSE}
        !SAGITTAL: slc = {name:'',x0:win.sprf[win.idxidx].first_slice,x1:win.sprf[win.idxidx].last_slice, $
            dx:win.sprf[win.idxidx].slice_incr,y0:0,y1:0,dy:0,z0:0,z1:0,dz:0,view:!SAGITTAL}
        !CORONAL: slc = {name:'',x0:0,x1:0,dx:0,y0:win.sprf[win.idxidx].first_slice,y1:win.sprf[win.idxidx].last_slice, $
            dy:win.sprf[win.idxidx].slice_incr,z0:0,z1:0,dz:0,view:!CORONAL}
        else: begin
            print,'Invalid value of win.view'
            return
        end
    endcase
endif else begin
    case win.sprf[win.idxidx].view of
        !TRANSVERSE: str = string(win.sprf[win.idxidx].first_slice,win.sprf[win.idxidx].last_slice, $
            win.sprf[win.idxidx].slice_incr,FORMAT='("(0,0,",i4,"...",i4,"\",i4,")")')
        !SAGITTAL: str = string(win.sprf[win.idxidx].first_slice,win.sprf[win.idxidx].last_slice, $
            win.sprf[win.idxidx].slice_incr,FORMAT='("(",i4,"...",i4,"\",i4,",0,0)")')
        !CORONAL: str = string(win.sprf[win.idxidx].first_slice,win.sprf[win.idxidx].last_slice, $
            win.sprf[win.idxidx].slice_incr,FORMAT='("(0,",i4,"...",i4,"\",i4,",0)")')
        else: begin
            print,'Invalid value of win.view'
            return
        end
    endcase
    str = strcompress(str,/REMOVE_ALL)
    slc = parse_slice_definition(str)
endelse
anat_mask = fltarr(hdr.xdim,hdr.ydim,hdr.zdim)
if win.sprf[win.idxidx].mask eq !TRUE then begin
    if win.mask_idx[win.idxidx] eq -1 then $
        anat_mask[*] = 1 $
    else $ 
        for z=0,hdr.zdim-1 do anat_mask[*,*,z] = get_image(z,fi,stc,FILNUM=win.mask_idx[win.idxidx]+1)
endif else begin
    anat_mask[*] = 1
endelse
labels = replicate({View_label},!NUM_FILES)
if win.over_idx[win.idxidx] ge 0 then begin
    over_image = fltarr(hdr.xdim,hdr.ydim,hdr.zdim)
    z1 = win.over_tidx[win.idxidx]*hdr.zdim
    for z=0,hdr.zdim-1 do over_image[*,*,z] = get_image(z1+z,fi,stc,FILNUM=win.over_idx[win.idxidx]+1)
    over_image_val = over_image
    index = where(over_image ne 0.,count)
    if count ne 0. then begin
        if win.prf.coding eq 1 then begin
            scrap = over_image
            over_image = fltarr(hdr.xdim,hdr.ydim,hdr.zdim)
            repeat begin
                ex = round(alog10(scrap))
                over_image = over_image + 2L^ex
                scrap = (scrap - 10L^ex) > 0.
            endrep until total(scrap) eq 0.
            junkmax = max(over_image,MIN=junkmin)
        endif
        win.over_max[win.idxidx] = max(over_image[index],MIN=scrap)
        win.over_min[win.idxidx] = scrap
        if win.prf.coding eq 1 then begin
            win.prf.scale_pos_max = win.over_max[win.idxidx]
            win.prf.scale_pos_min = win.over_min[win.idxidx]
        endif
    endif
    if win.nreg[win.idxidx] gt 0 then begin
        scrap = *fi.hdr_ptr[win.over_idx[win.idxidx]]
        if scrap.ifh.nreg ne 0 then begin
            scrapstr = intarr(win.nreg[win.idxidx])
            for i=0,win.nreg[win.idxidx]-1 do scrapstr[i] = total(over_image eq (i+2))
            scrapstr = strtrim(scrapstr,2)
            regnames = ['','',*scrap.ifh.region_names]
            for i=0,win.nreg[win.idxidx]-1 do regnames[i+2] = regnames[i+2] + ' ' + scrapstr[i]
            voxval = indgen(win.nreg[win.idxidx])+2
            regnamesi = voxval
        endif
    endif
endif
case slc.view of
    !TRANSVERSE: begin
        slices = lonarr((slc.z1 - slc.z0)/slc.dz + 1)

        ;print,'here0 slices=',slices
        ;print,'here0 n_elements(slices)=',n_elements(slices)

        num_slc = 0
        zs = float(slices)
        if win.sprf[win.idxidx].slice_not_z eq !TRUE or win.atlas_space[win.idxidx] eq !SPACE_DATA then begin
            if slc.z0 gt hdr.zdim-1 then z0 = hdr.zdim-1 else z0 = slc.z0
            if slc.z1 lt 0 then z1 = 0 else z1 = slc.z1 
            for z=z0,z1,slc.dz do begin
                slices[num_slc] = z
                num_slc = num_slc + 1
            endfor
        endif else begin

            ;print,'here1 slc.z0=',slc.z0,' slc.z1=',slc.z1,' slc.dz=',slc.dz

            for z=slc.z0,slc.z1,slc.dz do begin

                ;print,'here1a slc.x0=',slc.x0,' slc.y0=',slc.y0,' z=',z

                v = atlas_to_image_space(hdr,slc.x0,slc.y0,z,!TRANSVERSE,win.atlas_space[win.idxidx])
                v[2] = floor(v[2]) ;ADDED 020528
                if(n_elements(v) eq 3) then begin
                    slices[num_slc] = v[2]
                endif else begin
                    slices[num_slc] = 0
                    print,'Error in slice specification.'
                    return
                endelse

                ;print,'here0 view_images v=',v

                vv = get_atlas_coords(v[0],v[1],v[2],0,0,slc.view,win.atlas_space[win.idxidx],0,0,1, $
                    win.sprf[win.idxidx].sagittal_face_left,prf.orientation,hdr.xdim,hdr.ydim,fi,stc,dsp)

                ;print,'here1 view_images vv=',vv

                zs[num_slc] = vv[2]
                num_slc = num_slc + 1
            endfor
        endelse

        ;print,'here2 slices=',slices
        ;print,'here2 n_elements(slices)=',n_elements(slices)

        tmp_slices = slices[0:num_slc-1]
        slc_lab = strarr(num_slc)
        slcm1 = -1
        slc = 0
        for islc=0,num_slc-1 do begin
            if tmp_slices[islc] ne slcm1 then begin
                slices[slc] = tmp_slices[islc]
                slcm1 = tmp_slices[islc]
                if win.sprf[win.idxidx].slice_not_z eq !TRUE or win.atlas_space[win.idxidx] eq !SPACE_DATA then $
                    slc_lab[slc] = strcompress(string(slices[slc],FORMAT='("Slice ",i3)')) $
                else $
                    slc_lab[slc] = strcompress(string(zs[slc],FORMAT='("Z = ",i3)'))
                slc = slc + 1
            endif
        endfor
        num_slc = slc
        slices = slices[0:num_slc-1]
        ncell = win.ncell
        xdim = win.prf.transverse_width
        ydim = win.prf.transverse_height
        if xdim gt hdr.xdim then begin
            xdim = hdr.xdim
            xpad = 0
        endif else begin

            ;xpad = (hdr.xdim - xdim)/2
            ;START180309
            ;xpad = ceil(float(hdr.xdim - xdim)/2.) ;MNI111 xdim=197
            xpad = float(hdr.xdim - xdim)/2. ;MNI111 xdim=197

        endelse
        if ydim gt hdr.ydim then begin
            ydim = hdr.ydim
            ypad = 0
        endif else begin
            if win.atlas_space[win.idxidx] eq !SPACE_222 then begin 
                ypad = (hdr.ydim - ydim)/2 + !ATLAS_222_YOFFSET - 1
            endif else begin 

                ;ypad = (hdr.ydim - ydim)/2
                ;START180309
                ypad = float(hdr.ydim - ydim)/2.

            endelse
        endelse

        ;print,'************** view_images xpad=',xpad,' ypad=',ypad

        for islc=0,num_slc-1 do begin
            win.cell[ncell].view = !TRANSVERSE
            win.cell[ncell].xdim = xdim
            win.cell[ncell].ydim = ydim
            win.cell[ncell].xpad = xpad
            win.cell[ncell].ypad = ypad
            win.cell[ncell].valid = !TRUE
            win.cell[ncell].slice = slices[islc]
            win.cell[ncell].paint = !TRUE
            win.cell[ncell].image_set = win.idxidx
            if lcshow eq !TRUE then begin
                win.translation_table[win.ntrans] = ncell
                win.ntrans = win.ntrans + 1
            endif
            labels[0].x = 0 ; win.prf.sx
            labels[0].y = 0 ; win.prf.sy
            labels[0].s = slc_lab[islc]
            labels[0].font = vw.fnt.current_font
            win.cell[ncell].nlab = 1
            if ptr_valid(win.cell[ncell].labels) then ptr_free,win.cell[ncell].labels
            win.cell[ncell].labels = ptr_new(labels)

            ;print,'here5 slices=',slices
            ;print,'here5 n_elements(slices)=',n_elements(slices)
            ;print,'here5 islc=',islc


            img = get_image(slices[islc],fi,stc,FILNUM=win.image_idx[win.idxidx]+1)
            base_image_val = img
            img = img * anat_mask[*,*,slices[islc]]
            img = img[xpad:xpad+xdim-1,ypad:ypad+ydim-1]
            min1 = min(img,MAX=max1)
            win.cell[ncell].base_min = min1
            win.cell[ncell].base_max = max1
            win.cell[ncell].base_image = ptr_new(img)

            ;START180309
            base_image_val = base_image_val[xpad:xpad+xdim-1,ypad:ypad+ydim-1]

            win.cell[ncell].base_image_val = ptr_new(base_image_val)


            if win.over_idx[win.idxidx] ge 0 then begin
                over_img = over_image[*,*,slices[islc]] * anat_mask[*,*,slices[islc]]
                over_img = over_img[xpad:xpad+xdim-1,ypad:ypad+ydim-1]
                win.cell[ncell].overlay_image = ptr_new(over_img)
                min1 = min(over_img,MAX=max1)
                win.cell[ncell].overlay_min = min1
                win.cell[ncell].overlay_max = max1
                over_img_val = over_image_val[*,*,slices[islc]]
                over_img_val = over_img_val[xpad:xpad+xdim-1,ypad:ypad+ydim-1]
                win.cell[ncell].overlay_image_val = ptr_new(over_img_val)
            endif
            ncell = ncell + 1
        endfor
        win.ncell = ncell


;    over_image = fltarr(hdr.xdim,hdr.ydim,hdr.zdim)
;    z1 = win.over_tidx[win.idxidx]*hdr.zdim
;    for z=0,hdr.zdim-1 do over_image[*,*,z] = get_image(z1+z,fi,stc,FILNUM=win.over_idx[win.idxidx]+1)
;    over_image_val = over_image



    end
    !SAGITTAL: begin
        slices = lonarr((slc.x1 - slc.x0)/slc.dx + 1)
        num_slc = 0
        xs = float(slices)
        if win.sprf[win.idxidx].slice_not_z eq !TRUE or win.atlas_space[win.idxidx] eq !SPACE_DATA then begin
            if slc.x0 gt hdr.xdim-1 then x0 = hdr.xdim-1 else x0 = slc.x0
            if slc.x1 lt 0 then x1 = 0 else x1 = slc.x1
            for x=x0,x1,slc.dx do begin
                slices[num_slc] = x
                num_slc = num_slc + 1
            endfor
        endif else begin
            for x=slc.x0,slc.x1,slc.dx do begin
                v = atlas_to_image_space(hdr,x,slc.y0,slc.z0,!SAGITTAL,win.atlas_space[win.idxidx])
                v[2] = floor(v[2]) ;ADDED 020528
                if(n_elements(v) eq 3) then begin
                    slices[num_slc] = v[2]
                endif else begin
                    slices[num_slc] = 0
                    print,'Error in slice specification.'
                    return
                endelse
                vv = get_atlas_coords(v[0],v[1],v[2],0,0,slc.view,win.atlas_space[win.idxidx],0,0,1, $
                    win.sprf[win.idxidx].sagittal_face_left,prf.orientation,hdr.ydim,hdr.zdim,fi,stc,dsp)
                xs[num_slc] = vv[0]
                num_slc = num_slc + 1
            endfor
        endelse
        tmp_slices = slices[0:num_slc-1]
        slc_lab = strarr(num_slc)
        slcm1 = -1
        slc = 0
        for islc=0,num_slc-1 do begin
            if(tmp_slices[islc] ne slcm1) then begin
                slices[slc] = tmp_slices[islc]
                if win.sprf[win.idxidx].slice_not_z eq !TRUE or win.atlas_space[win.idxidx] eq !SPACE_DATA then $
                    slc_lab[slc] = strcompress(string(slices[slc],FORMAT='("Slice ",i3)')) $
                else $
                    slc_lab[slc] = strcompress(string(xs[slc],FORMAT='("X = ",i3)'))
                slc = slc + 1
                slcm1 = tmp_slices[islc]
            endif
        endfor
        num_slc = slc
        slices = slices[0:num_slc-1]
        ncell = win.ncell
        image = fltarr(hdr.xdim,hdr.ydim,hdr.zdim)
        base_image_val = fltarr(hdr.xdim,hdr.ydim,hdr.zdim) 
        for z=0,hdr.zdim-1 do begin
            image[*,*,z] = get_image(z,fi,stc,FILNUM=win.image_idx[win.idxidx]+1)
            base_image_val[*,*,z] = image[*,*,z]
            image[*,*,z] = image[*,*,z] * anat_mask[*,*,z]
        endfor
        if win.over_idx[win.idxidx] ge 0 then for z=0,hdr.zdim-1 do over_image[*,*,z] = over_image[*,*,z] * anat_mask[*,*,z]
        xdim = win.prf.sagittal_width
        ydim = win.prf.sagittal_height
        if xdim gt hdr.ydim then begin
            xdim = hdr.ydim
            xpad = 0
        endif else begin
            if win.atlas_space[win.idxidx] eq !SPACE_222 then $
                xpad = (hdr.ydim - xdim)/2 + !ATLAS_222_YOFFSET - 1 $
            else $
                xpad = (hdr.ydim - xdim)/2
        endelse
        if ydim gt hdr.zdim then begin
            ydim = hdr.zdim
            ypad = 0
        endif else begin
            ypad = (hdr.zdim - ydim)/2
        endelse
        scrapx = xpad+xdim-1
        if scrapx gt hdr.ydim-1 then scrapx = hdr.ydim-1
        scrapy = ypad+ydim-1
        if scrapy gt hdr.zdim-1 then scrapy = hdr.zdim-1
        for islc=0,num_slc-1 do begin
            win.cell[ncell].view = !SAGITTAL
            win.cell[ncell].xdim = xdim
            win.cell[ncell].ydim = ydim
            win.cell[ncell].xpad = xpad
            win.cell[ncell].ypad = ypad
            win.cell[ncell].valid = !TRUE
            win.cell[ncell].slice = slices[islc]
            win.cell[ncell].paint = !TRUE
            win.cell[ncell].image_set = win.idxidx
            if lcshow eq !TRUE then begin
                win.translation_table[win.ntrans] = ncell
                win.ntrans = win.ntrans + 1
            endif
            labels[0].x = 0 ; win.prf.sx
            labels[0].y = 0 ; win.prf.sy
            labels[0].s = slc_lab[islc]
            labels[0].font = vw.fnt.current_font
            win.cell[ncell].nlab = 1
            if(ptr_valid(win.cell[ncell].labels)) then $
                ptr_free,win.cell[ncell].labels
            win.cell[ncell].labels = ptr_new(labels)
            img = reform(image[slices[islc],*,*],hdr.ydim,hdr.zdim)
            img = img[xpad:scrapx,ypad:scrapy]
            img = reverse(img,2)
            if win.sprf[win.idxidx].sagittal_face_left eq !FALSE then img = reverse(img,1)
            win.cell[ncell].base_image = ptr_new(img)
            min1 = min(img,MAX=max1)
            win.cell[ncell].base_min = min1
            win.cell[ncell].base_max = max1
            img = reform(base_image_val[slices[islc],*,*],hdr.ydim,hdr.zdim)
            img = img[xpad:scrapx,ypad:scrapy]
            img = reverse(img,2)
            if win.sprf[win.idxidx].sagittal_face_left eq !FALSE then img = reverse(img,1)
            win.cell[ncell].base_image_val = ptr_new(img)
            if win.over_idx[win.idxidx] ge 0 then begin
                over_img = reform(over_image[slices[islc],*,*],hdr.ydim,hdr.zdim)
                over_img = over_img[xpad:scrapx,ypad:scrapy]
                over_img = reverse(over_img,2)
                if win.sprf[win.idxidx].sagittal_face_left eq !FALSE then over_img = reverse(over_img,1)
                win.cell[ncell].overlay_image = ptr_new(over_img)
                min1 = min(over_img,MAX=max1)
                win.cell[ncell].overlay_min = min1
                win.cell[ncell].overlay_max = max1
                over_img_val = reform(over_image_val[slices[islc],*,*],hdr.ydim,hdr.zdim)
                over_img_val = over_img_val[xpad:scrapx,ypad:scrapy]
                over_img_val = reverse(over_img_val,2)
                if win.sprf[win.idxidx].sagittal_face_left eq !FALSE then over_img_val = reverse(over_img_val,1)
                win.cell[ncell].overlay_image_val = ptr_new(over_img_val)

            endif
            ncell = ncell + 1
        endfor
        win.ncell = ncell
    end
    !CORONAL: begin
        slices = lonarr((slc.y1 - slc.y0)/slc.dy + 1)
        num_slc = 0
        ys = float(slices)
        if win.sprf[win.idxidx].slice_not_z eq !TRUE or win.atlas_space[win.idxidx] eq !SPACE_DATA then begin
            if slc.y0 gt hdr.ydim-1 then y0 = hdr.ydim-1 else y0 = slc.y0
            if slc.y1 lt 0 then y1 = 0 else y1 = slc.y1
            for y=y0,y1,slc.dy do begin
                slices[num_slc] = y
                num_slc = num_slc + 1
            endfor
        endif else begin
            for y=slc.y0,slc.y1,slc.dy do begin
                v = atlas_to_image_space(hdr,slc.x0,y,slc.z0,slc.view,win.atlas_space[win.idxidx])
                v[2] = floor(v[2]) ;ADDED 020528
                if n_elements(v) eq 3 then begin
                    slices[num_slc] = v[2]
                endif else begin
                    slices[num_slc] = 0
                    print,'Error in slice specification.'
                    return
                endelse
                vv = get_atlas_coords(v[0],v[1],v[2],0,0,slc.view,win.atlas_space[win.idxidx],0,0,1, $
                    win.sprf[win.idxidx].sagittal_face_left,prf.orientation,hdr.ydim,hdr.zdim,fi,stc,dsp)
                ys[num_slc] = vv[1]
                num_slc = num_slc + 1
            endfor
        endelse
        tmp_slices = slices[0:num_slc-1]
        labels = replicate({View_label},num_slc)
        slc_lab = strarr(num_slc)
        slcm1 = -1
        slc = 0
        for islc=0,num_slc-1 do begin
            if tmp_slices[islc] ne slcm1 then begin
                slices[slc] = tmp_slices[islc]
                if win.sprf[win.idxidx].slice_not_z eq !TRUE or win.atlas_space[win.idxidx] eq !SPACE_DATA then $
                    slc_lab[slc] = 'Slice ' + strtrim(fix(slices[slc]),2) $
                else $
                    slc_lab[slc] = 'Y = ' + strtrim(fix(ys[slc]),2)
                slc = slc + 1
                slcm1 = tmp_slices[islc]
            endif
        endfor
        num_slc = slc
        slices = slices[0:num_slc-1]
        ncell = win.ncell
        image = fltarr(hdr.xdim,hdr.ydim,hdr.zdim)
        base_image_val = fltarr(hdr.xdim,hdr.ydim,hdr.zdim)
        for z=0,hdr.zdim-1 do begin
            image[*,*,z] = get_image(z,fi,stc,FILNUM=win.image_idx[win.idxidx]+1)
            base_image_val[*,*,z] = image[*,*,z]
            image[*,*,z] = image[*,*,z] * anat_mask[*,*,z]
        endfor
        if win.over_idx[win.idxidx] ge 0 then for z=0,hdr.zdim-1 do over_image[*,*,z] = over_image[*,*,z] * anat_mask[*,*,z]
        xdim = win.prf.coronal_width
        ydim = win.prf.coronal_height
        if xdim gt hdr.xdim then begin
            xdim = hdr.xdim
            xpad = 0
        endif else begin
            if win.atlas_space[win.idxidx] eq !SPACE_222 then $
                xpad = (hdr.xdim - xdim)/2 + !ATLAS_222_YOFFSET - 1 $
            else $
                xpad = (hdr.xdim - xdim)/2
        endelse
        if ydim gt hdr.zdim then begin
            ydim = hdr.zdim
            ypad = 0
        endif else begin
            ypad = (hdr.zdim - ydim)/2
        endelse
        scrapx = xpad+xdim-1
        if scrapx gt hdr.xdim-1 then scrapx = hdr.xdim-1
        scrapy = ypad+ydim-1
        if scrapy gt hdr.zdim-1 then scrapy = hdr.zdim-1
        for islc=0,num_slc-1 do begin
            win.cell[ncell].view = !CORONAL
            win.cell[ncell].xdim = xdim
            win.cell[ncell].ydim = ydim
            win.cell[ncell].xpad = xpad
            win.cell[ncell].ypad = ypad
            win.cell[ncell].valid = !TRUE
            win.cell[ncell].slice = slices[islc]
            win.cell[ncell].paint = !TRUE
            win.cell[ncell].image_set = win.idxidx
            if lcshow eq !TRUE then begin
                win.translation_table[win.ntrans] = ncell
                win.ntrans = win.ntrans + 1
            endif
            labels[0].x = 0 ; win.prf.sx
            labels[0].y = 0 ; win.prf.sy
            labels[0].s = slc_lab[islc]
            labels[0].font = vw.fnt.current_font
            win.cell[ncell].nlab = 1
            if ptr_valid(win.cell[ncell].labels) then ptr_free,win.cell[ncell].labels
            win.cell[ncell].labels = ptr_new(labels)
            img = reform(image[*,slices[islc],*],hdr.xdim,hdr.zdim)
            img = img[xpad:scrapx,ypad:scrapy]
            img = reverse(img,2)
            win.cell[ncell].base_image = ptr_new(img)
            min1 = min(img,MAX=max1)
            win.cell[ncell].base_min = min1
            win.cell[ncell].base_max = max1
            img = reform(base_image_val[*,slices[islc],*],hdr.xdim,hdr.zdim)
            img = img[xpad:scrapx,ypad:scrapy]
            img = reverse(img,2)
            win.cell[ncell].base_image_val = ptr_new(img)
            if win.over_idx[win.idxidx] ge 0 then begin
                over_img = reform(over_image[*,slices[islc],*],hdr.xdim,hdr.zdim)
                over_img = over_img[xpad:scrapx,ypad:scrapy]
                over_img = reverse(over_img,2)
                win.cell[ncell].overlay_image = ptr_new(over_img)
                min1 = min(over_img,MAX=max1)
                win.cell[ncell].overlay_min = min1
                win.cell[ncell].overlay_max = max1
                over_img_val = reform(over_image_val[*,slices[islc],*],hdr.xdim,hdr.zdim)
                over_img_val = over_img_val[xpad:scrapx,ypad:scrapy]
                over_img_val = reverse(over_img_val,2)
                win.cell[ncell].overlay_image_val = ptr_new(over_img_val)
            endif
            ncell = ncell + 1
        endfor
        win.ncell = ncell
    end
    else: begin
        print,'Invalid value of view from parse_slice_defintion()'
        return
    end
endcase
;print,'reslice_view bottom'
end


;***************************************************************
pro outline_cell,win,itr,zoom_in,ydim_draw,display_mode,color
;***************************************************************
if((itr lt win.ntrans) and (itr ge 0)) then begin
    icell  = win.translation_table[itr]
    win.insertion_point = itr
    if(win.cell[icell].view eq !VIEW_COLOR_SCALE) then $
        zoom = 1 $
    else  $
        zoom = zoom_in


    case display_mode of
        !OVERLAY_MODE: ydim = win.cell[icell].ydim
        !ANAT_OVER_ACT_MODE: begin
            if(win.cell[icell].view eq !VIEW_COLOR_SCALE) then $
                 ydim = win.cell[icell].ydim $
             else $
                 ydim = 2*win.cell[icell].ydim
        end
        !VIEW_COLOR_SCALE: ydim = win.cell[icell].ydim
        else: print,'Invalid value of display_mode in outline_cell.'
    endcase

    ;This is for !OVERLAY_MODE
    ;case win.cell[icell].view of
    ;    !TRANSVERSE: ydim = win.prf.transverse_height
    ;    !SAGITTAL: ydim = win.prf.sagittal_height
    ;    !CORONAL: ydim = win.prf.coronal_height
    ;    else: begin
    ;        print,'Error in display view. ydim will be assigned the value in cell.ydim.'
    ;        ydim = win.cell[icell].ydim
    ;    endelse
    ;endcase
    ;case display_mode of
    ;    !OVERLAY_MODE: ;do nothing
    ;    !ANAT_OVER_ACT_MODE: begin
    ;        if(win.cell[icell].view eq !VIEW_COLOR_SCALE) then $
    ;             ydim = win.cell[icell].ydim $
    ;         else $
    ;             ydim = 2*win.cell[icell].ydim
    ;    end
    ;    !VIEW_COLOR_SCALE: ydim = win.cell[icell].ydim
    ;    else: print,'Invalid value of display_mode in outline_cell.'
    ;endcase


    x = intarr(5)
    y = intarr(5)
 
    x(0) = win.cell[icell].x
    y(0) = win.cell[icell].y
    x(1) = x[0]
    y(1) = win.cell[icell].y + zoom*ydim
    x(2) = win.cell[icell].x + zoom*win.cell[icell].xdim
    y(2) = win.cell[icell].y + zoom*ydim
    x(3) = win.cell[icell].x + zoom*win.cell[icell].xdim
    y(3) = win.cell[icell].y
    x(4) = x[0]
    y(4) = y[0]
    for i=0,4 do $
        y[i] = ydim_draw - y[i] - 1
    
    plots,x,y,/DEV,/NOCLIP,COLOR=color
    if(color eq 0) then $
       color1 = 0 $
    else $
       color1 = !YELLOW
    plots,[x[0]-5,x[0]-5],[y[0],y[1]],/DEV,/NOCLIP,COLOR=color1
endif else begin
    win.insertion_point = win.ntrans
    if(color eq 0) then $
       color1 = 0 $
    else $
       color1 = !YELLOW
    ic = win.translation_table[win.ntrans - 1]
help,ic
    if(win.cell[ic].view eq !VIEW_COLOR_SCALE) then ic = ic - 1
    y = [win.cell[ic].y,win.cell[ic].y + zoom_in*win.cell[ic].ydim]
    y = ydim_draw - y - 1
    x = [win.cell[ic].x + zoom_in*win.cell[ic].xdim,win.cell[ic].x + zoom_in*win.cell[ic].xdim]
    plots,x,y,/DEV,/NOCLIP,COLOR=color1
endelse
end

;******************************
pro find_label_num,lnum,win,x,y
;******************************

lnum = -1
if(ptr_valid(win.labels)) then $
    labels = *win.labels $
else $
    return

for ilab=0,win.nlab-1 do begin
    x0 = labels[ilab].x
    y0 = labels[ilab].y
    if((x ge x0) and (x lt x0+labels[ilab].wdth)) then begin
        if((y le y0) and (y gt y0-labels[ilab].hgth)) then begin
            lnum = ilab
            return
        endif
     endif
endfor

return
end


;*****************************************
pro outline_label,win,lnum,ydim_draw,color
;*****************************************
if ptr_valid(win.labels) then $
    labels = *win.labels $
else $
    return
x = lonarr(5)
y = lonarr(5)
x[0] = labels[lnum].x
y[0] = labels[lnum].y
x[1] = x[0]
y[1] = y[0] - labels[lnum].hgth
x[2] = x[0] + labels[lnum].wdth
y[2] = y[0] - labels[lnum].hgth
x[3] = x[0] + labels[lnum].wdth
y[3] = y[0]
x[4] = x[0]
y[4] = y[0]
for i=0,4 do y[i] = ydim_draw - y[i] - 1
plots,x,y,/DEV,/NOCLIP,COLOR=color
end


;***********************************************************************
pro outline_drag_label,win,current_label,ydim_draw,color,delta_x,delta_y
;***********************************************************************
labels = *win.labels
label = labels[current_label]
x = intarr(5)
y = intarr(5)
xx = label.x + delta_x
yy = label.y + delta_y
x[0] = xx
y[0] = yy
x[1] = x[0]
y[1] = yy + label.hgth
x[2] = xx + label.wdth
y[2] = yy + label.hgth
x[3] = xx + label.wdth
y[3] = yy
x[4] = x[0]
y[4] = y[0]
for i=0,4 do y[i] = ydim_draw - y[i] - 1
plots,x,y,/DEV,/NOCLIP,COLOR=color
end


;************************************************************************************
pro outline_drag_cell,win,itr,zoom_in,ydim_draw,display_mode,color,delta_x,delta_y
;************************************************************************************
icell  = win.translation_table[itr]
if(win.cell[icell].view eq !VIEW_COLOR_SCALE) then $
    zoom = 1 $
else  $
    zoom = zoom_in
case display_mode of
    !OVERLAY_MODE: ydim = win.cell[icell].ydim
    !ANAT_OVER_ACT_MODE: begin
        if(win.cell[icell].view eq !VIEW_COLOR_SCALE) then $
             ydim = win.cell[icell].ydim $
         else $
             ydim = 2*win.cell[icell].ydim
    end
    else: print,'Invalid value of display_mode in outline_cell.'
endcase
x = intarr(5)
y = intarr(5)
xx = win.cell[icell].x + delta_x
yy = win.cell[icell].y + delta_y
x(0) = xx
y(0) = yy
x(1) = x[0]
y(1) = yy + zoom*            ydim
x(2) = xx + zoom*win.cell[icell].xdim
y(2) = yy + zoom*            ydim
x(3) = xx + zoom*win.cell[icell].xdim
y(3) = yy
x(4) = x[0]
y(4) = y[0]
for i=0,4 do y[i] = ydim_draw - y[i] - 1
plots,x,y,/DEV,/NOCLIP,COLOR=color
end

;*******************
pro reload,fi,dsp,vw
;*******************
common view_images_comm_shared
common view_images_comm

win.lastx = prf.xgap
win.lasty = prf.ygap

ydim_draw_new = win.ydim_draw

if !D.WINDOW gt -1 then erase
win.xmax = 0
for itr=0,win.ntrans-1 do begin
    icell = win.translation_table[itr]
    if(win.cell[icell].paint eq !TRUE) then begin
        if(win.cell[icell].view eq !VIEW_COLOR_SCALE) then begin
;;;            win.cell[icell].x = win.xdim_draw - 4*!COLSCL_WIDTH 
;;;            win.cell[icell].y = win.ydim_draw - win.cell[icell].ydim
        endif else begin
            win.cell[icell].x = win.lastx
            win.cell[icell].y = win.lasty
            image_set = win.cell[icell].image_set
            win.lastx = win.lastx + prf.zoom*win.cell[icell].xdim + prf.xgap
            if((win.lastx+prf.zoom*win.cell[icell].xdim) gt win.xdim_draw) then begin
                win.lastx = prf.xgap
                case display_mode of
                    !OVERLAY_MODE: delta_y = prf.zoom*win.cell[icell].ydim
                    !ANAT_OVER_ACT_MODE: delta_y = 2*prf.zoom*win.cell[icell].ydim
                    else: print,'Invalid value of display_mode in reload.'
                endcase
                win.lasty = win.lasty + delta_y + prf.ygap
                if(win.lasty+prf.zoom*win.cell[icell].ydim+prf.ygap gt win.ydim_draw) then $
                    ydim_draw_new = win.lasty + delta_y + prf.ygap
            endif
            if(win.lastx+prf.zoom*win.cell[icell].xdim gt win.xmax) then $
                win.xmax = win.lastx+prf.zoom*win.cell[icell].xdim + prf.xgap
            win.xmax = win.xmax < win.xdim_draw
        endelse
    endif
endfor
if prf.color_y gt ydim_draw_new then ydim_draw_new = prf.color_y

;This was commented out on 050102
;if ydim_draw_new gt win.ydim_draw and vw.id[vw.cw] ge 0 then begin
;    win.ydim_draw = ydim_draw_new
;    widget_control,vw.id[vw.cw],DRAW_YSIZE=win.ydim_draw
;endif

*vw.win[vw.cw] = win
return
end

;*****************************************************************
pro draw_to_array,image,fi,dsp,font,nc,SELECTED_IMGS=selected_imgs
;*****************************************************************
common view_images_comm_shared
common view_images_comm

xmin = win.xdim_draw
xmax = 0
ymin = win.ydim_draw
ymax = 0
if(n_elements(current_cells) eq 1) then $
    if(current_cells[0] ge 0) then $
        nsel = 1 $
    else $
        nsel = 0 $
else $
    nsel = n_elements(current_cells)
if(keyword_set(SELECTED_IMGS)) then begin
    if(nsel gt 0) then begin
        nc = nsel 
        cell_set = current_cells
    endif else begin
        nc = 0
        return
    endelse
endif else begin
    nc = win.ntrans
    cell_set = indgen(win.ntrans)
endelse
for itr=0,nc-1 do begin
    icell = win.translation_table[cell_set[itr]]
    if(win.cell[icell].valid eq !TRUE) then begin
        x0 = win.cell[icell].x - prf.xgap
        if(win.cell[icell].view eq !VIEW_COLOR_SCALE) then begin
            x1 = win.cell[icell].x + win.cell[icell].xdim - 1
            y1 = win.cell[icell].y + win.cell[icell].ydim - 1
        endif else begin
            x1 = win.cell[icell].x + prf.zoom*win.cell[icell].xdim - 1
            y1 = win.cell[icell].y + prf.zoom*win.cell[icell].ydim - 1
        endelse
        y0 = win.cell[icell].y
        if(x0 lt xmin) then xmin = x0
        if(x1 gt xmax) then xmax = x1
        if(y0 lt ymin) then ymin = y0
        if(y1 gt ymax) then ymax = y1
        image_set = win.cell[icell].image_set
    endif
endfor
if ptr_valid(win.labels) then begin
    checked = *win.label[win.idxidx].checked
    index = where(checked,count)
    if count ne 0 then begin
        labs = *win.labels
        labs[0].s = win.filenamelabels[win.idxidx]
        for ilab=0,count-1 do begin
            i = index[ilab]
            x = labs[i].x
            ;y = win.ydim_draw - labs[i].y - 1
            y = labs[i].y
            if x lt xmin then xmin = x
            if x gt xmax then xmax = x
            if y lt ymin then ymin = y
            if y gt ymax then ymax = y
        endfor
    endif
endif
xdim = xmax - xmin + prf.xgap + 1
ydim = ymax - ymin + prf.ygap + 1
if xmin lt 0 then xmin = 0
if ymin lt 0 then ymin = 0
if nsel gt 0 then view_images_refresh,fi,dsp,font,/NO_OUTLINES
if (!FIDLCOLORDEPTH eq 24) or (!FIDLCOLORDEPTH eq 16) then begin
    image=tvrd(TRUE=1)
    image = reverse(image,3)
    xd = n_elements(image[0,*,0])
    yd = n_elements(image[0,0,*])
endif else begin
    image=tvrd() 
    image = reverse(image,2)
    xd = n_elements(image[*,0])
    yd = n_elements(image[0,*])
endelse
if xmax+prf.xgap gt xd-1 then xmax = xd - prf.xgap - 1
if ymax+prf.ygap gt yd-1 then ymax = yd - prf.ygap - 1

;if !MAC_OSX eq 'TRUE' eq 1 then $ 
;START110412
if (!FIDLCOLORDEPTH eq 24) or (!FIDLCOLORDEPTH eq 16) then $ 

    image = image[*,xmin:xmax+prf.xgap,ymin:ymax+prf.ygap] $
else $
    image = image[xmin:xmax+prf.xgap,ymin:ymax+prf.ygap]
for ic=0,nsel-1 do outline_cell,win,current_cells[ic],prf.zoom,win.ydim_draw,display_mode,!GREEN
end

pro write_tif,fi,dsp,font,SELECTED=selected,DONTASK=dontask
    common view_images_comm_shared
    common view_images_comm
    if keyword_set(SELECTED) then draw_to_array,image,fi,dsp,font,nc,/SELECTED_IMGS else draw_to_array,image,fi,dsp,font,nc
    if nc eq 0 then begin
        print,'No images to save'
        return
    endif
    if not keyword_set(DONTASK) then dontask=0
    idx = win.over_idx[win.idxidx]
    if idx eq -1 then idx = win.image_idx[win.idxidx]
    if dontask eq 0 then begin
        file = fi.list[idx]
        if win.sprf[win.idxidx].view eq !SAGITTAL then $
            file = file+'_sagittal' $
        else if win.sprf[win.idxidx].view eq !CORONAL then $
            file = file+'_coronal'
        file = file+'.tif'
        get_dialog_pickfile,'*.tif',over_path,'Please select file.',file,rtn_nfiles,path,FILE=file
        if file eq '' or file eq 'GOBACK' or file eq 'EXIT' then return
        keeppath = 1
    endif else begin 
        file = fi.tails[idx]
        if win.sprf[win.idxidx].view eq !SAGITTAL then $
            file = file+'_sagittal' $
        else if win.sprf[win.idxidx].view eq !CORONAL then $
            file = file+'_coronal'
        file = file+'.tif'
        keeppath = 0
    endelse
    write_tiff,file,image,RED=dsp.lct[*,0],GREEN=dsp.lct[*,1],BLUE=dsp.lct[*,2],COMPRESSION=1
    rtn=get_root(file,'.tif',KEEPPATH=keeppath)
    file = rtn.file+'_600dpi.tif'
    write_tiff,file,image,RED=dsp.lct[*,0],GREEN=dsp.lct[*,1],BLUE=dsp.lct[*,2],XRESOL=600,YRESOL=600
    print,'Image written to '+file
end

;****************************************************************************
pro display_view,cell,prefs,ydim_draw,MINMAX=minmax,OUTPUT_ARRAY=output_array
;****************************************************************************
common stats_comm
common view_images_comm_shared
common view_images_comm
;print,'display_view top'
if keyword_set(OUTPUT_ARRAY) then lc_array = !TRUE else lc_array = !FALSE
if(ptr_valid(cell.labels)) then begin
    labs = *cell.labels
    nlabs = cell.nlab
endif else begin
    nlabs = 0
endelse
bmin = 1.e20
bmax = -1.e20
omin = 1.e20
omax = -1.e20
for itr=0,win.ntrans-1 do begin
    ic = win.translation_table[itr]
    if((win.cell[ic].valid eq !TRUE) and (win.cell[ic].view ne !VIEW_COLOR_SCALE)) then begin
        bmin1 = min(*win.cell[ic].base_image,MAX=bmax1)
        if(bmin1 lt bmin) then bmin = bmin1
        if(bmax1 gt bmax) then bmax = bmax1
        if(ptr_valid(win.cell[ic].overlay_image)) then begin
            omin1 = min(*win.cell[ic].overlay_image,MAX=omax1)
            if(omin1 lt omin) then omin = omin1
            if(omax1 gt omax) then omax = omax1
         endif
    endif
endfor
if cell.view eq !VIEW_COLOR_SCALE then begin
    if prefs.anat_local_global eq 0 then begin
        dsp[fi.cw].min_primary = bmin
        dsp[fi.cw].max_primary = bmax
    endif else begin
        dsp[fi.cw].min_primary = prefs.scale_anat_min
        dsp[fi.cw].max_primary = prefs.scale_anat_max 
    endelse
    if win.over_idx[win.idxidx] eq -1 then begin
        omin = dsp[fi.cw].min_primary
        omax = dsp[fi.cw].max_primary
    endif else begin
        if((prefs.lc_auto_scale eq !TRUE) and (omin gt -1.e19)) then begin
            dsp[fi.cw].min_secondary = omin
            dsp[fi.cw].max_secondary = omax
        endif else begin
            dsp[fi.cw].min_secondary = prefs.scale_pos_min
            dsp[fi.cw].max_secondary = prefs.scale_pos_max
            case prf.lc_act_type of
                !POSITIVE_ACTIVATIONS: begin
                    omin = prefs.scale_pos_min
                    omax = prefs.scale_pos_max
                end
                !NEGATIVE_ACTIVATIONS: begin
                    omin = -abs(prefs.scale_neg_max)
                    omax = -abs(prefs.scale_neg_min)
                end
                !ALL_ACTIVATIONS: begin
                    omin = -abs(prefs.scale_neg_max)
                    omax =  prefs.scale_pos_max
                end
                else: print,'Invalid value of lc_act_type.'
            endcase
        endelse
    endelse
    if prf.lc_act_type eq !ALL_ACTIVATIONS then begin
        scl =  float(!COLSCL_YSZ-1)/float(prf.scale_pos_max + abs(prf.scale_neg_max))
        min_zero = scl*(abs(prf.scale_neg_max) - abs(prf.scale_neg_min))
        max_zero = scl*(prf.scale_pos_min + abs(prf.scale_neg_max))
    endif
    if prf.color_bar_type eq !NO_COLOR_BAR then begin 
        ;do nothing
    endif else begin
        cmd = 'plot_colorscale_labels'
        colorbarlabels = ''
        if win.nreg[win.idxidx] gt 0 then begin
            if ptr_valid(win.reg[win.idxidx].checked) then begin
                index = where(*win.reg[win.idxidx].checked gt 0,count)
                if count gt 0 then begin
                    if ptr_valid(win.reg[win.idxidx].colorbarlabels) then begin
                        colorbarlabels = *win.reg[win.idxidx].colorbarlabels
                    endif
                endif
            endif
        endif
        case prf.color_bar_type of
            !HORIZ_COLOR_BAR: begin
                prf.color_x = cell.x
                prf.color_y = cell.y
                x1 = !COLSCL_YOFF + cell.x
                y1 = ydim_draw - cell.y - cell.ydim + !COLSCL_XOFF
                x2 = cell.x + cell.xdim - !COLSCL_YOFF
                y2 = ydim_draw - cell.y - !COLSCL_WIDTH
                scale = *cell.base_image
                if prf.lc_act_type eq !ALL_ACTIVATIONS then scale[min_zero:max_zero,*] = 0
                tv,scale,x1,y1,/DEVICE
                y = y1 + !COLSCL_WIDTH
                arg3 = x1
                arg4 = y
                arg5 = x2
                arg6 = y+5
                vert_hor = 1
           end
           !VERT_COLOR_BAR: begin
                prf.color_x = cell.x
                prf.color_y = cell.y
                x1 = !COLSCL_XOFF + cell.x
                y1 = ydim_draw - cell.y - cell.ydim + !COLSCL_YOFF
                x2 = x1 + !COLSCL_WIDTH
                y2 = ydim_draw - cell.y - !COLSCL_YOFF
                scale = *cell.base_image
                if prf.lc_act_type eq !ALL_ACTIVATIONS then scale[*,min_zero:max_zero] = 0
                tv,scale,x1,y1,/DEVICE
                arg3 = x2
                arg4 = y1
                arg5 = x2+5
                arg6 = y2
                vert_hor = 0
           end
           else: print,'Invalid value of color_bar_type'
        endcase
        invert=0
        if omax gt omin then begin
            if prf.lc_act_type eq !NEGATIVE_ACTIVATIONS then invert=1 
        endif
        display_p=0
        rounded=0
        if prf.display_units eq !DISPLAY_P then $ 
            display_p=1 $
        else if prf.display_units eq !DISPLAY_P_ROUNDED then $ 
            rounded=1 ;/DISPLAY_P_ROUNDED is 'ambiguous' with /DISPLAY_P
        plot_colorscale_labels,omin,omax,arg3,arg4,arg5,arg6,colorbarlabels,'',vert_hor,display_p,rounded, $
            COLOR=win.prf.label_color+!LEN_COLORTAB
        if nlabs gt 0 then device,SET_FONT=labs[0].font ; Use same font as slice labels.
        device,SET_FONT=vw.fnt.current_font
    endelse
    prefs.color_x = cell.x
    prefs.color_y = cell.y
    win.prf = prefs
    return
endif
if ptr_valid(cell.base_image) then begin
    base_img = *cell.base_image
    anat_mask = base_img eq 0.
endif else begin
    print,'Invalid pointer to base image.'
    return
endelse
if ptr_valid(cell.overlay_image) then begin
    overlay_image = *cell.overlay_image
    img_max = max(overlay_image,MIN=img_min)
    if prefs.lc_auto_scale eq !TRUE then begin
        neg_min = abs(prefs.scale_neg_min)
        neg_max = abs(omin)
        pos_min = prefs.scale_pos_min
        pos_max = omax
    endif else begin
        neg_min = abs(prefs.scale_neg_min)
        neg_max = abs(prefs.scale_neg_max)
        pos_min = prefs.scale_pos_min
        pos_max = prefs.scale_pos_max
    endelse
    if win.nreg[win.idxidx] gt 0 then begin
        if ptr_valid(win.reg[win.idxidx].checked) then begin
            index = where(*win.reg[win.idxidx].checked gt 0,count)
            if count gt 0 then begin
                overlay_img = fltarr(cell.xdim,cell.ydim)
                if ptr_valid(win.reg[win.idxidx].color_index) then begin
                    color_index = *win.reg[win.idxidx].color_index
                    if ptr_valid(win.reg[win.idxidx].val) then voxval = *win.reg[win.idxidx].val
                    for i=0,count-1 do overlay_img = overlay_img + ((overlay_image eq voxval[index[i]])*color_index[index[i]])
                    mask_img = zoom_by_2n(overlay_img gt 0.,prefs.zoom,!NEAREST_NEIGHBOR)
                    overlay_img = zoom_by_2n(overlay_img+!LEN_COLORTAB1,prefs.zoom,!NEAREST_NEIGHBOR)
                    fi.color_scale1[!NUM_FILES] = !OVERLAY_SCALE
                    fi.color_scale2[!NUM_FILES] = !LINECOLORS
                endif else if ptr_valid(win.reg[win.idxidx].val) then begin
                    val = *win.reg[win.idxidx].val
                    for i=0,count-1 do overlay_img = overlay_img + ((overlay_image eq (index[i]+2))*val[index[i]])
                    mask_img = zoom_by_2n(overlay_img gt 0.,prefs.zoom,!NEAREST_NEIGHBOR)
                    overlay_img = zoom_by_2n(overlay_img,prefs.zoom,!NEAREST_NEIGHBOR)
                endif
            endif
        endif else begin
            if fi.color_scale2[!NUM_FILES] ne !LINECOLORS then begin

                ;START130311
                ;prefs.overlay_zoom_type = !NEAREST_NEIGHBOR

                goto,wallace
            endif
            mask_img = zoom_by_2n((overlay_image ge pos_min) and (overlay_image le pos_max),prefs.zoom,!NEAREST_NEIGHBOR)
            overlay_img = zoom_by_2n(overlay_image+!LEN_COLORTAB1-1,prefs.zoom,!NEAREST_NEIGHBOR)
        endelse
    endif else begin
        wallace:
        if prefs.coding ne 2 then begin
            case prefs.lc_act_type of
                !POSITIVE_ACTIVATIONS: begin
                    if pos_min eq 0. and pos_max le 1. then $ ;must be a power image
                        mask_img = zoom_by_2n(overlay_image gt pos_min,prefs.zoom,prefs.overlay_zoom_type) $
                    else $
                        mask_img = zoom_by_2n(overlay_image ge pos_min,prefs.zoom,prefs.overlay_zoom_type)
                    pos_overlay_img = (overlay_image ge pos_min)*(overlay_image < pos_max)
                    pos_overlay_img = zoom_by_2n(pos_overlay_img,prefs.zoom,prefs.overlay_zoom_type)
                    scl =  (!LEN_COLORTAB2-1)/(pos_max-pos_min)
                    overlay_img = scl*((pos_overlay_img - pos_min) > 0.) + !LEN_COLORTAB1
                end
                !NEGATIVE_ACTIVATIONS: begin
                    mask_img = zoom_by_2n(overlay_image le -neg_min,prefs.zoom,prefs.overlay_zoom_type)
                    neg_overlay_img = (overlay_image le -neg_min)*(overlay_image > (-neg_max))
                    neg_overlay_img = zoom_by_2n(neg_overlay_img,prefs.zoom,prefs.overlay_zoom_type)
                    scl =  (!LEN_COLORTAB2-1)/(neg_max-neg_min)
                    overlay_img = scl*((-neg_overlay_img - neg_min) > 0.) + !LEN_COLORTAB1
                end
                !ALL_ACTIVATIONS: begin
                    ;if elf eq 0 then begin 

                        orig = 0
                        if orig then begin 
                        pos_overlay_img = (overlay_image ge pos_min)*(overlay_image < pos_max)
                        pos_overlay_img = zoom_by_2n(pos_overlay_img,prefs.zoom,prefs.overlay_zoom_type)
                        pos_mask_img = pos_overlay_img gt 0.
                        neg_overlay_img = (overlay_image le -neg_min)*(overlay_image > (-neg_max))
                        neg_overlay_img = zoom_by_2n(neg_overlay_img,prefs.zoom,prefs.overlay_zoom_type)
                        neg_mask_img = neg_overlay_img lt 0.
                        mask_img = zoom_by_2n((overlay_image le -neg_min) or (overlay_image ge pos_min),prefs.zoom, $
                            prefs.overlay_zoom_type)
                        scl =  (!LEN_COLORTAB2-1)/(pos_max+neg_max)
                        pos_overlay_img = scl*(pos_overlay_img + pos_max) + !LEN_COLORTAB1
                        neg_overlay_img = scl*(neg_overlay_img + neg_max) + !LEN_COLORTAB1
                        overlay_img = pos_mask_img*pos_overlay_img + neg_mask_img*neg_overlay_img 
                        endif

                        ;;START081023
                        butface = 1
                        if butface then begin
                            pos_overlay_img = (overlay_image ge pos_min)*(overlay_image < pos_max)
                            pos_overlay_img = zoom_by_2n(pos_overlay_img,prefs.zoom,prefs.overlay_zoom_type)
                            pos_mask_img = pos_overlay_img gt 0.
                            neg_overlay_img = (overlay_image le -neg_min)*(overlay_image > (-neg_max))
                            neg_overlay_img = zoom_by_2n(neg_overlay_img,prefs.zoom,prefs.overlay_zoom_type)
                            neg_mask_img = neg_overlay_img lt 0.
                            
                            mask_img = zoom_by_2n((overlay_image le -neg_min) or (overlay_image ge pos_min),prefs.zoom, $
                                prefs.overlay_zoom_type)
                           
                            scl =  (!LEN_COLORTAB2-1)/(pos_max+neg_max)
                            pos_overlay_img = scl*(pos_overlay_img + pos_max) + !LEN_COLORTAB1
                            neg_overlay_img = scl*(neg_overlay_img + neg_max) + !LEN_COLORTAB1
                            ;print,'max(pos_overlay_img)=',max(pos_overlay_img),' min(pos_overlay_img)=',min(pos_overlay_img)
                            ;print,'max(neg_overlay_img)=',max(neg_overlay_img),' min(neg_overlay_img)=',min(neg_overlay_img)
                            
                            ;overlay_img = pos_mask_img*pos_overlay_img + neg_mask_img*neg_overlay_img
                            ;;overlay_img = pos_overlay_img + neg_overlay_img
                            ;overlay_img = pos_overlay_img
                            ;overlay_img = neg_overlay_img
                            ;overlay_img = pos_mask_img*pos_overlay_img
                            ;overlay_img = neg_mask_img*neg_overlay_img

                            junkindex = where((pos_mask_img and neg_mask_img) eq 1,junkcount)
                            ;print,'junkcount=',junkcount
                            if junkcount ne 0 then neg_mask_img[junkindex]=0
                            overlay_img = pos_mask_img*pos_overlay_img + neg_mask_img*neg_overlay_img
                        endif

                        ;START081024
                        dog = 0
                        if dog then begin
                            pos_overlay_img = (overlay_image ge pos_min)*(overlay_image < pos_max)
                            pos_mask_img = pos_overlay_img gt 0.
                            ;pos_mask_img = pos_overlay_img ge 0.
                            neg_overlay_img = (overlay_image le -neg_min)*(overlay_image > (-neg_max))
                            neg_mask_img = neg_overlay_img lt 0.
                            mask_img = zoom_by_2n((overlay_image le -neg_min) or (overlay_image ge pos_min),prefs.zoom, $
                                prefs.overlay_zoom_type)
                            scl =  (!LEN_COLORTAB2-1)/(pos_max+neg_max)
                            pos_overlay_img = scl*(pos_overlay_img + pos_max) + !LEN_COLORTAB1
                            neg_overlay_img = scl*(neg_overlay_img + neg_max) + !LEN_COLORTAB1
                            overlay_img = pos_mask_img*pos_overlay_img + neg_mask_img*neg_overlay_img
                            ;overlay_img = pos_overlay_img + neg_overlay_img
                            overlay_img = zoom_by_2n(overlay_img,prefs.zoom,prefs.overlay_zoom_type)
                        endif
                        cat = 0
                        if cat then begin
                            pos_overlay_img = (overlay_image ge pos_min)*(overlay_image < pos_max)
                            pos_overlay_img = zoom_by_2n(pos_overlay_img,prefs.zoom,prefs.overlay_zoom_type)
                            pos_mask_img = pos_overlay_img gt 0.
                            neg_overlay_img = (overlay_image le -neg_min)*(overlay_image > (-neg_max))
                            neg_overlay_img = zoom_by_2n(neg_overlay_img,prefs.zoom,prefs.overlay_zoom_type)
                            neg_mask_img = neg_overlay_img lt 0.
                            mask_img = zoom_by_2n((overlay_image le -neg_min) or (overlay_image ge pos_min),prefs.zoom, $
                                prefs.overlay_zoom_type)
                            scl =  (!LEN_COLORTAB2-1)/(pos_max+neg_max)
                            pos_overlay_img = scl*(pos_overlay_img + pos_max) + !LEN_COLORTAB1
                            neg_overlay_img = scl*(neg_overlay_img + neg_max) + !LEN_COLORTAB1
                            overlay_img = pos_mask_img*pos_overlay_img + neg_mask_img*neg_overlay_img
                        endif







                    ;endif else begin
                    ;    overlay_img = zoom_by_2n(overlay_image,prefs.zoom,prefs.overlay_zoom_type)
                    ;    mask_img = (overlay_img le -neg_min) or (overlay_img ge pos_min)
                    ;    pos_mask_img = overlay_img ge pos_min
                    ;    pos_overlay_img = pos_mask_img*(overlay_img < pos_max)
                    ;    neg_mask_img = overlay_img le -neg_min
                    ;    neg_overlay_img = neg_mask_img*(overlay_img > (-neg_max))
                    ;    pos_scl =  (!LEN_COLORTAB2-1)/(2.*pos_max)
                    ;    neg_scl =  (!LEN_COLORTAB2-1)/(2.*neg_max)
                    ;    overlay_img = pos_mask_img*(pos_scl*(pos_overlay_img + pos_max) + !LEN_COLORTAB1) + $
                    ;                  neg_mask_img*(neg_scl*(neg_overlay_img + neg_max) + !LEN_COLORTAB1)
                    ;endelse
                end
                else: print,'Invalid value of lc_act_type in display_view.'
            endcase
            mask_img = 0.*(mask_img lt .5) + (mask_img ge .5)
        endif else begin
            overlay_img = zoom_by_2n(overlay_image,prefs.zoom,prefs.overlay_zoom_type)
            mask_img = overlay_img le pos_max
        endelse
    endelse
    if fi.color_scale2[!NUM_FILES] ne !LINECOLORS then begin
        if prefs.coding eq 2 then begin
            scl =  (!LEN_COLORTAB2)/pos_max
            overlay_img = -scl*mask_img*((overlay_img < pos_max) - pos_max) + !LEN_COLORTAB1
        endif
    endif
endif

;START131016
bmin = min(base_img,MAX=bmax)

if sprf.slice_type eq !STATISTICAL then begin
    if prefs.lc_auto_scale eq !TRUE then begin
        min = min(base_img,MAX=max)
        img = base_img
    endif else begin
        min = prefs.scale_pos_min
        max = prefs.scale_pos_max
        img = base_img < max
        img = base_img > min
    endelse
endif else begin

    ;if prefs.scale_anat_min ne prefs.scale_anat_max then begin
    ;    min = prefs.scale_anat_min
    ;    max = prefs.scale_anat_max
    ;endif else begin
    ;    min = min(base_img,MAX=max)
    ;endelse
    ;START131016
    if prefs.anat_local_global eq 1 and prefs.scale_anat_min ne prefs.scale_anat_max then begin
        min = prefs.scale_anat_min
        max = prefs.scale_anat_max
    endif else begin
        min = min(base_img,MAX=max)
    endelse


    img = base_img
endelse
if max ne min then begin

    ;START131016
    ;bmin = min(base_img,MAX=bmax)

    scl = (!LEN_COLORTAB1-1)/(bmax - bmin)
    if prefs.anat_local_global eq 0 then begin
        max = max < bmax
        min = min > bmin
    endif
    scl1 = (!LEN_COLORTAB1-1)/(max - min)
endif else begin
    img = fltarr(cell.xdim,cell.ydim)
endelse
if sprf.slice_type eq !STATISTICAL then begin
    case prefs.lc_act_type of
        !POSITIVE_ACTIVATIONS: begin
            img = img*(img gt 0)
        end
        !NEGATIVE_ACTIVATIONS: begin
            img = -img*(img lt 0)
        end
        !ALL_ACTIVATIONS: 
        else: print,'Invalid value of lc_act_type in display_view.'
    endcase
endif
dummy_img = zoom_by_2n(base_img,prefs.zoom,!BILINEAR)
if ptr_valid(cell.overlay_image) then begin
    case display_mode of
        !OVERLAY_MODE: begin
            img = (1-mask_img)*(scl1*((zoom_by_2n(base_img,prefs.zoom,!BILINEAR) < max) - min) > 0.) + mask_img*overlay_img
            sdy = 0
            ydim = cell.ydim
        end
        !ANAT_OVER_ACT_MODE: begin
            img = zoom_by_2n(img,prefs.zoom,!BILINEAR)
            img1 = fltarr(prefs.zoom*cell.xdim,2*prefs.zoom*cell.ydim)
            img1[*,0:prefs.zoom*cell.ydim-1] = img
            img1[*,prefs.zoom*cell.ydim:2*prefs.zoom*cell.ydim-1] = overlay_img
            img = img1
            sdy = 0
            ydim =  2*cell.ydim
        end
        else: print,'Invalid value of display_mode in reload.'
    endcase
endif else begin

    ;if prefs.zoom gt 1 then begin
    ;    img = scl1*((zoom_by_2n(base_img,prefs.zoom,!BILINEAR) < max) - min) > 0.
    ;endif
    ;START131016
    if prefs.zoom gt 1 then $
        img = scl1*((zoom_by_2n(base_img,prefs.zoom,!BILINEAR) < max) - min) > 0. $
    else $
        img = scl1*((base_img < max) - min) > 0.
    

    sdy = 0
    ydim = cell.ydim
endelse
if(prf.orientation eq !NEURO) and (cell.view ne !SAGITTAL) then img = reverse(img,1)
if(vw.id[vw.cw] lt 0) and (lc_array eq !FALSE) then begin
    win.xdim_draw = prf.xdim_draw
    win.ydim_draw = prf.ydim_draw
    create_new_window,vw
    if prf.black_background eq !FALSE then begin
        background_img = bytarr(win.xdim_draw,win.ydim_draw,/NOZERO)
        background_img[*,*] = !WHITE
        tv,background_img
    endif
endif
if(prf.orientation eq !NEURO) and (cell.view ne !SAGITTAL) then begin
    dummy_img = reverse(dummy_img,1)
    anat_mask = reverse(anat_mask,1)
endif
mask = zoom_by_2n(anat_mask,prefs.zoom,!NEAREST_NEIGHBOR)
index = where(dummy_img lt prefs.crop_anat,count)
if count ne 0 then mask[index]=1
if prf.black_background_mask eq !FALSE then color=!WHITE else color=!BLACK 
img = (1-mask)*img + mask*color
text_color = win.prf.label_color+!LEN_COLORTAB
if lc_array eq !FALSE then begin
    tv,img,cell.x,ydim_draw-cell.y-prefs.zoom*ydim,/ORDER
    ;SLICE LABELS ARE PUT ON HERE slc_lab
    for i=0,nlabs-1 do begin
        x = cell.x + prf.sx
        y = ydim_draw - cell.y - prefs.zoom*ydim + prf.sy
        device,SET_FONT=labs[i].font
        xyouts,x,y,labs[i].s,/DEVICE,COLOR=text_color,CHARSIZE=2
    endfor
    device,SET_FONT=vw.fnt.current_font
endif else begin
    if n_elements(output_array) le 1 then output_array = fltarr(prefs.zoom*cell.xdim,prefs.zoom*cell.ydim,slicer_zdim)
    output_array[*,*,slicer_z] = float(img) > 0
endelse
check_math_errors,!FALSE
;print,'display_view bottom'
end

;************************************
pro plot_symbol,x,y,symbol,color,size
;************************************
plots,x,y,PSYM=symbol,COLOR=color,/DEVICE,SYMSIZE=size
end

;******************************
pro update_text,fi,help,vw,win1
;******************************
common view_images_comm_shared
common view_images_comm

;if elf eq 0 then $
;    str = 'Elf is off. Blobs accurate.' $
;else $
;    str = 'Elf is on. Blobs inaccurate.'
;widget_control,wd_elf_status,SET_VALUE=str

if(win1.image_idx[win1.idxidx] ge 0) then $
    str = 'Base: ' + fi.tails[win1.image_idx[win1.idxidx]] $
else $
    str = 'Base: None'
widget_control,wd_base_file,SET_VALUE=str
if(win1.over_idx[win1.idxidx] ge 0) then $
    str = 'Overlay: ' + fi.tails[win1.over_idx[win1.idxidx]] $
else $
    str = 'Overlay: None'
widget_control,wd_over_file,SET_VALUE=str


if(sprf.sup_to_inf) then begin
    first_slice = sprf.first_slice > sprf.last_slice
    last_slice = sprf.first_slice < sprf.last_slice
endif else begin
    first_slice = sprf.first_slice < sprf.last_slice
    last_slice = sprf.first_slice > sprf.last_slice
endelse
slice_incr = abs(sprf.slice_incr)
case win1.sprf[win1.idxidx].view of
    !TRANSVERSE: begin
        str = 'View: Transverse'
        if(sprf.sup_to_inf eq !TRUE) then begin
            order_str = 'Slice order: Superior to inferior'
            first_str = 'Most superior'
            last_str = 'Most inferior'
        endif else begin
            first_str = 'Most inferior'
            last_str = 'Most superior'
            order_str = 'Slice order: Inferior to superior'
        endelse
    end
    !SAGITTAL: begin
        str = 'View: Sagittal'
        if(sprf.sup_to_inf eq !TRUE) then begin
            order_str = 'Slice order: Right to left'
            first_str = 'Rightmost'
            last_str = 'Leftmost'
        endif else begin
            order_str = 'Slice order: Left to right'
            first_str = 'Leftmost'
            last_str = 'Rightmost'
        endelse
    end
    !CORONAL: begin
        str = 'View: Coronal'
        if(sprf.sup_to_inf eq !TRUE) then begin
            order_str = 'Slice order: Anterior to posterior'
            first_str = 'Most anterior'
            last_str = 'Most posterior'
        endif else begin
            order_str = 'Slice order: Posterior to anterior'
            first_str = 'Most posterior'
            last_str = 'Most anterior'
        endelse
    end
    else: print,'Invalid win1.view'
endcase
if(sprf.slice_not_z) then begin
    first_str = strcompress(string(first_str,first_slice,FORMAT='(" slice ",a,i3)'))
    last_str = strcompress(string(last_str,last_slice,FORMAT='(" slice ",a,i3)'))
    incr_str = strcompress(string(slice_incr,FORMAT='("Slice increment ",a,i3)'))
endif else begin
    case win1.sprf[win1.idxidx].view of
        !TRANSVERSE: begin
            first_str = strcompress(string(first_str,first_slice,FORMAT='(a," Z coordinate: ",i3," mm")'))
            last_str = strcompress(string(last_str,last_slice,FORMAT='(a," Z coordinate: ",i3," mm")'))
            incr_str = strcompress(string(slice_incr,FORMAT='("Z increment: ",i3," mm")'))
        end
        !SAGITTAL: begin
            first_str = strcompress(string(first_str,first_slice,FORMAT='(a," Y coordinate: ",i3," mm")'))
            last_str = strcompress(string(last_str,last_slice,FORMAT='(a," Y coordinate: ",i3," mm")'))
            incr_str = strcompress(string(slice_incr,FORMAT='("Y increment: ",i3," mm")'))
        end
        !CORONAL: begin
            first_str = strcompress(string(first_str,first_slice,FORMAT='(a," X coordinate: ",i3," mm")'))
            last_str = strcompress(string(last_str,last_slice,FORMAT='(a," X coordinate: ",i3," mm")'))
            incr_str = strcompress(string(slice_incr,FORMAT='("X increment: ",i3," mm")'))
        end
        else: print,'Invalid win1.view'
    endcase
endelse
widget_control,wd_first,SET_VALUE=first_str
widget_control,wd_last,SET_VALUE=last_str
widget_control,wd_increment,SET_VALUE=incr_str
widget_control,wd_view,SET_VALUE=str
widget_control,wd_slice_order,SET_VALUE=order_str
if(prf.orientation eq !NEURO) then $
    str = 'Transverse view orientation: Neuropsych' $
else $
    str = 'Transverse view orientation: Radiological'
widget_control,wd_orient,SET_VALUE=str
if(win1.sprf[win1.idxidx].slice_type eq !ANATOMIC) then $
    widget_control,wd_slice_type,SET_VALUE='Slice type: Anatomic' $
else $
    widget_control,wd_slice_type,SET_VALUE='Slice type: Statistical'
if(win1.sprf[win1.idxidx].view eq !SAGITTAL) then begin
    if(win1.sprf[win1.idxidx].sagittal_face_left eq !TRUE) then begin
        widget_control,wd_sag_face,SET_VALUE='Sagittals face left'
    endif else begin
        widget_control,wd_sag_face,SET_VALUE='Sagittals face right'
    endelse
endif else begin
    widget_control,wd_sag_face,SET_VALUE='                   '
endelse
widget_control,wd_preference_file,SET_VALUE='Preference file: ' + preference_file
widget_control,wd_wdw_name,SET_VALUE='Current window: ' + win1.name

return
end

;***************************************************************
pro create_view_color_scale,win,prf,fnt,HORIZONTAL=horizontal,cc
;***************************************************************
;print,'create_view_color_scale top'
win.cell[cc].view = !VIEW_COLOR_SCALE
win.cell[cc].xpad = 0
win.cell[cc].ypad = 0
win.cell[cc].valid = !TRUE
win.cell[cc].slice = 0
win.cell[cc].paint = !TRUE
win.cell[cc].image_set = 1000
labels = replicate({View_label},!NUM_FILES)
labels[0].font = fnt.current_font
if ptr_valid(win.cell[cc].labels) then ptr_free,win.cell[cc].labels
win.cell[cc].labels = ptr_new(labels)
if((prf.color_x gt 0) or (prf.color_y gt 0)) then lc_origin_pref = !TRUE else lc_origin_pref = !FALSE

;if win.over_idx[win.num_idx] ne -1 then $ 
;START130311
if win.over_idx[win.idxidx] ne -1 then $ 

    scrap = bytscl(indgen(!COLSCL_YSZ),TOP=!LEN_COLORTAB2-1) $
else $
    scrap = bytscl(indgen(!COLSCL_YSZ),TOP=!LEN_COLORTAB1-1)
if win.nreg[win.idxidx] gt 0 then begin
    if ptr_valid(win.reg[win.idxidx].checked) then begin
        index = where(*win.reg[win.idxidx].checked gt 0,count)
        if count gt 0 then begin
            if ptr_valid(win.reg[win.idxidx].color_index) then begin
                top = count - 1
                scrap = bytscl(indgen(!COLSCL_YSZ),TOP=top)
                scrap1 = scrap
                color_index = byte(*win.reg[win.idxidx].color_index)
                for i=0,top do begin
                    index = where(scrap1 eq i,count)
                    scrap[index] = color_index[i]
                endfor
            endif
        endif
    endif
endif
if keyword_set(HORIZONTAL) then begin
    scale = scrap # make_array(!COLSCL_WIDTH,/BYTE,VALUE=1)

    ;if win.over_idx[win.num_idx] ne -1 then scale = scale + !LEN_COLORTAB1 
    ;START130311
    if win.over_idx[win.idxidx] ne -1 then scale = scale + !LEN_COLORTAB1 

    win.cell[cc].base_image = ptr_new(scale)
    win.cell[cc].xdim = !COLSCL_YSZ + 2*!COLSCL_YOFF
    win.cell[cc].ydim = 4*!COLSCL_WIDTH
    if lc_origin_pref eq !TRUE then $
        win.cell[cc].x = prf.color_x $
    else $
        win.cell[cc].x = win.xdim_draw - !COLSCL_YSZ - 2*!COLSCL_YOFF
endif else begin
    scale = make_array(!COLSCL_WIDTH,/BYTE,VALUE=1) # scrap

    ;if win.over_idx[win.num_idx] ne -1 then scale = scale + !LEN_COLORTAB1
    ;START130311
    if win.over_idx[win.idxidx] ne -1 then scale = scale + !LEN_COLORTAB1

    win.cell[cc].base_image = ptr_new(scale)
    win.cell[cc].xdim = 4*!COLSCL_WIDTH
    win.cell[cc].ydim = !COLSCL_YSZ + 2*!COLSCL_YOFF
    if lc_origin_pref eq !TRUE then $
        win.cell[cc].x = prf.color_x $
    else $
        win.cell[cc].x = win.xdim_draw - 4*!COLSCL_WIDTH
endelse
win.translation_table[win.ntrans] = cc
win.ncell = win.ncell + 1
win.ntrans = win.ntrans + 1
if lc_origin_pref eq !TRUE then begin
    if prf.color_y lt 0 or prf.color_y gt win.ydim_draw then $
        win.cell[cc].y = 0 $
    else $
        win.cell[cc].y = prf.color_y
endif else begin 
    win.cell[cc].y = 0
endelse
end
;*******************************
pro create_new_view_window,vw,fi
;*******************************
common view_images_comm_shared
common view_images_comm
vw.cw = vw.num_windows
vw.num_windows = vw.num_windows + 1
home = getenv('HOME')
file = home + '/.view25d4'
find = findfile(file)
if find(0) eq '' then begin
    sprf = {View_slice_pref}
    sprf.sup_to_inf = !TRUE
    sprf.slice_not_z = !FALSE
    sprf.sagittal_face_left = !TRUE
    sprf.first_slice = 20
    sprf.last_slice = 40
    sprf.slice_incr = 2
    sprf.slice_type = !ANATOMIC
    sprf.mask = !TRUE
    prf = {View_window_pref}
    prf.xdim_draw = !VIEW_XDIM_DRAW
    prf.ydim_draw = !VIEW_YDIM_DRAW
    prf.xgap = 10
    prf.ygap = 10
    prf.transverse_width = !TRANSVERSE_WIDTH_DFLT
    prf.transverse_height = !TRANSVERSE_HEIGHT_DFLT
    prf.sagittal_width = !SAGITTAL_WIDTH_DFLT 
    prf.sagittal_height = !SAGITTAL_HEIGHT_DFLT 
    prf.coronal_width = !CORONAL_WIDTH_DFLT 
    prf.coronal_height = !CORONAL_HEIGHT_DFLT 
    prf.sx = 0L
    prf.sy = 0L
    prf.zoom = 2
    prf.overlay_zoom_type = !BILINEAR
    prf.lc_auto_scale = !TRUE
    prf.scale_pos_min = 3
    prf.scale_pos_max = 10
    prf.lc_act_type = !ALL_ACTIVATIONS
    prf.points_shape = 1 
    prf.points_color = 2 ;red 
    prf.points_size = 1.
    prf.label_color = 1
endif else begin
    sprf = {View_slice_pref}
    prf = {View_window_pref}
    read_view_pref,prf,sprf,file,pref_path,display_mode,reg,/DEFAULTS
    preference_file = 'User default'
    fi.color_scale1[!NUM_FILES] = prf.color_table1
    fi.color_scale2[!NUM_FILES] = prf.color_table2
endelse
atlas_space = win.atlas_space[0]
win = {View_window}
win.insertion_point = 0
win.prf = prf
win.sprf[0] = sprf
win.image_idx[*] = -1
win.over_idx[*] = -1
win.mask_idx[*] = -1
win.atlas_space[*] = atlas_space
win.xdim_draw = prf.xdim_draw
win.ydim_draw = prf.ydim_draw
create_new_window,vw
case prf.color_bar_type of
    !HORIZ_COLOR_BAR: begin
        create_view_color_scale,win,prf,vw.fnt,win.ncell,/HORIZONTAL
        widget_control,wd_hidecolors,/SENSITIVE
        widget_control,wd_showcolors_vert,SENSITIVE=0
        widget_control,wd_showcolors_horiz,SENSITIVE=0
        win.color_bar = !TRUE
    end
    !VERT_COLOR_BAR: begin
        create_view_color_scale,win,prf,vw.fnt,win.ncell
        widget_control,wd_hidecolors,/SENSITIVE
        widget_control,wd_showcolors_vert,SENSITIVE=0
        widget_control,wd_showcolors_horiz,SENSITIVE=0
        win.color_bar = !TRUE
    end
    !NO_COLOR_BAR: win.color_bar = !FALSE
    else: print,'Invalid value of color_bar_type.'
endcase


;START130311
;current_label = 0
;win.nlab = 1
;labels = replicate({View_label},!NUM_FILES)
;labels[*].s = '                                                                               '
;labels[*].font = vw.fnt.current_font
;win.labels = ptr_new(set_labels(labels,win.prf.label_color))
;START130311
init_labels,vw


vw.win[vw.cw] = ptr_new(win)
end

pro hard_defaults,prf,sprf
    prf.xdim_draw = !VIEW_XDIM_DRAW
    prf.ydim_draw = !VIEW_YDIM_DRAW
    prf.xgap = 10
    prf.ygap = 20
    prf.transverse_width = !TRANSVERSE_WIDTH_DFLT
    prf.transverse_height = !TRANSVERSE_HEIGHT_DFLT
    prf.sagittal_width = !SAGITTAL_WIDTH_DFLT
    prf.sagittal_height = !SAGITTAL_HEIGHT_DFLT
    prf.coronal_width = !CORONAL_WIDTH_DFLT
    prf.coronal_height = !CORONAL_HEIGHT_DFLT
    prf.sx = 40L
    prf.sy = -20L
    sprf.view = !TRANSVERSE
    sprf.sup_to_inf = !TRUE
    sprf.slice_not_z = !FALSE
    sprf.sagittal_face_left = !TRUE
    sprf.first_slice = 20
    sprf.last_slice = 40
    sprf.slice_incr = 2
    sprf.slice_type = !ANATOMIC
    sprf.mask = !TRUE
    prf.zoom = 2
    prf.overlay_zoom_type = !BILINEAR
    prf.lc_auto_scale = !TRUE
    prf.scale_pos_min = 3
    prf.scale_pos_max = 10
    prf.scale_neg_min = -3
    prf.scale_neg_max = -10
    prf.lc_act_type = !POSITIVE_ACTIVATIONS
    prf.printer_name = 'tweetie'
    prf.print_color = !TRUE
    prf.print_orientation = !PRINT_LANDSCAPE
    prf.print_x0 = .5
    prf.print_y0 = .5
    prf.print_xdim = 10
    prf.print_ydim = 7.5
    prf.orientation = !NEURO
    prf.color_min1 = 0
    prf.color_max1 = 100
    prf.color_gamma1 = -2.
    prf.color_min2 = 0
    prf.color_max2 = 100
    prf.color_gamma2 = 0.
    prf.black_background = !TRUE
    prf.black_background_mask = !FALSE
    prf.scale_anat_min = 500
    prf.scale_anat_max = 1300
    prf.anat_local_global = 0
    prf.crop_anat = 0.
end
pro wd_slice_def_draw_guts,sprfptr
    common stats_comm
    common view_images_comm_shared
    common view_images_comm
    win.sprf[win.idxidx] = *sprfptr
    *vw.win[vw.cw] = win
    sprf = win.sprf[win.idxidx]
    if win.ntrans gt 0 then begin
        virtual_cells = where(win.cell[win.translation_table[0:win.ntrans-1]].freeze eq !FALSE,count)
        if count ne 0 then begin
            image_sets = win.cell[win.translation_table[virtual_cells]].image_set
            delete_cells,prf,win,image_sets,wd_data_set_status,virtual_cells,lc_delete_cells
        endif
    endif
    update_text,fi,help,vw,win
    reslice_view,fi,vw,stc,/SHOW
    reload,fi,dsp,vw
    if win.idxidxcur ne win.idxidx then win.idxidxcur = win.idxidx
    lcview = sprf.view
    lc_color_bar_once = !FALSE
    view_images_refresh,fi,dsp,vw.fnt
end
;**************
pro view_images
;**************
common stats_comm
common view_images_comm_shared
common view_images_comm

;START130315
base_path = getenv('PWD')
over_path = getenv('PWD')

zoom = 2
overlay_zoom_type = !BILINEAR
vw = {View_images}
current_cells = -1
moved_cells = lonarr(!MAX_CELLS)
lc_depress = !FALSE
show_outlines = !FALSE
lc_add_label_file = !FALSE

;maskfi = make_array(4,/INTEGER,VALUE=-1)
;START180118
maskfi = make_array(7,/INTEGER,VALUE=-1)

xxm1 = 0
yym1 = 0
;;;!P.FONT = 1
;;;device,SET_FONT='Helvetica Bold',/TT_FONT
!P.FONT = 1
;help,!D,/STRUCTURE
if !D.WINDOW gt -1 then $
    device,SET_FONT='Helvetica Bold',/TT_FONT ;This command yields the window of death if an image has not been loaded in fidl.
wd_load_pref_list = 0
dragm1 = !FALSE
cwm1 = -1
vw.used_name[*] = !FALSE
 print_file_name = fi.printfile
lc_add_label = !FALSE
lc_refreshed = !FALSE
fi.color_scale1[!NUM_FILES] = !OVERLAY_SCALE
fi.color_scale2[!NUM_FILES] = !DIFF_SCALE
fi.current_colors = !NUM_FILES
wdw_names = ['Jerry','Bobby','Phil','Billy','Mickey','Brent','Mick','Keith','John','Paul']
;last_file_string = ''
lc_add_mark = !FALSE
home = getenv('HOME')
pref_path = home
file = home + '/.view25d4'
find = findfile(file)
if(find(0) eq '') then begin
    prf = {View_window_pref}
    sprf = {View_slice_pref}
    hard_defaults,prf,sprf
    preference_file = 'Hard-coded defaults'
endif else begin
    prf = {View_window_pref}
    sprf = {View_slice_pref}
    read_view_pref,prf,sprf,file,pref_path,display_mode,reg,/DEFAULTS
    fi.color_scale1[!NUM_FILES] = prf.color_table1
    fi.color_scale2[!NUM_FILES] = prf.color_table2
    preference_file = 'Default'
endelse
win = {View_window}
win.insertion_point = 0
win.prf = prf
win.sprf[0] = sprf
 
vw.view_leader = widget_base(TITLE='View Images',/COLUMN)
menu_base = widget_base(vw.view_leader,/ROW)
font_base = widget_base(vw.view_leader,/ROW)
slider_base = widget_base(vw.view_leader,/ROW)
info_base = widget_base(vw.view_leader,/ROW)
img_base = widget_base(info_base,/COLUMN)
output_base = widget_base(vw.view_leader,/COLUMN)

wd_file = widget_button(menu_base,value='File',MENU=3)
wd_edit = widget_button(menu_base,value='Edit',MENU=3)
m_data = widget_button(menu_base,value='Data',MENU=3)
m_format = widget_button(menu_base,value='Format',MENU=3)
m_disp = widget_button(menu_base,value='Display',MENU=3)
m_label = widget_button(menu_base,value='Label',MENU=3)
;m_elf = widget_button(menu_base,value='Elf',MENU=3)

font_families = !VIEW_FONT_FAMILIES
found = !FALSE
for i=0,n_elements(font_families)-1 do begin
y = strpos(font_families[i],prf.font_family)
    if((strpos(font_families[i],prf.font_family) ge 0) and (prf.font_family ne '')) then $
         found = !TRUE
endfor
if(found eq !TRUE) then begin
    vw.fnt.family = prf.font_family
    vw.fnt.size = prf.font_size
    vw.fnt.bold = prf.font_bold
    vw.fnt.italic = prf.font_italic
    vw.fnt.current_font_number = prf.font_size
endif else begin
    vw.fnt.family = 'helvetica'
    vw.fnt.size = 2
    vw.fnt.bold = 0
    vw.fnt.italic = 0
    vw.fnt.current_font_number = prf.font_size
endelse
get_font_list,vw.fnt,font_sizes,font_names,nfonts
font_sizes = string(long(font_sizes)/10)

print,'vw.fnt.current_font_number=',vw.fnt.current_font_number
if vw.fnt.current_font_number eq -1 then vw.fnt.current_font_number=0

vw.fnt.current_font = font_names[vw.fnt.current_font_number]
wd_font_name = widget_droplist(font_base,VALUE=font_families,/DYNAMIC_RESIZE)
wd_font_size = widget_droplist(font_base,VALUE=font_sizes,/DYNAMIC_RESIZE)
wd_font_button_base = widget_base(font_base,/ROW,/NONEXCLUSIVE)
wd_font_weight = widget_button(wd_font_button_base,VALUE='Bold')
wd_font_slant = widget_button(wd_font_button_base,VALUE='Italic')
wd_plot = widget_button(font_base,VALUE='Plot')
;;;wd_cut = widget_button(wd_edit,value='Cut')
wd_copy = widget_button(wd_edit,value='Copy')
wd_paste = widget_button(wd_edit,value='Paste')
wd_delete_cell = widget_button(wd_edit,VALUE='Delete selected cells')
wd_freeze_cell = widget_button(wd_edit,VALUE='Freeze selected cells')
wd_load = widget_button(wd_file,value='Load image set')
m_tiff = widget_button(wd_file,value='Save to tiff',MENU=3)
wd_tiff = widget_button(m_tiff,value='All')
wd_tiff_selected = widget_button(m_tiff,value='Selected cells')

;START120806
wd_tiff_all = widget_button(m_tiff,value='All loaded image sets')

wd_print_setup = widget_button(wd_file,value='Print setup...',/SEPARATOR)
m_print = widget_button(wd_file,value='Print',MENU=3)
wd_print_all = widget_button(m_print,value='All')
wd_print_selected = widget_button(m_print,value='Selected cells')
wd_print_file = widget_button(m_print,value='Select file name')
m_load_pref = widget_button(wd_file,value='Load preferences',MENU=3,/SEPARATOR)
wd_load_pref = widget_button(m_load_pref,value='All')
wd_load_pref2 = widget_button(m_load_pref,value='All (image sets)')
wd_load_slice_pref = widget_button(m_load_pref,value='Slice specification')
wd_load_user_pref = widget_button(m_load_pref,value='User defaults')
wd_load_def_pref = widget_button(m_load_pref,value='Hard-coded defaults')
m_save_pref = widget_button(wd_file,value='Save preferences',MENU=3)
wd_save_pref = widget_button(m_save_pref,value='To named file')
wd_save_def_pref = widget_button(m_save_pref,value='To user default file')
wd_save_pref_path = widget_button(m_save_pref,value='Save preference path')
wd_exit = widget_button(wd_file,value='Exit',FRAME=5)

pref_files = findfile(pref_path+'*.v25d')
nfiles = n_elements(pref_files)
if(pref_files[0] eq '') then $
    nfiles = 0
if(nfiles gt 0) then begin
    labels = strarr(nfiles+1)
    labels[0] = '1\Slice Prefs'
    for i=1,nfiles do begin
        pos = rstrpos(pref_files[i-1],'/') + 1
        len = strlen(pref_files[i-1])
        name = strmid(pref_files[i-1],pos,len-pos+1)
        labels[i] = string(name,FORMAT='("0\",a)')
    endfor
    wd_load_pref_list = cw_pdmenu(menu_base,labels,/RETURN_INDEX)
endif

wd_slice_def_draw = widget_button(m_data,VALUE='Define, reslice and draw slices...')
;wd_slice_def = widget_button(m_data,VALUE='Define and reslice...')
wd_slice_def = 0
wd_delete_image_set = widget_button(m_data,VALUE='Delete current image set')
wd_delete_all = widget_button(m_data,VALUE='Delete all image sets')

wd_format = widget_button(m_format,VALUE='Format Screen...')
wd_scale_zoom = widget_button(m_format,VALUE='Magnification and scale...')
m_mode = widget_button(m_format,value='Display mode',MENU=3)
wd_overlay = widget_button(m_mode,value='Overlay')
wd_top_bot = widget_button(m_mode,value='Anatomy over activaton')
widget_control,wd_overlay,SENSITIVE=0
display_mode = !OVERLAY_MODE
m_outlines = widget_button(m_format,value='Outlines',MENU=3)
wd_outlines = widget_button(m_outlines,value='Show')
wd_no_outlines = widget_button(m_outlines,value='Hide')
widget_control,wd_no_outlines,SENSITIVE=0
widget_control,wd_outlines,SENSITIVE=0
;wd_points = widget_button(m_format,VALUE='Format points')

wd_reload = widget_button(m_disp,value='Default layout')
wd_refresh = widget_button(m_disp,value='Refresh')
wd_new_window = widget_button(m_disp,value='New window')
wd_insert_set = widget_button(m_disp,VALUE='Insert current set',/SEPARATOR)
wd_append_set = widget_button(m_disp,VALUE='Append current set')
wd_clear = widget_button(m_disp,VALUE='Erase Screen',/SEPARATOR)
wd_erase_set = widget_button(m_disp,VALUE='Erase current set')
wd_erase_selected = widget_button(m_disp,VALUE='Erase selected cells')
wd_deselect = widget_button(m_disp,VALUE='Deselect cell',/SEPARATOR)
wd_colortables = widget_button(m_disp,VALUE='Modify color tables',/SEPARATOR)
wd_showcolors_vert = widget_button(m_disp,VALUE='Show vertical color scale')
wd_showcolors_horiz = widget_button(m_disp,VALUE='Show horizontal color scale')
wd_hidecolors = widget_button(m_disp,VALUE='Hide color scale')
;wd_slicer = widget_button(m_disp,VALUE='Volume display')
wd_slicer = 0 
widget_control,wd_hidecolors,SENSITIVE=0
wd_af3d_loci = widget_button(m_disp,VALUE='Plot atlas points',/SEPARATOR)
wd_reg = widget_button(m_disp,VALUE='Plot regions/Assign colors to integer valued blobs')
wd_reg_ass = widget_button(m_disp,VALUE='Assign values to regions')

wd_add_label = widget_button(m_label,value='Add')
wd_add_label_file = widget_button(m_label,value='Add filename label')
wd_add_mark = widget_button(m_label,value='Mark designated points')
wd_delete_label = widget_button(m_label,value='Delete')
wd_edit_label = widget_button(m_label,value='Edit')
wd_label_font = widget_button(m_label,value='Update fonts')
wd_label_color = widget_button(m_label,value='Select color')
wd_points = widget_button(m_label,VALUE='Format points',/SEPARATOR)
;wd_load_points =  widget_button(m_label,value='Load points from af3d or text file')
;wd_plot_points =  widget_button(m_label,value='Plot points')
;wd_clear_points =  widget_button(m_label,value='Clear points')

wd_idxidx = widget_slider(slider_base,Title='Image set',SCROLL=1,FRAME=5,XSIZE=450,MINIMUM=1,SENSITIVE=0)
wd_base_file = widget_label(img_base,VALUE='Base: ',/ALIGN_LEFT,XSIZE=450)
wd_over_file = widget_label(img_base,VALUE='Overlay: ',/ALIGN_LEFT,XSIZE=450)
wd_view = widget_label(img_base,VALUE='View: Transverse',/ALIGN_LEFT,XSIZE=450)
wd_slice_order = widget_label(img_base,VALUE='Slice order: ',/ALIGN_LEFT,XSIZE=450)
wd_orient = widget_label(img_base,VALUE='Orientation: Neuropsych',/ALIGN_LEFT,XSIZE=450)
wd_first = widget_label(img_base,VALUE='First slice: ',/ALIGN_LEFT,XSIZE=450)
wd_last = widget_label(img_base,VALUE='Last slice: ',/ALIGN_LEFT,XSIZE=450)
wd_increment = widget_label(img_base,VALUE='Slice increment: ',/ALIGN_LEFT,XSIZE=450)
wd_slice_type = widget_label(img_base,VALUE='Slice type: ',/ALIGN_LEFT,XSIZE=450)
wd_sag_face = widget_label(img_base,VALUE='               ',/ALIGN_LEFT,XSIZE=450)
wd_data_set_status = widget_label(img_base,VALUE='Status: Unsliced',/ALIGN_LEFT,XSIZE=450)
wd_preference_file = widget_label(img_base,VALUE='Preference file: Default',/ALIGN_LEFT,XSIZE=450)
wd_wdw_name = widget_label(img_base,VALUE='Current window: ',/ALIGN_LEFT,XSIZE=450)
;wd_file_name = widget_label(img_base,VALUE='Current file: ',/ALIGN_LEFT,XSIZE=450)
wd_coord = cw_field(output_base,TITLE='Atlas',VALUE='',/RETURN_EVENTS,XSIZE=50)
wd_regname = cw_field(output_base,TITLE='Region',VALUE='',/RETURN_EVENTS,XSIZE=48)

win.xdim_draw = prf.xdim_draw
win.ydim_draw = prf.ydim_draw
win.name = 'Harry'
win.color_bar = !FALSE
win.black_background = !TRUE
win.black_background_mask = !FALSE
win.naf3dpts = 0

widget_control,vw.view_leader,/REALIZE
xmanager,'view_images',vw.view_leader
vw.id[0] = -1

init = fltarr(6)
init[0] = prf.color_min1
init[1] = prf.color_max1
init[2] = prf.color_gamma1
init[3] = prf.color_min2
init[4] = prf.color_max2
init[5] = prf.color_gamma2
view_images_lct,fi.color_scale1[!NUM_FILES],fi.color_scale2[!NUM_FILES],INIT=init

nfam = 0
for i=0,n_elements(font_families)-1 do begin
    if(font_families[i] eq vw.fnt.family) then $
        nfam = i
endfor
widget_control,wd_font_name,SET_DROPLIST_SELECT=nfam
widget_control,wd_font_size,SET_DROPLIST_SELECT=vw.fnt.current_font_number

lc_color_bar_once = !FALSE
lc_delete_cells = !TRUE

;START130311
;vw.win[vw.cw] = ptr_new(win)
;vw.num_windows = 1

get_atlas_labels,atlaslabels,atlasfiles,natlas

cnt = readf_ascii_file(!LINECOLORSFILE)
if cnt.name eq 'ERROR' then begin
    stat=dialog_message('Cannot read '+!LINECOLORSFILE,/ERROR)
    return
end
data = cnt.data
data=reform(data,cnt.NF,cnt.NR,/OVERWRITE) ;NECESSARY if cnt.NR=1
    ;Tried this back in readf_ascii_file, but dimension is truncated after being passed.
    ;Tried sending cnt.dat as ptr_new(cnt.data) but dimension still truncated.
    ;Also need to rename data = cnt.data for dimensions to take.
colornames = reform(data[0,*])
af3dh = { $
    shape:['square','circle','diamond','trianglen','triangles','trianglew','trianglee','plus','x'], $
    psym: [   8    ,   8    ,    8    ,     8     ,     8     ,     8     ,     8     ,  1   , 7 ], $
    color:colornames, $
    fill:['unfilled.m.pm','filled.m.pm'] $
    }


;current_label = 0 
;win.nlab = 1
;labels = replicate({View_label},!NUM_FILES)
;labels[*].s = '                                                                               ' 
;labels[*].font = vw.fnt.current_font
;win.labels = ptr_new(set_labels(labels,win.prf.label_color))
;checked = intarr(!NUM_FILES)
;for i=0,!NUM_FILES-1 do win.label[i].checked = ptr_new(checked)
;START130311
init_labels,vw
vw.win[vw.cw] = ptr_new(win)
vw.num_windows = 1

end
