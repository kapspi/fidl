;Copyright 8/4/11 Washington University.  All Rights Reserved.
;get_morethanone.pro  $Revision: 1.12 $

;function get_morethanone,files_per_group,nfiles_per_group,rtn_between_treatments0,rtn_between_treatment_str,t4select1,concselect1, $
;    evselect1,glm_space1,bold_space1,ifh1
;START161215
function get_morethanone,files_per_group,nfiles_per_group,rtn_between_treatments0,rtn_between_treatment_str,t4select1,concselect1, $
    evselect1,glm_space1,bold_space1,ifh1,imgselecti1

total_nsubjects = total(nfiles_per_group)
nsubject_per_group = intarr(rtn_between_treatments0,/NOZERO)
if n_elements(t4select1) lt total_nsubjects then t4select='' else t4select=t4select1
if n_elements(concselect1) lt total_nsubjects then concselect='' else concselect=concselect1
if n_elements(evselect1) lt total_nsubjects then evselect='' else evselect=evselect1
if n_elements(glm_space1) lt total_nsubjects then glm_space=-1 else glm_space=glm_space1
if n_elements(bold_space1) lt total_nsubjects then bold_space=-1 else bold_space=bold_space1
if n_elements(ifh1) lt total_nsubjects then ifh=0 else ifh=ifh1

;START161215
if n_elements(imgselecti1) lt total_nsubjects then imgselecti=-1 else imgselecti=imgselecti1 

go_morethanone:
rtn_gr = get_root(files_per_group,'.')
junk = rtn_gr.file
max_strlen_glms = max(strlen(junk))
rtn = select_files([strtrim(indgen(max_strlen_glms)+1,2),'vc number'],TITLE='Match on characters',/GO_BACK, $
    MIN_NUM_TO_SELECT=-1,BASE_TITLE='Only the root is matched. Path ignored.',SPECIAL='Let me do this manually')
if rtn.files[0] eq 'GO_BACK' then return,rtn={msg:'GO_BACK'} 

;print,'get_morethanone here0 rtn.special=',rtn.special

if rtn.special eq 1 then begin
    k = 1
    repeat begin
        k = k + 1
        rem = total_nsubjects mod k
    endrep until rem eq 0
    ord_labels=intarr(total_nsubjects)
    j=1
    for i=0,total_nsubjects-1,k do begin
        for l=0,k-1 do ord_labels[i+l]=j
        j=j+1
    endfor
    nglm_per_subject=intarr(total_nsubjects)
    total_nsubjects1=0
    nsubject=0
    scrapstr=''
    lct4=0
    lcconc=0
    lcev=0
    lcglmsp=0
    lcboldsp=0
    lcifh=0
    if t4select[0] ne '' then begin
        t4str = t4select
        lct4 = 1
        t4select = ''
    endif
    if concselect[0] ne '' then begin
        concstr = concselect
        lcconc = 1
        concselect = ''
    endif
    if evselect[0] ne '' then begin
        evstr = evselect
        lcev = 1
        evselect = ''
    endif
    if glm_space[0] ne -1 then begin
        glmsp = glm_space 
        lcglmsp = 1
        glm_space = -1 
    endif
    if bold_space[0] ne -1 then begin
        boldsp = bold_space 
        lcboldsp = 1
        bold_space = -1 
    endif
    if n_tags(ifh[0]) ne 0 then begin
        ifh2 = ifh 
        lcifh = 1
        ifh = ifh[0] 
    endif
    j = 0
    k = 0
    for i=0,rtn_between_treatments0-1 do begin
        title='Please assign subject numbers'
        if rtn_between_treatments0 gt 1 then title=title+' for '+rtn_between_treatment_str[i]
        rtn = get_ordered_list(files_per_group[j:j+nfiles_per_group[i]-1],ord_labels[0:nfiles_per_group[i]-1], $
            TITLE=title,/BELOW)
        uniq_order_sorted=uniq(rtn.order_sorted)
        scrap=n_elements(uniq_order_sorted)
        nglm_per_subject[k]=uniq_order_sorted[0]+1
        k = k + 1
        for l=1,scrap-1 do begin
            nglm_per_subject[k]=uniq_order_sorted[l]-uniq_order_sorted[l-1]
            k = k + 1
        endfor
        nsubject=nsubject+scrap
        total_nsubjects1=total_nsubjects1+rtn.count
        scrapstr=[scrapstr,rtn.labels_sorted]

        if lct4 eq 1 then begin
            spider = t4str[j:j+nfiles_per_group[i]-1]
            t4select = [t4select,spider[rtn.isorted]] 
        endif
        if lcconc eq 1 then begin
            spider = concstr[j:j+nfiles_per_group[i]-1]
            concselect = [concselect,spider[rtn.isorted]] 
        endif
        if lcev eq 1 then begin
            spider = evstr[j:j+nfiles_per_group[i]-1]
            evselect = [evselect,spider[rtn.isorted]] 
        endif
        if lcglmsp eq 1 then begin
            spider = glmsp[j:j+nfiles_per_group[i]-1]
            glm_space = [glm_space,spider[rtn.isorted]]
        endif
        if lcboldsp eq 1 then begin
            spider = boldsp[j:j+nfiles_per_group[i]-1]
            bold_space = [bold_space,spider[rtn.isorted]]
        endif
        if lcifh eq 1 then begin
            spider = ifh2[j:j+nfiles_per_group[i]-1]
            ifh = [ifh,spider[rtn.isorted]]
        endif

        nfiles_per_group[i]=rtn.count
        nsubject_per_group[i]=scrap
        j = j + nfiles_per_group[i]
    endfor
    nglm_per_subject = nglm_per_subject[0:nsubject-1]
    files_per_group=scrapstr[1:*]

    if lct4 eq 1 then t4select = t4select[1:*]
    if lcconc eq 1 then concselect = concselect[1:*]
    if lcev eq 1 then evselect = evselect[1:*]
    if lcglmsp eq 1 then glm_space = glm_space[1:*]
    if lcboldsp eq 1 then bold_space = bold_space[1:*]
    if lcifh eq 1 then ifh = ifh[1:*]

    junk = get_root(files_per_group)
    junk = junk.file
    subject_names = ''
    j=0
    for i=0,nsubject-1 do begin
        subject_names = [subject_names,'sub'+strtrim(i+1,2)+'_ex_glm:'+junk[j]]
        j=j+nglm_per_subject[i]
    endfor
    subject_names = subject_names[1:*]
endif else begin
    index = rtn.index
    maxindex = max(index)
    if maxindex eq max_strlen_glms then $
        rtn_match_vc=match_vc(rtn_between_treatments0,files_per_group,nfiles_per_group,junk) $
    else $
        rtn_match_vc=match_vc(rtn_between_treatments0,files_per_group,nfiles_per_group,junk,SEARCHINDEX=index)
    if rtn_match_vc.msg eq 'ERROR' then return,rtn={msg:'ERROR'} else if rtn_match_vc.msg eq 'GO_BACK' then goto,go_morethanone
    files_per_group = rtn_match_vc.files
    nfiles_per_group = rtn_match_vc.nfiles_per_group
    total_nsubjects1 = rtn_match_vc.nfiles
    subject_names = rtn_match_vc.subject_names
    nglm_per_subject = rtn_match_vc.nfiles_per_subject
    nsubject = rtn_match_vc.nsubject
    nsubject_per_group = rtn_match_vc.nsubject_per_group
    if t4select[0] ne '' then t4select=t4select[rtn_match_vc.index]
    if concselect[0] ne '' then concselect=concselect[rtn_match_vc.index]
    if evselect[0] ne '' then evselect=evselect[rtn_match_vc.index]
    if glm_space[0] ne -1 then glm_space=glm_space[rtn_match_vc.index]
    if bold_space[0] ne -1 then bold_space=bold_space[rtn_match_vc.index]
    if n_tags(ifh[0]) ne 0 then ifh=ifh[rtn_match_vc.index]

    ;START161215
    if imgselecti[0] ne -1 then imgselecti=imgselecti[rtn_match_vc.index]

    scrap = -1
    for i=0,nsubject-1 do begin
        junk = make_array(nglm_per_subject[i],/STRING,VALUE=strtrim(i+1,2))
        scrap = [scrap,junk]
    endfor
    scrap = scrap[1:*]
    scrap = strtrim(get_str(total_nsubjects1,files_per_group,scrap,TITLE='Please check subject numbers.', $
        /ONE_COLUMN,/GO_BACK,/LEFT),2)
    if scrap[0] eq 'GO_BACK' then goto,go_morethanone
    files_per_group=files_per_group[sort(float(scrap)+float(indgen(total_nsubjects1))*.00001)]
endelse

;return,rtn={msg:'OK',nglm_per_subject:nglm_per_subject,total_nsubjects:total_nsubjects1,nsubject:nsubject, $
;    files_per_group:files_per_group,nfiles_per_group:nfiles_per_group,nsubject_per_group:nsubject_per_group, $
;    subject_names:subject_names,t4select:t4select,concselect:concselect,evselect:evselect,glm_space:glm_space, $
;    bold_space:bold_space,ifh:ifh}
;START161215
return,rtn={msg:'OK',nglm_per_subject:nglm_per_subject,total_nsubjects:total_nsubjects1,nsubject:nsubject, $
    files_per_group:files_per_group,nfiles_per_group:nfiles_per_group,nsubject_per_group:nsubject_per_group, $
    subject_names:subject_names,t4select:t4select,concselect:concselect,evselect:evselect,glm_space:glm_space, $
    bold_space:bold_space,ifh:ifh,imgselecti:imgselecti}

end
