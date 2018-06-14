;Copyright 3/15/01 Washington University.  All Rights Reserved.
;get_files.pro  $Revision: 1.29 $
function get_files,filterinout,TITLE=title,FILTER=filter,EXTRALABEL=extralabel,FRONT=front,HOWMANY=howmany,LABEL=label, $
    DIALOGEXTRATITLE=dialogextratitle,EXTRATITLE=extratitle
if not keyword_set(TITLE) then title = 'Please specify the proper filter.'
if not keyword_set(FILTER) then filter = '*.4dfp.img'
if n_elements(filterinout) ne 0 then filter = filterinout
if not keyword_set(EXTRALABEL) then extralabel = '' else extralabel = '  '+extralabel
if not keyword_set(FRONT) then front = 0 else if front eq '' then front = 0
if not keyword_set(HOWMANY) then howmany=0
width = 100 > (strlen(title) + 20)
if not keyword_set(LABEL) then label = 'You may include a path if you wish.'
label= label+extralabel
if not keyword_set(EXTRATITLE) then extratitle='' else extratitle=' '+extratitle
if not keyword_set(DIALOGEXTRATITLE) then dialogextratitle='' else dialogextratitle=dialogextratitle+' '
get_directory,directory,offset_spawn
repeat begin

    ;str=get_str(1,'Filter: ',filter,WIDTH=width,TITLE=title+extratitle,LABEL=label,/GO_BACK,/EXIT,/CR,/ADD,FRONT=front, $
    ;    SPECIAL='Dialog window')
    ;START180413
    str=get_str(1,'Filter: ',filter[0],WIDTH=width,TITLE=title+extratitle,LABEL=label,/GO_BACK,/EXIT,/CR,/ADD,FRONT=front, $
        SPECIAL='Dialog window')

    if str[0] eq 'GO_BACK' then $
        return,rtn={msg:'GO_BACK'} $
    else if str[0] eq 'EXIT' then $
        return,rtn={msg:'EXIT'} $
    else if strupcase(str[0]) eq 'NONE' then $
        return,rtn={msg:'OK',files:'NONE'} $
    else if strupcase(str[0]) eq 'TXT' then $
        return,rtn={msg:'OK',files:'TXT'} $
    else if str[0] eq 'SPECIAL' then begin
        get_dialog_pickfile,filter,directory,dialogextratitle+title,files,rtn_nfiles,rtn_path,/MULTIPLE_FILES,HOWMANY=howmany
        return,rtn={msg:'DONTCHECK',files:files}
    endif
    filterinout = str[0]
    widget_control,/HOURGLASS
    rtn = get_root(str)
    path = rtn.path
    str = rtn.file
    widget_control,/HOURGLASS
    files = ''
    goose = 'find '+path+' -name '+ '"' + str + '" | sort'
    for i=0,n_elements(str)-1 do begin
        spawn,goose[i],dummy
        files = [files,dummy]
    endfor
    files = files[1:*]
    nfiles = n_elements(files) - offset_spawn
    if strlen(files[offset_spawn]) eq 0 then begin
        stat=get_button(['ok','go back','exit'],BASE_TITLE='Error: No files found.',TITLE='Filter: '+filterinout+string(10B) $
            +'Please try again.')
        if stat eq 1 then return,rtn={msg:'GO_BACK'} else if stat eq 2 then return,rtn={msg:'EXIT'}
        scrap = 0
    endif else begin
        scrap = 1
    endelse
endrep until scrap eq 1
files = files[offset_spawn:nfiles+offset_spawn-1]
pos=strpos(files,'.')
for i=0,nfiles-1 do if pos[i] eq 0 then files[i] = directory + '/' + files[i]
return,rtn={msg:'OK',files:files,filter:filterinout}
end
