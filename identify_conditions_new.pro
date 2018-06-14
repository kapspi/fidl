;Copyright 12/13/01 Washington University.  All Rights Reserved.
;identify_conditions_new.pro  $Revision: 1.28 $

;function identify_conditions_new,length_label,labels,treatments1,treatment_str,time,nfactors,dummy,offset,DEFAULT=default, $
;    TOP_TITLE=top_title,TIME_FRAMES=time_frames,SKIP_CHECKS=skip_checks,LENGTH_LABEL2=length_label2,LABELS2=labels2, $
;    SET_BUTTONS=set_buttons,SET_UNSET=set_unset,SET_DIAGONAL=set_diagonal,SAMEFORALL=sameforall,COLUMN_LAYOUT=column_layout, $
;    NO_SELECT=no_select,SET_ORDER=set_order
;START180501
function identify_conditions_new,length_label,labels,treatments1,treatment_str,time,nfactors,dummy,offset,DEFAULT=default, $
    TOP_TITLE=top_title,TIME_FRAMES=time_frames,SKIP_CHECKS=skip_checks,LENGTH_LABEL2=length_label2,LABELS2=labels2, $
    SET_BUTTONS=set_buttons,SET_UNSET=set_unset,SET_DIAGONAL=set_diagonal,SAMEFORALL=sameforall,COLUMN_LAYOUT=column_layout, $
    NO_SELECT=no_select,SET_ORDER=set_order,NOPRESET=nopreset

treatments0 = treatments1
special = ''
if not keyword_set(TOP_TITLE) then top_title = ''
if not keyword_set(LENGTH_LABEL2) then length_label2 = 0
if n_elements(offset) eq 0 then offset = 0
scraparr = strarr(length_label>length_label2,treatments0)
lengths = intarr(treatments0)
if not keyword_set(LENGTH_LABEL2) then begin
    for i=0,treatments0-1 do begin
        scraparr[*,i] = labels
        lengths[i] = length_label
    endfor
endif else begin
    for i=0,treatments0-1,2 do begin
        scraparr[0:length_label-1,i] = labels
        lengths[i] = length_label
    endfor
    for i=1,treatments0-1,2 do begin
        scraparr[0:length_label2-1,i] = labels2
        lengths[i] = length_label2
    endfor
endelse
if not keyword_set(SET_BUTTONS) then set_buttons = 0
if not keyword_set(SET_UNSET) then set_unset = 0
if not keyword_set(SET_DIAGONAL) then set_diagonal = 0
if not keyword_set(SET_ORDER) then set_order=0
if not keyword_set(TOP_TITLE) then top_title = ''
if not keyword_set(SAMEFORALL) then sameforall=0
if not keyword_set(COLUMN_LAYOUT) then begin
    column_layout=0 
    staggered=1
    label_top=1
    nonexclusive=1
endif else begin
    staggered=0 
    label_top=0
    nonexclusive=0
endelse
repeat begin
    if time eq 0 then begin
        if not keyword_set(DEFAULT) and column_layout eq 0 then default = intarr(length_label,treatments0)

        ;if keyword_set(TIME_FRAMES) then for i=0,(treatments0<length_label)-1 do default[i,i] = 1
        ;START180501
        if not keyword_set(NOPRESET) then begin
            if keyword_set(TIME_FRAMES) then for i=0,(treatments0<length_label)-1 do default[i,i] = 1
        endif

        widget_control,/HOURGLASS
        rtn = get_choice_list_many(treatment_str,scraparr,TITLE=dummy,LABEL_TOP=label_top,NONEXCLUSIVE=nonexclusive,/GO_BACK, $
            TOP_TITLE=top_title,DEFAULT=default,LENGTHS=lengths,SET_BUTTONS=set_buttons,SET_UNSET=set_unset,SET_ORDER=set_order, $
            SET_DIAGONAL=set_diagonal,SAMEFORALL=sameforall,STAGGERED=staggered,COLUMN_LAYOUT=column_layout)
    endif else begin
        if not keyword_set(DEFAULT) then default = indgen(treatments0)
        widget_control,/HOURGLASS
        rtn = get_choice_list_many(treatment_str,scraparr,TITLE=dummy,/LABEL_TOP,/GO_BACK,DEFAULT=default,TOP_TITLE=top_title, $
            LENGTHS=lengths,SAMEFORALL=sameforall)
    endelse
    if rtn.special eq 'GO_BACK' then return,rtn_ic = {special:'GO_BACK'}
    scraplist = rtn.list
    special = rtn.special
    count_sumcols = 0
    if size(scraplist,/N_DIMENSIONS) ne 2 then begin 
        if time eq 0 then sumrows = total(scraplist)
    endif else begin 
        if time eq 0 then begin
            sumrows = total(scraplist,1)
            sumcols = total(scraplist,2)
            index_sumcols = where(sumcols gt 1,count_sumcols)
        endif
    endelse
    if time eq 0 then begin
        index_sumrows = where(sumrows eq 0,count_sumrows)
    endif else begin
        count_sumrows = 0
        sumrows = intarr(treatments0)
        sumrows[*] = 1
        if nfactors eq 0 then begin
            count_sumcols = 0
        endif else begin
            scrap = intarr(length_label,treatments0)
            scrap[scraplist,indgen(treatments0)] = 1
            sumcols = total(scrap,2)
            index_sumcols = where(sumcols gt 1,count_sumcols)
        endelse
    endelse
    scrapflag = 1
    if keyword_set(NO_SELECT) then begin
        scrapindex = where(sumrows eq 0,scrapcount)
        if scrapcount ne 0 then scraplist[*,scrapindex] = -1
    endif else if keyword_set(SKIP_CHECKS) then begin
        scrapindex = where(sumrows gt 0,scrapcount)
        if scrapcount eq 0 then begin 

            ;stat=dialog_message('You failed to make a selection. Please try again.',/ERROR)
            ;scrapflag = 0
            ;START130830
            return,rtn_ic={sumrows:0,special:special}

        endif else begin
            scraplist = scraplist[*,scrapindex]
            sumrows = sumrows[scrapindex] 
            treatments0 = scrapcount
        endelse
    endif else begin
        if count_sumrows ne 0 then begin
            scrap = 'At least one condition must be selected for each treatment. No conditions slected for treatments:'+string(10B)
            for i=0,count_sumrows-1 do scrap = scrap + ' ' +treatment_str[index_sumrows[i]]
            stat=widget_message(scrap+'. Please try again.',/ERROR)
            scrapflag = 0
        endif else if count_sumcols ne 0 then begin
            scrapflag = 0
            if not keyword_set(TIME_FRAMES) then $
                scrap = 'A condition may be selected only once. ' $
            else $
                scrap = 'An estimate may be selected only once. '
            for i=0,count_sumcols-1 do begin
                if not keyword_set(TIME_FRAMES) then $
                    scrap = scrap +labels[index_sumcols[i]]+' was selected for treatments:' $
                else $
                    scrap = scrap + 'Estimate ' + strtrim(labels[index_sumcols[i]],2) +' was selected for:'
                scrapindex = where(scraplist[index_sumcols[i],*],scrapcount)
                if scrapcount eq 0 then begin
                    stat=dialog_message('scrapcount should be greater then.',/ERROR)
                endif else begin
                    if time eq 0 then begin
                        goose = where(scraplist[*,0])
                        if labels[goose[0]] eq 'Hi-pass' then begin
                            special = 'Hi-pass'             
                            scrapflag = 1 
                        endif
                    endif
                    if scrapflag eq 0 then begin
                        for j=0,scrapcount-1 do scrap = scrap + ' ' + treatment_str[scrapindex[j]]
                        stat = get_button(['ok','cancel'],TITLE=scrap+'. Please try again.', $
                            BASE_TITLE='Error')
                        if stat eq 1 then begin
                            scrapflag=1
                            goto,outofthis 
                        endif
                    endif
                endelse
            endfor
            outofthis:
        endif
    endelse
endrep until scrapflag eq 1
if keyword_set(NO_SELECT) then $
    index_conditions = scraplist $
else begin
    index_conditions = intarr(treatments0,length_label)
    if time eq 0 then begin
        if column_layout eq 0 then $
            for i=0,treatments0-1 do index_conditions[i,0:sumrows[i]-1] = where(scraplist[*,i]) + offset $
        else $
            index_conditions = scraplist + offset
    endif else begin
        if size(scraplist,/N_DIMENSIONS) lt 2 then $
            index_conditions[*,0] = scraplist[*] + offset $
        else $
            index_conditions[*,0] = scraplist[*,0] + offset
    endelse
endelse
return,rtn_ic={sumrows:sumrows,index_conditions:index_conditions,special:special}
end
