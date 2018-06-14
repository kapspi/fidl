#Copyright 7/2/03 Washington University.  All Rights Reserved.
#make_fidl_rt.csh  $Revision: 1.46 $
if($#argv < 4) then

    #echo Usage: make_fidl_rt.csh LIB_PATH INSTALL_PATH FIDL_RT_PATH IDL_DIR LICENSE_FILE
    #START160720
    echo Usage: make_fidl_rt.csh LIB_PATH INSTALL_PATH FIDL_RT_PATH IDL_DIR VM_DIR LICENSE_FILE

    exit
endif
set LIB_PATH = $1
set INSTALL_PATH = $2
set FIDL_RT_PATH = $3
set IDL_DIR = $4

#set LICENSE_FILE = $5
#START160720
set VM_DIR = $5
set LICENSE_FILE = $6

set FILENAME = ${INSTALL_PATH}/scripts/fidl_rt
echo "#\!/bin/csh" > $FILENAME
echo setenv IDL_DIR $IDL_DIR >> $FILENAME 

#cat $FIDL_RT_PATH/fidl_rt.txt >> $FILENAME 
#START180118
tail -n +3 $FIDL_RT_PATH/fidl_rt.txt >> $FILENAME


#echo 'setenv FIDL_MASK_FILE_111 ' ${LIB_PATH}'/glm_atlas_mask_111.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_MASK_FILE_111ASIGMA ' ${LIB_PATH}'/averaged_Asigma_msk.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_MASK_FILE_222 ' ${LIB_PATH}'/glm_atlas_mask_222.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_MASK_FILE_333 ' ${LIB_PATH}'/glm_atlas_mask_333.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_ATLAS_FILE_111 ' ${LIB_PATH}'/711-2B_111.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_ATLAS_FILE_222 ' ${LIB_PATH}'/711-2B_b9.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_ATLAS_FILE_333 ' ${LIB_PATH}'/711-2B_b9_333.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_ATLAS_711_2C_111 ' ${LIB_PATH}'/711-2C_111.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_ATLAS_711_2C_222 ' ${LIB_PATH}'/young6+6_atrophy_avg_222.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_ATLAS_711_2O_111 ' ${LIB_PATH}'/711-2O_111.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_ATLAS_711_2O_222 ' ${LIB_PATH}'/atrophy_avg_222.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_ATLAS_711_2Y_111 ' ${LIB_PATH}'/young6+6_avg_111.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_ATLAS_711_2Y_222 ' ${LIB_PATH}'/young6+6_avg_222.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_ATLAS_711_2K_111 ' ${LIB_PATH}'/789yo_atlas_111.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_COLIN_222 ' ${LIB_PATH}'/colin_avg20_Toga_LR_full_sMRI_xflip_on_711-2B_222.4dfp.img' >> $FILENAME 
#echo 'setenv PIXMAP_PATH ' ${LIB_PATH}'/pixmaps' >> $FILENAME 
#echo 'setenv FIDL_SMALL_WM ' ${LIB_PATH}'/small_WM.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_LAT_VENT ' ${LIB_PATH}'/711-2B_lat_vent_333.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_TRIO_Y_NDC_111 ' ${LIB_PATH}'/TRIO_Y_NDC.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_TRIO_Y_NDC_222 ' ${LIB_PATH}'/TRIO_Y_NDC_222.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_TRIO_Y_NDC_333 ' ${LIB_PATH}'/TRIO_Y_NDC_333.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_TRIO_KY_NDC_111 ' ${LIB_PATH}'/TRIO_KY_NDC_111.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_TRIO_KY_NDC_222 ' ${LIB_PATH}'/TRIO_KY_NDC_222.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_TRIO_KY_NDC_333 ' ${LIB_PATH}'/TRIO_KY_NDC_333.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_MASK_MNI222 ' ${LIB_PATH}'/MNI_T1_2mm_dilbrainmask.4dfp.img' >> $FILENAME 
#echo 'setenv FIDL_T1_MNI222 ' ${LIB_PATH}'/MNI_T1_2mm.4dfp.4dfp.img' >> $FILENAME 
#START180118
tail -n +3 $FIDL_RT_PATH/fidl_rt2.txt >> $FILENAME 



#echo 'setenv FIDL_LINECOLORSFILE ' ${INSTALL_PATH}'/idl/linecolors.dat' >> $FILENAME 
#START160720
echo 'setenv FIDL_LINECOLORSFILE ' ${INSTALL_PATH}'/idl_6.3/linecolors.dat' >> $FILENAME 

echo "setenv FIDL_BIN_PATH " ${INSTALL_PATH}/bin >> $FILENAME 
echo "setenv FIDL_SHAREABLE_OBJ_LIB " ${INSTALL_PATH}/bin/fidl.so >> $FILENAME 
echo "setenv FIDL_BIN_PATH_LINUX " ${INSTALL_PATH}/bin_linux >> $FILENAME 
echo "setenv FIDL_SHAREABLE_OBJ_LIB_LINUX " ${INSTALL_PATH}/bin_linux/fidl.so >> $FILENAME 
echo "setenv FIDL_BIN_PATH_LINUX64 " ${INSTALL_PATH}/bin_linux64 >> $FILENAME 

echo "setenv FIDL_SHAREABLE_OBJ_LIB_LINUX64 " ${INSTALL_PATH}/bin_linux64/fidl.so >> $FILENAME 
#START150413
echo "setenv FIDL_SHAREABLE_OBJ_LIB_LINUX64_NEW " ${INSTALL_PATH}/bin_linux64/libfidl.so >> $FILENAME 

#START150206
echo "setenv FIDL_SO64CIFTI " ${INSTALL_PATH}/bin_linux64/libfidlcifti.so >> $FILENAME 

#START160722
echo 'setenv FIDL_MONTECARLO_PATH ' ${LIB_PATH}'/montecarlo' >> $FILENAME 




echo 'set gsldir = '${INSTALL_PATH}/bin >> $FILENAME
echo 'set goose = -32' >> $FILENAME
echo 'if(`uname` == Linux) then' >> $FILENAME
echo '    set dog = `uname -a`' >> $FILENAME
echo '    set cat = `expr $#dog - 1`' >> $FILENAME
echo '    if($dog[$cat] == x86_64) then' >> $FILENAME
echo '        set goose' >> $FILENAME
echo '        set gsldir = '${INSTALL_PATH}/bin_linux64 >> $FILENAME
echo '    else' >> $FILENAME
echo '        set gsldir = '${INSTALL_PATH}/bin_linux >> $FILENAME
echo '    endif' >> $FILENAME
echo '    set VM = TRUE' >> $FILENAME
echo 'endif' >> $FILENAME

echo 'if($VM == TRUE) then' >> $FILENAME

echo '    set IDL2 = idl_6.3' >> $FILENAME
#START151111
#echo '    set IDL2 = idl_8.3' >> $FILENAME

echo '    if(`uname` == Linux) then' >> $FILENAME

#echo '        setenv IDL_DIR /home/usr/fidl/vm/idl71linux' >> $FILENAME
#START160720
echo '        setenv IDL_DIR ' $VM_DIR >> $FILENAME

echo '    else' >> $FILENAME
echo '        setenv IDL_DIR /home/usr/fidl/vm/idl71solaris2' >> $FILENAME
echo '    endif' >> $FILENAME
echo 'else' >> $FILENAME
echo '    set IDL2 = idl' >> $FILENAME
echo '    setenv IDL_DIR /usr/local/pkg/idl_5.3' >> $FILENAME
echo '    setenv IDL_STARTUP /home/usr/fidl/fidl_code/fidl_2.65/idl/idl_startup.pro' >> $FILENAME
echo '    setenv LICENSE_FILE /usr/local/pkg/idl_5.3/license.dat' >> $FILENAME
echo 'endif' >> $FILENAME



echo 'if ( ${?LD_LIBRARY_PATH} ) then' >> $FILENAME
echo '   setenv LD_LIBRARY_PATH ${gsldir}:$LD_LIBRARY_PATH' >> $FILENAME
echo 'else' >> $FILENAME
echo '   setenv LD_LIBRARY_PATH $gsldir' >> $FILENAME
echo 'endif' >> $FILENAME
echo 'setenv IDL_LIB $IDL_DIR/lib' >> $FILENAME
echo 'setenv IDL_PATH +'${INSTALL_PATH}'/${IDL2}:+$IDL_DIR/lib' >> $FILENAME
cat $FIDL_RT_PATH/fidl_license.txt >> $FILENAME
echo 'if($VM == TRUE) then' >> $FILENAME
echo '    ${IDL_DIR}/bin/idl -vm='${INSTALL_PATH}'/${IDL2}/main.sav $goose' >> $FILENAME
echo 'else' >> $FILENAME
echo '    if($CMD_LINE == TRUE) then'  >> $FILENAME
echo '        ${IDL_DIR}/bin/$IDL2 $goose'  >> $FILENAME
echo '    else' >> $FILENAME
echo '        ${IDL_DIR}/bin/idl -rt='${INSTALL_PATH}'/${IDL2}/main.sav $goose' >> $FILENAME
echo '    endif' >> $FILENAME
echo 'endif' >> $FILENAME
chmod +x $FILENAME
