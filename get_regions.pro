;Copyright 11/14/00 Washington University.  All Rights Reserved.
;get_regions.pro  $Revision: 1.28 $
function get_regions,fi,wd,dsp,help,name,filenames,TITLE=title

;print,'get_regions top'

goback0:
if n_elements(filenames) eq 0 then filenames='' 
if filenames[0] eq '' then begin
    goback1:
    if not keyword_set(TITLE) then begin
        title = 'Please select region file(s)'
        if n_elements(name) gt 0 then if name ne '' then title = title + ' for ' + name
    endif
    title = title + '.'

    ;get_dialog_pickfile,'*img',fi.path,title,region_file,nregfiles,rtn_path,/MULTIPLE_FILES
    ;START180316
    ;get_dialog_pickfile,'*ifh',fi.path,title,region_file,nregfiles,rtn_path,/MULTIPLE_FILES
    get_dialog_pickfile,['*img','*.nii'],fi.path,title,region_file,nregfiles,rtn_path,/MULTIPLE_FILES

endif else begin
    region_file = filenames
    nregfiles = n_elements(region_file)
endelse 
if region_file[0] eq 'GOBACK' then return,rtn={msg:'GO_BACK'}
vol = intarr(nregfiles)
region_names = ''
harolds_num = -1
nreg_by_file = intarr(nregfiles)
reg_by_filei = -1
ireg_by_file = -1
region_str=''

;START180316
gft=get_filetype(region_file)
if gft.msg ne 'OK' then return,rtn={msg:gft.msg}
filetype=gft.filetype

widget_control,/HOURGLASS
for i=0,nregfiles-1 do begin

    ;ifh = read_mri_ifh(region_file[i])
    ;vol[i] = ifh.matrix_size_1*ifh.matrix_size_2*ifh.matrix_size_3
    ;nreg_by_file[i] = ifh.nreg
    ;if ifh.nreg gt 0 then begin
    ;    region_names = [region_names,*ifh.region_names]
    ;    harolds_num = [harolds_num,*ifh.harolds_num]
    ;    reg_by_filei = [reg_by_filei,make_array(ifh.nreg,/INTEGER,VALUE=i)]
    ;    ireg_by_file = [ireg_by_file,indgen(ifh.nreg)]
    ;endif
    ;START180316
    rh=read_header(region_file[i],filetype[i])
    if rh.msg eq 'GOBACK' then return,rtn={msg:'GO_BACK'} else if rh.msg eq 'EXIT' then return,rtn={msg:'EXIT'}
    if rh.msg ne 'SKIP' then begin

        ;if filetype[i] eq 'nii' then ifh = read_mri_ifh(region_file[i])
        ;START180411
        if filetype[i] eq 'img' then ifh=rh.ifh else ifh=read_mri_ifh(region_file[i])

        nreg_by_file[i] = ifh.nreg
        if ifh.nreg gt 0 then begin
            region_names = [region_names,*ifh.region_names]
            harolds_num = [harolds_num,*ifh.harolds_num]
            reg_by_filei = [reg_by_filei,make_array(ifh.nreg,/INTEGER,VALUE=i)]
            ireg_by_file = [ireg_by_file,indgen(ifh.nreg)]
        endif
        ifh=rh.ifh
        vol[i] = ifh.matrix_size_1*ifh.matrix_size_2*ifh.matrix_size_3
    endif 


endfor
nreg = total(nreg_by_file)
if nreg eq 0 then begin

    ;spawn,'whoami',scrap
    ;if dialog_message('Hey '+scrap+'!'+string(10B)+'No regions were found. Please check the ifh. Abort!'+string(10B)+string(10B) $
    ;    +'If this is an Avi grey,white,CSF segmentation, hit cancel.',/ERROR,/CANCEL) ne 'Cancel' then return,rtn={msg:'EXIT'}
    ;region_names = ['CSF','grey','white']
    ;index = indgen(nregfiles)
    ;START131009
    goback2:
    scrap=get_button(['These are mask files.','These are grey,white,CSF segmentation files.','go back','exit'], $
        TITLE='No regions were found.')
    if scrap eq 3 then return,rtn={msg:'EXIT'} else if scrap eq 2 then goto,goback1 else if scrap eq 1 then $
        region_names = ['CSF','grey','white'] $
    else begin
        scrap = get_str(1,'region name','mask',/GO_BACK,TITLE='Please name',LABEL='region name',WIDTH=50)
        if scrap[0] eq 'GO_BACK' then goto,goback2
        region_names = strtrim(scrap,2)
    endelse
    index = indgen(nregfiles)

endif else begin
    index = where(nreg_by_file eq 0,count)
    if count ne 0 then begin
        stat=dialog_message_long('INFORMATION','No regions were listed in the ifh files for' $
            +string(10B)+string(10B)+strjoin(strtrim(region_file[index],2),string(10B),/SINGLE))
        if stat eq 'EXIT' or count eq nregfiles then return,rtn={msg:'EXIT'} else if stat eq 'GOBACK' then return,rtn={msg:'GO_BACK'}
    endif
    index = where(nreg_by_file gt 0,count)
    nreg_by_file = nreg_by_file[index]
    region_file = region_file[index]
    region_str = ' -region_file ' + strjoin(region_file,' ',/SINGLE)
    region_names = region_names[1:*]
    harolds_num = harolds_num[1:*]
    reg_by_filei = reg_by_filei[1:*]
    ireg_by_file = ireg_by_file[1:*]
endelse
vol = vol[index]
if total(vol-vol[0]) ne 0 then begin
    scrap = get_button(['Try again','Exit'],BASE_TITLE='ERROR',WIDTH=200,TITLE='All region files must have the same dimensions.') 
    if scrap eq 0 then goto,goback0
    return,rtn={msg:'EXIT'}
endif
space = get_space(ifh.matrix_size_1,ifh.matrix_size_2,ifh.matrix_size_3)

print,'get_regions bottom'

return,rtn={msg:'OK',region_names:region_names,region_str:region_str,space:space,nregfiles:nregfiles,region_file:region_file, $
    harolds_num:harolds_num,nreg_by_file:nreg_by_file,reg_by_filei:reg_by_filei,ireg_by_file:ireg_by_file,nreg:nreg}
end
