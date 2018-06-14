;Copyright 1/13/16 Washington University.  All Rights Reserved.
;fidl_anova4.pro  $Revision: 1.21 $
pro fidl_anova4,fi,dsp,wd,glm,help,stc,pref,FTEST_SS=ftest_ss,LMERR=lmerr
if not keyword_set(LMERR) then begin 

    ;START160505
    ;title='This program sets up the driving file for fidl_anova, which can be run from ' $
    ;+'a unix prompt. The program is a recoded version of Gary Perlman'+string(39B)+'s multifactor analysis of variance ' $
    ;+'program (Copyright 1980) that he developed at the University of California, San Diego ' $
    ;+'and at the Wang Institute. The algorithm is taken from Geoffrey Keppel'+string(39B)+'s (1973)' $
    ;+' Design and Analysis: A Researcher'+string(39B)+'s Handbook.'+string([10B,10B])+'If you choose to analyze timecourses, ' $
    ;+'then each frame of the HRF is entered as a level of the factor time. If you choose to analyze magnitudes, then the HRF ' $
    ;+'is cross correlated with the appropriate model to yield a magnitude.'

    ;START170308
    lmerr=0

endif else begin

    ;title='This program writes a script that calls lmer and anova from the lmerTest package.'+string(10B) $
    ;+'Imaging data from glms, imgs or niis are the dependent variables.'+string(10B) $
    ;+'The main effects and interactions along with variates chosen from a slk are the independent variables.'+string(10B) $
    ;+'Significance is determined with the Satterthwaite approximation for the degrees of freedom.'+string(10B) $
    ;+'Use parallels the ANOVA with the set up of factor levels followed by the selection of variables from a slk.'+string(10B) $
    ;+'You must run on a machine that has R with lmerTest installed.'+string(10B) $
    ;+"Fidl's ANOVA is currently run WITHOUT covariate regression."
    ;START161215
    title='This program writes a script that calls lmer and anova from the lmerTest package.'+string(10B) $
    +'Imaging data from glms, imgs or niis are the dependent variables.'+string(10B) $
    +'The main effects and interactions along with variates chosen from a slk are the independent variables.'+string(10B) $
    +'Significance is determined with the Satterthwaite approximation for the degrees of freedom.'+string(10B) $
    +'Use parallels the ANOVA with the set up of factor levels followed by the selection of variables from a slk.'+string(10B) $
    +'You must run on a machine that has R with lmerTest installed.'+string(10B) $
    +"Fidl's ANOVA is currently run WITHOUT covariate regression."+string(10B) $
    +'Subjects with missing imaging data are ok.'

    scrap = get_button(['Ok','Exit'],BASE_TITLE='Linear Mixed Effects: lmer and anova from lmerTest (R version)',TITLE=title, $
        /ALIGN_LEFT)
    if scrap eq 1 then return
    if help.enable eq !TRUE then widget_control,help.id,SET_VALUE=title
endelse
cols_or_rows_str = ''
mode_str = ''
statview_str = ''
compress_str = ''
region_or_uncompress_str = ''
anova_threshold_str = ''
gauss_str = ''
lcmag = ''
single_region_str = ''
factor_names = ''
correction_str = ''
delay_str = ''
fwhm = 0.
region = 1
num_roi = 1
offset_spawn = 0
offline_online = 1
region_is_a_factor_online_num_roi = 1
nframes = 1
lcdelays = 'no' 
clean_up_str = ''
hemisphere = 1
LorR = strarr(hemisphere)
LorR[0] = ''
space = '                                                                       '
hem_num_roi = 1
hem_region_compound_names = strarr(hem_num_roi)
hem_region_compound_names[0] = ''
anova_output_str = ''
nlists = 1
glm_list_str = ''
Nimage = 0
Nimage_name_str = ''
threshold_extent_str = ''
monte_carlo_fwhm = 0.
analysis_type = 0
box_str = ''
Nimage_mask_str = ''
nt4s = 0 
magnorm_str = ''
region_file = ''
datatype=0
load=0
GIGAstr=''
unscaled_str = ''
tc_type = ''
print_unscaled_mag_str = ''
print_cov_matrix_str = ''
normstr = ''
specialstr = ''
cellmeansstr = ''
nblists = 1
t4files = ''
bf_str = ''
glm_list_str_single = ''
files_are_glms = 0
superspecial = ''
hipassstr = ''
atlas_str = ''
period=1
nglm_per_subject = -1
compute_avg_zstat_csh_str = ''
lcregnamestr = 0
atlas_str_anova = ''
mask=''
vol=0
filecount=0
morethanone=0
regionflag=0
regionflagstr=''
one_file_str=''
nreg=0
indbasestr = ''
t4ssstr=''
lcstruct=0
t4select=''
slkn=0
lmeoutstr=''
slkfile=''
mode_str_lmer=''
spawn,'whoami',whoami
whoamistr = 'Hey '+whoami[0]+'!'
get_directory,directory,offset_spawn
if directory eq 'EXIT' then return
print,'directory=',directory
print,'offset_spawn=',offset_spawn
scratchdir = fidl_scratchdir(/NOPATH)
if scratchdir eq 'ERROR' then return
print,'scratchdir = ',scratchdir
scratchdir_str = ' -scratchdir ' + scratchdir
goback0:
if not keyword_set(FTEST_SS) then begin
    ftest_ss = 0
    datatype = get_button(['BOLD','PET','Structural','slk','Exit'],TITLE='What type of data?')
    if datatype eq 4 then return
endif else begin

    ;lizard = get_button(['Continue','Exit'],BASE_TITLE='Single subject F test',TITLE='To model session effects, label the event ' $
    ;    +'types uniquely for each session.')
    ;START180413
    lizard = get_button(['Continue','Exit'],BASE_TITLE='Single subject F test',TITLE='To model session effects, label the event ' $
        +'types uniquely for each session.'+string(10B)+'Can do multiple subjects.')

    if lizard eq 1 then return
endelse
if datatype eq 2 then begin
    datatype=0
    time=0
    lcstruct=1
endif
if datatype eq 3 then begin 
    time = 1 
    if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='Behavioral variables are assumed to be stored in a ' $
        +'spreadsheet exported in the *.slk format. The spreadsheet must have the data organized in columns with the first ' $
        +'row in each column containing a label.'
    rtn_slk=get_slk(help)
    if rtn_slk.msg eq 'GO_BACK' then goto,goback0 else if rtn_slk.msg eq 'EXIT' then return
endif else begin
    if ftest_ss eq 0 then begin 
        goback0a:
        if lcstruct eq 0 then begin
            time = get_button(['timecourses (ie effects)','magnitudes (ie contrasts)','go back'], $
                BASE_TITLE='Which do you wish to analyze?', $
                TITLE='If you have asssumed a shape for the HRF in the GLM then always select timecourses.')
            if time eq 2 then goto,goback0
        endif
        if time eq 1 then begin 
            spider = get_button(['yes','no','go back'],BASE_TITLE='Magnitude normalization', $
                TITLE='Contrasts are normalized to have a magnitude of 1.'+string(10B)+'If contrasts are averages, hit no.' $
                +string(10B)+'If the number of condtions in the contrast varies from subject to subject, then yes.')
            if spider eq 2 then goto,goback0a
            if spider eq 0 then magnorm_str = ' -magnorm'
        endif
        if datatype eq 1 then begin

            ;START180413 moved down below so unscaled_str is set in only one place
            ;unscaled_str = ' -unscaled'

            if time eq 0 then begin
                tc_type = ' -print_unscaled_tc -dont_print_scaled_tc'
            endif else $
                print_unscaled_mag_str = ' -print_unscaled_mag'
        endif
    endif else begin 
        time = 0
    endelse
endelse
ic_time = time
goback2:
rtn_within = get_factor_levels(help,time,0,rtn_within,rtn_slk,DATATYPE=datatype)
if rtn_within.nfactors eq -2 then return else if rtn_within.nfactors eq -1 then goto,goback0
goback3:
addtitle=''
rtn_between = get_factor_levels(help,time,1,rtn_between,dummy_rtn_slk,addtitle,NFACTORSIS0=ftest_ss)
if rtn_between.nfactors eq -1 then goto,goback2 else if rtn_between.nfactors eq -2 then return
avgstat_output_str = strarr(rtn_between.treatments[0])
rtn_between = get_treatment_str(rtn_between)
rtn_within=get_treatment_str(rtn_within)
goback4:
if datatype eq 3 then begin
    widget_control,/HOURGLASS
    if rtn_between.nfactors gt 0 then title='Please assign subjects to groups.' else title='Please selelct subjects.'
    rtn_sub = identify_conditions_new(rtn_slk.nbehav_pts,rtn_slk.subject_id,rtn_between.treatments[0],rtn_between.treatment_str, $
        0,0,title)
    if rtn_sub.special eq 'GO_BACK' then goto,goback3
    nsubject_per_group = rtn_sub.sumrows
endif else begin
    if rtn_between.nfactors gt 0 then groupnames=rtn_between.treatment_str else groupnames=0
    if rtn_within.nfactors gt 0 then levelnames=rtn_within.treatment_str else levelnames=0
    goback5:
    gg=get_glms(pref,fi,help,GET_THIS='GLMs/4dfps/*nii/*nii.gz',GROUPNAMES=groupnames,LEVELNAMES=levelnames)
    if gg.msg eq 'EXIT' then $
        return $
    else if gg.msg ne 'OK' then begin
        if ftest_ss eq 0 then goto,goback3 else goto,goback2
    endif
    load = gg.load
    total_nsubjects = gg.total_nsubjects
    if gg.glmfiles[0] ne '' then begin
        files_per_group = gg.glmfiles
        glm_space = gg.glm_space
        lc_glm=0
        if strmid(gg.glmfiles[0],strlen(gg.glmfiles[0])-3) eq 'slk' then lc_glm=2

        ;START170308
        if total_nsubjects eq 1 and lmerr eq 0 then ftest_ss=1

    endif else begin
        files_per_group = gg.imgselect
        glm_space = gg.bold_space
        lc_glm=1

        ;print,'here0 gg.imgselect'
        ;print,gg.imgselect
        ;print,'n_elements(gg.imgselect)=',n_elements(gg.imgselect)
        ;print,'here0 gg.imgselecti'
        ;print,gg.imgselecti
        ;print,'here0 n_elements(gg.imgselecti)=',n_elements(gg.imgselecti)
        ;print,'here0 gg.nglm=',gg.nglm,' total(gg.nglm)=',total(gg.nglm)
        ;print,'here0 gg.total_nsubjects=',gg.total_nsubjects

    endelse
    nfiles_per_group = gg.nglm
    nsubject_per_group = gg.nglm
    if gg.t4select[0] ne '' then t4select = gg.t4select
    subject_names = files_per_group
    morethanone = gg.morethanone


    dimstr = ' -dimxyz '+trim(gg.ifh[0].matrix_size_1)+' '+trim(gg.ifh[0].matrix_size_2)+' '+trim(gg.ifh[0].matrix_size_3)

    ;if ftest_ss ne 0 and morethanone eq 0 then unscaled_str = ' -unscaled'
    ;START180413
    unscaled_str='' ;reset to handle goback
    if ftest_ss eq 0 then begin
        if datatype eq 1 then unscaled_str = ' -unscaled' 
    endif else begin
        if morethanone eq 0 then unscaled_str = ' -unscaled'
    endelse

endelse
goback6:
if n_elements(files_per_group) gt 0 then begin
    scrap = strmid(files_per_group[0],strlen(files_per_group[0])-3)
    if scrap eq 'glm' then $
        files_are_glms = 1 $
    else if scrap eq 'onc' then $
        files_are_glms = 2
    flag_fwhm_zero = 0
    goback7:
    imosteff = 0
endif
if files_are_glms eq 1 then begin 
    get_labels_struct = replicate({Get_labels_struct},total_nsubjects)
    widget_control,/HOURGLASS
    for i=0,total_nsubjects-1 do begin
        get_labels_struct[i] = get_labels_from_glm(fi,dsp,wd,glm,help,pref,ifh,files_per_group[i],CONTRASTS_ONLY=time, $
            START_INDEX_GLM=i)
        if get_labels_struct[i].ifh.glm_rev gt -17 then begin
            stat=dialog_message('Revision number for '+strmid(files_per_group[i],1)+' too old. Update file by resaving.',/ERROR)
            return
        endif
    endfor
    nreg = get_labels_struct[*].ifh.nreg
    if total(abs(nreg[0]-nreg)) ne 0 then begin
        print,'nreg=',nreg
        stat=dialog_message('GLMs contain different numbers of regions',/ERROR)
        return
    endif
    if nreg[0] gt 0 then region_or_uncompress_str = ' -regions ' + strjoin(*get_labels_struct[0].ifh.region_names,' ',/SINGLE)
    if morethanone eq 0 then begin
        subject_get_labels_struct = get_labels_struct
        nsubject = total_nsubjects
        nglm_per_subject = make_array(nsubject,/INTEGER,VALUE=1)
    endif else begin
        go_morethanone:
        if morethanone ne 0 and ftest_ss ne 0 and total_nsubjects eq 2 then begin
            nglm_per_subject = 2 
            total_nsubjects = 2 
            nsubject = 1 
            nsubject_per_group = 1 
        endif else begin
            rtn_get_morethanone=get_morethanone(files_per_group,nfiles_per_group,rtn_between.treatments[0], $
                rtn_between.treatment_str,t4select)
            if rtn_get_morethanone.msg eq 'GO_BACK' then goto,goback5 else if rtn_get_morethanone.msg eq 'ERROR' then return
            nglm_per_subject = rtn_get_morethanone.nglm_per_subject
            total_nsubjects = rtn_get_morethanone.total_nsubjects
            nsubject = rtn_get_morethanone.nsubject
            files_per_group = rtn_get_morethanone.files_per_group
            nfiles_per_group = rtn_get_morethanone.nfiles_per_group
            nsubject_per_group = rtn_get_morethanone.nsubject_per_group
            t4select = rtn_get_morethanone.t4select
            subject_names = rtn_get_morethanone.subject_names
            if keyword_set(FTEST_SS) then begin
                ss_dir=strarr(nsubject)
                for i=0,nsubject-1 do begin
                    ss_dir[i]=strmid(subject_names[i],0,strlen(subject_names[i])-1)
                    cmd='mkdir '+ss_dir[i] 
                    print,cmd
                    spawn,cmd
                endfor
            endif
        endelse

        ;print,'nglm_per_subject=',nglm_per_subject
        ;print,'total_nsubjects=',total_nsubjects
        ;print,'files_per_group=',files_per_group
        ;print,'nfiles_per_group=',nfiles_per_group
        ;print,'nsubject_per_group=',nsubject_per_group
        ;print,'subject_names=',subject_names
        ;print,'t4select=',t4select
        ;print,'nsubject=',nsubject

        subject_filenames = strarr(nsubject)
        subject_get_labels_struct = replicate({Get_labels_struct},nsubject)
        j = 0
        for i=0,nsubject-1 do begin
            subject_get_labels_struct[i] = get_labels_from_glm(fi,dsp,wd,glm,help,pref,ifh, $
                files_per_group[j:j+nglm_per_subject[i]-1],CONTRASTS_ONLY=time,START_INDEX_GLM=j)
            subject_filenames[i] = strjoin(files_per_group[j:j+nglm_per_subject[i]-1],string(10B),/SINGLE)
            j = j + nglm_per_subject[i]
        endfor
    endelse
    goback61:
    length_label=max(subject_get_labels_struct[*].nscraplabels,max_glm_index)
    labels=*subject_get_labels_struct[max_glm_index].scraplabels
    if morethanone eq 0 then begin
        if rtn_within.treatments[0] gt length_label then begin
            scrapstr='Too many conditions have been specified.'+string(10B)+'length_label = ' $
                +strtrim(length_label,2) +'  rtn_within.treatments[0] = '+strtrim(rtn_within.treatments[0],2)
            print,scrapstr
            scrap = get_button(['yes','no','go back'],TITLE='Do you have more than one GLM per subject?')
            if scrap eq 2 then $
                goto,goback5 $
            else if scrap eq 1 then begin
                scrap = get_button('ok',TITLE=scrapstr+string(10B)+'Abort!')
                return
            endif else begin
                morethanone=1
                goto,go_morethanone
            endelse
        endif
    endif
    for i=0,nsubject-1 do begin
        if i ne max_glm_index then begin
            scrap=*subject_get_labels_struct[i].scraplabels
            for j=0,subject_get_labels_struct[i].nscraplabels-1 do begin
                if total(strcmp(labels,scrap[j])) eq 0 then labels = [labels,scrap[j]]
            endfor
        endif
    endfor
    length_label_all = n_elements(labels)

    ;index = where((labels ne 'Trend') and (labels ne 'Baseline'),count)
    ;if count ne 0 then labels = strtrim(labels[index],2)
    ;START150109
    excludeHz=1
    index = where(strmatch(labels,"*Hz*") eq 1,count)
    if count ne 0 then begin
        scrap = get_button(['yes','no','go back'],TITLE='Is this a spectral analysis?'+string(10B) $
            +"Hit 'no' if don't know what this is.")
        if scrap eq 0 then excludeHz=0 else if scrap eq 2 then goto,goback6
    endif
    exclude_effects,labels,/EXCLUDEBASELINE,EXCLUDEHZ=excludeHz

    length_label = n_elements(labels)
    widget_control,/HOURGLASS
    if nt4s gt 0 then begin
        scrap = get_str(total_nsubjects,files_per_group,t4select,TITLE='Please check t4s.',/ONE_COLUMN,/BELOW,/GO_BACK)
        if scrap[0] eq 'GO_BACK' then begin
            if load eq 1 then goto,goback5 else goto,goback5
        endif
        t4select = scrap
    endif

    ;START110715
    if n_elements(subject_filenames) ne 0 then begin
        ;KEEP THESE PRINTS
        print,'max_glm_index=',max_glm_index
        print,subject_filenames[max_glm_index]
        print,'labels=',labels
    endif
endif else if datatype ne 3 then begin 

    ;print,'here1 gg.nglm=',gg.nglm

    ;if rtn_within.nfactors gt 0 then nsubject=total_nsubjects/rtn_within.treatments[0] else nsubject=total_nsubjects
    ;START161215
    if rtn_within.nfactors gt 0 then begin
        if total(abs(gg.nglm-gg.nglm[0])) eq 0 then nsubject=total_nsubjects/rtn_within.treatments[0] else begin

            ;rtn_get_morethanone=get_morethanone(files_per_group,nfiles_per_group,rtn_between.treatments[0], $
            ;    rtn_between.treatment_str,t4select)
            ;START161215
            rtn_get_morethanone=get_morethanone(gg.imgselect,gg.total_nsubjects,rtn_between.treatments[0], $
                rtn_between.treatment_str,gg.t4select,dum1,dum2,dum3,dum4,dum5,gg.imgselecti)

            if rtn_get_morethanone.msg eq 'GO_BACK' then goto,goback4 else if rtn_get_morethanone.msg eq 'ERROR' then return
            nglm_per_subject = rtn_get_morethanone.nglm_per_subject
            total_nsubjects = rtn_get_morethanone.total_nsubjects
            nsubject = rtn_get_morethanone.nsubject
            files_per_group = rtn_get_morethanone.files_per_group
            nfiles_per_group = rtn_get_morethanone.nfiles_per_group
            nsubject_per_group = rtn_get_morethanone.nsubject_per_group
            subject_names = rtn_get_morethanone.subject_names
            t4select = rtn_get_morethanone.t4select

            ;START161215
            imgselecti=rtn_get_morethanone.imgselecti

            ;print,'here2 files_per_group'
            ;print,files_per_group
            ;print,'here2 imgselecti'
            ;print,imgselecti
            ;print,'here2 n_elements(imgselecti)=',n_elements(imgselecti)
            ;print,'here-1 nglm_per_subject'
            ;print,nglm_per_subject
            ;print,'n_elements(nglm_per_subject)=',n_elements(nglm_per_subject)

        endelse
    endif else nsubject=total_nsubjects



    ifh_dim4 = gg.ifh[*].matrix_size_4
    ifh_vol = long(gg.ifh[*].matrix_size_1)*long(gg.ifh[*].matrix_size_2)*long(gg.ifh[*].matrix_size_3)
    if total(ifh_vol-ifh_vol[0]) ne 0 then begin 
        stat=dialog_message('Not all images are the same size. Abort!',/ERROR)
        return
    endif
    vol=ifh_vol[0]
    if time lt 2 then begin
        if total(ifh_dim4 - ifh_dim4[0]) ne 0 then begin
            stat=dialog_message('Not all images have the same time dimension. Abort!',/ERROR)
            return
        endif
        if time eq 0 then begin
            period = ifh_dim4[0]
            length_label = 1
        endif else if time eq 1 then begin
            period = 1
            length_label = ifh_dim4[0]
            labels = strarr(length_label)
            for i=0,length_label-1 do labels[i] = 'timepoint ' + strtrim(i+1,2)
            ic_time = 0
        endif
    endif
endif
if datatype ne 3 then begin
    if total(glm_space - glm_space[0]) ne 0 then begin
        stat=dialog_message('Not all GLMs are in the same space. Abort!',/ERROR)
        return
    endif
    atlasspace = glm_space[0]
    clean_up_str = ' -clean_up'

    ;START150331
    ;boldtype = subject_get_labels_struct[0].ifh.glm_boldtype
    ;idx=where(subject_get_labels_struct[*].ifh.glm_boldtype ne boldtype,cnt)
    ;if cnt ne 0 then begin
    ;    scrap=get_button(['Exit'],TITLE='Not all glms are formed from the same type of bolds.' $
    ;        +string(10B)+string(10B)+'Expecting '+boldtype)
    ;    return
    ;endif 
    ;START150429
    if files_are_glms eq 1 then begin
        boldtype = subject_get_labels_struct[0].ifh.glm_boldtype
        idx=where(subject_get_labels_struct[*].ifh.glm_boldtype ne boldtype,cnt)
        if cnt ne 0 then begin
            scrap=get_button(['Exit'],TITLE='Not all glms are formed from the same type of bolds.' $
                +string(10B)+string(10B)+'Expecting '+boldtype)
            return
        endif
    endif else boldtype='img'


    if time lt 2 then begin

        ;if glm_space[0] eq !SPACE_DATA and load eq 1 and nreg[0] eq 0 then begin
        ;START150331
        if glm_space[0] eq !SPACE_DATA and load eq 1 and nreg[0] eq 0 and boldtype eq 'img' then begin

            if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='If your files are in data space, you will need to ' $
                +'provide an anat_ave_to_711-2*_t4.'
            gf = get_files(TITLE='Please enter t4 filter.',FILTER='*anat_ave_to_*_t4',EXTRALABEL='Enter NONE if none.')
            if gf.msg eq 'GO_BACK' then goto,goback6 else if gf.msg eq 'EXIT' then return
            t4files = gf.files
            nt4s = n_elements(t4files)
            nlists = 2
            if t4files[0] ne 'NONE' then begin
                t4select = match_files(files_per_group,t4files,TITLE='Please select t4 file for ',/GO_BACK)
                if t4select[0] eq 'GO_BACK' then goto,goback7
            endif
        endif
        if files_are_glms eq 1 then begin
            effect_length_ts = make_array(nsubject,length_label_all,/INT,VALUE=1)
            if time eq 0 then begin
                for i=0,nsubject-1 do begin
                    effect_length_ts[i,0:subject_get_labels_struct[i].nscraplabels-1] = *subject_get_labels_struct[i].effect_length
               endfor
            endif
        endif else $
            effect_length_ts = make_array(total_nsubjects,length_label,/INT,VALUE=period)
    endif
    if glm_space[0] eq !SPACE_DATA and nreg[0] eq 0 then begin
        if t4select[0] ne '' then begin
            rtn=select_space()
            atlasspace=rtn.space
            atlas=rtn.atlas
            atlas_str=rtn.atlas_str
        endif
    endif
endif
goback8:

;START161219
lcnglm_per_subject=0

;START180131
idx=where(nglm_per_subject gt 1,nnglm_per_subjectgt1)
undefine,idx

rtn_within = get_treatment_str(rtn_within)
if datatype eq 3 then begin
    dummy = 'Multiple conditions are averaged.'
    top_title = 'Please select a condition for each treatment.'
    widget_control,/HOURGLASS
    rtn_ic = identify_conditions_new(rtn_slk.ncolumn_labels,rtn_slk.column_labels,rtn_within.treatments[0], $
        rtn_within.treatment_str,0,rtn_within.nfactors,dummy,TOP_TITLE=top_title,/SET_DIAGONAL)
    if rtn_ic.special eq 'GO_BACK' then goto,goback3
    ;print,'rtn_ic.sumrows=',rtn_ic.sumrows
    ;print,'rtn_ic.index_conditions=',rtn_ic.index_conditions
    scrap=-1
    for i=0,rtn_between.treatments[0]-1 do scrap = [scrap,reform(rtn_sub.index_conditions[i,0:rtn_sub.sumrows[i]-1])]
    behav_data=rtn_slk.behav_data[scrap[1:*],*]
    files_per_group = rtn_slk.subject_id[scrap[1:*]]
    scrapsub = intarr(total(rtn_sub.sumrows))
    scrapdata = dblarr(total(rtn_sub.sumrows),rtn_within.treatments[0])
    for i=0,total(rtn_sub.sumrows)-1 do begin
        for j=0,rtn_within.treatments[0]-1 do begin
            scrap = behav_data[i,rtn_ic.index_conditions[j,0:rtn_ic.sumrows[j]-1]]
            index = where(scrap ne '',count)
            if count ne 0 then begin
                scrapdata[i,j] = scrap[index]/double(count)
                scrapsub[i] = scrapsub[i] + 1
            endif
        endfor
    endfor
    index = where(scrapsub eq rtn_within.treatments[0],total_nsubjects)
    behav_data_str = strtrim(scrapdata[index,*],2)
    files_per_group = files_per_group[index]
    use_these = make_array(total_nsubjects,/INT,VALUE=1)
    nfiles_per_group = intarr(rtn_between.treatments[0])
    j=0
    for i=0,rtn_between.treatments[0]-1 do begin
        index = where(scrapsub[j:j+rtn_sub.sumrows[i]-1] eq rtn_within.treatments[0],count)
        nfiles_per_group[i] = count
        j = j + rtn_sub.sumrows[i]
    endfor
endif else begin

    ;START161219
    ;START161216
    ;lcnglm_per_subject=0

    if time lt 2 then begin
        index_conditions = indgen(rtn_within.treatments[0])
        if files_are_glms eq 1 or time eq 1 then begin 
            if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='All within group factors are ' $
                +'crossed to generate a set of treatments. Following the above example where factor "cue" is at levels "valid" ' $
                +'and "invalid" and factor "visual_field" is at levels "right" and "left", yields four treatments: 1, cue.valid ' $
                +'visual_field.right; 2, cue.valid visual_field.left; 3, cue.invalid visual_field.right; and 4, cue.invalid ' $
                +'visual_field.left. Select the condition that corresponds to each of these treatments.'
            top_title = ''
            time_frames = 0
            if time eq 0 then begin
                if rtn_within.nfactors eq 0 then begin
                    top_title = 'Please select a condition for the simple main effect of time.'
                endif else begin
                    top_title = 'Please select a condition for each treatment.'
                endelse
                dummy = 'Multiple conditions are combined as a weighted mean by the inverse of their variance.'
                time_frames = 1
            endif else begin
                if rtn_within.nfactors eq 0 then begin
                    if rtn_between.treatments[0] eq 1 then begin
                        stat=dialog_message('If you are not going to use region as a factor then you should try a t-test.', $
                            /INFORMATION,/CANCEL)
                        if stat eq 'Cancel' then return
                        dummy = 'Please select a contrast.'
                    endif else begin
                        dummy = 'Please select a contrast for the simple main effect of group.'
                    endelse
                endif else begin
                    if lc_glm eq 1 then begin 
                        top_title = 'Please select a timepoint(s) for each treatment.'
                        dummy = 'Multiple selections are summed.' 
                        time_frames = 1
                    endif else $
                        dummy = 'Please select a contrast for each treatment.'
                endelse
            endelse
            if help.enable eq !TRUE then widget_control,help.id,SET_VALUE=dummy

            print,'length_label=',length_label

            ;if length_label lt 50 then begin
            ;START180501
            if length_label lt 100 then begin

                ;pointandclick:
                ;widget_control,/HOURGLASS
                ;rtn_ic = identify_conditions_new(length_label,labels,rtn_within.treatments[0],rtn_within.treatment_str,ic_time, $
                ;    rtn_within.nfactors,dummy,TOP_TITLE=top_title,TIME_FRAMES=time_frames,/SET_DIAGONAL,/SET_UNSET)
                ;if rtn_ic.special eq 'GO_BACK' then goto,goback5
                ;sumrows = rtn_ic.sumrows
                ;index_conditions = rtn_ic.index_conditions
                ;special = rtn_ic.special
                ;START180501
                pointandclick:
                if length_label lt 200 then begin
                    widget_control,/HOURGLASS
                    rtn_ic = identify_conditions_new(length_label,labels,rtn_within.treatments[0],rtn_within.treatment_str,ic_time, $
                        rtn_within.nfactors,dummy,TOP_TITLE=top_title,TIME_FRAMES=time_frames,/SET_DIAGONAL,/SET_UNSET)
                    if rtn_ic.special eq 'GO_BACK' then goto,goback5
                    sumrows = rtn_ic.sumrows
                    index_conditions = rtn_ic.index_conditions
                    special = rtn_ic.special
                     
                    ;print,'sumrows'
                    ;print,sumrows
                    ;print,'index_conditions'
                    ;print,index_conditions
                    ;print,'size(sumrows)=',size(sumrows)
                    ;print,'size(index_conditions)=',size(index_conditions)

                endif else begin 
                    sumrows=intarr(rtn_within.treatments[0])
                    index_conditions=intarr(rtn_within.treatments[0],length_label)
                    for i=0,rtn_within.treatments[0]-1 do begin
                        widget_control,/HOURGLASS
                        rtn_ic = identify_conditions_new(length_label,labels,1,rtn_within.treatment_str[i],ic_time, $
                            rtn_within.nfactors,dummy,TOP_TITLE=top_title,TIME_FRAMES=time_frames,/SET_UNSET,/NOPRESET)
                        if rtn_ic.special eq 'GO_BACK' then goto,goback5
                        sumrows[i]=rtn_ic.sumrows
                        index_conditions[i,*]=rtn_ic.index_conditions
                        undefine,rtn_ic 
                    endfor

                    print,'sumrows=',sumrows
                    ;print,'index_conditions'
                    ;print,index_conditions
                    ;print,'size(sumrows)=',size(sumrows)
                    ;print,'size(index_conditions)=',size(index_conditions)

                endelse
            endif else begin
                rtn_gsgs = get_sic_get_str(length_label,labels,rtn_within.treatments[0])
                if rtn_gsgs.msg eq 'GO_BACK' then goto,goback5 else if rtn_gsgs.msg eq 'SPECIAL' then goto,pointandclick
                sumrows = rtn_gsgs.sumrows
                index_conditions = rtn_gsgs.index_conditions
            endelse
            if strpos(labels[index_conditions[0]],'Hz') ne -1 then superspecial = 'Hz'
            if files_are_glms eq 0 then begin
                conditions_time = index_conditions + 1
                sum_contrast_for_treatment = make_array(total_nsubjects,rtn_within.treatments[0],1,/INT,VALUE=1)
            endif else begin

                ;scrap = cft_and_scft(nsubject,rtn_within.treatments[0],length_label,sumrows,index_conditions,labels, $
                ;    subject_get_labels_struct,time,subject_names,rtn_within.treatment_str,rtn_within.nfactors,dummy)
                ;START161020
                scrap = cft_and_scft(nsubject,rtn_within.treatments[0],length_label,sumrows,index_conditions,labels, $
                    subject_get_labels_struct,time,subject_names,rtn_within.treatment_str,rtn_within.nfactors,dummy,NOTPRESENT=lmerr)


                if scrap.sum_contrast_for_treatment[0,0,0] eq -2 then $
                    return $

                ;else if scrap.sum_contrast_for_treatment[0,0,0] eq -1 then $
                ;    goto,goback8
                ;START161020
                else if scrap.sum_contrast_for_treatment[0,0,0] eq -1 then begin 
                    undefine,scrap
                    goto,goback8
                endif

                sum_contrast_for_treatment = scrap.sum_contrast_for_treatment

                ;START161020
                undefine,scrap

                ;print,'sum_contrast_for_treatment'
                ;print,sum_contrast_for_treatment
                ;print,'size(sum_contrast_for_treatment)=',size(sum_contrast_for_treatment)

            endelse
        endif else begin
            if rtn_within.nfactors gt 0 then begin

                ;KEEP THIS
                ;if n_elements(t4select) gt 0 then spider=1 else spider=0
                ;if time eq 0 then dummy='Please select timecourses.' else dummy='Please select magnitudes.'
                ;filesselect = strarr(total_nsubjects,rtn_within.treatments[0])
                ;m = 0
                ;pp=0
                ;for p=0,rtn_between.treatments[0]-1 do begin
                ;    files = files_per_group[pp:pp+nfiles_per_group[p]-1]
                ;    nfiles = n_elements(files)
                ;    nfiles_save = nfiles
                ;    nfiles_per_group[p]=nfiles_per_group[p]/rtn_within.treatments[0]
                ;    for k=1,nfiles_per_group[p] do begin
                ;        title = 'subject '+strtrim(k,2)+string(10B)
                ;        if spider eq 1 then title=title+t4select[m]+string(10B)
                ;        title = title + dummy        
                ;        if rtn_between.treatments[0] gt 1 then title = rtn_between.treatment_str[p]+',' + title
                ;        scrap = strarr(1+rtn_within.treatments[0],nfiles)
                ;        for i=0,nfiles-1 do begin
                ;            scrap[0,i] = 'ignore'
                ;            scrap[1:rtn_within.treatments[0],i] = rtn_within.treatment_str
                ;        endfor
                ;        repeat begin
                ;            rtn = get_choice_list_many(files,scrap,TITLE=title,/LABEL_TOP,/GO_BACK)
                ;            if rtn.special eq 'GO_BACK' then goto,goback8
                ;            scraplist = rtn.list
                ;            index = where(scraplist gt 0,count)
                ;            if count ne rtn_within.treatments[0] then begin
                ;                stat=dialog_message('You need to select '+strcompress(string(rtn_within.treatments[0]), $
                ;                    /REMOVE_ALL)+' magnitudes.',/ERROR)
                ;            endif else begin
                ;                scraplist = scraplist[index]
                ;                if total(scraplist[sort(scraplist)]-(indgen(rtn_within.treatments[0])+1)) ne 0 then begin
                ;                    stat=dialog_message('You have assigned the same magnitude to more than one treatment. ' $
                ;                        +'Please try again.',/ERROR)
                ;                    count = 0
                ;                endif else begin
                ;                    filesselect[m,0:rtn_within.treatments[0]-1] = files[index]
                ;                endelse
                ;            endelse
                ;        endrep until count eq rtn_within.treatments[0]
                ;        for i=rtn_within.treatments[0]-1,0,-1 do begin
                ;            for j = index[i],nfiles-2 do files[j] = files[j+1]
                ;            nfiles = nfiles - 1
                ;            if nfiles gt 0 then files = files[0:nfiles-1]
                ;        endfor
                ;        m = m + 1
                ;    endfor
                ;    ;pp=pp+nfiles_per_group[p]
                ;    pp=pp+nfiles_save
                ;endfor

                if total_nsubjects eq nsubject then begin
                    if n_elements(t4select) gt 0 then spider=1 else spider=0
                    if time eq 0 then dummy='Please select timecourses.' else dummy='Please select magnitudes.'
                    filesselect = strarr(total_nsubjects,rtn_within.treatments[0])
                    m=0
                    pp=0
                    special=''
                    for p=0,rtn_between.treatments[0]-1 do begin
                        files = files_per_group[pp:pp+nfiles_per_group[p]-1]
                        nfiles = n_elements(files)
                        nfiles_save = nfiles
                        nfiles_per_group[p]=nfiles_per_group[p]/rtn_within.treatments[0]
                        for k=1,nfiles_per_group[p] do begin
                            kk = (k-1)*rtn_within.treatments[0]
                            files1 = files[kk:kk+rtn_within.treatments[0]-1]
                            nfiles1 = rtn_within.treatments[0]
                            if special eq '' then begin
                                title = 'subject '+strtrim(k,2)+string(10B)
                                if spider eq 1 then title=title+t4select[m]+string(10B)
                                if rtn_between.treatments[0] gt 1 then title = rtn_between.treatment_str[p]+',' + title
                                rtn=identify_conditions_new(1+rtn_within.treatments[0],['ignore',rtn_within.treatment_str],nfiles1, $
                                    files1,time,0,title,-1,/SET_DIAGONAL,/SAMEFORALL,/COLUMN_LAYOUT,TOP_TITLE=dummy)
                                idx = rtn.index_conditions
                                if rtn.special eq 'GO_BACK' then goto,goback8
                                special = rtn.special
                            endif
                            filesselect[m,0:rtn_within.treatments[0]-1] = files1[idx]
                            m = m + 1
                        endfor
                        pp=pp+nfiles_save
                    endfor
                    nsubject_per_group = nsubject_per_group/rtn_within.treatments[0]
                    nsubject = total(nsubject_per_group)

                ;endif else begin
                ;    filesselect = reform(files_per_group,nsubject,rtn_within.treatments[0])
                ;endelse
                ;START161216
                endif else if rtn_within.nfactors eq 0 then begin 
                    filesselect = reform(files_per_group,nsubject,rtn_within.treatments[0])
                endif else if total(abs(gg.nglm-gg.nglm[0])) eq 0 then begin
                    filesselect = reform(files_per_group,nsubject,rtn_within.treatments[0])
                endif else begin
                    filesselect=make_array(nsubject,rtn_within.treatments[0],/STRING,VALUE='')
                    k=0
                    for i=0,nsubject-1 do begin
                        filesselect[i,imgselecti[k:k+nglm_per_subject[i]-1]]=files_per_group[k:k+nglm_per_subject[i]-1] 
                        k=k+nglm_per_subject[i]
                    endfor 
                    lcnglm_per_subject=1
                endelse


            endif
        endelse
    endif
    lcemptycells=0
    use_these0=intarr(nsubject)
    if n_elements(sum_contrast_for_treatment) ne 0 then begin
        for i=0,nsubject-1 do begin
            scrap = where(sum_contrast_for_treatment[i,*,0] eq 0,count)
            if count eq 0 then use_these0[i]=1 else lcemptycells=1
        endfor
    endif else begin
        for i=0,nsubject-1 do begin
            scrap=where(filesselect[i,*] eq '',count)
            if count eq 0 then use_these0[i]=1 else lcemptycells=1
        endfor
    endelse
    if not keyword_set(LMERR) then begin
        use_these=use_these0
        iuse_these = where(use_these eq 1,nuse_these,COMPLEMENT=index)
        if nuse_these lt nsubject then begin
            scrap=get_button(['ok','go back','exit'],TITLE='The following subjects will be excluded from the analysis because of ' $
                +'missing cells.'+string(10B)+string(10B)+strjoin(subject_names[index],string(10B),/SINGLE))
            if scrap eq 2 then return else if scrap eq 1 then goto,goback8

            ;START180504
            if keyword_set(FTEST_SS) then begin
                cmd='rm -r '+strjoin(ss_dir[index],' ',/SINGLE)
                print,cmd 
                spawn,cmd
            endif

        endif




;            if keyword_set(FTEST_SS) then begin
;                ss_dir=strarr(nsubject)
;                for i=0,nsubject-1 do begin
;                    ss_dir[i]=strmid(subject_names[i],0,strlen(subject_names[i])-1)
;                    cmd='mkdir '+ss_dir[i]
;                    print,cmd
;                    spawn,cmd
;                endfor
;            endif





    endif else begin
        use_these = make_array(nsubject,/INT,VALUE=1)
        iuse_these=indgen(nsubject)
        nuse_these=nsubject
    endelse

    goback10:
    if superspecial eq 'Hz' then begin
        ;do nothing
    endif else begin

        ;if files_are_glms eq 1 then period = effect_length_ts[imosteff,sum_contrast_for_treatment[0,0,0]-1]
        ;START170613
        if files_are_glms eq 1 then begin
            if sum_contrast_for_treatment[0,0,0] gt 0 then period=effect_length_ts[imosteff,sum_contrast_for_treatment[0,0,0]-1]
        endif

        if time eq 0 then begin
            gtperiod = 0
            if files_are_glms eq 1 then begin
                effect_length_max = intarr(rtn_within.treatments[0])
                for m=0,nsubject-1 do begin
                    for i=0,rtn_within.treatments[0]-1 do begin
                        for k=0,length_label-1 do begin
                            if sum_contrast_for_treatment[m,i,k] gt 0 then begin
                                effect_length_max[i] = effect_length_max[i] > effect_length_ts[m,sum_contrast_for_treatment[m,i,k]-1] 
                            endif
                        endfor
                    endfor
                endfor
                index = where(effect_length_max gt period,gtperiod)
                max_effect_length_max = max(effect_length_max)
            endif
            goback8a:
            if datatype eq 0 and period gt 1 then begin
                nframes = get_button([string(indgen(period)+1),'I have a contrast file.','go back'], $
                    TITLE='Time is a factor at how many levels?') + 1
            endif else begin
                goback8a1:
                nframes = 1
                if rtn_within.treatments[0] eq 1 and rtn_between.treatments[0] eq 1 then begin
                    spider = get_button(['exit','go back'], $
                        TITLE='This design is a single comparison. You need to run a T test instead.',BASE_TITLE=whoamistr)
                    if spider eq 1 then goto,goback2 else return
                endif
            endelse
            if nframes eq period+2 then begin
                if files_are_glms eq 1 or time eq 1 then $
                    goto,goback8 $
                else $
                    goto,goback7
            endif else if nframes eq period+1 then begin
                nframes = 1
                if files_are_glms eq 1 then begin
                    stat=dialog_message('This path has not been instantiated. See McAvoy if you need this.',/ERROR)
                    goto,goback8a
                endif else begin
                    goback8b:
                    c = read_contrast_file(constr,wstr,float_cdata)
                    if c.name eq 'GO_BACK' then goto,goback8a
                    widget_control,/HOURGLASS
                    rtn_ic = identify_conditions_new(c.NF,c.hdr,rtn_within.treatments[0],rtn_within.treatment_str,1,0, $
                        'Please select a contrast for each treatment.')
                    if rtn_ic.special eq 'GO_BACK' then goto,goback8b
                    scrap = ''
                    dummy = ''
                    for i=0,rtn_within.treatments[0]-1 do begin
                        scrap = scrap + ' ' + constr[rtn_ic.index_conditions[i]]
                        dummy = dummy + ' ' + wstr[rtn_ic.index_conditions[i]]
                    endfor
                    bf_str = ' -frames_of_interest' + scrap + ' -tc_weights' + dummy
                endelse
            endif else begin
                if nframes eq 1 then scrap = 0 else scrap = 1
                if nframes ne period then begin
                    if files_are_glms eq 1 or rtn_within.nfactors eq 0 then begin
                        rtn_ic = identify_conditions_new(max_effect_length_max,trim(indgen(max_effect_length_max)+1),nframes, $
                            'timepoint '+strtrim(indgen(max_effect_length_max)+1,2),time,scrap, $
                            'Multiple selections are weighted by the inverse of their variance.', $
                            TOP_TITLE='Please select an estimate for each timepoint.',/TIME_FRAMES,/SET_DIAGONAL)

                        if rtn_ic.special eq 'GO_BACK' then goto,goback10
                        sumrows_time = rtn_ic.sumrows
                        conditions_time = rtn_ic.index_conditions + 1
                    endif else begin
                        if nframes le 40 then begin

                            pasteloop = nframes*rtn_within.treatments[0]
                            scrapstr = strarr(pasteloop)
                            dummy = strtrim(indgen(nframes)+1,2)
                            k = 0
                            for i=0,rtn_within.treatments[0]-1 do begin
                                for j=0,nframes-1 do begin
                                    scrapstr[k] = rtn_within.treatment_str[i] + dummy[j]
                                    k = k + 1
                                endfor
                            endfor
                            ;START170609
                            ;if nframes eq 1 then $
                            ;    scrapstr=rtn_within.treatment_str $
                            ;else begin
                            ;    pasteloop = nframes*rtn_within.treatments[0]
                            ;    scrapstr = strarr(pasteloop)
                            ;    dummy = strtrim(indgen(nframes)+1,2)
                            ;    k = 0
                            ;    for i=0,rtn_within.treatments[0]-1 do begin
                            ;        for j=0,nframes-1 do begin
                            ;            scrapstr[k] = rtn_within.treatment_str[i] + dummy[j]
                            ;            k = k + 1
                            ;        endfor
                            ;    endfor
                            ;endelse

                            widget_control,/HOURGLASS
                            rtn_ic = identify_conditions_new(period,string(indgen(period)+1),pasteloop,scrapstr,time,scrap, $
                                'Multiple selections are weighted by the inverse of their variance.', $
                                TOP_TITLE='Please select estimates for each treatment.',/TIME_FRAMES,SET_BUTTONS=pasteloop)
                            if rtn_ic.special eq 'GO_BACK' then goto,goback10
                            sumrows_time = rtn_ic.sumrows
                            conditions_time = rtn_ic.index_conditions + 1
                            if n_elements(sum_contrast_for_treatment) eq 0 then begin
                                sum_contrast_for_treatment = make_array(total_nsubjects,rtn_within.treatments[0],1,/INT,VALUE=1)
                                for i=1,rtn_within.treatments[0]-1 do conditions_time[i*nframes:i*nframes+nframes-1,*] $
                                    = conditions_time[i*nframes:i*nframes+nframes-1,*] + i*period
                            endif
                        endif else begin
                            range=trim(ceil(gg.ifh[0].matrix_size_4/2.)-nframes/2)+'-'+trim(ceil(gg.ifh[0].matrix_size_4/2.)+nframes/2)
                            str = get_str(1,'Frames to include (e.g. 10-20,64,592-605 numbered from 1)',range,TITLE='Which frames?', $
                                /BELOW,/GO_BACK)

                            if str[0] eq 'GO_BACK' then goto,goback8a
                            sumrows_time = intarr(period)
                            segments = strsplit(str[0],'[ '+string(9B)+',]',/REGEX,/EXTRACT)
                            nseg = n_elements(segments)
                            limits_all = -1L
                            for seg=0,nseg-1 do begin
                                omit_reference = 1
                                limits = long(strsplit(segments[seg],'-',/EXTRACT)) - 1
                                if n_elements(limits) eq 1 then begin
                                    limits = long(strsplit(segments[seg],/EXTRACT)) - 1
                                    limits = [limits[0],limits[0]]
                                endif
                                if limits[0] lt 0 then limits[0] = 0
                                if limits[1] gt period-1 then limits[1] = period-1
                                if limits[1] lt 0 then omit_reference = 0
                                str=strcompress(string(scrap,limits+1,FORMAT='(a," frames: ",i4," to ",i4)'))
                                print,str
                                limits_all = [limits_all,limits[0],limits[1]]
                                if omit_reference eq 1 then sumrows_time[limits[0]:limits[1]] = 1
                            endfor
                            limits_all = limits_all[1:*]
                            conditions_time = where(sumrows_time eq 1,count) + 1
                        endelse
                    endelse
                endif else begin
                    sumrows_time = make_array(period,/INT,VALUE=1)
                    conditions_time = [indgen(period)+1]
                    if gtperiod ne 0 then begin

                        ;pasteloop = nframes*rtn_within.treatments[0]
                        ;scrapstr = strarr(pasteloop)
                        ;dummy = strtrim(indgen(nframes)+1,2)
                        ;k = 0
                        ;for i=0,rtn_within.treatments[0]-1 do begin
                        ;    for j=0,nframes-1 do begin
                        ;        scrapstr[k] = rtn_within.treatment_str[i] + dummy[j]
                        ;        k = k + 1
                        ;    endfor
                        ;endfor
                        ;START170609
                        if nframes eq 1 then $
                            scrapstr=rtn_within.treatment_str $
                        else begin
                            pasteloop = nframes*rtn_within.treatments[0]
                            scrapstr = strarr(pasteloop)
                            dummy = strtrim(indgen(nframes)+1,2)
                            k = 0
                            for i=0,rtn_within.treatments[0]-1 do begin
                                for j=0,nframes-1 do begin
                                    scrapstr[k] = rtn_within.treatment_str[i] + dummy[j]
                                    k = k + 1
                                endfor
                            endfor
                        endelse

                        scraparr = strarr(max_effect_length_max+1,rtn_within.treatments[0])
                        for i=0,rtn_within.treatments[0]-1 do $
                            scraparr[0:effect_length_max[i],i] = [strtrim(indgen(effect_length_max[i])+1,2),'ALL']
                        widget_control,/HOURGLASS
                        rtn = get_choice_list_many(scrapstr,scraparr,TOP_TITLE='Please select estimates for each treatment.', $
                            TITLE='Multiple selections are weighted by the inverse of their variance.', $
                            /LABEL_TOP,/NONEXCLUSIVE,/GO_BACK,LENGTHS=effect_length_max+1,/SET_DIAGONAL)
                        if rtn.special eq 'GO_BACK' then goto,goback8
                        scraplist = rtn.list
                        conditions_time_treat = intarr(rtn_within.treatments[0],max_effect_length_max)
                        sumrows_time_treat = intarr(rtn_within.treatments[0])
                        for i=0,rtn_within.treatments[0]-1 do begin 
                            if scraplist[effect_length_max[i],i] eq 1 then scraplist[0:effect_length_max[i]-1,i] = 1
                            index = where(scraplist[0:effect_length_max[i]-1,i],count)
                            if count ne 0 then begin
                                conditions_time_treat[i,0:count-1] = index + 1
                                sumrows_time_treat[i] = count
                            endif
                        endfor
                        ;print,'conditions_time_treat'
                        ;print,conditions_time_treat
                        ;print,'sumrows_time_treat'
                        ;print,sumrows_time_treat
                        ;print,'sumrows_time'
                        ;print,sumrows_time
                        ;print,'conditions_time'
                        ;print,conditions_time
                    endif
                endelse
            endelse
        endif
    endelse
    if not keyword_set(FTEST_SS) then begin

        ;START161020
        lmernomissdir='' & lmermissdir='' & lmeranovadir=''

        if rtn_within.nfactors eq 0 then lizard='time' else lizard=strjoin(rtn_within.factor_names,', ',/SINGLE)
        if rtn_between.nfactors gt 0 then lizard=lizard+', '+strjoin(rtn_between.factor_names,', ',/SINGLE)
        nfactors = (rtn_within.nfactors>1) + rtn_between.nfactors
        if nfactors eq 2 then lizard=lizard+' and their interaction.' else if nfactors gt 2 then lizard=lizard $
            +' and their interactions.'
        lizard='Your model includes independent variables '+lizard
        goback11:
        scrap=0
        if gg.slkfile eq '' then begin
            if keyword_set(LMERR) then scrap='independent variables' else scrap='factors'
            scrap = get_button(['Yes, let me load a slk','No','Go back','Exit'],BASE_TITLE='Please select',/ALIGN_LEFT, $
                TITLE=lizard+string(10B)+'Do you want to model additional '+scrap+'?')
            if scrap eq 2 then goto,goback10 else if scrap eq 3 then return 
            goback11a:
            if scrap eq 0 then begin
                gs=get_slk(help,gg.slkfile,'NA')
                if gs.msg eq 'GO_BACK' then goto,goback11 else if gs.msg eq 'EXIT' then return
                slkdata=gs.behav_data
                slkcollab=gs.column_labels
                slkfile=gs.slkfile
            endif
        endif else begin
            slkdata=gg.slkdata
            slkcollab=gg.slkcollab
            slkid=gg.slkid
            slkfile=gg.slkfile
        endelse
        goback12:
        slkn=0
        if scrap eq 0 then begin
            title="Don't select any to not add more independent variables to the model."
            if gg.slkfile ne '' then title=lizard+string(10B)+title
            sf = select_files(slkcollab,TITLE=title,BASE_TITLE='Define additional independent variables',/GO_BACK,/EXIT, $
                MIN_NUM_TO_SELECT=-1,/NO_ALL,/ALIGN_LEFT)
            if sf.files[0] eq 'GO_BACK' then begin
                if gg.slkfile eq '' then goto,goback11a else goto,goback10
            endif else if sf.files[0] eq 'EXIT' then return
            if sf.count gt 0 then begin
                if gg.slkfile eq '' then begin
                    goback12a:
                    filei=intarr(n_elements(files_per_group))
                    scrapi=indgen(n_elements(files_per_group))
                    i=0
                    k=0
                    for j=0,nsubject-1 do begin
                        if use_these[j] eq 1 then begin
                            filei[i:i+nglm_per_subject[j]-1]=scrapi[k:k+nglm_per_subject[j]-1]
                            i=i+nglm_per_subject[j]
                        endif
                        k=k+nglm_per_subject[j]
                    endfor
                    filei=filei[0:i-1]
                    subi=where(use_these eq 1,nsubi)
                    undefine,scrapi
                    if rtn_between.nfactors gt 0 then begin
                        grptag=intarr(total_nsubjects)
                        j=0
                        for i=0,rtn_between.treatments[0]-1 do begin
                            grptag[j:j+nfiles_per_group[i]-1]=i
                            j=j+nfiles_per_group[i]
                        endfor
                        grptag=grptag[filei]
                    endif

                    ;print,'here1 files_per_group[filei]=',files_per_group[filei]
                    ;print,'here1 n_elements(files_per_group[filei])=',n_elements(files_per_group[filei])
                    ;print,'here1 nglm_per_subject[subi]=',nglm_per_subject[subi]
                    ;print,'here1 n_elements(nglm_per_subject[subi])=',n_elements(nglm_per_subject[subi])
                    ;print,'here1 gs.subject_id=',gs.subject_id
                    ;print,'here1 n_elements(gs.subject_id)=',n_elements(gs.subject_id)

                    am=automatch(files_per_group[filei],t4select[filei],gs.subject_id,gs.slkfile,gs.nbehav_pts,gs.behav_data,$
                        grptag,slkcollab,nglm_per_subject[subi],/DONTSELECT,/REUSE)
                    if am.msg eq 'EXIT' then return else if am.msg eq 'GO_BACK' then goto,goback12
                    if rtn_between.nfactors gt 0 then begin
                        j=0
                        for i=0,rtn_between.treatments[0]-1 do begin
                            idx=where(grptag eq i,cnt)
                            nfiles_per_group[i]=cnt
                        endfor
                        idx=where(nfiles_per_group eq 0,cnt)
                        if cnt ne 0 then begin
                            idx=get_button(['Go back','Exit'],BASE_TITLE='Big problem',TITLE='No files left for:' $
                                +strjoin(groupnames[idx],', ',/SINGLE))
                            if idx eq 0 then goto,goback12a else return
                        endif
                    endif
                    slkdata=am.behav_data[*,sf.index]
                    slkid=am.subject_id_select

                    ;print,'here2 slkdata=',slkdata
                    ;print,'here2 n_elements(slkdata)=',n_elements(slkdata)
                    ;print,'here2 slkid=',slkid
                    ;print,'here2 n_elements(slkid)=',n_elements(slkid)

                    if am.subjectinot[0] ne -1 then begin
                        print,'******************* HERE ********************'
                        use_these[subi[am.subjectinot]]=0
                    endif
                    if am.nglmpersubject[0] ne -1 then begin
                        slknglmpersubject=am.nglmpersubject
                        for i=0,n_elements(am.nglmpersubject)-1 do begin
                            if nglm_per_subject[subi[am.subjecti[i]]] ne slknglmpersubject[i] then begin
                                idx=get_button(['Go back','Exit'],BASE_TITLE='Problem',TITLE='nglm_per_subject['$
                                    +trim(subi[am.subjecti[i]])+']='+trim(nglm_per_subject[subi[am.subjecti[i]]])$
                                    +' slknglmpersubject['+trim(i)+']='+trim(slknglmpersubject[i])$
                                    +' Should be equal. Code needs to be added to handle this.')
                                if idx eq 0 then goto,goback12a else return
                            endif
                        endfor
                    endif
                    undefine,am
                endif else begin
                    slkdata=slkdata[*,sf.index]
                endelse 
                slkcollab=slkcollab[sf.index]
                slkn=sf.count
                undefine,sf
                if n_elements(slknglmpersubject) eq 0 then slknglmpersubject=make_array(nuse_these,/INTEGER,VALUE=1)
                slklistnomiss=make_array(total(slknglmpersubject),/INTEGER,VALUE=1)
                lmernomissdir='' & lmermissdir='' & lmeranovadir=''
                idx0=-1
                cnt0=intarr(slkn)
                for i=0,slkn-1 do begin
                    idx=where(slkdata[*,i] eq 'NA',cnt)
                    if cnt ne 0 then begin
                        idx0=[idx0,idx]
                        cnt0[i]=cnt
                    endif
                endfor
                if total(cnt0) eq 0 then begin
                    lmeranovadir='fidlanova'
                    lmernomissdir='fidllmerTest'
                    if keyword_set(LMERR) then cmd='mkdir '+lmeranovadir+' '+lmernomissdir else cmd='mkdir '+lmeranovadir
                    print,cmd
                    spawn,cmd
                endif else begin
                    idx0=idx0[1:*]
                    j=0
                    for i=0,slkn-1 do begin
                        if cnt0[i] gt 0 then begin
                            slklistnomiss[idx0[j:j+cnt0[i]-1]]=0
                            j=j+cnt0[i]
                        endif
                    endfor
                    if total(slklistnomiss) gt 0 then begin
                        lmeranovadir='fidlanovaNoMiss'
                        lmernomissdir='fidllmerTestNoMiss'
                        if keyword_set(LMERR) then cmd='mkdir '+lmeranovadir+' '+lmernomissdir else cmd='mkdir '+lmeranovadir
                        print,cmd
                        spawn,cmd
                    endif
                endelse
                undefine,idx0,cnt0,idx,cnt
            endif 
        endif
        if keyword_set(LMERR) and lcemptycells eq 1 then begin
            cmd='mkdir'
            if lmeranovadir eq '' then begin
                lmeranovadir='fidlanovaNoMiss'
                cmd=cmd+' '+lmeranovadir
            endif
            if lmernomissdir eq '' then begin
                lmernomissdir='fidllmerTestNoMiss'
                cmd=cmd+' '+lmernomissdir
            endif
            if lmermissdir eq '' then begin
                lmermissdir='fidllmerTestMiss'
                cmd=cmd+' '+lmermissdir
            endif
            if strlen(cmd) gt 5 then begin
                print,cmd
                spawn,cmd
            endif
        endif 
    endif
    lcgoback16=0
    goback15:
    if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='In constructing your glms, you may have already done a certain ' $
        +'amount of smoothing. If you have not smoothed or wish to do an additional amount of smoothing, you may do so. Estimates ' $
        +'are smoothed with a 3D gaussian kernel.'
    if vol eq 0 then $
        vol = long(get_labels_struct[0].ifh.glm_xdim)*long(get_labels_struct[0].ifh.glm_ydim)*long(get_labels_struct[0].ifh.glm_zdim)
    region_str = make_array(total_nsubjects,/STRING,VALUE='')
    roi_str = make_array(total_nsubjects,/STRING,VALUE='')
    if nreg[0] gt 0 then begin 
        analysis_type = 0
        region_or_uncompress_str = ' -regions ' + strjoin(*subject_get_labels_struct[0].ifh.region_names,' ',/SINGLE)
    endif else if atlasspace eq !UNKNOWN then begin
        if vol eq 1 then analysis_type = 1
    endif else if boldtype eq 'img' then begin
        goback16:
        lcgoback16=1
        compress_str = ''
        region_or_uncompress_str = ''
        Nimage_mask_str = ''
        analysis_type = get_button(['voxel by voxel','specfic regions','GO BACK'],TITLE='Please select analysis type.')
        if analysis_type eq 2 then begin
            if period gt 1 then goto,goback10 else goto,goback8
        endif else if analysis_type eq 0 then begin
            if files_are_glms eq 0 then scrap = 'images' else scrap = 'GLMs'
            monte_carlo_fwhm = 0.
            if files_are_glms eq 1 then monte_carlo_fwhm = trim(get_labels_struct[0].ifh.glm_fwhm,2)
            goback15a:
            if monte_carlo_fwhm eq 0. then begin 
                title = 'You may smooth your '+scrap+' if you wish.'
                label = 'Enter 0 for no smoothing.' 
                if ftest_ss eq 0 then dummy = '2' else dummy = '0'
            endif else begin
                title = 'You have smoothed '+monte_carlo_fwhm+' voxels. You may do some additional smoothing if you wish.'
                label = 'Enter 0 for no additional smoothing.'
                dummy = '0'
            endelse
            scrap = get_str(1,'FWHM in voxels ',dummy,/GO_BACK,TITLE=title,LABEL=label,WIDTH=50)
            if scrap[0] eq 'GO_BACK' then  $
                goto,goback16 $
            else if float(scrap[0]) gt 0.01 then $
                gauss_str = ' -gauss_smoth '+ strtrim(scrap[0],2)
            fwhm = double(scrap[0])
            monte_carlo_fwhm = round(sqrt(monte_carlo_fwhm^2+fwhm^2))
            mask = ''
            if files_are_glms eq 1 then begin
                if ptr_valid(subject_get_labels_struct[0].ifh.glm_mask_file) then mask=*subject_get_labels_struct[0].ifh.glm_mask_file
            endif
            if mask eq '' then begin
                mask = get_mask(atlasspace,fi)
                if mask eq 'GO_BACK' then goto,goback15a
            endif
            if atlasspace ne !SPACE_DATA then begin
                if mask eq '' then begin
                    if atlasspace eq !SPACE_111 then begin
                        atlas_str_anova = ' -atlas 111'
                        vol = !VOL_111
                    endif else if atlasspace eq !SPACE_222 then begin
                        atlas_str_anova = ' -atlas 222'
                        vol = !VOL_222
                    endif else if atlasspace eq !SPACE_333 then begin
                        atlas_str_anova = ' -atlas 333'
                        vol = !VOL_333
                    endif else if atlasspace eq !SPACE_MNI222 then begin
                        atlas_str_anova = ' -atlas MNI222'
                        vol = !VOL_MNI222 
                    endif
                endif else begin
                    if atlasspace eq !SPACE_111 then $
                        vol = !LENBRAIN_111 $
                    else if atlasspace eq !SPACE_222 then $
                        vol = !LENBRAIN_222 $
                    else if atlasspace eq !SPACE_333 then $
                        vol = !LENBRAIN_333 $
                    else if atlasspace eq !SPACE_MNI222 then $
                        vol = !LENBRAIN_MNI222
                endelse
            endif
            if mask ne '' then begin 
                compress_str = ' -compress ' + mask
                region_or_uncompress_str = ' -uncompress ' + mask
                Nimage_mask_str = ' -Nimage_mask ' + mask
            endif else $ 
                region_or_uncompress_str = dimstr 
        endif
    endif
    if analysis_type eq 1 then begin
        if atlasspace ne !UNKNOWN then begin
            goback16a:
            rtn_getreg = get_regions(fi,wd,dsp,help)
            if rtn_getreg.msg eq 'EXIT' then return else if rtn_getreg.msg eq 'GO_BACK' then goto,goback16
            region_names = rtn_getreg.region_names
            region_file = rtn_getreg.region_file
            region_space = rtn_getreg.space
            nregfiles = rtn_getreg.nregfiles
            if glm_space[0] ne !SPACE_DATA and glm_space[0] ne region_space then begin
                stat = dialog_message("GLMs and region file are not in the same space. Abort!",/ERROR)
                return
            endif
            if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='For regional analyses, statistics may be computed for ' $
                +'selected regions in a region file either with region as a factor or region not as a factor.'
            goback16b:
            rtn_reg = select_files(region_names,TITLE='Please select regions you wish to analyze.',/GO_BACK,MIN_NUM_TO_SELECT=1, $
                SPECIAL='Assign regions to subjects')
            if rtn_reg.files[0] eq 'GO_BACK' then goto,goback16a
            reg_by_filei = rtn_getreg.reg_by_filei[rtn_reg.index]
            ireg_by_file = rtn_getreg.ireg_by_file[rtn_reg.index]
            ;print,'rtn_reg.files=',rtn_reg.files
            ;print,'reg_by_filei=',reg_by_filei
            ;print,'ireg_by_file=',ireg_by_file
            if rtn_reg.special eq 0 then begin
                uniq_reg_by_filei = reg_by_filei[uniq(reg_by_filei,sort(reg_by_filei))]
                uniq_nreg_by_file = rtn_getreg.nreg_by_file[uniq_reg_by_filei] 
                uniq_regfile = rtn_getreg.region_file[uniq_reg_by_filei]
                ;print,'uniq_reg_by_filei=',uniq_reg_by_filei
                ;print,'uniq_nreg_by_file=',uniq_nreg_by_file
                ;print,'uniq_regfile=',uniq_regfile
                regnum = intarr(rtn_reg.count) 
                regnum[0] = ireg_by_file[0]
                scrap = reg_by_filei[0]
                j = 0
                for i=1,rtn_reg.count-1 do begin
                    if reg_by_filei[i] ne reg_by_filei[i-1] then begin
                        scrap = scrap + uniq_nreg_by_file[j]
                        j = j + 1
                    endif    
                    regnum[i] = ireg_by_file[i] + scrap
                endfor
                regnum = regnum + 1
            endif else begin
                superspecial = 'YES'
                nreg_by_file = rtn_getreg.nreg_by_file[reg_by_filei]
                regfile = rtn_getreg.region_file[reg_by_filei]
                goback16b1:
                scrap = get_str(1,'Search root','vc',TITLE='Please enter search root.',/GO_BACK,SPECIAL='Let me do this manually')
                if scrap[0] eq 'GO_BACK' then goto,goback16b
                searchroot=scrap[0]
                if scrap[0] eq 'SPECIAL' then begin
                    text=strjoin(strjoin([transpose(strtrim(indgen(total_nsubjects)+1,2)),transpose(files_per_group)],'   '), $
                        string(10B),/SINGLE)
                    rtn_dml = dialog_message_long('Key',text,/NO_MODAL,NLINES=total_nsubjects)
                    scrap = rtn_reg.count/total_nsubjects
                    scrap1 = intarr(scrap)
                    ord_labels = -1
                    for i=1,total_nsubjects do begin
                        scrap1[*]=i
                        ord_labels=[ord_labels,scrap1]
                    endfor
                    ord_labels=ord_labels[1:*]
                    nreg_per_subject=intarr(total_nsubjects)
                    title='Please assign subject numbers'
                    rtn = get_ordered_list(rtn_reg.files,ord_labels,TITLE=title)
                    uniq_order_sorted=uniq(rtn.order_sorted)
                    scrap=n_elements(uniq_order_sorted)
                    nreg_per_subject[0]=uniq_order_sorted[0]+1
                    for l=1,scrap-1 do nreg_per_subject[l]=uniq_order_sorted[l]-uniq_order_sorted[l-1]
                    iregglm = rtn.isorted
                    num_roi = nreg_per_subject[0]
                    hem_region_names = rtn.labels_sorted[0:nreg_per_subject[0]-1] 
                endif else begin
                    rtn_gr = get_root(files_per_group,'.glm')
                    glmroots = rtn_gr.file
                    rtn_match_vc=match_vc(1,[files_per_group,rtn_reg.files],total_nsubjects+rtn_reg.count,[glmroots,rtn_reg.files],$
                        SEARCHROOT=searchroot)
                    if rtn_match_vc.msg eq 'ERROR' then return else if rtn_match_vc.msg eq 'GO_BACK' then goto,goback16b1
                    scraplabel=''
                    scrap=''
                    k=0
                    for i=0,total_nsubjects-1 do begin
                        scraplabel=[scraplabel,rtn_match_vc.files[k]]
                        scrap=[scrap,strjoin(rtn_match_vc.files[k+1:k+rtn_match_vc.nfiles_per_subject[i]-1],', ',/SINGLE)]
                        k=k+rtn_match_vc.nfiles_per_subject[i]
                    endfor
                    scrap=get_str(total_nsubjects,scraplabel[1:*],scrap[1:*],TITLE='Please check regions.',LABEL='This listing is '$
                        +'not editable.',/ONE_COLUMN,/BELOW,/GO_BACK)
                    if scrap[0] eq 'GO_BACK' then goto,goback16b1
                    if total(rtn_match_vc.nfiles_per_subject-rtn_match_vc.nfiles_per_subject[0]) ne 0 then begin
                        ;print,'rtn_match_vc.nfiles_per_subject=',rtn_match_vc.nfiles_per_subject
                        scrap=get_button(['go back','exit'],TITLE='Not all subjects have the same number of regions.', $
                            BASE_TITLE='Error')
                        if scrap eq 0 then goto,goback16b1 else return
                    endif
                    iregglm=-1
                    k=0
                    for i=0,total_nsubjects-1 do begin
                        iregglm=[iregglm,rtn_match_vc.index[k+1:k+rtn_match_vc.nfiles_per_subject[i]-1]]
                        k=k+rtn_match_vc.nfiles_per_subject[i]
                    endfor
                    iregglm=iregglm[1:*]-total_nsubjects
                    ;print,'iregglm=',iregglm
                    nreg_per_subject = rtn_match_vc.nfiles_per_subject-1
                    num_roi = nreg_per_subject[0]
                    hem_region_names = rtn_match_vc.files[1:rtn_match_vc.nfiles_per_subject[0]-1]
                endelse 
            endelse
            if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='When region is chosen as a factor, it is at as many ' $
                +'levels as selected regions in the region file.'
            goback16c:
            region = get_button(['yes','no','GO BACK'],TITLE='Do you wish region to be a factor?')
            if region eq 2 then begin 
                if rtn_reg.special eq 0 then goto,goback16b else goto,goback16b1 
            endif else if region eq 0 then begin
                if rtn_reg.special eq 0 then scrap=rtn_reg.count else scrap=min(nreg_per_subject)
                if scrap ge 4 and scrap mod 2 eq 0 then begin
                    goback16d:
                    LorR[0] = ''
                    hemisphere = get_button(['yes','no','GO BACK'],TITLE='Do you wish to look at hemisphere differences?')
                    if hemisphere eq 2 then goto,goback16c
                endif
            endif
            if rtn_reg.special eq 0 then begin
                region_numbers_str = make_array(total_nsubjects,/STRING,VALUE=strjoin(strtrim(regnum,2),' ',/SINGLE))
                region_str = make_array(total_nsubjects,/STRING,VALUE=' -region_file '+strjoin(uniq_regfile,' ',/SINGLE))
                num_roi = rtn_reg.count
                hem_region_names = rtn_reg.files
            endif else begin
                goback16c1:
                if region eq 0 or hemisphere eq 0 then begin
                    hem_region_names = get_str(num_roi,hem_region_names,hem_region_names,TITLE='Please enter region roots for ' $
                        +'matching.',/ALIGN_RIGHT,/GO_BACK,WIDTH=50)
                    if hem_region_names[0] eq 'GO_BACK' then goto,goback16c
                endif
                region_numbers_str = strarr(total_nsubjects)
                region_str = strarr(total_nsubjects)
                k=0
                for i=0,total_nsubjects-1 do begin
                    scrapi=iregglm[k:k+nreg_per_subject[i]-1]
                    scrap=reg_by_filei[scrapi]
                    scrap1=0
                    scrap2=intarr(nreg_per_subject[i])
                    scrap2[0]=ireg_by_file[scrapi[0]]
                    scrap3=regfile[scrap[0]]
                    for j=1,nreg_per_subject[i]-1 do begin
                        if scrap[j] ne scrap[j-1] then begin
                            scrap1 = scrap1 + nreg_by_file[scrapi[j-1]]
                            scrap3 = [scrap3,regfile[scrap[j]]]
                        endif
                        scrap2[j] = ireg_by_file[scrapi[j]] + scrap1
                    endfor
                    region_numbers_str[i] = strjoin(strtrim(scrap2+1,2),' ',/SINGLE)
                    region_str[i] = strjoin(scrap3,' ',/SINGLE)
                    k=k+nreg_per_subject[i]
                endfor
                scrap = get_str(total_nsubjects,files_per_group,region_str,TITLE='Please check region files.',/ONE_COLUMN,/BELOW, $ 
                    /GO_BACK)
                if scrap[0] eq 'GO_BACK' then begin 
                    if region eq 0 or hemisphere eq 0 then goto,goback16c1 else goto,goback16c
                endif
                region_str = ' -region_file '+scrap
            endelse
            if region eq 0 then begin
                if hemisphere eq 1 then begin
                    hem_num_roi = num_roi
                    hem_region_compound_names = hem_region_names
                endif else begin
                    hem_labels = strarr(num_roi,num_roi)
                    LorR = strarr(2)

                    ;LorR[0] = 'L '
                    ;LorR[1] = 'R '
                    ;START150506
                    LorR[0] = 'L'
                    LorR[1] = 'R'

                    for m=0,num_roi-1 do begin
                        k = 0
                        for j=1,num_roi/2 do begin
                            for i=0,1 do begin
                                hem_labels[k,m] = strcompress(string(j),/REMOVE_ALL)+LorR[i]
                                k = k + 1
                            endfor
                        endfor
                    endfor
                    goback16g:
                    repeat begin
                        rtn = get_choice_list_many(hem_region_names,hem_labels,/COLUMN_LAYOUT, $
                            TITLE='Please assign pairs of regions by hemisphere.',DEFAULT=indgen(num_roi),/GO_BACK)
                        if rtn.special eq 'GO_BACK' then goto,goback16d
                        hem_list = rtn.list
                        scrap = total(hem_list[sort(hem_list)]-indgen(num_roi))
                        if scrap ne 0 then $
                            stat=dialog_message('Only a single region may be assigned to a label. Please try again.',/ERROR)
                    endrep until scrap eq 0
                    hem_region_names = hem_region_names[sort(hem_list)]
                    hem_region_compound_names = strarr(num_roi/2)
                    for j=0,num_roi-1,2 do hem_region_compound_names[j/2] = hem_region_names[j]+'_and_'+hem_region_names[j+1]
                    hem_region_compound_names_labels = hem_region_compound_names
                    goback16h:hem_region_compound_names = get_str(num_roi/2,hem_region_compound_names_labels, $
                        hem_region_compound_names, TITLE='Please enter region names.',/ALIGN_RIGHT,/GO_BACK)
                    if hem_region_compound_names[0] eq 'GO_BACK' then goto,goback16g
                    hem_num_roi = num_roi/2
                    hemisphere=2

                    ;START150506
                    LorR=LorR+string(9B) 

                endelse
                if help.enable eq !TRUE then begin
                    widget_control,help.id,SET_VALUE=string('Selecting "offline" creates a text file that can be directly entered ' $
                    +'into statview. An online analysis is done in house.'+string([10B,10B])+'The file created for an offline ' $
                    +'analysis is named "statview.txt". The estimates follow the ordering as given in the driver.dat file.' $
                    +string([10B,10B])+'As a simple example, consider an analysis of timecourses that includes 12 subjects ' $
                    +'and a factor "cue_duration" at two levels, two and four frames. Let'+string(39B)+'s assume an HRF ' $
                    +'containing eight estimates. The file "statview.txt" will then contain 12 lines, one for each subject. ' $
                    +'On a single line, the first eight numbers will be the estimates for cue_duration at two frames averaged ' $
                    +'over region 1. The next eight numbers will be the estimtes for cue_duration at four frames averaged over ' $
                    +'region 1. Next will follow the eight estimates for cue_duration at two frames averaged over region 2, ' $
                    +'then the eight estimates for cue_duration at four frames averaged over region 2. This ordering continues ' $
                    +'for the N selected regions.'+string([10B,10B]) $
                    +'             region 1                          region 2             ...              region N'+string(10B) $
                    +'________________|________________ ________________|________________     ________________|________________' $
                    +string(10B) $
                    +'| cue_d.2_frames cue_d.4_frames | | cue_d.2_frames cue_d.4_frames | ... | cue_d.2_frames cue_d.4_frames |' $
                    +string(10B) $
                    +'   1...time...8   1...time...8       1...time...8   1...time...8    ...    1...time...8   1...time...8')
                endif
                goback16i:offline_online = get_button(['create statview file','fidl anova','go back'],TITLE='Please select')
                if offline_online eq 2 then begin
                    if hemisphere eq 1 then begin
                        goto,goback16c
                    endif else $
                        goto,goback16h
                endif
            endif
            if offline_online eq 0 then begin
                statview_str = ' -statview '
                if region eq 0 then begin
                    region_is_a_factor_online_num_roi = num_roi
                    single_region_str = ' -anova '
                endif else begin
                    hem_num_roi = 1
                    hem_region_compound_names = strarr(hem_num_roi)
                    hem_region_compound_names[0] = ''
                endelse
            endif else begin
                if region eq 0 then begin
                    region_is_a_factor_online_num_roi = num_roi
                    region_or_uncompress_str = ' -text '
                    single_region_str = ' -anova '
                endif else begin
                    region_or_uncompress_str = ' $REGIONS '
                    lcregnamestr = 1
                endelse
            endelse
            roi_str = ' -regions_of_interest ' + region_numbers_str
            vol = num_roi
        endif
    endif else begin
        option_list = intarr(7)
        option_list_lmer = intarr(7)
        Nimage = 1
        goback18:
        if time lt 2 then begin 
            if atlasspace ne !UNKNOWN and nreg[0] eq 0 then begin
                if files_are_glms eq 1 then begin
                    if mask eq '' then begin
                        scrap = get_bool_list(['Uncorrected F-maps','Uncorrected Z-maps', $
                            'Uncorrected Sphericity Adjusted Z-maps'],TITLE='Please select output options.',/GO_BACK)
                        scrap = scrap.list
                        if scrap[0] eq -1 then goto,goback15
                        option_list[[0,1,4]] = scrap
                    endif else begin


                        ;if not keyword_set(LMERR) then begin
                        ;START170308
                        ;if keyword_set(FTEST_SS) then begin
                        if ftest_ss ne 0  then begin

                            scrap = get_bool_list(['Uncorrected F-maps','Uncorrected Z-maps','Monte Carlo Corrected Z-maps'], $
                                TITLE='Please select output options.',/GO_BACK)
                            scrap = scrap.list
                            if scrap[0] eq -1 then goto,goback15
                            option_list[[0,1,5]]=scrap


                        endif else if not keyword_set(LMERR) then begin
                            scrap = get_bool_list(['Uncorrected F-maps','Uncorrected Z-maps', $
                                'Uncorrected Sphericity Adjusted Z-maps','Monte Carlo Corrected Z-maps', $
                                'Monte Carlo Corrected Sphericity Adjusted Z-maps'],TITLE='Please select output options.',/GO_BACK)
                            scrap = scrap.list
                            if scrap[0] eq -1 then goto,goback15
                            option_list[[0,1,4,5,6]] = scrap
                        endif else begin
                            if lmeranovadir ne '' then option_list[[1,5,6]] = 1 
                            scrap = get_bool_list(['Uncorrected Z-maps','Monte Carlo Corrected Z-maps'], $
                                TITLE='Please select output options.',/GO_BACK)
                            scrap = scrap.list
                            if scrap[0] eq -1 then goto,goback15
                            option_list_lmer[[1,5]] = scrap
                        endelse
                    endelse
                endif else begin
                    scrap = get_bool_list(['Uncorrected Z-maps','Monte Carlo Corrected Z-maps'], $
                        TITLE='Please select output options.',/GO_BACK)
                    scrap = scrap.list
                    if scrap[0] eq -1 then goto,goback15
                    option_list[[1,5]] = scrap
                endelse
            endif else begin ;Bay0
                if nreg[0] eq 0 then begin
                    scrap = get_bool_list(['Uncorrected F-maps','Uncorrected Z-maps','Uncorrected Sphericity Adjusted Z-maps'], $
                        TITLE='Please select output options.',/GO_BACK)
                    scrap = scrap.list
                    if scrap[0] eq -1 then goto,goback15
                    option_list[[0,1,4]] = scrap
                endif else begin
                    goback160:
                    region = get_button(['yes','no','go back'],TITLE='Do you wish region to be a factor?')
                    if region eq 2 then goto,goback15
                    if region eq 0 then begin 
                        region_names=*get_labels_struct[0].ifh.region_names
                        rtn = select_files(region_names,TITLE='Please select regions.',/GO_BACK,MIN_NUM_TO_SELECT=1)
                        if rtn.files[0] eq 'GO_BACK' then goto,goback160
                        hem_region_names = rtn.files
                        hem_region_numbers = rtn.index + 1
                        num_roi = rtn.count
                        hem_region_compound_names = hem_region_names
                        hem_num_roi = num_roi
                        region_numbers_str = strjoin(strtrim(hem_region_numbers,2),' ',/SINGLE)
                        region_is_a_factor_online_num_roi = num_roi
                        region_or_uncompress_str = ' -text '
                        single_region_str = ' -anova '
                        roi_str[*] = ' -regions_of_interest ' + region_numbers_str
                        vol = num_roi
                    endif
                    analysis_type=1
                    Nimage=0
                endelse
            endelse
        endif else begin ;STRUCTURAL    
            option_list[1] = 1  
        endelse
        if analysis_type eq 0 then begin
            if option_list[0] eq -1 then goto,goback18
            mode_str = ' -output '
            if option_list[0] eq 1 then mode_str = mode_str + 'F_uncorrected '
            if option_list[1] eq 1 then mode_str = mode_str + 'Z_uncorrected '
            if option_list[2] eq 1 then mode_str = mode_str + 'F_corrected '
            if option_list[3] eq 1 then mode_str = mode_str + 'Z_corrected '
            if option_list[4] eq 1 then mode_str = mode_str + 'Z_uncorrected_BOX '
            if option_list[5] eq 1 then mode_str = mode_str + 'Z_monte_carlo '
            if option_list[6] eq 1 then mode_str = mode_str + 'Z_BOX_monte_carlo '
            if keyword_set(LMERR) then begin
                if option_list_lmer[0] eq -1 then goto,goback18
                mode_str_lmer = ' -output '
                if option_list_lmer[0] eq 1 then mode_str_lmer = mode_str_lmer + 'F_uncorrected '
                if option_list_lmer[1] eq 1 then mode_str_lmer = mode_str_lmer + 'Z_uncorrected '
                if option_list_lmer[2] eq 1 then mode_str_lmer = mode_str_lmer + 'F_corrected '
                if option_list_lmer[3] eq 1 then mode_str_lmer = mode_str_lmer + 'Z_corrected '
                if option_list_lmer[4] eq 1 then mode_str_lmer = mode_str_lmer + 'Z_uncorrected_BOX '
                if option_list_lmer[5] eq 1 then mode_str_lmer = mode_str_lmer + 'Z_monte_carlo '
                if option_list_lmer[6] eq 1 then mode_str_lmer = mode_str_lmer + 'Z_BOX_monte_carlo '
            endif
            if option_list[2]+option_list[3] gt 0 then begin
                if(help.enable eq !TRUE) then begin
                    widget_control,help.id,SET_VALUE=string('If you have specified delays, this correction to the p-value is ' $
                    +'automatically factored into the multiple comparisons correction.'+string([10B,10B])+'Ex. Say you have ' $
                    +'specified three delays and you desire a p-value of 0.05. You would then enter 0.05 as your desired p-value. ' $
                    +string([10B,10B])+'NOTE: This automatic correction for multiple delays is only implemented within ' $
                    +'fidl_anova.')
                endif
                scrap = get_str(1,'Please specify a p-value for F-field correction','0.05',WIDTH=15, $
                    TITLE='Worsley and Bonferroni Correction for Multiple Comparisons')
                p_val_str = ' -p_val '+ scrap[0];
                if lcdelays eq 'yes' then begin
                    if float(scrap[0]) gt .05/num_delays-.1 and float(scrap[0]) lt .05/num_delays+.1 then begin
                        stat=dialog_message('The correction for the number of delays has been automatically implemented into ' $
                                            +'fidl_anova.',/INFORMATION)
                        change_p_value = get_button(['yes','no - The p-value of '+strcompress(scrap[0],/REMOVE_ALL) $
                                         +' is what I really want.'],TITLE='Do you desire a p-value of 0.05?')
                        if change_p_value eq 0 then p_val_str = ' -p_val 0.05 '
                    endif
                endif
                if help.enable eq !TRUE then begin
                    widget_control,help.id,SET_VALUE='If statistics corrected for multiple comparisons are desired, select ' $
                    +'from a Bonferroni correction, Keith Worsley'+string(39B)+'s correction, or the more sensitive of the two. ' $
                    +'For whole brain corrections, the Bonferroni and Worsley thresholds have been found to be comparable. ' $
                    +'All inferences are made on the F-maps.'+string([10B,10B]) $
                    +'Worsley, K.J., Marrett, S., Neelin, P., Vandal, A.C., ' $
                    +'Friston, K.J., and Evans, A.C. '+string(40B)+'1996'+string(41B) $
                    +'. A unified statistical approach for determining ' $
                    +'significant signals in images of cerebral activation. Human Brain Mapping, 4:58-73.'
                endif
                scrap = get_button(['Most sensitive threshold','Bonferroni','Worsley'], $
                    TITLE='Please specify the method for F-field correction')
                if scrap eq 0 then $
                    correction_str = ' -correction most_sensitive ' $
                else if scrap eq 1 then $
                    correction_str = ' -correction Bonferroni ' $
                else $
                    correction_str = ' -correction Worsley '
            endif
            if option_list[5]+option_list[6]+option_list_lmer[5]+option_list_lmer[6] gt 0 then begin
                monte_carlo_fwhm = round(monte_carlo_fwhm)
                rtn = threshold_extent(!F_STAT,monte_carlo_fwhm,atlasspace,mask)
                if rtn.threshold_extent_str ne 'ERROR' then threshold_extent_str = rtn.threshold_extent_str
            endif
        endif
    endelse
    if analysis_type eq 1 and offline_online eq 1 and region eq 1 then begin
        if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='Individual and group averaged timecourses and the ' $
            +'associated stderr may be outputed for each previously selected region. The option of row or column formatted ' $
            +'output is merely for convenience to satisfy a variety of plotting software.'

        ;START150506
        lcgoback16e=0
        lcgoback16f=0 

        ;if time eq 0 and superspecial eq '' and nglm_per_subject[0] eq 1 then begin
        ;START180131
        if time eq 0 and superspecial eq '' and nnglm_per_subjectgt1 eq 0 then begin

            goback16e:

            ;START150506
            lcgoback16e=1

            cols_or_rows=get_button(['columns','rows','GO BACK'],TITLE='For "Average Statistics", average timecourses should be in')
            if cols_or_rows eq 2 then begin
                if region eq 1 then $
                    goto,goback16c $
                else $
                    goto,goback16i
            endif else if cols_or_rows eq 0 then begin
                goback16f:

                ;START150506
                lcgoback16f=1

                scrap = get_button(['yes','no','GO BACK'],TITLE='Do you want individual subject timecourses?')
                if scrap eq 2 then $
                    goto,goback16e $
                else if scrap eq 0 then $
                    cols_or_rows_str = ' -columns INDIVIDUAL' $
                else $
                    cols_or_rows_str = ' -columns'
            endif
        endif
    endif
endelse
goback18a:
if offline_online eq 1 then begin
    scrap = get_str(1,'voxel threshold','0.01',WIDTH=50,LABEL='Average of the absolute value of all data points at a particular ' $
        +string(10B)+'voxel must exceed this value for the voxel to be analyzed.',/GO_BACK, $
        TITLE='Units are percent change for BOLD data from GLMs.')
    if scrap[0] eq 'GO_BACK' then begin
        if n_elements(lcgoback16f) gt 0 then begin
            if lcgoback16f eq 1 then goto,goback16f else if lcgoback16e eq 1 then goto,goback16e
        endif else if n_elements(lcgoback16) gt 0 then goto,goback16 else goto,goback8
    endif
    anova_threshold_str = ' -voxel_threshold '+ scrap[0]
endif


;START151124
timelabel='' & specialstr=''
if nframes gt 1 then begin
    if rtn_within.nfactors eq 0 then begin
        scrap='This is a simple main effect of ' 
        if files_are_glms eq 1 then specialstr = 'Simple main effect of time is ' $
            +strjoin(strtrim(labels[index_conditions[0,0:sumrows[0]-1]],2),'+',/SINGLE)
    endif else $
        scrap='Main effect of time should be labeled' 
    dummy = get_str(1,scrap,'time',TITLE='Simple main effect',WIDTH=50,/GO_BACK)
    if dummy[0] eq 'GO_BACK' then goto,goback18a
    timelabel=dummy[0]
endif


if time eq 1 then lcmag = ' -mag ONLY '
goback19:
if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='Three files are created. The first is the driving file ' $
    +'that contains the design. The second is the anova script. The third is a script that produces timecourses and ' $
    +'magnitudes averaged across subjects. For a regional analysis, you may specify a filename where the results get ' $
    +'printed. If GLMS have been loaded individually, you may specify a filename where those GLM path names are saved ' $
    +'in a glm list file for future anovas.'
if files_are_glms eq 1 and superspecial eq '' and offline_online eq 1 then begin
    if nglm_per_subject[0] eq 1 then lccompute_avg_zstat=1 else lccompute_avg_zstat=2
endif else $
    lccompute_avg_zstat=0 
goback20:
if not keyword_set(LMERR) then begin
    scrap = ['driving file','anova script']
    scraplabels = ['driver.dat','fidl_anova.csh']
endif else begin
    scrap = ['driving file','lme script']
    scraplabels = ['driver.dat','fidl_lmerTest.csh']
endelse
if lccompute_avg_zstat ge 1 then begin
    scrap = [scrap,'group averages script']
    if time eq 0 then $
        scraplabels = [scraplabels,'avg_timecourses.csh'] $
    else if time eq 1 then $
        scraplabels = [scraplabels,'avg_magnitudes.csh']
endif
if analysis_type eq 1 or datatype eq 3 then begin
    if region+offline_online eq 0 then begin 
        scrap = [scrap,'anova output']
        scraplabels = [scraplabels,'statview.txt']
    endif else begin
        scrap = [scrap,'anova output']
        scraplabels = [scraplabels,'anova_region_results.txt']
        scrap = [scrap,'cellmeans']
        scraplabels = [scraplabels,'anova_region_results_cellmeans.txt']
        if keyword_set(LMERR) then begin
            scrap = [scrap,'lme output']
            scraplabels = [scraplabels,'lmerTest.txt']
            scrap = [scrap,'lme estimates']
            scraplabels = [scraplabels,'lmerTest_est.txt']
        endif
    endelse
endif
if lccompute_avg_zstat ge 1 and analysis_type eq 1 then begin
    if time lt 2 then begin
        for i=0,rtn_between.treatments[0]-1 do begin
            if rtn_between.treatments[0] eq 1 then begin
                scrap = [scrap,'group average output']
                scraplabels = [scraplabels,'avg_stats_for_regions.txt']
            endif else begin
                scrap = [scrap,'group average output, ' + rtn_between.treatment_str[i]]
                scraplabels=[scraplabels,'avg_stats_for_regions_'+ fix_script_name(rtn_between.driver_levels[i],/SPACE_ONLY) + '.txt']
            endelse
        endfor
    endif
endif
if time lt 2 then begin
    if load eq 1 then begin
        scrap = [scrap,'GLM list']
        scraplabels = [scraplabels,'list_of_glms.glm_list']
    endif
endif else if load eq 1 then begin
    scrap = [scrap,'anatomical list']
    scraplabels = [scraplabels,'list_of_anats.anat_list']
endif
if Nimage eq 1 then begin
    scrap = [scrap,'Number of subjects at each voxel']
    scraplabels = [scraplabels,'Nimage.4dfp.img']
endif
names = get_str(n_elements(scrap),scrap,scraplabels,WIDTH=50,TITLE='Please enter desired filenames.',/GO_BACK)
if names[0] eq 'GO_BACK' then begin
    if analysis_type eq 0 then $
        goto,goback18a $
    else if cols_or_rows eq 0 then $
        goto,goback16f $
    else $
        goto,goback16e
endif
names[0:1] = fix_script_name(names[0:1])
driver0 = names[0]
fidl_anova_csh = names[1]
j = 1
if lccompute_avg_zstat ge 1 then begin
    j = j + 1
    compute_avg_zstat_csh_str = fix_script_name(names[j])
endif
if analysis_type eq 1 or datatype eq 3 then begin
    j = j + 1
    anova_output_str = ' -regional_anova_name "' + names[j] + '"'
    if region+offline_online ne 0 then begin
        j = j + 1
        cellmeansstr = ' -cellmeans "' + names[j] + '"'
    endif
    if keyword_set(LMERR) then begin
        j=j+1
        lmeoutstr=' -regional_anova_name "' + names[j] + '"'
        if region+offline_online ne 0 then begin
            j = j + 1
            lmeoutstr=lmeoutstr+' -lmebetase "' + names[j] + '"'
        endif
    endif
endif
if lccompute_avg_zstat ge 1 and analysis_type eq 1 then begin
    if time lt 2 then begin
        avgstat_output_str = strarr(rtn_between.treatments[0])
        for i=0,rtn_between.treatments[0]-1 do begin
            j = j + 1
            avgstat_output_str[i] = ' -regional_avgstat_name "' + names[j] + '"'
        endfor
    endif
endif
if time le 2 and load eq 1 then begin
    j = j + 1
    glm_list_str = names[j]
endif
if Nimage eq 1 then begin
    j = j + 1
    if names[j] ne '' then begin
        if rstrpos(names[j],'.4dfp.img') eq -1 then names[j] = names[j] + '.4dfp.img'
        Nimage_name_str = ' -Nimage_name "' + names[j] + '"'
    endif
endif
if(files_are_glms eq 0 or files_are_glms eq 2) and time eq 0 and period gt 1 then begin
    if dialog_message('Do your timecourses need to be normalized?'+string(10B)+string(10B)+'If you answer "yes", they will ' $
        +'be normalized to the mean of all frames in the file, not just the ones you have selected for the ANOVA.'+string(10B) $
        +'If this is not what you want, then tell McAvoy and he will code whatever you want.',/QUESTION) eq 'Yes' then $
        normstr = ' -normalize ALL'
endif
if superspecial eq 'Hz' then begin
    scrap = get_button(['yes','no','go back'],TITLE='Normalize to individual baselines?')
    if scrap eq 2 then goto,goback19
    if scrap eq 0 then indbasestr = ' -indbase'
endif
action = get_button(['execute','return','go back'],TITLE='Please select action.',BASE_TITLE=fidl_anova_csh)
if action eq 2 then goto,goback19
scrap = 'The text file '+names[0]+' that outlines the anova design will be created. This file lists the treatments ' $
       +'line by line and is viewable in any text editor.'+string([10B,10B])+'You may choose to execute the ' $
       +fidl_anova_csh+' now, or at your convenience from any unix prompt.'
if time eq 0 then dummy = 'timecourse' else dummy = 'magnitude'
scrap = scrap +string([10B,10B])+'Upon execution of '+fidl_anova_csh+', a set of scratch files in the same ' $
       +'directories as the associated glms will be created. These files are used in the computation of the anova. ' $
       +'They are automatically deleted upon completion.'
if compute_avg_zstat_csh_str ne '' then begin
       scrap = scrap + string([10B,10B])+'A second executable ' $
       +compute_avg_zstat_csh_str +' will also be created. This program will produce a '+dummy+' averaged across all ' $
       +'subjects for each respective treatment in '+names[0]+'. This executable is equivalent to that produced by ' $
       +'"Average statistics".'
endif
if help.enable eq !TRUE then widget_control,help.id,SET_VALUE=scrap
if region_is_a_factor_online_num_roi ne 1 then begin
    hem_region_compound_names=hem_region_compound_names+string(9B)
    if hemisphere eq 2 then begin
        scrap = strlen(hem_region_names)
        maxlength = max(scrap)
        for p=0,region_is_a_factor_online_num_roi/hemisphere-1 do $
            hem_region_names[p] = hem_region_names[p] + strmid(space,0,maxlength-scrap[p]+1)
    endif
endif
chunks=1
chunks_str=''
proc=1
if datatype ne 3 then begin 
    chunkthis=ulong64(1000000000./4.)
    total_vol=ulong64(nsubject)*ulong64(rtn_within.treatments[0])*ulong64(nframes)*ulong64(vol)
    if keyword_set(LMERR) and analysis_type eq 0 then begin
        chunks = 20
        print,'Compressed files will be in chunks.'
        chunks_str = ' -chunks'
        spawn,'cat /proc/cpuinfo | grep processor | wc -l',processor
        print,'processor=',processor
        if processor[0] ge 20 then proc=20 else if processor[0] ge 10 then proc=10 else if processor[0] ge 5 then proc=5
    endif else if total_vol gt chunkthis or total_vol lt 0 then begin
        if pref.chunk eq 1 then begin
            chunks = 20
            chunks_str = ' -chunks'
            print,'Compressed files will be in chunks.'
        endif else $ 
            print,'pref.chunk=',trim(pref.chunk),' Chunk off. Compressed files will not be in chunks.' 
    endif
endif
driver='' & driverlmernomiss='' & driverlmermiss=''

;START170706
ss_driver=make_array(nsubject,/STRING,VALUE='')

if not keyword_set(LMERR) then begin

    ;START180427
    driverf = ''

    if not keyword_set(FTEST_SS) or nsubject eq 1 then $
        driverf=directory+'/'+driver0 $
    else if monte_carlo_fwhm gt 0. then begin
        dir0='all'
        cmd='mkdir '+dir0
        print,cmd
        spawn,cmd
        driverf=directory+'/'+dir0+'/'+driver0
    endif

    ;if driver ne '' then begin
    ;START180425
    if driverf ne '' then begin

        driver=' -driver "'+driverf+'"'
        openw,lu_driver,driverf,/GET_LUN
    endif


    ;if keyword_set(FTEST_SS) and nsubject gt 1 then begin
    ;    ss_driverf=strarr(nsubject)
    ;    for i=0,nsubject-1 do begin
    ;        ss_driverf[i]=directory+'/'+ss_dir[i]+'/'+driver0
    ;        ss_driver[i]=' -driver "'+ss_driverf[i]+'"'
    ;    endfor
    ;endif
    ;START180417
    if keyword_set(FTEST_SS) and nsubject gt 1 then begin
        if n_elements(ss_dir) ne 0 then $ 
            superbird0 = ss_dir+'/' $
        else begin
            rtn_gr = get_root(files_per_group,'.glm')
            superbird0 = rtn_gr.file+'_'
        endelse
        ss_driverf=strarr(nsubject)
        for i=0,nsubject-1 do begin
            ss_driverf[i]=directory+'/'+superbird0[i]+driver0
            ss_driver[i]=' -driver "'+ss_driverf[i]+'"'
        endfor
    endif


endif else begin
    if lmeranovadir ne '' then begin
        driverf=directory+'/'+lmeranovadir+'/'+driver0
        driver = ' -driver "'+driverf+'"'
        openw,lu_driver,driverf,/GET_LUN
    endif
    if lmernomissdir ne '' or lmermissdir ne '' then begin
        if proc eq 1 then begin
            if lmernomissdir ne '' then begin
                driverlmernomissf=directory+'/'+lmernomissdir+'/'+driver0
                driverlmernomiss=' -driver "'+driverlmernomissf+'"'
                openw,lu_driverlmernomiss,driverlmernomissf,/GET_LUN
            endif
            if lmermissdir ne '' then begin
                driverlmermissf=directory+'/'+lmermissdir+'/'+driver0
                driverlmermiss=' -driver "'+driverlmermissf+'"'
                openw,lu_driverlmermiss,driverlmermissf,/GET_LUN
            endif
        endif else begin
            if lmernomissdir ne '' and lmermissdir ne '' then proc=proc/2
            driver1=get_root(driver0,'.dat')
            undefine,driver0
            wallace0=trim(indgen(proc)+1)
            idx=where(strlen(wallace0) eq 1,cnt)
            if cnt ne 0 then wallace0[idx]='0'+wallace0[idx]
            driver0=strarr(proc)
            for i=0,proc-1 do driver0[i]=driver1.file+wallace0[i]+'.dat'
            undefine,driver1,idx,cnt
            if lmernomissdir ne '' then begin
                driverlmernomissf=directory+'/'+lmernomissdir+'/'+driver0
                driverlmernomiss=' -driver "'+driverlmernomissf+'"'
                lu_driverlmernomiss=lonarr(proc)
                for i=0,proc-1 do begin
                    openw,lu0,driverlmernomissf[i],/GET_LUN
                    lu_driverlmernomiss[i]=lu0
                endfor
            endif
            if lmermissdir ne '' then begin
                driverlmermissf=directory+'/'+lmermissdir+'/'+driver0
                driverlmermiss=' -driver "'+driverlmermissf+'"'
                lu_driverlmermiss=lonarr(proc)
                for i=0,proc-1 do begin
                    openw,lu0,driverlmermissf[i],/GET_LUN
                    lu_driverlmermiss[i]=lu0
                endfor
            endif
            undefine,lu0
        endelse
    endif
endelse
scrap='subject'
if region_is_a_factor_online_num_roi ne 1 then begin
    scrap=scrap+string(9B)+'regionfidl'
    if hemisphere ne 1 then scrap=scrap+string(9B)+'hemispherefidl'
endif
if rtn_within.nfactors gt 0 then scrap=scrap+string(9B)+strjoin(rtn_within.factor_names,string(9B),/SINGLE)
scrap=scrap+string(9B)+timelabel
if rtn_between.nfactors gt 0 then scrap=scrap+string(9B)+strjoin(rtn_between.factor_names,string(9B),/SINGLE)
if not keyword_set(FTEST_SS) then begin
    if slkn gt 0 then scrap=scrap+string(9B)+strjoin(slkcollab,string(9B),/SINGLE)
endif


;if driver ne '' then begin
;    if datatype eq 3 then $
;        wallace=scrap+string(9B)+'DATA' $
;    else if chunks eq 20 then $
;        wallace=scrap+string(9B)+'CHUNKS20' $
;    else $
;        wallace=scrap+string(9B)+'*.4dfp.img'
;    printf,lu_driver,wallace
;
;    ;START170706
;    ss_wallace=wallace
;
;endif
;START170706
if datatype eq 3 then $
    wallace=scrap+string(9B)+'DATA' $
else if chunks eq 20 then $
    wallace=scrap+string(9B)+'CHUNKS20' $
else $
    wallace=scrap+string(9B)+'*.4dfp.img'
if driver ne '' then printf,lu_driver,wallace 
ss_wallace=wallace



chunksproc=chunks/proc
if driverlmernomiss[0] ne '' or driverlmermiss[0]  ne '' then begin
    wallace=scrap+string(9B)+'CHUNKS'+trim(chunksproc)
    if driverlmernomiss[0] ne '' then for i=0,proc-1 do printf,lu_driverlmernomiss[i],wallace
    if driverlmermiss[0] ne '' then for i=0,proc-1 do printf,lu_driverlmermiss[i],wallace
endif
mailstr = ''
filecount = rtn_within.treatments[0]
nfiles_val = filecount*nframes*region_is_a_factor_online_num_roi*chunks
framestr = strarr(rtn_between.treatments[0],nframes)
nfpg_use_these = intarr(rtn_between.treatments[0])
filenameall = ''
t4all = ''
subject_namesall = ''
nsubject_namesall = intarr(rtn_between.treatments[0]) 
if superspecial eq 'Hz' then tcHzstr = strarr(total_nsubjects,rtn_within.treatments[0]) 
if datatype ne 3 then bigcstr = strarr(nsubject)
if n_elements(slklistnomiss) eq 0 then slklistnomiss=1
idxnomiss=-1
idxnomiss2=-1
nglmpersubnomiss=-1
fidl_anova_csh = directory+'/'+fidl_anova_csh 
openw,lu_csh,fidl_anova_csh,/GET_LUN
top_of_script,lu_csh
;STARTLOOP
j=0 & jj=0 & j1=0 & j2=0
widget_control,/HOURGLASS
for n=0,rtn_between.treatments[0]-1 do begin
    if time eq 0 and bf_str eq '' and superspecial ne 'Hz' then begin
        for m=0,nframes-1 do begin
            framestr[n,m] = strcompress(conditions_time[m,0],/REMOVE_ALL)
            for r=1,sumrows_time[m]-1 do $
                framestr[n,m] = string(framestr[n,m],strcompress(conditions_time[m,r],/REMOVE_ALL),FORMAT='(a,"+",a)')
        endfor
    endif
    startm=0 & savem=0

    ;print,'here2 n=',n,' nsubject_per_group[n]=',nsubject_per_group[n]
    ;print,'here2 total(nglm_per_subject)=',total(nglm_per_subject)
    ;print,'here2 size(sum_contrast_for_treatment)=',size(sum_contrast_for_treatment)
    ;print,'here2 nsubject=',nsubject,' n_elements(nglm_per_subject)=',n_elements(nglm_per_subject)

    for i=0,nsubject_per_group[n]-1 do begin
        if j eq total_nsubjects then goto,endofthis
        if use_these[j] eq 1 then begin

            ;START170706
            ;ss_driver=''
            ;if keyword_set(FTEST_SS) and nsubject gt 1 then begin
            ;    ss_driverf=directory+'/'+ss_dir[j]+'/'+driver0
            ;    ss_driver=' -driver "'+ss_driverf+'"'
            ;    openw,ss_lu_driver,ss_driverf,/GET_LUN
            ;    printf,ss_lu_driver,ss_wallace
            ;endif
            ;START170706
            if ss_driver[j] ne '' then begin
                openw,ss_lu_driver,ss_driverf[j],/GET_LUN
                printf,ss_lu_driver,ss_wallace
            endif


            t4=''
            if datatype ne 3 then begin
                if glm_space[0] eq !SPACE_DATA then begin
                    if t4select[0] ne '' then begin
                        t4 = strjoin(t4select[jj:jj+nglm_per_subject[j]-1],' ',/SINGLE) 
                    endif
                endif
                if nglm_per_subject[0] ne -1 then begin
                    nfpg_use_these[n] = nfpg_use_these[n] + nglm_per_subject[j] 
                    subject_namesall = [subject_namesall,subject_names[j]]
                    nsubject_namesall[n] = nsubject_namesall[n] + 1
                endif
                if slkn gt 0 then subjectstr=slkid[j1] else subjectstr=j+1
                if bf_str ne '' then $
                    cstr = bf_str $
                else if time eq 0 then begin
                    if files_are_glms eq 0 then cstr = ' -frames_of_interest' else cstr = ' -tc'
                    precstr=cstr
                    if files_are_glms eq 1 then begin
                        if superspecial eq 'Hz' then begin
                            startm = savem

                            ;utij = use_thesei[j]
                            ;START161020
                            utij = iuse_these[j]

                            filecount = 0
                            index_glm = *subject_get_labels_struct[utij].index_glm
                            index_glm = index_glm[sum_contrast_for_treatment[utij,startm,0]-1]
                            filename = files_per_group[index_glm]
                            if glm_space[0] eq !SPACE_DATA then begin
                                if t4select[0] ne '' then t4 = t4select[index_glm]
                            endif
                            subjectstr = strtrim(utij+1,2)
                            for m=startm,rtn_within.treatments[0]-1 do begin
                                index = where(sum_contrast_for_treatment[utij,m,*] ne 0,count)
                                junk = ' '
                                goose = ''
                                for k=0,count-1 do begin
                                    scfti = sum_contrast_for_treatment[utij,m,index[k]]-1
                                    ig = *subject_get_labels_struct[utij].index_glm
                                    ;print,'j=',j,' m=',m,' scfti=',scfti,' ig[scfti]=',ig[scfti],' index_glm=',index_glm
                                    if ig[scfti] ne index_glm then begin
                                        savem = m
                                        goto,outofhere
                                    endif
                                    sum_all = *subject_get_labels_struct[utij].sum_all
                                    scft = sum_contrast_for_treatment[utij,m,k]-sum_all[scfti]
                                    for l=0,effect_length_ts[index_glm,scft-1]-1 do begin
                                        get_cstr_new,goose,scft,effect_length_ts[index_glm,*],l+1,1,junk
                                        junk = '+'
                                    endfor
                                    filecount = filecount + 1
                                endfor
                                cstr = cstr + goose
                                tcHzstr[j,m] = goose
                            endfor
                            savem = 0
                            if utij eq (nsubject-1) then $
                                j = total_nsubjects-1 $
                            else $

                                ;while use_thesei[j+1] eq utij do j = j + 1
                                ;START161020
                                while iuse_these[j+1] eq utij do j = j + 1

                            outofhere:
                        endif else begin
                            utij=j
                            cstr0=strarr(rtn_within.treatments[0])
                            effect_length=*subject_get_labels_struct[utij].effect_length
                            scft=sum_contrast_for_treatment[utij,*,*]
                            if n_elements(conditions_time_treat) eq 0 then begin
                                for m=0,rtn_within.treatments[0]-1 do begin
                                    cstrscrap=''
                                    for k=0,nframes-1 do rtn=get_cstr_new2(cstrscrap,scft[0,m,*],effect_length, $
                                        conditions_time[k,*],sumrows_time[k],' ') ;This must be a space. ' '
                                    cstr0[m]=cstrscrap
                                endfor
                            endif else begin
                                for m=0,rtn_within.treatments[0]-1 do begin
                                    cstrscrap=''
                                    rtn=get_cstr_new2(cstrscrap,scft[0,m,*],effect_length, $
                                        conditions_time_treat[m,*],sumrows_time_treat[m],' ') ;This must be a space. ' '
                                    cstr0[m]=cstrscrap
                                endfor
                            endelse
                            bigcstr[utij]=strjoin(cstr0,' ',/SINGLE)
                            cstr = precstr + bigcstr[utij]
                        endelse
                    endif else begin
                        if n_elements(pasteloop) ne 0 then begin
                            for k=0,pasteloop-1 do begin
                                get_cstr_new,cstr,sum_contrast_for_treatment[j,0,*],effect_length_ts[j,*],conditions_time[k,*], $
                                    sumrows_time[k],' ' ;This must be a space. ' '
                            endfor
                        endif else begin

                            ;for m=0,rtn_within.treatments[0]-1 do $
                            ;    cstr=cstr+' '+strjoin(strtrim(conditions_time+m*period,2),' ',/SINGLE)
                            ;START161216
                            if lcnglm_per_subject eq 0 then begin
                                for m=0,rtn_within.treatments[0]-1 do $
                                    cstr=cstr+' '+strjoin(strtrim(conditions_time+m*period,2),' ',/SINGLE)
                            endif else begin
                                cstr=cstr+' '+strjoin(trim(indgen(nglm_per_subject[j])+1),' ',/SINGLE)
                            endelse

                        endelse
                    endelse
                endif else if time eq 1 then begin ;use magnitudes
                    if ic_time eq 1 then begin
                        cstr = ' -contrasts' ;need space for compute_zstat calls
                        for m=0,rtn_within.treatments[0]-1 do $
                            cstr = string(cstr,strcompress(string(sum_contrast_for_treatment[j,m,0]),/REMOVE_ALL),FORMAT='(a," ",a)')
                    endif else begin
                        cstr = ' -frames_of_interest'
                        for m=0,rtn_within.treatments[0]-1 do begin
                            for k=0,nframes-1 do begin
                                get_cstr_new,cstr,sum_contrast_for_treatment[j,m,*],effect_length_ts[j,*],conditions_time[m,*], $
                                    sumrows[m],' ' ;This must be a space. ' '
                            endfor
                        endfor
                    endelse
                endif else $ ;structural
                    cstr = ' -frames_of_interest 1'
                scrap = ''
                if glm_space[0] eq !SPACE_DATA then begin
                    if t4 ne '' then begin
                        scrap = ' -xform_file ' + t4
                        if ftest_ss eq 1 then t4ssstr = scrap
                    endif
                endif
            endif 
            if filecount gt 0 then begin
                if datatype ne 3 then begin
                    subjectstr = trim(subjectstr) + string(9B)
                    slkstr=''
                    if not keyword_set(FTEST_SS) then begin
                        if slkn gt 0 then begin ;CHANGE for multiple glms per subject with different slk for each glm 
                            slkstr=strjoin(slkdata[j1,*],string(9B),/SINGLE)+string(9B)
                        endif
                    endif
                    if files_are_glms eq 1 then begin
                        ;print,'files_per_group[j]=',files_per_group[j]
                        ;print,'cstr=',cstr
                        ;print,'lcmag=',lcmag
                        ;print,'scrap=',scrap
                        ;print,'gauss_str=',gauss_str
                        ;print,'compress_str=',compress_str
                        ;print,'region_str=',region_str
                        ;print,'roi_str=',roi_str
                        ;print,'single_region_str=',single_region_str
                        ;print,'delay_str=',delay_str
                        ;print,'chunks_str=',chunks_str
                        ;print,'magnorm_str=',magnorm_str
                        ;print,'unscaled_str=',unscaled_str
                        ;print,'scratchidr_str=',scratchdir_str
                        ;print,'n_elements(filename)=',n_elements(filename)
                        ;print,'n_elements(cstr)=',n_elements(cstr)
                        ;print,'n_elements(scrap)=',n_elements(scrap)

                        if slklistnomiss[j1] eq 0 and driverlmermiss[0] eq '' then goto,bottom

                        ;START170706
                        ;if ss_driver[j] ne '' then ss_cstr[j]=cstr



                        ;filename = strjoin(files_per_group[jj:jj+nglm_per_subject[j]-1],' ',/SINGLE)
                        ;command = '/fidl_zstat2 -glm_file '+filename+cstr+lcmag+scrap+gauss_str+compress_str+atlas_str $
                        ;    +region_str[j]+roi_str[j]+single_region_str+delay_str+chunks_str+magnorm_str+unscaled_str+hipassstr $
                        ;    +one_file_str+indbasestr+scratchdir_str
                        ;dummy = !BINEXECUTE + command + ' -names_only '
                        ;print,dummy
                        ;widget_control,/HOURGLASS
                        ;spawn,dummy,result
                        ;START180502
                        spawn,!BINEXECUTE+'/fidl_timestr2',timestr
                        fidl_zstat2_csh=scratchdir+'fidl_zstat2_'+timestr[0]+'.csh'
                        openw,lu0,fidl_zstat2_csh,/GET_LUN
                        printf,lu0,'#!/bin/csh'+string(10B)+'unlimit'
                        print_files_to_csh,lu0,nglm_per_subject[j],files_per_group[jj:jj+nglm_per_subject[j]-1],'GLM_FILE', $
                            'glm_file',/NO_NEWLINE
                        command = '/fidl_zstat2 $GLM_FILE'+cstr+lcmag+scrap+gauss_str+compress_str+atlas_str $
                            +region_str[j]+roi_str[j]+single_region_str+delay_str+chunks_str+magnorm_str+unscaled_str+hipassstr $
                            +one_file_str+indbasestr+scratchdir_str
                        dummy = !BINEXECUTE + command + ' -names_only '
                        printf,lu0,dummy
                        close,lu0
                        free_lun,lu0
                        widget_control,/HOURGLASS
                        spawn,'chmod +x '+fidl_zstat2_csh
                        print,fidl_zstat2_csh
                        widget_control,/HOURGLASS
                        spawn,fidl_zstat2_csh,result
                        nfiles = n_elements(result) - offset_spawn
                        result = result[offset_spawn:nfiles+offset_spawn-1]
                        if strpos(result[nfiles-1],'fidlError') ne -1 then begin
                            stat=dialog_message_long('ERROR',result)
                            return
                        endif
                        if nfiles ne nfiles_val then begin
                            if region eq 0 then begin
                                use_these[j] = 0
                                nfpg_use_these[n] = nfpg_use_these[n] - 1
                                regionflag = 1
                                regionflagstr = [regionflagstr,stat.str]
                                goto,bottom
                            endif
                            if keyword_set(LMERR) then begin
                                result0=result
                                result=make_array(nfiles_val,/STRING,VALUE='NA')
                                idx=where(cstr0 ne '',cnt)
                                ;idx=idx*chunks+indgen(chunks) THIS DOESN'T WORK
                                idx=idx*chunks
                                idx1=-1
                                for m=0,cnt-1 do idx1=[idx1,idx[m]+indgen(chunks)]
                                idx1=idx1[1:*]
                                result[idx1]=result0
                            endif
                        endif

                        ;printf,lu_csh,'nice +19 $BIN' + command + mailstr
                        ;printf,lu_csh,''
                        ;START180502
                        print_files_to_csh,lu_csh,nglm_per_subject[j],files_per_group[jj:jj+nglm_per_subject[j]-1],'GLM_FILE', $
                            'glm_file',/NO_NEWLINE
                        printf,lu_csh,'nice +19 $BIN' + command 
                        printf,lu_csh,''

                        ;scrap3 = strsplit(filename,/EXTRACT)
                        ;filenameall = [filenameall,scrap3]
                        ;START180503
                        filenameall = [filenameall,files_per_group[jj:jj+nglm_per_subject[j]-1]]

                        if glm_space[0] eq !SPACE_DATA then begin
                            if t4 ne '' then t4all = [t4all,strsplit(t4,/EXTRACT)]
                        endif
                    endif else begin
                        if files_are_glms eq 2 then begin
                            rl=read_list(files_per_group[j])
                            if rl.msg ne 'OK' then return
                            dummy = rl.img
                        endif else if n_elements(filesselect) eq 0 then $
                            dummy = files_per_group[j] $
                        else $
                            dummy = filesselect[j,*]
                        filenameall = [[filenameall],[dummy]]
                        dummy = ' -tc_files ' + strjoin(dummy,' ',/SINGLE) 
                        command = '/fidl_bolds'+dummy+scrap+gauss_str+compress_str+atlas_str+region_str[j]+roi_str[j] $
                            +single_region_str+chunks_str+cstr+normstr+scratchdir_str
                        dummy = !BINEXECUTE + command + ' -names_only '
                        print,dummy
                        widget_control,/HOURGLASS
                        spawn,dummy,result
                        nfiles = n_elements(result) - offset_spawn
                        result = result[offset_spawn:nfiles+offset_spawn-1]

                        ;START161219
                        if strpos(result[nfiles-1],'fidlError') ne -1 then begin
                            stat=dialog_message_long('ERROR',result)
                            return
                        endif
                        if nfiles ne nfiles_val then begin
                            if keyword_set(LMERR) then begin
                                result0=result
                                result=make_array(nfiles_val,/STRING,VALUE='NA')

                                ;idx=imgselecti[jj:jj+nglm_per_subject[j]-1]*chunks
                                ;idx1=-1
                                ;for m=0,nglm_per_subject[j]-1 do idx1=[idx1,idx[m]+indgen(chunks)]
                                ;idx1=idx1[1:*]

                                idx=imgselecti[jj:jj+nglm_per_subject[j]-1]*chunks
                                idx1=intarr(nfiles)
                                ;print,'here3 idx=',idx,' nfiles=',nfiles,' size(idx1)=',size(idx1)
                                l=0
                                for m=0,nglm_per_subject[j]-1 do begin
                                    ;print,'here4 l=',l,' l+chunks-1=',l+chunks-1 
                                    ;print,'here4 idx[m]+indgen(chunks)=',idx[m]+indgen(chunks)
                                    idx1[l:l+chunks-1]=idx[m]+indgen(chunks)
                                    l=l+chunks
                                endfor
                                ;print,'here5'

                                result[idx1]=result0
                                ;print,'here6'
                            endif
                        endif
                        printf,lu_csh,'nice +19 $BIN' + command + mailstr
                        printf,lu_csh,''
                    endelse
                endif 
                kk = startm
                for k=0,filecount-1 do begin
                    if datatype eq 3 then begin
                        printf,lu_driver,FORMAT='(100(a),:)',files_per_group[j],' ',rtn_within.driver_levels[kk], $
                            rtn_between.driver_levels[n],' ',behav_data_str[j,k]
                    endif else begin
                        for m=0,nframes-1 do begin
                            if nframes eq 1 then $
                                framestr_scrap = '' $
                            else $
                                framestr_scrap=framestr[n,m]+string(9B)
                            p = 0
                            superbird=strtrim(rtn_within.driver_levels[kk],2)+string(9B)+framestr_scrap $
                                +strtrim(rtn_between.driver_levels[n],2)+string(9B)+slkstr
                            for q=0,hem_num_roi-1 do begin
                                for r=0,hemisphere-1 do begin
                                    wallace=subjectstr+hem_region_compound_names[q]+LorR[r]+superbird
                                    if chunks eq 1 then begin
                                        scrap = region_is_a_factor_online_num_roi*(k*nframes+m) + p
                                        scraplabel = strsplit(result[scrap],/EXTRACT)
                                        darby = wallace+scraplabel[n_elements(scraplabel)-1]
                                        if driver ne '' and slklistnomiss[j1] eq 1 then printf,lu_driver,darby
                                        if driverlmernomiss ne '' and slklistnomiss[j1] eq 1 then printf,lu_driverlmernomiss,darby
                                        if driverlmermiss ne '' then printf,lu_driverlmermiss,darby
                                        if ss_driver[j] ne '' and slklistnomiss[j1] eq 1 then printf,ss_lu_driver,darby
                                        p = p + 1
                                    endif else begin
                                        darby=wallace+'CHUNKS'
                                        dummy = chunks*region_is_a_factor_online_num_roi*(k*nframes+m)
                                        if proc eq 1 then begin
                                            if driver ne '' and slklistnomiss[j1] eq 1 then printf,lu_driver,darby
                                            if driverlmernomiss ne '' and slklistnomiss[j1] eq 1 then printf,lu_driverlmernomiss,darby
                                            if driverlmermiss ne '' then printf,lu_driverlmermiss,darby
                                            if ss_driver[j] ne '' and slklistnomiss[j1] eq 1 then printf,ss_lu_driver,darby
                                            for s=0,chunks-1 do begin
                                                scraplabel = strsplit(result[dummy+p],/EXTRACT)
                                                darby=string(scraplabel[n_elements(scraplabel)-1],FORMAT='("          ",a)')
                                                if driver ne '' and slklistnomiss[j1] eq 1 then printf,lu_driver,darby
                                                if driverlmernomiss ne '' and slklistnomiss[j1] eq 1 then $
                                                    printf,lu_driverlmernomiss,darby
                                                if driverlmermiss ne '' then printf,lu_driverlmermiss,darby
                                                if ss_driver[j] ne '' and slklistnomiss[j1] eq 1 then printf,ss_lu_driver,darby
                                                p = p + 1
                                            endfor
                                        endif else begin
                                            if slklistnomiss[j1] eq 1 and use_these0[j] eq 1 then begin
                                                if driver ne '' then begin
                                                    printf,lu_driver,darby
                                                    p0=p
                                                    for s=0,chunks-1 do begin
                                                        scraplabel = strsplit(result[dummy+p0],/EXTRACT)
                                                        wallace=string(scraplabel[n_elements(scraplabel)-1],FORMAT='("          ",a)')
                                                        printf,lu_driver,wallace
                                                        p0=p0+1
                                                    endfor
                                                endif
                                                if ss_driver[j] ne '' then begin
                                                    printf,ss_lu_driver,darby
                                                    p0=p
                                                    for s=0,chunks-1 do begin
                                                        scraplabel = strsplit(result[dummy+p0],/EXTRACT)
                                                        wallace=string(scraplabel[n_elements(scraplabel)-1],FORMAT='("          ",a)')
                                                        printf,ss_lu_driver,wallace
                                                        p0=p0+1
                                                    endfor
                                                endif
                                                if driverlmernomiss[0] ne '' then begin 
                                                    for s=0,proc-1 do printf,lu_driverlmernomiss[s],darby
                                                    p0=p
                                                    for t=0,proc-1 do begin
                                                        for s=0,chunksproc-1 do begin
                                                            scraplabel = strsplit(result[dummy+p0],/EXTRACT)
                                                            wallace=string(scraplabel[n_elements(scraplabel)-1], $
                                                                FORMAT='("          ",a)')
                                                            printf,lu_driverlmernomiss[t],wallace
                                                            p0=p0+1
                                                        endfor
                                                    endfor
                                                endif
                                            endif
                                            if driverlmermiss[0] ne '' then begin 
                                                for s=0,proc-1 do printf,lu_driverlmermiss[s],darby
                                                p0=p
                                                for t=0,proc-1 do begin
                                                    for s=0,chunksproc-1 do begin
                                                        scraplabel = strsplit(result[dummy+p0],/EXTRACT)
                                                        wallace=string(scraplabel[n_elements(scraplabel)-1],FORMAT='("          ",a)')
                                                        printf,lu_driverlmermiss[t],wallace
                                                        p0=p0+1
                                                    endfor
                                                endfor
                                            endif
                                            p=p+chunks
                                        endelse
                                    endelse
                                endfor
                            endfor
                        endfor
                    endelse 
                    kk = kk + 1
                endfor
                if slklistnomiss[j1] eq 1 and use_these0[j] eq 1 then begin
                    if files_are_glms eq 1 then begin
                        idxnomiss=[idxnomiss,jj+indgen(nglm_per_subject[j])]
                        nglmpersubnomiss=[nglmpersubnomiss,nglm_per_subject[j]]
                    endif else begin
                        idxnomiss=[idxnomiss,jj+indgen(rtn_within.treatments[0])]
                        nglmpersubnomiss=[nglmpersubnomiss,rtn_within.treatments[0]]
                    endelse
                    idxnomiss2=[idxnomiss2,j]
                endif
                bottom:
            endif
            if slkn gt 0 then begin
                j1=j1+slknglmpersubject[j2] 
                j2=j2+1
            endif
        endif    
        if ss_driver[j] then begin
            close,ss_lu_driver
            free_lun,ss_lu_driver
        endif
        if files_are_glms eq 1 or lcnglm_per_subject eq 1 then jj=jj+nglm_per_subject[j] else jj=jj+rtn_within.treatments[0]
        j = j + 1 ;increment for use_these 
    endfor ;for i=0,nsubject_per_group[n]-1 do begin
endfor
idxnomiss=idxnomiss[1:*]
idxnomiss2=idxnomiss2[1:*]
endofthis:
if driver ne '' then begin
    close,lu_driver
    free_lun,lu_driver
endif
if driverlmernomiss[0] ne '' then begin
    for i=0,proc-1 do begin
        close,lu_driverlmernomiss[i]
        free_lun,lu_driverlmernomiss[i]
    endfor
endif
if driverlmermiss[0] ne '' then begin
    for i=0,proc-1 do begin
        close,lu_driverlmermiss[i]
        free_lun,lu_driverlmermiss[i]
    endfor
endif
if regionflag eq 1 then begin
    stat=dialog_message_long('ERROR',strjoin(strtrim(regionflagstr[1:*],2),string(13B),/SINGLE))
    iuse_these = where(use_these eq 1,nuse_these)
    print,'use_these=',use_these
    index = where(use_these eq 0,count)
    scrap=get_button(['ok','go back','exit'],TITLE='The following GLMs will be excluded from the analysis because of ' $
        +'missing cells.'+string(10B)+string(10B)+strjoin(subject_names[index],string(10B),/SINGLE))
    if scrap eq 2 then return else if scrap eq 1 then goto,goback4
    if nuse_these eq 1 then begin
        scrap = get_button(['Go back','Exit'],TITLE='You have only one glm. You need at least two.')
        if scrap eq 0 then goto,goback4 else return
    endif
endif

;START170706
if datatype ne 3 then begin
    if files_are_glms eq 1 then begin
        if time eq 0 then begin
            if superspecial ne 'Hz' then begin
                box_str = ' $GLM_FILES $TIME_COURSES'
            endif else if nglm_per_subject[0] eq 1 then begin
                box_str = ' $GLM_FILES $TIME_COURSES'
            endif
        endif else begin
            box_str = ' $GLM_FILES $CONTRASTS'
        endelse
    endif
endif
glmpersubstr1=''
if n_elements(nglmpersubnomiss) gt 1 then nglmpersubnomiss=nglmpersubnomiss[1:*]

;print,'here0 nsubject=',nsubject,' nglmpersubnomiss=',nglmpersubnomiss
;print,'n_elements(nglmpersubnomiss)=',n_elements(nglmpersubnomiss)


fidlbinstr='fidl_anova4'
if datatype eq 3 then GIGAstr=' -GIGA' else if ftest_ss eq 1 then GIGAstr='' else GIGAstr=' -GIGAdesign'
if ss_driver[0] ne '' then begin

    ;print,'here1 bigcstr'
    ;print,bigcstr
    ;print,'n_elements(bigcstr)=',n_elements(bigcstr)

    ;START180417
    if n_elements(ss_dir) ne 0 then $
        superbird=superbird0+ss_dir+'_' $
    else $
        superbird=superbird0

    wallace=files_per_group[idxnomiss]
    ii=0

    ;for i=0,nsubject-1 do begin
    ;START180501
    for i=0,n_elements(nglmpersubnomiss)-1 do begin

        print_files_to_csh,lu_csh,nglmpersubnomiss[i],wallace[ii:ii+nglmpersubnomiss[i]-1],'GLM_FILES','glm_files',/NO_NEWLINE

        ;print_files_to_csh,lu_csh,1,bigcstr[i],'TIME_COURSES','tc',/NO_NEWLINE
        ;START180501
        ;print_files_to_csh,lu_csh,1,bigcstr[idxnomiss[i]],'TIME_COURSES','tc',/NO_NEWLINE
        ;START180504
        print_files_to_csh,lu_csh,1,bigcstr[idxnomiss2[i]],'TIME_COURSES','tc',/NO_NEWLINE


        ;scrap = 'nice +19 $BIN/'+fidlbinstr+ss_driver[i]+anova_threshold_str+region_or_uncompress_str+mode_str $
        ;    +statview_str+correction_str+box_str+anova_output_str+Nimage_name_str+threshold_extent_str+glm_list_str_single $
        ;    +print_cov_matrix_str+scratchdir_str+cellmeansstr+GIGAstr+atlas_str_anova+t4ssstr $
        ;    +' -glmpersub '+trim(nglmpersubnomiss[i])+' -prepend "'+ss_dir[i]+'/'+ss_dir[i]+'_'+'"'
        ;START180417
        ;scrap = 'nice +19 $BIN/'+fidlbinstr+ss_driver[i]+anova_threshold_str+region_or_uncompress_str+mode_str $
        ;    +statview_str+correction_str+box_str+anova_output_str+Nimage_name_str+threshold_extent_str+glm_list_str_single $
        ;    +print_cov_matrix_str+scratchdir_str+cellmeansstr+GIGAstr+atlas_str_anova+t4ssstr $
        ;    +' -glmpersub '+trim(nglmpersubnomiss[i])+' -prepend "'+superbird[i]+'"'
        ;START180501
        ;scrap = 'nice +19 $BIN/'+fidlbinstr+ss_driver[i]+anova_threshold_str+region_or_uncompress_str+mode_str $
        ;    +statview_str+correction_str+box_str+anova_output_str+Nimage_name_str+threshold_extent_str+glm_list_str_single $
        ;    +print_cov_matrix_str+scratchdir_str+cellmeansstr+GIGAstr+atlas_str_anova+t4ssstr $
        ;    +' -glmpersub '+trim(nglmpersubnomiss[i])+' -prepend "'+superbird[idxnomiss[i]]+'"'
        ;START180504
        scrap = 'nice +19 $BIN/'+fidlbinstr+ss_driver[idxnomiss2[i]]+anova_threshold_str+region_or_uncompress_str+mode_str $
            +statview_str+correction_str+box_str+anova_output_str+Nimage_name_str+threshold_extent_str+glm_list_str_single $
            +print_cov_matrix_str+scratchdir_str+cellmeansstr+GIGAstr+atlas_str_anova+t4ssstr $
            +' -glmpersub '+trim(nglmpersubnomiss[i])+' -prepend "'+superbird[idxnomiss2[i]]+'"'


        printf,lu_csh,scrap
        printf,lu_csh,''
        ii=ii+nglmpersubnomiss[i]
    endfor
endif
if datatype ne 3 then begin
    if n_elements(filenameall) gt 1 then begin
        filenameall = filenameall[1:*]
        nfilenameall = n_elements(filenameall)
        if n_elements(t4all) gt 1 then t4all = t4all[1:*]
        if files_are_glms eq 1 and driver ne '' then begin
            print_files_to_csh,lu_csh,n_elements(idxnomiss),files_per_group[idxnomiss],'GLM_FILES','glm_files' 
        endif
        if load eq 1 then begin
            if t4all[0] ne '' then begin
                print,'nfilenameall=',nfilenameall
                print,'filenameall'
                print,filenameall
                print,'t4all'
                print,t4all
                write_list,glm_list_str,[nfilenameall,nfilenameall],[filenameall,t4all]
            endif else begin
                write_list,glm_list_str,nfilenameall,filenameall
            endelse
        endif
    endif
    if files_are_glms eq 1 and driver ne '' then begin
        if time eq 0 then begin
            if superspecial ne 'Hz' then begin

                ;print,'here0 bigcstr'
                ;print,bigcstr
                ;print,'n_elements(bigcstr)=',n_elements(bigcstr)
                ;print,'idxnomiss2=',idxnomiss2
                ;print,'n_elements(idxnomiss2)=',n_elements(idxnomiss2)
                ;print,'bigcstr[idxnomiss2]'
                ;print,bigcstr[idxnomiss2]
                ;print,'n_elements(bigcstr[idxnomiss2])=',n_elements(bigcstr[idxnomiss2])

                print_bigcstr,n_elements(idxnomiss2),bigcstr[idxnomiss2],lu_csh

                ;START170706
                ;box_str = ' $GLM_FILES $TIME_COURSES'

            endif else if nglm_per_subject[0] eq 1 then begin
                printf,lu_csh,'set TIME_COURSES = (-tc \'
                tcHzstr = strtrim(tcHzstr,2)
                for i=0,rtn_within.treatments[0]-2 do printf,lu_csh,strjoin(tcHzstr[*,i],',',/SINGLE)+' \'
                printf,lu_csh,strjoin(tcHzstr[*,i],',',/SINGLE)+')'

                ;START170706
                ;box_str = ' $GLM_FILES $TIME_COURSES'

            endif
        endif else begin
            printf,lu_csh,'set CONTRASTS = (-contrasts \'
            print_contrasts_to_csh,lu_csh,rtn_within.treatments[0],n_elements(idxnomiss2),0, $
                sum_contrast_for_treatment[idxnomiss2,*,*],rtn_within.treatments[0]
            printf,lu_csh,''

            ;START170706
            ;box_str = ' $GLM_FILES $CONTRASTS'

        endelse
    endif

    ;KEEP
    ;if time lt 2 then begin
    ;    if files_are_glms eq 1 then begin
    ;        scrap = strsplit(fidl_anova_csh,'/',/EXTRACT)
    ;        print_cov_matrix_str = ' -print_cov_matrix ' + '/' +strjoin(scrap[0:n_elements(scrap)-2],'/') + '/COV/'  $
    ;            + strmid(scrap[n_elements(scrap)-1],0,strlen(scrap[n_elements(scrap)-1])-4)
    ;    endif
    ;endif
    
    if glm_list_str[0] ne '' then begin
        glm_list_str_single = ' -glm_list_file "'
        glm_list_str_single = glm_list_str_single+strjoin(glm_list_str,' ',/SINGLE)
        glm_list_str_single = glm_list_str_single + '"'
    endif
    if lcregnamestr eq 1 then print_files_to_csh,lu_csh,num_roi,hem_region_names,'REGIONS','regions'
endif

;START170706
;fidlbinstr = 'fidl_anova4'

glmpersubstr1=''
if datatype ne 3 and driver ne '' then begin

    ;START170706
    ;nglmpersubnomiss=nglmpersubnomiss[1:*]

    glmpersubstr1 = ' -glmpersub '+strjoin(trim(nglmpersubnomiss),' ',/SINGLE)
endif

;print,'n_elements(fidlbinstr)=',n_elements(fidlbinstr)
;print,'n_elements(driver)=',n_elements(driver)
;print,'n_elements(anova_threshold_str)=',n_elements(anova_threshold_str)
;print,'n_elements(region_or_uncompress_str)=',n_elements(region_or_uncompress_str)
;print,'n_elements(mode_str)=',n_elements(mode_str)
;print,'n_elements(statview_str)=',n_elements(statview_str)
;print,'n_elements(correction_str)=',n_elements(correction_str)
;print,'n_elements(box_str)=',n_elements(box_str)
;print,'n_elements(num_delays_str)=',n_elements(num_delays_str)
;print,'n_elements(anova_output_str)=',n_elements(anova_output_str)
;print,'n_elements(Nimage_name_str)=',n_elements(Nimage_name_str)
;print,'n_elements(threshold_extent_str)=',n_elements(threshold_extent_str)
;print,'n_elements(glm_list_str_single)=',n_elements(glm_list_str_single)
;print,'n_elements(print_cov_matrix_str)=',n_elements(print_cov_matrix_str)
;print,'n_elements(scratchdir_str)=',n_elements(scratchdir_str)
;print,'n_elements(cellmeansstr)=',n_elements(cellmeansstr)
;print,'n_elements(GIGAstr)=',n_elements(GIGAstr)
;print,'n_elements(atlas_str_anova)=',n_elements(atlas_str_anova)
;print,'n_elements(glmpersubstr)=',n_elements(glmpersubstr)

;START170706
;if datatype eq 3 then GIGAstr=' -GIGA' else if ftest_ss eq 1 then GIGAstr='' else GIGAstr=' -GIGAdesign'

;STARTHERE
if driver ne '' then begin
    scrap = 'nice +19 $BIN/'+fidlbinstr+driver+anova_threshold_str+region_or_uncompress_str+mode_str $
        +statview_str+correction_str+box_str+anova_output_str+Nimage_name_str+threshold_extent_str+glm_list_str_single $
        +print_cov_matrix_str+scratchdir_str+cellmeansstr+GIGAstr+atlas_str_anova+glmpersubstr1+t4ssstr
    if not keyword_set(LMERR) then scrap=scrap+clean_up_str else scrap=scrap+' -directory '+lmeranovadir+'/'
    printf,lu_csh,scrap
    printf,lu_csh,''
endif
if keyword_set(LMERR) then begin
    if proc eq 1 then begin
        if driverlmernomiss[0] ne '' then begin

            ;scrap = 'nice +19 $BIN/'+fidlbinstr+driverlmernomiss+anova_threshold_str+region_or_uncompress_str $
            ;    +mode_str_lmer+threshold_extent_str+glm_list_str_single $
            ;    +print_cov_matrix_str+scratchdir_str+atlas_str_anova+t4ssstr+' -lmerTest'+lmeoutstr $
            ;    +' -directory '+lmernomissdir
            ;START161027
            scrap = 'nice +19 $BIN/'+fidlbinstr+driverlmernomiss+anova_threshold_str+region_or_uncompress_str $
                +mode_str_lmer+threshold_extent_str+glm_list_str_single $
                +print_cov_matrix_str+scratchdir_str+atlas_str_anova+t4ssstr+' -lmerTest'+lmeoutstr $
                +' -directory '+lmernomissdir+'/'

            if driverlmermiss[0] eq '' then scrap=scrap+clean_up_str
            printf,lu_csh,scrap
            printf,lu_csh,''
        endif
        if driverlmermiss[0] ne '' then begin

            ;scrap = 'nice +19 $BIN/'+fidlbinstr+driverlmermiss+anova_threshold_str+region_or_uncompress_str $
            ;    +mode_str_lmer+threshold_extent_str+glm_list_str_single $
            ;    +print_cov_matrix_str+scratchdir_str+atlas_str_anova+t4ssstr+' -lmerTest'+lmeoutstr $
            ;    +' -directory '+lmermissdir+clean_up_str
            ;START161027
            scrap = 'nice +19 $BIN/'+fidlbinstr+driverlmermiss+anova_threshold_str+region_or_uncompress_str $
                +mode_str_lmer+threshold_extent_str+glm_list_str_single $
                +print_cov_matrix_str+scratchdir_str+atlas_str_anova+t4ssstr+' -lmerTest'+lmeoutstr $
                +' -directory '+lmermissdir+'/'+clean_up_str

            printf,lu_csh,scrap
            printf,lu_csh,''
        endif
    endif else if driverlmernomiss[0] ne '' or driverlmermiss[0] ne '' then begin
        if driverlmernomiss[0] ne '' then begin
            scratchnomiss=scratchdir+lmernomissdir
            spawn,'mkdir '+scratchnomiss

            ;cmd=!BINEXECUTE+'/'+fidlbinstr+driverlmernomiss[0]+scratchdir_str+' -directory '+scratchnomiss+' -lmerTest' $
            ;    +' -output PROC'+wallace0[0]+' -names_only'
            ;START161027
            cmd=!BINEXECUTE+'/'+fidlbinstr+driverlmernomiss[0]+scratchdir_str+' -directory '+scratchnomiss+'/ -lmerTest' $
                +' -output PROC'+wallace0[0]+' -names_only'

            print,cmd
            spawn,cmd,result,errresult
            if n_elements(errresult) gt 0 then begin
                print,'n_elements(errresult)=',n_elements(errresult)
                print,'errresult=',errresult
                idx=where(strcmp(errresult,'Warning:',8) eq 1,cnt)
                if cnt ne 0 then begin
                    idx1=where(strcmp(errresult[idx],errresult[idx[0]]) eq 1,cnt1,COMPLEMENT=idx2)
                    title=errresult[idx[0]]
                    if cnt ne cnt1 then title=[title,errresult[idx[idx2]]]
                    if action eq 0 then labels=['Ok, execute anyway','Return','Go back'] else labels=['Ok','Go back']
                    scrap=get_button(labels,BASE_TITLE='R does not like your design',TITLE=title)
                    if action eq 0 then begin
                        if scrap eq 1 then action=1 else if scrap eq 2 then goto,goback12
                    endif else begin
                        if scrap eq 1 then goto,goback12
                    endelse
                endif

            endif


            idx=where(strpos(result,'fidlError') ne -1,cnt)
            if cnt ne 0 then begin
                stat=dialog_message_long('ERROR',result)
                return
            endif
            idx=where(strpos(result,'fidlWarning') eq -1,cnt)
            if cnt ne 0 then result=result[idx] 
            nomissf01=strarr(n_elements(result))
            for i=0,n_elements(result)-1 do begin
                scraplabel=strsplit(result[i],/EXTRACT)
                nomissf01[i]=scraplabel[n_elements(scraplabel)-1]
            endfor
            nomisspid=strarr(proc)
            for i=0,proc-1 do begin
                nomisspid[i]=scratchnomiss+'/PROC'+wallace0[i]+'.pid'

                ;scrap='nice +19 $BIN/'+fidlbinstr+driverlmernomiss[i]+scratchdir_str+' -directory '+scratchnomiss+' -lmerTest' $
                ;    +' -output PROC'+wallace0[i]+anova_threshold_str+' >& '+scratchnomiss+'/PROC'+wallace0[i]+'.log & echo $! >& ' $
                ;    +nomisspid[i]
                ;START161027
                scrap='nice +19 $BIN/'+fidlbinstr+driverlmernomiss[i]+scratchdir_str+' -directory '+scratchnomiss+'/ -lmerTest' $
                    +' -output PROC'+wallace0[i]+anova_threshold_str+' >& '+scratchnomiss+'/PROC'+wallace0[i]+'.log & echo $! >& ' $
                    +nomisspid[i]

                printf,lu_csh,scrap
                printf,lu_csh,''
            endfor
        endif
        if driverlmermiss[0] ne '' then begin
            scratchmiss=scratchdir+lmermissdir
            spawn,'mkdir '+scratchmiss

            ;cmd=!BINEXECUTE+'/'+fidlbinstr+driverlmermiss[0]+scratchdir_str+' -directory '+scratchmiss+' -lmerTest' $
            ;    +' -output PROC'+wallace0[0]+' -names_only'
            ;START161027
            cmd=!BINEXECUTE+'/'+fidlbinstr+driverlmermiss[0]+scratchdir_str+' -directory '+scratchmiss+'/ -lmerTest' $
                +' -output PROC'+wallace0[0]+' -names_only'

            print,cmd
            spawn,cmd,result
            idx=where(strpos(result,'fidlError') ne -1,cnt)
            if cnt ne 0 then begin
                stat=dialog_message_long('ERROR',result)
                return
            endif
            missf01=strarr(n_elements(result))
            for i=0,n_elements(result)-1 do begin
                scraplabel=strsplit(result[i],/EXTRACT)
                missf01[i]=scraplabel[n_elements(scraplabel)-1]
            endfor
            misspid=strarr(proc)
            for i=0,proc-1 do begin
                misspid[i]=scratchmiss+'/PROC'+wallace0[i]+'.pid'

                ;scrap='nice +19 $BIN/'+fidlbinstr+driverlmermiss[i]+scratchdir_str+' -directory '+scratchmiss+' -lmerTest' $
                ;    +' -output PROC'+wallace0[i]+anova_threshold_str+' >& '+scratchmiss+'PROC'+wallace0[i]+'.log & echo $! >& ' $
                ;    +misspid[i]
                ;START161027
                scrap='nice +19 $BIN/'+fidlbinstr+driverlmermiss[i]+scratchdir_str+' -directory '+scratchmiss+'/ -lmerTest' $
                    +' -output PROC'+wallace0[i]+anova_threshold_str+' >& '+scratchmiss+'/PROC'+wallace0[i]+'.log & echo $! >& ' $
                    +misspid[i]

                printf,lu_csh,scrap
                printf,lu_csh,''
            endfor
        endif
        if driverlmernomiss[0] ne '' then begin
            print_files_to_csh,lu_csh,n_elements(nomissf01),nomissf01,'FILES01','files01',/NO_NEWLINE
            print_files_to_csh,lu_csh,n_elements(nomisspid),nomisspid,'PID','pid',/NO_NEWLINE

            ;cmd='nice +19 $BIN/fidl_chunkstack $FILES01 $PID -proc '+trim(proc)+' -mask '+mask $
            ;    +threshold_extent_str+glm_list_str_single+' -directory '+lmernomissdir+' -scratchdir '+scratchdir
            ;START161027
            cmd='nice +19 $BIN/fidl_chunkstack $FILES01 $PID -proc '+trim(proc)+' -mask '+mask $
                +threshold_extent_str+glm_list_str_single+' -directory '+lmernomissdir+'/ -scratchdir '+scratchdir

            if files_are_glms eq 1 then cmd=cmd+' -glm '+files_per_group[idxnomiss[0]]

            ;START161111
            ;if driverlmermiss[0] eq '' then cmd=cmd+' -cleanup'

            printf,lu_csh,cmd
            printf,lu_csh,''
        endif
        if driverlmermiss[0] ne '' then begin
            print_files_to_csh,lu_csh,n_elements(missf01),missf01,'FILES01','files01',/NO_NEWLINE
            print_files_to_csh,lu_csh,n_elements(misspid),misspid,'PID','pid',/NO_NEWLINE

            ;cmd='nice +19 $BIN/fidl_chunkstack $FILES01 $PID -proc '+trim(proc)+' -mask '+mask $
            ;    +threshold_extent_str+glm_list_str_single+' -directory '+lmermissdir+' -scratchdir '+scratchdir
            ;START161027
            cmd='nice +19 $BIN/fidl_chunkstack $FILES01 $PID -proc '+trim(proc)+' -mask '+mask $
                +threshold_extent_str+glm_list_str_single+' -directory '+lmermissdir+'/ -scratchdir '+scratchdir

            if files_are_glms eq 1 then cmd=cmd+' -glm '+files_per_group[idxnomiss[0]]

            ;START161111
            ;cmd=cmd+' -cleanup'

            printf,lu_csh,cmd
            printf,lu_csh,''
        endif


    endif

endif
if slkfile ne '' then printf,lu_csh,'#'+slkfile
if datatype ne 3 then begin
    if gg.glm_list[0] ne '' then for i=0,n_elements(gg.glm_list)-1 do printf,lu_csh,'#'+gg.glm_list[i]
endif
if specialstr ne '' then begin
    printf,lu_csh,'#'+specialstr
endif
close,lu_csh
free_lun,lu_csh
spawn,'chmod +x '+fidl_anova_csh
if compute_avg_zstat_csh_str ne '' and not keyword_set(LMERR) and slkn eq 0 then begin
    subject_namesall = subject_namesall[1:*]
    framesstr = ''
    if time eq 0 then $
        framesstr=' -frames '+strjoin(make_array(rtn_within.treatments[0],/STRING,VALUE=strtrim(nframes,2)),' ',/SINGLE)
    openw,lu_avg,compute_avg_zstat_csh_str,/GET_LUN
    top_of_script,lu_avg
    avgstatstr = ' $GLM_FILES'
    if t4all[0] ne '' then avgstatstr = avgstatstr + ' $T4_FILES'
    if region_file[0] ne '' then begin
        avgstatstr = avgstatstr + ' $ROOTS'
        print_files_to_csh,lu_avg,n_elements(region_file),region_file,'REGION_FILE','region_file'
        avgstatstr = avgstatstr + ' $REGION_FILE'
    end
    if time eq 0 then begin
        bigcstr = bigcstr[iuse_these]
        avgstatstr = avgstatstr + ' $TIME_COURSES'
    endif else begin
        sum_contrast_for_treatment = sum_contrast_for_treatment[iuse_these,*,*]
        avgstatstr = avgstatstr + ' $CONTRASTS'
    endelse
    index = where(roi_str eq '',count)
    if count eq total_nsubjects then roi_str = ''
    roi_str = roi_str[0]
    n1=0
    n2=0
    for n=0,rtn_between.treatments[0]-1 do begin
        print_files_to_csh,lu_avg,nfpg_use_these[n],filenameall[n1:n1+nfpg_use_these[n]-1],'GLM_FILES','glm_files',/NO_NEWLINE
        if t4all[0] ne '' then $
            print_files_to_csh,lu_avg,nfpg_use_these[n],t4all[n1:n1+nfpg_use_these[n]-1],'T4_FILES','xform_files',/NO_NEWLINE
        if region_file[0] ne '' then $
            print_files_to_csh,lu_avg,nsubject_namesall[n],subject_namesall[n2:n2+nsubject_namesall[n]-1],'ROOTS','roots', $
                /NO_NEWLINE
        if time eq 0 then begin
            print_bigcstr,nsubject_namesall[n],bigcstr[n2:n2+nsubject_namesall[n]-1],lu_avg
        endif else begin
            printf,lu_avg,'set CONTRASTS = (-contrasts \'
            print_contrasts_to_csh,lu_avg,rtn_within.treatments[0],nsubject_namesall[n],0, $
                sum_contrast_for_treatment[n2:n2+nsubject_namesall[n]-1,*,*],rtn_within.treatments[0]
        endelse

        ;scrap = 'nice +19 $BIN/fidl_avg_zstat -sem'+avgstatstr+cols_or_rows_str+roi_str+avgstat_output_str[n] $
        ;    +' -glmpersub '+strjoin(glmpersubstr[n2:n2+nsubject_namesall[n]-1],' ',/SINGLE)+framesstr
        ;START160803
        ;scrap = 'nice +19 $BIN/fidl_avg_zstat -sem'+avgstatstr+cols_or_rows_str+roi_str+avgstat_output_str[n] $
        ;    +' -glmpersub '+strjoin(trim(nglmpersubnomiss[n2:n2+nsubject_namesall[n]-1]),' ',/SINGLE)+framesstr
        ;START161116
        scrap = 'nice +19 $BIN/fidl_avg_zstat2 -sem'+avgstatstr+cols_or_rows_str+roi_str+avgstat_output_str[n] $
            +' -glmpersub '+strjoin(trim(nglmpersubnomiss[n2:n2+nsubject_namesall[n]-1]),' ',/SINGLE)+framesstr


        printf,lu_avg,scrap
        printf,lu_avg,''
        n1 = n1 + nfpg_use_these[n]
        n2 = n2 + nsubject_namesall[n]
    endfor
    close,lu_avg
    free_lun,lu_avg
    spawn,'chmod +x '+compute_avg_zstat_csh_str
endif
if action eq 0 then begin
    spawn,fidl_anova_csh+'>'+fidl_anova_csh+'.log'+' &'
    stats = dialog_message(fidl_anova_csh+string(10B)+string(10B)+'Script has been executed.',/INFORMATION)
endif
print,'DONE'
end
