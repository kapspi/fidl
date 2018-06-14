;Copyright 5/18/06 Washington University.  All Rights Reserved.
;automatch.pro  $Revision: 1.15 $

function automatch,files,t4files,subject_id,slkfile,nbehav_pts,behavdata,grptag,column_labels,nglm_per_subject,special, $
    DONTSELECT=dontselect,REUSE=reuse
;START180501
;function automatch,files,t4files,subject_id,slkfile,nbehav_pts,behavdata,grptag,column_labels,nglm_per_subject,special, $
;    DONTSELECT=dontselect,REUSE=reuse,USESUBID=usesubid

    goback14b1:
    subject_id_select = ''
    behav_data = behavdata
    nt4 = n_elements(t4files)
    if nt4 eq 0 then t4_files = '' else t4_files = t4files
    ngrptag = n_elements(grptag)
    if not keyword_set(DONTSELECT) then begin
        rtn = select_files(files,TITLE='Please select files.',/GO_BACK,MIN_NUM_TO_SELECT=2,/ONE_COLUMN)
        if rtn.files[0] eq 'GO_BACK' then return,rtn={msg:'GO_BACK'} 
        nglm = rtn.count
        glm_files = rtn.files
        if nt4 gt 0 then begin
            nt4 = nglm
            t4_files = t4_files[rtn.index]
        endif
        if ngrptag gt 0 then grptag = grptag[rtn.index] 
    endif else begin
        nglm = n_elements(files)
        glm_files = files
    endelse
    subjectinot=-1
    subjecti=-1
    nglmpersubject=-1
    goback14c1:


    idx=get_button(['Yes, match on first column','Yes, select a different column','Let me do this manually','Go back','Exit'], $
        TITLE='If the slk file has labels that are contained within the filenames, then FIDL can match those labels to the ' $
        +'filenames.'+string(10B)+string(10B)+'Would you like FIDL to automatically match slk labels to filenames?', $
        BASE_TITLE='Matching')
    if idx eq 4 then return,rtn={msg:'EXIT'} else if idx eq 3 then begin
        if not keyword_set(DONTSELECT) then goto,goback14b1 else return,rtn={msg:'GO_BACK'}
    endif else if idx eq 1 then begin
        if n_elements(column_labels) eq 0 then column_labels=trim(indgen(nbehav_pts)+2) 
        idx1=get_button([column_labels,'Go back','Exit'],TITLE='Match on column',BASE_TITLE='Please select column')
        if idx1 eq n_elements(column_labels) then goto,goback14c1 else if idx1 eq n_elements(column_labels)+1 then $
            return,rtn={msg:'EXIT'}
        subject_id=behav_data[*,idx1]
        idx=0
    endif
    ;START180501
    ;if keyword_set(USESUBID) then begin
    ;    idx=0
    ;endif else begin
    ;    idx=get_button(['Yes, match on first column','Yes, select a different column','Let me do this manually','Go back','Exit'], $
    ;        TITLE='If the slk file has labels that are contained within the filenames, then FIDL can match those labels to the ' $
    ;        +'filenames.'+string(10B)+string(10B)+'Would you like FIDL to automatically match slk labels to filenames?', $
    ;        BASE_TITLE='Matching')
    ;    if idx eq 4 then return,rtn={msg:'EXIT'} else if idx eq 3 then begin
    ;        if not keyword_set(DONTSELECT) then goto,goback14b1 else return,rtn={msg:'GO_BACK'}
    ;    endif else if idx eq 1 then begin
    ;        if n_elements(column_labels) eq 0 then column_labels=trim(indgen(nbehav_pts)+2) 
    ;        idx1=get_button([column_labels,'Go back','Exit'],TITLE='Match on column',BASE_TITLE='Please select column')
    ;        if idx1 eq n_elements(column_labels) then goto,goback14c1 else if idx1 eq n_elements(column_labels)+1 then $
    ;            return,rtn={msg:'EXIT'}
    ;        subject_id=behav_data[*,idx1]
    ;        idx=0
    ;    endif
    ;endelse


    if idx eq 0 then begin 
        slkindex = intarr(nglm)
        used = intarr(nbehav_pts)
        m = strlen(subject_id)
        n = strlen(glm_files)
        widget_control,/HOURGLASS
        for i=0,nglm-1 do begin
            j = 0
            doitagain:
            repeat begin
                k = strpos(glm_files[i],subject_id[j])
                j = j + 1
            endrep until k ne -1 or j eq nbehav_pts
            if k ne -1 then begin
                j = j - 1
                if m[j] lt n[i]-k then begin
                    c = strmid(glm_files[i],k+m[j],1)
                    if c eq '0' or c eq '1' or c eq '2' or c eq '3' or c eq '4' or c eq '5' or c eq '6' or c eq '7' or c eq '8' or $
                        c eq '9' or used[j] eq 1 then begin
                        j = j + 1
                        goto,doitagain
                    endif
                endif
                if not keyword_set(REUSE) then used[j]=1
            endif
            slkindex[i] = j
        endfor
        index = where(slkindex eq nbehav_pts,count)
        if count ne 0 then begin

            ;scrap=dialog_message_long('INFORMATION','The following files did not have labels in '+slkfile+string(10B) $
            ;    +'These '+trim(count)+' files will be excluded from the analysis.'+string(10B) $
            ;    +strjoin(glm_files[index],string(10B),/SINGLE))
            ;START171207
            if count eq 1 then begin
                superbird='The following file did not have a label in '+slkfile+string(10B) $
                    +'This file will be excluded from the analysis.'+string(10B)+string(10B)
            endif else begin
                superbird='The following files did not have labels in '+slkfile+string(10B) $
                    +'These '+trim(count)+' files will be excluded from the analysis.'+string(10B)+string(10B)
            endelse
            scrap=dialog_message_long('INFORMATION',superbird+strjoin(glm_files[index],string(10B),/SINGLE),SPECIAL=special)

            if scrap eq 'EXIT' then return,rtn={msg:'EXIT'} else if scrap eq 'GOBACK' then goto,goback14c1
            index = where(slkindex ne nbehav_pts,count)
            glm_files = glm_files[index]
            nglm = n_elements(glm_files)
            behav_data = behav_data[slkindex[index],*]
            if nt4 gt 0 then begin
                t4_files = t4_files[index]
                nt4 = nglm
            endif
            if ngrptag gt 0 then grptag = grptag[index] 
            subject_id_select=subject_id[slkindex[index]]

            ;START151123
            ;if n_elements(nglm_per_subject) ne 0 then begin 
            ;    subjecti=intarr(n_elements(nglm_per_subject))
            ;    subjectinot=intarr(n_elements(nglm_per_subject))
            ;    nglmpersubject=intarr(n_elements(nglm_per_subject))
            ;    j=0
            ;    k=0
            ;    l=0
            ;    for i=0,n_elements(nglm_per_subject)-1 do begin
            ;        idx=where(slkindex[j:j+nglm_per_subject[i]-1] ne nbehav_pts,cnt)
            ;        nglmpersubject[i]=cnt
            ;        if cnt eq 0 then begin
            ;            subjectinot[l]=i
            ;            l=l+1
            ;        endif else begin
            ;            subjecti[k]=i
            ;            nglmpersubject[k]=cnt
            ;            k=k+1
            ;        endelse
            ;        j=j+nglm_per_subject[i]
            ;    endfor
            ;    subjectinot=subjectinot[0:l-1]
            ;    subjecti=subjecti[0:k-1]
            ;    nglmpersubject=nglmpersubject[0:k-1]
            ;endif

        endif else begin 
            behav_data = behav_data[slkindex,*]

            ;subject_id_select=subject_id
            ;START151123
            subject_id_select=subject_id[slkindex]

        endelse

        ;print,'here100 slkindex=',slkindex
        ;print,'here100 nbehav_pts=',nbehav_pts

        ;START151123
        if n_elements(nglm_per_subject) ne 0 then begin

            subjecti=intarr(n_elements(nglm_per_subject))
            subjectinot=intarr(n_elements(nglm_per_subject))
            nglmpersubject=intarr(n_elements(nglm_per_subject))
            ;START151123
            ;subjecti=make_array(n_elements(nglm_per_subject),/INTEGER,VALUE=-1)
            ;subjectinot=make_array(n_elements(nglm_per_subject),/INTEGER,VALUE=-1)
            ;nglmpersubject=make_array(n_elements(nglm_per_subject),/INTEGER,VALUE=-1)

            j=0
            k=0
            l=0
            for i=0,n_elements(nglm_per_subject)-1 do begin
                idx=where(slkindex[j:j+nglm_per_subject[i]-1] ne nbehav_pts,cnt)
                nglmpersubject[i]=cnt
                if cnt eq 0 then begin
                    subjectinot[l]=i
                    l=l+1
                endif else begin
                    subjecti[k]=i
                    nglmpersubject[k]=cnt
                    k=k+1
                endelse
                j=j+nglm_per_subject[i]
            endfor

            ;print,'here101 l=',l,' k=',k

            ;subjectinot=subjectinot[0:l-1]
            ;subjecti=subjecti[0:k-1]
            ;nglmpersubject=nglmpersubject[0:k-1]
            ;START151123
            if l gt 0 then subjectinot=subjectinot[0:l-1] else subjectinot=-1
            if k gt 0 then begin
                subjecti=subjecti[0:k-1]
                nglmpersubject=nglmpersubject[0:k-1]
            endif else begin
                subjecti=-1
                nglmpersubject=-1
            endelse

        endif



    endif else begin
        subject_id_select = match_files(glm_files,subject_id,index,TITLE='Please select a label for ',/GO_BACK)
        if subject_id_select[0] eq 'GO_BACK' then goto,goback14c1
        behav_data = behav_data[index,*]
    endelse
    return,rtn={msg:'OK',nfiles:nglm,files:glm_files,nt4:nt4,t4_files:t4_files,behav_data:trim(behav_data), $
        subject_id_select:subject_id_select,subjectinot:subjectinot,subjecti:subjecti,nglmpersubject:nglmpersubject}
end
