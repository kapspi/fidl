;Copyright 3/4/04 Washington University.  All Rights Reserved.
;define_single_dsgn_matrix_new2.pro  $Revision: 1.9 $
function get_valid_frms,tdim,nrun,tdim_sum,tdim_file,txtfile,concfile,lc_omit_frames,rtn_get_frames,omit_reference,init_skip, $
    valid_frms_max
    valid_frms = make_array(tdim,/FLOAT,VALUE=1.)
    if lc_omit_frames eq 1 then begin
        if omit_reference eq !DATASET then $
            valid_frms = valid_frms_max[0:tdim-1] $
        else if omit_reference eq !RUN then begin
            for seg=0,rtn_get_frames.nseg-1 do begin
                limits=rtn_get_frames.limits_all[2*seg:2*seg+1]
                t0 = 0
                for file=0,nrun-1 do begin
                    for t=limits[0],limits[1] do valid_frms[t0+t] = 0
                    t0 = t0 + tdim_file[file]
                endfor
            endfor
        endif
    endif
    index = where(init_skip[0:nrun-1] ne 0,count)
    if count ne 0 then for i=0,count-1 do valid_frms[tdim_sum[index[i]]:tdim_sum[index[i]]+init_skip[index[i]]-1]=0
    if txtfile ne '' then begin
        print,'Reading ',txtfile
        rtn=readf_ascii_file(txtfile,/NOHDR)
        if rtn.NR ne tdim then begin
            stat=dialog_message(concfile+string(10B)+' has '+strtrim(tdim,2)+' frames.'+string(10B)+string(10B) $
                +txtfile+string(10B)+' has '+strtrim(rtn.NR,2)+' frames.'+string(10B)+'Must be equal. Abort!',/ERROR)
            return,rtn={msg:'ERROR'}
        endif
        data = reform(fix(rtn.data[0,*]))
        for i=0L,rtn.NR-1 do if data[i] eq 0 then valid_frms[i]=0
    endif
    return,rtn={msg:'OK',valid_frms:valid_frms}
end
function get_ncondfreq,nrun,identify
    which = make_array(nrun,/INTEGER,VALUE=-1)
    ncondfreq = 0
    ncondfreqc = -1
    ncondfreqi = -1
    ncondfreqs = ''
    repeat begin
        index = where(which eq -1,count)
        if count ne 0 then begin
            index2 = where(strmatch(identify,identify[index[0]]) eq 1,count2)
            which[index2] = ncondfreq
            ncondfreq = ncondfreq + 1
            ncondfreqc = [ncondfreqc,count2]
            ncondfreqi = [ncondfreqi,index2]
            ncondfreqs = [ncondfreqs,identify[index[0]]]
        endif
    endrep until count eq 0
    return,rtn={ncondfreq:ncondfreq,ncondfreqc:ncondfreqc[1:*],ncondfreqi:ncondfreqi[1:*],ncondfreqs:ncondfreqs[1:*]}
end
function get_ncondfreq2,ncondfreq,rr_useev_cbyrun,frequseev_idx,effect_label

    ;print,'get_ncondfreq2 top ncondfreq=',ncondfreq
    ;print,'get_ncondfreq2 top frequseev_idx=',frequseev_idx
    ;print,'get_ncondfreq2 top n_elements(frequseev_idx)=',n_elements(frequseev_idx)

    ncondfreqi = -1
    ncondfreqs = ''
    for i=0,ncondfreq-1 do begin
        index2 = where(rr_useev_cbyrun[*,frequseev_idx[i]] eq 1,count)
        if count eq 0 then begin
            scrap = 'count='+strtrim(count,2)+' Should not be zero!'
            print,scrap
            stat=dialog_message(scrap,/ERROR)
            return,rtn={msg:'ERROR'} 
        endif else begin
            ncondfreqi = [ncondfreqi,index2]
            ncondfreqs = [ncondfreqs,effect_label[frequseev_idx[i]]]
        endelse
    endfor
    return,rtn={msg:'OK',ncondfreqi:ncondfreqi[1:*],ncondfreqs:ncondfreqs[1:*]}
end

;function readf_and_check_ascii_file,file,valid_frms,tdim,concfile
;START170118
function readf_and_check_ascii_file,file,valid_frms,tdim,concfile,whoamistr

    widget_control,/HOURGLASS
    er = readf_ascii_file(file)
    if er.NR ne tdim then begin
        stat=dialog_message(concfile+string(10B)+' has '+strtrim(tdim,2)+' frames.'+string(10B)+string(10B) $
            +file+string(10B)+' has '+strtrim(er.NR,2)+' frames.'+string(10B)+'Must be equal. Abort!',/ERROR)
        return,rtn={msg:'ERROR'}
    endif
    junki = where((er.nhdr-er.NF_each_line) ne 0,junkc)
    if junkc ne 0 then begin
        junks = ''
        junki = junki+2
        junkc = junkc-1
        inc = 20
        ji = -1
        repeat begin
            jistart = ji + 1
            ji = (jistart+inc) < junkc
            print,'ji=',ji
            junks = junks + strjoin(strtrim(junki[jistart:ji],2),' ',/SINGLE) + string(10B)
        endrep until ji ge junkc
        print,'Wrong number of columns on lines:'
        print,junks
        if junkc gt 500 then junks = 'See terminal window.'
        junk = get_button(['goback','exit'],BASE_TITLE=whoamistr,TITLE=file+string(10B)+string(10B)+'BIG PROBLEM' $
            +string(10B)+string(10B)+'Wrong number of columns on lines:'+string(10B)+junks)
        if junk eq 0 then msg='GOBACK' else msg='ERROR'
        return,rtn={msg:msg}
    end
    widget_control,/HOURGLASS
    for i=0,er.NR-1 do begin
        index = where(er.data[*,i] eq 'x' or er.data[*,i] eq 'X',count)
        if count ne 0 then begin
            valid_frms[index] = 0
            er.data[index,i] = '0'
        endif
    endfor
    return,rtn={msg:'OK',er:er}
end
function assign_er,er,G,effect_column,effect_length,effect_label,lcfunc,delay,stimlen,nmain,all_eff,tot_eff
    float_er_data = float(er.data)
    if all_eff ne 0 then begin
        G = [[G],[transpose(float_er_data)]]
        tmp1 = effect_column
        tmp2 = effect_length
        tmp3 = effect_label
        nmain = nmain + er.NF
        effect_column = intarr(all_eff+er.NF)
        effect_length = intarr(all_eff+er.NF)
        effect_label = strarr(all_eff+er.NF)
        effect_column[0:all_eff-1] = tmp1
        effect_length[0:all_eff-1] = tmp2
        effect_label[0:all_eff-1] = tmp3
        j = all_eff
        k = effect_column[all_eff-1]+effect_length[all_eff-1]
        for i=0,er.NF-1 do begin
            effect_column[j] = k
            effect_length[j] = 1
            effect_label[j] = er.hdr[i]
            j = j + 1
            k = k + 1
        endfor
        all_eff = all_eff + er.NF
        lcfunc = [lcfunc,intarr(er.NF)]
        delay = [delay,fltarr(er.NF)]
        stimlen = [stimlen,fltarr(er.NF)]
        tot_eff = tot_eff + er.NF
    endif else begin 
        G = transpose(float_er_data)
        nmain = nmain + er.NF
        effect_column = intarr(all_eff+er.NF)
        effect_length = intarr(all_eff+er.NF)
        effect_label = strarr(all_eff+er.NF)
        for i=0,er.NF-1 do begin
            effect_column[i] = i 
            effect_length[i] = 1
            effect_label[i] = er.hdr[i]
        endfor
        all_eff = all_eff + er.NF
        lcfunc = intarr(er.NF)
        delay = fltarr(er.NF)
        stimlen = fltarr(er.NF)
        tot_eff = tot_eff + er.NF
    endelse
    return,rtn={nmain:nmain,all_eff:all_eff,tot_eff:tot_eff}
end
function get_t4s,lc_atlas,t4select,total_nsubjects,concselect,t4_identify_str,atlasspace,atlas_str,neach
    neach = intarr(total_nsubjects)
    if t4select[0] ne '' then $
        neach[*]=1 $
    else begin
        goback1:
        gf = get_files(TITLE='Please enter atlas filter.',FILTER='*anat_ave_to_*_t4')
        if gf.msg eq 'GO_BACK' then return,'GO_BACK' else if gf.msg eq 'EXIT' then return,'EXIT'
        t4select=gf.files
        if t4select[0] ne 'NONE' then begin
            scrap = ''
            neach[*]=0
            for i=0,total_nsubjects-1 do begin
                rtn_gol = get_ordered_list(t4select,indgen(1000)+1,TITLE='Please select t4(s) for '+concselect[i])
                if rtn_gol.count ne 0 then begin
                    list = rtn_gol.labels_sorted
                    scrap = [scrap,list]
                    neach[i] = n_elements(list)
                endif
            endfor
        endif
        t4select = scrap[1:*]
        if total(neach-1) ne 0 then begin
            tneach = total(neach)
            dummy = strarr(tneach)
            k = 0
            for i=0,total_nsubjects-1 do begin
                for j=0,neach[i]-1 do begin
                    dummy[k] = concselect[i] + string(10B) + t4select[k]
                    k = k + 1
                endfor
            endfor
            scrap = make_array(tneach,/STRING,VALUE='0')
            goback1b:
            scrap = get_str(tneach,dummy,scrap,WIDTH=50,TITLE='Please assign runs to t4s. First run is 1.', $
                LABEL='Elements can be separated by spaces, tabs, or commas. ex. 1-10,11-24',/GO_BACK,/ONE_COLUMN,/BELOW)
            if scrap[0] eq 'GO_BACK' then goto,goback1
            k = 0
            for i=0,total_nsubjects-1 do begin
                t4_identify = intarr(1000)
                for j=1,neach[i] do begin
                    segments = strsplit(scrap[k],'[ '+string(9B)+',]',/REGEX,/EXTRACT)
                    nseg = n_elements(segments)
                    for seg=0,nseg-1 do begin
                        limits = long(strsplit(segments[seg],'-',/EXTRACT)) - 1
                        if n_elements(limits) eq 1 then limits = [limits[0],limits[0]]
                        ;print,'limits=',limits,' j=',j
                        t4_identify[limits[0]:limits[1]] = j
                    endfor
                    k = k + 1
                endfor
                index = where(t4_identify,count)
                if count eq 0 then begin
                    stat=dialog_message('You have failed to assign runs from '+concselect[i]+string(10B)+'Please try again.')
                    goto,goback1b
                endif
                t4_identify = strtrim(t4_identify[index],2)
                t4_identify_str[i] = ' -t4'
                for j=0,count-1 do t4_identify_str[i] = t4_identify_str[i] + ' ' + t4_identify[j]
            endfor
        endif
    endelse
    return,''
end
function get_reg,fi,wd,dsp,help,voxwtfile_path,total_nsubjects,rr,ggimgselect,GOBACK=goback,TITLE1=title1
    if keyword_set(GOBACK) then begin
        if goback eq 'goback18c' then goto,goback18c $
        else if goback eq 'goback18c1' then goto,goback18c1
    endif
    goback18a:
    rtn = get_regions(fi,wd,dsp,help,'regressors',ggimgselect)
    if rtn.msg eq 'EXIT' or rtn.msg eq 'GO_BACK' then return,rtn={msg:rtn.msg}
    goback18b:
    if rtn.nreg ne 0 then begin 
        rr.region_str=ptr_new(make_array(total_nsubjects,/STRING,VALUE=' -region_file '+strjoin(rtn.region_file,' ',/SINGLE)))
        if not keyword_set(TITLE1) then title1='Please select regions for regressors.'
        rtn1 = select_files(rtn.region_names,TITLE=title1,/GO_BACK)
        if rtn1.files[0] eq 'GO_BACK' then goto,goback18a
        rr.roi_str = ' -regions_of_interest ' + strjoin(strtrim(rtn1.index+1,2),' ',/SINGLE)
        rr.nreg = rtn1.count
        rr.region_names = ptr_new(rtn1.files)
        rr.index = ptr_new(rtn1.index)
    endif else begin
        rr.region_str=ptr_new(' -mask '+ggimgselect)
        rr.regnamestr = ' -regname '+rtn.region_names
        rr.nreg = 1
        rr.region_names = ptr_new(rtn.region_names)
    endelse
    goback18c:
    lcvoxwt = get_button(['yes','no','go back'],BASE_TITLE='Do you want to weight the individual voxels?',TITLE='Each region ' $
        +'is a separate file.'+string(10B)+'Each row is a voxel.'+string(10B)+'Each column is a regressor and must have a text ' $
        +'label.')
    if lcvoxwt eq 2 then begin
        if rtn.nreg ne 0 then goto,goback18b else goto,goback18a
    endif
    lcvoxwt = abs(lcvoxwt-1)
    voxwt_str=''
    if lcvoxwt eq 1 then begin
        if voxwtfile_path eq '' then voxwtfile_path = fi.path
        goback18c1:
        lcvoxwt=1
        nvoxwtfile = intarr(rr.nreg)
        voxwtfile = ''
        for i=0,rr.nreg-1 do begin
            get_dialog_pickfile,'*.txt',voxwtfile_path,*rr.region_names[i]+' Please load weight file(s).',scrap,nscrap, $
                voxwtfile_path,/MULTIPLE_FILES
            if nscrap eq 0 then goto,goback18c
            nvoxwtfile[i] = nscrap
            voxwtfile = [voxwtfile,scrap]
        endfor
        voxwtfile = voxwtfile[1:*]
        print,'voxwtfile=',voxwtfile
        goback18c2:
        if total_nsubjects eq 1 then begin
            voxwt_str = strjoin(voxwtfile,' ',/SINGLE)
        endif else begin
            index = where(nvoxwtfile eq 1,count)
            if count eq rr.nreg then begin
                voxwt_str[0] = strjoin(voxwtfile,' ',/SINGLE)
                voxwt_str[1:*] = voxwt_str[0]
            endif else begin
                rtn_gr_conc = get_root(concselect,'.conc')
                minl=min(strlen(rtn_gr_conc.file),mini)
                rtn = select_files([strtrim(indgen(minl)+1,2),'vc number'], $
                      TITLE='Fidl will automatically pair conc and voxel weight files.' $
                      +' If you do not want to use this option, select manual pairing below.'+string(10B)+string(10B) $
                      +'Only the root is matched. Path ignored.'+string(10B)+'Ex. '+rtn_gr_conc.file[mini]+string(10B) $
                      +string(10B)+'Match on characters',/GO_BACK,MIN_NUM_TO_SELECT=-1,BASE_TITLE='Match concs to event files', $
                      SPECIAL='Let me do this manually')
                if rtn.files[0] eq 'GO_BACK' then goto,goback18c1
                rtn_special = rtn.special
                if rtn_special eq 1 then begin
                    junk = 0
                    for i=0,rr.nreg-1 do begin
                        scrap1 = voxwtfile[junk:junk+nvoxwtfile[i]-1]
                        scrap2 = match_files(concselect,scrap1,TITLE='Please select event file for ',/GO_BACK)
                        if scrap2[0] eq 'GO_BACK' then goto,goback18c2
                        voxwt_str = voxwt_str + ' ' + scrap2
                        junk = junk + nvoxwtfile[i]
                    endfor
                endif else begin
                    junk = 0
                    for i=0,rr.nreg-1 do begin
                        scrap1 = voxwtfile[junk:junk+nvoxwtfile[i]-1]
                        rtn_gr_scrap1 = get_root(scrap1,'.')
                        if max(rtn.index) eq minl then $
                            rtn_match_vc2=match_vc2(rtn_gr_conc.file,rtn_gr_scrap1.file,concselect,scrap1) $
                        else $
                            rtn_match_vc2=match_vc2(rtn_gr_conc.file,rtn_gr_scrap1.file,concselect,scrap1,SEARCHINDEX=rtn.index)
                        if rtn_match_vc2.msg eq 'ERROR' or rtn_match_vc2.msg eq 'EXIT' then $
                            return,rtn={msg:rtn_match_vc2.msg} $
                        else if rtn_match_vc2.msg eq 'GO_BACK' then $
                            goto,goback18c2
                        scrap2 = rtn_match_vc2.files2
                        voxwt_str = voxwt_str + ' ' + scrap2
                        junk = junk + nvoxwtfile[i]
                    endfor
                    voxwt_str = get_str(total_nsubjects,concselect,voxwt_str,TITLE='Please check voxel weight files.',/ONE_COLUMN, $
                        /BELOW,/GO_BACK)
                    if voxwt_str[0] eq 'GO_BACK' then goto,goback18c2
                endelse
            endelse
        endelse
        scrap=strsplit(voxwt_str[0],/EXTRACT)
        scrap = scrap[0]
        junk = readf_ascii_file(scrap,/HDR_ONLY)
        rtn = select_files([strtrim(indgen(junk.nhdr)+1,2)],TITLE='Please select columns.',/GO_BACK,MIN_NUM_TO_SELECT=1, $
            BASE_TITLE='Read '+scrap)
        if rtn.files[0] eq 'GO_BACK' then goto,goback18c2
        rr.scale_str = rr.scale_str + ' -columns '+strjoin(strtrim(rtn.index+1,2),' ',/SINGLE)
        voxwt_str = ' -voxwt ' + voxwt_str
    endif
    return,rtn={msg:'OK',lcvoxwt:lcvoxwt,voxwt_str:voxwt_str}
end
function get_Grr,stc2,sng,rr_useev_list,max_num_levels,whoamistr
    tdim_sum_new = *stc2.tdim_sum_new
    rr_useev_cbyrun = intarr(stc2.n,max_num_levels)
    times = *sng.times
    conditions = *sng.conditions
    frames = *sng.frames
    j=0
    widget_control,/HOURGLASS
    for trial=0,sng.num_trials-1 do begin
        eff = conditions[trial]
        t0 = frames[trial,0]
        if t0 gt tdim_sum_new[stc2.n-1] then begin
            stat=dialog_message(whoamistr+' You have '+strtrim(tdim_sum_new[stc2.n-1],2)+' frames of data.'+string(10B)+'However, ' $
                +'your next event occurs at frame '+strtrim(t0,2)+'.'+string(10B)+'You are asking me to model events where you ' $
                +'have no data.'+string(10B)+'Please check line '+strtrim(trial+2,2)+' in your event file.'+string(10B)+string(10B) $
                +evselect+string(10B)+concselect,/ERROR)
            return,rtn={msg:'ERROR'}
        endif
        if j lt stc2.n-1 then while times[trial] ge float(tdim_sum_new[j]-1)*sng.TR do j = j + 1
        if times[trial] le float(tdim_sum_new[j]-1)*sng.TR then begin
            if rr_useev_list[0] ne -1 then begin
                if rr_useev_list[eff] eq 1 then rr_useev_cbyrun[j,eff] = 1
            endif
        endif
    endfor
    return,rtn={msg:'OK',rr_useev_cbyrun:rr_useev_cbyrun}
end
function get_func,sng,stc2,pause,    geffect_length,geffect_column,gnmain,lcfunc_save, $
    count_present_ts,index_present_ts,widget_type,durdel,evselect,concselect,frequseev_list,max_num_levels,rr_useev_list,functype, $
    lcusedur,len_hrf,lcround_tc,delay_save,ud,hrf_params,lq_scale_globally,whoamistr
    if sng.name eq 'ERROR' then return,rtn_G={msg:'ERROR'}
    nmain = gnmain
    effect_column = geffect_column
    effect_length = geffect_length
    lcfunc = lcfunc_save
    conditions = *sng.conditions
    stimlen_vs_t = *sng.stimlen_vs_t
    frames = *sng.frames
    stimlenframes_vs_t = *sng.stimlenframes_vs_t
    fraction = *sng.fraction
    for eff=0,count_present_ts-1 do begin
        if widget_type[eff*2] eq 'field' then begin
            index = where(conditions eq index_present_ts[eff],count)
            if count ne 0 then stimlen_vs_t[index] = durdel[eff*2]
        endif
    endfor
    if frames[sng.num_trials-1,0] gt stc2.tdim_all then begin
        title='Your event file is longer than the amount of data.'+string(10B)+string(10B)+evselect+string(10B)+'    ' $
            +trim(frames[sng.num_trials-1,0])+' frames'+string(10B)+concselect+string(10B)+'    '+trim(stc2.tdim_all)+' frames' $
            +string(10B)+string(10B)+'Do you want to keep going?'

        stat = get_button(['yes, skip this one and continue','go back','exit'],BASE_TITLE=whoamistr,TITLE=title[0])
        if stat eq 0 then $
            return,rtn_G={msg:'BOTTOMOFLOOP'} $
        else if stat eq 1 then $
            return,rtn_G={msg:'GO_BACK'} $
        else $
            return,rtn_G={msg:'ERROR'}
    endif
    if pause.lcpause eq 1 then begin ;Move "pause" effect to rightmost columns.
        effect_column[pause.effp] = nmain
        if pause.lcpause_per_run eq 1 then $
            effect_length[pause.effp]=nrun*pause.pause_length $
        else $
            effect_length[pause.effp]=pause.pause_length
        nmain = nmain + effect_length[pause.effp]
        lcfunc[pause.eff_pause] = !TIME_COURSE_EST
        max = max(conditions)
        eff = 0
        conditions_new = intarr(sng.num_trials)
        for effp=0,max_num_levels-1 do begin
            if effp eq pause.eff_pause then begin
                mask = conditions eq pause.eff_pause
                conditions_new = conditions_new + mask*max
            endif else begin
                mask = conditions eq effp
                conditions_new = conditions_new + mask*eff
                eff = eff + 1
            endelse
        endfor
        conditions = conditions_new
        pause.eff_pause = max_num_levels - 1
        undefine,conditions_new
    endif
    len_hrf_vs_t = intarr(sng.num_trials)
    widget_control,/HOURGLASS
    for trial=0,sng.num_trials-1 do begin
        eff = fix(conditions[trial])
        if eff ge 0 then begin
            case functype[eff] of
                !TIME_COURSE_EST: begin
                    if lcusedur[eff] eq 0 then $
                        len_hrf_vs_t[trial] = len_hrf[eff] $
                    else $
                        len_hrf_vs_t[trial] = round(stimlenframes_vs_t[trial])*lcround_tc[eff]
                    ;print,'trial=',trial,' len_hrf_vs_t[trial]=',len_hrf_vs_t[trial]
                    ;print,'    stimlenframes_vs_t[trial]=',stimlenframes_vs_t[trial],' ceil=',ceil(stimlenframes_vs_t[trial]), $
                    ;    ' round=',round(stimlenframes_vs_t[trial])
                end
                !BOYNTON: begin
                    len_hrf_vs_t[trial] = round((stimlen_vs_t[trial] + delay_save[eff] + 20.)/sng.TR)
                end
                !FRISTON: begin
                    len_hrf_vs_t[trial] = round((stimlen_vs_t[trial] + delay_save[eff] + 32.)/sng.TR)
                end
                !SPM_CANNONICAL: begin
                    len_hrf_vs_t[trial] = round((stimlen_vs_t[trial] + delay_save[eff] + 32.)/sng.TR)
                end
                !GAMMA_DERIV: begin
                    len_hrf_vs_t[trial] = round((stimlen_vs_t[trial] + delay_save[eff] + 20.)/sng.TR)
                end
                !BOXCAR_STIM: begin
                    len_hrf_vs_t[trial] = round((stimlen_vs_t[trial]+delay_save[eff])/sng.TR)
                end
                !TREND: begin
                    len_hrf_vs_t[trial] = round((stimlen_vs_t[trial]+delay_save[eff])/sng.TR)
                end
                !USER_DEFINED: begin
                    len_hrf_vs_t[trial] = ud[eff].nhrf
                end
                -1: ;do nothing
                else: print,'Invalid type specified for functype.'
            endcase
        endif
    endfor
    func = fltarr(max(len_hrf_vs_t),max(effect_length),4,sng.num_trials)
    widget_control,/HOURGLASS
    for trial=0,sng.num_trials-1 do begin
        eff = fix(conditions[trial])
        if eff ge 0 then begin
            if functype[eff] ne -1 then begin
                trial_t = fltarr(len_hrf_vs_t[trial])
                for i=0,len_hrf_vs_t[trial]-1 do trial_t[i] = sng.TR*(float(i)-fraction[trial]) - delay_save[eff]
                case functype[eff] of
                    !BOYNTON: begin
                        hrf = fltarr(len_hrf_vs_t[trial])
                        for i=0,len_hrf_vs_t[trial]-1 do begin
                            t = trial_t[i] > 0.
                            hrf[i] = hemo_resp(t,stimlen_vs_t[trial],PARAMETERS=hrf_params[*,eff])
                        endfor
                        if lq_scale_globally eq 'No' then hrf=hrf/max(hrf)
                        for i=0,len_hrf_vs_t[trial]-1 do func[i,0,0,trial] = hrf[i]
                    end
                    !FRISTON: begin
                        for i=0,len_hrf_vs_t[trial]-1 do begin
                            hrf = hemo_friston(trial_t[i])
                            func[i,0,0,trial] = hrf[0]
                            func[i,1,0,trial] = hrf[1]
                            func[i,2,0,trial] = hrf[2]
                            func[i,3,0,trial] = hrf[3]
                            func[i,4,0,trial] = hrf[4]
                            func[i,5,0,trial] = hrf[5]
                        endfor
                        for i=0,5 do begin
                            func[0:len_hrf_vs_t[trial]-1,i,0,trial] = $
                                func[0:len_hrf_vs_t[trial]-1,i,0,trial]/sqrt(total(func[0:len_hrf_vs_t[trial]-1,i,0,trial]^2))
                        endfor
                    end
                    !SPM_CANNONICAL: begin
                        hrf = fltarr(len_hrf_vs_t[trial])
                        for i=0,len_hrf_vs_t[trial]-1 do begin
                            adjusted_frame = trial_t[i]/sng.TR > 0.
                            hrf[i]  = hemo_resp_spm(adjusted_frame,sng.TR,STIMLEN=stimlen_vs_t[trial],DELAY=delay_save[eff])
                        endfor
                        if lq_scale_globally eq 'No' then hrf=hrf/max(hrf)
                        for i=0,len_hrf_vs_t[trial]-1 do func[i,0,0,trial] = hrf[i]
                    end
                    !GAMMA_DERIV: begin
                        hrf = fltarr(len_hrf_vs_t[trial],2)
                        for i=0,len_hrf_vs_t[trial]-1 do begin
                            t = trial_t[i]
                            hrf[i,0] = (t^!GAMMA_ORDER)*exp(-t)/gamma(!GAMMA_ORDER+1)
                            hrf[i,1] = (t^(!GAMMA_ORDER-1))*(!GAMMA_ORDER-t)*exp(-t)/gamma(!GAMMA_ORDER+1)
                        endfor
                        hrf[*,0] = hrf[*,0]/max(hrf[*,0])
                        hrf[*,1] = hrf[*,1]/max(hrf[*,1])
                        j = 0
                        for i=0,len_hrf_vs_t[trial]-1 do begin
                            func[i,0,0,trial] = hrf[i,0]
                            func[i,1,0,trial] = hrf[i,1]
                        endfor
                    end
                    !BOXCAR_STIM: begin
                        hrf = fltarr(len_hrf_vs_t[trial])
                        scrap = delay_save[eff]/sng.TR
                        hrf[scrap:len_hrf_vs_t[trial]-1] = 1
                        func[0:len_hrf_vs_t[trial]-1,0,0,trial] = hrf
                    end
                    !TREND: begin
                        hrf = fltarr(len_hrf_vs_t[trial])
                        scrap = delay_save[eff]/sng.TR
                        hrf[scrap:len_hrf_vs_t[trial]-1] = 2.*findgen(len_hrf_vs_t[trial]-scrap)/(len_hrf_vs_t[trial]-scrap-1) - 1
                        func[0:len_hrf_vs_t[trial]-1,0,0,trial] = hrf
                    end
                    !USER_DEFINED: begin
                        func[0:len_hrf_vs_t[trial]-1,0,0,trial] = *ud[eff].hrf
                        func[0:len_hrf_vs_t[trial]-1,0,1,trial] = *ud[eff].hrf
                        func[0:len_hrf_vs_t[trial]-1,0,2,trial] = *ud[eff].hrf
                        func[0:len_hrf_vs_t[trial]-1,0,3,trial] = *ud[eff].hrf
                    end
                    !TIME_COURSE_EST: begin
                        if eff eq pause.eff_pause then scrap=pause.pause_length else scrap = len_hrf_vs_t[trial]
                        ;CHANGE
                        for i=0,scrap-1 do for j=0,lcround_tc[eff]-1 do func[i,lcround_tc[eff]*i+j,j,trial] = 1.
                    end
                    else: print,'Invalid value of functype.
                endcase
            endif
        endif
    endfor
    undefine,stimlen_vs_t,frames,stimlenframes_vs_t,fraction
    return,rtn_G={msg:'OK',conditions:conditions,len_hrf_vs_t:len_hrf_vs_t,func:func,nmain:nmain,effect_column:effect_column, $
        effect_length:effect_length,lcfunc:lcfunc}
end
function get_G,sng,stc2,pause,gf,b0,    delay_save,stimlen_save,effect_label_save,gnbehavcol,evselect,concselect,valid_frms, $
    scrap_eff,frequseev_list,rr_useev_list,lcppi,functype,funclen,max_num_levels,whoamistr,lcround_tc_index,omit_reference, $
    omit_trials_list,geff_all_same_len,lq_scale_globally,skipflag,talk,effect_TR_save,effect_shift_TR_save, $
    trial0 ;trial0: ignore if -1, first trial is 0
    common getG,G,G_behav
    frames=*sng.frames
    times=*sng.times
    offsets=*sng.offsets
    nbehavcol=sng.nbehavcol
    behav_vs_t=*sng.behav_vs_t
    tdim_sum_new=*stc2.tdim_sum_new
    t_to_file=*stc2.t_to_file
    conditions=gf.conditions
    len_hrf_vs_t=gf.len_hrf_vs_t
    func=gf.func
    nmain=gf.nmain
    effect_column=gf.effect_column
    effect_length=gf.effect_length
    effect_label=effect_label_save
    lcfunc=gf.lcfunc
    tot_eff=max_num_levels
    all_eff=tot_eff
    delay=delay_save
    stimlen=stimlen_save
    effect_TR=effect_TR_save
    effect_shift_TR=effect_shift_TR_save
    widget_control,/HOURGLASS
    if lq_scale_globally eq 'Yes' then begin
        print,'Scaling regressors globally.'
        for eff=0,max_num_levels-1 do begin
            if functype[eff] eq !BOYNTON or functype[eff] eq !SPM_CANNONICAL then begin
                index = where(conditions eq eff,count)
                if count ne 0 then func[*,0,*,index] = func[*,0,*,index]/max(func[*,0,*,index])
            endif
        endfor
    endif
    if frequseev_list[0] ne -1 then begin
        frequseev_frms = make_array(stc2.tdim_all,/FLOAT,VALUE=-1.)
        frequseev_cbyrun = intarr(stc2.n,max_num_levels)
    endif else begin
        frequseev_frms = -1.
        frequseev_cbyrun = -1
    endelse
    if rr_useev_list[0] ne -1 then begin
        rr_useev_frms = make_array(stc2.tdim_all,/FLOAT,VALUE=-1.)
        rr_useev_cbyrun = intarr(stc2.n,max_num_levels)
    endif else begin
        rr_useev_frms = -1.
        rr_useev_cbyrun = -1
    endelse
    G_behav=0.
    if gnbehavcol gt 0 then begin
        i1 = where(b0.ibehavcol lt nbehavcol,c1)
        if c1 ne 0 then begin
            nbehavcol = c1
            ib = b0.ibehavcol[i1]
            G_behav = fltarr(stc2.tdim_all,nmain*nbehavcol)
            for i=0,nbehavcol-1 do begin
                behavcol = reform(behav_vs_t[ib[i],*])
                if b0.behav_global_scaling[i] eq 1 then begin
                    index = where(behavcol ne float(!UNSAMPLED_VOXEL),count)
                    if count ne 0 then begin
                        behav_col = behavcol[index]
                        if b0.behav_vs_t_norm[i] eq b0.noscalei then begin
                            ;do nothing
                        endif else if b0.behav_vs_t_norm[i] eq b0.binaryi then begin
                            minimum = min(behav_col,MAX=maximum)
                            iscrap = where(behav_col eq minimum,count)
                            behav_col[iscrap] = -1.
                            iscrap = where(behav_col eq maximum,count)
                            behav_col[iscrap] = 1.
                        endif else if b0.behav_vs_t_norm[i] eq 0 then $
                            behav_col = behav_col/max(abs(behav_col)) $
                        else begin
                            behav_col = behav_col - total(behav_col)/count
                            if b0.behav_vs_t_norm[i] eq 2 then $
                                behav_col = behav_col/max(abs(behav_col)) $
                            else if b0.behav_vs_t_norm[i] eq 3 then $
                                behav_col = behav_col/sqrt(total(behav_col^2)) $
                            else if b0.behav_vs_t_norm[i] eq 4 then $
                                behav_col = behav_col/sqrt(total(behav_col^2)/float(count-1))
                            minimum = min(behav_col,MAX=maximum)
                            print,b0.bvstn_str[b0.behav_vs_t_norm[i]],': mean=',strtrim(total(behav_col)/count,2),' var=', $
                                strtrim(total(behav_col^2)/float(count-1),2),' min=',strtrim(minimum,2),' max=',strtrim(maximum,2)
                        endelse
                        behavcol[index]=behav_col
                    endif
                endif else begin
                    eff = where(scrap_eff[*,i] ne 0,count_scrap)
                    for j=0,count_scrap-1 do begin
                        index_eff = where(conditions eq eff[j],count_eff)
                        index = where(behavcol[index_eff] ne float(!UNSAMPLED_VOXEL),count)
                        if count eq 0 then goto,skip1
                        behav_col=behavcol[index_eff[index]]
                        if b0.behav_vs_t_norm[i] eq b0.noscalei then begin
                            ;do nothing
                        endif else if b0.behav_vs_t_norm[i] eq b0.binaryi then begin
                            minimum = min(behav_col,MAX=maximum)
                            if minimum eq maximum then begin
                                if b0.binaryi_set eq -2 then begin
                                    junk=get_button(['assign to +1', $
                                                     'assign to -1', $
                                                     'skip', $
                                                     'assign pos to +1, assign neg to -1, do not ask me again ', $
                                                     'skip all, do not ask me again', $
                                                     'exit'], $
                                        BASE_TITLE=evselect,TITLE='Column '+b0.nbehavcolstr[i]+'  '+effect_label[eff[j]] $
                                        +string(10B)+'Only a single value found = '+strtrim(minimum,2)+string(10B)+'Please select')
                                    if junk eq 0 then $
                                        junk=1. $
                                    else if junk eq 1 then $
                                        junk=-1. $
                                    else if junk eq 2 then $
                                        goto,skip1 $
                                    else if junk eq 3 then $
                                        b0.binaryi_set=1 $
                                    else if junk eq 4 then begin 
                                        b0.binaryi_set=0
                                        goto,skip1
                                    endif else $
                                        rtn_G={msg:!ERROR}
                                endif else begin
                                    if b0.binaryi_set eq 0 then $
                                        goto,skip1 $
                                    else begin
                                        if minimum lt 0. then junk=-1. else junk=1. 
                                    endelse 
                                endelse
                                iscrap = where(behav_col eq minimum,count)
                                behav_col[iscrap] = junk
                            endif else begin
                                iscrap = where(behav_col eq minimum,count)
                                behav_col[iscrap] = -1.
                                iscrap = where(behav_col eq maximum,count)
                                behav_col[iscrap] = 1.
                            endelse
                        endif else if b0.behav_vs_t_norm[i] eq 0 then $
                            behav_col = behav_col/max(abs(behav_col)) $
                        else begin
                            behav_col = behav_col - total(behav_col)/count
                            if b0.behav_vs_t_norm[i] eq 2 then $
                                behav_col = behav_col/max(abs(behav_col)) $
                            else if b0.behav_vs_t_norm[i] eq 3 then $
                                behav_col = behav_col/sqrt(total(behav_col^2)) $
                            else if b0.behav_vs_t_norm[i] eq 4 then $
                                behav_col = behav_col/sqrt(total(behav_col^2)/float(count-1))
                            minimum = min(behav_col,MAX=maximum)
                            print,b0.bvstn_str[b0.behav_vs_t_norm[i]],': mean=',strtrim(total(behav_col)/count,2),' var=', $
                                strtrim(total(behav_col^2)/float(count-1),2),' min=',strtrim(minimum,2),' max=',strtrim(maximum,2)
                        endelse
                        behavcol[index_eff[index]]=behav_col
                        skip1:
                    endfor
                endelse
                behav_vs_t[ib[i],*] = behavcol
            endfor
        endif
    endif

;START170113
    nskipeff=0
    skipstr = 'time      frames'+string(10B)
    frequseev_ntrials = 0
    rr_useev_ntrials = 0
    j = 0
    next_tdim = 0

    ;G = fltarr(tdim,nmain)
    ;START170113
    if trial0 eq -1 then $
        G=fltarr(stc2.tdim_all,nmain) $
    else begin
        G=fltarr(stc2.tdim_all,nmain+funclen[conditions[trial0]])
    endelse


    widget_control,/HOURGLASS
    for trial=0,sng.num_trials-1 do begin
        eff = conditions[trial]
        if eff ge 0 then begin
            if lcfunc[eff] ne !TIME_COURSE_EST then $
                t0 = frames[trial,0] $
            else begin
                t0 = frames[trial,lcround_tc_index[eff]]
            endelse
        endif else $
            t0 = frames[trial,0]
        if t0 gt tdim_sum_new[stc2.n-1] then begin
            stat=dialog_message(whoamistr+' You have '+strtrim(tdim_sum_new[stc2.n-1],2)+' frames of data.'+string(10B)+'However, ' $
               +'your next event occurs at frame '+strtrim(t0,2)+'.'+string(10B)+'You are asking me to model events where you ' $
                +'have no data.'+string(10B)+'Please check line '+strtrim(trial+2,2)+' in your event file.'+string(10B)+string(10B) $
                +evselect+string(10B)+concselect,/ERROR)
            return,rtn_G={msg:'ERROR'}
        endif
        if j lt stc2.n-1 then while times[trial] ge float(tdim_sum_new[j]-1)*sng.TR do j = j + 1
        next_tdim = tdim_sum_new[j]
        if eff ge 0 then begin
            if functype[eff] eq -1 then begin
                ;do nothing
            endif else if times[trial] le float(tdim_sum_new[j]-1)*sng.TR then begin
                if lcfunc[eff] ne !TIME_COURSE_EST then $
                    offset = 0 $
                else $
                    offset = offsets[trial,lcround_tc_index[eff]]
                if eff eq pause.eff_pause then begin
                    if pause.lcpause_per_run eq 1 then $
                        mm = effect_column[eff] + pause.pause_length*t_to_file[t0] $
                    else $
                        mm = effect_column[eff]
                    t1 = t0 + pause.pause_length - 1

                ;endif else begin
                ;START170113
                endif else if trial eq trial0 then begin
                    mm=0

                    ;START170120
                    t1 = t0 + len_hrf_vs_t[trial] - 1

                endif else begin

                    ;mm = effect_column[eff]
                    ;START170113
                    if trial0 eq -1 then begin
                        mm=effect_column[eff]
                    endif else begin
                        if trial eq trial0 then mm=0 else mm=funclen[conditions[trial0]]+effect_column[eff]
                    endelse


                    t1 = t0 + len_hrf_vs_t[trial] - 1
                    ;print,'trial=',trial,' len_hrf_vs_t=',len_hrf_vs_t[trial]
                endelse
                if t1 gt next_tdim-1 then t1 = next_tdim-1
                prd = t1 - t0 + 1
                prw = funclen[eff]
                ;print,'t0=',t0,' t1=',t1
                ;print,'prd=',prd,' prw=',prw,' offset=',offset,' trial=',trial
                ;print,'func=',func[0:prd-1,0:prw-1,offset,trial]
                G[t0:t1,mm:mm+prw-1] = G[t0:t1,mm:mm+prw-1] + func[0:prd-1,0:prw-1,offset,trial]
                if gnbehavcol gt 0 then begin
                    for i=0,nbehavcol-1 do begin
                        si = i*nmain
                        behav_col = reform(behav_vs_t[ib[i],*])
                        if behav_col[trial] ne float(!UNSAMPLED_VOXEL) then begin
                            func_behav=fltarr(max(len_hrf_vs_t),max(effect_length)) ;do this so it's zeroed
                            if b0.behav_vs_t_reg_type[i] eq 1 or functype[eff] ne !TIME_COURSE_EST then begin
                                if b0.behav_mult[i] eq 0 then begin
                                    func_behav[0:len_hrf_vs_t[trial]-1,0] = behav_col[trial]
                                endif else begin
                                    scrap = len_hrf_vs_t[trial] < prd
                                    func_behav[0:scrap-1,0] = behav_col[trial]*func[0:scrap-1,0:prw-1,offset,trial]
                                endelse
                            endif else begin
                                for ii=0,effect_length[eff]-1 do func_behav[ii,ii] = behav_col[trial]
                                ;print,'i=',i,' eff=',eff,' effect_length=',effect_length[eff],' trial=',trial, $
                                ;    ' behav_col=',behav_col[trial]
                            endelse
                            if b0.behav_vs_t_reg_type[i] eq 2 then begin
                                ;print,'t0=',t0,' t1=',t1,' si=',si,' si+prw-1=',si+prw-1
                                G_behav[t0:t1,si:si+prw-1] = G_behav[t0:t1,si:si+prw-1] + func_behav[0:prd-1,0:prw-1]
                                ;print,'func_behav=',func_behav[0:prd-1,0:prw-1]
                                ;print,'G_behav=',G_behav[t0:t1,si:si+prw-1]
                            endif else $
                                G_behav[t0:t1,si+mm:si+mm+prw-1] = G_behav[t0:t1,si+mm:si+mm+prw-1] + func_behav[0:prd-1,0:prw-1]
                            undefine,func_behav
                        endif
                    endfor
                endif
                if omit_reference eq !TRIAL then valid_frms[t0:t1] = 0
                if omit_trials_list[eff] eq 1 then begin
                    scrap = strtrim([t0,t1],2)
                    print,'Deleting frames indexed '+scrap[0]+' to '+scrap[1]+' for effect '+effect_label[eff]
                    valid_frms[t0:t1] = 0
                endif
                if frequseev_list[0] ne -1 then begin
                    if frequseev_list[eff] eq 1 then begin
                        frequseev_frms[t0:t1] = eff
                        frequseev_cbyrun[j,eff] = 1
                        frequseev_ntrials = frequseev_ntrials + 1
                    endif
                endif
                if rr_useev_list[0] ne -1 then begin
                    if rr_useev_list[eff] eq 1 then begin
                        rr_useev_frms[t0:t1] = eff
                        rr_useev_cbyrun[j,eff] = 1
                        rr_useev_ntrials = rr_useev_ntrials + 1
                    endif
                endif
            endif
        endif else begin ;Skip number of frames given by eff
            t1 = t0 - eff - 1
            if t1 gt stc2.tdim_all-1 then t1 = stc2.tdim_all-1
            if t0 gt stc2.tdim_all-1 then t0 = stc2.tdim_all-1
            valid_frms[t0:t1] = 0
            skipstr = skipstr + strtrim(times[trial],2)+' '+strtrim(t0+1,2)+'-'+strtrim(t1+1,2)+string(10B)
            nskipeff=nskipeff+(t1-t0+1)
        endelse
    endfor
    if nskipeff ne 0 and (skipflag eq 0 or talk eq 1) then begin
        skipstr = 'In your event file, you have entered codes to skip '+strtrim(nskipeff,2)+' frames.'+string(10B)+skipstr
        special = 'Talk off'
        stat=dialog_message_long(evselect,skipstr,SPECIAL=special)
        if stat eq 'EXIT' then return,rtn_G={msg:'ERROR'} $
        else if stat eq 'GOBACK' then return,rtn_G={msg:'GO_BACK'} $
        else if stat eq special then talk=0
        skipflag=1
    endif

    ;START170113
    if trial0 ne -1 then begin
        eff=conditions[trial0]
        effect_column=[0,effect_column+funclen[eff]]
        effect_length=[funclen[eff],effect_length]
        darby=trim(trial0+1)
        superbird=strlen(darby)
        if superbird lt 3 then darby='0'+darby
        if superbird lt 2 then darby='0'+darby
        effect_label=[darby+effect_label[eff],effect_label]
        nmain=total(effect_length)
        all_eff=all_eff+1
        lcfunc=[lcfunc[eff],lcfunc]
        delay=[delay[eff],delay]
        stimlen=[stimlen[eff],stimlen]
        tot_eff=tot_eff+1

        ;START170120
        effect_TR=[effect_TR[eff],effect_TR]
        effect_shift_TR=[effect_shift_TR[eff],effect_shift_TR]
       
    endif

    if gnbehavcol gt 0 then begin
        eco = effect_column
        elo = effect_length
        nmo = nmain
        te = max_num_levels
        delay=delay_save
        stimlen=stimlen_save
        tot_eff=max_num_levels
        for i=0,nbehavcol-1 do begin
            si = i*nmo
            if b0.behav_vs_t_reg_type[i] eq 2 then begin
                naddbehav = 1
                effect_column = [effect_column,nmain]
                behav_matrix = G_behav[*,si:si+geff_all_same_len[i]-1]
                effect_length = [effect_length,geff_all_same_len[i]]
            endif else begin
                naddbehav = te
                behav_matrix = G_behav[*,si:si+nmo-1]
                effect_column = [effect_column,eco+nmain]
                effect_length = [effect_length,elo]
            endelse
            nmain = total(effect_length)
            G = [[G],[behav_matrix]]
            all_eff = all_eff + naddbehav
            lcfunc = [lcfunc,intarr(naddbehav)]
            delay = [delay,fltarr(naddbehav)]
            stimlen = [stimlen,fltarr(naddbehav)]
            tot_eff = tot_eff + naddbehav
            effect_TR=[effect_TR,fltarr(naddbehav)]
            effect_shift_TR=[effect_shift_TR,fltarr(naddbehav)]
        endfor
        effect_label = [effect_label,b0.behav_label]
        if lcppi ne 1 then undefine,G_behav
    endif
    undefine,tdim_sum_new,t_to_file,conditions,frames,times,offsets,behav_vs_t,func,len_hrf_vs_t
    return,rtn_G={msg:'OK',frequseev_frms:frequseev_frms,frequseev_cbyrun:frequseev_cbyrun,nmain:nmain, $
        effect_column:effect_column,effect_length:effect_length,effect_label:effect_label,all_eff:all_eff,lcfunc:lcfunc, $
        delay:delay,stimlen:stimlen,tot_eff:tot_eff,frequseev_ntrials:frequseev_ntrials,nbehavcol:nbehavcol, $
        rr_useev_frms:rr_useev_frms,rr_useev_cbyrun:rr_useev_cbyrun,rr_useev_ntrials:rr_useev_ntrials,effect_TR:effect_TR,$
        effect_shift_TR:effect_shift_TR}
end
function get_gio,frequseev_cbyrun
    ncondfreqc = total(frequseev_cbyrun,1)
    frequseev_idx = where(ncondfreqc ne 0,ncondfreq)
    ncondfreqc = ncondfreqc[frequseev_idx]
    return,rtn={msg:'OK',ncondfreq:ncondfreq,ncondfreqc:ncondfreqc,frequseev_idx:frequseev_idx}
end
function get_yes,frequseev_frms,frequseev_idx,length,valid_frms
    index_yes = where(frequseev_frms eq frequseev_idx,count)
    vec_yes = intarr(length)
    vec_yes[index_yes] = 1
    index_yes = where((valid_frms+vec_yes) eq 2,count_yes)
    if count_yes eq 0 then begin
        scrap = 'count_yes='+strtrim(count_yes,2)+' Should not be zero!'
        print,scrap
        stat=dialog_message(scrap,/ERROR)
        return,rtn={msg:'ERROR'}
    endif
    return,rtn={msg:'OK',index_yes:index_yes,count_yes:count_yes}
end                                  
function define_single_dsgn_matrix_new2,params,glm,dsp,fi,pref,wd,help,BATCH=batch,DEFAULT=default,stc,BASIS_CLASS=basis_class, $
    TRIALS=trials
    if keyword_set(TRIALS) then begin
        title='Estimates are extracted for each trial using the FS method in Turner 2012 NeuroImage.'+string(10B) $
            +'Your event file should not code an event type for each trial.'+string(10B)+'Rather it should be coded conventially ' $
            +'with an event type for each condition.'
        scrap=get_button(['ok','exit'],BASE_TITLE='Rapid event, trial estimates',TITLE=title,/ALIGN_LEFT)
        if scrap eq 1 then return,!ERROR
        if help.enable eq !TRUE then widget_control,help.id,SET_VALUE=title
    endif
    common getG
    errorstr = ''
    lc_smoothness_estimate = 0
    nregion_files = 0
    gohere = 0
    region_str = ''
    roi_str = ''
    atlas_str = ''
    glm_conc = 0
    rr={region_str:ptr_new(),roi_str:'',nreg:0,region_names:ptr_new(),index:ptr_new(),scale_str:'',regnamestr:''}
    nomenu = 0
    t4str = ''
    globmask = ''
    nglob = 0
    omit_reference = 0
    freqloop = 1
    freqnorm = 0
    frequseev_count = 0
    run_trial = 0
    giofreqstartindex = 0
    rr_useidentify=0
    skipflag=0
    lc_rr_separate=0
    gnbehavcol=0
    giowtfile_path = ''
    binaryi_set = -2.
    voxwtfile_path = ''
    voxwt_str = ''
    lcppi=0
    rr_useev_count=0
    lcdetrend=0
    geff_all_same_len=0
    addsub_roi_str = ''
    ppi_cnt=0
    R2str=''
    lq_scale_globally = 'No'

    ;START170331
    count_present_ts=0
    lcexclude_count=0

    directory = getenv('PWD')
    spawn,'whoami',whoami
    whoamistr = 'Hey '+whoami[0]+'!'
    spawn,!BINEXECUTE+'/fidl_timestr2',timestr
    parroot=whoami[0]+'_'+timestr[0]
    pardir=directory+'/'+'glmpar_'+parroot
    spawn,'mkdir '+pardir,result
    goback99:
    if n_elements(parf0) ne 0 then undefine,parf0
    if n_elements(glmlist0) ne 0 then undefine,glmlist0
    talk0=1
    talk=0
    scrap = get_button(['yes','no','exit','load parameter file'],BASE_TITLE=whoamistr,TITLE='Do you want me to talk to you?')
    if scrap eq 0 then talk=1 else if scrap eq 1 then talk=0 else if scrap eq 2 then return,!ERROR else begin
        get_dialog_pickfile,'*.glmpar',fi.path,'Please select glm parameter file',parf0,nparf0,rtn_path
        if nparf0 eq 0 then goto,goback99
        parf0=parf0[0]
        par0=read_glm_pref(parf0)
        idx=where(tag_names(par0) eq 'TALK',cnt)
        if cnt ne 0 then talk=par0.talk 
        talk0=talk
        idx=where(tag_names(par0) eq 'LIST',cnt)
        if cnt ne 0 then glmlist0=par0.list
    endelse
    if n_elements(par1) ne 0 then undefine,par1
    par1=create_struct('talk',talk)
    goback100:
    widget_control,/HOURGLASS
    gg=get_glms(pref,/GET_CONC,/NOTMORETHANONE,GLM_LIST=glmlist0,NOTALK=abs(talk0-1))
    if gg.msg eq 'GO_BACK' then goto,goback99 else if gg.msg eq 'EXIT' then return,!ERROR
    total_nsubjects = gg.total_nsubjects
    concselect = gg.concselect
    t4select = gg.t4select
    evselect = gg.evselect
    txtselect = gg.txtselect
    if n_elements(glmlist0) eq 0 or talk0 eq 1 then begin
        glmlist0=pardir+'/'+parroot+'.list'
        openw,lu,glmlist0,/GET_LUN
        for i=0,total_nsubjects-1 do printf,lu,concselect[i]
        if t4select[0] ne '' then for i=0,total_nsubjects-1 do printf,lu,t4select[i]
        if evselect[0] ne '' then for i=0,total_nsubjects-1 do printf,lu,evselect[i]
        if txtselect[0] ne '' then for i=0,total_nsubjects-1 do printf,lu,txtselect[i]
        close,lu
        free_lun,lu
    endif
    idx=where(tag_names(par1) eq 'LIST',cnt)
    if cnt ne 0 then par1.list=glmlist0 else par1=create_struct(par1,'list',glmlist0)
    undefine,idx,cnt
    if txtselect[0] eq '' then txtselect = make_array(total_nsubjects,/STRING,VALUE='')
    goback100b:
    if evselect[0] eq '' or gohere eq 1 then begin
        gohere = 1
        goback0:
        widget_control,/HOURGLASS
        extralabel='Enter NONE if none.'
        if txtselect[0] ne '' then extralabel=extralabel+'  Enter TXT to use *.txt files.'
        gf = get_files(FILTER='*fi??',TITLE='Please enter event file filter.',EXTRALABEL=extralabel,FRONT=pref.fidlpath)
        if gf.msg eq 'GO_BACK' then goto,goback100 else if gf.msg eq 'EIXT' then return,!ERROR
        if gf.files[0] eq 'GOBACK' then goto,goback0
        msg=gf.msg
        evselect=gf.files
        if evselect[0] eq 'NONE' then begin
            evselect = make_array(total_nsubjects,/STRING,VALUE='NONE')
        endif else if txtselect[0] ne '' and evselect[0] eq 'TXT' then begin
            evselect = txtselect
            txtselect = make_array(total_nsubjects,/STRING,VALUE='') 
        endif else begin
            goback11:
            rtn_gr_conc = get_root(concselect,'.conc')
            minl=min(strlen(rtn_gr_conc.file),mini)
            if total_nsubjects eq 1 then begin
                if msg ne 'DONTCHECK' then rtn_special=1 else rtn_special=-1 
            endif else begin
                rtn = select_files([strtrim(indgen(minl)+1,2),'vc number'], $
                      TITLE='Fidl will automatically pair conc and event files.' $
                      +' If you do not want to use this option, select manual pairing below.'+string(10B)+string(10B) $
                      +'Only the root is matched. Path ignored.'+string(10B)+'Ex. '+rtn_gr_conc.file[mini]+string(10B) $
                      +string(10B)+'Match on characters',/GO_BACK,MIN_NUM_TO_SELECT=-1,BASE_TITLE='Match concs to event files', $
                      SPECIAL='Let me do this manually')
                if rtn.files[0] eq 'GO_BACK' then goto,goback0
                rtn_special = rtn.special
            endelse
            if rtn_special eq 1 then begin
                evselect = match_files(concselect,evselect,TITLE='Please select event file for ',/GO_BACK)
                if evselect[0] eq 'GO_BACK' then goto,goback11
            endif else if rtn_special ne -1 then begin
                goback12:
                rtn_gr_ev = get_root(evselect,'.')
                if max(rtn.index) eq minl then $
                    rtn_match_vc2=match_vc2(rtn_gr_conc.file,rtn_gr_ev.file,concselect,evselect) $
                else $
                    rtn_match_vc2=match_vc2(rtn_gr_conc.file,rtn_gr_ev.file,concselect,evselect,SEARCHINDEX=rtn.index)
                if rtn_match_vc2.msg eq 'ERROR' or rtn_match_vc2.msg eq 'EXIT' then $
                    return,!ERROR $
                else if rtn_match_vc2.msg eq 'GO_BACK' then $
                    goto,goback11
                concselect=rtn_match_vc2.files1
                evselect=rtn_match_vc2.files2
                total_nsubjects = n_elements(concselect) 
                scrap = get_str(total_nsubjects,concselect,evselect,TITLE='Please check event files.',/ONE_COLUMN,/BELOW,/GO_BACK)
                if scrap[0] eq 'GO_BACK' then goto,goback12 
                evselect = scrap
            endif
        endelse
    endif
    lc_atlas = 0
    lc_est_in_atlas = 0
    if total_nsubjects eq 1 and evselect[0] ne 'NONE' and evselect[0] ne 'DETREND' then begin
        glm_conc = get_button(['compute glm','bayfidl - make conc','go back'],TITLE='Please select')
        if glm_conc eq 2 then begin
            if evselect[0] eq '' or gohere eq 1 then goto,goback100b else goto,goback100
        endif
    endif
    if strmid(concselect[0],strlen(concselect[0])-3) eq 'img' then lc_concs = 0 else lc_concs = 1 
    t4_identify_str = make_array(total_nsubjects,/STR,VALUE='')
    space = intarr(total_nsubjects)
    atlas_str_conc = make_array(total_nsubjects,/STRING,VALUE='') 
    stc2 = replicate({Stitch},total_nsubjects)
    ifh2 = replicate({InterFile_Header},total_nsubjects)
    identify_yes=intarr(total_nsubjects)
    gt4=replicate({nt4:0L,t4:ptr_new()},total_nsubjects)
    glm_bigendian=abs(!SunOS_Linux-1)
    lctdim_same=1
    lcnii = intarr(total_nsubjects)
    if gg.replaceselect[0] ne '' then begin
        stc3=replicate({Stitch},total_nsubjects)
    endif
    i=0
    keeplist=intarr(total_nsubjects)
    widget_control,/HOURGLASS
    for m=0,total_nsubjects-1 do begin
        print,'Loading ',concselect[m]
        rload_conc=load_conc(fi,stc,dsp,help,wd,concselect[m],/DONT_PUT_IMAGE,/SKIP)
        if rload_conc.msg eq 'GOBACK' then goto,goback100b else if rload_conc.msg eq 'EXIT' then return,!ERROR $
        else if rload_conc.msg ne 'SKIP' then begin
            keeplist[m]=1
            stc2[i] = rload_conc.stc1
            lcnii[i] = rload_conc.lcnii
            if rload_conc.nt4s ne 0 then begin
                gt4[i].nt4 = rload_conc.nt4s
                gt4[i].t4 = ptr_new(rload_conc.t4s)
            endif
            identify = *stc2[i].identify
            if identify[0] ne '' then identify_yes[i]=1
            if lctdim_same eq 1 then begin
                tdim_file = *stc2[i].tdim_file
                if m eq 0 then tdim_file0 = tdim_file[0]
                if total(tdim_file-tdim_file0) ne 0 then lctdim_same=0
            endif
            if gg.replaceselect[0] ne '' then begin
                ;undefine,rload_conc CRASHES STC2 STC3 ARE POINTERS
                print,'Loading ',gg.replaceselect[i]
                rload_conc=load_conc(fi,stc,dsp,help,wd,gg.replaceselect[i],/DONT_PUT_IMAGE)
                if rload_conc.msg eq 'GOBACK' then goto,goback100b else if rload_conc.msg eq 'EXIT' then return,!ERROR
                stc3[i] = rload_conc.stc1
            endif
            ifh2[i] = rload_conc.ifh
            ifh2[i].matrix_size_4 = stc2[i].tdim_all
            ifh2[i].bigendian = glm_bigendian
            space[i] = get_space(rload_conc.ifh.matrix_size_1,rload_conc.ifh.matrix_size_2,rload_conc.ifh.matrix_size_3)
            if space[i] eq !SPACE_222 then atlas_str_conc[i]=' -atlas 222' else if space[i] eq !SPACE_333 then $
                atlas_str_conc[i]=' -atlas 333'
            ;undefine,rload_conc CRASHES STC2 STC3 ARE POINTERS
            i=i+1
        endif
    endfor
    if i lt total_nsubjects then begin
        total_nsubjects=i
        t4_identify_str=t4_identify_str[0:total_nsubjects-1]
        space=space[0:total_nsubjects-1]
        atlas_str_conc=atlas_str_conc[0:total_nsubjects-1]
        stc2=stc2[0:total_nsubjects-1]
        ifh2=ifh2[0:total_nsubjects-1]
        identify_yes=identify_yes[0:total_nsubjects-1]
        gt4=gt4[0:total_nsubjects-1]
        lcnii=lcnii[0:total_nsubjects-1]    
        if gg.replaceselect[0] ne '' then stc3=stc3[0:total_nsubjects-1] 
        idx=where(keeplist eq 1,cnt)
        concselect=concselect[idx]
        evselect=evselect[idx]
        undefine,idx,cnt
    endif 
    undefine,keeplist
    if total(space - space[0]) ne 0 then begin
        stat=dialog_message('Not all images are in the same space. Abort!',/ERROR)
        return,!ERROR
    endif
    bold_space = space[0]
    total_identify_yes = total(identify_yes)
    swapbytes=0
    atlasspace = bold_space
    bold_size = long(ifh2[0].matrix_size_1)*long(ifh2[0].matrix_size_2)*long(ifh2[0].matrix_size_3)
    bs=0
    idx=where(lcnii eq 1,cntlcnii)
    if cntlcnii eq 0 then begin
        if bold_space ne !SPACE_111 and bold_space ne !SPACE_222 and bold_space ne !SPACE_333 and bold_size ne 1 then begin
            bs = 1
            goback100a:
            scrap = get_button(['yes','no','go back'],TITLE='Do you want the GLMs computed in atlas space?')
            if scrap eq 2 then goto,goback100
            if scrap eq 0 then begin
                rtn=select_space()
                atlasspace=rtn.space
                atlas_str=rtn.atlas_str
                lc_atlas=1
                lc_est_in_atlas=1
                if gt4[0].nt4 eq 0 then begin
                    msg=get_t4s(lc_atlas,t4select,total_nsubjects,concselect,t4_identify_str,atlasspace,atlas_str,neach)
                    if msg eq 'GO_BACK' then goto,goback100a else if msg eq 'EXIT' then return,!ERROR
                endif
            endif
        endif else begin
            lc_est_in_atlas = 1
        endelse
    endif
    goback101:
    if n_elements(analysis_type) ne 0 then undefine,analysis_type
    if n_elements(parf0) ne 0 then begin
        idx=where(tag_names(par0) eq 'VOXELWISE',cnt)
        if cnt ne 0 then analysis_type=1-par0.voxelwise 
        undefine,idx,cnt 
    endif
    if n_elements(analysis_type) eq 0 then begin
        if glm_conc eq 0 and bold_size ne 1 then begin
            analysis_type = get_button(['voxel by voxel','specfic regions','go back'],TITLE='Please select analysis type.')
            if analysis_type eq 2 then begin
                if bs eq 1 then goto,goback100a else goto,goback100
            endif
        endif else $
            analysis_type = 0
    endif
    idx=where(tag_names(par1) eq 'VOXELWISE',cnt)
    if cnt ne 0 then par1.voxelwise=1-analysis_type else par1=create_struct(par1,'voxelwise',1-analysis_type) 
    undefine,idx,cnt
    goback102:
    fwhm=0.
    mask=''
    if analysis_type eq 0 then begin
        if bold_size ne 1 and glm_conc eq 0 then begin
            goback102a:
            cnt=0
            if n_elements(parf0) ne 0 then begin
                idx=where(tag_names(par0) eq 'FWHM',cnt)
                if cnt ne 0 then fwhm=par0.fwhm 
            endif
            if cnt eq 0 then begin
                scrap = get_str(1,'FWHM in voxels ','0',/GO_BACK,TITLE='Smooth with Gaussian 3D filter',$
                    LABEL='Enter 0 for no smoothing.',WIDTH=50)
                if scrap[0] eq 'GO_BACK' then goto,goback101
                fwhm = float(scrap[0])
            endif
            idx=where(tag_names(par1) eq 'FWHM',cnt)
            if cnt ne 0 then par1.fwhm=fwhm else par1=create_struct(par1,'fwhm',fwhm)
            undefine,idx,cnt
            goback102b:
            cnt=0
            if n_elements(parf0) ne 0 then begin
                idx=where(tag_names(par0) eq 'MASK',cnt)
                if cnt ne 0 then mask=par0.mask
            endif
            if cnt eq 0 then begin
                if n_elements(mfile) gt 0 then undefine,mfile 
                if gg.imgselect[0] ne '' and n_elements(gg.imgselect) eq 1 then mfile=gg.imgselect[0]
                scrap=get_mask(atlasspace,fi,mfile)
                if scrap eq 'GO_BACK' then goto,goback102a
                mask=scrap
            endif
            idx=where(tag_names(par1) eq 'MASK',cnt)
            if cnt ne 0 then par1.mask=mask else par1=create_struct(par1,'mask',mask)
            undefine,idx,cnt
        endif
    endif else if analysis_type eq 1 then begin
        rtn = get_regions(fi,wd,dsp,help)
        if rtn.msg eq 'EXIT' then return,!ERROR else if rtn.msg eq 'GO_BACK' then goto,goback101
        region_names = rtn.region_names
        region_space = rtn.space
        region_files = rtn.region_file
        nregion_files = rtn.nregfiles
        rtn = select_files(region_names,TITLE='Please select regions you wish to analyze.')
        scrap = strtrim(rtn.index+1,2)
        roi_str = ' -regions_of_interest ' + strjoin(strtrim(rtn.index+1,2),' ',/SINGLE)
    endif
    if evselect[0] eq 'NONE' then begin
        goback103:
        vals = get_str(1,'TR in sec',strtrim(pref.TR,2),/GO_BACK)
        if vals[0] eq 'GO_BACK' then goto,goback102 
        TR = float(vals[0])
    endif
    goback17:
    if n_elements(init_skip) ne 0 then undefine,init_skip
    if n_elements(scrap) ne 0 then undefine,scrap
    max_nrun = max(stc2[*].n,imax_nrun)
    min_tdim = min(stc2[*].tdim_all,index)
    min_tdim = min_tdim/stc2[index].n
    if n_elements(parf0) ne 0 then begin
        idx=where(tag_names(par0) eq 'SKIP',cnt)
        if cnt ne 0 then begin
            if n_elements(par0.skip) ge max_nrun then init_skip=par0.skip[0:max_nrun-1] else begin
                scrap=strarr(max_nrun)
                scrap[0:n_elements(par0.skip)-1]=trim(par0.skip)
            endelse
        endif
    endif
    if min_tdim eq 1 then $
        init_skip = intarr(max_nrun) $
    else if n_elements(init_skip) eq 0 then begin
        if n_elements(scrap) eq 0 then scrap=make_array(max_nrun,/STRING,VALUE=trim(pref.paraskip))
        widget_control,/HOURGLASS
        vals = get_str(max_nrun,'run'+trim(indgen(max_nrun)+1),scrap, $
            TITLE='Number of frames to skip.',WIDTH=25,/GO_BACK,FRONT=trim(pref.paraskip))
        if vals[0] eq 'GO_BACK' then begin
            if evselect[0] eq 'NONE' then goto,goback103 else goto,goback101
        endif
        init_skip = fix(vals)
    endif
    idx=where(tag_names(par1) eq 'SKIP',cnt)
    if cnt ne 0 then par1.skip=init_skip else par1=create_struct(par1,'skip',init_skip)
    undefine,idx,cnt
    goback18:
    if n_elements(lcpause) ne 0 then undefine,lcpause 
    if n_elements(hipass) ne 0 then undefine,hipass
    if n_elements(lcinter) ne 0 then undefine,lcinter
    if n_elements(lc_constrain) ne 0 then undefine,lc_constrain
    if n_elements(lc_omit_frames) ne 0 then undefine,lc_omit_frames
    if n_elements(lc_omit_trials) ne 0 then undefine,lc_omit_trials
    if n_elements(lc_trend) ne 0 then undefine,lc_trend
    if n_elements(lc_baseline) ne 0 then undefine,lc_baseline
    if n_elements(lc_dontexclude) ne 0 then undefine,lc_dontexclude
    if n_elements(lc_rr) ne 0 then undefine,lc_rr
    if n_elements(lc_er) ne 0 then undefine,lc_er
    if n_elements(lc_contrast) ne 0 then undefine,lc_contrast
    cnt1=0
    if n_elements(parf0) ne 0 then begin
        idx=where(tag_names(par0) eq 'SCANNER_PAUSED_DURING_TRIALS',cnt)
        if cnt ne 0 then lcpause=par0.scanner_paused_during_trials
        cnt1=cnt1+cnt
        idx=where(tag_names(par0) eq 'TEMPORAL_FILTER',cnt)
        if cnt ne 0 then hipass=par0.temporal_filter
        cnt1=cnt1+cnt
        idx=where(tag_names(par0) eq 'INTERACTION_TERMS',cnt)
        if cnt ne 0 then lcinter=par0.interaction_terms
        cnt1=cnt1+cnt
        idx=where(tag_names(par0) eq 'CONSTRAIN_DESIGN_MATRIX',cnt)
        if cnt ne 0 then lc_constrain=par0.constrain_design_matrix
        cnt1=cnt1+cnt
        idx=where(tag_names(par0) eq 'OMIT_FRAMES',cnt)
        if cnt ne 0 then lc_omit_frames=par0.omit_frames
        cnt1=cnt1+cnt
        idx=where(tag_names(par0) eq 'OMIT_FRAMES_OF_TRIAL_TYPES',cnt)
        if cnt ne 0 then lc_omit_trials=par0.omit_frames_of_trial_types
        cnt1=cnt1+cnt
        idx=where(tag_names(par0) eq 'LINEAR_TREND',cnt)
        if cnt ne 0 then lc_trend=par0.linear_trend
        cnt1=cnt1+cnt
        idx=where(tag_names(par0) eq 'INTERCEPT',cnt)
        if cnt ne 0 then lc_baseline=par0.intercept
        cnt1=cnt1+cnt
        idx=where(tag_names(par0) eq 'MODEL_ALL_EVENT_TYPES',cnt)
        if cnt ne 0 then lc_dontexclude=par0.model_all_event_types
        cnt1=cnt1+cnt
        idx=where(tag_names(par0) eq 'REGIONAL_REGRESSORS',cnt)
        if cnt ne 0 then lc_rr=par0.regional_regressors
        cnt1=cnt1+cnt
        idx=where(tag_names(par0) eq 'EXTERNAL_REGRESSORS',cnt)
        if cnt ne 0 then lc_er=par0.external_regressors
        cnt1=cnt1+cnt
        idx=where(tag_names(par0) eq 'SET_UP_CONTRASTS',cnt)
        if cnt ne 0 then lc_contrast=par0.set_up_contrasts
        cnt1=cnt1+cnt
    endif
    if cnt1 ne 13 then begin
        dflt = [0,0,0,0,0,0,1,1,1,0,0,1]
        if cnt1 ne 0 then begin
            if n_elements(lcpause) ne 0 then dflt[0]=lcpause
            if n_elements(hipass) ne 0 then dflt[1]=hipass
            if n_elements(lcinter) ne 0 then dflt[2]=lcinter
            if n_elements(lc_constrain) ne 0 then dflt[3]=lc_constrain
            if n_elements(lc_omit_frames) ne 0 then dflt[4]=lc_omit_frames
            if n_elements(lc_omit_trials) ne 0 then dflt[5]=lc_omit_trials
            if n_elements(lc_trend) ne 0 then dflt[6]=lc_trend
            if n_elements(lc_baseline) ne 0 then dflt[7]=lc_baseline
            if n_elements(lc_dontexclude) ne 0 then dflt[8]=lc_dontexclude
            if n_elements(lc_rr) ne 0 then dflt[9]=lc_rr
            if n_elements(lc_er) ne 0 then dflt[10]=lc_er
            if n_elements(lc_contrast) ne 0 then dflt[11]=lc_contrast
        endif
        if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='Select the pause option if acquistion was paused after ' $
            +'stimulus presentation.  Applying a high-pass filter will remove low-frequency components in the data due to ' $
            +'motion artifacts.  This reduces the variance in the estimates, but also decreases the signal in rapidly presented ' $
            +'single trials.  This option is recommended for widely spaced single trials, block design studies, and studies ' $
            +'in which there is a low-frequency confound in the data.  The interaction option computes interaction terms to ' $
            +'model nonlinearity of the BOLD response.  Always select "No" unless the study was specifically designed to test ' $
            +'for nonlinearity.  If the design matrix has very few rows that uniquely represent the mean, the estimate of the ' $
            +'mean will be inaccurate.  This can lead to non-zero means in other effects, especially for widely-spaced ' $
            +'event-related designs, mixed block/event-related designs, and designs where scanner acquistion is paused to ' $
            +'recover vocal responses.  In these cases, click the "yes" button.  This will lead to more accurate mean ' $
            +'estimates.  It will also lead to time-courses with means that are nearly zero.'
        scraplabels = strarr(2,12)
        scraplabels[0,0] = 'Scanner paused during trials'
        scraplabels[1,0] = 'No pause'
        scraplabels[0,1] = 'Apply temporal filter'
        scraplabels[1,1] = 'No temporal filter'
        scraplabels[0,2] = 'Compute interaction terms'
        scraplabels[1,2] = 'No interaction terms'
        scraplabels[0,3] = 'Constrain design matrix'
        scraplabels[1,3] = "Don't constrain design matrix"
        scraplabels[0,4] = 'Omit specified frames from the analysis?'
        scraplabels[1,4] = "Don't omit specified frames from the analysis?'
        scraplabels[0,5] = 'Omit frames of specified trial types from the analysis?'
        scraplabels[1,5] = "Don't omit specified trial types from the analysis?"
        scraplabels[0,6] = 'Model linear trend'
        scraplabels[1,6] = "Don't model linear trend"
        scraplabels[0,7] = 'Model baseline (intercept)'
        scraplabels[1,7] = "Don't model baseline"
        scraplabels[0,8] = 'Model all event types in event file'
        scraplabels[1,8] = "Don't model some event types"
        scraplabels[0,9] = 'Regional regressors'
        scraplabels[1,9] = 'None'
        scraplabels[0,10] = 'Externally defined regressors'
        scraplabels[1,10] = 'None'
        scraplabels[0,11] = 'Set up contrasts'
        scraplabels[1,11] = "Don't set up"
        lclist = get_choice_list(scraplabels,TITLE='Select options',DEFAULT=dflt,/GO_BACK)
        if lclist[0] eq -1 then goto,goback17
        lcpause = lclist[0]
        hipass = lclist[1]
        lcinter = lclist[2]
        lc_constrain = lclist[3]
        lc_omit_frames = lclist[4]
        lc_omit_trials = lclist[5]
        lc_trend = lclist[6]
        lc_baseline = lclist[7]
        lc_dontexclude = lclist[8]
        lc_rr = lclist[9]
        lc_er = lclist[10]
        lc_contrast = lclist[11]
    endif
    idx=where(tag_names(par1) eq 'SCANNER_PAUSED_DURING_TRIALS',cnt)
    if cnt ne 0 then begin
        par1.scanner_paused_during_trials=lcpause & par1.temporal_filter=hipass & par1.interaction_terms=lcinter
        par1.constrain_design_matrix=lc_constrain & par1.omit_frames=lc_omit_frames & par1.omit_frames_of_trial_types=lc_omit_trials
        par1.linear_trend=lc_trend & par1.intercept=lc_baseline & par1.model_all_event_types=lc_dontexclude
        par1.regional_regressors=lc_rr & par1.external_regressors=lc_er & par1.set_up_contrasts=lc_contrast
    endif else $ 
        par1=create_struct(par1,'scanner_paused_during_trials',lcpause,'temporal_filter',hipass,'interaction_terms',lcinter,$
            'constrain_design_matrix',lc_constrain,'omit_frames',lc_omit_frames,'omit_frames_of_trial_types',lc_omit_trials,$
            'linear_trend',lc_trend,'intercept',lc_baseline,'model_all_event_types',lc_dontexclude,$
            'regional_regressors',lc_rr,'external_regressors',lc_er,'set_up_contrasts',lc_contrast)
    undefine,idx,cnt
    gb18_181 = 0
    if lc_rr eq 1 then begin
        if lc_est_in_atlas eq 0 then begin
            goback181:
            scrap=get_button(['atlas space analysis','go back'],BASE_TITLE=whoamistr,TITLE='You seem to want to do a correlational ' $
                +'analysis.'+string(10B)+'You probably want to do this in atlas space.')
            if scrap eq 1 then begin
                if lc_rr eq 1 then goto,goback180a else goto,goback18
            endif
            rtn=select_space()
            atlasspace=rtn.space
            atlas_str=rtn.atlas_str
            lc_atlas=1
            lc_est_in_atlas = 1
            if gt4[0].nt4 eq 0 then begin
                msg=get_t4s(lc_atlas,t4select,total_nsubjects,concselect,t4_identify_str,atlasspace,atlas_str,neach)
                if msg eq 'GO_BACK' then goto,goback181 else if msg eq 'EXIT' then return,!ERROR
            endif
            gb18_181 = 1
        endif
    endif
    goback22a:
    lc_multiple_stimlen = !FALSE
    max_num_levels_index=0
    if evselect[0] ne 'NONE' then begin 
        nbehavcolts = intarr(total_nsubjects) 
        sng = replicate({Eventfile},total_nsubjects)
        for m=0,total_nsubjects-1 do begin
            sng[m] = define_single_trial(fi,pref,help,EVENT_FILE=evselect[m])
            if sng[m].name eq 'ERROR' then return,!ERROR
            nbehavcolts[m] = sng[m].nbehavcol
        endfor
        max_num_levels = max(sng[*].num_levels,max_num_levels_index)
        if total(abs(max_num_levels-sng[*].num_levels)) ne 0. then begin
            idx = where(sng[*].num_levels ne max_num_levels,cnt)
            print,'Our prototype is:'
            print,evselect[max_num_levels_index]
            print,'These files have fewer than '+trim(max_num_levels)+' conditions:'
            print,evselect[idx]
            stat = get_button(['Ok','Exit'],BASE_TITLE=whoamistr,TITLE='Your event files must have identical headers.'+string(10B) $
                +'If one or more of your event files just has extra conditions listed at the end, then ok.'+string(10B) $
                +'Otherwise exit and fix those headers.')
            if stat eq 1 then return,!ERROR
        endif
        present = intarr(max_num_levels)
        for m=0,total_nsubjects-1 do present[*sng[m].index_present] = 1
        index_present_ts = where(present,count_present_ts)
        effect_label = *sng[max_num_levels_index].factor_labels
        present = intarr(max_num_levels)
        for m=0,total_nsubjects-1 do present[*sng[m].index_present] = 1
        index_present_ts = where(present,count_present_ts)
        if count_present_ts eq 0 then begin
            stat=dialog_message(whoamistr+string(10B)+'None of these events appear in the event file. Abort!',/ERROR)
            return,!ERROR
        endif
        index_notpresent_ts = where(present eq 0,count_notpresent_ts)
        if count_notpresent_ts ne 0 then begin
            print,'index_notpresent_ts=',index_notpresent_ts
            if total_nsubjects eq 1 then begin
                penguin = strjoin(effect_label[index_notpresent_ts],string(10B),/SINGLE)
                stat = get_button(['yes','exit'],BASE_TITLE=whoamistr, $
                    TITLE='The following events are not present in the event file'+string(10B)+string(10B)+penguin+string(10B) $
                        +string(10B)+'Is this ok?')
                if stat eq 1 then return,!ERROR
            endif
        endif
        goback183:
        lcgoback183=0
        spider = ''
        for m=0,total_nsubjects-1 do begin
            scrap = *sng[m].factor_labels
            for i=0,sng[m].num_levels-1 do begin
                if scrap[i] ne effect_label[i] then begin
                    spider = [spider,evselect[m]+string(10B)+'    '+strjoin(scrap,' ',/SINGLE)]
                    goto,outofhere
                endif
            endfor
            outofhere:
        endfor
        if n_elements(spider) gt 1 then begin
            lcgoback183=1
            spider='The reference event file is'+string(10B)+'    '+evselect[max_num_levels_index]+string(10B) $
                +'with effects'+string(10B)+string(10B)+'    '+strjoin(effect_label,' ',/SINGLE)+string(10B)+string(10B) $
                +'The following have different effects listed'+string(10B)+string(10B)+strjoin(spider[1:*],string(10B),/SINGLE)
            special = 'Edit effect labels'
            stat=dialog_message_long('Effect labels are not consistent',spider,SPECIAL=special)
            if stat eq 'EXIT' then return,!ERROR $
            else if stat eq 'GOBACK' then goto,goback18 $
            else if stat eq special then begin
                scrap = get_str(max_num_levels,effect_label,effect_label,WIDTH=60,TITLE='Please edit efect labels', $
                    LABEL='Your event files will not be modified.')
                if scrap[0] eq 'GO_BACK' then goto,goback18
                effect_label = strtrim(scrap,2)
            endif
        endif
        index_present_ts0 = index_present_ts
        lcexclude = make_array(max_num_levels,/INT,VALUE=0)
        lcexclude_count = 0
        if lc_dontexclude eq 0 then begin
            rtn=select_files(effect_label[index_present_ts],TITLE='Effects not to model',/GO_BACK,/ONE_COLUMN)
            if rtn.files[0] eq 'GO_BACK' then goto,goback18
            lcexclude[index_present_ts] = rtn.list
            lcexclude_save = lcexclude
            lcexclude_count = rtn.count
            idx = where(rtn.list eq 0,cnt)
            if cnt ne 0 then begin
                index_present_ts = index_present_ts[idx]
                count_present_ts = cnt
            endif
        endif
        lcnotfound=0
        if n_elements(parf0) ne 0 then begin
            idx=where(tag_names(par0) eq 'EFFECT_LABEL',cnt)
            if cnt eq 0 then begin
                eli_notfound=indgen(n_elements(effect_label))
            endif else begin
                eli_found=intarr(n_elements(effect_label[index_present_ts]))
                eli_found_par0=intarr(n_elements(effect_label[index_present_ts]))
                for i=0,n_elements(effect_label[index_present_ts])-1 do begin
                    idx=where(effect_label[index_present_ts[i]] eq par0.effect_label,cnt)
                    if cnt eq 0 then begin
                        print,effect_label[index_present_ts[i]]+' not found in '+parf0
                        lcnotfound=lcnotfound+1
                    endif else if cnt gt 1 then begin
                        print,'cnt='+trim(cnt)+' Label should only appear once'
                        print,'par0.effect_label='+par0.effect_label
                        return,!ERROR
                    endif else begin
                        eli_found[i]=index_present_ts[i]
                        eli_found_par0[i]=idx
                    endelse
                endfor
            endelse
        endif
        hrf_params = fltarr(2,max_num_levels)
        hrf_params[0,*] = !HRF_DELTA
        hrf_params[1,*] = !HRF_TAU
        tot_eff = max_num_levels 
        TR = sng[max_num_levels_index].TR
        print,'TR=',TR
        if n_elements(stimlen) ne 0 then undefine,stimlen
        if n_elements(delay) ne 0 then undefine,delay
        if n_elements(parf0) ne 0 and lcnotfound eq 0 then begin
            idx=where(tag_names(par0) eq 'STIMLEN',cnt)
            if cnt ne 0 then begin
                if n_elements(par0.stimlen) eq n_elements(par0.effect_label) then begin
                    stimlen=fltarr(max_num_levels)
                    stimlen[eli_found]=par0.stimlen[eli_found_par0]
                endif
            endif
            undefine,idx,cnt
            idx=where(tag_names(par0) eq 'DELAY',cnt)
            if cnt ne 0 then begin
                if n_elements(par0.delay) eq n_elements(par0.effect_label) then begin
                    delay=fltarr(max_num_levels)
                    delay[eli_found]=par0.delay[eli_found_par0]
                endif
            endif
            undefine,idx,cnt
        endif


        str = make_array(max_num_levels*2,/STRING,VALUE='')
        for m=0,max_num_levels-1 do str[2*m+1]=trim(pref.glm_hrf_delay)
        widget_type = make_array(max_num_levels*2,/STRING,VALUE='field') ;'button' or 'field'
        incdur = 1000
        maxdur = max_num_levels>incdur
        dur = fltarr(max_num_levels,maxdur)
        ndur = intarr(max_num_levels)
        widget_control,/HOURGLASS
        for m=0,total_nsubjects-1 do begin
            conditions1 = *sng[m].conditions
            index_present = *sng[m].index_present
            stimlen_vs_t = *sng[m].stimlen_vs_t
            for eff=0,sng[m].count_present-1 do begin
                index_conditions = where(conditions1 eq index_present[eff],count)
                scrap = stimlen_vs_t[index_conditions]
                scrap = scrap[uniq(scrap,sort(scrap))]
                i = n_elements(scrap)
                if i gt 1 then widget_type[2*index_present[eff]] = 'button'
                for j=0,i-1 do begin
                    if ndur[index_present[eff]] eq maxdur then begin
                        lizard = dur
                        maxdur1 = maxdur + incdur
                        undefine,dur
                        dur = fltarr(max_num_levels,maxdur1)
                        dur[*,0:maxdur-1] = lizard
                        maxdur = maxdur1
                        undefine,lizard,maxdur1
                    endif
                    for k=0,ndur[index_present[eff]]-1 do if scrap[j] eq dur[index_present[eff],k] then goto,bottom
                    dur[index_present[eff],ndur[index_present[eff]]] = scrap[j]
                    ndur[index_present[eff]] = ndur[index_present[eff]] + 1
                    str[2*index_present[eff]] = str[2*index_present[eff]] + trim(scrap[j]) + ' '
                    bottom:
                endfor
                undefine,index_conditions,scrap
            endfor
            undefine,conditions1,index_present,stimlen_vs_t
        endfor
        undefine,incdur,maxdur,dur,ndur
        str_resp_param = str
        goback2:
        scrap = [index_present_ts*2,index_present_ts*2+1]
        scrap = scrap[sort(scrap)]
        widget_type = widget_type[scrap]
        if n_elements(stimlen) eq 0 or n_elements(delay) eq 0 then begin
            label = strarr(max_num_levels*2)
            for m=0,max_num_levels-1 do begin
                label[2*m] = 'Duration: '+effect_label[m]
                label[2*m+1] = 'Delay: '+effect_label[m]
            endfor
            if lcexclude_count eq count_present_ts then $
                durdel = float(str_resp_param[scrap]) $
            else if count_present_ts gt 350 then begin
                print,'count_present_ts=',count_present_ts,' Current display limit is set at 350.'
                durdel = float(str_resp_param[scrap])
            endif else begin
                str = get_str_bool_intermixed(count_present_ts*2,label[scrap],'',str_resp_param[scrap],widget_type,'', $
                    TITLE='Response Parameters (sec)',LABEL='Delays: Time-shift from default ('+strtrim(!HRF_DELTA,2)+' sec)' $
                    +string(10B)+'Boxcar and user defined regressors have a shift of 0 sec.',WIDTH=10,/GO_BACK,/ONE_COLUMN)
                if str[0] eq 'GO_BACK' then begin
                    if lcgoback183 eq 0 then goto,goback18 else goto,goback183
                endif
                durdel = float(str)
            endelse
            delay = fltarr(max_num_levels)
            stimlen = fltarr(max_num_levels)
            for eff=0,count_present_ts-1 do begin
                stimlen[index_present_ts[eff]] = durdel[eff*2]
                delay[index_present_ts[eff]] = durdel[eff*2+1]
            endfor
            undefine,label
        endif else begin
            durdel = fltarr(count_present_ts*2)
            for eff=0,count_present_ts-1 do begin
                durdel[eff*2]=stimlen[index_present_ts[eff]]
                durdel[eff*2+1]=delay[index_present_ts[eff]]
            endfor
        endelse
        idx=where(tag_names(par1) eq 'EFFECT_LABEL',cnt)
        if cnt ne 0 then begin 
            par1.effect_label=effect_label & par1.stimlen=stimlen & par1.delay=delay 
        endif else $
            par1=create_struct(par1,'effect_label',effect_label,'stimlen',stimlen,'delay',delay)
        undefine,idx,cnt
        goback21:
        if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='The default action is to esimate the ' $
            +'timecourse of the hemodynamic response for each effect. Selecting an effect makes the program ' $
            +'encode the expected hemodynamic response into the design matrix. This encoding makes stronger ' $
            +'assumptions about the shape of the hemodynamic response function. The Friston model consists  ' $
            +'of three delayed gamma functions and their derivatives.'+' The SPM Cannonical model is the model ' $
            +'used in SPM to generate magnitudes. It is the difference of a gamma function with a positive peak ' $
            +'at five seconds and a second gamma function with a peak at 15 seconds.'
        types0=[!TIME_COURSE_EST,!BOYNTON,!FRISTON,!SPM_CANNONICAL,!GAMMA_DERIV,!BOXCAR_STIM,!TREND,!USER_DEFINED,-1]
        typesstr0=['TIME_COURSE_EST','BOYNTON','FRISTON','SPM_CANNONICAL','GAMMA_DERIV','BOXCAR_STIM','TREND','USER_DEFINED','-1']
        if n_elements(lcfunc) ne 0 then undefine,lcfunc
        if n_elements(functype) ne 0 then undefine,functype
        if n_elements(functypestr) ne 0 then undefine,functypestr
        if n_elements(parf0) ne 0 and lcnotfound eq 0 then begin
            idx=where(tag_names(par0) eq 'HRF',cnt)
            if cnt ne 0 then begin
                undefine,idx,cnt
                if n_elements(par0.hrf) eq n_elements(par0.effect_label) then begin
                    functype=make_array(max_num_levels,/INTEGER,VALUE=-1)
                    functypestr=make_array(max_num_levels,/STRING,VALUE='-1')
                    for i=0,n_elements(eli_found_par0)-1 do begin
                        idx=where(par0.hrf[eli_found_par0[i]] eq typesstr0,cnt)
                        if cnt eq 0 then begin
                            print,'i=',i,' par0.hrf[i]=',par0.hrf[i],' not found'
                            print,'typesstr0=',typesstr0
                            return,!ERROR
                        endif else if cnt gt 1 then begin
                            print,'i=',i,' par0.hrf[i]=',par0.hrf[i],' cnt=',cnt,' idx=',idx,' more than one found'
                            print,'typesstr0=',typesstr0
                            return,!ERROR
                        endif else begin
                            functype[eli_found[i]]=types0[idx]
                            functypestr[eli_found[i]]=typesstr0[idx]
                        endelse
                        undefine,idx,cnt
                    endfor
                    lcfunc=make_array(max_num_levels,/INT,VALUE=-1)
                    idx=where(functype ne -1,cnt)
                    if cnt ne 0 then begin
                        lcfunc[idx]=1
                        undefine,idx,cnt
                        idx=where(functype eq !TIME_COURSE_EST,cnt)
                        if cnt ne 0 then lcfunc[idx]=0
                        undefine,idx,cnt
                    endif else undefine,idx,cnt


                    ;functype_all_ud=0
                    ;count_userdefined=0
                    ;if total(lcfunc[index_present_ts]) gt 0 then begin
                    ;    idx=where(lcfunc eq 1,count_lcfunceq1)
                    ;    index_userdefined=where(functype eq !USER_DEFINED,count_userdefined)
                    ;    if count_userdefined ne 0 then begin
                    ;        if count_userdefined eq count_lcfunceq1 then functype_all_ud=1
                    ;    endif
                    ;    undefine,idx,count_lcfunceq1
                    ;endif

                endif
            endif else undefine,idx,cnt
        endif
        if n_elements(lcfunc) eq 0 then begin
            lcfunc=make_array(max_num_levels,/INT,VALUE=-1)
            if glm_conc eq 0 then begin
                if lcexclude_count eq count_present_ts then $
                    lcfunc[index_present_ts] = 0 $
                else begin
                    ;biggest=100
                    ;if count_present_ts le biggest then begin
                        rtn=select_files(effect_label[index_present_ts],TITLE='Effects with an assumed response shape',$
                            MIN_NUM_TO_SELECT=-1,/GO_BACK,/ONE_COLUMN)
                        if rtn.files[0] eq 'GO_BACK' then goto,goback2
                        lcfunc[index_present_ts] = rtn.list
                    ;endif else begin
                    ;    ceil0=ceil(count_present_ts/float(biggest))
                    ;    floor0=count_present_ts/biggest
                    ;    lizard = intarr(ceil0)
                    ;    lizard[0:floor0-1]=biggest
                    ;    if ceil0 ne floor0 then lizard[floor0]=count_present_ts mod biggest
                    ;    ribbit = effect_label[index_present_ts]
                    ;    list=-1
                    ;    j=0
                    ;    for i=0,ceil0-1 do begin
                    ;        rtn=select_files(ribbit[j:j+lizard[i]-1],TITLE='Effects with an assumed response shape', $
                    ;            MIN_NUM_TO_SELECT=-1,/GO_BACK,/ONE_COLUMN)
                    ;        if rtn.files[0] eq 'GO_BACK' then begin
                    ;            if i eq 0 then goto,goback2 else goto,goback21
                    ;        endif
                    ;        list = [list,rtn.list]
                    ;        j=j+lizard[i]
                    ;    endfor
                    ;    lcfunc[index_present_ts] = list[1:*]
                    ;endelse
                endelse
            endif else $
                lcfunc[index_present_ts] = 1
        endif

        ;START160429
        ;idx=where(tag_names(par1) eq 'ASSUMED_RESPONSE',cnt)
        ;if cnt ne 0 then par1.assumed_response=lcfunc else par1=create_struct(par1,'assumed_response',lcfunc)
        ;undefine,idx,cnt

        goback211:
        lcusedur = intarr(max_num_levels)
        index_hrf = where(lcfunc eq !TIME_COURSE_EST,count_hrf)
        if n_elements(len_hrf) ne 0 then undefine,len_hrf
        if n_elements(parf0) ne 0 and lcnotfound eq 0 then begin
            idx=where(tag_names(par0) eq 'LEN_HRF',cnt)
            if cnt ne 0 then begin
                if n_elements(par0.len_hrf) eq n_elements(par0.effect_label) then begin
                    len_hrf=make_array(max_num_levels,/INT,VALUE=pref.period)
                    len_hrf[eli_found]=par0.len_hrf[eli_found_par0]
                endif
            endif
        endif
        if n_elements(len_hrf) eq 0 then begin
            len_hrf = make_array(max_num_levels,/INT,VALUE=pref.period)
            widget_control,/HOURGLASS
            for m=0,total_nsubjects-1 do begin
                conditions1 = *sng[m].conditions
                index_present = *sng[m].index_present
                stimlenframes_vs_t = *sng[m].stimlenframes_vs_t
                for eff=0,sng[m].count_present-1 do begin
                    index_conditions = where(conditions1 eq index_present[eff],count)
                    len_hrf[index_present[eff]] = round(max(stimlenframes_vs_t[index_conditions])) > len_hrf[index_present[eff]]
                    undefine,index_conditions
                endfor
                undefine,conditions1,index_present,stimlenframes_vs_t
            endfor
            if count_hrf gt 0 then begin
                if min_tdim eq 1 then $
                    len_hrf[*] = 1 $
                else begin
                    if lcexclude_count eq count_present_ts then $
                        len_hrf[*] = 1 $
                    else begin
                        scrap = get_str(count_hrf,effect_label[index_hrf],len_hrf[index_hrf],WIDTH=60,/ONE_COLUMN,/GO_BACK, $
                            TITLE='Please enter the number of frames to estimate for each timecourse.',FRONT=pref.period, $
                            LABEL='Enter DURATION to use column 3 of the event file.')
                        if scrap[0] eq 'GO_BACK' then begin
                            if glm_conc eq 0 and lcexclude_count ne count_present_ts then goto,goback21 else goto,goback2
                        endif
                        idx=where(strupcase(scrap) eq 'DURATION',cnt)
                        if cnt ne 0 then begin
                            scrap[idx]=len_hrf[index_hrf[idx]]
                            lcusedur[index_hrf[idx]]=1
                        endif
                        len_hrf[index_hrf] = fix(scrap)
                    endelse
                endelse
            endif
        endif
        idx=where(tag_names(par1) eq 'LEN_HRF',cnt)
        if cnt ne 0 then par1.len_hrf=len_hrf else par1=create_struct(par1,'len_hrf',len_hrf)
        undefine,idx,cnt
        frequseev_list = -1
        rr_useev_list = -1
        ;lcround 1:TR 2:half_TR 4:quarter_TR
        lcround_tc = intarr(max_num_levels)
        lcround_tc_index = intarr(max_num_levels)
        lcround_tc_index[index_present_ts] = lcfunc[index_present_ts]*2
        geffect_shift_TR = fltarr(max_num_levels)
        if count_hrf gt 0 then begin
            dummy = strarr(3,count_hrf)
            dummy[0,*] = 'TR'
            dummy[1,*] = 'half TR'
            dummy[2,*] = 'quarter TR'
            dummy_lengths = make_array(count_hrf,/INT,VALUE=1)
            rtn = get_htr_qtr(max_num_levels,total_nsubjects,sng)
            hTR = rtn.hTR[index_hrf]
            qTR = rtn.qTR[index_hrf]
            qTRo1 = rtn.qTRo1
            qTRo2 = rtn.qTRo2
            qTRo3 = rtn.qTRo3
            count_qTR = 0
            index = where(hTR eq total_nsubjects,count_hTR)
            if count_hTR ne 0 then begin
                dummy_lengths[index] = 2
                index_qTR = where(qTR eq total_nsubjects,count_qTR)
                if count_qTR ne 0 then dummy_lengths[index_qTR] = 3
            endif
            if count_hTR gt 0 or count_qTR gt 0 then begin
                rtn = get_choice_list_many(effect_label[index_hrf],dummy,TOP_TITLE='Please select.', $
                    /COLUMN_LAYOUT,TITLE='Place timecourses at the nearest',LENGTHS=dummy_lengths)
                lcround_tc_index[index_hrf] = rtn.list
                lcround_tc[index_hrf] = lcround_tc_index[index_hrf]*2
            endif
            index = where(dummy_lengths eq 1,count)
            if count ne 0 then begin
                iqTRo1 = where(qTRo1[index_hrf] eq total_nsubjects,cqTRo1)
                if cqTRo1 ne 0 then geffect_shift_TR[index_hrf[iqTRo1]] = TR*.25
                iqTRo2 = where(qTRo2[index_hrf] eq total_nsubjects,cqTRo2)
                if cqTRo2 ne 0 then geffect_shift_TR[index_hrf[iqTRo2]] = TR*.5
                iqTRo3 = where(qTRo3[index_hrf] eq total_nsubjects,cqTRo3)
                if cqTRo3 ne 0 then geffect_shift_TR[index_hrf[iqTRo3]] = TR*.75
            endif
        endif
        lcround_tc = lcround_tc > 1
        goback212:
        functype_all_ud=0
        count_userdefined=0
        if total(lcfunc[index_present_ts]) gt 0 then idx_lcfunceq1=where(lcfunc eq 1,count_lcfunceq1)
        domore=0
        if n_elements(functype) eq 0 then begin
            domore=1
            functype = make_array(max_num_levels,/INTEGER,VALUE=-1)
            functype[index_present_ts]=0
            functypestr=make_array(max_num_levels,/STRING,VALUE='-1')
            ;functype_all_ud = 0
            ;count_userdefined = 0
            if total(lcfunc[index_present_ts]) gt 0 then begin
                goback212a:
                ;idx = where(lcfunc eq 1,count_lcfunceq1)
                if count_lcfunceq1 le 350 then begin
                    ;goose = effect_label[idx]
                    goose = effect_label[idx_lcfunceq1]
                    spider = count_lcfunceq1
                endif else begin
                    stat = get_button(['yes', 'go back','exit'],BASE_TITLE=whoamistr, $
                        TITLE='I am unable to display all these event types. Will the same assumed response be applied to all?')
                    if stat eq 1 then goto,goback21 else if stat eq 2 then return,!ERROR
                    goose = 'All'
                    spider = 1
                endelse
                labels = ['Boynton','SPM Gamma + derivatives','SPM "Cannonical"','Gamma with shifter','Boxcar','Trend','User Defined']
                types=types0[1:*]
                typesstr=typesstr0[1:*]
                scraparr = strarr(n_elements(labels),spider)
                for i=0,spider-1 do scraparr[*,i] = labels
                rtn = get_choice_list_many(goose,scraparr,TITLE='Please specify assumed responses.',/LABEL_TOP,/NO_BUTTONS_SCROLL, $
                    /SET_SAME,/GO_BACK)
                if rtn.special eq 'GO_BACK' then goto,goback21
                ;scraplist = rtn.list
                ;functype[idx] = types[scraplist]
                ;functypestr[idx]=typesstr[scraplist]
                functype[idx_lcfunceq1]=types[rtn.list]
                functypestr[idx_lcfunceq1]=typesstr[rtn.list]
            endif
        endif
        if total(lcfunc[index_present_ts]) gt 0 then begin
             index_userdefined=where(functype eq !USER_DEFINED,count_userdefined)
             if count_userdefined ne 0 then begin
                 if count_userdefined eq count_lcfunceq1 then functype_all_ud=1
                 if domore eq 1 then begin
                    if spider eq 1 then nfiles_userdefined=1 else nfiles_userdefined=count_userdefined
                    files_userdefined = strarr(nfiles_userdefined)
                    for i=0,nfiles_userdefined-1 do begin
                        title = 'Please select response file for '+effect_label[index_userdefined[i]]+'.'
                        get_dialog_pickfile,'*.dat',fi.path,title,filename,nregfiles,rtn_path
                        if filename eq 'GOBACK' then begin
                            i = i -1
                            goto,goback212a
                        endif else if filename eq 'EXIT' then return,!ERROR
                        files_userdefined[i] = filename
                    endfor
                 endif
             endif
        endif
        idx=where(tag_names(par1) eq 'HRF',cnt)
        if cnt ne 0 then par1.hrf=functypestr else par1=create_struct(par1,'hrf',functypestr)
        undefine,idx,cnt
        omit_trials_list = intarr(tot_eff)
        if lc_omit_trials eq !TRUE then begin ;Omit frames corresponding to specific trial types.
            rtn=select_files(effect_label[0:tot_eff-1],TITLE='Effects specifying frames to omit.',/GO_BACK,/ONE_COLUMN)
            if rtn.files[0] eq 'GO_BACK' then goto,goback21
            omit_trials_list = rtn.list
        endif
        all_eff = tot_eff
        if functype_all_ud eq 0 then begin
            index1 = where(functype eq !BOYNTON or functype eq !SPM_CANNONICAL or functype eq !BOXCAR_STIM,count1)
            if count1 ne 0 then begin
                conditions = *sng[max_num_levels_index].conditions
                stimlen_vs_t = *sng[max_num_levels_index].stimlen_vs_t
                for j=0,count1-1 do begin
                    eff = index1[j]
                    index = where(conditions eq eff,count)
                    if count eq 0 then begin
                        print,effect_label[eff]+' not found in '+evselect[max_num_levels_index]
                    endif else begin
                        scrap = stimlen_vs_t[index]
                        scrap = scrap[uniq(scrap,sort(scrap))]
                        i = n_elements(scrap)
                        if i gt 1 then begin
                            scrapstr = 'The following durations have been read for ' + effect_label[eff] + ':' + string(10B) + '    '
                            for jj=0,i-1 do scrapstr = scrapstr + strtrim(scrap[jj]) + ' '
                            lc_multiple_stimlen = !TRUE
                            print,evselect[max_num_levels_index]
                            print,scrapstr
                        endif
                    endelse
                endfor
                undefine,conditions,stimlen_vs_t
            endif
        endif
        goback22:
        lq_scale_globally = 'No' ;need this default
        if lc_multiple_stimlen eq !TRUE then begin 
            junk=get_button(['yes','no','go back'],BASE_TITLE='Do you wish the regressors to be scaled globally?', $
                TITLE='Note: Currently only Boynton, SPM Cannonical, and boxcar regressors may be scaled globally.'+string(10B) $
                +'With global scaling, the HRF of the longest duration trial will have the largest amplitude.'+string(10B) $
                +'Without global scaling, all HRFs will be the same amplitude regardless of duration.')
            if junk eq 2 then goto,goback2
            if junk eq 0 then lq_scale_globally = 'Yes' 
        endif
        ud1 = {U_D, $
            hrf:ptr_new(), $
            nhrf:0L $
            }
        ud = replicate({U_D},tot_eff)
        i = 0
        widget_control,/HOURGLASS
        for eff=0,tot_eff-1 do begin
            if functype[eff] eq !USER_DEFINED then begin
                ud_struct = readf_ascii_file(files_userdefined[i],/NOHDR)
                ud[eff].nhrf = ud_struct.NR
                scrap = ud_struct.data
                scrap = scrap/max(abs(scrap))
                ud[eff].hrf = ptr_new(scrap)
                if spider ne 1 then i=i+1
            endif
        endfor
    endif
    if lcpause eq 1 then begin
        eff_pause = get_button(effect_label,TITLE='Effect name of no-stimulus trials ')
        str = get_str(1,'Number of frames for T1 to equilibrate','3')
        pause_length = fix(str[0])
        if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='Always choose Yes to this question unless you have designed ' $
            +'the study such that this will yield a singular design matrix. If `Yes` yields a singular design matrix, choose `No`. ' $
            +'This might yield a valid nonsingular design matrix, but it will degrade accuracy.'
        lcpause_per_run = get_boolean('Estimate T1 effect for each run?',['Yes','No'])
    endif else begin
        eff_pause = -1
        pause_length = 0
        lcpause_per_run=-1
    endelse
    goback29:
    if lc_omit_frames eq 1 then begin
        tdim = max(stc2[*].tdim_all,index)
        nrun = stc2[index].n
        tdim_file = *stc2[index].tdim_file
        valid_frms_max = make_array(tdim,/FLOAT,VALUE=1.)
        rtn_get_frames = get_frames(tdim,nrun,tdim_file,len_hrf,valid_frms_max)
        omit_reference = rtn_get_frames.omit_reference
    endif
    nbehavcol = 0
    if evselect[0] ne 'NONE' then begin
        funclen = intarr(tot_eff)
        for eff=0,tot_eff-1 do begin
            case functype[eff] of
                !TIME_COURSE_EST: funclen[eff] = len_hrf[eff]*lcround_tc[eff]
                !BOYNTON: funclen[eff] = 1
                !FRISTON: funclen[eff] = 6
                !SPM_CANNONICAL: funclen[eff] = 1
                !GAMMA_DERIV: funclen[eff] = 2
                !BOXCAR_STIM: funclen[eff] = 1
                !TREND: funclen[eff] = 1
                !USER_DEFINED: funclen[eff] = 1
                -1: ;do nothing
                else: print,'Invalid function type in define_single_dsgn_matrix.'
            endcase
        endfor
        geffect_column = intarr(tot_eff)
        geffect_length = intarr(tot_eff)
        effect_labelp = effect_label
        geffp = 0
        gnmain=0
        for eff=0,tot_eff-1 do begin
            if eff ne eff_pause then begin
                geffect_column[geffp] = gnmain
                effect_labelp[geffp] = effect_label[eff]
                gnmain = gnmain + funclen[eff]
                geffect_length[geffp] = funclen[eff]
                geffp = geffp + 1
            endif
        endfor
        if lcpause eq 1 then begin ;Move "pause" effect to rightmost columns.
            effect_labelp[geffp] = effect_label[eff_pause]
            effect_label = effect_labelp
        endif
        delay_save = delay
        effect_label_save = effect_label
        lcfunc_save = lcfunc
        stimlen_save = stimlen
        if lcexclude_count ne count_present_ts then begin
            nbehavcol = nbehavcolts[max_num_levels_index]
            num_offsets_eff = lcround_tc
            effect_TR = TR/float(num_offsets_eff)
            effect_TR_save=effect_TR
            effect_shift_TR = geffect_shift_TR
            effect_shift_TR_save=effect_shift_TR

        ;endif
        ;START170719
        endif else begin
            effect_TR_save=TR
            effect_shift_TR_save=fltarr(max_num_levels)
        endelse

    endif

    ;pause={lcpause:lcpause,eff_pause:eff_pause,pause_length:pause_length,lcpause_per_run:lcpause_per_run,geffp:geffp}
    ;START170331
    pause={lcpause:lcpause,eff_pause:eff_pause,pause_length:pause_length,lcpause_per_run:lcpause_per_run}

    tdim_file = *stc2[max_num_levels_index].tdim_file
    tdim_sum = *stc2[max_num_levels_index].tdim_sum
    tdim = stc2[max_num_levels_index].tdim_all
    nrun = stc2[max_num_levels_index].n
    rtn_valid_frms=get_valid_frms(tdim,nrun,tdim_sum,tdim_file,txtselect[max_num_levels_index],concselect[max_num_levels_index], $
        lc_omit_frames,rtn_get_frames,omit_reference,init_skip,valid_frms_max)
    if rtn_valid_frms.msg eq 'ERROR' then return,!ERROR
    valid_frms = rtn_valid_frms.valid_frms
    if nbehavcol gt 0 then begin
        effect_column0 = geffect_column
        effect_length0 = geffect_length
        nmain = gnmain
        if total(nbehavcolts-nbehavcolts[0]) gt 0 then begin
            junk=get_button(['ok','exit - something wrong'],BASE_TITLE=whoamistr, $
                TITLE='Not all event files have the same number of behavioral columns.'+string(10B) $
                +'We will base our model on '+evselect[max_num_levels_index])
            if junk eq 1 then return,!ERROR
        endif
        goback33:
        if nbehavcol gt 1 then $
            junk = 'Fidl interprets these as behavioral regressors.' $
        else $
            junk = 'Fidl interprets this as a behavioral regressor.'
        nbehavcolstr = strtrim(indgen(nbehavcol)+4,2)
        rtn=select_files(nbehavcolstr,TITLE=evselect[max_num_levels_index]+string(10B)+'You have included numbers in columns: ' $
            +strjoin(nbehavcolstr,' ',/SINGLE)+string(10B)+junk+string(10B)+'You may also put numbers in additional columns.' $
            +string(10B)+'To skip a column use NA.'+string(10B)+string(10B)+'Please select columns to model', $
            BASE_TITLE='Behavioral regressors',/GO_BACK,/ONE_COLUMN,/CANCEL,/EXIT,MIN_NUM_TO_SELECT=-1)
        if rtn.files[0] eq 'GO_BACK' then goto,goback30 else if rtn.files[0] eq 'EXIT' then return,!ERROR
        gnbehavcol=rtn.count
        nbehavcol = rtn.count
        if nbehavcol gt 0 then begin
            ibehavcol = rtn.index
            nbehavcolmin = ibehavcol[nbehavcol-1]+1
            index = where(nbehavcolts lt nbehavcolmin,count)
            if count ne 0 then begin
                junk=get_button(['ok - model what is there','go back','exit - something wrong with these event files'], $
                    BASE_TITLE=whoamistr,TITLE='The following event files have fewer columns then what you have selected:' $
                    +string(10B)+string(10B)+strjoin(evselect[index],string(10B),/SINGLE))
                if junk eq 1 then goto,goback33 else if junk eq 2 then return,!ERROR
            endif
            ibehavcol = rtn.index
            bvstn_str = ['divide by largest absolute value', $
                         'subtract the mean', $
                         'subtract the mean, divide by largest absolute value', $
                         'subtract the mean, divide by the magnitude', $
                         'subtract the mean, divide by the sd', $
                         'no scale', $
                         'binary valued: 1 and -1']
            noscalei = 5
            binaryi = 6
            nbvstn_str = make_array(nbehavcol,/INT,VALUE=n_elements(bvstn_str))
            dbvstn_str = 1
            bvstrt_str = ['regressor for each timepoint of each condition', $
                          'single regressor for each condition', $
                          'regressor for each timepoint across all conditions']
            nbvstrt_str = n_elements(bvstrt_str)
            bgs_str = ['individually for each condition.','globally across all conditions.']
            nbgs_str = n_elements(bgs_str)
            bm_str = ['a boxcar.','the assumed response.']
            nbm_str = n_elements(bm_str)
            behav_vs_t = *sng[max_num_levels_index].behav_vs_t
            behav_vs_t_norm = make_array(nbehavcol,/INTEGER,VALUE=-1)
            behav_vs_t_reg_type = intarr(nbehavcol)
            behav_global_scaling = make_array(nbehavcol,/INT,VALUE=1)
            behav_label = ''
            behav_mult = intarr(nbehavcol)
            scrap_eff0 = intarr(tot_eff,nbehavcol,total_nsubjects)
            scrap_eff = intarr(tot_eff,nbehavcol)
            for m=0,total_nsubjects-1 do begin
                conditions = *sng[m].conditions
                behav_vs_t = *sng[m].behav_vs_t
                nbehavcol1 = sng[m].nbehavcol
                i1 = where(ibehavcol lt nbehavcol1,c1)
                if c1 ne 0 then begin
                    nbehavcol1 = c1
                    ib = ibehavcol[i1]
                    for eff=0,tot_eff-1 do begin
                        index_eff = where(conditions eq eff,count_eff)
                        if count_eff ne 0 then begin
                            for i=0,nbehavcol1-1 do begin
                                behav_col = reform(behav_vs_t[ib[i],*])
                                index = where(behav_col[index_eff] ne float(!UNSAMPLED_VOXEL),count)
                                if count ne 0 then begin
                                    if functype[eff] eq !TIME_COURSE_EST or functype[eff] eq !BOXCAR_STIM then begin
                                        scrap_eff0[eff,i,m] = 1
                                        scrap_eff[eff,i] = 1
                                    endif else begin
                                        scrap_eff0[eff,i,m] = 1
                                        scrap_eff[eff,i] = -1
                                    endelse
                                endif
                            endfor
                        endif
                    endfor
                endif
            endfor
            base_title = 'Behavioral regressor column '+ nbehavcolstr[ibehavcol]
            bl = 'behav' + nbehavcolstr[ibehavcol]
            bm_special = ''
            bvstn_special = ''
            bvstrt_special = ''
            bgs_special = ''
            bl_special = ''
            for i=0,nbehavcol-1 do begin
                idx = where(scrap_eff[*,i] ne 0,cnt)
                if cnt ne 0 then begin
                    goback39:
                    here39=0
                    junk = where(scrap_eff[*,i] eq -1,count_scrap)
                    if count_scrap ne 0 then  begin
                        if bm_special eq '' then begin
                            rtn_bm = identify_conditions_new(nbm_str,bm_str,1,base_title[i],1,0, $
                                'Behavioral regressors should be multiplied by',/SAMEFORALL)
                            if rtn_bm.special eq 'GO_BACK' then goto,goback33
                            bm_special = rtn_bm.special
                            here39=1
                        endif
                        behav_mult[i] = rtn_bm.index_conditions[0]
                    endif
                    goback40:
                    if bvstn_special eq '' then begin
                        behavcol = reform(behav_vs_t[i,*])
                        index = where(behavcol ne float(!UNSAMPLED_VOXEL),count)
                        minimum = min(behav_col,MAX=maximum)
                        iscrap = where(behav_col eq minimum,cnt1)
                        iscrap = where(behav_col eq maximum,cnt2)
                        junk = 1
                        if cnt1+cnt2 eq count then begin
                            goback40a:
                            junk=get_button(['yes','no','go back'],BASE_TITLE=base_title[i], $
                                TITLE='Do you want to do a psychophysiological interaction?')
                            if junk eq 2 then goto,goback39
                            if junk eq 0 then begin
                                behav_vs_t_norm[i] = binaryi
                                lcppi=1
                                if lc_rr eq 0 then begin
                                    lc_rr = 1
                                    rtn_get_reg = get_reg(fi,wd,dsp,help,voxwtfile_path,total_nsubjects,rr, $
                                        TITLE1='Please select regions for psychophysiological interaction.')
                                    if rtn_get_reg.msg eq 'EXIT' then return,!ERROR $
                                    else if rtn_get_reg.msg eq 'GO_BACK' then goto,goback40a
                                    lcvoxwt = rtn_get_reg.lcvoxwt
                                    voxwt_str = rtn_get_reg.voxwt_str
                                    ppi_nreg0=rr.nreg
                                    ppi_ireg0=*rr.index
                                endif else begin
                                    rtn = select_files(*rr.region_names,BASE_TITLE=base_title[i],/GO_BACK,/ONE_COLUMN, $
                                        TITLE='Please select regions for psychophysiological interaction.') 
                                    if rtn.files[0] eq 'GO_BACK' then goto,goback40a
                                    ppi_nreg0=rtn.count
                                    ppi_ireg0=*rtn.index
                                endelse
                                if n_elements(ppi_nreg) eq 0 then begin
                                    ppi_nreg = intarr(nbehavcol)
                                    ppi_ireg = intarr(nbehavcol,rr.nreg)
                                endif
                                ppi_nreg[i]=ppi_nreg0
                                ppi_ireg[i,0:ppi_nreg[i]-1]=ppi_ireg0
                            endif
                        endif
                        if junk eq 1 then begin
                            rtn_bvstn = identify_conditions_new(nbvstn_str[i],bvstn_str[0:nbvstn_str[i]-1],1,base_title[i],1,0, $
                                'How do you want the behavioral regressors scaled?',DEFAULT=dbvstn_str,/SAMEFORALL)
                            if rtn_bvstn.special eq 'GO_BACK' then begin
                                if here39 eq 1 then goto,goback39 else goto,goback30
                            endif
                            bvstn_special = rtn_bvstn.special
                        endif
                    endif
                    if behav_vs_t_norm[i] eq -1 then behav_vs_t_norm[i] = rtn_bvstn.index_conditions[0]
                    goback41:
                    if bvstrt_special eq '' then begin
                        rtn_bvstrt = identify_conditions_new(nbvstrt_str,bvstrt_str,1,base_title[i],1,0, $
                            'What type of behavioral regressor?',/SAMEFORALL)
                        if rtn_bvstrt.special eq 'GO_BACK' then goto,goback40
                        bvstrt_special = rtn_bvstrt.special
                        ;print,'rtn_bvstrt.index_conditions=',rtn_bvstrt.index_conditions
                    endif
                    behav_vs_t_reg_type[i] = rtn_bvstrt.index_conditions[0]
                    goback42:
                    lc_goback42=0
                    if behav_vs_t_reg_type[i] lt 2 and total(abs(scrap_eff[*,i])) gt 1 and behav_vs_t_norm[i] ne noscalei then begin
                        if bgs_special eq '' then begin
                            lc_goback42=1
                            rtn_bgs = identify_conditions_new(nbgs_str,bgs_str,1,base_title[i],1,0, $
                                'Behavioral regressors should be scaled',/SAMEFORALL)
                            if rtn_bgs.special eq 'GO_BACK' then goto,goback41
                            bgs_special = rtn_bgs.special
                            ;print,'rtn_bgs.index_conditions=',rtn_bgs.index_conditions
                        endif
                        behav_global_scaling[i] = rtn_bgs.index_conditions[0]
                    endif
                    goback43:
                    if behav_vs_t_reg_type[i] eq 2 then begin
                        scrap = 'behavior'
                        scraplabel = bl[i]
                        idx = 0
                        cnt = 1
                    endif else begin
                        scrap = effect_label
                        scraplabel = effect_label + '_' + bl[i]
                    endelse
                    if bl_special eq '' then begin
                        spider = get_str(cnt,scrap[idx],scraplabel[idx],TITLE=base_title[i],Label='Behavioral regressor names', $
                            /GO_BACK,SPECIAL='OK - use default names for all')
                        if spider[0] eq 'GO_BACK' then begin
                            if lc_goback42 eq 1 then goto,goback42 else goto,goback41
                        endif
                        if spider[0] eq 'SPECIAL' then begin
                            bl_special=spider[0]
                            spider = spider[1:*]
                        endif
                        scraplabel[idx] = spider
                    endif
                    behav_label = [behav_label,scraplabel]
                endif
            endfor
            behav_label = behav_label[1:*]
            geff_all_same_len = intarr(nbehavcol)
            idx = where(behav_vs_t_reg_type eq 2,cnt0)
            if cnt0 ne 0 then begin
                for m=0,total_nsubjects-1 do begin
                    num_trials = sng[m].num_trials
                    conditions = *sng[m].conditions
                    behav_vs_t = *sng[m].behav_vs_t
                    nbehavcol = sng[m].nbehavcol
                    i1 = where(ibehavcol lt nbehavcol,c1)
                    if c1 ne 0 then begin
                        nbehavcol = c1
                    endif
                    if nbehavcol gt 0 then begin
                        for trial=0,num_trials-1 do begin
                            eff = conditions[trial]
                            if eff ge 0 then begin
                                if total(scrap_eff0[eff,*,m]) gt 0 then begin
                                    for i=0,cnt0-1 do begin
                                        if geff_all_same_len[idx[i]] eq 0 then $
                                            geff_all_same_len[idx[i]] = geffect_length[eff] $
                                        else if geff_all_same_len[idx[i]] ne geffect_length[eff] then begin
                                            print,evselect[m]
                                            print,'geff_all_same_len=',geff_all_same_len[idx[i]],' idx[i]=',idx[i],$ 
                                                ' geffect_length=',geffect_length[eff],' eff=',eff,' trial=',trial
                                            stat=dialog_message('Effects with associated behavioral regressors must all be the same ' $
                                               +'length. Abort!',/ERROR)
                                            return,!ERROR
                                        endif
                                    endfor
                                endif
                            endif
                        endfor
                    endif
                endfor
            endif
            nmain = gnmain
            effect_column = geffect_column
            effect_length = geffect_length
            eco = effect_column
            elo = effect_length
            te = tot_eff
            noe = num_offsets_eff
            esTR = effect_shift_TR
            eTR = effect_TR
            spider = max_num_levels
            goose = indgen(n_elements(eco))
            index_present_ts1 = index_present_ts
            for i=0,nbehavcol-1 do begin
                if behav_vs_t_reg_type[i] eq 2 then begin
                    naddbehav = 1
                    effect_column = [effect_column,nmain]
                    effect_length = [effect_length,geff_all_same_len[i]]
                    index_present_ts = [index_present_ts,spider]
                    spider = index_present_ts[n_elements(index_present_ts)-1] + 1
                endif else begin
                    naddbehav = te
                    effect_column = [effect_column,eco+nmain]
                    effect_length = [effect_length,elo]
                    index_present_ts = [index_present_ts,(i+1)*max_num_levels+index_present_ts1]
                endelse
                nmain = total(effect_length)
                all_eff = all_eff + naddbehav
                lcfunc = [lcfunc,intarr(naddbehav)]
                delay = [delay,fltarr(naddbehav)]
                stimlen = [stimlen,fltarr(naddbehav)]
                tot_eff = tot_eff + naddbehav
                num_offsets_eff = [num_offsets_eff,noe]
                effect_shift_TR = [effect_shift_TR,esTR]
                effect_TR = [effect_TR,eTR]
            endfor
            effect_label = [effect_label,behav_label]
            effect_column0 = effect_column
            effect_length0 = effect_length
            b0={behav_label:behav_label,behav_global_scaling:behav_global_scaling,behav_vs_t_norm:behav_vs_t_norm, $
                bvstn_str:bvstn_str,behav_vs_t_reg_type:behav_vs_t_reg_type,behav_mult:behav_mult,ibehavcol:ibehavcol, $
                binaryi:binaryi,binaryi_set:binaryi_set,noscalei:noscalei,nbehavcolstr:nbehavcolstr}
        endif
    endif else if evselect[0] ne 'NONE' then begin
        effect_column0 = geffect_column
        effect_length0 = geffect_length
        nmain = total(geffect_length)
    endif
    if evselect[0] ne 'NONE' then begin
        geffect_label = effect_label
        gindex_present_ts = index_present_ts
    endif
    proc=1
    proceach=1
    if glm_conc eq 0 and total_nsubjects gt 1 then begin

        ;spawn,'cat /proc/cpuinfo | grep processor | wc -l',processor
        ;print,'processor=',processor
        ;if pref.cpu ne '' then superbird=100/pref.cpu else superbird=2
        ;proc=(processor/superbird)>1
        ;print,'superbird=',superbird
        ;proc=proc[0]<total_nsubjects
        ;proceach=ceil(float(total_nsubjects)/proc)
        ;print,'proc='+trim(proc)+' proceach='+trim(proceach)+' glms will be assigned to each processor'
        ;START170712
        gp=getproc(pref,total_nsubjects)
        proc=gp.proc
        proceach=gp.proceach
        undefine,gp

    endif
    if lc_trend eq 1 and lc_rr eq 1 then lcdetrend=1
    scratchdir = ''
    scratchdirstr=''
    cleanupstr=''
    if n_elements(times0) ne 0 then undefine,times0
    if lcdetrend eq 1 or lc_atlas eq 1 then begin
        scratchdir=strarr(proc)
        widget_control,/HOURGLASS
        for i=0,proc[0]-1 do begin
            scratchdir[i]=fidl_scratchdir()
            if scratchdir[i] eq 'ERROR' then return,!ERROR
        endfor
        scratchdirstr=' -scratchdir '+scratchdir
        cleanupstr = ' -clean_up'
        times0=strmid(scratchdir,transpose(strpos(scratchdir,'SCRATCH_')+8))
        times0=strmid(times0,0,transpose(strlen(times0)-1))
    endif
    if lc_rr eq 1 then begin
        if rr.nreg eq 0 then begin
            goback179:
            ggimgselect=''
            rrloop=0
            if gg.imgselect[0] ne '' then begin
                scrap=get_button(['yes','no, let me load them','go back'],TITLE='Do you want to use the 4dfps ' $
                    +'in your list as the region files?')
                if scrap eq 2 then begin
                    if total(lcfunc[index_present_ts]) gt 0 then goto,goback212 else goto,goback29
                endif
                if scrap eq 0 then begin
                    scrap=get_button(['yes','no','go back'],TITLE='Have you used the same region names across all subjects?' $
                        +string(10B)+"The region names in subject 1's region files are same as those used for all subjects.")
                    if scrap eq 2 then goto,goback179
                    if scrap eq 1 then $
                        ggimgselect=gg.imgselect $
                    else begin
                        rrloop = n_elements(gg.imgselect)/total_nsubjects
                        ggimgselect = gg.imgselect[indgen(rrloop)*total_nsubjects]
                    endelse
                endif
            endif
            rtn_get_reg = get_reg(fi,wd,dsp,help,voxwtfile_path,total_nsubjects,rr,ggimgselect)
            if rtn_get_reg.msg eq 'EXIT' then return,!ERROR else if rtn_get_reg.msg eq 'GO_BACK' then begin
                if gg.imgselect[0] ne '' then goto,goback179 else if total(lcfunc[index_present_ts]) gt 0 then goto,goback212 $
                    else goto,goback29
            endif
            if rrloop eq 0 then $
                rr_region_str = *rr.region_str $
            else begin
                rr_region_str = strarr(total_nsubjects)
                for i=0,total_nsubjects-1 do begin
                    rr_region_str[i] = ' -region_file '+strjoin(gg.imgselect[indgen(rrloop)*total_nsubjects+i],' ',/SINGLE)
                endfor
            endelse
            lcvoxwt = rtn_get_reg.lcvoxwt
            voxwt_str = rtn_get_reg.voxwt_str
        endif
        goback180:
        scrap=get_button(['single regressor','separate regressor for each run','go back'],TITLE='Regional regressors')
        if scrap eq 2 then goto,goback18
        lc_rr_separate = scrap
        goback180a:
        scrap=get_button(['scale 0 to 1','scale -1 to 1','do not scale','zero mean','zero mean, magnitude normalized', $
            'zero mean, zero slope','gio','gio with file','go back'],TITLE='Regional regressors')
        if scrap eq 9 then goto,goback180
        if scrap eq 0 then $
            rr.scale_str = ' -scale0to1' $
        else if scrap eq 1 then $
            rr.scale_str = ' -scale-1to1' $
        else if scrap eq 2 then $
            rr.scale_str = ' -donotscale' $
        else if scrap eq 3 then $
            rr.scale_str = ' -zeromean' $
        else if scrap eq 4 then $
            rr.scale_str = ' -zeromeanmagnorm' $
        else if scrap eq 5 then $
            rr.scale_str = ' -zeromeanslope' $
        else if scrap eq 6 then $
            rr.scale_str = ' -gio' $
        else begin
            if giowtfile_path eq '' then giowtfile_path = fi.path
            get_dialog_pickfile,'*.dat',giowtfile_path,'Please load gio weight file',giowtfile,ngiowtfile,giowtfile_path, $
                /MULTIPLE_FILES
            if ngiowtfile eq 0 then goto,goback180
            rr.scale_str = ' -gio '
            if ngiowtfile ne 0 then begin
                print,'giowtfile=',giowtfile
                rr.scale_str = rr.scale_str + strjoin(giowtfile,' ',/SINGLE)
            endif
        endelse
        goback180b:
        if total_identify_yes gt 0 then begin
            if total_identify_yes lt total_nsubjects then begin
                title='Some of your concs have run type identifiers.'+string(10B)+'For those that do, would you like to separate ' $
                    +'regional regressors by run type?'
            endif else begin
                title='Separate regional regressors by run type?'
            endelse
            scrap = get_button(['yes','no','go back'],TITLE=title)
            if scrap eq 2 then goto,goback180a
            rr_useidentify = abs(scrap-1)
        endif
        ppi_tsumrows = 0
        if evselect[0] ne 'NONE' then begin
            goback180c:
            rtn=select_files(effect_label[index_present_ts0],BASE_TITLE='Separate regional regressors by conditions', $
                MIN_NUM_TO_SELECT=-1,/GO_BACK,/ONE_COLUMN,TITLE='Leave blank to not separate')
            if rtn.files[0] eq 'GO_BACK' then begin
                if total_identify_yes gt 0 then goto,goback180b else goto,goback180a
            endif
            rr_useev_count = rtn.count
            if rr_useev_count gt 0 then begin
                rr_useev_list = intarr(max_num_levels)
                rr_useev_list[index_present_ts0] = rtn.list
                rr_useev_el = effect_label[index_present_ts0]
            endif
            goback180d:
            effect_column = geffect_column
            effect_length = geffect_length
            effect_label = geffect_label
            index_present_ts = gindex_present_ts
            if lcexclude_count eq count_present_ts then begin
                ;do nothing
            endif else if count_present_ts gt 0 then begin
                ppi = identify_conditions_new(n_elements(index_present_ts),effect_label[index_present_ts],rr.nreg,*rr.region_names, $
                    0,0,'Leave blank for no ppi regressors',/SKIP_CHECKS,/SET_UNSET, $
                    TOP_TITLE='Please select conditions for psychophysiological interactions')
                if ppi.special eq 'GO_BACK' then goto,goback180c
                ppi_idx = where(ppi.sumrows gt 0,ppi_cnt)
                ppi_tsumrows = total(ppi.sumrows)
            endif
        endif
        goback18d:
        rr_roi_str = rr.roi_str
        if rr.nreg ge 2 and ppi_tsumrows eq 0 then begin
            addsub = get_button(['None','Form ||A-B|| and ||A+B|| regressors','Form A-B and A+B regressors','go back'], $
                BASE_TITLE='Please select difference and sum regressors.',TITLE='These will be used in place of the two respective ' $
                +'regional regressors.'+string(10B)+'If you have no idea what this is then hit none.')
            if addsub eq 2 then begin
                if lcvoxwt eq 1 then goback='goback18c1' else goback='goback18c'
                rtn_get_reg = get_reg(fi,wd,dsp,help,voxwtfile_path,total_nsubjects,rr,GOBACK=goback)
                if rtn_get_reg.msg eq 'EXIT' then return,!ERROR else if rtn_get_reg.msg eq 'GO_BACK' then goto,goback18
                lcvoxwt = rtn_get_reg.lcvoxwt
                voxwt_str = rtn_get_reg.voxwt_str
            endif
            addsub_roi_str=''
            if addsub ne 0 then begin
                if addsub eq 1 then begin
                    addsub_roi_str = ' -addsubroinorm2 ' 
                    top_title = '||A-B|| and ||A+B|| regressors'
                endif else begin
                    addsub_roi_str = ' -addsubroinonorm2 '
                    top_title = 'A-B and A+B regressors'
                endelse
                abstr = strarr(rr.nreg)
                abstr[indgen(rr.nreg/2)*2]='A'
                abstr[indgen(rr.nreg/2)*2+1]='B'
                rtn_ic = identify_conditions_new(rr.nreg,*rr.region_names,rr.nreg,abstr,1,0, $
                    'Please select a region for each. Blank columns are ignored.',/SET_DIAGONAL,/SET_ORDER,TOP_TITLE=top_title)
                if rtn_ic.special eq 'GO_BACK' then goto,goback18d
                idx = *rr.index
                addsub_roi_str = addsub_roi_str + strjoin(trim(idx[rtn_ic.index_conditions[*,0]]+1),' ',/SINGLE)
                rr_roi_str = ''
                undefine,idx,abstr,top_title,rtn_ic
            endif
        endif
        spawn,!BINEXECUTE+'/fidl_timestr2',timestr
        scrap=*rr.region_str
        cmd=!BINEXECUTE+'/fidl_cov '+scrap[0]+rr.regnamestr+rr_roi_str+rr.scale_str+addsub_roi_str+' -out '+scratchdir[0] $
            +'fidl_'+timestr[0]+'.dat'
        print,cmd
        widget_control,/HOURGLASS
        spawn,cmd,result
        print,result
        if strpos(result[0],'Error') ne -1 or strpos(result[0],'Segmentation') ne -1 then begin
            stat=dialog_message(result,/ERROR)
            return,!ERROR
        endif
        scrap = strsplit(result[0],/EXTRACT)
        scrap = scrap[n_elements(scrap)-1]
        scrap1=readf_ascii_file(scrap,/HDR_ONLY)
        if scrap1.name eq 'ERROR' then return,!ERROR
        if ptr_valid(rr.region_names) then ptr_free,rr.region_names
        rr.region_names = ptr_new(scrap1.hdr)
        if scratchdir[0] eq '' then spawn,'rm -f '+scrap
    endif
    if voxwt_str[0] eq '' then voxwt_str = make_array(total_nsubjects,/STRING,VALUE='')
    er_file = make_array(total_nsubjects,/STRING,VALUE='')
    if lc_er eq 1 then begin
        if gg.extselect[0] eq '' then begin
            scrap=get_button(['Ok','Go back','Exit'],TITLE='You will be prompted to load in the files containing the external ' $
                +'regressors, one for each subject.'+string(10B)+string(10B)+'Regressors are in columns.'+string(10B)+'First line ' $
                +'is regressor names.'+string(10B)+'Length should equal the number of frames in the conc set.'+string(10B) $
                +'Regressors should be scaled exactly as they should appear in the design matrix.'+string(10B)+'x or X denotes a ' $
                +'frame to omit.')
            if scrap eq 2 then return,!ERROR else if scrap eq 1 then goto,goback18 
            goback20:
            gf = get_files(FILTER='*.ext',TITLE='Please enter regressor file filter.',EXTRALABEL='Enter NONE if none.')
            if gf.msg eq 'GO_BACK' then goto,goback18 else if gf.msg eq 'EXIT' then return,!ERROR
            undefine,er_file
            er_file = gf.files
            if er_file[0] eq 'NONE' then $
                lc_er = 0 $
            else begin
                er_file = match_files(concselect,er_file,TITLE='Please select regressor file for ',/GO_BACK)
                if er_file[0] eq 'GO_BACK' then goto,goback20
            endelse
        endif else begin
            undefine,er_file
            er_file = gg.extselect
        endelse
    endif
    goback30:
    giocount = 0
    lchpf=0
    lclpf=0
    if hipass eq 1 then begin
        goback301:
        scrap=['High pass filter','Low pass filter']
        if lctdim_same eq 1 then scrap=[scrap,'Select specific harmonics'] 
        str = get_str_bool_intermixed(n_elements(scrap),scrap,['yes','no'], $
            [string(!HIPASS_CUTOFF,FORMAT='(f7.4)'),'0.080','Wait for next window'],'','cutoff Hz', $
            TITLE='Temporal filter',LABEL='Selecting both high and low pass implements a bandpass filter.',/GO_BACK,/ONE_COLUMN)
        if str[0] eq 'GO_BACK' then goto,goback29 
        if lctdim_same eq 1 then begin
            if str[2] eq 0 then str[0]='SPECIAL'
        endif
        if str[0] ne 'SPECIAL' then begin
            if str[0] eq '0' then begin
                lchpf=1
                hpf_cut=str[2+lctdim_same]
                print,'HPF '+str[2+lctdim_same]
                hpf_num_freq = intarr(max_nrun)
            endif
            if str[1] eq '0' then begin
                lclpf=1
                lpf_cut=str[3+lctdim_same]
                print,'LPF '+str[3+lctdim_same]
                lpf_num_freq = intarr(max_nrun)
                lpf_idx0 = intarr(max_nrun)
            endif
            if lchpf eq 0 and lclpf eq 0 then $
                hipass=0 $
            else begin
                oll_fmin = fltarr(max_nrun)
                oll_t0 = intarr(max_nrun)
                oll_t1 = intarr(max_nrun)
            endelse
        endif else begin
            goose = 1
            if evselect[0] ne 'NONE' then begin
                goback302:
                rtn=select_files(effect_label[index_present_ts],TITLE='Separate frequencies by conditions',MIN_NUM_TO_SELECT=0, $
                    /GO_BACK,/ONE_COLUMN)
                if rtn.files[0] eq 'GO_BACK' then goto,goback301
                frequseev_count = rtn.count
                if frequseev_count gt 0 then begin
                    frequseev_list = intarr(max_num_levels)
                    frequseev_list[index_present_ts] = rtn.list
                    frequseev_el = effect_label[index_present_ts]
                    goose = 3
                endif
            endif
            goback304:
            goosestr = ['Adjusted for deleted frames','Not adjusted','Adjusted for deleted frames and task', $
                'Number of frames is the trial duration']
            adj_not_adjplustask = get_button([goosestr[0:goose],'go back'],BASE_TITLE='Please select.',TITLE='Frequency regressors')
            if adj_not_adjplustask eq (goose+1) then begin
                if evselect[0] ne 'NONE' then goto,goback302 else goto,goback301
            endif
            goback305:
            if goose eq 3 then begin
                run_trial = get_button(['column for each run','column for each trial','go back'],BASE_TITLE='Please select.', $
                    TITLE='Frequency regressors - design matrix')
                if run_trial eq 2 then goto,goback304
            endif
            goback306:
            if adj_not_adjplustask lt 3 then begin
                giofreqstartframe = get_str(1,'start frame','1',TITLE='Fequency regressors start on frame',LABEL='First frame is 1.', $
                    WIDTH=25,/GO_BACK)
                if giofreqstartframe[0] eq 'GO_BACK' then goto,goback305
                giofreqstartindex = fix(giofreqstartframe) - 1
                print,'giofreqstartindex=',giofreqstartindex
            endif
            goback31:
            if adj_not_adjplustask eq 1 then begin
                fmin = 1./((tdim_file[0]-giofreqstartindex)*TR)
            endif else if adj_not_adjplustask eq 0 or adj_not_adjplustask eq 2 then begin
                t0 = tdim_sum[0] + giofreqstartindex
                t1 = tdim_sum[1] - 1
                if t1 gt tdim-1 then t1 = tdim - 1
                if adj_not_adjplustask eq 0 then begin
                    idx = where(valid_frms[t0:t1],nidx)
                    fmin = 1./(nidx*TR)
                endif else begin
                    gf0=get_func(sng[max_num_levels_index],stc2[max_num_levels_index],pause,geffect_length,geffect_column,gnmain, $
                        lcfunc_save,count_present_ts,index_present_ts,widget_type,durdel,evselect[max_num_levels_index], $
                        concselect[max_num_levels_index],frequseev_list,max_num_levels,rr_useev_list,functype,lcusedur,len_hrf, $
                        lcround_tc,delay_save,ud,hrf_params,lq_scale_globally,whoamistr)
                    if gf0.msg eq 'ERROR' then $
                        return,!ERROR $
                    else if gf0.msg eq 'GO_BACK' then $
                        goto,goback304 $
                    else if gf0.msg eq 'BOTTOMOFLOOP' then $
                        goto,goback304
                    rtn_G=get_G(sng[max_num_levels_index],stc2[max_num_levels_index],pause,gf0,b0,delay_save,stimlen_save, $
                        effect_label_save,gnbehavcol,evselect,concselect,valid_frms,scrap_eff,frequseev_list,rr_useev_list,lcppi, $
                        functype,funclen,max_num_levels,whoamistr,lcround_tc_index,omit_reference,omit_trials_list, $
                        geff_all_same_len,lq_scale_globally,skipflag,talk,effect_TR_save,effect_shift_TR_save,-1)
                    frequseev_frms = rtn_G.frequseev_frms
                    frequseev_cbyrun = rtn_G.frequseev_cbyrun
                    rtn_gio = get_gio(frequseev_cbyrun)
                    if rtn_gio.msg eq 'ERROR' then return,!ERROR
                    frequseev_idx = rtn_gio.frequseev_idx
                    length = t1-t0+1
                    rtn_yes = get_yes(frequseev_frms[t0:t1],frequseev_idx[0],length,valid_frms[t0:t1])
                    if rtn_yes.msg eq 'ERROR' then return,!ERROR
                    count_yes = rtn_yes.count_yes
                    fmin = 1./(count_yes*TR)
                endelse
            endif else begin
                index = where(frequseev_list,count)
                if count eq 0 then begin
                    stat=dialog_message('Something wrong HERE100!',/ERROR) 
                    return,!ERROR
                endif
                freq_stimlen = stimlen[index]
                if total(abs(freq_stimlen-freq_stimlen[0])) ne 0 then begin
                    stat=dialog_message('All stimlens must be equal HERE101!',/ERROR)
                    return,!ERROR
                endif
                freq_stimlenframes = freq_stimlen[0]/TR
                fmin = 1./freq_stimlen[0]
            endelse
            fnyq = 1./(2.5*TR)
            gionfreq = fnyq/fmin
            giofreq = (indgen(gionfreq)+1.)*fmin
            index = where(giofreq le 1.,count)
            giofreqstr = strtrim(giofreq[index],2)
            rtn = select_files(giofreqstr,BASE_TITLE='Please select frequencies (Hz).',MIN_NUM_TO_SELECT=1, $
                TITLE='Currently, if the runs differ in length'+string(10B)+'the frequencies will differ but the'+string(10B) $
                +'selected harmonics will be the same.'+string(10B)+'If this is not what you desire, then see McAvoy.',/GO_BACK)
            if rtn.files[0] eq 'GO_BACK' then goto,goback304
            giolist = rtn.list
            gioindex = rtn.index
            giocount = rtn.count
            giofreqstr = rtn.files + 'Hz'
        
            freqperglm = giocount
            ;KEEP
            ;freqperglm = 1
            ;if giocount gt 1 then begin
            ;    goback311:
            ;    onefreqperglm = get_button(;['yes','no','go back'],BASE_TITLE='Please select.',TITLE='One frequency per GLM?')
            ;    if onefreqperglm eq 2 then goto,goback31
            ;    if onefreqperglm eq 1 then freqperglm = giocount else freqloop = giocount
            ;    goback312:
            ;    if adj_not_adjplustask le 2 then begin
            ;        freqnorm = get_button(['no','mean normalized','variance normalized','mean and variance normalized','go back'], $
            ;            BASE_TITLE='Please select.',TITLE='Normalize frequencies?')
            ;        if freqnorm eq 4 then goto,goback311
            ;    endif
            ;endif
        endelse
    endif
    if lc_rr eq 1 then begin
        goback50:
        if rr_useidentify eq 0 then begin
            if rr_useev_count eq 0 then begin
                if lc_rr_separate eq 0 then wallace=1 else wallace=max_nrun
                if evselect[0] ne 'NONE' then begin
                    tot_eff = tot_eff + rr.nreg
                    effect_label = [effect_label,*rr.region_names]
                    effect_length0 = [effect_length0,make_array(rr.nreg,/INTEGER,VALUE=wallace)]
                    effect_column0 = [effect_column0,nmain+indgen(rr.nreg)*wallace]
                    index_present_ts = [index_present_ts,indgen(rr.nreg)+max_num_levels]
                    nmain= nmain + rr.nreg*wallace
                    if ppi_cnt gt 0 then begin
                        tot_eff = tot_eff + ppi_tsumrows 
                        effect_label2 = strarr(ppi_tsumrows)
                        lizard = *rr.region_names
                        k=0
                        for i=0,ppi_cnt-1 do begin
                            for j=0,ppi.sumrows[ppi_idx[i]]-1 do begin
                                effect_label2[k] = effect_label[ppi.index_conditions[ppi_idx[i],j]]+'_'+lizard[ppi_idx[i]]
                                k=k+1
                            endfor
                        endfor
                        effect_label = [effect_label,effect_label2]
                        effect_column0 = [effect_column0,nmain+indgen(ppi_tsumrows)*wallace]
                        effect_length0 = [effect_length0,make_array(ppi_tsumrows,/INTEGER,VALUE=wallace)]
                        index_present_ts = [index_present_ts,indgen(ppi_tsumrows)+max_num_levels+rr.nreg]
                        nmain = nmain + ppi_tsumrows*wallace
                    endif
                endif else begin
                    tot_eff = rr.nreg
                    effect_label = *rr.region_names
                    nmain= rr.nreg*wallace
                    effect_length0 = make_array(rr.nreg,/INTEGER,VALUE=wallace)
                    effect_column0 = nmain+indgen(rr.nreg)*wallace 
                endelse
            endif else begin
                if lc_rr_separate eq 0 then begin
                    stat=dialog_message('This needs to be coded.',/ERROR)
                    return,!ERROR
                endif else begin
                    rr_useev_cbyrun0 = intarr(max_nrun,max_num_levels)
                    for i=0,max_num_levels-1 do if rr_useev_list[i] eq 1 then rr_useev_cbyrun0[*,i]=1
                    rtn_gio0 = get_gio(rr_useev_cbyrun0)
                    if rtn_gio0.msg eq 'ERROR' then return,!ERROR
                    ncondfreq0 = rtn_gio0.ncondfreq
                    ncondfreqc0 = rtn_gio0.ncondfreqc
                    frequseev_idx0 = rtn_gio0.frequseev_idx
                    rtn_get_ncondfreq2=get_ncondfreq2(ncondfreq0,rr_useev_cbyrun0,frequseev_idx0,effect_label)
                    if rtn_get_ncondfreq2.msg ne 'OK' then return,!ERROR
                    ncondfreqs0 = rtn_get_ncondfreq2.ncondfreqs
                    rr_region_names = *rr.region_names
                    rr_tot_eff = rr.nreg*ncondfreq0
                    rr_effect_length0 = intarr(rr_tot_eff)
                    rr_effect_label = strarr(rr_tot_eff)
                    rr_effect_column0 = intarr(rr_tot_eff)
                    j=0
                    for l=0,rr.nreg-1 do begin
                        for i=0,ncondfreq0-1 do begin
                            rr_effect_length0[j] = ncondfreqc0[i]
                            rr_effect_label[j] = rr_region_names[l]+'_'+ncondfreqs0[i]
                            if j gt 0 then rr_effect_column0[j] = rr_effect_column0[j-1] + rr_effect_length0[j-1]
                            j=j+1
                        endfor
                    endfor
                    if lcexclude_count ne count_present_ts then begin
                        nmain = nmain+rr.nreg*total(ncondfreqc0)
                        tot_eff = tot_eff+rr_tot_eff
                        effect_length0 = [effect_length0,rr_effect_length0]
                        effect_label = [effect_label,rr_effect_label]
                        effect_column0 = [effect_column,rr_effect_column0]
                    endif else begin
                        nmain = rr.nreg*total(ncondfreqc0)
                        tot_eff = rr_tot_eff
                        effect_length0 = rr_effect_length0
                        effect_label = rr_effect_label
                        effect_column0 = rr_effect_column0
                    endelse
                    undefine,rr_tot_eff,rr_effect_length0,rr_effect_label,rr_effect_column0
    
                    ;print,'**************** HEREA ********************'
                    ;print,'tot_eff=',tot_eff
                    ;print,'effect_length0=',effect_length0
                    ;print,'**************** HEREB ********************'
    
                endelse
            endelse
        endif else begin
            identify = *stc2[imax_nrun].identify
            rtn=get_ncondfreq(max_nrun,identify)
            ncondfreq0=rtn.ncondfreq
            ncondfreqc0=rtn.ncondfreqc
            ncondfreqi0=rtn.ncondfreqi
            ncondfreqs0=rtn.ncondfreqs
            ;print,'identify0=',identify
            ;print,'ncondfreq0=',ncondfreq0
            ;print,'ncondfreqc0=',ncondfreqc0
            ;print,'ncondfreqi0=',ncondfreqi0
            ;print,'ncondfreqs0=',ncondfreqs0
    
            if n_elements(nmain) eq 0 then nmain=0
            if n_elements(tot_eff) eq 0 then tot_eff=0
            tot_eff1 = rr.nreg*ncondfreq0
            effect_length1 = intarr(tot_eff1)
            rr_region_names = *rr.region_names
            if lc_rr_separate eq 0 then begin
                effect_length1[*]=1
                effect_label1=''
                for l=0,rr.nreg-1 do effect_label1 = [effect_label1,rr_region_names[l]+'_'+ncondfreqs0]
                effect_label1 = effect_label1[1:*]
                nmain = nmain+tot_eff1
            endif else begin
                effect_label1 = strarr(tot_eff1)
                j=0
                for l=0,rr.nreg-1 do begin
                    for i=0,ncondfreq0-1 do begin
                        effect_length1[j] = ncondfreqc0[i]
                        effect_label1[j] = rr_region_names[l]+'_'+ncondfreqs0[i]
                        j=j+1
                    endfor
                endfor
                nmain = nmain+rr.nreg*total(ncondfreqc0)
            endelse
            if tot_eff eq 0 then begin
                effect_length0=effect_length1
                effect_label=effect_label1
            endif else begin
                effect_length0=[effect_length0,effect_length1]
                effect_label=[effect_label,effect_label1]
            endelse
            tot_eff=tot_eff+tot_eff1
        endelse
        lcfunc = make_array(tot_eff,/INTEGER,VALUE=!BOYNTON)
    endif
    if n_elements(tot_eff) ne 0 then tot_eff0 = tot_eff
    if n_elements(effect_label) ne 0 then effect_label0 = effect_label
    goback60:
    if glm_conc eq 0 then begin
        if lc_contrast then begin
            if n_elements(contrast_glm) ne 0 then undefine,contrast_glm
            if n_elements(parf0) ne 0 then begin
                idx=where(tag_names(par0) eq 'CONTRAST_GLM',cnt)
                if cnt ne 0 then begin
                    contrast_glm=par0.contrast_glm
                    rtn=load_linmod(fi,dsp,wd,glm,help,ifh,FILENAME=contrast_glm,/STOPATCNORM)
                    nc=rtn.nc
                    contrast_labels=rtn.contrast_labels
                    c=rtn.c
                    cnorm=rtn.cnorm
                    undefine,rtn
                endif
            endif
            if n_elements(contrast_glm) eq 0 then begin
                print,'Setting up contrasts'
                nc=0
                if evselect[0] ne 'NONE' then begin
                    if lcexclude_count eq count_present_ts then begin
                        if n_elements(effect_label) ne 0 then begin
                            widget_control,/HOURGLASS
                            define_contrast_noglm,help,contrast_labels,nc,c,cnorm,nmain,tot_eff,effect_label,effect_length0,$
                                effect_TR,effect_shift_TR,stimlen,delay,lcfunc,hrf_param,nmain,hrf_model
                            tot_eff1=tot_eff
                        endif
                    endif else if count_present_ts gt 0 then begin
                        effect_column0 = effect_column0[index_present_ts]
                        effect_length0 = effect_length0[index_present_ts]
                        widget_control,/HOURGLASS
                        define_contrast_noglm,help,contrast_labels,nc,c,cnorm,nmain,n_elements(index_present_ts),$
                            effect_label[index_present_ts],effect_length0,effect_TR[index_present_ts],$
                            effect_shift_TR[index_present_ts],stimlen[index_present_ts],delay[index_present_ts],$
                            lcfunc[index_present_ts],hrf_param,nmain,hrf_model
                        tot_eff1=n_elements(index_present_ts)
                    endif
                endif else if n_elements(effect_label) ne 0 then begin
                    widget_control,/HOURGLASS
                    define_contrast_noglm,help,contrast_labels,nc,c,cnorm,nmain,tot_eff,effect_label,effect_length0,effect_TR, $
                        effect_shift_TR,stimlen,delay,lcfunc,hrf_param,nmain,hrf_model
                    tot_eff1=tot_eff
                endif
                if nc ne 0 then begin
                    glm0={General_linear_model}
                    glm0.nc=nc
                    if ptr_valid(glm0.c) then ptr_free,glm0.c
                    glm0.c=ptr_new(c)
                    if ptr_valid(glm0.cnorm) then ptr_free,glm0.cnorm
                    glm0.cnorm=ptr_new(cnorm)
                    if ptr_valid(glm0.contrast_labels) then ptr_free,glm0.contrast_labels
                    glm0.contrast_labels=ptr_new(contrast_labels)
                    glm0.N=nmain
                    glm0.tot_eff=tot_eff1
                    contrast_glm=pardir+'/'+parroot+'.glm'
                    glm_file = save_linmod(fi,dsp,wd,glm,help,stc,pref,glm_idx,glm_idx,swapbytes,FILENAME=contrast_glm,/DESIGN_ONLY,$
                        IFH=ifh2[0],GLM1=glm0)
                endif
            endif
            if n_elements(contrast_glm) ne 0 then begin 
                idx=where(tag_names(par1) eq 'CONTRAST_GLM',cnt)
                if cnt ne 0 then par1.contrast_glm=contrast_glm else par1=create_struct(par1,'contrast_glm',contrast_glm)
            endif
            undefine,idx,cnt
            if nc ne 0 then begin
                cnorm_ones = cnorm
                for i=0,nc-1 do begin
                    idx = where(cnorm_ones[*,i] ne 0.,cnt)
                    if cnt ne 0 then cnorm_ones[idx,i]=1.
                endfor
            endif
        endif
        goback61:
        if n_elements(R2str) ne 0 then undefine,R2str
        if n_elements(parf0) ne 0 then begin
            idx=where(tag_names(par0) eq 'R2',cnt)
            if cnt ne 0 then begin
                if par0.R2 eq 1 then R2str=' -R2' else R2str=''
            endif
        endif
        if analysis_type eq 0 then begin
            if n_elements(R2str) eq 0 then begin
                if total_nsubjects gt 1 then lizard='s' else lizard=''
                scrap = get_button(['yes','no','go back'],TITLE='In addition to the glm'+lizard+', do you also want R2 images?',$
                    BASE_TITLE='Please select')
                if scrap eq 0 then R2str=' -R2' else if scrap eq 1 then R2str='' else begin
                    if n_elements(ppi) ne 0 then goto,goback180d
                    if lc_dontexclude eq 0 then $
                        goto,goback60 $
                    else begin
                        if hipass eq 0 then begin
                            if lc_multiple_stimlen eq !TRUE then goto,goback22 else goto,goback211
                        endif
                        goto,goback30
                    endelse
                endelse
            endif
        endif

        ;idx=where(tag_names(par1) eq 'R2',cnt)
        ;if cnt ne 0 then par1.R2=R2str else par1=create_struct(par1,'R2',R2str ne '')
        ;undefine,idx,cnt
        ;START160520
        if n_elements(R2str) ne 0 then begin
            idx=where(tag_names(par1) eq 'R2',cnt)
            if cnt ne 0 then par1.R2=R2str else par1=create_struct(par1,'R2',R2str ne '')
            undefine,idx,cnt
        endif else R2str=''


        goback62:
        dummy=proc+total_nsubjects
        scrap = strarr(dummy)
        scraplabels = strarr(dummy)
        wallace0=''
        if total_nsubjects gt 1 then begin
            wallace0=trim(indgen(proc)+1)
            idx=where(strlen(wallace0) eq 1,cnt)
            if cnt ne 0 then wallace0[idx]='0'+wallace0[idx]
        endif
        scrap[0:proc-1] = 'script'+wallace0
        if n_elements(times0) eq 0 then begin
            times0=strarr(proc)
            for i=0,proc-1 do begin
                spawn,!BINEXECUTE+'/fidl_timestr2',timestr
                times0[i]=timestr
            endfor
        endif


        ;if gg.nlists eq 0 then $
        ;    scraplabels[0:proc-1] = 'compute_glm_'+times0+'.csh' $
        ;else begin
        ;    gr1 = get_root(gg.glm_list,'.')
        ;    scraplabels[0:proc-1] = 'compute_glm_'+strjoin(gr1.file,'_',/SINGLE)+'_'+times0+'.csh'
        ;    undefine,gr1
        ;endelse
        ;START160223
        ;if gg.glm_list[0] eq '' then $
        ;    scraplabels[0:proc-1] = 'compute_glm_'+times0+'.csh' $
        ;else begin
        ;    gr1 = get_root(gg.glm_list,'.')
        ;    scraplabels[0:proc-1] = 'compute_glm_'+strjoin(gr1.file,'_',/SINGLE)+'_'+times0+'.csh'
        ;    undefine,gr1
        ;endelse
        ;START160223
        scraplabels[0:proc-1] = 'compute_glm_'+times0+'.csh'


        if lc_concs eq 1 then ext='.' else ext='.4dfp.img'
        rtn_gr = get_root(concselect,ext,/KEEPPATH)

        ;scraplabels[1:total_nsubjects] = rtn_gr.file + '.glm'
        ;for i=1,total_nsubjects do scrap[i] = 'GLM '+strtrim(i,2)
        ;START160222
        scraplabels[proc:proc+total_nsubjects-1] = rtn_gr.file + '.glm'
        wallace0=''
        if total_nsubjects gt 1 then begin
            wallace0=trim(indgen(total_nsubjects)+1) 
            idx=where(strlen(wallace0) eq 1,cnt)
            if cnt ne 0 then wallace0[idx]='0'+wallace0[idx]
        endif
        scrap[proc:proc+total_nsubjects-1]='GLM'+wallace0
        names = get_str(dummy[0],scrap,scraplabels,WIDTH=100,TITLE='Please enter filenames.',/ONE_COLUMN,/GO_BACK, $
            FRONT=directory+'/',/REPLACE)
        if names[0] eq 'GO_BACK' then begin
            if analysis_type eq 0 then goto,goback61 else goto,goback60 
        endif
        script = fix_script_name(names[0:proc-1])
        rtn_gr = get_root(names[proc:*],'.glm')

        ;START170119
        ;roots = rtn_gr.file


    endif else $
        names = make_array(dummy+1,/STRING,VALUE='scratch.glm')
    uncompressstr=''
    ;LOOP
    lcfirstcomputable = 0
    mmm = 0
    hrf_model = -1
    if lcdetrend eq 1 then concdetrend = strarr(total_nsubjects)
    proceach0=0
    proceach1=0
    for m=0,total_nsubjects-1 do begin
        if glm_conc eq 0 then begin
            if proceach0 eq 0 then begin
                print,'m=',m,' proceach1=',proceach1
                print,'***************** OPENW '+script[proceach1]+' *****************'
                openw,lu,script[proceach1],/GET_LUN
                top_of_script,lu
                if nregion_files gt 0 then begin
                    region_str = ' $REGION_FILE'
                    print_files_to_csh,lu,nregion_files,region_files,'REGION_FILE','region_file'
                endif
            endif
        endif
        print,concselect[m]
        ;print,'NEXTNEXTNEXTNEXTNEXT ',concselect[m],' NEXTNEXTNEXTNEXTNEXT'
        tdim_file = *stc2[m].tdim_file
        tdim_sum = *stc2[m].tdim_sum
        tdim_sum_new = *stc2[m].tdim_sum_new
        nrun = stc2[m].n
        tdim = stc2[m].tdim_all
        identify = *stc2[m].identify
        if gg.replaceselect[0] ne '' then filenames=*stc3[m].filnam else filenames=*stc2[m].filnam
        rtn_valid_frms=get_valid_frms(tdim,nrun,tdim_sum,tdim_file,txtselect[m],concselect[m],lc_omit_frames,rtn_get_frames, $
            omit_reference,init_skip,valid_frms_max)
        if rtn_valid_frms.msg eq 'ERROR' then return,!ERROR
        valid_frms = rtn_valid_frms.valid_frms
        index = where(valid_frms eq 0.,count)
        exclude_frames_str = ''
        if count ne 0 then exclude_frames_str = ' -exclude_frames '+strjoin(strtrim(index+1,2),' ',/SINGLE)
        print,'number of invalid frames = ',count
        if count ne 0 then begin
            print,''
            print,'---------------------------'
            print,'invalid frames = ',index+1
        endif
        goback201:

;STARTHERE

        ;gf1=get_func(sng[m],stc2[m],pause,geffect_length,geffect_column,gnmain,lcfunc_save,count_present_ts,index_present_ts, $
        ;    widget_type,durdel,evselect[m],concselect[m],frequseev_list,max_num_levels,rr_useev_list,functype,lcusedur,len_hrf, $
        ;    lcround_tc,delay_save,ud,hrf_params,lq_scale_globally,whoamistr)
        ;if gf1.msg eq 'ERROR' then $
        ;    return,!ERROR $
        ;else if gf1.msg eq 'GO_BACK' then $
        ;    goto,goback60 $
        ;else if gf1.msg eq 'BOTTOMOFLOOP' then $
        ;    goto,bottomofloop
        ;START170331
        if evselect[0] ne 'NONE' then begin
            gf1=get_func(sng[m],stc2[m],pause,geffect_length,geffect_column,gnmain,lcfunc_save,count_present_ts,index_present_ts, $
                widget_type,durdel,evselect[m],concselect[m],frequseev_list,max_num_levels,rr_useev_list,functype,lcusedur,len_hrf, $
                lcround_tc,delay_save,ud,hrf_params,lq_scale_globally,whoamistr)
            if gf1.msg eq 'ERROR' then $
                return,!ERROR $
            else if gf1.msg eq 'GO_BACK' then $
                goto,goback60 $
            else if gf1.msg eq 'BOTTOMOFLOOP' then $
                goto,bottomofloop
        endif

        if not keyword_set(TRIALS) then num_trials0=0 else num_trials0=sng[m].num_trials
        for m1=-1,num_trials0-1 do begin
            nmain = 0
            all_eff = 0
            tot_eff = 0
            nbehavcol = 0
            if evselect[m] ne 'NONE' then begin 
                if gnbehavcol gt 0 then spider = scrap_eff0[*,*,m]
                rtn_G=get_G(sng[m],stc2[m],pause,gf1,b0,delay_save,stimlen_save,effect_label_save,gnbehavcol,evselect,concselect, $
                    valid_frms,spider,frequseev_list,rr_useev_list,lcppi,functype,funclen,max_num_levels,whoamistr,lcround_tc_index, $
                    omit_reference,omit_trials_list,geff_all_same_len,lq_scale_globally,skipflag,talk,effect_TR_save, $
                    effect_shift_TR_save,m1)
                if rtn_G.msg eq 'ERROR' then $
                    return,!ERROR $
                else if rtn_G.msg eq 'GO_BACK' then $
                    goto,goback60 $
                else if rtn_G.msg eq 'BOTTOMOFLOOP' then $
                    goto,bottomofloop
                frequseev_frms = rtn_G.frequseev_frms
                frequseev_cbyrun = rtn_G.frequseev_cbyrun
                frequseev_ntrials = rtn_G.frequseev_ntrials
                rr_useev_frms = rtn_G.rr_useev_frms
                rr_useev_cbyrun = rtn_G.rr_useev_cbyrun
                nmain = rtn_G.nmain
                effect_column = rtn_G.effect_column
                effect_length = rtn_G.effect_length
                effect_label = rtn_G.effect_label
                all_eff = rtn_G.all_eff
                lcfunc = rtn_G.lcfunc
                delay = rtn_G.delay
                stimlen = rtn_G.stimlen
                tot_eff = rtn_G.tot_eff

                ;START170120
                effect_TR=rtn_G.effect_TR
                effect_shift_TR=rtn_G.effect_shift_TR

                frames = *sng[m].frames
                conditions = *sng[m].conditions
                stimlenframes_vs_t = *sng[m].stimlenframes_vs_t
                index_present = *sng[m].index_present
                for i=0,n_elements(*sng[m].index_present)-1 do begin
                    idx = where(index_present[i] eq index_present_ts,cnt)
                    if cnt eq 0 then  index_present[i]=-1
                endfor
                idx = where(index_present ne -1,cnt)
                if cnt eq 0 then begin
                     scrap=get_button('ok',BASE_TITLE=evselect[m],TITLE='No nonexcluded event types are present.')
                     return,!ERROR
                endif
                index_present = index_present[idx]
                if gnbehavcol ne 0 then nbehavcol = rtn_G.nbehavcol
                idx = where(functype[index_present] ne !USER_DEFINED,cnt)
                if cnt ne 0 then begin
                    index_notuserdefined = -1
                    for i=0,cnt-1 do begin   
                        index_notuserdefined=[index_notuserdefined,indgen(funclen[index_present[idx[i]]])$
                            +effect_column[index_present[idx[i]]]]
                    endfor
                    index_notuserdefined = index_notuserdefined[1:*]
                endif
                undefine,rtn_G
            endif
            xdim = ifh2[m].matrix_size_1
            ydim = ifh2[m].matrix_size_2
            zdim = ifh2[m].matrix_size_3
            dxdy = ifh2[m].scale_1
            dz   = ifh2[m].scale_3
            tcomp = fix(total(valid_frms ne 0.))
            if lc_atlas then begin
                if gt4[m].nt4 eq 0 then $
                    t4str = ' -xform_file ' + strjoin(t4select[mmm:mmm+neach[m]-1],' ',/SINGLE)+t4_identify_str[m] $
                else $ 
                    t4str = ' -xform_file ' + strjoin(*gt4[m].t4,' ',/SINGLE)+' -t4 '+strjoin(trim(indgen(gt4[m].nt4)+1),' ',/SINGLE)
            endif
            if lc_er eq 1 then begin

                ;START180426
                print,'Reading ',er_file[m]

                rtn_er=readf_and_check_ascii_file(er_file[m],valid_frms,tdim,concselect[m],whoamistr)
                if rtn_er.msg eq 'GOBACK' then goto,goback201 else if rtn_er.msg eq 'ERROR' then return,!ERROR
                er = rtn_er.er
                rtn_assign_er=assign_er(er,G,effect_column,effect_length,effect_label,lcfunc,delay,stimlen,nmain,all_eff,tot_eff)
                nmain=rtn_assign_er.nmain
                all_eff=rtn_assign_er.all_eff
                tot_eff=rtn_assign_er.tot_eff
            endif
            if gg.replaceselect[0] ne '' then begin
                tdim_sum3=*stc3[m].tdim_sum
                tdim_sum_new3 = *stc3[m].tdim_sum_new
                nrun3=nrun<stc3[m].n
                tdim3=tdim_sum_new3[nrun3-1]
                valid_frms3=fltarr(tdim3)

                ;print,'here2 valid_frms3=',valid_frms3
                ;print,'here2 valid_frms=',valid_frms

                ;print,'tdim_sum3=',tdim_sum3
                ;print,'tdim_sum_new3=',tdim_sum_new3
                ;print,'tdim_sum=',tdim_sum
                ;print,'tdim_sum_new=',tdim_sum_new

                for i=0,nrun3-1 do begin

                    ;i1=tdim_sum_new3[i]<(tdim_sum3[i]+tdim_sum_new[i]-1-tdim_sum[i])
                    ;START170510
                    ;i1=(tdim_sum_new3[i]-tdim_sum3[i])<(tdim_sum_new[i]-tdim_sum[i])
                    len3=tdim_sum_new3[i]-tdim_sum3[i]
                    len=tdim_sum_new[i]-tdim_sum[i]
                    i1=len3<len

                    ;print,'tdim_sum3[i]=',tdim_sum3[i],' i1=',i1
                    ;print,'tdim_sum[i]=',tdim_sum[i],' tdim_sum_new[i]-1=',tdim_sum_new[i]-1

                    ;valid_frms3[tdim_sum3[i]:i1]=valid_frms[tdim_sum[i]:tdim_sum_new[i]-1]
                    ;START170510
                    valid_frms3[tdim_sum3[i]:tdim_sum3[i]+i1-1]=valid_frms[tdim_sum[i]:tdim_sum[i]+i1-1]
                    if len3 lt len then valid_frms[tdim_sum[i]+i1:tdim_sum_new[i]-1]=0.

                endfor
                idx3=where(valid_frms[0:tdim_sum_new[nrun3-1]-1] eq 1.,tcomp3)

                ;print,'here0 idx3=',idx3
                ;print,'here0 tcomp3=',tcomp3

                if tcomp3 ne fix(total(valid_frms3 ne 0.)) then begin
                    title=gg.replaceselect[m]+string(10B)+'tcomp3='+trim(tcomp3)+' fix(total(valid_frms3 ne 0.))=' $
                        +trim(fix(total(valid_frms3 ne 0.)))+' Must be equal.'
                    print,title
                    stat=get_button(['skip this one and continue','go back','exit'],BASE_TITLE='Big Problem',TITLE=title[0])
                    if stat eq 0 then $
                        goto,bottomofloop $
                    else if stat eq 1 then $
                        goto,goback60 $
                    else $
                        return,!ERROR
                endif
                tdim_file3=*stc3[m].tdim_file
            endif else begin
                nrun3=nrun
                tdim3=tdim
                tdim_file3=tdim_file
                valid_frms3=valid_frms
                tcomp3=tcomp
            endelse
            if lc_trend eq 1 or lcdetrend eq 1 then begin
                slope = dblarr(tdim3)
                stat=call_external(!SHARE_LIB,'_trend',nrun3,tdim3,tdim_file3,valid_frms3,slope,VALUE=[1,1,0,0,0])
                if stat eq 0L then begin
                    print,'Error in _trend'
                    return,!ERROR
                endif
                if lcdetrend eq 1 then begin
                    print,'*************** detrending bolds ************************'
                    tempGdetrend=float(slope)
                    Adetrend = fltarr(tcomp,2*nrun)
                    base_coldetrend = nrun
                    dbase_coldetrend = 1
                    trend_coldetrend = 0
                    dtrend_coldetrend = 1
                    j0 = 0
                    for file=0,nrun-1 do begin
                        t0 = tdim_sum[file]
                        t1 = tdim_sum_new[file] - 1
                        if t1 gt tdim-1 then t1 = tdim - 1
                        idx = where(valid_frms[t0:t1],nidx)
                        j1 = j0 + nidx
                        Adetrend[j0:j1-1,base_coldetrend] = 1.
                        base_coldetrend = base_coldetrend + dbase_coldetrend
                        slp = tempGdetrend[t0:t1]
                        Adetrend[j0:j1-1,trend_coldetrend] = slp[idx]
                        trend_coldetrend = trend_coldetrend + dtrend_coldetrend
                        j0 = j1
                    endfor
                    effect_columndetrend = [0,nrun]
                    effect_lengthdetrend = make_array(2,/INTEGER,VALUE=nrun)
                    effect_labeldetrend = ['Trend','Baseline']
                    all_effdetrend = 2
                    glm1detrend = {General_linear_model}
                    ;stimlendetrend,lcfuncdetrend,functypedetrend are undefined as they should be
                    set_glm1,glm1detrend,Adetrend,valid_frms,0,2*nrun,0,xdim,ydim,zdim,tdim,dxdy,dz,nrun,tcomp, $
                        0,TR,stimlendetrend,lcfuncdetrend,functypedetrend,funclendetrend, $
                        all_effdetrend,effect_columndetrend,effect_labeldetrend,effect_lengthdetrend

                    ;glm_filedetrend = save_linmod(fi,dsp,wd,glm,help,stc,pref,glm_idx,glm_idx,swapbytes, $
                    ;    FILENAME=scratchdir[proceach1]+roots[m]+'_detrend.glm',/DESIGN_ONLY,IFH=ifh2[m],GLM1=glm1detrend)
                    ;START170119
                    glm_filedetrend = save_linmod(fi,dsp,wd,glm,help,stc,pref,glm_idx,glm_idx,swapbytes, $
                        FILENAME=scratchdir[proceach1]+rtn_gr.file[m]+'_detrend.glm',/DESIGN_ONLY,IFH=ifh2[m],GLM1=glm1detrend)

                    if glm_filedetrend eq 'ERROR' then return,!ERROR

                    ;goose = scratchdir[proceach1]+'/'+roots[m]+'_detrend.csh'
                    ;START170119
                    goose = scratchdir[proceach1]+'/'+rtn_gr.file[m]+'_detrend.csh'

                    openw,lu1,goose,/GET_LUN
                    top_of_script,lu1
                    print_files_to_csh,lu1,nrun,filenames,'BOLD_FILES','bold_files',/NO_NEWLINE
                    printf,lu1,'set INPUT_GLM_FILE = "-input_glm_file '+glm_filedetrend+'"'
                    printf,lu1,'set OUTPUT_GLM_FILE = "-output_glm_file '+glm_filedetrend+'"'
                    if lc_atlas then begin
                        if gt4[m].nt4 eq 0 then begin
                            print_files_to_csh,lu1,neach[m],t4select[mmm:mmm+neach[m]-1],'T4_FILE','xform_file',/NO_NEWLINE
                        endif else begin 
                            print_files_to_csh,lu1,gt4[m].nt4,*gt4[m].t4,'T4_FILE','xform_file',/NO_NEWLINE
                            t4_identify_str[m] = ' -t4 '+strjoin(trim(indgen(gt4[m].nt4)+1),' ',/SINGLE)
                        endelse
                    endif else $
                        printf,lu1,'set T4_FILE
                    if fwhm gt 0. and lc_rr eq 0 then begin 
                        printf,lu1,'set FWHM = ','"','-fwhm ',fwhm,'"',FORMAT='(a,a,a,a,a)'
                        lc_gaussdetrend=1
                    endif else begin 
                        printf,lu1,'set FWHM'
                        lc_gaussdetrend=0
                    endelse
                    if mask ne '' and rr_region_str[m] eq '' then $
                        print_files_to_csh,lu1,1,mask,'MASK_FILE','mask',/NO_NEWLINE $
                    else $
                        printf,lu1,'set MASK_FILE'
                    mode = 4*lc_smoothness_estimate + 2.*lc_atlas + lc_gaussdetrend
                    printf,lu1,'set MODE = ','"','-mode ',mode,'"',FORMAT='(a,a,a,1x,i1,a)'
                    if rr_region_str[m] ne '' then detrendstr = ' -voxels' else begin
                        print,'SHOULD NOT BE HERE 160616'
                        scrap=get_button(['Exit'],BASE_TITLE='SHOULD NOT BE HERE 160616',TITLE='Contact McAvoy') 
                        return,!ERROR
                    endelse
                    printf,lu1,'$BIN/compute_glm2 $BOLD_FILES $INPUT_GLM_FILE $OUTPUT_GLM_FILE $T4_FILE $MASK_FILE $FWHM $MODE' $
                        +rr_region_str[m]+rr.roi_str+detrendstr+t4_identify_str[m]+atlas_str+' -detrend '+scratchdirstr[proceach1]
                    close,lu1
                    free_lun,lu1
                    spawn,'chmod 0777 '+ goose
                    widget_control,/HOURGLASS
                    spawn,'csh '+goose,result ;ADD THIS BACK IN
                    print,'result'
                    print,result
                    nfiles = n_elements(result)
                    if strpos(result[nfiles-1],'fidlError') ne -1 then begin
                        stat=dialog_message_long('ERROR',result)
                        return,!ERROR
                    endif
                    if nfiles ne nrun+1 then begin
                        print,'nfiles=',nfiles,' nrun=',nrun
                        stat = spawn_check_and_load(result,fi,wd,dsp,help,nfiles_loaded,files_loaded,stc)
                        return,!ERROR
                    endif
                    scraplabel = strsplit(result[nrun],/EXTRACT)
                    concdetrend[m] = scraplabel[n_elements(scraplabel)-1]
                endif
            endif
            if lc_rr eq 1 then begin
                spawn,!BINEXECUTE+'/fidl_timestr2',timestr
                if lcdetrend eq 0 then $ 
                    lizard = concselect[m] + atlas_str + t4str $
                else $ 
                    lizard = concdetrend[m] + atlas_str + atlas_str_conc[m] + ' -tdim '+strjoin(trim(tdim_file),' ',/SINGLE)
                cmd=!BINEXECUTE+'/fidl_cov -tc_files '+lizard+rr_region_str[m]+rr.regnamestr+rr_roi_str+exclude_frames_str $
                    +rr.scale_str+voxwt_str[m]+addsub_roi_str+' -out '+scratchdir[proceach1]+'fidl_'+timestr[0]+'.dat'
                print,cmd
                widget_control,/HOURGLASS
                spawn,cmd,result
                if strpos(result[0],'Error') ne -1 or strpos(result[0],'Segmentation') ne -1 then begin
                    stat=dialog_message(result,/ERROR)
                    return,!ERROR
                endif
                rr_file = strsplit(result[0],/EXTRACT)
                rr_file = rr_file[n_elements(rr_file)-1]

                ;rtn_rr=readf_and_check_ascii_file(rr_file,valid_frms,tdim,concselect[m])
                ;START170118
                rtn_rr=readf_and_check_ascii_file(rr_file,valid_frms,tdim,concselect[m],whoamistr)

                if rtn_rr.msg eq 'GOBACK' then goto,goback201 else if rtn_rr.msg eq 'ERROR' then return,!ERROR
                if scratchdir[0] eq '' then spawn,'rm -f '+rr_file
                if lcppi eq 1 then begin
                    rtn_assign_er=assign_er(rtn_rr.er,G,effect_column,effect_length,effect_label,lcfunc,delay,stimlen,nmain,all_eff, $
                        tot_eff)
                    nmain=rtn_assign_er.nmain
                    all_eff=rtn_assign_er.all_eff
                    tot_eff=rtn_assign_er.tot_eff
                    eco = effect_column
                    elo = effect_length
                    nmo = nmain
                    te = tot_eff
                    ii = 0
                    for i=0,nbehavcol-1 do begin
                        if ppi_nreg[i] gt 0 then begin
                            si = i*nmo
                            if behav_vs_t_reg_type[i] eq 2 then begin
                                naddbehav = ppi_nreg[i]
                                behav_matrix = G_behav[*,si:si+geff_all_same_len[i]-1]
                                G_ppi = fltarr(tdim,ppi_nreg[i]*geff_all_same_len[i])
                                o=0
                                for l=0,ppi_nreg[i]-1 do begin
                                    scrap1=float_er_data[ppi_ireg[i,l],*]
                                    for n=0,geff_all_same_len[i]-1 do begin
                                        G_ppi[*,o]=scrap1*behav_matrix[*,n]
                                        o=o+1
                                    endfor
                                    effect_label = [effect_label,behav_label[i]+'_'+*rr.region_names[ppi_ireg[i,l]]]
                                endfor
                                effect_column = [effect_column,nmain+indgen(ppi_nreg[i])*geff_all_same_len[i]]
                                effect_length = [effect_length,make_array(ppi_nreg[i],/INT,VALUE=geff_all_same_len[i])]
                            endif else begin
                                naddbehav = te*ppi_nreg[i]
                                behav_matrix = G_behav[*,si:si+nmo-1]
                                G_ppi = fltarr(tdim,ppi_nreg[i]*nmo)
                                o=0
                                for l=0,ppi_nreg[i]-1 do begin
                                    scrap1=float_er_data[ppi_ireg[i,l],*]
                                    for n=0,nmo-1 do begin
                                        G_ppi[*,o]=scrap1*behav_matrix[*,n]
                                        o=o+1
                                    endfor
                                    effect_column = [effect_column,nmain*(l+1)+eco]
                                    effect_length = [effect_length,elo]
                                    for p=0,tot_eff-1 do begin
                                        effect_label = [effect_label,behav_label[ii]+'_'+*rr.region_names[ppi_ireg[i,l]]]
                                        ii = ii + 1
                                    endfor
                                endfor
                            endelse
                            nmain = total(effect_length)
                            G = [[G],[G_ppi]]
                            all_eff = all_eff + naddbehav
                            lcfunc = [lcfunc,intarr(naddbehav)]
                            delay = [delay,fltarr(naddbehav)]
                            stimlen = [stimlen,fltarr(naddbehav)]
                            tot_eff = tot_eff + naddbehav
                        endif
                    endfor
                endif else begin
                    if rr_useidentify eq 0 or identify[0] eq '' then begin
                        rr_names = fix_script_name(rtn_rr.er.hdr)
                        if rr_useev_count eq 0 then begin
                            if lc_rr_separate eq 0 then begin
                                goose = rtn_rr.er.NF
                                G1 = [[transpose(float(rtn_rr.er.data))]]
                                wallace=1
                            endif else begin
                                goose=rtn_rr.er.NF*nrun
                                tempG=[[transpose(float(rtn_rr.er.data))]]
                                G1 = fltarr(tdim,goose)
                                slpcol=0
                                for l=0,rtn_rr.er.NF-1 do begin
                                    for i=0,nrun-1 do begin
                                        t0 = tdim_sum[i]
                                        t1 = tdim_sum_new[i] - 1
                                        if t1 gt tdim-1 then t1 = tdim - 1
                                        G1[t0:t1,slpcol]=tempG[t0:t1,l]
                                        slpcol=slpcol+1
                                    endfor
                                endfor
                                wallace=nrun
                            endelse
                            effect_label1=rr_names
                            effect_column1 = nmain+indgen(rtn_rr.er.NF)*wallace
                            effect_length1=make_array(rtn_rr.er.NF,/INTEGER,VALUE=wallace)
                            if evselect[m] ne 'NONE' then begin
                                G = [[G],[G1]]
                                effect_column = [effect_column,effect_column1]
                                effect_length = [effect_length,effect_length1]
                                effect_label = [effect_label,effect_label1]
                            endif else if lc_er eq 1 then begin
                                G = [[G1],[G]]
                                effect_column = [effect_column1,effect_column]
                                effect_length = [effect_length1,effect_length]
                                effect_label = [effect_label1,effect_label]
                            endif else begin
                                G = G1
                                effect_column = effect_column1
                                effect_length = effect_length1
                                effect_label = effect_label1
                            endelse
                            nmain = nmain + goose
                            tot_eff = tot_eff + rtn_rr.er.NF
                            all_eff = all_eff + rtn_rr.er.NF
                            rr_neff = rtn_rr.er.NF
                            if evselect[m] ne 'NONE' then begin
                                lcfunc = [lcfunc,intarr(rtn_rr.er.NF)]
                                delay = [delay,fltarr(rtn_rr.er.NF)]
                                stimlen = [stimlen,fltarr(rtn_rr.er.NF)]
                                effect_TR = [effect_TR,make_array(rtn_rr.er.NF,/FLOAT,VALUE=TR)]
                            endif
                            if ppi_cnt gt 0 then begin
                                if lc_rr_separate eq 0 then begin
                                    G2 = fltarr(tdim,ppi_tsumrows)
                                    k=0
                                    for i=0,ppi_cnt-1 do begin
                                        for j=0,ppi.sumrows[ppi_idx[i]]-1 do begin
                                            G2[*,k] = G[*,ppi.index_conditions[ppi_idx[i],j]]*G1[*,ppi_idx[i]]
                                            k=k+1
                                        endfor
                                    endfor
                                    wallace = 1
                                endif else begin
                                    G2 = fltarr(tdim,ppi_tsumrows*nrun)
                                    k=0
                                    l1=0
                                    for i=0,ppi_cnt-1 do begin
                                        for j=0,ppi.sumrows[ppi_idx[i]]-1 do begin
                                            for l=0,nrun-1 do begin
                                                t0 = tdim_sum[l]
                                                t1 = tdim_sum_new[l] - 1
                                                if t1 gt tdim-1 then t1 = tdim - 1
                                                G2[t0:t1,l1] = G[t0:t1,ppi.index_conditions[ppi_idx[i],j]]*tempG[t0:t1,ppi_idx[i]]
                                                l1=l1+1
                                            endfor
                                            k=k+1
                                        endfor
                                    endfor
                                    wallace = nrun 
                                endelse
                                G = [[G],[G2]]
                                effect_column = [effect_column,nmain+indgen(ppi_tsumrows)*wallace]
                                effect_length = [effect_length,make_array(ppi_tsumrows,/INTEGER,VALUE=wallace)]
                                effect_label2 = strarr(ppi_tsumrows)
                                k=0
                                for i=0,ppi_cnt-1 do begin
                                    for j=0,ppi.sumrows[ppi_idx[i]]-1 do begin
                                        effect_label2[k] = effect_label[ppi.index_conditions[ppi_idx[i],j]]+'_'+rr_names[ppi_idx[i]]
                                        k=k+1
                                    endfor
                                endfor
                                effect_label = [effect_label,effect_label2]
                                nmain = nmain + ppi_tsumrows*wallace 
                                tot_eff = tot_eff + ppi_tsumrows 
                                all_eff = all_eff + ppi_tsumrows 
                                rr_neff = rr_neff + ppi_tsumrows 
                                lcfunc = [lcfunc,intarr(ppi_tsumrows)]
                                delay = [delay,fltarr(ppi_tsumrows)]
                                stimlen = [stimlen,fltarr(ppi_tsumrows)]
                                effect_TR = [effect_TR,make_array(ppi_tsumrows,/FLOAT,VALUE=TR)]
                            endif
                        endif else begin
                            if lc_rr_separate eq 0 then begin
                                stat=dialog_message('This needs to be coded.',/ERROR)
                                return,!ERROR
                            endif else begin

                                ;rtn_Grr = get_Grr(stc2[m],sng[m],rr_useev_list,whoamistr)
                                ;START170719
                                rtn_Grr = get_Grr(stc2[m],sng[m],rr_useev_list,max_num_levels,whoamistr)

                                if rtn_Grr.msg eq 'ERROR' then return,!ERROR
                                rr_useev_cbyrun = rtn_Grr.rr_useev_cbyrun
                                rtn_gio = get_gio(rr_useev_cbyrun)
                                if rtn_gio.msg eq 'ERROR' then return,!ERROR
                                ncondfreq = rtn_gio.ncondfreq
                                ncondfreqc = rtn_gio.ncondfreqc
                                frequseev_idx = rtn_gio.frequseev_idx
                                rtn_get_ncondfreq2=get_ncondfreq2(ncondfreq,rr_useev_cbyrun,frequseev_idx,effect_label)
                                if rtn_get_ncondfreq2.msg ne 'OK' then return,!ERROR
                                ncondfreqi = rtn_get_ncondfreq2.ncondfreqi
                                ncondfreqs = rtn_get_ncondfreq2.ncondfreqs
                                tncondfreqc = total(rr_useev_cbyrun)
                                goose = rtn_rr.er.NF*tncondfreqc
                                G1 = fltarr(tdim,goose)
                                tempG=[[transpose(float(rtn_rr.er.data))]]
                                col = 0
                                for l=0,rtn_rr.er.NF-1 do begin
                                    used=intarr(tdim)
                                    iii = 0
                                    for i=0,ncondfreq-1 do begin
                                        for ii=0,ncondfreqc[i]-1 do begin
                                            t0 = tdim_sum[ncondfreqi[iii]]
                                            t1 = tdim_sum_new[ncondfreqi[iii]] - 1
                                            if t1 gt tdim-1 then t1 = tdim - 1
                                            length = t1-t0+1
                                            index1 = where(frames[*,0] ge t0 and frames[*,0] le t1,count1)
                                            if count1 eq 0 then begin
                                                scrap = 'count='+strtrim(count1,2)+' Should not be zero!'
                                                print,scrap
                                                stat=dialog_message(scrap,/ERROR)
                                            endif
                                            index2 = where(conditions[index1] eq frequseev_idx[i],count2)
                                            if count2 eq 0 then begin
                                                scrap = 'count='+strtrim(count2,2)+' Should not be zero!'
                                                print,scrap
                                                stat=dialog_message(scrap,/ERROR)
                                            endif
                                            index2 = index1[index2]
                                            for icol=0,count2-1 do begin
                                                istart = index2[icol]
                                                index3 = indgen(ceil(stimlenframes_vs_t[istart]))+frames[istart,0]
                                                if istart eq (sng[m].num_trials-1) then $
                                                    idx=where(index3 le t1,cnt) $
                                                else $
                                                    idx=where(index3 lt frames[(istart+1),0],cnt)
                                                if cnt ne 0 then begin
                                                    idx0=where(used[index3[idx]] eq 0,cnt0)
                                                    if cnt0 ne 0 then G1[index3[idx[idx0]],col]=tempG[index3[idx[idx0]],l] 
                                                    used[index3[idx[idx0]]]=1
                                                endif
                                            endfor
                                            col = col + 1
                                            iii = iii + 1
                                        endfor
                                    endfor
                                endfor
                                if all_eff gt 0 then begin
                                    tmp1 = effect_column
                                    tmp2 = effect_length
                                    tmp3 = effect_label
                                    G = [[G],[G1]]
                                    lcfunc = [lcfunc,intarr(ncondfreq*rtn_rr.er.NF)]
                                    delay = [delay,fltarr(ncondfreq*rtn_rr.er.NF)]
                                    stimlen = [stimlen,fltarr(ncondfreq*rtn_rr.er.NF)]
                                endif else begin 
                                    G = G1
                                    lcfunc = intarr(ncondfreq*rtn_rr.er.NF)
                                    delay = fltarr(ncondfreq*rtn_rr.er.NF)
                                    stimlen = fltarr(ncondfreq*rtn_rr.er.NF)
                                endelse
                                effect_column = intarr(all_eff+ncondfreq*rtn_rr.er.NF)
                                effect_length = intarr(all_eff+ncondfreq*rtn_rr.er.NF)
                                effect_label = strarr(all_eff+ncondfreq*rtn_rr.er.NF)
                                if all_eff gt 0 then begin
                                    effect_column[0:all_eff-1] = tmp1
                                    effect_length[0:all_eff-1] = tmp2
                                    effect_label[0:all_eff-1] = tmp3
                                endif
                                for l=0,rtn_rr.er.NF-1 do begin
                                    for i=0,ncondfreq-1 do begin
                                        if all_eff gt 0 then effect_column[all_eff]=effect_column[all_eff-1]+effect_length[all_eff-1]
                                        effect_length[all_eff] = ncondfreqc[i]
                                        effect_label[all_eff] = rr_names[l] + '_' + ncondfreqs[i]
                                        all_eff = all_eff + 1
                                    endfor
                                 endfor
                                 nmain = nmain + goose
                                 tot_eff = tot_eff + ncondfreq*rtn_rr.er.NF
                                 rr_neff = ncondfreq*rtn_rr.er.NF
                            endelse
                        endelse
                    endif else begin
                        rtn=get_ncondfreq(nrun,identify)
                        ncondfreq=rtn.ncondfreq
                        ncondfreqc=rtn.ncondfreqc
                        ncondfreqi=rtn.ncondfreqi
                        ncondfreqs=rtn.ncondfreqs
                        goose=rtn_rr.er.NF*ncondfreq
                        effect_column1 = intarr(goose)
                        effect_length1 = intarr(goose)
                        effect_label1 = strarr(goose)
                        tempG=[[transpose(float(rtn_rr.er.data))]]
                        if lc_rr_separate eq 0 then begin
                            G1 = fltarr(tdim,goose)
                            slpcol=0
                            for l=0,rtn_rr.er.NF-1 do begin
                                iii = 0
                                for i=0,ncondfreq-1 do begin
                                    for ii=0,ncondfreqc[i]-1 do begin
                                        t0 = tdim_sum[ncondfreqi[iii]]
                                        t1 = tdim_sum_new[ncondfreqi[iii]] - 1
                                        if t1 gt tdim-1 then t1 = tdim - 1
                                        G1[t0:t1,slpcol]=tempG[t0:t1,l]
                                        iii = iii + 1
                                    endfor
                                    slpcol=slpcol+1
                                endfor
                            endfor
                            j=0
                            for l=0,rtn_rr.er.NF-1 do begin
                                for i=0,ncondfreq-1 do begin
                                    effect_column1[j] = j
                                    effect_length1[j] = 1
                                    effect_label1[j] = rtn_rr.er.hdr[l]+'_'+ncondfreqs[i]
                                    j=j+1
                                endfor
                            endfor
                            nmain1 = goose
                        endif else begin
			    spider=rtn_rr.er.NF*total(ncondfreqc)
			    G1 = fltarr(tdim,spider)
			    slpcol=0
			    for l=0,rtn_rr.er.NF-1 do begin
			        iii = 0
			        for i=0,ncondfreq-1 do begin
				    for ii=0,ncondfreqc[i]-1 do begin
				        t0 = tdim_sum[ncondfreqi[iii]]
				        t1 = tdim_sum_new[ncondfreqi[iii]] - 1
				        if t1 gt tdim-1 then t1 = tdim - 1
				        G1[t0:t1,slpcol]=tempG[t0:t1,l]
				        iii = iii + 1
				        slpcol=slpcol+1
				    endfor
			        endfor
			    endfor
			    j=0
			    jj=nmain
			    for l=0,rtn_rr.er.NF-1 do begin
			        for i=0,ncondfreq-1 do begin
				    effect_column1[j] = jj
				    effect_length1[j] = ncondfreqc[i]
				    effect_label1[j] = rtn_rr.er.hdr[l]+'_'+ncondfreqs[i]
				    j=j+1
				    jj=jj+ncondfreqc[i]
			        endfor
			    endfor
			    nmain1 = spider
		        endelse
		        if all_eff gt 0 then begin
			    G = [[G],[G1]]
			    effect_column = [effect_column,effect_column1]
			    effect_length = [effect_length,effect_length1]
			    effect_label = [effect_label,effect_label1]
			    lcfunc = [lcfunc,intarr(goose)]
			    delay = [delay,fltarr(goose)]
			    stimlen = [stimlen,fltarr(goose)]
		        endif else begin
			    G = G1
			    effect_column = effect_column1
			    effect_length = effect_length1
			    effect_label = effect_label1
			    lcfunc = intarr(goose)
			    delay = fltarr(goose)
			    stimlen = fltarr(goose)
		        endelse
		        nmain = nmain+nmain1
		        tot_eff = tot_eff+goose
		        all_eff = all_eff+goose
		        rr_neff = rtn_rr.er.NF*ncondfreq
		    endelse
	        endelse
	        if n_elements(delay) eq 0 then delay = fltarr(tot_eff)
	        if n_elements(stimlen) eq 0 then stimlen = fltarr(tot_eff)
	    endif
	    if evselect[m] ne 'NONE' then begin
	        if total(lcfunc) eq 0 then begin
		    if lcinter eq !TRUE then begin
		        done = !FALSE
		        labs = strarr(tot_eff^2 + 1)
		        for i=0,tot_eff-1 do begin
			    for j=0,tot_eff-1 do labs[i*tot_eff+j] = effect_label[i] + '_X_' + effect_label[j]
		        endfor
		        labs[tot_eff^2] = 'Done'
		        repeat begin
			    lidx = get_button(labs,TITLE='Effects involved in this set of interaction terms')
			    if(lidx lt tot_eff^2) then begin
			         eff1 = lidx/tot_eff
			         eff2 = lidx - eff1*tot_eff
			         leff1 = effect_length[eff1]
			         leff2 = effect_length[eff2]
			         mask = fltarr(nmain)
			         mask[effect_column[eff1]:effect_column[eff1]+leff1-1] = 1
			         mask[effect_column[eff2]:effect_column[eff2]+leff2-1] = 1
			         inter_code = lonarr(tdim)
			         inter_code[*] = -1
			         for t=0,tdim-1 do begin
				     if(total(G[t,*]*mask) gt 1) then begin
				         ;There is an interaction at this frame.
				         row = G[t,*]*mask ne 0.
				         n1 = where(row[effect_column[eff1]:effect_column[eff1]+leff1-1])
				         n2 = where(row[effect_column[eff2]:effect_column[eff2]+leff2-1])
				         for i=0,n_elements(n1)-1 do $
					     if n1[i] ge 0 then inter_code[t] = inter_code[t] + n1[i]*leff1^i
				         for i=0,n_elements(n2)-1 do $
					     if n2[i] ge 0 then inter_code[t] = inter_code[t] + n2[i]*leff2^(i+3)
				     endif
			         endfor
			         unique_idxs = uniq(inter_code,sort(inter_code))
			         unique_codes = inter_code[unique_idxs]
			         max_inter = n_elements(unique_codes) - 1
			         inter_idx = lonarr(max_inter)
			         interaction = bytarr(tdim,max_inter)
			         num_code = 0
			         for t=0,tdim-1 do begin
				     if(inter_code[t] ge 0) then begin
				         idx = where(unique_codes eq inter_code[t]) - 1
				         interaction(t,idx) = 1
				     endif
			         endfor
			         ;Now compress the interaction submatrix.
			         num_inter = max_inter
			         ;Insert interactions into the design matrix.
			         x = fltarr(tdim,nmain + num_inter)
			         x[*,0:nmain-1] = G
			         x[*,nmain:nmain+num_inter-1] = interaction[*,0:num_inter-1]
			         G = x
			         nmain = nmain + num_inter
			         tmp1 = effect_column
			         tmp2 = effect_length
			         tmp3 = effect_label
			         effect_column = intarr(all_eff+1)
			         effect_length = intarr(all_eff+1)
			         effect_label = strarr(all_eff+1)
			         effect_column[0:all_eff-1] = tmp1
			         effect_length[0:all_eff-1] = tmp2
			         effect_label[0:all_eff-1] = tmp3
			         effect_column[all_eff] = effect_column[all_eff-1] + effect_length[all_eff-1]
			         effect_length[all_eff] = num_inter
			         effect_label[all_eff] = labs[lidx]
			         all_eff = all_eff + 1
			    endif else begin
			        done = !TRUE
			    endelse
		        endrep until done eq !TRUE
		    endif
	        endif
	    endif
	    len_hipass = 0
	    hpf_len = 0
	    lpf_len = 0
	    if hipass eq 1 then begin
	        if giocount eq 0 then begin
	            t0 = 0
		    for file=0,nrun-1 do begin
		        t1 = t0 + tdim_file[file] - 1
		        if t1 gt tdim-1 then t1 = tdim - 1
		        idx = where(valid_frms[t0:t1],nidx)
		        oll_fmin[file] = 1./(nidx*TR)
		        oll_t0[file] = t0
		        oll_t1[file] = t1
		        t0 = t0 + tdim_file[file]
		        undefine,idx
		    endfor
		    if lchpf eq 1 then begin
		        hpf_num_freq[0:nrun-1] = round(hpf_cut/oll_fmin[0:nrun-1]) 
		        hpf_len = 2*total(hpf_num_freq[0:nrun-1])
		    endif
		    if lclpf eq 1 then begin
		        fnyq = 1./(2.5*TR)
		        nfreq = fnyq/oll_fmin[0:nrun-1]
		        for file=0,nrun-1 do begin
			    freq = (indgen(nfreq[file])+1.)*oll_fmin[file]
			    idx = where(freq le 1.,cnt)
			    idx = where(freq[idx] ge lpf_cut,cnt)
			    lpf_num_freq[file] = cnt
			    lpf_idx0[file] = idx[0]
			    undefine,idx
		        endfor
		        lpf_len = 2*total(lpf_num_freq[0:nrun-1])
		        undefine,nfreq,freq,cnt,cnt1
		    endif
		    len_hipass = hpf_len+lpf_len
	        endif else begin
		    if frequseev_count eq 0 then $ 
		        len_hipass = freqperglm*2*nrun $
		    else begin
		        if run_trial eq 0 then $
			    len_hipass = freqperglm*2*total(frequseev_cbyrun) $
		        else $
			    len_hipass = freqperglm*2*frequseev_ntrials
	            endelse
	        endelse
	    endif
	    n_nuisance = len_hipass
	    nuisance_col = nmain
            slpcol = nuisance_col
            dtrend_col = 0
            if lc_trend then begin
                n_nuisance = n_nuisance + nrun
                trend_col = slpcol
                dtrend_col = 1
                tempG=float(slope)
            endif
            dbase_col = 0
            nbaseline=0
            if lc_baseline then begin
                idx=where(tdim_file gt 1,cnt)
                if cnt eq 0 then $
                    nbaseline=1 $
                else begin
                    nbaseline=nrun
                    dbase_col = 1
                endelse
                n_nuisance = n_nuisance + nbaseline
                base_col = slpcol + dtrend_col*nrun
            endif
            if lc_trend eq 1 or lcdetrend eq 1 then undefine,slope

            ;START170118
            ;if lc_glob then n_nuisance = n_nuisance + nglob*nrun

            N = nmain + n_nuisance
            A = fltarr(tcomp,N)
            nbase_col=0
            ntrend_col=0
            j0 = 0
            for file=0,nrun3-1 do begin
                t0 = tdim_sum[file]
                t1 = tdim_sum_new[file] - 1
                if t1 gt tdim-1 then t1 = tdim - 1
                idx = where(valid_frms[t0:t1],nidx)
                if nidx ne 0 then begin
                    j1 = j0 + nidx
                    if n_elements(G) gt 0 then A[j0:j1-1,0:nmain-1] = G[idx+t0,*]
                    if lc_baseline then begin
                        A[j0:j1-1,base_col] = 1.
                        base_col=base_col+dbase_col
                        nbase_col=nbase_col+1
                    endif
                    if lc_trend then begin
                        if gg.replaceselect[0] ne '' then begin
                            t0=tdim_sum3[file]
                            t1=tdim_sum_new3[file]-1
                            if t1 gt tdim3-1 then t1=tdim3-1
                            idx=where(valid_frms3[t0:t1],nidx)
                        endif
                        if nidx ne 0 then begin
                            slp=tempG[t0:t1]
                            A[j0:j1-1,trend_col]=slp[idx]
                        endif
                        trend_col=trend_col+dtrend_col
                        ntrend_col=ntrend_col+1
                    endif
                    j0 = j1
                endif
            endfor
	    if n_elements(G) gt 0 then undefine,G
	    if lc_trend then undefine,tempG
	    slpcol = slpcol + nbase_col + ntrend_col
	    lenslp = slpcol - nuisance_col
	    if lenslp gt 0 then begin
	        if lc_trend then begin
		    if all_eff gt 0 then begin
		        effect_column = [effect_column,effect_column[all_eff-1]+effect_length[all_eff-1]]
		        effect_length = [effect_length,nrun]
		        effect_label = [effect_label,'Trend']
		    endif else begin
		        effect_column = 0
		        effect_length = nrun
		        effect_label = 'Trend'
		    endelse
		    all_eff = all_eff + 1
	        endif
	        if lc_baseline then begin
		    if all_eff gt 0 then begin
		        effect_column = [effect_column,effect_column[all_eff-1]+effect_length[all_eff-1]]
		        effect_length = [effect_length,nbaseline]
		        effect_label = [effect_label,'Baseline']
		    endif else begin
		        effect_column = 0
		        effect_length = nbaseline
		        effect_label = 'Baseline'
		    endelse
		    all_eff = all_eff + 1
	        endif
	    endif
	    ;Add exponential confound.
	    ;;;j = 0
	    ;;;k = 0
	    ;;;t0 = 0
	    ;;;for file=0,nrun-1 do begin
	    ;;;    t1 = t0 + tdim_file[file] - 1
	    ;;;    if(t1 gt tdim-1) then $
	    ;;;        t1 = tdim - 1
	    ;;;    t = 0.
	    ;;;    for i=t0,t1 do begin
	    ;;;        if(valid_frms(k) gt 0) then begin
	    ;;;            A(j,slpcol) = 1. - exp((-t/15.))
	    ;;;            j = j + 1
	    ;;;            t = t + TR
	    ;;;        endif
	    ;;;        k = k + 1
	    ;;;    endfor
	    ;;;    slpcol = slpcol + 1  ; **** Patch ****
	    ;;;    t0 = t0 + tdim_file[file]
	    ;;;endfor
	    ;;;tmp1 = effect_column
	    ;;;tmp2 = effect_length
	    ;;;tmp3 = effect_label
	    ;;;effect_column = intarr(all_eff+1)
	    ;;;effect_length = intarr(all_eff+1)
	    ;;;effect_label = strarr(all_eff+1)
	    ;;;effect_column[0:all_eff-1] = tmp1[0:all_eff-1]
	    ;;;effect_length[0:all_eff-1] = tmp2[0:all_eff-1]
	    ;;;effect_label[0:all_eff-1] = tmp3[0:all_eff-1]
	    ;;;effect_column[all_eff] = effect_column[all_eff-1] + effect_length[all_eff-1]
	    ;;;effect_length[all_eff] = nrun
	    ;;;effect_label[all_eff] = 'Exponential'
	    ;;;all_eff = all_eff + 1
	    if n_elements(functype) ne 0 then functype0 = functype
	    if n_elements(funclen) ne 0 then funclen0 = funclen
	    if all_eff gt 0 then begin
	        lcexclude = intarr(all_eff)
	        if lc_dontexclude eq 0 then lcexclude[0:max_num_levels-1] = lcexclude_save
	        effp = 0
	        colp = 0
	        col1p = 0
	        tot_effp = tot_eff
	        all_effp = all_eff
	        eff_col = 0
	        deleted = intarr(all_eff)
	        ndel=0
	        del = intarr(N)
	        for eff=0,all_eff-1 do begin
		    col1 = effect_column[eff]
		    col2 = col1 + effect_length[eff] - 1
		    ndel_eff = 0
		    for col = col1,col2 do begin
		        if total(A[*,col]^2) eq 0. or lcexclude[eff] eq 1 then begin
			    print,col,effect_label(eff),FORMAT='("**** Deleting column #",i3," (",a,") "," from design matrix. ****")'
			    ndel_eff = ndel_eff + 1
			    del[ndel] = col
			    ndel=ndel+1
		        endif else begin
			    colp = colp + 1
		        endelse
		    endfor
		    if ndel_eff lt effect_length[eff] then begin
		        effect_length[eff] = effect_length[eff] - ndel_eff
		        effect_column[eff] = eff_col
		        eff_col = eff_col + effect_length[eff]
		        effp = effp + 1
		    endif else begin
		        deleted[eff] = 1
		        if eff lt tot_eff then tot_effp = tot_effp - 1
		        all_effp = all_effp - 1
		    endelse
	        endfor
	        if talk eq 1 then begin
		    str = ''
		    index = where(deleted eq 1,count)
		    if count ne 0 then begin
		        str = str+'Deleted effects'+string(10B)
		        for i=0,count-1 do str = str + '    ' + effect_label[index[i]] + string(10B)
		    endif
		    index = where(deleted eq 0,count)
		    if count ne 0 then begin
		        str = str+'Kept effects'+string(10B)
		        for i=0,count-1 do str = str + '    ' + effect_label[index[i]] + string(10B)
		    endif
		    special = 'Talk off'
		    stat=dialog_message_long(evselect[m],str,SPECIAL=special)
		    if stat eq 'EXIT' then return,!ERROR $
		    else if stat eq 'GOBACK' then goto,goback60 $
		    else if stat eq special then talk=0
	        endif
	        if all_effp gt 0 then begin
		    idx = where(deleted eq 0,cnt)
		    effect_column = effect_column[idx]
		    effect_length = effect_length[idx]
		    effect_label = effect_label[idx]
	        endif
	        if tot_effp gt 0 then begin
		    idx = idx[0:tot_effp-1]
		    lcfunc = lcfunc[idx]
		    delay = delay[idx]
		    stimlen = stimlen[idx]
		    if n_elements(functype) ne 0 then functype0 = functype[idx]
		    if n_elements(funclen) ne 0 then funclen0 = funclen[idx]
	        endif
	        all_eff = all_effp
	        tot_eff = tot_effp
	        ;A = Ap[*,0:N-1]
	        ;START140710
	        ;A = A[*,Acol[1:*]]
	        ;START140715
	        ;KEEP
	        ;N0 = N
	        if ndel gt 0 then begin
		    del[ndel] = N
		    i1=del[0]
		    for i=0,ndel-1 do begin
		        for j=del[i]+1,del[i+1]-1 do begin
			    A[*,i1] = A[*,j]
			    i1=i1+1
		        endfor
		    endfor
		    nmain = nmain - ndel
		    N = N - ndel
		    slpcol = slpcol - ndel
	        endif
	    endif    
	    ;size_A = size(A)
	    ;print,'size_A=',size_A
	    ;for i=0,size_A[1]-1 do begin
	    ;    print,'i=',i
	    ;    ;print,transpose(A[i,*])
	    ;endfor
	    ;print,'************************ here6 ****************************'
	    if freqloop gt 1 then begin 
	        Asave = A[*,0:N-1]
	        slpcolsave = slpcol
	        if evselect[0] ne 'NONE' then begin
		    delay_save_fl = delay
		    lcfunc_save_fl = lcfunc
		    stimlen_save_fl = stimlen
	        endif
	        tot_eff_save_fl = tot_eff
	        effect_column_save_fl = effect_column
	        effect_length_save_fl = effect_length
	        effect_label_save_fl = effect_label
	        all_eff_save_fl = all_eff
	    endif
	    if hipass eq 1 then slpcol_hipass=slpcol
	    for nfl=0,freqloop-1 do begin
	        if freqloop gt 1 then begin 
		    A = Asave
		    slpcol = slpcolsave
		    if evselect[0] ne 'NONE' then begin
		        delay = delay_save_fl
		        lcfunc = lcfunc_save_fl
		        stimlen = stimlen_save_fl
		    endif
		    tot_eff = tot_eff_save_fl
		    effect_column = effect_column_save_fl
		    effect_length = effect_length_save_fl
		    effect_label = effect_label_save_fl
		    all_eff = all_eff_save_fl
	        endif
	        if hipass eq 1 then begin
		    ;This method of implementing the high-pass filter reduces the number of frequencies and increases the
		    ;lowest frequency as more frames are omitted from the study.  If differing numbers of frames are
		    ;omitted from different runs, the frequencies filtered will be different although the cutoff
		    ;frequency will be the same.  This method ensures that the columns are nearly orthogonal.
		    if giocount eq 0 then begin 
		        j0 = 0
		        if lchpf eq 1 then begin
			    for file=0,nrun-1 do begin
			        t0 = oll_t0[file]
			        t1 = oll_t1[file]
			        num_freq = hpf_num_freq[file]
			        idx = where(valid_frms[t0:t1],nidx)
			        fscl = 2*!PI/nidx
			        j1 = j0 + nidx
			        for ifreq=0,num_freq-1 do begin
				    sincol = sin(findgen(t1-t0+1)*(ifreq+1)*fscl)
				    coscol = cos(findgen(t1-t0+1)*(ifreq+1)*fscl)
				    sincol = sincol[idx]
				    coscol = coscol[idx]
				    A[j0:j1-1,slpcol+2*ifreq] = sincol - total(sincol)/nidx
				    A[j0:j1-1,slpcol+2*ifreq+1] = coscol - total(coscol)/nidx
			        endfor
			        slpcol = slpcol + 2*num_freq
			        j0 = j1
			    endfor
		        endif
		        if lclpf eq 1 then begin
			    for file=0,nrun-1 do begin
			        t0 = oll_t0[file]
			        t1 = oll_t1[file]
			        num_freq = lpf_num_freq[file]
			        idx = where(valid_frms[t0:t1],nidx)
			        fscl = 2*!PI/nidx
			        j1 = j0 + nidx
			        idx0 = lpf_idx0[file]
			        for ifreq=0,num_freq-1 do begin
				    sincol = sin(findgen(t1-t0+1)*(idx0+1)*fscl)
				    coscol = cos(findgen(t1-t0+1)*(idx0+1)*fscl)
				    sincol = sincol[idx]
				    coscol = coscol[idx]
				    A[j0:j1-1,slpcol+2*ifreq] = sincol - total(sincol)/nidx
				    A[j0:j1-1,slpcol+2*ifreq+1] = coscol - total(coscol)/nidx
				    idx0 = idx0 + 1
			        endfor
			        slpcol = slpcol + 2*num_freq
			        j0 = j1
			    endfor
		        endif
		    endif else begin
		        if nfl eq 0 then begin
			    if frequseev_count gt 0 then begin
			        rtn_gio = get_gio(frequseev_cbyrun)
			        if rtn_gio.msg eq 'ERROR' then return,!ERROR
			        ncondfreq = rtn_gio.ncondfreq
			        ncondfreqc = rtn_gio.ncondfreqc
			        frequseev_idx = rtn_gio.frequseev_idx
			        ncondfreqi = -1
			        ncondfreqs = ''
			        ;print,'frequseev_idx=',frequseev_idx
			        for i=0,ncondfreq-1 do begin
				    index2 = where(frequseev_cbyrun[*,frequseev_idx[i]] eq 1,count)
				    if count eq 0 then begin
				        scrap = 'count='+strtrim(count,2)+' Should not be zero!'
				        print,scrap
				        stat=dialog_message(scrap,/ERROR)
				    endif else begin
				        ncondfreqi = [ncondfreqi,index2]
				        ncondfreqs = [ncondfreqs,frequseev_el[frequseev_idx[i]]]
				    endelse
			        endfor
			        ncondfreqi = ncondfreqi[1:*]
			        ncondfreqs = ncondfreqs[1:*]
			    endif else if identify[0] ne '' then begin
			        which = make_array(nrun,/INTEGER,VALUE=-1)
			        ncondfreq = 0
			        ncondfreqc = -1
			        ncondfreqi = -1
			        ncondfreqs = ''
			        repeat begin
				    index = where(which eq -1,count)
				    if count ne 0 then begin
				        index2 = where(strmatch(identify,identify[index[0]]) eq 1,count2)
				        which[index2] = ncondfreq
				        ncondfreq = ncondfreq + 1
				        ncondfreqc = [ncondfreqc,count2]
				        ncondfreqi = [ncondfreqi,index2]
				        ncondfreqs = [ncondfreqs,identify[index[0]]]
				    endif
			        endrep until count eq 0
			        ncondfreqc = ncondfreqc[1:*]
			        ncondfreqi = ncondfreqi[1:*]
			        ncondfreqs = ncondfreqs[1:*]
			    endif else begin
			        ncondfreq = 1
			        ncondfreqc = nrun
			        ncondfreqi = indgen(nrun)
			        ncondfreqs = ''
			    endelse
			    valfrmsperrun = intarr(nrun)
			    j_file = lonarr(nrun+1)
			    j0 = 0L
			    for file=0,nrun-1 do begin
			        t0 = tdim_sum[file]
			        t1 = tdim_sum_new[file]-1
			        if t1 gt tdim-1 then t1 = tdim - 1
			        idx = where(valid_frms[t0:t1],nidx)
			        j1 = j0 + nidx
			        j0 = j1
			        j_file[file+1] = j1
			        valfrmsperrun[file] = nidx
			    endfor
		        endif
		        if freqperglm eq 1 then ifreqend = nfl+1 else ifreqend = giocount
		        freq_ntrials = intarr(ncondfreq) 
		        for ifreq=nfl,ifreqend-1 do begin
			    iii = 0
			    for i=0,ncondfreq-1 do begin
			        for ii=0,ncondfreqc[i]-1 do begin
				    t0 = tdim_sum[ncondfreqi[iii]] + giofreqstartindex
				    t1 = tdim_sum_new[ncondfreqi[iii]] - 1
				    if t1 gt tdim-1 then t1 = tdim - 1
				    idx = where(valid_frms[t0:t1],nidx)
				    length = t1-t0+1
				    if freqnorm gt 0 or adj_not_adjplustask eq 2 then begin
				        rtn_yes = get_yes(frequseev_frms[t0:t1],frequseev_idx[i],length,valid_frms[t0:t1])
				        if rtn_yes.msg eq 'ERROR' then return,!ERROR
				        count_yes = rtn_yes.count_yes
				        index_yes = rtn_yes.index_yes
				    endif
				    len = length
				    if adj_not_adjplustask eq 0 then $
				        fscl=2*!PI/nidx $
				    else if adj_not_adjplustask eq 1 then $
				        fscl=2*!PI/length $
				    else if adj_not_adjplustask eq 2 then $
				        fscl=2*!PI/count_yes $
				    else begin
				        fscl=2*!PI/freq_stimlenframes
				        len = freq_stimlenframes
				    endelse
				    sincol = sin(findgen(len)*(gioindex[ifreq]+1)*fscl)
				    coscol = cos(findgen(len)*(gioindex[ifreq]+1)*fscl)
				    if frequseev_count gt 0 and adj_not_adjplustask le 2 then begin
				        index_not = where(frequseev_frms[t0:t1] ne frequseev_idx[i],count)
				        if count eq 0 then begin
					    scrap = 'count='+strtrim(count,2)+' Should not be zero!'
					    print,scrap
					    stat=dialog_message(scrap,/ERROR)
				        endif
				        sincol[index_not]=0.
				        coscol[index_not]=0.
				    endif
				    if freqnorm gt 0 then begin
				        sincol_yes = sincol[index_yes]
				        coscol_yes = coscol[index_yes]
				        sincol1 = sincol_yes - total(sincol_yes)/count_yes
				        coscol1 = coscol_yes - total(coscol_yes)/count_yes
				        if freqnorm eq 1 or freqnorm eq 3 then begin
					    print,'Mean normalizing frequencies.'
					    sincol_yes = sincol1
					    coscol_yes = coscol1
				        endif
				        if freqnorm eq 2 or freqnorm eq 3 then begin
					    print,'Variance normalizing frequencies.'
					    sincol_yes = sincol_yes/(total(sincol1^2)/(count_yes-1))
					    coscol_yes = coscol_yes/(total(coscol1^2)/(count_yes-1))
				        endif
				        sincol[index_yes] = sincol_yes
				        coscol[index_yes] = coscol_yes
				    endif else if len ne nidx then begin
				        print,'Mean normalizing frequencies.'
				        sincol[idx] = sincol[idx] - total(sincol[idx])/nidx 
				        coscol[idx] = coscol[idx] - total(coscol[idx])/nidx 
				    endif
				    if run_trial eq 0 and adj_not_adjplustask le 2 then begin
				        A[j_file[ncondfreqi[iii]]:j_file[ncondfreqi[iii]+1]-1,slpcol] = sincol[idx]
				        A[j_file[ncondfreqi[iii]]:j_file[ncondfreqi[iii]+1]-1,slpcol+1] = coscol[idx]
				        slpcol = slpcol + 2
				        freq_ntrials[i] = freq_ntrials[i] + 1
				    endif else begin
				        index1 = where(frames[*,0] ge t0 and frames[*,0] le t1,count1)
				        if count1 eq 0 then begin
					    scrap = 'count='+strtrim(count1,2)+' Should not be zero!'
					    print,scrap
					    stat=dialog_message(scrap,/ERROR)
				        endif
				        index2 = where(conditions[index1] eq frequseev_idx[i],count2)
				        if count2 eq 0 then begin
					    scrap = 'count='+strtrim(count2,2)+' Should not be zero!'
					    print,scrap
					    stat=dialog_message(scrap,/ERROR)
				        endif
				        index2 = index1[index2]
				        sinreg = fltarr(length)
				        cosreg = fltarr(length)
				        if run_trial eq 1 then begin
					    if adj_not_adjplustask le 2 then begin
					        for icol=0,count2-1 do begin
						    istart = index2[icol]
						    index3 = indgen(round(stimlenframes_vs_t[istart]))+frames[istart,0]
						    sinreg[*] = 0.
						    cosreg[*] = 0.
						    sinreg[index3] = sincol[index3]
						    cosreg[index3] = coscol[index3]
						    A[j_file[ncondfreqi[iii]]:j_file[ncondfreqi[iii]+1]-1,slpcol] = sinreg[idx]
						    A[j_file[ncondfreqi[iii]]:j_file[ncondfreqi[iii]+1]-1,slpcol+1] = cosreg[idx]
						    slpcol = slpcol + 2
					        endfor
					        freq_ntrials[i] = freq_ntrials[i] + count2
					    endif else begin
					        for icol=0,count2-1 do begin
						    istart = index2[icol]
						    index3 = indgen(stimlenframes_vs_t[istart])+frames[istart,0]
						    sinreg[*] = 0.
						    cosreg[*] = 0.
						    sinreg[index3] = sincol
						    cosreg[index3] = coscol
						    A[j_file[ncondfreqi[iii]]:j_file[ncondfreqi[iii]+1]-1,slpcol] = sinreg[idx]
						    A[j_file[ncondfreqi[iii]]:j_file[ncondfreqi[iii]+1]-1,slpcol+1] = cosreg[idx]
						    slpcol = slpcol + 2
					        endfor
					        freq_ntrials[i] = freq_ntrials[i] + count2
					    endelse
				        endif else begin
					    for icol=0,count2-1 do begin
					        istart = index2[icol]
					        index3 = indgen(stimlenframes_vs_t[istart])+frames[istart,0]
					        sinreg[index3] = sincol
					        cosreg[index3] = coscol
					    endfor 
					    A[j_file[ncondfreqi[iii]]:j_file[ncondfreqi[iii]+1]-1,slpcol] = sinreg[idx]
					    A[j_file[ncondfreqi[iii]]:j_file[ncondfreqi[iii]+1]-1,slpcol+1] = cosreg[idx]
					    slpcol = slpcol + 2
					    freq_ntrials[i] = freq_ntrials[i] + 1
				        endelse
				    endelse
				    iii = iii + 1
			        endfor
			    endfor
		        endfor
		    endelse
		    if all_eff gt 0 then begin
		        tmp1 = effect_column
		        tmp2 = effect_length
		        tmp3 = effect_label
		    endif
		    if giocount eq 0 then scrap = 1 else scrap = freqperglm*ncondfreq
		    effect_column = intarr(all_eff+scrap)
		    effect_length = intarr(all_eff+scrap)
		    effect_label = strarr(all_eff+scrap)
		    if all_eff gt 0 then begin
		        effect_column[0:all_eff-1] = tmp1
		        effect_length[0:all_eff-1] = tmp2
		        effect_label[0:all_eff-1] = tmp3
		    endif
		    if giocount eq 0 then begin
		        if all_eff gt 0 then effect_column[all_eff] = effect_column[all_eff-1] + effect_length[all_eff-1]
		        if lchpf eq 1 then begin
			    effect_length[all_eff] = hpf_len
			    effect_label[all_eff] = 'fidlhpf'
			    all_eff = all_eff + 1
		        endif
		        if lclpf eq 1 then begin
			    effect_length[all_eff] = lpf_len
			    effect_label[all_eff] = 'fidllpf'
			    all_eff = all_eff + 1
		        endif
		    endif else begin
		        supergoose=0
		        if run_trial eq 0 then goose=ncondfreqc $
		        else if adj_not_adjplustask le 2 then goose=freq_ntrials $
		        else begin
			    goose=freq_ntrials/freqperglm
			    supergoose=1
		        endelse
		        for ifreq=nfl,ifreqend-1 do begin
			    for i=0,ncondfreq-1 do begin
			        if all_eff gt 0 then effect_column[all_eff] = effect_column[all_eff-1] + effect_length[all_eff-1]
			        effect_length[all_eff] = 2*goose[i]
			        effect_label[all_eff] = giofreqstr[ifreq] + ncondfreqs[i]
			        all_eff = all_eff + 1
			    endfor
		        endfor
		        if supergoose eq 1 then begin
			    supergoose = total(goose)
			    N = N + supergoose
			    A = [[A[*,0:N-1]],[fltarr(tcomp,supergoose)]]
			    iii=0
			    for i=0,ncondfreq-1 do begin
			        for ii=0,ncondfreqc[i]-1 do begin
				    t0 = tdim_sum[ncondfreqi[iii]] + giofreqstartindex
				    t1 = tdim_sum_new[ncondfreqi[iii]] - 1
				    if t1 gt tdim-1 then t1 = tdim - 1
				    idx = where(valid_frms[t0:t1],nidx)
				    length = t1-t0+1
				    len = freq_stimlenframes
				    sincol = make_array(len,/FLOAT,VALUE=1.)
				    index1 = where(frames[*,0] ge t0 and frames[*,0] le t1,count1)
				    if count1 eq 0 then begin
				        scrap = 'count='+strtrim(count1,2)+' Should not be zero!'
				        print,scrap
				        stat=dialog_message(scrap,/ERROR)
				    endif
				    index2 = where(conditions[index1] eq frequseev_idx[i],count2)
				    if count2 eq 0 then begin
				        scrap = 'count='+strtrim(count2,2)+' Should not be zero!'
				        print,scrap
				        stat=dialog_message(scrap,/ERROR)
				    endif
				    sinreg = fltarr(length)
				    for icol=0,count2-1 do begin
				        istart = index2[icol]
				        index3 = indgen(stimlenframes_vs_t[istart])+frames[istart,0]
				        sinreg[*] = 0.
				        sinreg[index3] = sincol
				        ;print,'slpcol=',slpcol
				        ;print,'sinreg[idx]=',sinreg[idx]
				        ;print,'j_file[ncondfreqi[iii]]=',j_file[ncondfreqi[iii]]
				        ;print,'j_file[ncondfreqi[iii]+1]-1=',j_file[ncondfreqi[iii]+1]-1
				        A[j_file[ncondfreqi[iii]]:j_file[ncondfreqi[iii]+1]-1,slpcol] = sinreg[idx]
				        slpcol = slpcol + 1
				    endfor
				    iii = iii + 1
			        endfor
			    endfor
			    effect_column = [effect_column,effect_column[all_eff-1] + effect_length[all_eff-1]]
			    effect_length = [effect_length,supergoose]
			    effect_label = [effect_label,'Baseline']
			    all_eff = all_eff + 1
		        endif
		        ;for i=0,tcomp-1 do begin
		        ;    print,'i=',i
		        ;    print,strcompress(transpose(fix(A[i,504:545])))
		        ;endfor
		    endelse
		    hipass_eff = all_eff
	        endif
	        widget_control,/HOURGLASS
	        if min_tdim gt 1 then begin
		    cond_number_thresh=10000.
		    cond_number = double(0.)
		    cond_number_svd = double(0.)
		    n_depend = 0L
		    stat=0L
		    stat=call_external(!SHARE_LIB,'_Acond_norm1',A,tcomp,long(N),cond_number,VALUE=[0,1,1,0])
		    if stat eq 8L then begin
		        print,'Error in _Acond1_norm "GSL_ENOMEM   = 8,   /* malloc failed */"'
		        scrap=get_button(['Continue','Go back','Exit'],BASE_TITLE='Not enough memory',TITLE='Not enough memory to '$
                            +'compute the condition number.'+string(10B)+'What do you want to do?')
		        if scrap eq 2 then return,!ERROR else if scrap eq 1 then goto,goback60
		    endif else begin
		        if stat eq 0L then print,'_Acond_norm1 cond_number=',cond_number
		        if cond_number lt 0.00001 or cond_number gt cond_number_thresh then begin
			    stat=call_external(!SHARE_LIB,'_AtoATAgsl_svd',A,tcomp,long(N),cond_number_svd,n_depend,VALUE=[0,1,1,0,0])
			    if stat eq 0L then begin
			        print,'Error in _AtoATAgsl_svd'
			        return,!ERROR
			    endif
			    print,'_AtoATAgsl_svd cond_number_svd=',cond_number_svd,' n_depend=',n_depend
		        endif
		        if cond_number gt cond_number_thresh or cond_number_svd gt cond_number_thresh or n_depend gt 0 then begin
			    if n_depend gt 0 then print,'Design matrix is rank deficient by ',strtrim(n_depend,2),' columns'
			    if n_depend le 1 then begin
			        if m eq 0 then begin
				    lc_constrain = 0
				    if tot_eff gt 0 then begin
				        scrap=get_button(['yes','no','go back'],BASE_TITLE=evselect[m], $
                                            TITLE='number of dependencies = ' $
					    +strtrim(n_depend,2)+string(10B)+'condition number = '+strtrim(cond_number,2)+string(10B) $
					    +string(10B)+'You are reading this because of a single dependency and/or the condition '$
                                            +'number is greater than '+strtrim(fix(cond_number_thresh),2)+'.'+string(10B)+string(10B)$
                                            +'Possibly ill-conditioned or singular design matrix.'+string(10B)+'Constraining the '$
                                            +'design matrix will force the mean over all estimates of interest to be zero.' $
                                            +string(10B)+string(10B)+'Do you want to constrain the design matrix?')
				        if scrap eq 2 then goto,goback60 else if scrap eq 0 then lc_constrain=1
				    endif else begin
				        if hipass eq !FALSE then begin
					    scrap=get_button(['continue','quit','go back'],BASE_TITLE=evselect[m],TITLE='number of ' $
					        +'dependencies = '+strtrim(n_depend,2)+string(10B)+'condition number = ' $
					        +strtrim(cond_number,2)+string(10B)+string(10B)+'You are reading this because of a ' $
					        +'single dependency and/or the condition number is greater than ' $
					        +strtrim(fix(cond_number_thresh),2)+'.'+string(10B)+string(10B)+'Possibly ' $
					        +'ill-conditioned or singular design matrix.')
					    if scrap eq 2 then goto,goback60 else if scrap eq 1 then return,!ERROR 
				        endif else begin
					    scrap=get_button(['continue','orthogonalize','quit','go back'],BASE_TITLE=evselect[m], $
					        TITLE='number of dependencies = '+strtrim(n_depend,2)+string(10B) $
                                                +'condition number = ' $
					        +strtrim(cond_number,2)+string(10B)+string(10B)+'You are reading this because of a '$
                                                +'single dependency and/or the condition number is greater than '$
                                                +strtrim(fix(cond_number_thresh),2)+'.'+string(10B)+string(10B)+'Possibly '$
                                                +'ill-conditioned or singular design matrix.')
					    if scrap eq 3 then goto,goback60 else if scrap eq 2 then return,!ERROR 
					    widget_control,/HOURGLASS
					    if scrap eq 1 then begin
					        ;orthogonalize whole design matrix at once (not run by run)
					        Ahipass = A[*,slpcol_hipass:slpcol_hipass+len_hipass-1]
					        Ahipass = double(Ahipass)
					        size_Ahipass = size(Ahipass)
					        stat=call_external(!SHARE_LIB,'_orthogonalize',size_Ahipass[1],size_Ahipass[2], $
                                                    Ahipass,VALUE=[1,1,0])
					        if stat eq 0L then begin
						    print,'Error in _orthogonalize'
						    return,!ERROR
					        endif
					        A[*,slpcol_hipass:slpcol_hipass+len_hipass-1] = float(Ahipass)
					        ATA = transpose(A)#A
					        size_ATA = size(ATA)
					        stat=call_external(!SHARE_LIB,'_cond_norm1',ATA,size_ATA[1],cond_number,VALUE=[0,1,0])
					        if stat eq 0L then begin
						    print,'Error in _cond1_norm'
						    return,!ERROR
					        endif
					        print,'_cond_norm1 cond_number=',cond_number,' n_depend=',n_depend
					    endif
				        endelse
				    endelse
			        endif
			    endif else begin
			        dummy = 'Design matrix is singular. The event file must be modified to yield a computable design.'
			        if total_nsubjects eq 1 then begin
				    scrap=get_button(['yes, set it up anyway','no','go back'], $
				        BASE_TITLE=evselect[m], $
				        TITLE='number of dependencies = '+strtrim(n_depend,2)+string(10B)+'condition number = ' $
				        +strtrim(cond_number_svd,2)+string(10B)+string(10B)+'You are reading this because there is ' $
                                        +'more than a single dependency.'+string(10B)+'Design matrix is singular. The event file ' $
                                        +'must be modified to yield a computable design.'+string(10B)+string(10B)+'Do you wish to ' $
                                        +'continue?')
				    if scrap eq 2 then goto,goback60 else if scrap eq 1 then return,!ERROR
			        endif else if talk eq 1 then begin
				    scrap=get_button(['yes, skip this one, try the next event file','yes, set it up anyway','no',$
                                        'go back'],BASE_TITLE=evselect[m],TITLE='number of dependencies = '+strtrim(n_depend,2)$
                                        +string(10B)+'condition number = '+strtrim(cond_number,2)+string(10B)+string(10B)+'You are '$
                                        +'reading this because there is more than a single dependency.'+string(10B)+'Design matrix '$
                                        +'is singular. The event file must be modified to yield a computable design.'+string(10B)$
                                        +string(10B)+'Do you wish to continue?')
				    if scrap eq 3 then goto,goback60 $
				    else if scrap eq 2 then return,!ERROR $
				    else if scrap eq 0 then begin
				        errorstr = errorstr + evselect[m] + string(10B)
				        goto,bottomofloop
				    endif
			        endif
			    endelse
		        endif
		    endelse
	        endif
	        if lc_constrain eq !TRUE then begin
		    if n_elements(index_notuserdefined) ne 0 then begin    
		        print,'Constraining design matrix.'
		        A[*,index_notuserdefined] = A[*,index_notuserdefined] - 1./n_elements(index_notuserdefined)
		    endif
	        endif
	        widget_control,/HOURGLASS
	        ;KEEP
	        ;if nmain gt 1 then begin
	        ;    sdbar = 0.
	        ;    sd = fltarr(nmain)
	        ;    for i=0,nmain-1 do begin
	        ;        sd[i] = sqrt(ATAm1[i,i])
	        ;        sdbar = sdbar + sd[i]
	        ;    endfor
	        ;    sdbar = sdbar/nmain
	        ;    mse_sd = 0.
	        ;    for i=0,nmain-1 do mse_sd = mse_sd + (sd[i] - sdbar)^2
	        ;    rmse_sd = sqrt(mse_sd)
	        ;    ms_cc = 0.
	        ;    for i=0,nmain-1 do begin
	        ;        for j=i+1,nmain-1 do begin
	        ;            ms_cc = ms_cc + ATAm1[i,j]^2/(ATAm1[i,i]*ATAm1[j,j])
	        ;        endfor
	        ;    endfor
	        ;    rms_cc = sqrt(2.*ms_cc/(nmain*(nmain-1)))
	        ;    var = fltarr(nmain)
	        ;    for i=0,nmain-1 do var[i] = ATAm1[i,i]
	        ;    min = min(var,MAX=max)
	        ;    ratio = max/min
	        ;    print,'Design metrics:'
	        ;    print,'Mean (scaled) std dev: ',sdbar
	        ;    print,'RMS deviation of scaled std dev: ',rmse_sd
	        ;    print,'RMS correlation coefficient: ',rms_cc
	        ;    if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='These metrics describe the variance and '$
                ;        +'homogeneity of the timecourse estimates.  The first metric is the average of numbers proportional to the '$
                ;        +'standard deviation of each timepoint of the timecourse.  The second number, given by ' $
	        ;        +'rmse_sd = sqrt(sum_from_1_to_N (sd[n] - sd_bar[n])^2/N), where sd is the std. dev. and sdbar is its ' $
                ;        +'sample mean, is a measure of how constant the variance is across timepoints.  The last metric is the '$
                ;        +'root-mean-square value of the correlation coefficients.  It is zero if the estimates are uncorrelated and '$
                ;        +'one if they are completely correlated.  The repeated measures ANOVA relies on the assumption that the '$
                ;        +'variance of differences are constant.  The degrees of freedom are adjusted downward as this assumption is '$
                ;        +'validated.  Therefore, the best designs minimize all of these metrics.'
	        ;endif
	        if evselect[0] ne 'NONE' then begin
		    if nbehavcol eq 0 then begin
		        nindex_present = n_elements(index_present)
		        idx = where(deleted eq 0,count)
		        if count ne 0 then begin
			    if n_elements(num_offsets_eff) eq 0 then num_offsets_eff = lcround_tc[index_present]
			    if n_elements(effect_shift_TR) eq 0 then effect_shift_TR = geffect_shift_TR[index_present]
		        endif
		    endif
		    if lc_er eq 1 then begin
		        goose = make_array(er.NF,/INTEGER,VALUE=1)
		        num_offsets_eff = [num_offsets_eff,goose]
		        goose[*] = 0
		        effect_shift_TR = [effect_shift_TR,goose]
		        goose = make_array(er.NF,/FLOAT,VALUE=TR)
		        if n_elements(effect_TR) ne 0 then effect_TR = [effect_TR,goose] else effect_TR=goose
		        undefine,goose
		    endif
		    if lc_rr eq 1 then begin
		        goose = make_array(rr_neff,/INTEGER,VALUE=1)
		        num_offsets_eff = [num_offsets_eff,goose]
		        goose[*] = 0 
		        effect_shift_TR = [effect_shift_TR,goose]
		        goose = make_array(rr_neff,/FLOAT,VALUE=TR)
		        if n_elements(effect_TR) ne 0 then effect_TR = [effect_TR,goose] else effect_TR=goose
		        undefine,goose
		    endif
		    num_trials = sng[m].num_trials
	        endif else begin
		    num_trials = 0
		    if tot_eff gt 0 then begin
		        num_offsets_eff = make_array(tot_eff,/INTEGER,VALUE=1) 
		        effect_shift_TR = intarr(tot_eff)
		    endif else $
		        num_offsets_eff = 1
	        endelse


                ;openw,lucheck,'check.txt',/GET_LUN
                ;size_A = size(A)
                ;printf,lucheck,'size_A=',size_A
                ;for i=0,size_A[1]-1 do begin
                ;    printf,lucheck,'i=',i
                ;    printf,lucheck,transpose(A[i,*])
                ;endfor
                ;close,lucheck
                ;free_lun,lucheck


                if gg.replaceselect[0] ne '' then begin

                    ;print,'here1 idx3=',idx3

                    ;A=A[idx3,*]
                    ;START170516
                    A=A[0:tcomp3-1,*]

                endif
	        if evselect[0] ne 'NONE' then if n_elements(effect_TR) eq 0 then effect_TR = TR/float(num_offsets_eff)
	        glm1 = {General_linear_model}
	        event_file = evselect[m]
	        if evselect[m] ne 'NONE' then glm1.stimlen_vs_t = ptr_new(stimlen_vs_t)
	        ifh2[m].glm_event_file = ptr_new(evselect[m])
	        set_glm1,glm1,A,valid_frms3,nmain,N,tot_eff,xdim,ydim,zdim,tdim3,dxdy,dz,nrun3,tcomp3, $
                    num_trials,TR,stimlen,lcfunc,functype0,DELAY=delay,funclen0, $
                    NF=1,all_eff,effect_column,effect_label,effect_length,EFFECT_TR=effect_TR,EFFECT_SHIFT_TR=effect_shift_TR
	        glm[fi.n] = glm1
	        glm_idx = fi.n
	        ;print,'fi.n=',fi.n
	        ;print,'nmain=',nmain
	        ;print,'N=',N
	        ;print,'tot_eff=',tot_eff
	        ;print,'tcomp=',tcomp
	        ;print,'num_trials=',num_trials
	        ;print,'stimlen=',stimlen
	        ;print,'lcfunc=',lcfunc
	        ;print,'functype=',functype
	        ;print,'delay=',delay
	        ;print,'funclen=',funclen 
	        ;print,'all_eff=',all_eff
	        ;print,'num_offsets_eff=',num_offsets_eff
	        ;print,'hereD effect_shift_TR=',effect_shift_TR
	        ;print,'hereD effect_TR=',effect_TR
	        ;print,'hereD max_num_levels=',max_num_levels

	        if glm_conc eq 0 and lc_contrast then begin

		    ;if n_elements(nc) ne 0 then begin
                    ;START170126
                    if m1 gt -1 then begin
                        if m eq 0 then begin
                            undefine,hrf_param
                            hrf_param = fltarr(2,tot_eff)
                            hrf_param[0,*] = !HRF_DELTA
                            hrf_param[1,*] = !HRF_TAU
                        endif
                        widget_control,/HOURGLASS

                        rtn=default_contrasts_noglm(effect_label,effect_length,tot_eff,N,effect_TR,effect_shift_TR,stimlen,delay, $
                            lcfunc,hrf_param,nmain,hrf_model)
                        ;nc=rtn.nc
                        ;c=rtn.c
                        ;cnorm=rtn.cnorm
                        ;contrast_labels=rtn.contrast_labels

                        glm[glm_idx].nc = rtn.nc
                        if ptr_valid(glm[glm_idx].c) then ptr_free,glm[glm_idx].c
                        glm[glm_idx].c = ptr_new(rtn.c)
                        if ptr_valid(glm[glm_idx].cnorm) then ptr_free,glm[glm_idx].cnorm
                        glm[glm_idx].cnorm = ptr_new(rtn.cnorm)
                        if ptr_valid(glm[glm_idx].contrast_labels) then ptr_free,glm[glm_idx].contrast_labels
                        glm[glm_idx].contrast_labels = ptr_new(rtn.contrast_labels)
                        undefine,rtn
                    endif else if n_elements(nc) ne 0 then begin

		        ;if tot_eff gt 0 then begin
                        ;START170705
		        if tot_eff gt 0 and n_elements(effect_label0) ne 0 then begin

			    if evselect[0] ne 'NONE' then begin
			        if lcexclude_count eq count_present_ts then begin
				    lizard = effect_label0
			        endif else lizard = effect_label0[index_present_ts] 
			    endif else lizard = effect_label0
			    idx=-1
			    for i=0,tot_eff-1 do begin
			        idx0=where(effect_label[i] eq lizard,cnt0)
			        if cnt0 ne 0 then idx=[idx,idx0]
			    endfor
			    if n_elements(idx) eq 1 then begin
			        glm[glm_idx].nc = 0 
			        if ptr_valid(glm[glm_idx].c) then ptr_free,glm[glm_idx].c
			        if ptr_valid(glm[glm_idx].cnorm) then ptr_free,glm[glm_idx].cnorm
			        if ptr_valid(glm[glm_idx].contrast_labels) then ptr_free,glm[glm_idx].contrast_labels
			    endif else begin
			        idx=idx[1:*]
			        wallace = fltarr(tot_eff0)
			        wallace[idx] = 1.
			        nci=-1
			        for i=0,nc-1 do begin
				    idx0 = where(wallace-cnorm_ones[*,i] eq -1.,cnt)
				    if cnt eq 0 then nci = [nci,i]
			        endfor
			        nci = nci[1:*]
			        nc1 = n_elements(nci)
			        contrast_labels1 = contrast_labels[nci]
			        cnorm1 = cnorm[idx,*]
			        cnorm1 = cnorm1[*,nci]
			        darby = effect_column0[idx]
			        goose = -1
			        for i=0,n_elements(idx)-1 do goose = [goose,darby[i]+indgen(effect_length0[idx[i]])]
			        goose = goose[1:*]
			        c1 = c[goose,*]
			        c1 = c1[*,nci]
			        darby=N-n_elements(goose)
			        if darby ne 0 then begin
				    if evselect[0] ne 'NONE' then begin
				        if lcexclude_count eq count_present_ts and lc_er eq 1 then begin
					    c1=[fltarr(er.NF,nc1),c1,fltarr(n_nuisance,nc1)] 
				        endif else c1=[c1,fltarr(darby,nc1)]
				    endif else c1=[c1,fltarr(darby,nc1)]
			        endif
			        if lc_er eq 1 then begin
				    c2 = fltarr(glm[glm_idx].N,er.NF)
				    cnorm2 = fltarr(glm[glm_idx].tot_eff,er.NF)


				    if lcexclude_count eq count_present_ts then begin
				        j=0
				        k=0
				    endif else begin
				        j = glm[glm_idx].n_interest - er.NF
				        k = glm[glm_idx].tot_eff - er.NF
				    endelse
                                    ;START170331
                                    ;j=0 & k=0
                                    ;if evselect[0] ne 'NONE' then begin
				    ;    if lcexclude_count ne count_present_ts then begin
				    ;        j = glm[glm_idx].n_interest - er.NF
				    ;        k = glm[glm_idx].tot_eff - er.NF
				    ;    endif
                                    ;endif


				    for i=0,er.NF-1 do begin
				        c2[j,i] = 1
				        cnorm2[k,i] = 1
				            j = j+1
				        k = k+1
				    endfor
				    if lcexclude_count eq count_present_ts then begin
				        c1 = [[c2],[c1]]
				        cnorm1 = [[cnorm2],[fltarr(er.NF,nc1),cnorm1]]
				        contrast_labels1 = [er.hdr[*],contrast_labels1]
				    endif else begin
				        c1 = [[c1],[c2]]
				        cnorm1 = [[cnorm1,fltarr(er.NF,nc1)],[cnorm2]]
				        contrast_labels1 = [contrast_labels1,er.hdr[*]]
				    endelse
				    nc1 = nc1 + er.NF
			        endif
			        glm[glm_idx].nc = nc1
			        if ptr_valid(glm[glm_idx].c) then ptr_free,glm[glm_idx].c
			        glm[glm_idx].c = ptr_new(c1)
			        if ptr_valid(glm[glm_idx].cnorm) then ptr_free,glm[glm_idx].cnorm
			        glm[glm_idx].cnorm = ptr_new(cnorm1)
			        if ptr_valid(glm[glm_idx].contrast_labels) then ptr_free,glm[glm_idx].contrast_labels
			        glm[glm_idx].contrast_labels = ptr_new(contrast_labels1)
			    endelse
		        endif
		    endif
                    if lcfirstcomputable eq 0 then begin
                        lcfirstcomputable = 1
                        goback19:
                        if n_elements(fstat_glm) ne 0 then undefine,fstat_glm
                        if n_elements(parf0) ne 0 then begin
                            idx=where(tag_names(par0) eq 'FSTAT_GLM',cnt)
                            if cnt ne 0 then begin
                                fstat_glm=par0.fstat_glm
                                rtn=load_linmod(fi,dsp,wd,glm,help,ifh,FILENAME=fstat_glm,/STOPATFSTAT)
                                nF=rtn.nF
                                F_names=rtn.F_names
                                F_names2=rtn.F_names2
                                undefine,rtn
                            endif
                        endif
                        if n_elements(fstat_glm) eq 0 then begin
                            print,'Setting up F statistics'
                            effect_label1=*glm[glm_idx].effect_label
                            if n_elements(effect_label0) eq 0 then effect_label=effect_label1 else begin
                                effect_label=effect_label0 
                                for i=0,n_elements(effect_label1)-1 do begin
                                    idx=where(effect_label1[i] eq effect_label0,cnt)
                                    if cnt eq 0 then effect_label=[effect_label,effect_label1[i]]
                                endfor
                            endelse
                            nlab=n_elements(effect_label)
                            glab=strarr(nlab+1)
                            lcstop=0
                            nF=1
                            F_names='Omnibus'
                            if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='This dialog allows you to compute F ' $
                                +'statistics for any combination of effects.  "Define another F statistic" lets you select effects ' $
                                +'to be included.  Listings below "Exit loop" describe F statistics that will be computed.  An ' $
                                +'omnibus F statistic (over all effects of interest) is always computed.  Select "Exit loop" to ' $
                                +'stop defining F statistics.'
                            repeat begin
                                labels = ['Define another F statistic','Define all F statistics','Exit loop',F_names,'go back']
                                action = get_button(labels,TITLE='List of F statistics and Actions')
                                case action of
                                    0: begin ; Define New F statistic.
                                        rtn = get_bool_list(effect_label[0:nlab-1],TITLE='Specify effects to include')
                                        idx = where(rtn.list eq 1,cnt)
                                        if cnt ne 0 then begin
                                            F_names = [F_names,strjoin(effect_label[idx],'&',/SINGLE)]
                                            nF = nF + 1
                                        endif
                                        undefine,idx
                                       end
                                    1: begin
                                        rtn=fidl_combinations(tot_eff,effect_label,'')
                                        if rtn.msg eq 'ERROR' then return,!ERROR
                                        nF = rtn.nF
                                        F_names = rtn.F_names
                                       end
                                    2: lcstop = 1
                                    nF+3: goto,goback60
                                    else: begin
                                        print,'Select one of the first three options.'
                                        widget_control,wd.error,SET_VALUE=string('Select one of the first three options.')
                                       end
                                endcase
                            endrep until lcstop eq 1
                            F_names2 = F_names
                            if nF gt 1 then begin
                                name_max=0L
                                path_max=0L
                                stat=0L
                                stat=call_external(!SHARE_LIB,'_get_limits',name_max,path_max,VALUE=[0,0])
                                print,'name_max=',name_max,' path_max=',path_max
                                lizard=strarr(nF)
                                idx=where(strlen(F_names2) gt name_max,cnt)
                                repeat begin
                                    lizard[*] = 'Ok'
                                    if cnt ne 0 then lizard[idx]='Too long'
                                    scrap=get_str(nF,trim(indgen(nF)+1)+' '+lizard,F_names2,TITLE='Please name F statistics', $
                                        LABEL='Please shorten names that are too long.'+string(10B)+'Maximum length is ' $
                                        +trim(name_max)+' characters',/ONE_COLUMN,/GO_BACK,/REPLACE,/RESET)
                                    if scrap[0] eq 'GO_BACK' then goto,goback19
                                    F_names2=scrap
                                    undefine,idx
                                    idx=where(strlen(F_names2) gt name_max,cnt)
                                    if cnt ne 0 then F_names2[idx]=F_names[idx]
                                endrep until cnt eq 0
                                undefine,idx,lizard
                            endif
                            if nF ne 0 then begin
                                if n_elements(glm0) eq 0 then glm0={General_linear_model}
                                glm0.nF = nF
                                if ptr_valid(glm0.F_names) then ptr_free,glm0.F_names
                                glm0.F_names = ptr_new(F_names)
                                if ptr_valid(glm0.F_names2) then ptr_free,glm0.F_names2
                                glm0.F_names2 = ptr_new(F_names2)
                                if n_elements(contrast_glm) eq 0 then fstat_glm=pardir+'/'+parroot+'.glm' else fstat_glm=contrast_glm
                                glm_file=save_linmod(fi,dsp,wd,glm,help,stc,pref,glm_idx,glm_idx,swapbytes,FILENAME=fstat_glm,$
                                    /DESIGN_ONLY,IFH=ifh2[0],GLM1=glm0)
                            endif
                        endif
                        idx=where(tag_names(par1) eq 'FSTAT_GLM',cnt)
                        if cnt ne 0 then begin
                            if n_elements(fstat_glm) eq 0 then fstat_glm='' 
                            par1.fstat_glm=fstat_glm
                        endif else $
                            par1=create_struct(par1,'fstat_glm',fstat_glm)
                        undefine,idx,cnt
                        nomenu = 1
                        if n_elements(action0) ne 0 then undefine,action0
                        if n_elements(parf0) ne 0 then begin
                            idx=where(tag_names(par0) eq 'EXECUTE',cnt)
                            if cnt ne 0 then action0=1-par0.execute
                        endif
                        if n_elements(action0) eq 0 then begin
                            action0 = get_button(['execute','return','go back'],TITLE='Please select action.',BASE_TITLE=script[0])
                            if action0 eq 2 then goto,goback62
                        endif
                        idx=where(tag_names(par1) eq 'EXECUTE',cnt)
                        if cnt ne 0 then par1.execute=1-action0 else par1=create_struct(par1,'execute',1-action0)
                        undefine,idx,cnt
                    endif
                    glm[glm_idx].nF = nF
                    if ptr_valid(glm[glm_idx].F_names) then ptr_free,glm[glm_idx].F_names
                    if nF ne 0 then glm[glm_idx].F_names = ptr_new(F_names)
                    if ptr_valid(glm[glm_idx].F_names2) then ptr_free,glm[glm_idx].F_names2
                    if nF ne 0 then glm[glm_idx].F_names2 = ptr_new(F_names2)
                endif
                gname = names[proc+m]

                ;if giocount gt 0 then begin
                ;    if freqperglm eq 1 then begin
                ;        gr1 = get_root(gname,'.glm')
                ;        gname = gr1.file[0] + '_' + giofreqstr[nfl] + '.glm'
                ;    endif
                ;endif
                ;START170126
                if giocount gt 0 then begin
                    if freqperglm eq 1 then begin
                        gname = rtn_gr.path[m]+rtn_gr.file[m] + '_' + giofreqstr[nfl] + '.glm'
                    endif
                endif
                if m1 gt -1 then begin
                    if m1 eq 0 then spawn,'mkdir '+rtn_gr.file[m],result
                    darby=trim(m1+1)
                    superbird=strlen(darby)
                    if superbird lt 3 then darby='0'+darby
                    if superbird lt 2 then darby='0'+darby
                    gname=rtn_gr.path[m]+'/'+rtn_gr.file[m]+'/'+darby+'_'+rtn_gr.file[m]+'.glm'
                endif
                glm_file = save_linmod(fi,dsp,wd,glm,help,stc,pref,glm_idx,glm_idx,swapbytes,FILENAME=gname,/DESIGN_ONLY,IFH=ifh2[m])
                if glm_file eq 'ERROR' then return,!ERROR
                if glm_conc eq 0 then begin
                    print_files_to_csh,lu,nrun3,filenames,'BOLD_FILES','bold_files',/NO_NEWLINE
                    printf,lu,'set INPUT_GLM_FILE = ','"','-input_glm_file ',glm_file,'"',FORMAT='(a,a,a,a,a)'
                    printf,lu,'set OUTPUT_GLM_FILE = ','"','-output_glm_file ',glm_file,'"',FORMAT='(a,a,a,a,a)'
                    if lc_atlas then begin
                        if gt4[m].nt4 eq 0 then begin
                            print_files_to_csh,lu,neach[m],t4select[mmm:mmm+neach[m]-1],'T4_FILE','xform_file',/NO_NEWLINE

                            ;START170125
                            ;mmm = mmm + neach[m]

                        endif else begin 
                            print_files_to_csh,lu,gt4[m].nt4,*gt4[m].t4,'T4_FILE','xform_file',/NO_NEWLINE
                            t4_identify_str[m] = ' -t4 '+strjoin(trim(indgen(gt4[m].nt4)+1),' ',/SINGLE)
                        endelse
                    endif else $
                        printf,lu,'set T4_FILE
                    if fwhm gt 0. then $
                        printf,lu,'set FWHM = ','"','-fwhm ',fwhm,'"',FORMAT='(a,a,a,a,a)' $
                    else $
                        printf,lu,'set FWHM'
                    if mask ne '' then $
                        print_files_to_csh,lu,1,mask,'MASK_FILE','mask',/NO_NEWLINE $
                    else $
                        printf,lu,'set MASK_FILE'
                    mode = 4*lc_smoothness_estimate + 2.*lc_atlas
                    if fwhm gt 0. then mode=mode+1
                    printf,lu,'set MODE = ','"','-mode ',mode,'"',FORMAT='(a,a,a,1x,i1,a)'
                    if(proceach0 eq proceach-1) or (m eq total_nsubjects-1) then lizard=cleanupstr else lizard=''
                    if scratchdirstr[0] eq '' then tokage='' else tokage=scratchdirstr[proceach1] 
                    printf,lu,'nice +19 $BIN/compute_glm2 $BOLD_FILES $INPUT_GLM_FILE $OUTPUT_GLM_FILE $T4_FILE $MASK_FILE $FWHM ' $
                        +'$MODE'+region_str+roi_str+t4_identify_str[m]+atlas_str+uncompressstr+tokage+R2str+lizard

                    ;START180426
                    if lc_er eq 1 then printf,lu,'# ',er_file[m] 
                    ;if lc_rr eq 1 then begin
                    ;    ;printf,lu,'# ',rr_region_str[m]
                    ;    ;printf,lu,'# ',cmd ;cmd on newline (not commented out)
                    ;    printf,lu,'# '+cmd
                    ;endif

                    printf,lu,''
                endif else begin
                    if fidl_genconc(fi,dsp,wd,glm,help,stc,pref,glm_file,concselect[m]) eq 'GO_BACK' then goto,goback60
                endelse
            endfor

    ;        if glm_conc eq 0 then begin
    ;            proceach0=proceach0+1
    ;            if(proceach0 eq proceach) or (m eq total_nsubjects-1) then begin
    ;                close,lu
    ;                free_lun,lu
    ;                print,'Script written to '+script[proceach1]
    ;                spawn,'chmod 0777 '+script[proceach1]
    ;                if action0 eq 0 then spawn,script[proceach1]+'>'+script[proceach1]+'.log'+' &'
    ;                proceach0=0
    ;                proceach1=proceach1+1
    ;            endif
    ;        endif
    ;        bottomofloop:
    ;    endfor ;for m1=0,num_trails0-1 do begin
    ;endfor ;for m=0,total_nsubjects-1 do begin
    ;START170125
        endfor ;for m1=0,num_trails0-1 do begin
        if glm_conc eq 0 then begin
            proceach0=proceach0+1
            if(proceach0 eq proceach) or (m eq total_nsubjects-1) then begin
                close,lu
                free_lun,lu
                print,'Script written to '+script[proceach1]
                spawn,'chmod 0777 '+script[proceach1]
                if action0 eq 0 then spawn,script[proceach1]+'>'+script[proceach1]+'.log'+' &'
                proceach0=0
                proceach1=proceach1+1
            endif
            if lc_atlas then begin
                if gt4[m].nt4 eq 0 then mmm = mmm + neach[m]
            endif
        endif
        bottomofloop:
    endfor ;for m=0,total_nsubjects-1 do begin




    if errorstr ne '' then begin
        if dialog_message('No GLMs will be computed for the following files.'+string(10B)+string(10B)+errorstr,/ERROR,/CANCEL) $
            eq 'Cancel' then return,!ERROR 
    endif
    if glm_conc eq 0 then begin
        if action0 eq 0 then $
            stat=get_button('ok',BASE_TITLE='The following scripts have been executed',TITLE=strjoin(script,string(10B),/SINGLE))
    endif
    parf=pardir+'/'+parroot+'.glmpar'
    write_glm_pref,parf,par1
    print,'GLM parameters written to '+parf
    print,'DONE'
    return,0
end
