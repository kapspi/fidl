;Copyright 12/31/99 Washington University.  All Rights Reserved.
;stats_init.pro  $Revision: 12.121 $
pro stats_init,dsp,fi,pr,fl,st,wd,pref
common fidl_batch,macro_commands,num_macro_cmd,lcbatch,batch_base,batch_id,batch_name,macro_descriptor,num_macro_dsc,batch_pro
defsysv,'!COLOR_TABLE',getenv('FIDL_COLOR_TABLES')
defsysv,'!BIN',getenv('FIDL_BIN_PATH')
defsysv,'!BINLINUX',getenv('FIDL_BIN_PATH_LINUX')
defsysv,'!BINLINUX64',getenv('FIDL_BIN_PATH_LINUX64')
defsysv,'!HOME',getenv('HOME')
defsysv,'!PLOTFILE',!HOME+'/idl.ps'
defsysv,'!MASK_FILE_111',getenv('FIDL_MASK_FILE_111')
defsysv,'!MASK_FILE_111ASIGMA',getenv('FIDL_MASK_FILE_111ASIGMA')
defsysv,'!MASK_FILE_222',getenv('FIDL_MASK_FILE_222')
defsysv,'!MASK_FILE_333',getenv('FIDL_MASK_FILE_333')
defsysv,'!ATLAS_FILE_111',getenv('FIDL_ATLAS_FILE_111')
defsysv,'!ATLAS_FILE_222',getenv('FIDL_ATLAS_FILE_222')
defsysv,'!ATLAS_FILE_333',getenv('FIDL_ATLAS_FILE_333')
defsysv,'!ATLAS_711_2C_111',getenv('FIDL_ATLAS_711_2C_111')
defsysv,'!ATLAS_711_2C_222',getenv('FIDL_ATLAS_711_2C_222')
defsysv,'!ATLAS_711_2O_111',getenv('FIDL_ATLAS_711_2O_111')
defsysv,'!ATLAS_711_2O_222',getenv('FIDL_ATLAS_711_2O_222')
defsysv,'!ATLAS_711_2Y_111',getenv('FIDL_ATLAS_711_2Y_111')
defsysv,'!ATLAS_711_2Y_222',getenv('FIDL_ATLAS_711_2Y_222')
defsysv,'!ATLAS_711_2K_111',getenv('FIDL_ATLAS_711_2K_111')
defsysv,'!COLIN_222',getenv('FIDL_COLIN_222')
defsysv,'!BLANK','----------------------------------------------'
defsysv,'!PIXMAP_PATH',getenv('PIXMAP_PATH')
defsysv,'!SMALL_WM',getenv('FIDL_SMALL_WM')
defsysv,'!LAT_VENT',getenv('FIDL_LAT_VENT')
defsysv,'!LINECOLORSFILE',getenv('FIDL_LINECOLORSFILE')
defsysv,'!TRIO_Y_NDC_111',getenv('FIDL_TRIO_Y_NDC_111')
defsysv,'!TRIO_Y_NDC_222',getenv('FIDL_TRIO_Y_NDC_222')
defsysv,'!TRIO_Y_NDC_333',getenv('FIDL_TRIO_Y_NDC_333')
defsysv,'!TRIO_KY_NDC_111',getenv('FIDL_TRIO_KY_NDC_111')
defsysv,'!TRIO_KY_NDC_222',getenv('FIDL_TRIO_KY_NDC_222')
defsysv,'!TRIO_KY_NDC_333',getenv('FIDL_TRIO_KY_NDC_333')
defsysv,'!MASK_MNI222',getenv('FIDL_MASK_MNI222')
defsysv,'!T1_MNI222',getenv('FIDL_T1_MNI222')
defsysv,'!MONTECARLO_PATH',getenv('FIDL_MONTECARLO_PATH')


;START180118
defsysv,'!mni_icbm152_t1_tal_nlin_sym_09a',getenv('FIDL_mni_icbm152_t1_tal_nlin_sym_09a')
defsysv,'!mni_icbm152_t1_tal_nlin_sym_09a_mask',getenv('FIDL_mni_icbm152_t1_tal_nlin_sym_09a_mask')


if getenv('FIDL_ARCANE') eq "TRUE" then defsysv,'!FIDL_ARCANE',!TRUE else defsysv,'!FIDL_ARCANE',!FALSE
defsysv,'!DISPXMAX',long(getenv('FIDL_DISPX'))
defsysv,'!DISPYMAX',long(getenv('FIDL_DISPY'))
print,' !DISPXMAX=',strtrim(!DISPXMAX,2),' !DISPYMAX=',strtrim(!DISPYMAX,2)
if !VERSION.OS_FAMILY eq 'Windows' then $
    defsysv,'!SLASH','\' $
else $
    defsysv,'!SLASH','/'
for i=0,!MAX_WORKING_WINDOWS-1 do begin
    dsp[i].low_bandwidth = !FALSE  ; Assume net login.
    dsp[i].white = !LEN_COLORTAB
    dsp[i].yellow = !LEN_COLORTAB+1
    dsp[i].red = !LEN_COLORTAB+2
    dsp[i].green = !LEN_COLORTAB+3
    dsp[i].blue = !LEN_COLORTAB+4
    dsp[i].black = !LEN_COLORTAB+9
    dsp[i].orientation = pref.orientation
    dsp[i].img_minmax(1) = 255
endfor
pr.color = dsp[0].green
defsysv,'!WHITE',dsp[0].white
defsysv,'!YELLOW',dsp[0].yellow
defsysv,'!GREEN',dsp[0].green
defsysv,'!RED',dsp[0].red
defsysv,'!BLUE',dsp[0].blue
defsysv,'!BLACK',dsp[0].black
defsysv,'!PURPLE',!LEN_COLORTAB+6
defsysv,'!YELLOW3',!LEN_COLORTAB+7
defsysv,'!RED3',!LEN_COLORTAB+8
defsysv,'!GREEN8',!LEN_COLORTAB+9
defsysv,'!BLUE3',!LEN_COLORTAB+10
defsysv,'!ORANGE',!LEN_COLORTAB+11
defsysv,'!PURPLE1',!LEN_COLORTAB+12
defsysv,'!YELLOW10',!LEN_COLORTAB+13
defsysv,'!RED1',!LEN_COLORTAB+14
batch_base = -1

fi.color_min1 = 0
fi.color_max1 = 100
fi.color_gamma1 = 1.
fi.color_min2 = 0
fi.color_max2 = 100
fi.color_gamma2 = 1.

wd.draw[*] = -1
wd.wkg_base[*] = -1

imgtype = !CTI
fl.custom = !FALSE

fi.path = !PWD
wd.oprof = -12345       ; Nonsense widget value.
pr.prof_on = !PROF_NO
fit_name = 'null'
fi.printfile = !HOME + '/idl.ps'
fi.secondary[*] = -1

sig = fltarr(!GLMSIM_MAX_STIMLEN)
for i=0,!GLMSIM_MAX_STIMLEN-1 do begin
    t = pref.TR*(i + .5)
    tp = t-!HRF_DELTA
    if(tp lt 0) then $
        sig[i] = 0. $
    else $
        sig[i] = ((tp/!HRF_TAU)^2)*exp(-tp/!HRF_TAU)
endfor
sig[*] = sig[*]/max(sig[*])
for eff=0,!GLMSIM_MAXEFF-1 do begin
    pref.glmsim_stimlen[eff] = !GLMSIM_MAX_STIMLEN
    pref.glmsim_sigstr[eff] = 1.
    for i=0,!GLMSIM_MAX_STIMLEN-1 do $
        pref.glmsim_sig[*,eff] = sig
endfor

end
