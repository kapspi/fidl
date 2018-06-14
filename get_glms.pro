;Copyright 12/19/01 Washington University.  All Rights Reserved.
;get_glms.pro  $Revision: 1.136 $

;function get_glms,pref,fi,help,GET_CONC=get_conc,BASE_TITLE=base_title,GET_THIS=get_this,MAGICWORD=magicword,GROUPNAMES=groupnames,$
;    PAIRED=paired,FILTER=filter,GLM_LIST=glm_list,NOTMORETHANONE=notmorethanone,LEVELNAMES=levelnames,HOWMANY=howmany,$
;    NOTALK=notalk
;START180430
function get_glms,pref,fi,help,GET_CONC=get_conc,BASE_TITLE=base_title,GET_THIS=get_this,MAGICWORD=magicword,GROUPNAMES=groupnames,$
    PAIRED=paired,FILTER=filter,GLM_LIST=glm_list,NOTMORETHANONE=notmorethanone,LEVELNAMES=levelnames,HOWMANY=howmany,$
    NOTALK=notalk,QUIET=quiet


directory=getenv('PWD')
if not keyword_set(MAGICWORD) then magicword = '' else magicword = magicword + ' '

;if not keyword_set(BASE_TITLE) then base_title = ''
;START170206
if not keyword_set(BASE_TITLE) then base_title=0 

if not keyword_set(GLM_LIST) then glm_list0='' else glm_list0=glm_list
if not keyword_set(HOWMANY) then howmany=0
if not keyword_set(GET_THIS) then get_this = ''
if keyword_set(LEVELNAMES) then levelnames0 = levelnames
if not keyword_set(NOTALK) then notalk=0

;START180430
if not keyword_set(QUIET) then quiet=0

goback0:
nlists=0
t4select = ''
glmlist = ''
concselect = ''
evselect = ''
front = ''
t4files=''
evs=''
txt=''
txtselect = ''
img=''
imgselect='' 
vals=''
ext=''
extselect=''
identify=''
identifyselect=''
glm_space=-1
bold_space=-1
nlists0=1
morethanone=0
nglm=0
ifh=0
twolists=0
msg=''
total_nsubjects=0
slkfile=''
slkdata=''
slkcollab=''
slkid=-1

;START160819
replace=''
replaceselect=''

if n_elements(glmfiles) ne 0 then undefine,glmfiles
title1=0
extratitle1=0
if glm_list0 ne '' then begin
    load = 0
    rtn_nfiles = 1
endif else if keyword_set(GET_THIS) then begin
    scrap0 = get_this
    if not keyword_set(FILTER) then begin
        if get_this eq 'concs/imgs/glms' then filter='*.conc' $
        else if get_this eq 'GLMs/4dfps' or get_this eq 'GLMs/4dfps/*nii/*nii.gz' then begin

            ;filter='*.glm' 
            ;title1 = 'Please specify the proper filter: *.glm or *.4dfp.img.'
            ;START180316
            filter=['*.glm','*.img','*nii','*nii.gz'] 
            title1 = 'Please specify the proper filter: *.glm, *.img, *nii, *nii.gz.'

            extratitle1 = 'You may include a path.'
        endif else if get_this eq 'imgs' then filter='*.4dfp.img' $
        else if get_this eq 'imgs/concs' then begin
            filter='*.4dfp.img'
            title1 = 'Please specify the proper filter: *.4dfp.img or *.conc'
        endif else filter='*.glm'
    endif
endif else if keyword_set(GET_CONC) then begin
    scrap0 = '*.conc, *.img or *.nii'
    filter = '*.conc'
    front = pref.concpath
endif else begin
    if keyword_set(FILTER) then $
        scrap0 = '' $
    else begin
        scrap0 = 'GLMs'
        filter = '*.glm'
    endelse
endelse
if glm_list0 eq '' then begin
    load = get_button(['list','individual files','go back','exit'],TITLE='Please load '+scrap0,BASE_TITLE=base_title,WIDTH=200)
    if load eq 2 then return,rtn={msg:'GO_BACK'} else if load eq 3 then return,rtn={msg:'EXIT'}
endif
goback1:
nlists1=0
lcallatonce=0
imgselecti=-1
if load eq 1 then begin
    glmfiles=''
    if keyword_set(GROUPNAMES) then begin
        scrap = get_button(['Load files separately for each group or session','Load them all at once','Go back'],TITLE='I want to') 
        if scrap eq 2 then goto,goback0 else if scrap eq 0 then nlists0=n_elements(groupnames)
        if scrap eq 1 then lcallatonce=1
    endif else if keyword_set(LEVELNAMES) then begin
        scrap = get_button(['Load files separately for each factor level','Load them all at once (select this for glms)','Go back'], $
            TITLE='I want to') 
        if scrap eq 2 then goto,goback0 else if scrap eq 0 then nlists0=n_elements(levelnames)

        ;if scrap eq 1 then lcallatonce=1
        ;START180413
        if scrap eq 1 then begin
            lcallatonce=1
            filter = '*.glm'
        endif

    endif else if get_this eq 'imgs' then begin
        get_filelist_labels,fi,lab_imgn,lab_img,lab_imgi,lab_glmn,lab_glm,lab_glmi,lab_concn,lab_conc,lab_conci,/NAMES
        if lab_imgn ne 0 then begin

            ;rtn = select_files([lab_img,'On disk'],TITLE='Please select files',/CANCEL,/EXIT,/ONE_COLUMN,/GO_BACK)
            ;if rtn.files[0] eq 'GO_BACK' then goto,goback0 else if rtn.files[0] eq 'EXIT' then return,rtn={msg:'EXIT'}
            ;cnt=rtn.count
            ;if rtn.list[lab_imgn] eq 0 then nlists0=0 else cnt=cnt-1
            ;if total(rtn.list[0:lab_imgn-1]) ge 1 then begin
            ;    glmfiles=[glmfiles,rtn.files[0:cnt-1]]
            ;    nglm[0]=cnt
            ;    imgselecti=[imgselecti,rtn.index[where(rtn.index lt lab_imgn)]]
            ;endif
            ;START170203
            rtn = select_files(['On disk',lab_img],TITLE='Please select files',/CANCEL,/EXIT,/ONE_COLUMN,/GO_BACK,/NO_ALL, $
                BASE_TITLE=base_title)
            if rtn.files[0] eq 'GO_BACK' then goto,goback0 else if rtn.files[0] eq 'EXIT' then return,rtn={msg:'EXIT'}
            cnt=rtn.count
            if rtn.list[0] eq 0 then nlists0=0 else cnt=cnt-1
            if total(rtn.list[1:lab_imgn]) ge 1 then begin
                if rtn.list[0] eq 0 then $
                    glmfiles=[glmfiles,rtn.files[0:rtn.count-1]] $
                else $
                    glmfiles=[glmfiles,rtn.files[1:rtn.count]] 
                nglm[0]=cnt
                imgselecti=[imgselecti,rtn.index-1]
            endif

        endif
    endif
    if nlists0 gt 0 then begin
        nglm = intarr(nlists0)
        nlists1 = intarr(nlists0)
        msg = make_array(nlists0,/STRING,VALUE='')
        asked=0
        for j=0,nlists0-1 do begin
            goback1b:
            if keyword_set(GROUPNAMES) and nlists0 gt 1 then begin
                label = 'Please load files for '+groupnames[j]
                dialogextratitle = groupnames[j]
            endif else if keyword_set(LEVELNAMES) and nlists0 gt 1 then begin
                label = 'Please load files for '+levelnames[j]
                dialogextratitle = levelnames[j]
            endif else begin
                label = 'Please load files'
                dialogextratitle = ''
            endelse
            gf = get_files(filter,FRONT=front,TITLE=title1,HOWMANY=howmany,LABEL=label,EXTRATITLE=extratitle1, $
                DIALOGEXTRATITLE=dialogextratitle)
            if gf.msg eq 'EXIT' then $
                return,rtn={msg:'EXIT'} $
            else if gf.msg eq 'GO_BACK' then begin
                if keyword_set(GROUPNAMES) then goto,goback1 else goto,goback0
            endif
            dummy = strmid(gf.files[0],strlen(gf.files[0])-3)
            if gf.msg eq 'DONTCHECK' then begin

                ;if keyword_set(LEVELNAMES) then begin
                ;START170331
                if keyword_set(LEVELNAMES) and nlists0 gt 1 then begin

                    if nlists0 eq n_elements(levelnames) then msg[j]='DONTCHECK' else nlists1[j]=1
                endif else $
                    msg[j]='DONTCHECK'
                if asked eq 0 and n_elements(gf.files) gt 1 and dummy eq 'glm' and not keyword_set(NOTMORETHANONE) then begin
                    scrap = get_button(['yes','no','go back'],TITLE='Do you have more than one glm per subject?')
                    if scrap eq 2 then goto,goback1b
                    morethanone=1-scrap
                    asked=1
                endif
            endif else begin 
                nlists1[j]=1
            endelse
            glmfiles=[glmfiles,gf.files]
            nglm[j]=n_elements(gf.files)

            ;imgselecti=[imgselecti,make_array(nglm[j],/INTEGER,VALUE=-1)]
            ;START161215
            imgselecti=[imgselecti,make_array(nglm[j],/INTEGER,VALUE=j)]

        endfor
    endif
    glmfiles=glmfiles[1:*]
endif else if load eq 0 then begin
    ask=1
    ngroup=1
    nlevel=1
    neither=0
    s2=0
    btitlesf=0
    if keyword_set(GROUPNAMES) and keyword_set(LEVELNAMES) then begin
        scrap = get_button(['a separate list for each group','a separate list for each factor level','both','neither', $
            'go back'],TITLE='I have')

        ;if scrap eq 4 then goto,goback0a
        ;START160617
        if scrap eq 4 then goto,goback0

        ngroup = n_elements(groupnames)
        nlevel = n_elements(levelnames)
        if scrap eq 3 then begin
            if n_elements(groupnames) ne 0 then begin
                neither=1
                s2='Ok - load slk'
                btitlesf="Hit 'Ok - load slk' after selecting all files to be analyzed (not just for this group) to assign " $
                    +'group membership from a slk'
            endif
            ask=n_elements(groupnames)
            twolists=0
            undefine,levelnames0
        endif else if scrap eq 2 then begin
            ask = n_elements(groupnames)+n_elements(levelnames)
            twolists = 1
        endif else if scrap eq 1 then begin
            ask = n_elements(levelnames)
            twolists = 1
        endif else if scrap eq 0 then begin
            ask = n_elements(groupnames)
            twolists = 1
            undefine,levelnames0
        endif
    endif else if keyword_set(GROUPNAMES) then begin
        scrap = get_button(['yes','no','go back'],TITLE='Do you have a separate list for each group?')

        ;if scrap eq 2 then goto,goback0a else if scrap eq 0 then ask=n_elements(groupnames)
        ;START160617
        if scrap eq 2 then goto,goback0 else if scrap eq 0 then ask=n_elements(groupnames)

        ngroup=n_elements(groupnames)
        twolists = abs(scrap-1)
    endif else if keyword_set(LEVELNAMES) then begin
        scrap = get_button(['yes','no','go back'],TITLE='Do you have a separate list for each factor level?')

        ;if scrap eq 2 then goto,goback0a else if scrap eq 0 then ask=n_elements(levelnames)
        ;START160617
        if scrap eq 2 then goto,goback0 else if scrap eq 0 then ask=n_elements(levelnames)

        nlevel=n_elements(levelnames)
        twolists=abs(scrap-1)
    endif
    if twolists ne 0 then begin
        title1 = 'Please select list for'
    endif else begin
        title1 = 'Please select '+magicword+' list.'
    endelse
    goback1a:

    ;nglm = intarr(ngroup)
    ;START161220
    nglm=intarr(ngroup*nlevel)

    nlists = intarr(ask)
    glmlist = ''
    glmfiles = ''
    t4files = ''
    concs = ''
    evs = ''
    txt = ''
    vals = ''
    ext = ''
    img = ''
    identify=''

    ;START160819
    replace=''

    path = '*.*list'
    rtn_path = ''
    k = 0
    i = 0
    repeat begin 
        for j=0,ngroup-1 do begin
            title = title1
            subtitle = ''
            if keyword_set(GROUPNAMES) then subtitle = subtitle + ' ' + groupnames[j]
            if n_elements(levelnames0) ne 0 and twolists ne 0 then subtitle = subtitle + ' ' + levelnames[i]
            if subtitle ne '' and neither eq 0 then begin
                title = title + subtitle
            endif
            if k lt ask then begin
                lc1a=0
                if glm_list0[0] eq '' or twolists ne 0 then begin
                    if twolists ne 0 then saf='Do you wish to select another list for'+subtitle+'?' else saf=0
                    get_dialog_pickfile,rtn_path+path,directory,title,glm_list0,rtn_nfiles,rtn_path,/MULTIPLE_FILES,SAF=saf
                    if rtn_nfiles eq 0 then begin
                        glm_list0 = ''

                        ;if keyword_set(GROUPNAMES) then goto,goback1 else goto,goback0a
                        ;START160617
                        if keyword_set(GROUPNAMES) then goto,goback1 else goto,goback0

                    endif
                    lc1a=1
                endif
                glmlist = [glmlist,glm_list0]
                t4files0=''
                glmfiles0=''
                concs0=''
                img0=''
                evs0=''
                txt0=''
                vals0=''
                ext0=''
                identify0=''

                ;START160819
                replace0=''

                for l=0,rtn_nfiles-1 do begin
                    print,'Reading '+glm_list0[l]
                    rl = read_list(glm_list0[l])

                    ;if rl.msg eq 'EXIT' then return,rtn={msg:'ERROR'} else if rl.msg eq 'GOBACK' then goto,goback1a
                    ;START170216
                    if rl.msg eq 'EXIT' then return,rtn={msg:'EXIT'} else if rl.msg eq 'GOBACK' then goto,goback1a

                    if rl.nt4 gt 0 then t4files0 = [t4files0,rl.t4]
                    if rl.nglm gt 0 then glmfiles0 = [glmfiles0,rl.glm]
                    if rl.nconc gt 0 then concs0 = [concs0,rl.conc]
                    if rl.nimg gt 0 then img0 = [img0,rl.img]
                    if rl.nev gt 0 then evs0 = [evs0,rl.ev]
                    if rl.ntxt gt 0 then txt0 = [txt0,rl.txt]
                    if rl.nvals gt 0 then vals0 = [vals0,rl.vals]
                    if rl.next gt 0 then ext0 = [ext0,rl.ext]
                    if rl.nidentify gt 0 then identify0 = [identify0,rl.identify]

                    ;START160819
                    if rl.nreplace gt 0 then replace0 = [replace0,rl.replace]

                endfor
                wallace=''
                special=0
                if n_elements(evs0) gt 1 then evs0 = evs0[1:*]
                if n_elements(txt0) gt 1 then txt0 = txt0[1:*]
                if keyword_set(GET_CONC) and n_elements(concs0) eq 1 then begin

                    ;concselect = glm_list[0]
                    ;START151106
                    concselect = glm_list0[0]

                    total_nsubjects=1
                    goto,goback10
                endif
                if n_elements(concs0) gt 1 then begin
                    concs0 = concs0[1:*]
                    wallace = concs0
                endif
                if n_elements(t4files0) gt 1 then t4files0 = t4files0[1:*]
                if n_elements(glmfiles0) gt 1 then begin ;must be after concs
                    glmfiles0 = glmfiles0[1:*]
                    wallace = glmfiles0
                endif
                if n_elements(img0) gt 1 then begin
                    img0 = img0[1:*]
                    if wallace[0] eq '' then wallace=img0
                endif
                if n_elements(vals0) gt 1 then begin
                    vals0 = vals0[1:*]
                    if wallace[0] eq '' then wallace=vals0
                endif
                if n_elements(ext0) gt 1 then begin
                    ext0 = ext0[1:*]
                    if wallace[0] eq '' then wallace=ext0
                endif
                if n_elements(identify0) gt 1 then identify0 = identify0[1:*]

                ;START160819
                if n_elements(replace0) gt 1 then replace0 = replace0[1:*]

                idx0 = indgen(n_elements(wallace))
                list0 = intarr(n_elements(wallace))
            endif
            if wallace[0] ne '' then begin
                goback2:
                if notalk eq 1 then begin
                    rtn={count:n_elements(wallace[idx0]),index:indgen(n_elements(wallace[idx0])),s2:0}
                endif else begin
                    stitle = 'Please select files'
                    if subtitle ne '' then stitle = stitle + ' for' + subtitle
                    dummy = strmid(wallace[idx0[0]],strlen(wallace[idx0[0]])-3)
                    if dummy eq 'glm' and not keyword_set(NOTMORETHANONE) then special = 'OK - More than one glm per subject'
                    rtn = select_files(wallace[idx0],TITLE=stitle,/CANCEL,/EXIT,/ONE_COLUMN,/GO_BACK,SPECIAL=special,S2=s2, $
                        BASE_TITLE=btitlesf)
                    if rtn.files[0] eq 'GO_BACK' then begin
                        if lc1a eq 1 then glm_list0=''
                        goto,goback1a
                    endif else if rtn.files[0] eq 'EXIT' then return,rtn={msg:'EXIT'}
                    morethanone=rtn.special
                endelse


                ;if i eq 0 then $
                ;    nglm[j] = rtn.count $
                ;else if rtn.count ne nglm[j] then begin
                ;    darby = get_button(['You have selected '+trim(rtn.count)+' files. You must select '+trim(nglm[j])+'.', $
                ;        'Go back','Exit'],TITLE='Number of files within a group must match.',BASE_TITLE='Big mistake',WIDTH=200)
                ;    if darby eq 1 then goto,goback2 else if darby eq 2 then return,rtn={msg:'EXIT'}
                ;endif
                ;START161220
                nglm[k]=rtn.count
                imgselecti=[imgselecti,make_array(nglm[k],/INTEGER,VALUE=k)]

                if glmfiles0[0] ne '' then glmfiles = [glmfiles,glmfiles0[idx0[rtn.index]]]
                if concs0[0] ne '' then begin
                    concs = [concs,concs0[idx0[rtn.index]]]
                    if img0[0] ne '' then begin
                        nec0 = n_elements(concs0)
                        loop = n_elements(img0)/nec0
                        for l=0,loop-1 do img = [img,img0[l*nec0+idx0[rtn.index]]]
                        undefine,nec0,loop
                    endif
                endif else if img0[0] ne '' then $
                    img = [img,img0[idx0[rtn.index]]]
                if evs0[0] ne '' then evs = [evs,evs0[idx0[rtn.index]]]
                if txt0[0] ne '' then txt = [txt,txt0[idx0[rtn.index]]]
                if vals0[0] ne '' then vals = [vals,vals0[idx0[rtn.index]]]
                if ext0[0] ne '' then ext = [ext,ext0[idx0[rtn.index]]]
                if t4files0[0] ne '' then t4files = [t4files,t4files0[idx0[rtn.index]]]
                if identify0[0] ne '' then identify = [identify,identify0[idx0[rtn.index]]]
                if replace0[0] ne '' then replace = [replace,replace0[idx0[rtn.index]]]
                list0[idx0[rtn.index]] = 1
                idx0 = where(list0 eq 0,cnt)
                if rtn.s2 ne 0 then begin
                    gs=get_slk(help,'','NA')
                    if gs.msg eq 'GO_BACK' then goto,goback2 else if gs.msg eq 'EXIT' then return,rtn={msg:'EXIT'}
                    slkfile=gs.slkfile
                    if n_elements(t4files) gt 1 then t4files = t4files[1:*]
                    am=automatch(glmfiles[1:*],t4files,gs.subject_id,slkfile,gs.nbehav_pts,gs.behav_data,grptag,gs.column_labels, $
                       /DONTSELECT)
                    if am.msg eq 'EXIT' then return,rtn={msg:'EXIT'} else if am.msg eq 'GO_BACK' then goto,goback2
                    idx1=-1
                    col=intarr(gs.ncolumn_labels)
                    for l=0,n_elements(groupnames)-1 do begin
                        ribbet=strsplit(groupnames[l],'.',/EXTRACT)
                        if n_elements(ribbet) ne 2 then begin
                            darby = get_button(['Go back','Exit'],TITLE='Currently only set up to handle 2 strings. See McAvoy.', $
                               BASE_TITLE='This capability needs to be added')
                            if darby eq 1 then goto,goback2 else if darby eq 2 then return,rtn={msg:'EXIT'}
                        endif
                        idx=where(ribbet[0] eq gs.column_labels,cnt)
                        if cnt eq 0 then begin
                            darby = get_button(['Go back','Exit'],TITLE=ribbet[0]+' not found in '+slkfile,BASE_TITLE='Error')
                            if darby eq 1 then goto,goback2 else if darby eq 2 then return,rtn={msg:'EXIT'}
                        endif else if cnt gt 1 then begin
                            darby = get_button(['Go back','Exit'],BASE_TITLE='Error',TITLE=ribbet[0]+' found in columns ' $
                                +strjoin(trim(idx+1),' ',/SINGLE)+' of '+slkfile+'. There can only be one.')
                            if darby eq 1 then goto,goback2 else if darby eq 2 then return,rtn={msg:'EXIT'}
                        endif
                        col[idx]=1
                        idx0=where(ribbet[1] eq am.behav_data[*,idx],cnt)
                        if cnt eq 0 then begin
                            darby = get_button(['Go back','Exit'],BASE_TITLE='Error',TITLE=ribbet[1]+' not found in column ' $
                                +trim(idx+1)+' of '+slkfile+'. Labels must match.')
                            if darby eq 1 then goto,goback2 else if darby eq 2 then return,rtn={msg:'EXIT'}
                        endif
                        idx1=[idx1,idx0]
                        nglm[l]=cnt
                    endfor
                    glmfiles=am.files[idx1[1:*]]
                    if n_elements(t4files) gt 1 then t4files = am.t4_files[idx1[1:*]]
                    idx=where(col eq 0,cnt)
                    if cnt ne 0 then begin
                        ;dummy dim 0 is necessary, otherwise "Error message: All array subscripts must be same size. Var = <No name>"
                        slkdata=am.behav_data[idx1[1:*],idx,0] 
                        slkcollab=gs.column_labels[idx]

                        ;START150506
                        slkid=am.subject_id_select[idx1[1:*]]

                    endif
                    undefine,gs,am
                    goto,afters2
                endif
            endif
            k = k + 1
        endfor
        if n_elements(dummy) gt 0 then begin
            if dummy eq 'glm' then i=nlevel else i=i+1
        endif else i=i+1
    endrep until i eq nlevel
    if n_elements(glmfiles) gt 1 then glmfiles = glmfiles[1:*]
    if n_elements(t4files) gt 1 then t4files = t4files[1:*]
    if n_elements(concs) gt 1 then concs = concs[1:*]
    if n_elements(evs) gt 1 then evs = evs[1:*]
    if n_elements(txt) gt 1 then txt = txt[1:*]
    if n_elements(vals) gt 1 then vals = vals[1:*]
    if n_elements(ext) gt 1 then ext = ext[1:*]
    if n_elements(img) gt 1 then img = img[1:*]
    if n_elements(identify) gt 1 then identify = identify[1:*]

    if n_elements(replace) gt 1 then replace=replace[1:*]

    afters2:
    glmlist = glmlist[1:*]
endif
if keyword_set(GET_CONC) and load eq 0 then glmfiles = concs
if glmfiles[0] eq '' then begin
    if concs[0] eq '' and vals[0] eq '' and img[0] eq '' then begin
        return,rtn={msg:'ERROR'}
    endif
    if concs[0] ne '' then begin
        glmfiles = concs
    endif
endif
if glmfiles[0] ne '' then begin
    idx = indgen(n_elements(glmfiles))
    dummy = strmid(glmfiles[0],strlen(glmfiles[0])-3)
endif
goback7a:
if total(nlists1) ge 1 then begin
    if dummy eq 'img' and keyword_set(PAIRED) then begin
        groupnames = get_str(2,['task','control'],['task','control'],TITLE='Please name',/GO_BACK)
        if groupnames[0] eq 'GO_BACK' then goto,goback1
        groupnames = fix_script_name(groupnames,/SPACE_ONLY)
        nlists0=n_elements(groupnames)
        glmfiles1 = glmfiles
        glmfiles = ''
        idx1 = idx
        idx = ''
        nglm = intarr(nlists0)
        for i=0,nlists0-1 do begin
            title = 'Please select files for '+groupnames[i]
            widget_control,/HOURGLASS
            rtn = get_ordered_list(glmfiles1,indgen(n_elements(glmfiles1))+1,TITLE=title,/BELOW)
            ;print,'rtn.labels_sorted=',rtn.labels_sorted
            ;print,'rtn.isorted=',rtn.isorted
            ;print,'rtn.count=',rtn.count
            ;print,'rtn.order_sorted=',rtn.order_sorted
            nglm[i] = rtn.count
            glmfiles = [glmfiles,rtn.labels_sorted]
            idx = [idx,idx1[rtn.isorted]]
            idxnot=intarr(n_elements(glmfiles1))
            idxnot[rtn.isorted]=1
            idxnot = where(idxnot eq 0,cntnot)
            if cntnot ne 0 then begin
                glmfiles1 = glmfiles1[idxnot]
                idx1 = idx1[idxnot]
            endif
        endfor
        glmfiles = glmfiles[1:*]
        idx = idx[1:*]
    endif else if msg[0] ne 'DONTCHECK' then begin
        if dummy eq 'glm' and not keyword_set(NOTMORETHANONE) then special = 'OK - More than one glm per subject'
        if lcallatonce eq 0 then begin
            if keyword_set(GROUPNAMES) then nlists0=n_elements(groupnames) $
            else if keyword_set(LEVELNAMES) then nlists0=n_elements(levelnames)
        endif
        glmfiles1 = glmfiles
        if n_elements(nlists1) gt 1 and nlists0 eq n_elements(nlists1) then begin
            glmfiles2 = glmfiles
            nglm2 = nglm
        endif
        glmfiles = ''
        idx1 = idx
        idx = -1 
        nglm = intarr(nlists0)
        i1 = 0
        for i=0,nlists0-1 do begin
            if n_elements(nlists1) gt 1 and nlists0 eq n_elements(nlists1) then glmfiles1 = glmfiles2[i1:i1+nglm2[i]-1] 
            if keyword_set(GROUPNAMES) and lcallatonce eq 0 then $
                title = 'Please select files for '+groupnames[i] $
            else if keyword_set(LEVELNAMES) and lcallatonce eq 0 then $
                title = 'Please select files for '+levelnames[i] $
            else $
                title = 'Please select'
            rtn = select_files(glmfiles1,TITLE=title,/CANCEL,/EXIT,/ONE_COLUMN,/GO_BACK,SPECIAL=special)

            ;if rtn.files[0] eq 'GO_BACK' then goto,goback1 else if rtn.files[0] eq 'EXIT' then return,rtn={msg:'EXIT'}
            ;START160617
            if rtn.files[0] eq 'GO_BACK' then begin
                undefine,idx
                goto,goback1 
            endif else if rtn.files[0] eq 'EXIT' then return,rtn={msg:'EXIT'}

            morethanone=rtn.special
            nglm[i] = rtn.count
            glmfiles = [glmfiles,rtn.files]
            idx = [idx,idx1[rtn.index+i1]]
            if n_elements(nlists1) gt 1 and nlists0 eq n_elements(nlists1) then begin 
                 i1 = i1 + nglm2[i]
            endif else begin
                idxnot = where(rtn.list eq 0,cntnot)
                if cntnot ne 0 then begin
                    glmfiles1 = glmfiles1[idxnot]
                    idx1 = idx1[idxnot]
                endif
            endelse
        endfor
        glmfiles = glmfiles[1:*]
        idx = idx[1:*]
    endif else begin
        nglm = n_elements(glmfiles)
    endelse
endif else begin
    if glmfiles[0] ne '' then idx = indgen(n_elements(glmfiles)) else if concs[0] ne '' then idx = indgen(n_elements(concs))
endelse
if vals[0] ne '' then $
    return,rtn={msg:'OK',total_nsubjects:n_elements(vals),valsselect:vals} $
else if glmfiles[0] ne '' then $
    total_nsubjects = n_elements(glmfiles) $
else if n_elements(idx) gt 0 then begin
    total_nsubjects = n_elements(idx)
    glmfiles = img[idx]
endif else begin
    total_nsubjects = n_elements(img)
    glmfiles = img
endelse
if dummy eq 'glm' then begin
    concselect = strarr(total_nsubjects)
    evselect = strarr(total_nsubjects)
    glm_space = intarr(total_nsubjects)
endif 
if dummy eq 'slk' then begin
    ;do nothing
endif else if keyword_set(GET_CONC) or dummy eq 'onc' then begin
    if n_elements(concs) ne 0 then begin
        if concs[0] ne '' then concselect=concs else concselect=glmfiles
    endif else concselect=glmfiles
    glmfiles = ''
endif else if total_nsubjects gt 0 then begin
    gft=get_filetype(glmfiles)
    if gft.msg ne 'OK' then return,rtn={msg:gft.msg}
    filetype=gft.filetype

    ;START180227
    ext=gft.ext

    undefine,gft
    ;if filetype[0] ne 'nii' then begin
        ifh = replicate({InterFile_Header},total_nsubjects)
        bold_space = intarr(total_nsubjects)
        idx=-1
        i=0
        nglm1=intarr(n_elements(nglm))
        widget_control,/HOURGLASS
        for j=0,n_elements(nglm)-1 do begin
            for k=0,nglm[j]-1 do begin
                ;print,'Reading '+glmfiles[i]

                ;rh=read_header(glmfiles[i],filetype[i])
                ;START170430
                rh=read_header(glmfiles[i],filetype[i],quiet)

                if rh.msg eq 'GOBACK' then return,rtn={msg:'GO_BACK'} else if rh.msg eq 'EXIT' then return,rtn={msg:'EXIT'}
                if rh.msg ne 'SKIP' then begin
                    ifh[i]=rh.ifh

                    idx=[idx,i]
                    bold_space[i] = get_space(ifh[i].matrix_size_1,ifh[i].matrix_size_2,ifh[i].matrix_size_3)
                    if dummy eq 'glm' then begin
                        if ifh[i].glm_rev gt -17 then begin
                            stat=dialog_message('Revision number for '+strmid(glmfiles[i],1)+' too old. Update file by resaving.', $
                                /ERROR)
                            return,rtn={msg:'ERROR'}
                        endif
                        glm_space[i] = get_space(ifh[i].glm_xdim,ifh[i].glm_ydim,ifh[i].glm_zdim)
                        concselect[i] = ifh[i].data_file
                        if ptr_valid(ifh[i].glm_event_file) then evselect[i] = *ifh[i].glm_event_file
                    endif
                    nglm1[j]=nglm1[j]+1
                endif
                i=i+1

                ;START170614
                ;START170703
                ;undefine,rh

            endfor
        endfor
        nglm=nglm1
        idx = idx[1:*]
        ifh=ifh[idx]
        total_nsubjects = n_elements(idx)
        bold_space = bold_space[idx]
        if dummy eq 'glm' then begin
            glm_space = glm_space[idx]
            concselect = concselect[idx]
            evselect = evselect[idx]
            glmfiles=glmfiles[idx]
        endif else begin

            ;if dummy eq 'img' then imgselect=glmfiles
            ;START170615
            ;if dummy eq 'img' or dummy eq '.gz' then imgselect=glmfiles
            ;START180226
            if dummy eq 'img' or dummy eq '.gz' or dummy eq 'nii' then imgselect=glmfiles

            glmfiles = ''
        endelse
    ;endif


endif
goback10:
if t4files[0] ne '' then begin
    if dummy ne 'glm' then scrap=concselect else scrap=glmfiles
    widget_control,/HOURGLASS
    scrap = get_str(total_nsubjects,scrap,t4files[idx],TITLE='Please check t4s.',/ONE_COLUMN,/BELOW,/GO_BACK)
    if scrap[0] eq 'GO_BACK' then begin
        if nlists1 eq 1 then goto,goback7a else goto,goback1
    endif
    t4select = scrap
endif
if evs[0] ne '' then begin
    if concselect[0] eq '' or notalk eq 1 then $
        evselect=evs[idx] $
    else begin
        widget_control,/HOURGLASS
        scrap = get_str(total_nsubjects,concselect,evs[idx],TITLE='Please check event files.',/ONE_COLUMN,/BELOW,/GO_BACK)
        if scrap[0] eq 'GO_BACK' then goto,goback7a
        evselect = scrap
    endelse
endif
if txt[0] ne '' then begin
    if concselect[0] eq '' then $
        txtselect=txt[idx] $
    else begin
        widget_control,/HOURGLASS
        scrap = get_str(total_nsubjects,concselect,txt[idx],TITLE='Please check txt files.',/ONE_COLUMN,/BELOW,/GO_BACK)
        if scrap[0] eq 'GO_BACK' then goto,goback7a
        txtselect = scrap
    endelse
endif
if ext[0] ne '' then begin
    if concselect[0] eq '' then $
        extselect=ext[idx] $
    else begin
        widget_control,/HOURGLASS
        scrap = get_str(total_nsubjects,concselect,ext[idx],TITLE='Please check ext files.',/ONE_COLUMN,/BELOW,/GO_BACK)
        if scrap[0] eq 'GO_BACK' then goto,goback7a
        extselect = scrap
    endelse
endif
if img[0] ne '' then begin

    ;if concselect[0] eq '' then $
    ;    imgselect=img[idx] $
    ;START160812
    if keyword_set(LEVELNAMES) and not keyword_set(GROUPNAMES) then begin 
        imgselect=img
        total_nsubjects=n_elements(img)
    endif else if concselect[0] eq '' then $
        imgselect=img[idx] $

    else begin
        imgselect = ''
        loop = n_elements(img)/total_nsubjects
        for i=0,loop-1 do begin
             widget_control,/HOURGLASS
             scrap = get_str(total_nsubjects,concselect,img[i*total_nsubjects+idx],TITLE='Please check 4dfp files.',/ONE_COLUMN, $
                 /BELOW,/GO_BACK)
             if scrap[0] eq 'GO_BACK' then goto,goback7a
             imgselect = [imgselect,scrap]
        endfor
        imgselect = imgselect[1:*]
    endelse
endif
if identify[0] ne '' then begin
    if concselect[0] eq '' and imgselect[0] eq '' then $
        identifyselect=identify[idx] $
    else begin
        if concselect[0] eq '' then lizard=imgselect else lizard=concselect
        widget_control,/HOURGLASS
        scrap = get_str(total_nsubjects,lizard,identify[idx],TITLE='Please check identifiers.',/ONE_COLUMN,/BELOW,/GO_BACK)
        if scrap[0] eq 'GO_BACK' then goto,goback7a
        identifyselect = scrap
    endelse
endif

;START160819
if replace[0] ne '' then begin
    if concselect[0] eq '' then $
        replaceselect=replace[idx] $
    else begin
        widget_control,/HOURGLASS
        scrap = get_str(total_nsubjects,concselect,replace[idx],TITLE='Please check replacement concs.',/ONE_COLUMN,/BELOW,/GO_BACK)
        if scrap[0] eq 'GO_BACK' then goto,goback7a
        replaceselect = scrap
    endelse
endif


;print,'load=',load
;print,'glmfiles=',glmfiles
;print,'nlists=',nlists
;print,'total_nsubjects=',total_nsubjects
;print,'t4select=',t4select
;print,'glm_list0=',glmlist0
;print,'concselect=',concselect
;print,'evselect=',evselect
;print,'txtselect=',txtselect
;print,'glm_space=',glm_space
;print,'bold_space=',bold_space
;print,'ifh=',ifh
;print,'morethanone=',morethanone
;print,'here2 nglm=',nglm
;print,'imgselect=',imgselect

if howmany eq 2 and nglm[0] eq 0 then begin
    wallace = get_button([glmfiles,'go back','exit'],TITLE='Please select glm for '+groupnames[0])
    if wallace eq 2 then return,rtn={msg:'GO_BACK'} else if wallace eq 3 then return,rtn={msg:'EXIT'}
    glmfiles = [glmfiles[wallace],glmfiles[1-wallace]]
    nglm = [1,1]    
endif
if n_elements(imgselecti) gt 1 then imgselecti=imgselecti[1:*]

if n_elements(groupnames) eq 0 then groupnames=0 
if n_elements(filetype) eq 0 then filetype=0 

;rtn={msg:'OK',load:load,glmfiles:glmfiles,nlists:nlists,total_nsubjects:total_nsubjects,t4select:t4select,glm_list:glmlist, $
;    concselect:concselect,evselect:evselect,txtselect:txtselect,glm_space:glm_space,bold_space:bold_space,ifh:ifh, $
;    morethanone:morethanone,nglm:nglm,groupnames:groupnames,imgselect:imgselect,imgselecti:imgselecti,extselect:extselect, $
;    identifyselect:identifyselect,slkfile:slkfile,slkdata:slkdata,slkcollab:slkcollab,slkid:slkid,replaceselect:replaceselect, $
;    filetype:filetype}
;START180227
if n_elements(ext) eq 0 then ext=0 
rtn={msg:'OK',load:load,glmfiles:glmfiles,nlists:nlists,total_nsubjects:total_nsubjects,t4select:t4select,glm_list:glmlist, $
    concselect:concselect,evselect:evselect,txtselect:txtselect,glm_space:glm_space,bold_space:bold_space,ifh:ifh, $
    morethanone:morethanone,nglm:nglm,groupnames:groupnames,imgselect:imgselect,imgselecti:imgselecti,extselect:extselect, $
    identifyselect:identifyselect,slkfile:slkfile,slkdata:slkdata,slkcollab:slkcollab,slkid:slkid,replaceselect:replaceselect, $
    filetype:filetype,ext:ext}



return,rtn
end
