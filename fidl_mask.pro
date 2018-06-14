;Copyright 3/12/13 Washington University.  All Rights Reserved.
;fidl_mask.pro  $Revision: 1.4 $
pro fidl_mask,fi,wd,dsp,help,stc,pref
spawn,!BINEXECUTE+'/fidl_timestr2',timestr

;if get_button(['Ok','Exit'],BASE_TITLE='Apply mask (C)',TITLE='If the image to masked is a region file, then the surviving ' $
;    +'region names will be preserved.'+string(10B)+'You may use a statistical image as a mask. All voxels with an absolute ' $
;    +'value >1e-37 are inclusive to the mask.'+string(10B)+string(10B)+'You may load multiple masks and multiple images.') eq 1 $
;    then return
;START180420
if get_button(['Ok','Exit'],BASE_TITLE='Apply mask (C)',TITLE='If the image to masked is a region file, then the surviving ' $
    +'region names will be preserved.'+string(10B)+'You may use a statistical image as a mask. All voxels with an absolute ' $
    +'value >1e-37 are inclusive to the mask.'+string(10B)+string(10B)+'You may load multiple masks and multiple images.'$
    +string(10B)+'A threshold can be applied (also extent, but not in point and click).' $
    +string(10B)+'The sign from the mask can be applied.') eq 1 $
    then return

goback0:
ggmask=get_glms(pref,fi,help,GET_THIS='imgs',BASE_TITLE='Please load mask(s)')
if ggmask.msg ne 'OK' then return
goback1:
ggimg=get_glms(pref,fi,help,GET_THIS='imgs',BASE_TITLE='Please load images to be masked')
if ggimg.msg eq 'EXIT' then return else if ggimg.msg eq 'GO_BACK' then goto,goback0
lizard=ggmask.total_nsubjects*ggimg.total_nsubjects
superbird=strarr(lizard)

;print,'here0 ggmask.filetype'
;print,ggmask.filetype
;print,'here0 ggmask.ext'
;print,ggmask.ext
;print,'here0 ggimg.filetype'
;print,ggimg.filetype
;print,'here0 ggimg.ext'
;print,ggimg.ext

;grmask=get_root(ggmask.imgselect,'.4dfp.img')
;grimg=get_root(ggimg.imgselect,'.4dfp.img')
;START180227
grmask=get_root(ggmask.imgselect,ggmask.ext)
grimg=get_root(ggimg.imgselect,ggimg.ext)


;START180306
goback2:
thresh=''
threshstr=''
scrap=get_str(1,'threshold','1.96',/GO_BACK,TITLE='Zero for no threshold.')
if scrap[0] eq 'GO_BACK' then goto,goback1
threshf=float(scrap[0])
if threshf > 0. then begin
    thresh = trim(scrap[0])
    threshstr = ' -thresh '+thresh
endif

;START180420
goback3:
signstr=''
scrap=get_button(['yes','no','go back','exit'],TITLE='Do you want to apply the sign from the mask?')
if scrap eq 3 then return else if scrap eq 2 then goto,goback2
if scrap eq 0 then signstr=' -sign'
 
j=0
for i=0,ggmask.total_nsubjects-1 do begin

    ;superbird[j:j+ggimg.total_nsubjects-1]=grimg.file+'_masked_'+grmask.file[i]+'.4dfp.img'
    ;START180227
    ;superbird[j:j+ggimg.total_nsubjects-1]=grimg.file+'_masked_'+grmask.file[i]+ggimg.ext[i]
    ;START180306
    superbird[j:j+ggimg.total_nsubjects-1]=grimg.file+'_masked'+thresh+'_'+grmask.file[i]+ggimg.ext[i]

    j=j+ggimg.total_nsubjects
endfor 

goback4:

;names=get_str(1+lizard,['script','out '+trim(indgen(lizard)+1)],/GO_BACK,/ONE_COLUMN,/REPLACE,['fidl_mask'+timestr[0]+'.csh', $
;    superbird],TITLE="Please enter filenames.")
;START180420
names=get_str(1+lizard,['script','out '+trim(indgen(lizard)+1)],/GO_BACK,/ONE_COLUMN,/REPLACE,['fidl_mask'+timestr[0]+'.csh', $
    superbird],TITLE="Please enter filenames. Leave blank any you don't want.")

if names[0] eq 'GO_BACK' then goto,goback3
csh = fix_script_name(names[0])
openw,lu,csh,/GET_LUN
top_of_script,lu
k=1
for i=0,ggmask.total_nsubjects-1 do begin
    for j=0,ggimg.total_nsubjects-1 do begin

        ;printf,lu,'nice +19 $BIN/fidl_mask -file '+ggimg.imgselect[j]+' -mask '+ggmask.imgselect[i]+' -out '+names[k]+threshstr
        ;START180420
        if names[k] ne '' then begin
            printf,lu,'nice +19 $BIN/fidl_mask -file '+ggimg.imgselect[j]+' -mask '+ggmask.imgselect[i]+' -out '+names[k] $
                +threshstr+signstr
        endif

        k=k+1
    endfor
endfor



close,lu
free_lun,lu
spawn,'chmod +x '+csh
action = get_button(['execute','return','go back'],TITLE='Please select action',BASE_TITLE=csh)
if action eq 2 then goto,goback4
if action eq 0 then spawn_cover,csh,fi,wd,dsp,help,stc
print,'DONE'
end
