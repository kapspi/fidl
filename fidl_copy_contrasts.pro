;Copyright 9/4/12 Washington University.  All Rights Reserved.
;fidl_copy_contrasts.pro  $Revision: 1.3 $
pro fidl_copy_contrasts,fi,dsp,wd,glm,help,stc
goback0:
cglmi = get_bolds(fi,dsp,wd,glm,help,stc,FILTER='*.glm',TITLE='Please select contrast glm.')
if not ptr_valid(glm[fi.n].contrast_labels) then begin 
    scrap = get_button(['go back','exit'],TITLE='No contrasts found.',BASE_TITLE=fi.names[cglmi],WIDTH=200)
    if scrap eq 0 then goto,goback0 else return
endif
contrast_labels = strtrim(*glm[fi.n].contrast_labels,2)
goback1:
rtn = select_files(contrast_labels,TITLE='Please select contrasts to be copied.',/CANCEL,/EXIT,/ONE_COLUMN,/GO_BACK)
if rtn.files[0] eq 'GO_BACK' then goto,goback0 else if rtn.files[0] eq 'EXIT' then return
goback2:

;rtn1 = get_glms(pref,BASE_TITLE='Contrasts will be copied to these glms.')
;START140303
rtn1 = get_glms(pref,BASE_TITLE='Contrasts will be copied to these glms.',/NOTMORETHANONE)

if rtn1.msg eq 'GO_BACK' then goto,goback1 else if rtn1.msg eq 'EXIT' then return
goback3:

;spawn,'fidl_timestr',timestr
;START180419
spawn,!BINEXECUTE+'/fidl_timestr2',timestr

csh = 'fidl_copy_contrasts_'+timestr[0]+'.csh'
names = get_str(1,'script',csh,WIDTH=50,TITLE='Please enter.',/GO_BACK)
if names[0] eq 'GO_BACK' then goto,goback2
csh = fix_script_name(names[0])
openw,lu,csh,/GET_LUN
top_of_script,lu
for i=0,rtn1.total_nsubjects-1 do begin
    printf,lu,'nice +19 $BIN/fidl_copy_contrasts -cglm '+fi.names[cglmi]+' -contrasts '+strjoin(strtrim(rtn.index+1,2),' ',/SINGLE) $
        +' -glm '+rtn1.glmfiles[i]
endfor
close,lu
free_lun,lu
spawn,'chmod +x '+csh
action = get_button(['execute','return','go back'],TITLE='Please select action.',BASE_TITLE=csh)
if action eq 2 then goto,goback3
if action eq 0 then begin
    spawn,csh+'>'+csh+'.log &'
    stats = dialog_message(csh+string(10B)+string(10B)+'Script has been executed.',/INFORMATION)
endif
print,'DONE'
end
