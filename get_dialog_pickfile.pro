;Copyright 7/17/01 Washington University.  All Rights Reserved.
;get_dialog_pickfile.pro  $Revision: 1.27 $

;pro get_dialog_pickfile,filter,path,title,rtn_filenames,rtn_nfiles,rtn_path,MULTIPLE_FILES=multiple_files,FILE=file, $
;    DONT_ASK=dont_ask,QUESTION=question,LINMOD=linmod,SAF=saf,HOWMANY=howmany
;START150219
pro get_dialog_pickfile,filter,path,title,rtn_filenames,rtn_nfiles,rtn_path,MULTIPLE_FILES=multiple_files,FILE=file, $
    DONT_ASK=dont_ask,LINMOD=linmod,SAF=saf,HOWMANY=howmany

;START180209
if n_elements(filter) eq 1 then begin
    if filter eq '*.4dfp.img' or filter eq '*.img' then filter=['*.nii','*.nii.gz','*.img']
endif

if not keyword_set(SAF) then saf='Do you wish to select another file?'
if not keyword_set(HOWMANY) then howmany=0
if not keyword_set(MULTIPLE_FILES) then begin
    if keyword_set(FILE) then begin
        initial_file = file
        if strpos(initial_file,'/') ne -1 then path = ''
    endif
    repeat begin
        if keyword_set(FILE) then $
            file = dialog_pickfile(FILTER=filter,PATH=path,GET_PATH=rtn_path,TITLE=title,FILE=initial_file) $
        else $
            file = dialog_pickfile(FILTER=filter,PATH=path,GET_PATH=rtn_path,TITLE=title)
        filename = ''
        for i=0,n_elements(file)-1 do filename = filename + file[i]
        filename = strtrim(filename,2)
        if filename eq '' then begin
            
            ;START150220
            filename='GOBACK'

            if keyword_set(DONT_ASK) then scrap = 1 else scrap = 0 
        endif else if strmid(filename,0,/REVERSE_OFFSET) eq '/' then $
            scrap = 0 $
        else $
            scrap = 1
        if scrap eq 0 then begin


            ;if keyword_set(QUESTION) then begin
            ;    filename = dialog_message('You failed to select a file. Do you wish to try again?',/QUESTION)
            ;    if filename eq 'No' then scrap = 1
            ;endif else if keyword_set(LINMOD) then begin
            ;    scrap = get_button(['Try again.','No event file for this design.','Exit'], $
            ;        TITLE='You failed to select a file. What do you wish to do?')
            ;    if scrap eq 1 then $
            ;        filename = 'NONE' $
            ;    else if scrap eq 2 then $
            ;        filename = ''
            ;endif else begin
            ;    scrap = get_button(['Try again','Go back','Exit'],TITLE='You failed to select a file. What do you wish to do?')
            ;    if scrap eq 1 then $
            ;        filename = 'GO_BACK' $
            ;    else if scrap eq 2 then $
            ;        filename = 'EXIT'
            ;endelse
            ;START150219
            if keyword_set(LINMOD) then begin
                scrap = get_button(['Try again.','No event file for this design.','Exit'], $
                    TITLE='You failed to select a file. What do you wish to do?')
                if scrap eq 1 then $
                    filename = 'NONE' $
                else if scrap eq 2 then $
                    filename = ''
            endif else begin
                scrap = get_button(['Try again','Go back','Exit'],TITLE='You failed to select a file. What do you wish to do?')
                if scrap eq 1 then $
                    filename = 'GOBACK' $
                else if scrap eq 2 then $
                    filename = 'EXIT'
            endelse





        endif
        if strpos(filename,'NONE') ne -1 then filename = 'NONE'
    endrep until scrap ge 1
    rtn_filenames = filename
    rtn_nfiles = 1
endif else begin
    goback1:
    scrap = 0
    rtn_nfiles = 0
    rtn_filenames = '' 
    rtn_path = path
    title = title + '  Control and shift to select multiple files.'
    repeat begin
        repeat begin
            file = dialog_pickfile(FILTER=filter,PATH=rtn_path,GET_PATH=rtn_path,/MULTIPLE_FILES,TITLE=title)
            nfiles = n_elements(file)
            if file[0] eq '' or strmid(file[0],0,/REVERSE_OFFSET) eq '/' then begin
                if file[0] eq '' and rtn_nfiles eq 0 then begin 
                    rtn_filenames = 'GOBACK'
                    scrap = 1
                endif else begin 
                    nfiles = get_button(['Try again.','Cancel'],TITLE='You failed to select a file.')
                    if nfiles eq 1 then scrap = 1
                endelse
            endif
        endrep until nfiles gt 0
        if scrap eq 0 then begin
            rtn_filenames = [rtn_filenames,file[0:nfiles-1]]
            rtn_nfiles = rtn_nfiles + nfiles
            if keyword_set(DONT_ASK) then $
                scrap = 1 $
            else if howmany eq 0 then begin
                scrap = get_button(['yes','no','unload'],TITLE=saf)
                if scrap eq 2 then goto,goback1
            endif else if rtn_nfiles lt howmany then begin
                scrap = get_button(['yes','no','unload'],TITLE=saf)
                if scrap eq 2 then goto,goback1
            endif else $
                scrap = 1
        endif
    endrep until scrap eq 1
    if rtn_nfiles gt 0 then rtn_filenames = rtn_filenames[1:*]
endelse
end
