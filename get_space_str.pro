;Copyright 9/18/01 Washington University.  All Rights Reserved.
;get_space_str.pro  $Revision: 1.11 $
function get_space_str,space,v,LABEL=label,SECOND_LABEL=second_label,THIRD_LABEL=third_label,CODING=coding
if not keyword_set(CODING) then coding = 0 
if space eq !SPACE_111 then $
    str = '111' $
else if space eq !SPACE_222 then $
    str = '222' $
else if space eq !SPACE_333 then $
    str = '333' $

;START171218
else if space eq !SPACE_MNI111 then $
    str = '111MNI' $

else if space eq !SPACE_MNI222 then $
    str = '222MNI' $
else if space eq !SPACE_MNI333 then $
    str = '333MNI' $
else $
    str = 'unknown'
if not keyword_set(LABEL) then label = ''
if not keyword_set(SECOND_LABEL) then second_label = ''


;str = str + strcompress(string(v[0],v[1],v[2],FORMAT='("(",f5.1,",",f5.1,",",f5.1,")")'),/REMOVE_ALL)
;nv = n_elements(v)
;START160715
nv = n_elements(v)
if nv ne 0 then str = str + strcompress(string(v[0],v[1],v[2],FORMAT='("(",f5.1,",",f5.1,",",f5.1,")")'),/REMOVE_ALL)


if coding ne 1 then begin
    if nv gt 3 then begin
        if keyword_set(LABEL) then str = str + ' ' + label + ' ='

        ;START150709
        str = str + strcompress(string(v[3],FORMAT='(g10.4)'))
        ;START150708
        ;str = str + strcompress(string(v.v[3],FORMAT='(g10.4)'))

        ;print,'get_space_str str=',str

        if nv gt 4 then begin
            if keyword_set(SECOND_LABEL) then str = str + ' ' + second_label + ' ='

            ;START150709
            str = str + strcompress(string(v[4],FORMAT='(g10.4)'))
            ;START150708
            ;str = str + strcompress(string(v.v[4],FORMAT='(g10.4)'))

            if nv gt 5 then begin
                if keyword_set(THIRD_LABEL) then str = str + ' ' + third_label + ' ='

                ;START150709
                str = str + strcompress(string(v[5],FORMAT='(g10.4)'))
                ;START150708
                ;str = str + strcompress(string(v.v[5],FORMAT='(g10.4)'))

            endif
        endif
    endif
endif else begin
    if nv gt 3 then begin
        if keyword_set(LABEL) then str = str + ' ' + label + ' ='

        ;START150709
        str = str + strcompress(string(v[3],FORMAT='(i)'))
        ;START150708
        ;str = str + strcompress(string(v.v[3],FORMAT='(i)'))

        if nv gt 4 then begin
            if keyword_set(SECOND_LABEL) then str = str + ' ' + second_label + ' ='

            ;START150709
            str = str + strcompress(string(v[4],FORMAT='(i)'))
            ;START150708
            ;str = str + strcompress(string(v.v[4],FORMAT='(i)'))

            if nv gt 5 then begin
                if keyword_set(THIRD_LABEL) then str = str + ' ' + third_label + ' ='

                ;START150709
                str = str + strcompress(string(v[5],FORMAT='(i)'))
                ;START150708
                ;str = str + strcompress(string(v.v[5],FORMAT='(i)'))

            endif
        endif
    endif
endelse
return,str
end
