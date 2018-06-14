;Copyright 12/31/99 Washington University.  All Rights Reserved.
;get_str.pro  $Revision: 12.144 $
pro get_str_event,ev
    common getint_comm,strs,intbase,g_ok,g_go_back,g_exit,wdstr,nstr,g_special,g_front,frontstr,g_delpath,g_reset,strscopy,g_cr,path, $
        g_find,g_replace,g_add,glabels,gvalues,xsize,leader,gtitle,glabel,gone_column,gbelow,glongtext,glonglabel,gspecial,gfront, $
        val_delpath,val_reset,gvalues_init,g_back,backstr,gback,frontt,gadd,galign_left,gleft
    if ev.id eq g_ok then begin
        for istr=0,nstr-1 do begin
            widget_control,wdstr[istr],GET_VALUE=scrap
            strs[istr] = scrap[0]
        endfor
        widget_control,ev.top,/DESTROY
    endif else if ev.id eq g_add then begin
        for istr=0,nstr-1 do begin
            widget_control,wdstr[istr],GET_VALUE=scrap
            gvalues[istr] = scrap[0]
        endfor
        widget_control,ev.top,/DESTROY
        glabels = [glabels,glabels[nstr-gadd:nstr-1]]
        gvalues = [gvalues,gvalues_init]
        nstr = nstr + gadd 
        cwboss
    endif else if ev.id eq g_go_back then begin
        strs[0] = 'GO_BACK'
        widget_control,ev.top,/DESTROY
    endif else if ev.id eq g_exit then begin
        strs[0] = 'EXIT'
        widget_control,ev.top,/DESTROY
    endif else if ev.id eq g_special then begin
        strs[0] = 'SPECIAL'
        for istr=0,nstr-1 do begin
            widget_control,wdstr[istr],GET_VALUE=scrap
            strs[istr] = scrap[0]
        endfor
        strs = ['SPECIAL',strs]
        widget_control,ev.top,/DESTROY
    endif else if ev.id eq g_front then begin
        frontstr = ev.value
        if frontt eq 'Front' then begin
            for istr=0,nstr-1 do begin
                widget_control,wdstr[istr],GET_VALUE=scrap
                rtn_gr = get_root(scrap[0])
                scrap = frontstr + rtn_gr.file[0]
                widget_control,wdstr[istr],SET_VALUE=scrap
            endfor
        endif else begin
            for istr=0,nstr-1 do widget_control,wdstr[istr],SET_VALUE=frontstr
        endelse
    endif else if ev.id eq g_back then begin
        backstr = ev.value
        for istr=0,nstr-1 do begin
            widget_control,wdstr[istr],GET_VALUE=scrap
            rtn_gr = get_root(scrap[0])
            scrap = rtn_gr.file[0] + backstr
            widget_control,wdstr[istr],SET_VALUE=scrap
        endfor
    endif else if ev.id eq g_delpath then begin
        if path ne -1 then begin
            for istr=0,nstr-1 do begin
                widget_control,wdstr[istr],GET_VALUE=scrap
                rtn_gr = get_root(scrap[0]) 
                scrap = rtn_gr.file 
                widget_control,wdstr[istr],SET_VALUE=scrap
            endfor
        endif else begin
            for istr=0,nstr-1 do begin
                strs[istr] = ''
                widget_control,wdstr[istr],SET_VALUE=strs[istr]
            endfor
        endelse
    endif else if ev.id eq g_reset then begin
        strs = strscopy
        for istr=0,nstr-1 do widget_control,wdstr[istr],SET_VALUE=strs[istr]
    endif else if ev.id eq g_replace then begin
        widget_control,g_find,GET_VALUE=findstr 
        findstr = findstr[0]
        widget_control,g_replace,GET_VALUE=replacestr 
        replacestr = replacestr[0]
        scraplabel = ['*', '?']
        replace    = ['.*','.?']
        for i=0,1 do findstr = strjoin(strsplit(findstr,scraplabel[i],/EXTRACT),replace[i])
        for istr=0,nstr-1 do begin
            widget_control,wdstr[istr],GET_VALUE=scrap
            scrap = scrap[0]
            pos=stregex(scrap,findstr,LENGTH=len)
            if pos[0] eq -1 then begin
                junk='Could not find '+string(10B)+findstr+string(10B)+'in'+string(10B)+scrap
                print,junk
                ;stat=dialog_message(junk,/INFORMATION)
            endif else begin
                if pos[0] eq 0 then begin
                    scrap = replacestr + strmid(scrap,len[0])
                endif else begin
                    firstpart = strmid(scrap,0,pos[0]) + replacestr
                    startfrom = pos[0]+len[0]
                    if startfrom lt strlen(scrap) then secondpart = strmid(scrap,startfrom) else secondpart = ''
                    scrap = firstpart + secondpart
                endelse
                widget_control,wdstr[istr],SET_VALUE=scrap
            endelse
        endfor
    endif else begin
        if g_cr eq -1 then begin
            tst = 0
            for istr=0,nstr-1 do begin
                if ev.id eq wdstr[istr] then begin
                    tst = 1
                    strs[istr] = ev.value
                endif
            endfor
            if tst eq 0 then stat=dialog_message('Invalid event in get_str_event',/ERROR)
        endif else begin
            for istr=0,nstr-1 do begin
                widget_control,wdstr[istr],GET_VALUE=scrap
                strs[istr] = scrap[0]
            endfor
            widget_control,ev.top,/DESTROY
        endelse
    endelse
    check_math_errors,!FALSE
end

;pro create_widgets,align_left,X_SCROLL_SIZE=x_scroll_size,Y_SCROLL_SIZE=y_scroll_size
;START150828
;function create_widgets,align_left,X_SCROLL_SIZE=x_scroll_size,Y_SCROLL_SIZE=y_scroll_size
;START160401
function create_widgets,xdim,ydim

    common getint_comm
    intbase = widget_base(/COLUMN,TITLE=gtitle,GROUP_LEADER=leader,/MODAL)
    if glongtext[0] eq '' then begin
        if glabel ne '' then labelbase = widget_label(intbase,VALUE=glabel,ALIGN_LEFT=galign_left)


        ;if keyword_set(X_SCROLL_SIZE) or keyword_set(Y_SCROLL_SIZE) then begin
        ;    print,'X_SCROLL_SIZE=',!DISPXMAX<x_scroll_size,' XSIZE=',x_scroll_size
        ;    print,'Y_SCROLL_SIZE=',(!DISPYMAX-100)<y_scroll_size,' YSIZE=',y_scroll_size
        ;    ;butcol = widget_base(intbase,/ROW,X_SCROLL_SIZE=(!DISPXMAX<x_scroll_size),XSIZE=x_scroll_size, $
        ;    ;    Y_SCROLL_SIZE=(!DISPYMAX<y_scroll_size),YSIZE=y_scroll_size)
        ;    ;START151230
        ;    ;butcol = widget_base(intbase,/ROW,X_SCROLL_SIZE=(!DISPXMAX<x_scroll_size),XSIZE=x_scroll_size, $
        ;    ;    Y_SCROLL_SIZE=((!DISPYMAX-100)<y_scroll_size),YSIZE=y_scroll_size)
        ;START160401
        if n_elements(xdim) ne 0 then begin


            ;butcol=widget_base(intbase,/ROW,SCR_XSIZE=(xdim<!DISPXMAX),SCR_YSIZE=((ydim+20)<!DISPYMAX),/SCROLL)
            ;START160412
            ;butcol = widget_base(intbase,/ROW,X_SCROLL_SIZE=(!DISPXMAX<xdim),XSIZE=xdim,Y_SCROLL_SIZE=(!DISPYMAX<ydim),YSIZE=ydim)
            ;START160816
            ;butcol=widget_base(intbase,/ROW,SCR_XSIZE=(!DISPXMAX<xdim),SCR_YSIZE=(!DISPYMAX<ydim),/SCROLL)
            butcol = widget_base(intbase,/ROW,X_SCROLL_SIZE=(!DISPXMAX<xdim),XSIZE=xdim,Y_SCROLL_SIZE=(!DISPYMAX<ydim),YSIZE=ydim)

        endif else $
            butcol = widget_base(intbase,/ROW)
    endif else begin
        row = widget_base(intbase,/ROW)
        col1 = widget_base(row,/COLUMN)
        if glonglabel ne '' then longlabelbase = widget_label(col1,VALUE=glonglabel)
        col2 = widget_base(row,/COLUMN)
        if glabel ne '' then labelbase = widget_label(col2,VALUE=glabel,ALIGN_LEFT=galign_left)

        ;if keyword_set(X_SCROLL_SIZE) or keyword_set(Y_SCROLL_SIZE) then begin
        ;    butcol = widget_base(col2,/ROW,X_SCROLL_SIZE=(!DISPXMAX<x_scroll_size),XSIZE=x_scroll_size, $
        ;        Y_SCROLL_SIZE=(!DISPYMAX<y_scroll_size),YSIZE=y_scroll_size)
        ;START160401
        if n_elements(xdim) then begin
            butcol = widget_base(col2,/ROW,X_SCROLL_SIZE=(!DISPXMAX<xdim),XSIZE=xdim,Y_SCROLL_SIZE=(!DISPYMAX<ydim),YSIZE=ydim)

        endif else $
            butcol = widget_base(col2,/ROW)
    endelse
    if gone_column eq 1 then begin
        next = nstr
        ncols = 1
    endif else begin
        next = 15
        ncols = fix((nstr+1)/next) + 1
    endelse
    butbase = lonarr(ncols)
    for icol=0,ncols-1 do butbase[icol] = widget_base(butcol,/COLUMN)
    wdstr = lonarr(nstr)
    if gleft eq 1 then begin
        rowstr=lonarr(nstr)
        labelstr=lonarr(nstr)
    endif
    if g_cr eq -1 then begin
        if gbelow eq 0 then begin
            if gleft eq 0 then begin
                for i=0,nstr-1 do begin
                    icol = i/next
                    wdstr[i] = cw_field(butbase[icol],TITLE=glabels[i],VALUE=gvalues[i],XSIZE=xsize,/ALL_EVENTS)
                endfor
            endif else begin
                gval=trim(gvalues)
                for i=0,nstr-1 do begin
                    icol = i/next
                    rowstr[i]=widget_base(butbase[icol],/ROW)
                    wdstr[i]=widget_text(rowstr[i],/EDITABLE,VALUE=gval[i],XSIZE=5)
                    labelstr[i]=widget_label(rowstr[i],/ALIGN_LEFT,VALUE=glabels[i])
                endfor
                undefine,gval
            endelse
        endif else begin
            for i=0,nstr-1 do begin
                icol = i/next
                wdstr[i] = cw_field(butbase[icol],TITLE=glabels[i],VALUE=gvalues[i],XSIZE=xsize,/ALL_EVENTS,/COLUMN,FRAME=3)
            endfor
        endelse
    endif else begin
        if gbelow eq 0 then begin
            for i=0,nstr-1 do begin
                icol = i/next
                wdstr[i] = cw_field(butbase[icol],TITLE=glabels[i],VALUE=gvalues[i],XSIZE=xsize,/RETURN_EVENTS)
            endfor
        endif else begin
            for i=0,nstr-1 do begin
                icol = i/next
                wdstr[i] = cw_field(butbase[icol],TITLE=glabels[i],VALUE=gvalues[i],XSIZE=xsize,/RETURN_EVENTS,/COLUMN,FRAME=3)
            endfor
        endelse
    endelse
    goose = 1
    repeat begin
        if g_go_back+g_exit eq -2 and gspecial eq '' then begin
            g_ok = widget_button(intbase,value='OK')
            scrap = intbase
        endif else begin
            if goose eq 1 then begin 
                okbase  = widget_base(intbase,/ROW)
                goose = 0
            endif else begin 
                okbase  = widget_base(intbase,/ROW)
                goose = 1
            endelse
            g_ok = widget_button(okbase,value='OK',XSIZE=100)
            if g_go_back gt -1 then g_go_back = widget_button(okbase,value='GO BACK')
            if g_exit gt -1 then g_exit = widget_button(okbase,value='EXIT',XSIZE=100)
            if gspecial ne '' then g_special = widget_button(okbase,value=gspecial)
            if g_add gt -1 then g_add = widget_button(okbase,value='Add another')
            scrap = okbase
        endelse
        if g_delpath gt -1 then g_delpath = widget_button(scrap,value=val_delpath,XSIZE=100)
        if g_reset gt -1 then g_reset = widget_button(scrap,value=val_reset,XSIZE=100)

        ;START151230
        ;if g_back gt -1 then g_back = cw_field(scrap,TITLE='Back',VALUE=gback,XSIZE=xsize/2+10,/RETURN_EVENTS)

        if goose eq 0 then begin
            geo = widget_info(okbase,/GEOMETRY)
            xdim = geo.scr_xsize + 2*geo.margin
            if xdim gt !DISPXMAX then widget_control,okbase,/DESTROY else goose=1
        endif
    endrep until goose eq 1


    ;if g_front gt -1 then begin
    ;    frontbase = widget_base(intbase,/ROW)
    ;
    ;    ;g_front = cw_field(frontbase,TITLE=frontt,VALUE=gfront,XSIZE=strlen(gfront)>xsize,/RETURN_EVENTS)
    ;    ;START151230
    ;    g_front = cw_field(frontbase,TITLE=frontt,VALUE=gfront,XSIZE=30,/RETURN_EVENTS)
    ;  
    ;endif
    ;;START151230
    ;if g_back gt -1 then begin
    ;    if g_front le -1 then frontbase = widget_base(intbase,/ROW)
    ;
    ;    ;g_back = cw_field(frontbase,TITLE='Back',VALUE=gback,XSIZE=strlen(gback)>xsize,/RETURN_EVENTS)
    ;    ;START151230
    ;    g_back = cw_field(frontbase,TITLE='Back',VALUE=gback,XSIZE=30,/RETURN_EVENTS)
    ;
    ;endif
    ;START160127
    if g_front gt -1 then begin
        frontbase = widget_base(intbase,/ROW)

        ;if g_back le -1 then $
        ;    g_front = cw_field(frontbase,TITLE=frontt,VALUE=gfront,XSIZE=strlen(gfront)>xsize,/RETURN_EVENTS) $
        ;else $
        ;    g_front = cw_field(frontbase,TITLE=frontt,VALUE=gfront,XSIZE=30,/RETURN_EVENTS)
        ;START160816
        if g_back le -1 then begin 

            ;g_front = cw_field(frontbase,TITLE=frontt,VALUE=gfront,XSIZE=strlen(gfront)>xsize,/RETURN_EVENTS)
            ;START160816
            ;g_front = cw_field(frontbase,TITLE=frontt,VALUE=gfront,XSIZE=strlen(gfront)>xdim,/RETURN_EVENTS)
            ;g_front = cw_field(frontbase,TITLE=frontt,VALUE=gfront,XSIZE=strlen(gfront),/RETURN_EVENTS) ;OK
            ;g_front = cw_field(frontbase,TITLE=frontt,VALUE=gfront,XSIZE=strlen(gfront)>(!DISPXMAX<xdim),/RETURN_EVENTS)
            ;g_front = cw_field(frontbase,TITLE=frontt,VALUE=gfront,/RETURN_EVENTS) ;OK
            ;g_front = cw_field(frontbase,TITLE=frontt,VALUE=gfront,XSIZE=strlen(gfront)>((!DISPXMAX<xdim)-500),/RETURN_EVENTS)
            g_front = cw_field(frontbase,TITLE=frontt,VALUE=gfront,/RETURN_EVENTS)

        endif else $
            g_front = cw_field(frontbase,TITLE=frontt,VALUE=gfront,XSIZE=30,/RETURN_EVENTS)

    endif
    if g_back gt -1 then begin
        if g_front le -1 then begin
            frontbase = widget_base(intbase,/ROW)
            g_back = cw_field(frontbase,TITLE='Back',VALUE=gback,XSIZE=strlen(gback)>xsize,/RETURN_EVENTS)
        endif else $
            g_back = cw_field(frontbase,TITLE='Back',VALUE=gback,XSIZE=30,/RETURN_EVENTS)
    endif






    if g_replace gt -1 then begin
        replacebase = widget_base(intbase,/ROW)
        g_find = cw_field(replacebase,TITLE='Find',XSIZE=30)
        g_replace = cw_field(replacebase,TITLE='Replace',XSIZE=30,/RETURN_EVENTS)
    endif
    return,butcol
end
pro cwboss
    common getint_comm
    strs = string(gvalues)
    strscopy = strs
    maxlen = max(strlen(strs))
    if maxlen gt xsize then xsize = fix(maxlen*1.5)

    ;create_widgets
    ;geo = widget_info(intbase,/GEOMETRY)
    ;START150828
    butcol=create_widgets()
    geo = widget_info(butcol,/GEOMETRY)

    xdim = geo.scr_xsize + 2*geo.margin
    ydim = geo.scr_ysize + 2*geo.margin
    if xdim gt !DISPXMAX or ydim gt !DISPYMAX then begin
        ;print,'get_str0 xdim=',xdim,' ydim=',ydim,' !DISPXMAX=',!DISPXMAX,' !DISPYMAX=',!DISPYMAX
        widget_control,intbase,/DESTROY

        ;create_widgets,X_SCROLL_SIZE=xdim,Y_SCROLL_SIZE=ydim
        ;START150828
        ;butcol=create_widgets(X_SCROLL_SIZE=xdim,Y_SCROLL_SIZE=ydim)
        ;START160401
        butcol=create_widgets(xdim,ydim)

    endif
    widget_control,intbase,/REALIZE
    widget_control,g_ok,/INPUT_FOCUS
    widget_control,intbase,/SHOW
    xmanager,'get_str',intbase
end
function get_str,number,labels,values,WIDTH=width,TITLE=title,LABEL=label,GROUP_LEADER=group_leader,GO_BACK=go_back,EXIT=exit, $
    ONE_COLUMN=one_column,BELOW=below,LONGTEXT=longtext,LONGLABEL=longlabel,SPECIAL=special,FRONT=front,CR=cr,REPLACE=replace, $
    ADD=add,BACK=back,RESET=reset,ALIGN_LEFT=align_left,LEFT=left
    common getint_comm
    if keyword_set(GROUP_LEADER) then leader = group_leader else leader = !FMRI_LEADER
    if keyword_set(WIDTH) then xsize = (width<100) else xsize = 15
    if keyword_set(TITLE) then gtitle = title else gtitle = ''
    if not keyword_set(LABEL) then glabel = '' else glabel=label
    if not keyword_set(ONE_COLUMN) then gone_column = 0 else gone_column = 1
    if not keyword_set(BELOW) then gbelow = 0 else gbelow = 1
    if not keyword_set(LONGTEXT) then glongtext = '' else glongtext = longtext
    if not keyword_set(LONGLABEL) then glonglabel = '' else glonglabel = longlabel
    if not keyword_set(GO_BACK) then g_go_back = -1 else g_go_back = 1
    if not keyword_set(EXIT) then g_exit = -1 else g_exit = 1
    if number eq 1 then g_cr = 1 else g_cr = -1
    if not keyword_set(REPLACE) then g_replace = -1 else g_replace = 1
    if not keyword_set(ADD) then begin
        g_add = -1 
        gadd=0
        gvalues_init = values[0]
    endif else begin
        g_add = 1
        gadd=add
        gvalues_init = values[0:add-1]
    endelse
    val_delpath=''
    val_reset=''
    if keyword_set(FRONT) or keyword_set(BACK) then begin
        g_reset = 1
        g_delpath = 1
        val_reset = 'Reset'
        val_delpath='Delete'
    endif else if keyword_set(RESET) then begin
        g_reset = 1
        val_reset = 'Reset'
        g_delpath = -1
    endif else begin
        g_reset = -1
        g_delpath = -1
    endelse
    if not keyword_set(FRONT) then begin
        g_front = -1
    endif else begin
        g_front = 1
        path = strpos(front,'/')
        if path ne -1 then val_delpath='Delete path'
        if path ne -1 or keyword_set(BACK) then begin
            frontt = 'Front'
        endif else $
            frontt = 'Replace'
        gfront = front
    endelse
    frontstr = ''
    if not keyword_set(BACK) then begin
        g_back = -1
    endif else begin
        g_back = 1
        gback = back 
    endelse
    backstr = ''
    if not keyword_set(SPECIAL) then begin
        gspecial = ''
        g_special = -1
    endif else begin 
        gspecial = special
        g_special = 1
    endelse
    nstr = number
    glabels = labels
    gvalues = values
    if keyword_set(ALIGN_LEFT) then galign_left=1 else galign_left=0
    if keyword_set(LEFT) then gleft=1 else gleft=0
    cwboss
    return,strs
end
