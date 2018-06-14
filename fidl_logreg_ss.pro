;Copyright 12/28/02 Washington University.  All Rights Reserved.
;fidl_logreg_ss.pro  $Revision: 1.45 $
function fidl_logreg_ss,fi,dsp,wd,glm,help,stc,pref,cc
if n_elements(cc) ne 0 then lccc = 1 else lccc = 0 
directory = getenv('PWD')
anova_output_str = ''
mask_str = ''
all_or_ROC = 2
logregwts_str = ''
mailstr = ''
lrs_name = ''
ROCarea_str=''
nbehav_var = 0
boldconditions_str = ''
names_tc_str = ''
index_tc_str = ''
bf_str = ''
frames_cond_str = ''
filename_str = ''
reducedmodelsstr = ''
glmlist=''
compress_str=''
scratchdir = fidl_scratchdir(/NOPATH)
if scratchdir eq 'ERROR' then return,-1
print,'scratchdir = ',scratchdir
scratchdir_str = ' -scratchdir ' + scratchdir

if lccc eq 0 then begin
    region_str = ''
    roi_str = ''
    t4_str = ''
    wfiles = ''

    ;gg=get_glms(pref,GET_THIS='concs/lists/imgs/nii.gz',/NOTMORETHANONE,FILTER='*.nii.gz')
    ;START180430
    gg=get_glms(pref,GET_THIS='concs/lists/imgs/nii.gz',/NOTMORETHANONE,FILTER='*.nii.gz',/QUIET)

    if gg.msg eq 'GO_BACK' or gg.msg eq 'EXIT' then return,-1
    files=gg.imgselect
    space=gg.bold_space
    if gg.glm_list[0] ne '' then glmlist=strjoin(gg.glm_list,' ',/SINGLE)
    ls=load_stitch(fi,stc,help,gg.total_nsubjects,files,'',1,'',gg.filetype)
    if ls.msg ne 'OK' then return,-1 
    tdim_file = *ls.stc1.tdim_file
    nrun = ls.stc1.n
    tdim_sum = *ls.stc1.tdim_sum
    t_to_file = *ls.stc1.t_to_file
    tdim = ls.stc1.tdim_all
    undefine,gg,ls
endif else begin
    data_idx = cc.data_idx
    sng = cc.sng
    nframes = cc.nframes
    region_str = cc.region_str
    roi_str = cc.roi_str
    t4_str = cc.t4_str
    wfiles = cc.wfiles
    bf_str = cc.bf_str
    anova_output_str = ' -regional_name "' + cc.out_str + '"'
    csh = cc.csh
    files = *stc[data_idx].filnam
    tdim_file = *stc[data_idx].tdim_file
    nrun = stc[data_idx].n
    tdim_sum = *stc[data_idx].tdim_sum
    t_to_file = *stc[data_idx].t_to_file
    tdim = stc[data_idx].tdim_all
    space = fi.space[data_idx]
endelse
goback1:
if lccc eq 0 then begin

    ;get_dialog_pickfile,'*slk',fi.path[0],'Please select an event or slk file.',filename,rtn_nfiles,rtn_path
    ;if filename eq 'GOBACK' or filename eq 'EXIT' then return,-1
    ;filename_str = ' -event_file "'+filename+'"'
    ;if strmid(filename,strlen(filename)-3) ne 'slk' then begin
    ;START180430
    get_dialog_pickfile,'*slk',fi.path[0],'Please select slk files or an event file.',filename,nfilename,rtn_path,/MULTIPLE_FILES
    if filename[0] eq 'GOBACK' or filename[0] eq 'EXIT' then return,-1
    if strmid(filename[0],strlen(filename[0])-3) ne 'slk' then begin
        filename_str = ' -event_file "'+filename+'"'


        sng = define_single_trial(fi,pref,help,EVENT_FILE=filename)
        if sng.event_file eq '' or sng.event_file eq 'NONE' then return,-1
    endif else begin

        ;gs=get_slk(help,filename,'NA')
        ;if gs.msg eq 'GO_BACK' then goto,goback1 else if gs.msg eq 'EXIT' then return,-1
        ;slkdata=gs.behav_data
        ;slkcollab=gs.column_labels
        ;slkfile=gs.slkfile
        ;goback1a0:
        ;sf=select_files(slkcollab,TITLE='Please select variables',/GO_BACK,/EXIT,MIN_NUM_TO_SELECT=-1,/NO_ALL,/ALIGN_LEFT,/ONE_COLUMN)
        ;if sf.files[0] eq 'GO_BACK' then goto,goback1 else if sf.files[0] eq 'EXIT' then return,-1
        ;nbehav_var=sf.count
        ;behav_str=sf.files 
        ;goback1a:
        ;am=automatch(files,t4files,gs.subject_id,filename,gs.nbehav_pts,gs.behav_data,grptag,gs.column_labels,nglm_per_subject,
        ;    /DONTSELECT)
        ;if am.msg eq 'EXIT' then return,-1 else if am.msg eq 'GO_BACK' then goto,goback1a0
        ;slkn=sf.count
        ;nrun=am.nfiles
        ;files=am.files
        ;slkdata=am.behav_data[*,sf.index]
        ;idx0=-1
        ;cnt0=intarr(slkn)
        ;for i=0,slkn-1 do begin
        ;    idx=where(slkdata[*,i] eq 'NA',cnt)
        ;    if cnt ne 0 then begin
        ;        idx0=[idx0,idx]
        ;        cnt0[i]=cnt
        ;    endif
        ;endfor
        ;if total(cnt0) ne 0 then begin
        ;    slklistnomiss=make_array(am.nfiles,/INTEGER,VALUE=1)
        ;    idx0=idx0[1:*]
        ;    j=0
        ;    for i=0,slkn-1 do begin
        ;        if cnt0[i] gt 0 then begin
        ;            slklistnomiss[idx0[j:j+cnt0[i]-1]]=0
        ;            j=j+cnt0[i]
        ;        endif
        ;    endfor
        ;    idx=where(slklistnomiss eq 1,cnt)
        ;    if cnt ne 0 then begin
        ;        nrun=cnt  
        ;        files=files[idx]
        ;        slkdata=slkdata[idx,*]
        ;    endif
        ;    undefine,slklistnomiss,idx0,cnt0,idx,cnt
        ;endif
        ;undefine,sf
        ;START180430
        gr=get_root(filename)

        gs=get_slk(help,filename[0],'NA')
        if gs.msg eq 'GO_BACK' then goto,goback1 else if gs.msg eq 'EXIT' then return,-1
        if n_elements(filename) gt 1 then begin
            gs2=get_slk(help,filename[1],'NA')
            if gs2.msg eq 'GO_BACK' then goto,goback1 else if gs.msg eq 'EXIT' then return,-1
        endif

        goback1a0:
        sf=select_files(gs.column_labels,BASE_TITLE=gr.file[0],TITLE='Please select variables',/GO_BACK,/EXIT,MIN_NUM_TO_SELECT=-1, $
            /NO_ALL,/ALIGN_LEFT,/ONE_COLUMN)
        if sf.files[0] eq 'GO_BACK' then goto,goback1 else if sf.files[0] eq 'EXIT' then return,-1
        nbehav_var=sf.count
        behav_str=sf.files

        goback1a:
        am=automatch(files,t4files,gs.subject_id,filename[0],gs.nbehav_pts,gs.behav_data,grptag,gs.column_labels,nglm_per_subject, $
            /DONTSELECT)
        if am.msg eq 'EXIT' then return,-1 else if am.msg eq 'GO_BACK' then goto,goback1a0
        slkn=sf.count
        nrun=am.nfiles
        files=am.files
        slkdata=am.behav_data[*,sf.index]

        if n_elements(filename) gt 1 then begin
            sf2=select_files(gs2.column_labels,BASE_TITLE=gr.file[1],TITLE='Please select variables',/GO_BACK,/EXIT, $
                MIN_NUM_TO_SELECT=-1,/NO_ALL,/ALIGN_LEFT,/ONE_COLUMN)
            if sf2.files[0] eq 'GO_BACK' then goto,goback1 else if sf2.files[0] eq 'EXIT' then return,-1
            nbehav_var=sf.count+sf2.count
            behav_str=[sf.files,sf2.files]

            am2=automatch(am.files,am.t4_files,gs2.subject_id,filename[1],gs2.nbehav_pts,gs2.behav_data,grptag,gs2.column_labels, $
                nglm_per_subject,/DONTSELECT)
            if am2.msg eq 'EXIT' then return,-1 else if am2.msg eq 'GO_BACK' then goto,goback1a0
            slkn=sf.count+sf2.count

            ;nrun=am2.nfiles
            ;files=am2.files
            ;START180501
            index=-1
            index2=-1
            for i=0,am2.nfiles-1 do begin
                idx=where(strmatch(am.subject_id_select,am2.subject_id_select[i]) eq 1,cnt)
                if cnt eq 1 then begin
                    index=[index,idx]
                    index2=[index2,i]
                endif else if cnt gt 1 then begin
                    scrap=get_button(['go back','exit'],TITLE='Error label found twice: '+am2.subject_id_select[i], $
                        BASE_TITLE=filename[0])
                    if scrap eq 0 then goto,goback1 else return,-1 
                endif
            endfor 
            if n_elements(index) ne n_elements(index2) or n_elements(index) eq 1 then begin
                scrap=get_button(['go back','exit'],TITLE='n_elements(index)='+n_elements(index)+string(10B)+'n_elements(index2)=' $
                    +n_elements(index2)+string(10B)+'Both must be equal and greater than 1.',BASE_TITLE='Error')
                if scrap eq 0 then goto,goback1 else return,-1 
            endif
            slk=am.behav_data[*,sf.index]
            slk=slk[index[1:*],*]
            slk2=am2.behav_data[*,sf2.index]
            slk2=slk2[index2[1:*],*]
            slkdata=[[slk],[slk2]]

            files=am.files[index[1:*]] 
            nrun=n_elements(index[1:*]) 

            print,'size(slkdata)=',size(slkdata)
            print,'nrun=',nrun
        endif

        idx0=-1
        cnt0=intarr(slkn)
        for i=0,slkn-1 do begin
            idx=where(slkdata[*,i] eq 'NA',cnt)

            ;print,'i=',i,' cnt=',cnt
            ;print,'idx=',idx

            if cnt ne 0 then begin
                idx0=[idx0,idx]
                cnt0[i]=cnt
            endif
        endfor
        if total(cnt0) ne 0 then begin
            slklistnomiss=make_array(nrun,/INTEGER,VALUE=1)
            idx0=idx0[1:*]

            ;print,'here0 idx0=',idx0

            j=0
            for i=0,slkn-1 do begin
                if cnt0[i] gt 0 then begin
                    slklistnomiss[idx0[j:j+cnt0[i]-1]]=0
                    j=j+cnt0[i]
                endif
            endfor
            idx=where(slklistnomiss eq 1,cnt)
            if cnt ne 0 then begin
                nrun=cnt   
                files=files[idx]
                slkdata=slkdata[idx,*]
            endif
            undefine,slklistnomiss,idx0,cnt0,idx,cnt
        endif
        undefine,am,sf,gr
        if n_elements(filename) gt 1 then undefine,am2,sf2

;STARTHERE

    endelse
endif

if nbehav_var eq 0 then begin 

    goback2:
    factor_names = get_str(2,['mapped to 1','mapped to -1'],['cor','inc'],WIDTH=50,TITLE='Please enter names.',/GO_BACK)
    if factor_names[0] eq 'GO_BACK' then goto,goback1
    factor_names = fix_script_name(factor_names,/SPACE_ONLY)

    goback3:
    factor_str = factor_names[0]+' and '+factor_names[1]
    nlevels = get_str(1,factor_str,'1',WIDTH=50,TITLE='Please enter the number of levels.',/GO_BACK)
    if nlevels[0] eq 'GO_BACK' then goto,goback2
    nlevels = fix(nlevels[0])
    ntreatments = 2*nlevels
    if nlevels gt 1 then begin
        if nlevels eq 2 then $ 
            level_names = ['val','inv'] $
        else begin
            level_names = strarr(nlevels)
            for j=0,nlevels[i]-1 do level_names[j] = 'level' + strtrim(j+1,2)
        endelse
        labels_level_names = strarr(nlevels)
        for j=0,nlevels-1 do labels_level_names[j] = factor_str + '.' + 'level' + strtrim(j+1,2)
        ;print,'level_names=',level_names
        ;print,'labels_level_names=',labels_level_names
        level_names = get_str(nlevels,labels_level_names,level_names,WIDTH=40,TITLE='Please enter names.',/GO_BACK)
        if level_names[0] eq 'GO_BACK' then goto,goback3
        treatment_str = strarr(ntreatments)
        k = 0
        for i=0,1 do begin
            for j=0,nlevels-1 do begin
                treatment_str[k] = factor_names[i] + '.' + level_names[j] 
                k = k + 1
            endfor
        endfor
    endif else $
        treatment_str = factor_names
    goback3a:
    widget_control,/HOURGLASS
    rtn_ic = identify_conditions_new(sng.num_levels,*sng.factor_labels,ntreatments,treatment_str,0,1, $
        'Multiple selections are permitted.',TOP_TITLE='Please select a condition for each treatment.')
    if rtn_ic.special eq 'GO_BACK' then goto,goback3
    sumrows = rtn_ic.sumrows
    index_conditions = rtn_ic.index_conditions
    ;print,'sumrows=',sumrows
    ;print,'index_conditions=',index_conditions
    nc = total(sumrows)

    rtn_gi = get_index(ntreatments,nlevels,index_conditions,sumrows,nc,sng)
    index = rtn_gi.index
    boldframes = rtn_gi.boldframes
    index_which_level = rtn_gi.index_which_level
    boldconditions = rtn_gi.boldconditions
    index_whichc = rtn_gi.index_whichc
    ic = rtn_gi.ic
    ic_by_levels = rtn_gi.ic_by_levels
    nic_by_levels = rtn_gi.nic_by_levels
    
    rtn_frames_ss = frames_ss(sng,nrun,tdim_sum)
    trialframes = rtn_frames_ss.trialframes[index]
    conditions_max = rtn_frames_ss.conditions_max
    
    tp_max = max(conditions_max[ic])
    goback4:
    scraparr = strarr(tp_max,nlevels)
    min_lengths_by_levels = intarr(nlevels)
    max_lengths_by_levels = intarr(nlevels)
    for i=0,nlevels-1 do begin
        min_lengths_by_levels[i] = min(conditions_max[ic_by_levels[i,0:nic_by_levels[i]-1]],MAX=scrap)
        max_lengths_by_levels[i] = scrap
        scraparr[0:min_lengths_by_levels[i]-1,i] = strtrim(indgen(min_lengths_by_levels[i])+1,2)
    endfor
    ;print,'min_lengths_by_levels=',min_lengths_by_levels
    ;print,'max_lengths_by_levels=',max_lengths_by_levels

    levelstr = make_array(nlevels,/STRING,VALUE='') 
    k = 0
    for j=0,nlevels-1 do begin
        levelstr[j] = treatment_str[k]
        k = k + 1
    endfor
    for j=0,nlevels-1 do begin
        levelstr[j] = levelstr[j] + ' and ' + treatment_str[k]
        k = k + 1
    endfor

    goback3b:
    if lccc eq 0 then begin
        bt = get_button(['enter text.','press buttons.','GO BACK'],TITLE='I would like to',BASE_TITLE='Please select timepoints.')
        if bt eq 2 then goto,goback3a
    endif else $
        bt = 1
    if bt eq 0 then begin
        factor_labels = *sng.factor_labels
        scrap = get_str(nc,factor_labels[ic_by_levels],make_array(nc,/STRING,VALUE='0'),WIDTH=50, $
            TITLE='Please enter the timepoints for each condition.', $
            LABEL='First timepoint is 1. Order counts. Zero is 0.  ex. 1-5,0',/GO_BACK)
        if scrap[0] eq 'GO_BACK' then goto,goback3b
        index_timec = make_array(nc,max(max_lengths_by_levels),max(max_lengths_by_levels),/INT,VALUE=-2)
        nframesc = intarr(nc)
        sumrows_timec = intarr(nc,max(max_lengths_by_levels))
        for i=0,nc-1 do begin
            segments = strsplit(scrap[i],'[ '+string(9B)+',]',/REGEX,/EXTRACT)
            nseg = n_elements(segments)
            start = 0
            for seg=0,nseg-1 do begin
                limits = long(strsplit(segments[seg],'-',/EXTRACT)) - 1
                if n_elements(limits) eq 1 then limits = [limits[0],limits[0]]
                subtract = limits[1] - limits[0]
                last = start + subtract
                index_timec[i,start:last,0] = indgen(subtract+1) + limits[0]
                start = last + 1
            endfor
            ;print,'index_timec[i,*,0]=',index_timec[i,*,0]
            index = where(index_timec[i,*,0] ge 0,count)
            nframesc[i] = count
            sumrows_timec[i,0:nframesc[i]-1] = 1
    
            index = where(index_timec[i,0:nframesc[i]-1,0] eq -1,count)
            if count ne 0 then begin
                stat = dialog_message('Not currently set up to do this. Tell McAvoy. Abort!',/ERROR)
                return,-1
            endif
        endfor
        print,'index_timec=',index_timec
        print,'sumrows_timec=',sumrows_timec
        print,'nframesc=',nframesc
    endif else begin
        if lccc eq 0 then begin
            goback100:
            widget_control,/HOURGLASS
            rtn = get_choice_list_many(levelstr,scraparr,TITLE='How many timepoints?',/LABEL_TOP,/GO_BACK, $
                LENGTHS=min_lengths_by_levels)
            if rtn.special eq 'GO_BACK' then goto,goback3b
            scraplist = rtn.list
            nframes = scraplist + 1
        endif
        sumrows_time = intarr(nlevels,max(nframes))
        index_time = intarr(nlevels,max(nframes),max(min_lengths_by_levels))
        for i=0,nlevels-1 do begin
            scrap = indgen(min_lengths_by_levels[i])+1
            widget_control,/HOURGLASS
            rtn_ic = identify_conditions_new(min_lengths_by_levels[i],string(scrap),nframes[i],'timepoint '+strtrim(scrap,2),0,0, $
                +'Please select a frame for each time point. Multiple selections are summed.',TOP_TITLE=levelstr[i],/TIME_FRAMES, $
                /SET_DIAGONAL)
            if rtn_ic.special eq 'GO_BACK' then goto,goback100
            index_time[i,0:nframes[i]-1,0:min_lengths_by_levels[i]-1] = rtn_ic.index_conditions
            sumrows_time[i,0:nframes[i]-1] = rtn_ic.sumrows
        endfor
        print,'index_time=',index_time
        print,'sumrows_time=',sumrows_time
    endelse
    names_tc_str = ' -tc_names "'+factor_names[0]+'" "'+factor_names[1]+'"'
    str = ' -tc_cond'
    scrap = strtrim(index_conditions,2)
    for i=0,ntreatments-1 do begin
        str = str + ' ' + scrap[i,0]
        for j=1,sumrows[i]-1 do str = str + ',' + scrap[i,j]
    endfor
    index_tc_str = str
endif
goback4a:
lc_t4 = !FALSE

;START170615
;space = fi.space[data_idx]

if lccc eq 1 then $
    analysis_type = 1 $
else begin 
    analysis_type = get_button(['voxel by voxel','specfic regions'],TITLE='Please select analysis type.')
    if analysis_type eq 1 then begin
        goback = 0
        goback4a1:
        rtn = analysis_type1(fi,dsp,wd,help,space,goback)
        if rtn.goback eq -1 then return,-1 else if rtn.goback eq 1 then goto,goback4a
        region_str = rtn.region_str
        roi_str = rtn.roi_str
        lc_t4 = rtn.lc_t4
        wfiles = rtn.wfiles
    endif

    ;if space ne !SPACE_111 and space ne !SPACE_222 and space ne !SPACE_333 then begin
    ;START170615
    if space[0] eq !SPACE_DATA then begin

        rtn=select_space()
        space=rtn.space
        atlas=rtn.atlas
        atlas_str=rtn.atlas_str
        lc_t4 = !TRUE
    endif

    ;if analysis_type eq 0 then mask_str = ' -mask ' + get_mask(space,fi)
    ;START170615
    ;if analysis_type eq 0 then mask_str = ' -mask ' + get_mask(space[0],fi)
    ;START170712
    if analysis_type eq 0 then begin

        ;mask=get_mask(space[0],fi,mask)
        ;START170714
        filter='*.nii'
        mask=get_mask(space[0],fi,mask,filter)

        if mask eq 'GO_BACK' then goto,goback4a
        mask_str=' -mask "'+mask +'"'
        compress_str=' -compress "'+mask+'"'
    endif


    if lc_t4 eq !TRUE then begin
        goback4b:
        get_dialog_pickfile,'*_anat_ave_to_711-2B_t4',directory,'Please select transformation matrix.',t4_file
        if t4_file eq 'GOBACK' then goto,goback4a else if t4_file eq 'EXIT' then return,-1
        t4_str = ' -xform_file ' + t4_file
    endif
    goback10:

    ;KEEP 
    ;all_or_ROC = get_button(['I want all of it.','Area under ROC only.','Logistic regression only.','GO BACK'], $
    ;    TITLE='What type of analysis?',BASE_TITLE='Please select.')
    ;if all_or_ROC eq 3 then goto,goback4a
    ;START170714
    all_or_ROC=2

    if all_or_ROC eq 0 then $
        ROCarea_str = ' -ROCarea' $
    else if all_or_ROC eq 1 then begin
        get_dialog_pickfile,'*.4dfp.img',directory,'Please select logistic regression weights file.',logregwts_file
        if logregwts_file eq 'GOBACK' then goto,goback4a else if logregwts_file eq 'EXIT' then return,-1
        logregwts_str = ' -logregwts_file ' + logregwts_file
    endif


    ;scrap = get_button(['yes','no','go back'],BASE_TITLE='Please select.', $
    ;    TITLE='Evaluate the statistical significance of the contribution of each independent variable.')
    ;if scrap eq 2 then goto,goback10
    ;if scrap eq 0 then reducedmodelsstr = ' -reducedmodels'
    ;START110906
    if nbehav_var eq 0 or nbehav_var gt 1 then begin
        scrap = get_button(['yes','no','go back'],BASE_TITLE='Please select.', $
            TITLE='Evaluate the statistical significance of the contribution of each independent variable.')
        if scrap eq 2 then goto,goback10
        if scrap eq 0 then reducedmodelsstr = ' -reducedmodels'
    endif


endelse
ct = ' -constant'
goback5:
if lccc eq 0 then begin
    if nbehav_var eq 0 then begin
        scrap = 'bold frames file'
        scraplabels = 'boldframes.dat'
    endif else begin
        scrap = 'driver file'

        ;rtn_gr = get_root(filename,'.slk')
        ;scraplabels = rtn_gr.file + '.dat'
        ;START180502
        rtn_gr = get_root(filename[0],'.slk')
        scraplabels = rtn_gr.file[0] + '.dat'

    endelse
    scrap = [scrap,'logistic regression script']

    ;scraplabels = [scraplabels,'fidl_logreg_ss.csh']
    ;START180306
    scraplabels = [scraplabels,'fidl_logreg.csh']

    if analysis_type eq 1 then begin
        scrap = [scrap,'output']

        ;scraplabels = [scraplabels,'fidl_logreg_ss.txt']
        ;START180306
        scraplabels = [scraplabels,'fidl_logreg.txt']

    endif else begin
        scrap = [scrap,'output root']
        if all_or_ROC eq 0 or all_or_ROC eq 2 then begin
            scraplabels = [scraplabels,'logreg']
        endif else begin
            rtn_gr1 = get_root(fi.names[data_idx],'.conc')
            rtn_gr2 = get_root(logregwts_file,'.4dfp.img')
            scraplabels = [scraplabels,rtn_gr1.file+'_'+rtn_gr2.file]
        endelse
    endelse
    names = get_str(n_elements(scrap),scrap,scraplabels,WIDTH=50,TITLE='Please enter names.',/GO_BACK,/FRONT)
    if names[0] eq 'GO_BACK' then goto,goback4a
    j = 0
    bf_str = names[j]
    j = j + 1
    csh = fix_script_name(names[j])
    j = j + 1
    if analysis_type eq 1 then begin
        anova_output_str = ' -regional_name "' + names[j] + '"'
        j = j + 1
    endif else begin
        lrs_name = ' -root "' + names[j] + '"'
    endelse
endif
if nbehav_var eq 0 then begin
    widget_control,/HOURGLASS
    if bt eq 0 then begin
        rtn_bf=print_boldframes(nframesc,sumrows_timec,index_timec,trialframes,boldframes,bf_str,index_whichc,/LOGREG)
    endif else begin
        rtn_bf=print_boldframes(nframes,sumrows_time,index_time,trialframes,boldframes,bf_str,index_which_level,/LOGREG)
    endelse
    boldconditions_index = rtn_bf.index
    nboldconditions_index = rtn_bf.count
    boldconditions = strcompress(boldconditions[boldconditions_index])
    str = 'set FRAMES_COND = (-frames_cond'
    for m=0,nboldconditions_index-1 do str = str + boldconditions[m]
    str = str + ')'
    boldconditions_str = str
    frames_cond_str = ' $FRAMES_COND'
endif

if lccc eq 0 then begin

    ;action = get_button(['execute','return','GO BACK'],TITLE='Please select action.'+string(10B)+string(10B)+'To run on linux, ' $
    ;    +'please select return. Login to a linux machine and enter the script on the command line.',BASE_TITLE=csh)
    ;START170615
    action = get_button(['execute','return','go back'],TITLE='Please select',BASE_TITLE=csh)

    if action eq 2 then goto,goback5
endif
goose='/fidl_bolds $TC_FILES $FRAMES_OF_INTEREST'+region_str+roi_str+compress_str+scratchdir_str
fidl_bolds_csh=scratchdir+'fidl_bolds.csh'
openw,lu,fidl_bolds_csh,/GET_LUN
printf,lu,FORMAT='("#!/bin/csh")'
printf,lu,FORMAT='("unlimit")'
printf,lu,''
print_files_to_csh,lu,nrun,files,'TC_FILES','tc_files'
print_files_to_csh,lu,nrun,trim(indgen(nrun)+1),'FRAMES_OF_INTEREST','frames_of_interest'
;printf,lu,''
printf,lu,!BINEXECUTE+goose+' -names_only'
close,lu
free_lun,lu
spawn,'chmod +x '+fidl_bolds_csh
widget_control,/HOURGLASS
spawn,fidl_bolds_csh,result
if n_elements(result) ne nrun then begin 
    scrap=get_button('exit',TITLE='nrun='+trim(nrun)+' n_elements(result)='+trim(n_elements(result)),BASE_TITLE='Error')
    return,-1 
endif
result=strsplit(result,/EXTRACT)
for i=0,nrun-1 do begin
    scrap=result[i]
    result[i]=scrap[n_elements(scrap)-1]
endfor
if nbehav_var gt 0 then begin
    openw,lu,bf_str,/GET_LUN
    printf,lu,'subject'+string(9B)+strjoin(behav_str,string(9B),/SINGLE)
    for j=0,nrun-1 do printf,lu,result[j]+string(9B)+strjoin(slkdata[j,*],string(9B),/SINGLE)
    close,lu
    free_lun,lu
endif


openw,lu,csh,/GET_LUN
top_of_script,lu

;print_files_to_csh,lu_csh,nrun,files,'TC_FILES','tc_files'
;START170714
print_files_to_csh,lu,nrun,files,'TC_FILES','tc_files',/NO_NEWLINE
print_files_to_csh,lu,nrun,trim(indgen(nrun)+1),'FRAMES_OF_INTEREST','frames_of_interest',/NO_NEWLINE
printf,lu,'nice +19 $BIN'+goose
printf,lu,''

if wfiles[0] ne '' then begin
    print_files_to_csh,lu,n_elements(wfiles),wfiles,'WEIGHT_FILES','weight_files'
    wfiles = ' $WEIGHT_FILES'
endif

;printf,lu_csh,boldconditions_str
;printf,lu_csh,''
;if nbehav_var eq 0 then $
;    bf_str = ' -frames "' + directory + '/' + bf_str + '"' $
;else $
;    bf_str = ' -driver "' + directory + '/' + bf_str + '"'
;START170714
if nbehav_var eq 0 then begin
    printf,lu,boldconditions_str
    printf,lu,''
    bf_str = ' -frames "' + directory + '/' + bf_str + '"'
endif else $
    bf_str = ' -driver "' + directory + '/' + bf_str + '"'


;printf,lu_csh,'nice +19 $BIN/fidl_logreg_ss $TC_FILES'+frames_cond_str+wfiles+names_tc_str+index_tc_str $
;    +region_str+roi_str+t4_str+anova_output_str+mask_str+bf_str+lrs_name+ct+filename_str+ROCarea_str $
;    +' -conc_file "'+fi.names[data_idx]+'"'+reducedmodelsstr+' -clean_up'
;START170615
;printf,lu,'nice +19 $BIN/fidl_logreg_ss $TC_FILES'+frames_cond_str+wfiles+names_tc_str+index_tc_str $
;    +region_str+roi_str+t4_str+anova_output_str+mask_str+bf_str+lrs_name+ct+filename_str+ROCarea_str $
;    +reducedmodelsstr+' -clean_up'
;START170714
;printf,lu,'nice +19 $BIN/fidl_logreg_ss $TC_FILES'+frames_cond_str+wfiles+names_tc_str+index_tc_str $
;    +region_str+roi_str+t4_str+anova_output_str+mask_str+bf_str+lrs_name+ct+filename_str+ROCarea_str $
;    +reducedmodelsstr+scratchdir_str+' -clean_up'
;START180112
printf,lu,'nice +19 $BIN/fidl_logreg $TC_FILES'+frames_cond_str+wfiles+names_tc_str+index_tc_str $
    +region_str+roi_str+t4_str+anova_output_str+mask_str+bf_str+lrs_name+ct+filename_str+ROCarea_str $
    +reducedmodelsstr+scratchdir_str+' -lambda 0.01 -clean_up'

if glmlist ne '' then begin
    printf,lu,''
    printf,lu,'# ',glmlist

    ;START180430
    for i=0,nfilename-1 do printf,lu,'# ',filename[i] 

    printf,lu,''
endif

if lccc eq 1 then $
    return,lu $
else begin 
    close,lu
    free_lun,lu
    spawn,'chmod +x '+csh
    widget_control,/HOURGLASS
    if action eq 0 then begin
        spawn,'csh '+csh+' > '+csh+'.log &'
        stat=dialog_message(csh+string(10B)+string(10B)+'Has been executued.',/INFORMATION)
    endif
endelse
print,'DONE'
end
