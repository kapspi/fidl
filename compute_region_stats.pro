;Copyright 12/31/99 Washington University.  All Rights Reserved.
;compute_region_stats.pro  $Revision: 1.17 $

;***********************************************
pro compute_region_stats,fi,wd,dsp,help,stc,pref
;***********************************************
if help.enable eq !TRUE then widget_control,help.id,SET_VALUE='Given a Z-statistic or magnitude image, this program computes ' $
    +'the center of mass and identifies the voxel of peak activation for selected regions in the associated region file. If a ' $
    +'Z-statistic or magnitude image is not provided, then only the center of mass is returned.+string([10B,10B])+Results are ' $
    +'written to region_center_of_mass.txt' 

goback0:
if get_hdr_4dfp(fi,st,dsp,wd,stc,help,glm,pref,hdr_stat,idx_stat,'statistical image', $
    TITLE='Please select statistical image.') eq !ERROR then return


;if fi.space[idx_stat] ne !SPACE_111 and fi.space[idx_stat] ne !SPACE_222 and fi.space[idx_stat] ne !SPACE_333 then begin
;    stat=dialog_message('Statistical image must be in atlas space.',/ERROR)
;    return
;endif
;START180222
if fi.space[idx_stat] eq !SPACE_DATA then begin
    stat=dialog_message('Statistical image must be in atlas space.',/ERROR)
    return
endif



statstr = ' -statistical_image '+ fi.names[idx_stat]

goback1:
rtn = get_regions(fi,wd,dsp,help)



;if rtn.goback eq !TRUE then $
;    goto,goback0 $
;else if rtn.error_flag eq !TRUE then $
;    return $
;else begin
;    region_names = rtn.region_names
;    region_str = rtn.region_str
;    region_space = rtn.space
;endelse

if rtn.msg eq 'EXIT' then return else if rtn.msg eq 'GO_BACK' then goto,goback0
region_names = rtn.region_names
region_str = rtn.region_str
region_space = rtn.space







if fi.space[idx_stat] ne region_space then begin
    stat = dialog_message("All files must be in the same space. Abort!",/ERROR)
    return
endif
goback2:
rtn = select_files(region_names,TITLE='Please select regions you wish to analyze.',/GO_BACK,MIN_NUM_TO_SELECT=1)
if rtn.files[0] eq 'GO_BACK' then goto,goback1
roi_str = ' -regions_of_interest ' + strjoin(strtrim(rtn.index+1,2),' ',/SINGLE)




;names = get_str(1,'output',directory+'/region_center_of_mass.txt',WIDTH=50,TITLE='Please enter desired filename.')
;names_str = ' -region_center_of_mass_name '+names[0]
;spawn,'date',date
;date_str = ' -date "'+date+'"'

goback3:
names = get_str(2,['script','output'],['fidl_com.csh','region_center_of_mass.txt'],WIDTH=50,TITLE='Please enter desired names.', $
    /GO_BACK)
if names[0] eq 'GO_BACK' then goto,goback2
csh = fix_script_name(names[0])
names_str = ' -region_center_of_mass_name '+names[1]
spawn,'date',date
date_str = ' -date "'+date+'"'





;command = string(statistical_image,region_file,roi_str,names_str,date_str,FORMAT='("compute_region_stats ",a,a,a,a,a)')
;print,'******************'
;print,'HERE IS THE SCRIPT'
;print,'******************'
;print,command
;print,''
;widget_control,/HOURGLASS
;spawn,command,result
;print,'DONE - Results have been written to '+names[0]

;action = get_button(['execute','return','go back'],TITLE='Please select action.'+string(10B)+string(10B) $
;    +'To run on linux, please select return. Login to a linux machine and enter the script on the command line.',BASE_TITLE=csh)
;START141027
action = get_button(['execute','return','go back'],TITLE='Please select action.',BASE_TITLE=csh)



if action eq 2 then goto,goback3
 
openw,lu,csh,/GET_LUN
top_of_script,lu
printf,lu,'nice +19 $BIN'+'/compute_region_stats'+statstr+region_str+roi_str+names_str+date_str
close,lu
free_lun,lu
spawn,'chmod +x '+csh
if action eq 0 then begin
    spawn,'source '+csh,result
    stat=dialog_message(result,/INFORMATION)
    print,result
endif
print,'DONE'
end
