;Copyright 12/31/99 Washington University.  All Rights Reserved.
;compute_avg_zstat.pro  $Revision: 1.215 $
function compute_avg_zstat,fi,dsp,wd,glm,help,stc,pref,ls,APPEND=append,BOY=boy,TC=tc,SLOPES=slopes
if not keyword_set(APPEND) then append = 0
if not keyword_set(BOY) then boy = 0
if not keyword_set(SLOPES) then slopes = 0
goback6:
basetitle = ''
lczstat = 1
lcsee = 0
one_file_str = ''
ncontrast = 0
accumulate_str = ''
c_labels_str = ''
region_file = ''
gauss_str = ''
cols_or_rows_str = ''
roi_str = ''
avgstat_output_str = ''
cstr = ''
tcstr = ''
lcsd = ''
tags_c = ''
tags_tc = ''
t4string = ''
delay_str = ''
Nimage_name_str = ''
Nimage_mask_str = ''
directory_str = ''
print_unscaled_mag_str = ''
within_subject_sd_str = ''
tc_type = ''
magnorm_str = ''
ntc = 0
glm_list_str = ''
tc_labels_str = ''
mail_str = ''
root = ''
pop_ind = 0
analysis_type = 0
asksmooth = 1
lc_Hz = 0
lc_notHz = 0
Hzstr = ''
concstr = ''
indhipassstr = ''
atlas_str = ''
lcphaseonly = 0
sameforall = 0
magwtstr=''
lcgroup = 1
lcind = 0

;START170616
lcsum=0

wfiles = ''
files_are_glms=0
nreg=0
stimlenstr = ''
framesstr = ''
voxelsstr = ''
nglm_per_subject_str = ''
ampstr = ''
phasestr = ''
maxdefstr = ''
loadslk=0
framesoutstr = ''
mask = ''
monte_carlo_fwhm=0.
region_or_uncompress_str=''
t4select=''

;START170131
nregfiles=0

directory = getenv('PWD')
nels = n_elements(ls)
if nels ne 0 then begin
    load = ls.load
    glmfiles = ls.glmfiles
    nlists = ls.nlists
    total_nsubjects = ls.total_nsubjects
    t4select = ls.t4select
    glm_list_str = ls.glm_list_str
    basetitle = ls.basetitle
    lczstat = ls.lczstat
    analysis_type = ls.analysis_type
    tc_type = ls.tc_type
    pop_ind = ls.pop_ind
    script = ls.script
    region_file = ls.region_file
    roi_str = ls.roi_str
    asksmooth = ls.asksmooth
    gauss_str = ls.gauss_str
    Nimage_mask_str = ls.mask_str
    atlas_str = ls.atlas_str
    if glm_list_str ne '' then begin
        rtn_gr = get_root(glm_list_str,'.')
        root = rtn_gr.file
        basetitle = root
    endif else if total_nsubjects eq 1 then begin 
        rtn_gr = get_root(glmfiles[0],'.glm')
        root = rtn_gr.file 
    endif
    if analysis_type eq 1 then begin
        scrap = strlen(basetitle)-1
        if strmid(basetitle,scrap) eq '_' then basetitle = strmid(basetitle,0,scrap)
        avgstat_output_str = ' -regional_avgstat_name "' + basetitle + '.dat"' + ' -flip -sd'
    endif
endif else begin
    goback7:

    ;scrap=get_glms(GET_THIS='GLMs/4dfps')
    ;START180316
    scrap=get_glms(GET_THIS='GLMs/4dfps/*nii/*nii.gz')

    if scrap.msg eq 'GO_BACK' or scrap.msg eq 'EXIT' then return,rtn={msg:'ERROR'} 
    load = scrap.load
    glmfiles = scrap.glmfiles
    ifh = scrap.ifh
    glm_space = scrap.glm_space
    bold_space = scrap.bold_space
    nlists = scrap.nlists
    total_nsubjects = scrap.total_nsubjects
    t4select = scrap.t4select
    glm_list_str = scrap.glm_list
    morethanone = scrap.morethanone
    if glmfiles[0] ne '' then begin
        files_are_glms = 1
        scrap = '.glm'
        if morethanone eq 0 then begin
            nsubject = total_nsubjects
            nglm_per_subject = make_array(nsubject,/INTEGER,VALUE=1)
            junk = get_root(glmfiles) 
            subject_names = junk.file
        endif else begin 
            rtn_get_morethanone=get_morethanone(glmfiles,total_nsubjects,1,'')
            if rtn_get_morethanone.msg eq 'GO_BACK' then goto,goback7 else if rtn_get_morethanone.msg eq 'ERROR' then $
                return,rtn={msg:'ERROR'} 
            nglm_per_subject = rtn_get_morethanone.nglm_per_subject
            total_nsubjects = rtn_get_morethanone.total_nsubjects
            nsubject = rtn_get_morethanone.nsubject
            glmfiles = rtn_get_morethanone.files_per_group
            subject_names = rtn_get_morethanone.subject_names
            t4select = rtn_get_morethanone.t4select
        endelse
        nglm_per_subject_str = ' -glmpersub '+strjoin(strtrim(nglm_per_subject,2),' ',/SINGLE)
        subject_filenames = strarr(nsubject)
        subject_get_labels_struct = replicate({Get_labels_struct},nsubject)
        j = 0
        for i=0,nsubject-1 do begin
            subject_get_labels_struct[i] = get_labels_from_glm(fi,dsp,wd,glm,help,pref,ifh1, $
                glmfiles[j:j+nglm_per_subject[i]-1],/CONTRASTS_ONLY,START_INDEX_GLM=j)
            subject_filenames[i] = strjoin(glmfiles[j:j+nglm_per_subject[i]-1],string(10B),/SINGLE)
            j = j + nglm_per_subject[i]
        endfor
    endif else begin 
        glmfiles = scrap.imgselect
        scrap = '.4dfp.img'
        nsubject=total_nsubjects
    endelse
    rtn_root = get_root(glmfiles,scrap)
    if total_nsubjects eq 1 then begin
        root = rtn_root.file[0]
        basetitle = root
    endif
endelse
vol = lonarr(total_nsubjects)
if files_are_glms eq 1 then begin
    cstrwts = make_array(nsubject,/STRING,VALUE='')
    for i=0,total_nsubjects-1 do vol[i] = long(ifh[i].glm_xdim)*long(ifh[i].glm_ydim)*long(ifh[i].glm_zdim)
    ne_max = max(subject_get_labels_struct[*].neffectlabels,ne_max_glm_index,MIN=ne_min)
    effectlabels=*subject_get_labels_struct[ne_max_glm_index].effectlabels
    effectlength=*subject_get_labels_struct[ne_max_glm_index].effect_length
    for i=0,nsubject-1 do begin
        if i ne ne_max_glm_index then begin
            scrap=*subject_get_labels_struct[i].effectlabels
            scrap1=*subject_get_labels_struct[i].effect_length
            for j=0,subject_get_labels_struct[i].neffectlabels-1 do begin
                if total(strcmp(effectlabels,scrap[j])) eq 0 then begin
                    effectlabels = [effectlabels,scrap[j]]
                    effectlength = [effectlength,scrap1[j]]
                endif
            endfor
        endif
    endfor
    neffectlabels_all = n_elements(effectlabels)
    index = where((effectlabels ne 'Trend') and (effectlabels ne 'Baseline'),count)
    if count ne 0 then begin
        effectlabels = effectlabels[index]
        effectlength = effectlength[index]
    endif
    neffectlabels = n_elements(effectlabels)
    effect_length_ts = make_array(nsubject,neffectlabels_all,/INT,VALUE=1)
    for i=0,nsubject-1 do $
        effect_length_ts[i,0:subject_get_labels_struct[i].neffectlabels-1] = *subject_get_labels_struct[i].effect_length
    nreg = subject_get_labels_struct[ne_max_glm_index].ifh.nreg
    ncontrastlabels = max(subject_get_labels_struct[*].ncontrastlabels,nc_max_glm_index,MIN=nc_min)
    if ncontrastlabels gt 0 then begin
        contrastlabels=*subject_get_labels_struct[nc_max_glm_index].contrastlabels
        for i=0,nsubject-1 do begin
            if i ne nc_max_glm_index then begin
                scrap=*subject_get_labels_struct[i].contrastlabels
                for j=0,subject_get_labels_struct[i].ncontrastlabels-1 do begin
                    if total(strcmp(contrastlabels,scrap[j])) eq 0 then contrastlabels = [contrastlabels,scrap[j]]
                endfor
            endif
        endfor
    endif
    ifh_fwhm = ifh[*].glm_fwhm
endif else begin
    glm_space = bold_space
    ifh_dim4 = ifh[*].matrix_size_4 
    for i=0,total_nsubjects-1 do vol[i] = long(ifh[i].matrix_size_1)*long(ifh[i].matrix_size_2)*long(ifh[i].matrix_size_3)
    tc_labels = 'timecourse'
    tcmax = max(ifh_dim4)
    tcmin = 1
    ntc = 1
    tc_length = tcmax
    ifh_fwhm = ifh[*].fwhm 
endelse
if total(vol[0]-vol) ne 0 then begin
    stat=dialog_message('Not all files are in the same space.',/ERROR)
    return,rtn={msg:'ERROR'}
endif
goback7a1:
atlasspace = glm_space[0]
if atlasspace eq !SPACE_DATA then begin
    if nels ne 0 then begin
        if ls.atlasspace ne -1 then begin
            atlasspace = ls.atlasspace
        endif
    endif
    if t4select[0] ne '' then begin
        rtn=select_space()
        atlasspace=rtn.space
        atlas=rtn.atlas
        atlas_str=rtn.atlas_str
        if nels ne 0 then ls.atlas_str = atlas_str
    endif
endif
goback7a:
if glm_space[0] eq !SPACE_DATA and (atlasspace eq !SPACE_111 or atlasspace eq !SPACE_222 or atlasspace eq !SPACE_333) then begin
    if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='If your glms are in data space, you will need to provide ' $
        +'a t4 file for each.'
    if load eq 1 then begin
        gf = get_files(TITLE='Please enter atlas filter.',FILTER='*anat_ave_to_*_t4')
        if gf.msg eq 'G0_BACK' then goto,goback7a1 else if gf.msg eq 'EXIT' then return,rtn={msg:'ERROR'}
        t4files = gf.files
        nlists = 2
        if n_elements(t4files) eq 1 and total_nsubjects gt 1 then $
            t4select = make_array(total_nsubjects,/STRING,VALUE=t4files[0]) $
        else if gf.msg ne 'DONTCHECK' then $
            t4select = match_files(glmfiles,t4files,TITLE='Please select t4 file for ')
    endif
endif
goback7b:
if files_are_glms eq 1 and boy eq 0 then begin
    if nels ne 0 then begin
        scrap = get_button(['magnitudes','timecourses','GO BACK'],TITLE='Please choose',BASE_TITLE=basetitle) + 1
        if scrap eq 3 then goto,goback7
        select_est = intarr(4)
        select_est[scrap] = 1
    endif else begin
        if not keyword_set(TC) then begin

            ;rtn=get_bool_list(['z statistics','magnitudes','timecourses','timecourses - maximum deflection'],TITLE='Please choose', $
            ;    /GO_BACK,/EXIT,BASE_TITLE=basetitle)
            ;START160524
            if nreg eq 0 then $
                scrap=['z statistics','magnitudes','timecourses','timecourses - maximum deflection'] $
            else $
                scrap=['z statistics, correlation coefficients','magnitudes','timecourses','timecourses - maximum deflection']
            rtn=get_bool_list(scrap,TITLE='Please choose',/GO_BACK,/EXIT,BASE_TITLE=basetitle)
            undefine,scrap


            select_est = rtn.list
            if select_est[0] eq -1 then goto,goback7 else if select_est[0] eq -2 then return,rtn={msg:'ERROR'} 
        endif else begin
            select_est = intarr(4)
            select_est[2] = 1
        endelse
        if select_est[2] eq 1 and select_est[3] eq 1 then maxdefstr = ' -maxdef' $
        else if select_est[2] eq 0 and select_est[3] eq 1 then maxdefstr = ' -maxdefonly' 
    endelse
endif else begin
    select_est = intarr(4)
    select_est[2] = 1
endelse
if select_est[1] eq 1 and total_nsubjects gt 1 then begin
    scrap = get_button(['arithmetic mean','weighted by the inverse of the variance','go back'],BASE_TITLE='Average magnitudes', $
    TITLE='If you do not know what this is, then select the arithmetic mean.')
    if scrap eq 2 then goto,goback7b
    if scrap eq 1 then magwtstr = ' -invvar'
endif
goback7b1:
if nels eq 0 then begin
    if nreg gt 0 or vol[0] eq 1 then begin
        analysis_type = 0
        lcsee = 1
    endif else begin
        if select_est[0] eq 1 and total(select_est) eq 1 then begin
            analysis_type = get_button(['voxel by voxel','z statistics for all voxels in the regions (text output)','go back'], $
                BASE_TITLE='Please select analysis type',TITLE='Regional z statistics can only be obtained from regional GLMs')
            if analysis_type eq 2 then goto,goback7b
            if analysis_type eq 1 then voxelsstr = ' -voxels' 
        endif else begin
            analysis_type = get_button(['voxel by voxel','specfic regions','go back'],TITLE='Please select analysis type')
            if analysis_type eq 2 then goto,goback7b
        endelse
    endelse
endif
if analysis_type eq 1 and nels eq 0 then begin
    widget_control,/HOURGLASS
    rtn = get_regions(fi,wd,dsp,help)
    if rtn.msg eq 'EXIT' then return,rtn={msg:'ERROR'} else if rtn.msg eq 'GO_BACK' then goto,goback7b
    region_names = rtn.region_names
    nregfiles = rtn.nregfiles
    region_file = rtn.region_file
    region_space = rtn.space
    goback7d:
    rtn = select_files(region_names,TITLE='Please select regions you wish to analyze.')
    roi_str = strjoin(strtrim(rtn.index+1,2),' ',/SINGLE)
    lcsee = 1
    if keyword_set(SLOPES) then region_or_uncompress_str = ' -regions "'+strjoin(region_names,'" "',/SINGLE)+'"'
    if glm_space[0] ne region_space and t4select[0] eq '' then begin
        gf = get_files(TITLE='Please enter atlas filter.',FILTER='*anat_ave_to_*_t4')
        if gf.msg eq 'GO_BACK' then goto,goback7d else if gf.msg eq 'EXIT' then return,rtn={msg:'ERROR'}
        t4files = gf.files
        nlists = 2
        if n_elements(t4files) eq 1 and total_nsubjects gt 1 then $
            t4select = make_array(total_nsubjects,/STRING,VALUE=t4files[0]) $
        else if gf.msg ne 'DONTCHECK' then $
            t4select = match_files(glmfiles,t4files,TITLE='Please select t4 file for ')

        rtn=select_space(region_space)
        atlasspace=rtn.space
        atlas=rtn.atlas
        atlas_str=rtn.atlas_str
    endif
endif
if (analysis_type eq 1 or nreg gt 0) and nels eq 0 and files_are_glms eq 1 then begin
    if select_est[2] eq 1 then begin
        if nreg eq 0 then begin
            goback7e:
            scrap = get_button(['yes','no','go back'],TITLE='Do you want timecourses for all the voxels in the regions?')
            if scrap eq 2 then goto,goback7d
            if scrap eq 0 then voxelsstr = ' -voxels' 
        endif
        cols_or_rows = get_button(['columns','rows','tony format','annalisa format','annalisa format with mags','conc','go back'], $
            TITLE='Timecourses should be in')
        if cols_or_rows eq 5 then goto,goback7e
        if cols_or_rows eq 0 then begin
            cols_or_rows_str = ' -columns'
            if total_nsubjects gt 1 then begin
                scrap = get_button(['yes','no'],TITLE='Do you want individual subject timecourses?')
                if scrap eq 0 then $
                    cols_or_rows_str = cols_or_rows_str + ' INDIVIDUAL' $
                else begin
                    lcsee = 0
                endelse
                scrap = get_button(['yes','no'],TITLE='Do you want standard errors of the mean?')
                if scrap eq 0 then lcsd = ' -sem'
            endif else begin
                if total_nsubjects gt 1 then lcsee = 0
            endelse
        endif else if cols_or_rows eq 1 then begin
            if total_nsubjects gt 1 then lcsd = ' -sem'
        endif else if cols_or_rows eq 2 then begin
            cols_or_rows_str = ' -tony'
            lcsee = 0
            within_subject_sd_str = ' -within_subject_sd'
            if total_nsubjects gt 1 then lcsd = ' -sem'
        endif else if cols_or_rows eq 3 then begin
            cols_or_rows_str = ' -annalisa'
            lcsee = 0
            if total_nsubjects gt 1 then lcsd = ' -sem'
        endif else if cols_or_rows eq 4 then begin
            cols_or_rows_str = ' -annalisa'
            lcsee = 0
            if total_nsubjects gt 1 then lcsd = ' -sem'
            get_dialog_pickfile,'*.txt',fi.path,'Please select contrast file.',rtn_filenames,rtn_nfiles,rtn_path, $
                FILE=subject_get_labels_struct[0].ifh.data_file
            if rtn_filenames eq 'EXIT' then return,rtn={msg:'ERROR'} else if rtn_filenames eq 'GOBACK' then goto,goback7b1
            cols_or_rows_str = ' -annalisa '+rtn_filenames[0]
        endif else begin
            get_dialog_pickfile,'*.conc',fi.path,'Please select conc file.',rtn_filenames,rtn_nfiles,rtn_path, $
                FILE=subject_get_labels_struct[0].ifh.data_file
            if rtn_filenames eq 'EXIT' then return,rtn={msg:'ERROR'} else if rtn_filenames eq 'GOBACK' then goto,goback7b1
            concstr = ' -conc ' + rtn_filenames[0]
        endelse
    endif else if select_est[1] eq 1 then begin
        scrap = get_button(['magnitude x region','region x magnitude'],TITLE='Average magnitudes should be formatted')
        if scrap eq 1 then cols_or_rows_str = ' -regxmag'
    endif
endif
if analysis_type eq 1 and nels eq 0 then begin
    if n_elements(cols_or_rows) eq 0 then $
        regavg_type = 0 $
    else if cols_or_rows eq 2 or cols_or_rows eq 5 then $
        regavg_type = 0 $
    else $
        regavg_type = get_button(['arithmetic mean','weighted mean'],TITLE='Regional averages should be the')
    if regavg_type eq 1 then begin
        wfiles = get_list_of_files('*.img',directory,'Please select weight files.',string(indgen(1000)+1))
    endif
endif
goback8:
if (select_est[2]+select_est[3]) ge 1 then begin ;timecourses
    if files_are_glms eq 1 then begin
        ntc = neffectlabels
        scrap = factorial(ntc)
        for i=2,neffectlabels do ntc = ntc + scrap/(factorial(i)*factorial(neffectlabels-i))
        if ntc gt 10 then ntc = 10
        if ntc lt neffectlabels then ntc = neffectlabels
        if ntc le 50 then begin

            ;START150930
            ntc0=ntc
            goback81:
            if ntc gt 25 then begin
                scrap=get_str(1,'Number of timecourses',trim(ntc),LABEL='I will put up windows that will enable you to specify '$
                    +trim(ntc)+' timecourses.'+string(10B)+'This could take a few minutes. Maybe you need fewer which would be '$
                    +'much faster.',TITLE='Number of timecourses',/GO_BACK)
                if scrap[0] eq 'GO_BACK' then goto,goback7b1
                ntc=fix(scrap)
            endif

            pointandclick:
            treatment_str = strarr(ntc)
            if slopes eq 0 then lizard = 'timecourse ' else lizard = ''
            for i=0,ntc-1 do treatment_str[i] = lizard + strtrim(i+1,2)
            framesout = intarr(slopes+1)
            sumrows = -1
            index_conditions = intarr(1,neffectlabels) 
            for i=0,slopes do begin
                top_title = 'Please select conditions. Blank columns are ignored.'
                if slopes eq 1 then top_title = 'Slope '+strtrim(i+1,2)+'. '+top_title+' Order counts.'
                widget_control,/HOURGLASS
                rtn_ic = identify_conditions_new(neffectlabels,effectlabels,ntc,treatment_str,0,0, $
                    'Multiple conditions are summed. Next window will provide options.',/SKIP_CHECKS,/SET_DIAGONAL,/SET_ORDER, $
                    TOP_TITLE=top_title)
                if rtn_ic.special eq 'GO_BACK' then begin

                    ;if i eq 0 then goto,goback7b1 else goto,pointandclick
                    ;START150930
                    if i eq 0 then begin
                        if ntc0 gt 25 then goto,goback81 else goto,goback7b1 
                    endif else goto,pointandclick

                endif
                framesout[i] = n_elements(rtn_ic.sumrows)
                sumrows = [sumrows,rtn_ic.sumrows]
                ;print,'size(index_conditions)=',size(index_conditions)
                ;print,'size(rtn_ic.index_conditions)=',size(rtn_ic.index_conditions)
                index_conditions = [index_conditions,rtn_ic.index_conditions]
            endfor
            ntc = total(framesout) 
            sumrows = sumrows[1:*]
            index_conditions = index_conditions[1:*,*]
            if slopes ne 0 then framesoutstr = ' -framesout '+strjoin(trim(framesout),' ',/SINGLE)


        endif else begin
            rtn_gsgs = get_sic_get_str(neffectlabels,effectlabels)
            if rtn_gsgs.msg eq 'GO_BACK' then goto,goback6 else if rtn_gsgs.msg eq 'SPECIAL' then goto,pointandclick
            sumrows = rtn_gsgs.sumrows
            index_conditions = rtn_gsgs.index_conditions
            ntc = rtn_gsgs.ntc
        endelse
        goback9:
        loadslk=0
        idx = where(sumrows gt 1,cnt)
        if cnt ne 0 then begin
            loadslk = get_button(['weighted mean by the inverse of their variance','load slk','go back','exit'], $
                TITLE='Conditions should be weighted')
            if loadslk eq 2 then goto,goback8 else if loadslk eq 3 then return,rtn={msg:'ERROR'}
        endif
        if loadslk eq 1 then begin
            goback10:    
            rtn_slk=get_slk(help)
            if rtn_slk.msg eq 'GO_BACK' then goto,goback9 else if rtn_slk.msg eq 'EXIT' then return,rtn={msg:'ERROR'} 
            if rtn_slk.subject_id[0] eq 'ALL' then begin
                size_behav_data1 = size(rtn_slk.behav_data)
                behav_data1 = strarr(nsubject,size_behav_data1[2])
                for i=0,nsubject-1 do behav_data1[i,*] = rtn_slk.behav_data[0,*]
            endif else begin
                rtn=automatch(glm_files,t4_files,rtn_slk.subject_id,rtn_slk.slkfile,rtn_slk.nbehav_pts,rtn_slk.behav_data,/DONTSELECT)
                if rtn.msg eq 'EXIT' then return,rtn={msg:'ERROR'} else if rtn.msg eq 'GO_BACK' then goto,goback10
                nglm = rtn.nfiles
                glm_files = rtn.files
                t4_files = rtn.t4_files
                behav_data1 = rtn.behav_data
                lcdontcheckt4again=1
            endelse
            goback11:
            indvar = intarr(ntc,max(sumrows))
            for i=0,ntc-1 do begin
                scrap = strtrim(effectlabels[index_conditions[i,0:sumrows[i]-1]],2)
                scrap1 = rtn_slk.column_labels
                scrap1i = indgen(rtn_slk.nbehav_pts)
                for j=0,sumrows[i]-1 do begin
                    idx = where(strmatch(scrap1,scrap[j]) eq 1,cnt)
                    if cnt eq 0 or cnt gt 1 then begin
                        if cnt eq 0 then base_title = 'Not found' else base_title = 'More than one match found'
                        idx = get_button([scrap1,'go back','exit'],BASE_TITLE=base_title, $
                            TITLE='Please select a column for '+scrap[j])
                        lizard = n_elements(scrap1)
                        if idx eq lizard then goto,goback11 else if idx eq lizard+1 then return,rtn={msg:'ERROR'}
                   endif
                   indvar[i,j] = scrap1i[idx]
                   scrap2 = intarr(n_elements(scrap1))
                   scrap2[idx] = 1
                   idx = where(scrap2 eq 0,cnt)
                   if cnt ne 0 then begin
                       scrap1 = scrap1[idx]
                       scrap1i = scrap1i[idx]
                   endif
                endfor
            endfor
        endif
        scraplabels = replicate(ptr_new(),nsubject)
        for m=0,nsubject-1 do scraplabels[m] = subject_get_labels_struct[m].effectlabels
        scrap = cft_and_scft(nsubject,ntc,neffectlabels,sumrows,index_conditions,effectlabels,subject_get_labels_struct,0, $
            subject_names,treatment_str,0,dummy,scraplabels,/REPEATS)
        if scrap.sum_contrast_for_treatment[0,0,0] eq -2 then $
            return,rtn={msg:'ERROR'} $
        else if scrap.sum_contrast_for_treatment[0,0,0] eq -1 then $
            goto,goback8
        sum_contrast_for_treatment = scrap.sum_contrast_for_treatment
        lcmess = scrap.lcmess
        tc_length = intarr(ntc)
        tc_labels = strarr(ntc)
        tc_length_min = intarr(ntc)
        scrap = intarr(nsubject)
        for i=0,ntc-1 do begin
            goose = ''
            for j=0,nsubject-1 do begin
                scrapstr = *subject_get_labels_struct[j].effectlabels
                index = where(sum_contrast_for_treatment[j,i,*] gt 0,count)
                penguin = effect_length_ts[j,sum_contrast_for_treatment[j,i,index]-1]
                pmax = max(penguin,MIN=pmin)

                ;START140227
                ;if pmax ne pmin then begin
                ;    junk = strjoin(scrapstr[sum_contrast_for_treatment[j,i,index]-1]+' '+strtrim(penguin,2),string(10B),/SINGLE)
                ;    stat = get_button(['try again','exit'],BASE_TITLE=whoamistr,TITLE=treatment_str[i]+string(10B) $
                ;        +'You have selected '+strtrim(count,2)+' conditions with the following number of timepoints.' $
                ;        +string(10B)+junk+string(10B)+'Currently, the number of timepoints must be equal.'+string(10B) $
                ;        +'Perhaps your intention was to place the conditions under different timecourses.')
                ;    if stat eq 0 then goto,goback8 else return,rtn={msg:'ERROR'}
                ;endif

                goose = [goose,reform(scrapstr[sum_contrast_for_treatment[j,i,index]-1])]
                if j eq 0 then begin
                    tc_length[i] = pmax
                    tc_length_min[i] = pmin
                endif else begin
                    if pmax gt tc_length[i] then tc_length[i]=pmax
                    if pmin lt tc_length_min[i] then tc_length_min[i] = pmin
                endelse
            endfor
            goose = goose[1:*]
            neg = n_elements(goose)
            keepers = -1
            for j=0,neg-1 do begin
                for k=j+1,neg-1 do if goose[j] eq goose[k] then goto,continue
                keepers = [keepers,j]
                continue:
            endfor
            tc_labels[i] = strjoin(goose[keepers[1:*]],'+',/SINGLE)
        endfor
        Hz = intarr(ntc)
        Hzi = where(strpos(tc_labels,'Hz') ne -1,count)
        tcmin = tc_length_min
        tcmax = tc_length
        if count ne 0 then begin
            Hz[Hzi] = 1
            tcmin[Hzi] = tc_length_min[Hzi]/2 
            tcmax[Hzi] = tc_length[Hzi]/2 
        endif
        lc_Hz = total(Hz)
    endif else $
        Hz = intarr(ntc)
    goback100:
    if(nels ne 0) or total(tcmax-tcmin)>0 or lc_Hz then begin
        scraplabels = tc_labels
        if nels ne 0 then begin
            nframesstr = make_array(ntc,/STRING,VALUE='1')
        endif else begin
            nframesstr = strtrim(tcmax,2)
            scraplabels = scraplabels + '   most = ' + nframesstr + '   least = ' + strtrim(tcmin,2)
        endelse
        if ntc eq 1 then front = 0 else front = '1'
        scrap = get_str(ntc,scraplabels,nframesstr,TITLE='How many frames?',/GO_BACK,FRONT=front)
        if scrap[0] eq 'GO_BACK' then goto,goback8
        nframes = fix(scrap)
    endif else begin 
        nframes = tc_length
    endelse 
    maxnframes = max(nframes)
    sumrows_time_tc = intarr(ntc,maxnframes)
    conditions_time_tc = intarr(ntc,maxnframes,max(tc_length))
    goback101:
    if total_nsubjects eq 1 then mickey = '' else mickey = 'avgtc_'
    timept_label = strarr(ntc)
    special = ''
    for i=0,ntc-1 do begin
        if tc_length[i] eq 1 then begin
            nframes[i] = 1 
            conditions_time_tc[i,0:nframes[i]-1,0:tc_length[i]-1] = 1
            sumrows_time_tc[i,0:nframes[i]-1] = 1 
        endif else begin
            if Hz[i] eq 0 then begin
                spider = 'Please select an estimate for each timepoint. Blank columns are ignored.'
                goose = 'Multiple timepoints are combined as a weighted mean'+string(10B)
                if loadslk eq 0 then $
                    goose = goose + 'by the inverse of their variance.' $
                else $
                    goose = goose + 'by the weighting in your slk file.'
                lc_notHz = 1
                sameforall = 1
                skip_checks = 1
            endif else begin
                spider = 'Please select estimates in pairs.'
                goose = 'Amplitude: Multiple selections are squared, summed, and divided by the number of selected pairs.' $
                    +string(10B)+'Phase for each run.'
                lc_Hz = 1
                sameforall = 1
                skip_checks = 0
            endelse
            if nframes[i] eq 1 then begin
                set_unset = 1
                set_diagonal = 0
                default = 0
            endif else begin
                set_unset = 0
                set_diagonal = 1
                if lc_Hz eq 1 then begin
                    default = intarr(tc_length[i],nframes[i])
                    k = 0
                    for j=0,nframes[i]-1 do begin
                        default[k:k+1,j] = 1
                        k = k + 2
                    endfor
                endif
            endelse
            if special eq '' then begin
                widget_control,/HOURGLASS
                rtn_ic = identify_conditions_new(tc_length[i],string(indgen(tc_length[i])+1),nframes[i], $
                    'timepoint '+strtrim(indgen(tc_length[i])+1,2),0,0,+spider+string(10B)+goose,TOP_TITLE=tc_labels[i], $
                    SET_UNSET=set_unset,SET_DIAGONAL=set_diagonal,DEFAULT=default,SAMEFORALL=sameforall,SKIP_CHECKS=skip_checks)
                if rtn_ic.special eq 'GO_BACK' then begin
                    if(nels ne 0) or total(tcmax-tcmin)>0 or lc_Hz then goto,goback100 else goto,goback8
                endif
                special = rtn_ic.special
            endif
            sumrows_time = rtn_ic.sumrows
            nframes[i] = n_elements(rtn_ic.sumrows)
            conditions_time_tc[i,0:nframes[i]-1,0:tc_length[i]-1] = rtn_ic.index_conditions + 1
            sumrows_time_tc[i,0:nframes[i]-1] = sumrows_time
            cstr = ''
            for j=0,nframes[i]-1 do get_cstr_new,cstr,1,tc_length[i],rtn_ic.index_conditions[j,*]+1,rtn_ic.sumrows[j],'_'
            tc_labels[i] = mickey + tc_labels[i] + cstr
            timept_label[i] = cstr
        endelse 


    endfor
    framesstr = ' -frames ' + strjoin(strtrim(nframes,2),' ',/SINGLE)
    if loadslk eq 1 then begin
        s0=0
        for i=0,ntc-1 do begin
            scraparr = intarr(nsubject)
            for j=0,sumrows[i]-1 do scraparr = scraparr + (behav_data1[*,indvar[i,j]] eq '')
            index = where(scraparr eq 0,count)
            if count ne 0 then begin
                for j=0,count-1 do begin
                    for l=0,nframes[i]-1 do begin                 
                        scrap1 = strarr(sumrows_time_tc[i,l])
                        scrap2 = ''
                        for k=0,sumrows[i]-1 do begin
                            scrap1[*] = behav_data1[index[j],indvar[i,k]]
                            scrap2 = [scrap2,scrap1]
                        endfor
                    endfor
  
                    ;cstrwts[index[j]] = cstrwts[index[j]] + ' ' + strcompress(strjoin(scrap2[1:*],',',/SINGLE),/REMOVE_ALL)
                    ;START130522
                    cstrwts[index[j]] = cstrwts[index[j]] + ' ' + strjoin(trim(float(scrap2[1:*])),',',/SINGLE)

                endfor
            endif
            s0 = s0 + sumrows[i]
        endfor
        index = where(cstrwts ne '',count)
        if count ne 0 then cstrwts[index] = ' -tc_weights'+cstrwts[index]
    endif
    if lc_Hz eq 1 then begin
        goback102:
        Hzstr = ''
        amp_phase_both = get_button(['amplitude','phase','both','go back'],TITLE='I would like',BASE_TITLE='Power spectrum')
        if amp_phase_both eq 3 then goto,goback101
        if amp_phase_both eq 0 then $
            ampstr = ' -amp' $
        else if amp_phase_both eq 1 then begin 
            phasestr = ' -phase' 
            lcphaseonly = 1
        endif else begin
            ampstr = ' -amp'
            phasestr = ' -phase'
        endelse 
        goback102a:
        if amp_phase_both eq 0 or amp_phase_both eq 2 then begin
            scrap = get_button(['root mean square amplitude power','mean square amplitude power','go back'],TITLE='I would like', $
                BASE_TITLE='Amplitude')
            if scrap eq 2 then goto,goback102
            if scrap eq 0 then ampstr=ampstr+' rms' else ampstr = ampstr+' ms' 
        endif
        goback103:
        if total_nsubjects gt 1 and analysis_type eq 0 and vol[0] gt 1 and nreg eq 0 then begin
            scrap = get_button(['yes','no','go back'],TITLE='Output individual subject images?')
            if scrap eq 2 then begin
                if amp_phase_both eq 0 or amp_phase_both eq 2 then goto,goback102a else goto,goback102
            endif
            if scrap eq 0 then indhipassstr = ' -indhipass'
        endif
        if vol[0] eq 1 then begin
            cols_or_rows_str = ' -columns'
            lcsd = ' -sem'
        endif
    endif
    goback104:
    scrap = ''
    if concstr ne '' then $
        scrap = tc_labels $
    else begin
        if nreg ne 0 then begin 
            if atlasspace ne !UNKNOWN then scrap = tc_labels
        endif else begin
            if (analysis_type eq 0 and vol[0] gt 1 and ntc le 50) or keyword_set(SLOPES) then begin 
                if loadslk eq 1 then begin
                    if rtn_slk.subject_id[0] eq 'ALL' then begin
                        tc_labels = make_array(ntc,/STRING,VALUE='')
                        for i=0,ntc-1 do begin
                            for j=0,sumrows[i]-1 do begin
                                spider = strmid(effectlabels[index_conditions[i,j]],0,1)
                                if spider eq '-' or spider eq '0' or spider eq '1' or spider eq '2' or spider eq '3' $
                                    or spider eq '4' or spider eq '5' or spider eq '6' or spider eq '7' or spider eq '8' $
                                    or spider eq '9' then bunny = '_' else bunny = ''
                                tc_labels[i] = tc_labels[i]+behav_data1[0,indvar[i,j]]+bunny+effectlabels[index_conditions[i,j]]
                            endfor
                        endfor
                        tc_labels = mickey + tc_labels + timept_label
                    endif
                endif
                if slopes eq 0 then begin
                    if root ne '' then junk = root + '_' else junk = ''

                    ;START170127
                    if n_elements(treatment_str) eq 0 then treatment_str='tc'+trim(indgen(ntc)+1)

                    goose = junk+tc_labels
                    title='Timecourse roots'
                    scrap = get_str(ntc,treatment_str,goose,TITLE=title,/ONE_COLUMN,/GO_BACK,WIDTH=100)
                    if scrap[0] eq 'GO_BACK' then begin
                        if lc_Hz eq 0 then goto,goback100 else goto,goback103
                    endif
                endif else begin
                    scrap = ['slope1','slope2']
                endelse
            endif
        endelse
    endelse
    if scrap[0] ne '' then tc_labels_str = ' -tc_names "' + strjoin(scrap,'" "',/SINGLE) + '"'
    tc_roots = scrap
endif
if select_est[0] eq 1 or select_est[1] eq 1 then begin ;zstats and mags
    goback7c:
    melts = max(effectlength)
    if melts gt 1 then begin
        lcdelays = get_delays(delay_str)
        if lcdelays eq 'GOBACK' then goto,goback7
    endif
    if select_est[0] eq 0 and select_est[1] eq 1 then begin ;mag only
        title = 'Please select magnitudes'
    endif else if select_est[0] eq 1 and select_est[1] eq 1 then begin ;zstat and mag
        title = 'Please select z statistics and magnitudes'
    endif else begin ;zstat only
        title = 'Please select z statistics'
    endelse
    contrast_labels = *subject_get_labels_struct[nc_max_glm_index].contrastlabels
    rtn = get_searchstr(contrast_labels,title)
    if rtn.label[0] eq 'GO_BACK' then begin
        if melts gt 1 then goto,goback7c else goto,goback7b
    endif
    ncontrast = rtn.count
    sumrows = make_array(ncontrast,/INT,VALUE=1)
    scrap = cft_and_scft(nsubject,ncontrast,ncontrastlabels,sumrows,rtn.index,rtn.searchstr,subject_get_labels_struct,1, $
        subject_names,rtn.label,0,'Please select contrast.',/MATCH)
    if scrap.sum_contrast_for_treatment[0,0,0] eq -2 then $
        return,rtn={msg:'ERROR'} $
    else if scrap.sum_contrast_for_treatment[0,0,0] eq -1 then $
        goto,goback8
    contrast = scrap.sum_contrast_for_treatment

    ;c_labels_str = ' -c_names "' + strjoin(rtn.label,'" "',/SINGLE) + '"'
    ;START121019
    c_names = rtn.label
    c_labels_str = ' -c_names "' + strjoin(c_names,'" "',/SINGLE) + '"'

    if select_est[1] eq 1 then begin
        scrap = 0
        m = 0
        repeat begin
            cnorm = *subject_get_labels_struct[m].cnorm
            cnorm = cnorm[*,contrast[m,0:ncontrast-1]-1,0]
            neff_per_contrast = total(abs(cnorm),1)
            index = where(neff_per_contrast gt 1,count)
            if count ne 0 then begin
                if dialog_message('Do you want the contrast weights normalized to have a magnitude of 1?',/QUESTION) eq 'Yes' then $
                   magnorm_str = ' -magnorm'
                scrap = 1
            endif
            m = m + 1
        endrep until scrap eq 1 or m eq total_nsubjects
    endif
endif
goback199:
if select_est[1] eq 1 then begin
    scrap = get_button(['percent change','MR units','go back'],TITLE='Output',BASE_TITLE='Magnitudes')
    if scrap eq 2 then goto,goback7c
    if scrap eq 1 then print_unscaled_mag_str = ' -print_unscaled_mag'
endif
if select_est[2] eq 1 and files_are_glms eq 1 then begin ;tc
    goback210:
    if pop_ind eq 0 and lcphaseonly eq 0 then begin
        if nels eq 0 then $
            goose=['percent change','MR units','both','go back'] $
        else $
            goose=['percent change','MR units','go back']
        scrap = get_button(goose,TITLE='I want timecourses in')
        if scrap eq n_elements(goose)-1 then begin
            if lc_Hz eq 1 then goto,goback102a else if select_est[1] eq 1 then goto,goback199 else goto,goback104
        endif
        if scrap eq 0 then $ ;need fof goback to work
            tc_type = ''
        if scrap eq 1 then $
            tc_type = ' -print_unscaled_tc -dont_print_scaled_tc' $
        else if scrap eq 2  then $
            tc_type = ' -print_unscaled_tc'

        ;START130403 KEEP
        ;if nels eq 0 then begin
        ;    if total_nsubjects gt 1 then begin
        ;        if lc_notHz eq 1 then begin
        ;            scrap = get_button(['yes','no','GO BACK'],TITLE='Do you want the standard deviation?'+string(10B) $
        ;                +'THIS IS NOT THE STANDARD ERROR OF THE MEAN.')
        ;            if scrap eq 2 then goto,goback210
        ;            if scrap eq 0 then lcsd = ' -sd'
        ;        endif
        ;    endif
        ;endif else if pop_ind eq 0 then $
        ;    lcsd = ' -sd'

        goback211:
        if boy eq 1 then begin
            goback212:
            stimlenstr=''
            scrap = make_array(ntc,/STRING,VALUE='4-8')
            if ntc eq 3 then begin
                scrap[1]='8-12'
                scrap[2]='12-16'
            endif
            scrap = get_str(ntc,scraplabels,scrap,LABEL='Please provide a range for the duration in seconds.'+string(10B) $
                +'Ex. 4-8',/GO_BACK,FRONT=front,TITLE='Nonlinear least squares')
            if scrap[0] eq 'GO_BACK' then goto,goback211
            for i=0,ntc-1 do $
                scrap[i]=strjoin(strsplit(scrap[i],'[ '+string(9B)+string(44B)+string(45B)+']+',/REGEX,/EXTRACT),',',/SINGLE)
            stimlenstr = ' -stimlen '+strjoin(strtrim(scrap,2),' ',/SINGLE)
        endif
    endif
endif
goback200:
lc204=0
if analysis_type eq 0 and nreg eq 0 then begin
    if select_est[0] eq 1 or select_est[1] eq 1 then begin
        if total_nsubjects gt 1 then begin
            goback202:
            scrap = get_button(['yes','no','go back'],TITLE='Do you want individual subject images?')
            if scrap eq 2 then goto,goback199
            if scrap eq 0 then accumulate_str = ' -accumulate'
            goback203:

            ;START130109
            lc203=0

            if accumulate_str ne '' then begin
                rtn = get_bool_list(['separate file for each subject','one file - each subject a timepoint'],TITLE='I want', $
                    /GO_BACK,/EXIT)
                if rtn.list[0] eq -1 then goto,goback202 else if rtn.list[0] eq -2 then return,rtn={msg:'ERROR'}
                if rtn.list[0] eq 1 then accumulate_str = accumulate_str + ' SEPARATE'
                if rtn.list[1] eq 1 then accumulate_str = accumulate_str + ' ONE'
               
                ;START130109
                lc203=1

            endif
        endif
        goback204:
        scrap = get_str(ncontrast,c_names,c_names,LABEL='Please name',/GO_BACK,WIDTH=50)
        if scrap[0] eq 'GO_BACK' then begin
            if total_nsubjects gt 1 then begin
                if accumulate_str ne '' then goto,goback203 else goto,goback202
            endif else $
                goto,goback199
            lc204=1
        endif
        c_labels_str = ' -c_names "' + strjoin(scrap,'" "',/SINGLE) + '"'
    endif
endif else if concstr ne '' then $
    tc_type = ' -print_unscaled_tc -dont_print_scaled_tc'
goback205:
lc205=0
if total(select_est[1:2]) ne 0 and lcsee eq 1 and concstr eq '' and total_nsubjects eq 1 and lc_Hz eq 0 then begin 
    scrap = get_button(['yes','no'],TITLE='Do you want standard errors of the estimates?')
    if scrap eq 0 then within_subject_sd_str = ' -within_subject_sd'
    lc205=1
endif
goback206:
lc206=0

;START130109
;gauss_str = ''

if asksmooth eq 1 then begin
    if atlasspace ne !SPACE_DATA and analysis_type eq 0 and nreg eq 0 and vol[0] gt 1 then begin
        fwhm0 = 0.
        lc2060=0
        if files_are_glms eq 0 then scrap = 'images' else scrap = 'GLMs'
        if total(ifh_fwhm - ifh_fwhm[0]) ne 0 then begin
            scrap = get_button(['ok','exit'],BASE_TITLE='Information',TITLE='Not all images have been smoothed the same amount.')
            if scrap eq 1 then return,rtn={msg:'ERROR'}
            fwhm0 = max(ifh_fwhm[*])
            lc2060=1
        endif else if glm_space[0] eq !SPACE_111 or glm_space[0] eq !SPACE_222 or glm_space[0] eq !SPACE_333 then begin

            ;lizard = get_str(1,'FWHM in voxels ',ifh_fwhm[0],WIDTH=50,/GO_BACK, $
            ;    TITLE='Have your '+scrap+' been smoothed?',LABEL='Enter 0 if they have not been smoothed.')
            ;START170127
            if ifh_fwhm[0] eq -1 then ifh_fwhm0=0 else ifh_fwhm0=ifh_fwhm[0]
            lizard = get_str(1,'FWHM in voxels ',ifh_fwhm0,WIDTH=50,/GO_BACK, $
                TITLE='Have your '+scrap+' been smoothed?',LABEL='Enter 0 if they have not been smoothed.')

            if lizard[0] eq 'GO_BACK' then begin
                if lc205 eq 1 then goto,goback205 else if lc204 eq 1 then goto,goback204 $
                else if lc203 eq 1 then goto,goback203 else goto,goback199
            endif
            fwhm0 = double(lizard[0])
            lc2060=1
        endif
        goback207:
        gauss_str = ''
        if fwhm0 eq 0. then begin
            title = 'You may smooth your '+scrap+' if you wish.'
            label = 'Enter 0 for no smoothing.'
            if nsubject eq 1 then scrap='0' else scrap='2'
        endif else begin
            title = 'You may do some additional smoothing if you wish.'
            label = 'Enter 0 for no additional smoothing.'
            scrap = '0'
        endelse
        if slopes eq 0 then $
            special = 0 $
        else begin
            label=label+string(10B)+string(10B)+"Hit 'OK' for the data to be smoothed prior to slope and intercept estimation." $
                +string(10B)+'You will have an option to smooth after estimation later. You may do both.'
        endelse
        scrap = get_str(1,'3D Gaussian kernel FWHM in voxels ',scrap,/GO_BACK,TITLE=title,LABEL=label,WIDTH=50,SPECIAL=special)
        if scrap[0] eq 'GO_BACK' then begin
            if lc2060 eq 1 then goto,goback206 else goto,goback200
        endif
        if scrap[0] eq 'SPECIAL' then idx=1 else idx=0
        fwhm = double(scrap[idx])
        if fwhm gt 0.01 and idx eq 0 then gauss_str = ' -gauss_smoth '+ trim(fwhm)
        monte_carlo_fwhm = round(sqrt(fwhm0^2+fwhm^2))
        if nels ne 0 then ls.gauss_str = gauss_str
        lc206=1



        



    endif
    if analysis_type eq 0 then begin
        if atlasspace eq !SPACE_111 or atlasspace eq !SPACE_222 or atlasspace eq !SPACE_333 then begin
            mask = ''
            widget_control,/HOURGLASS
            scrap  = get_mask(atlasspace,fi)
            if scrap eq 'GO_BACK' then begin
                if lc206 eq 1 then goto,goback207 else goto,goback200
            endif 
            if scrap ne '' then begin
                Nimage_mask_str = ' -mask ' + scrap
                mask = scrap
            endif
            if nels ne 0 then ls.mask_str = Nimage_mask_str 
        endif
    endif
endif
if select_est[2]+select_est[3] gt 0 and slopes ne 0 then begin
    lcgroup=0
    lcind=1
    nglm_per_subject_str=''
endif else if select_est[2] eq 1 and analysis_type eq 0 and nreg eq 0 and total_nsubjects gt 1 then begin

    ;rtn = get_bool_list(['group timecourses','individual timecourses'],TITLE='I want',/GO_BACK,/EXIT)
    ;START170616
    rtn = get_bool_list(['group timecourses','individual timecourses','sum'],TITLE='I want',/GO_BACK,/EXIT)

    scrap = rtn.list
    if scrap[0] eq -1 then goto,goback206 else if scrap[0] eq -2 then return,rtn={msg:'ERROR'}
    lcgroup = scrap[0]
    lcind = scrap[1]

    ;START170616
    lcsum=scrap[2]
   
endif
if files_are_glms eq 0 then begin
    labelstr=''
    if tc_length[0] gt 1 then begin
        scrap = get_str(3,['label','TR (s)','start time (s)'],['closed','3.013','-313.352'],TITLE='Leave blank for no labels', $
            LABEL = 'Each line of the text file will be time_label ex. -313.352_closed will be the label for the first line', $
            WIDTH=50,/GO_BACK)
        if scrap[0] eq 'GO_BACK' then goto,goback8
        if scrap[0] ne '' then labelstr = ' -label "'+scrap[0]+'"'
        if scrap[1] ne '' and scrap[2] ne '' then labelstr=labelstr+' -TR '+scrap[1]+' -starttime '+scrap[2]
    endif
endif
lcmag=''
if select_est[0] eq 1 then lcmag = ' -zstat'
if select_est[1] eq 1 then lcmag = lcmag + ' -mag' 
if total_nsubjects eq 1 then lcsd = ''
goback300:
scratchdir_str = ''
if slopes ne 0 then begin
    widget_control,/HOURGLASS
    scratchdir = fidl_scratchdir()
    if scratchdir eq 'ERROR' then return,rtn={msg:'ERROR'}
    scratchdir_str = ' -scratchdir ' + scratchdir
    script = 'fidl_slopesintercepts.csh'
    script_namesonly = scratchdir+'compute_avg_zstat_namesonly.csh'
    scrap = strpos(scratchdir,'SCRATCH')
    scratchdir=strmid(scratchdir,scrap,strpos(scratchdir,'/',/REVERSE_SEARCH)-scrap+1)
endif else if nels eq 0 then begin

    ;START160524
    spawn,!BINEXECUTE+'/fidl_timestr2',timestr


    scrap = ['output storage path','script']
    scraplabels = directory

    ;if files_are_glms eq 1 then scraplabels=[scraplabels,'compute_avg_zstat.csh'] else scraplabels=[scraplabels,'fidl_bolds.csh']
    ;START160524
    if files_are_glms eq 1 then scraplabels=[scraplabels,'fidl_avg_zstat_'+timestr[0]+'.csh'] else $
        scraplabels=[scraplabels,'fidl_bolds_'+timestr[0]+'.csh']

    if analysis_type eq 0 and nreg eq 0 and vol[0] gt 1 then begin
        scrap = [scrap,'Number of subjects at each voxel']
        scraplabels = [scraplabels,'Nimage.4dfp.img']
        label = 'Leaving Nimage blank will prevent its output.'
    endif else begin
        scrap = [scrap,'average statistics output']
        if total_nsubjects eq 1 then begin
            if concstr eq '' then $

                ;scraplabels = [scraplabels,root+'.txt'] $
                ;START160524
                scraplabels = [scraplabels,root+'_'+timestr[0]+'.txt'] $

            else $
                scraplabels = [scraplabels,root+'_predicted.conc']
        endif else begin

            ;if files_are_glms eq 1 then $
            ;    scraplabels=[scraplabels,'avg_stats_for_regions.txt'] $
            ;else $
            ;    scraplabels=[scraplabels,'fidl_bolds.txt']
            ;START160524
            if files_are_glms eq 1 then $
                scraplabels=[scraplabels,'avg_stats_for_regions_'+timestr[0]+'.txt'] $
            else $
                scraplabels=[scraplabels,'fidl_bolds_'+timestr[0]+'.txt']


        endelse
        label = '' 
    endelse
    if load eq 1 and total_nsubjects gt 1 then begin
        scrap = [scrap,'GLM list']
        scraplabels = [scraplabels,'list_of_glms.glm_list']
    endif
    if one_file_str ne '' then begin
        scrap = [scrap,'Output root']
        scraplabels = [scraplabels,'mag']
    endif
    names = get_str(n_elements(scrap),scrap,scraplabels,WIDTH=50,TITLE='Please enter desired filenames.',LABEL=label,/GO_BACK)
    if names[0] eq 'GO_BACK' then begin 
        if select_est[2] eq 1 and files_are_glms eq 1 then begin ;tc
            if pop_ind eq 0 and lcphaseonly eq 0 then begin
                if stimlenstr eq '' then goto,goback210 else goto,goback212 
            endif
        endif else $
            goto,goback200
    endif
    directory = names[0]
    if directory ne '' then begin
        if strmid(directory,0,1) ne '/' then directory = '/' + directory
        if strmid(directory,strlen(directory)-1,1) ne '/' then directory = directory + '/'
        directory_str = ' -directory ' + directory
    endif
    script = directory+fix_script_name(names[1])
    j = 2
    if analysis_type eq 0 and nreg eq 0 and vol[0] gt 1 then begin
        if names[2] ne '' then begin
            if rstrpos(names[2],'.4dfp.img') eq -1 then names[2] = names[2] + '.4dfp.img'
            Nimage_name_str = ' -Nimage_name "' + names[2] + '"'
        endif
    endif else begin 
        if files_are_glms eq 1 then $
            avgstat_output_str = ' -regional_avgstat_name "' + names[2] + '"' $
        else $
            avgstat_output_str = ' -avgacrossfiles "' + names[2] + '"'
    endelse
    if load eq 1 and total_nsubjects gt 1 then begin
        j = j + 1
        glm_list_str = directory+names[j]
    endif
    if one_file_str ne '' then begin
        j = j + 1
        one_file_str = one_file_str + ' "' + names[j] + '"'
    endif
endif
if load eq 1 and total_nsubjects gt 1 and glm_list_str[0] ne '' then begin
    if t4select[0] ne '' then $
        write_list,glm_list_str,[total_nsubjects,total_nsubjects],[glmfiles,t4select] $
    else begin 

        ;write_list,glm_list_str,total_nsubjects,glmfiles
        ;START180226
        if glmfiles[0] ne '' then write_list,glm_list_str,total_nsubjects,glmfiles

    endelse
endif

ntreatments = [ntc,ncontrast]
if glm_list_str[0] ne '' then glm_list_str = ' -glm_list_file '+strjoin(glm_list_str,' ',/SINGLE)
if files_are_glms eq 1 then begin
    binary_name = 'fidl_avg_zstat2'
    if lcgroup eq 1 then begin
        compute_avg_zstat_csh,script,1,0,total_nsubjects,glmfiles,t4select,0,ntreatments,'','', $
            gauss_str,region_file,cols_or_rows_str,roi_str,avgstat_output_str,Nimage_name_str,Nimage_mask_str,delay_str, $
            within_subject_sd_str,magnorm_str, $
            nframes,effect_length_ts,conditions_time_tc,sumrows_time_tc,sum_contrast_for_treatment, $
            tc_type,print_unscaled_mag_str,lcmag,glm_list_str,directory_str,contrast,wfiles,lcsd, $
            accumulate_str+one_file_str+tc_labels_str+c_labels_str+Hzstr+concstr+indhipassstr+atlas_str+magwtstr+stimlenstr $
            +framesstr+voxelsstr+nglm_per_subject_str+ampstr+phasestr+maxdefstr, $
            nglms_per_sub,mail_str,append,binary_name,nsubject,behav_data1,indvar
        if lcind eq 1 then append=1
    endif
    if lcind eq 1 then begin
        if t4select[0] eq '' then t4select_temp = make_array(total_nsubjects,/STRING,VALUE='') else t4select_temp = t4select
        Nimage_name_str_temp = ''
        widget_control,/HOURGLASS
        for i=0,nsubject-1 do begin
            tc_labels_str = ' -tc_names "' + strjoin(rtn_root.file[i]+ '_'+tc_roots,'" "',/SINGLE) + '"'
            compute_avg_zstat_csh,script,1,0,1,glmfiles[i],t4select_temp[i],0,ntreatments,'','', $
                gauss_str,region_file,cols_or_rows_str,roi_str,avgstat_output_str,Nimage_name_str_temp,Nimage_mask_str,delay_str, $
                within_subject_sd_str,magnorm_str, $
                nframes,effect_length_ts[i,*],conditions_time_tc,sumrows_time_tc,sum_contrast_for_treatment[i,*,*], $
                tc_type,print_unscaled_mag_str,lcmag,glm_list_str,directory_str,contrast,wfiles,lcsd, $
                accumulate_str+one_file_str+tc_labels_str+c_labels_str+Hzstr+concstr+indhipassstr+atlas_str+magwtstr+framesstr $
                +voxelsstr+' -glmpersub '+trim(nglm_per_subject[i])+ampstr+phasestr+cstrwts[i]+scratchdir_str+maxdefstr+framesoutstr, $
                nglms_per_sub,mail_str,append,binary_name,1
            append = 1
        endfor
        if slopes ne 0 then begin
            append = 0
            widget_control,/HOURGLASS
            for i=0,nsubject-1 do begin
                tc_labels_str = ' -tc_names "' + strjoin(rtn_root.file[i]+ '_'+tc_roots,'" "',/SINGLE) + '"'
                compute_avg_zstat_csh,script_namesonly,1,0,1,glmfiles[i],t4select_temp[i],0,ntreatments,'','', $
                    gauss_str,region_file,cols_or_rows_str,roi_str,avgstat_output_str,Nimage_name_str_temp,Nimage_mask_str,delay_str, $
                    within_subject_sd_str,magnorm_str, $
                    nframes,effect_length_ts[i,*],conditions_time_tc,sumrows_time_tc,sum_contrast_for_treatment[i,*,*], $
                    tc_type,print_unscaled_mag_str,lcmag,glm_list_str,directory_str,contrast,wfiles,lcsd, $
                    accumulate_str+one_file_str+tc_labels_str+c_labels_str+Hzstr+concstr+indhipassstr+atlas_str+magwtstr+framesstr $
                    +voxelsstr+' -glmpersub '+trim(nglm_per_subject[i])+ampstr+phasestr+cstrwts[i]+scratchdir_str+maxdefstr $
                    +framesoutstr+' -names_only',nglms_per_sub,mail_str,append,binary_name,1
                append = 1
            endfor
        endif
    endif
endif else begin

    openw,lu,script,/GET_LUN
    top_of_script,lu
    print_files_to_csh,lu,total_nsubjects,glmfiles,'TC_FILES','tc_files',/NO_NEWLINE,/QUOTES
    wallace=''
    if t4select[0] ne '' then begin
        print_files_to_csh,lu,total_nsubjects,t4select,'T4_FILES','xform_file',/NO_NEWLINE
        wallace = ' $T4_FILES -t4 '+strjoin(trim(indgen(total_nsubjects)+1),' ',/SINGLE)
    endif
    printf,lu,'set FRAMES_OF_INTEREST = (-frames_of_interest \'
    for i=0,total_nsubjects-1 do begin
        cstr = '    '
        for k=0,nframes[0]-1 do rtn=get_cstr_new2(cstr,i+1,ifh_dim4[0:i],conditions_time_tc[0,k,*],sumrows_time_tc[0,k],' ')
        printf,lu,cstr+' \'
    endfor
    printf,lu,'     )'

    ;print_files_to_csh,lu,nregfiles,region_file,'REGION_FILE','region_file'
    ;print_files_to_csh,lu,n_elements(roi_str),roi_str,'ROI','regions_of_interest'
    ;printf,lu,'nice +19 $BIN/fidl_bolds $TC_FILES $FRAMES_OF_INTEREST $REGION_FILE $ROI'+avgstat_output_str+labelstr+wallace
    ;START170131
    if nregfiles gt 0 then begin
        print_files_to_csh,lu,nregfiles,region_file,'REGION_FILE','region_file'
        print_files_to_csh,lu,n_elements(roi_str),roi_str,'ROI','regions_of_interest'
        wallace=wallace+' $REGION_FILE $ROI'
    endif else begin
        wallace=wallace+gauss_str+Nimage_mask_str
        if lcgroup eq 1 then wallace=wallace+' -avgacrossfiles "'+fix_script_name(tc_roots[0])+'.4dfp.img"'

        ;START170616
        if lcsum eq 1 then wallace=wallace+' -sumacrossfiles "'+fix_script_name(tc_roots[0])+'"'

    endelse
    printf,lu,'nice +19 $BIN/fidl_bolds $TC_FILES $FRAMES_OF_INTEREST'+wallace+avgstat_output_str+labelstr

    close,lu
    free_lun,lu
    spawn,'chmod +x '+script
endelse
if nels eq 0 then begin
    if slopes eq 0 then begin
        action = get_button(['execute','return','go back'],BASE_TITLE=script,TITLE='Please select action.')
        if action eq 2 then goto,goback300
        if action eq 0 then begin
            scrap='>'+script+'.log'
            spawn,script+scrap+' &'
            stats = dialog_message(script+string(10B)+string(10B)+'Script has been executed.',/INFORMATION)
        endif
    endif
endif else begin
    if ls.tc_type eq '' then ls.tc_type = tc_type
    if ls.atlasspace eq -1 then $
        ls.atlasspace = atlasspace $
    else begin
        if ls.atlasspace ne atlasspace then stat=dialog_message('Population and individual must be in the same space.',/ERROR)
    endelse
    goose = 'junk.csh'
    compute_avg_zstat_csh,goose,1,0,total_nsubjects,glmfiles,t4select,0,ntreatments,'','', $
        gauss_str,region_file,cols_or_rows_str,roi_str,avgstat_output_str,Nimage_name_str,Nimage_mask_str,delay_str, $
        within_subject_sd_str,magnorm_str, $
        nframes,effect_length_ts,conditions_time_tc,sumrows_time_tc,sum_contrast_for_treatment, $
        tc_type,print_unscaled_mag_str,lcmag,glm_list_str,directory_str,contrast,wfiles,lcsd, $
        accumulate_str+one_file_str+tc_labels_str+c_labels_str+atlas_str+' -names_only',nglms_per_sub,mail_str
    spawn,goose,result
    print,'result=',result
    if analysis_type eq 0 then begin 
        if pop_ind eq 0 then nfiles = 2 else nfiles = 1
    endif else $
        nfiles = 1
    if n_elements(result) ne nfiles then begin
        stat=dialog_message('Wrong number of files.',/ERROR)
    endif else begin
        scrap = strsplit(result[0],/EXTRACT)
        ls.mean = scrap[n_elements(scrap)-1]
        if nfiles eq 2 then begin
            scrap = strsplit(result[1],/EXTRACT)
            ls.sd = scrap[n_elements(scrap)-1]
        endif
    endelse
    spawn,'rm '+goose
endelse

;if nels eq 0 then print,'DONE'
;START130118
if nels eq 0 and slopes eq 0 then print,'DONE'
if slopes eq 0 then $
    rtn={msg:'OK'} $
else $
    return,rtn={msg:'OK',script_namesonly:script_namesonly,script:script,nsubject:nsubject,scratchdir:scratchdir, $
        roots:rtn_root.file,monte_carlo_fwhm:monte_carlo_fwhm,mask:mask,atlasspace:atlasspace, $
        region_or_uncompress_str:region_or_uncompress_str}
return,rtn

end
