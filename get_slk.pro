;Copyright 2/28/08 Washington University.  All Rights Reserved.
;get_slk.pro  $Revision: 1.9 $
function get_slk,help,slkfile,nodatalabel
if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='Behavioral variables are assumed to be stored in a ' $
    +'spreadsheet exported in the *.slk format. The spreadsheet must have the data organized in columns with the first ' $
    +'row in each column containing a label.'
load=0
if n_elements(slkfile) eq 0 then load=1 else if slkfile eq '' then load=1
if load eq 1 then begin
    get_dialog_pickfile,'*.slk',directory,'Please load a slk file.',slkfile,rtn_nfiles,rtn_path
    if slkfile eq 'GOBACK' then return,rtn={msg:'GO_BACK'} else if slkfile eq 'EXIT' then return,rtn={msg:'EXIT'}
endif
widget_control,/HOURGLASS

;column_labels = fix_script_name(strtrim(read_sylk_str(slkfile,NROWS=1,/ARRAY,/SKIP_FIRST_CELL),2),/SPACE_ONLY)
;START170616
column_labels=read_sylk_str(slkfile,NROWS=1,/ARRAY,/SKIP_FIRST_CELL)
;print,'here0 n_elements(column_labels)=',n_elements(column_labels)
column_labels=strtrim(column_labels,2)
;print,'here1 n_elements(column_labels)=',n_elements(column_labels)
column_labels=fix_script_name(column_labels,/SPACE_ONLY)
;print,'here2 n_elements(column_labels)=',n_elements(column_labels)

widget_control,/HOURGLASS
sdata = read_sylk_str(slkfile,STARTROW=1,/ARRAY,/USEDOUBLES,/USELONGS)

idx=where(sdata[*,0] ne '',nbehav_pts)
if nbehav_pts ne 0 then subject_id=strtrim(sdata[idx,0],2) else subject_id=''
if n_elements(nodatalabel) eq 0 then nodatalabel=''

;index_nodata = where(sdata eq '' or sdata eq '.',count_nodata)
;START171020
index_nodata = where(sdata eq '' or sdata eq '.' or sdata eq '#NULL!',count_nodata)

if count_nodata ne 0 then sdata[index_nodata] = nodatalabel 
return,rtn={msg:'OK',ncolumn_labels:n_elements(column_labels),column_labels:column_labels,nbehav_pts:nbehav_pts, $
    subject_id:subject_id,behav_data:sdata[*,1:*],slkfile:slkfile}
end
