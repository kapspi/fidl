;Copyright 1/26/2015 Washington University.  All Rights Reserved.
;get_filetype.pro  $Revision: 1.10 $
function get_filetype,files
lcnii=0
nfiles = n_elements(files)
if nfiles ne 0 then begin
    filetype = strarr(nfiles)

    ;START180227
    ext = strarr(nfiles)

    for i=0,nfiles-1 do begin
        dummy = strmid(files[i],strlen(files[i])-3)
        if dummy eq 'glm' then filetype[i]='glm' $

        ;else if dummy eq 'img' then filetype[i]='img' $
        ;else if dummy eq '_t4' then filetype[i]='t4' $
        ;START180227
        else if dummy eq 'img' then begin
            filetype[i]='img'
            ext[i]='.4dfp.img'
        endif else if dummy eq '_t4' then filetype[i]='t4' $

        else if dummy eq 'onc' then filetype[i]='conc' $
        else if dummy eq 'idl' or dummy eq 'ile' then filetype[i]='ev' $
        else if dummy eq 'txt' then filetype[i]='txt' $
        else if dummy eq 'als' then filetype[i]='vals' $
        else if dummy eq 'ext' then filetype[i]='ext' $
        else if dummy eq 'nii' or dummy eq '.gz' then begin
            if strpos(files[i],'dtseries') ne -1 then filetype[i]='dtseries' $
            else if strpos(files[i],'dscalar') ne -1 then filetype[i]='dscalar' $

            ;else filetype[i]='nii'
            ;START180227
            else begin
                filetype[i]='nii'
                ext[i]='.nii'
            endelse

            lcnii=1

        ;endif else begin
        ;    stat=get_button(['glm','img','t4','conc','event file','text','vals','external regressor','cifti dtseries', $
        ;        'cifti dscalar','nifti','ignore','go back','exit'],BASE_TITLE='Unknown ending',TITLE=files[i]+string(10B) $
        ;        +'Please select a file type.')
        ;    if stat eq 13 then return,rtn={msg:'EXIT'} else if stat eq 12 then rtn={msg:'GOBACK'} $
        ;    else if stat eq 11 then filetype[i]='IGNORE' $
        ;START160819
        endif else if dummy eq 'ace' then filetype[i]='replace' $ 
        else begin
            stat=get_button(['glm','img','t4','conc','event file','text','vals','external regressor','cifti dtseries', $
                'cifti dscalar','nifti','replace','ignore','go back','exit'],BASE_TITLE='Unknown ending',TITLE=files[i]+string(10B) $
                +'Please select a file type.')
            if stat eq 14 then return,rtn={msg:'EXIT'} else if stat eq 13 then rtn={msg:'GOBACK'} $
            else if stat eq 12 then filetype[i]='IGNORE' $
            else if stat eq 11 then filetype[i]='replace' $ 


            else if stat eq 10 then begin
                filetype[i]='nii'
                lcnii=1

                ;START180227
                ext[i]='.nii'

            endif else if stat eq 9 then begin
                filetype[i]='dscalar'
                lcnii=1
            endif else if stat eq 8 then begin
                filetype[i]='dtseries'
                lcnii=1
            endif else if stat eq 7 then filetype[i]='ext' $ 
            else if stat eq 6 then filetype[i]='vals' $ 
            else if stat eq 5 then filetype[i]='txt' $ 
            else if stat eq 4 then filetype[i]='ev' $ 
            else if stat eq 3 then filetype[i]='conc' $ 
            else if stat eq 2 then filetype[i]='t4' $ 

            ;else if stat eq 1 then filetype[i]='img' $ 
            ;else filetype[i]='glm'
            ;START180227
            else if stat eq 1 then begin
                filetype[i]='img'
                ext[i]='.4dfp.img'
            endif else filetype[i]='glm'



        endelse
    endfor
endif

;return,rtn={msg:'OK',filetype:filetype,lcnii:lcnii}
;START180227
return,rtn={msg:'OK',filetype:filetype,lcnii:lcnii,ext:ext}

end
