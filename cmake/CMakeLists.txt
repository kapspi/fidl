#cmake_minimum_required(VERSION 2.8)
cmake_minimum_required(VERSION 2.6)
#
# Name of Project
#
PROJECT(fidl)

#START150423
##SET(LIBS0 ${LIBS})

#SET(CMAKE_CXX_FLAGS "-W -Wall")
#START150522
#SET(CMAKE_CXX_FLAGS "-W -Wall -O3")
SET(CMAKE_CXX_FLAGS "-W -Wall -Wextra -O3")
#SET(CMAKE_C_FLAGS "-W -Wall -O3 -std=c99")
SET(CMAKE_C_FLAGS "-W -Wall -Wextra -O3 -std=c99")

SET(IGNORE_QT FALSE CACHE BOOL "don't try to use QT")

#
# Use QT for xml/string
#
IF (NOT IGNORE_QT)
    FIND_PACKAGE(Qt4 4.8)
    IF (QT_FOUND)
        SET(QT_DONT_USE_QTGUI TRUE)
        SET(CIFTILIB_USE_QT TRUE)
        ADD_DEFINITIONS(-DCIFTILIB_USE_QT)
        INCLUDE(${QT_USE_FILE})
        SET(LIBS ${LIBS} ${QT_LIBRARIES})
    ENDIF (QT_FOUND)
ENDIF (NOT IGNORE_QT)

IF (NOT QT_FOUND)
    SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
    FIND_PACKAGE(libxml++ REQUIRED)
    INCLUDE_DIRECTORIES(${libxml++_INCLUDE_DIRS})
    SET(LIBS ${LIBS} ${libxml++_LIBRARIES})
    ADD_DEFINITIONS(-DCIFTILIB_USE_XMLPP)
ENDIF (NOT QT_FOUND)

IF (NOT QT_FOUND)
    FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem system)
ELSE (NOT QT_FOUND)
    FIND_PACKAGE(Boost REQUIRED)
ENDIF (NOT QT_FOUND)
SET(LIBS ${LIBS} ${Boost_LIBRARIES})
IF (Boost_VERSION LESS 104400)
    #absolute() was added in 1.44.0, with filesystem v3
    ADD_DEFINITIONS(-DCIFTILIB_BOOST_NO_FSV3)
ENDIF (Boost_VERSION LESS 104400)
IF (Boost_VERSION LESS 104800)
    #canonical() was added in 1.48.0
    ADD_DEFINITIONS(-DCIFTILIB_BOOST_NO_CANONICAL)
ENDIF (Boost_VERSION LESS 104800)
IF (Boost_VERSION LESS 105600)
    #try_lexical_cast was added in 1.56.0
    ADD_DEFINITIONS(-DCIFTILIB_BOOST_NO_TRY_LEXICAL)
ENDIF (Boost_VERSION LESS 105600)

#
# Need XML from Qt
#
#SET(QT_DONT_USE_QTGUI)

#
# Add QT for includes
#
#INCLUDE(${QT_USE_FILE})

#FIND_LIBRARY(CIFTI NAMES Cifti libCifti PATHS /home/hannah/mcavoy/FIDL/cifti/build/Cifti)
#FIND_LIBRARY(NIFTI NAMES Nifti libNifti PATHS /home/hannah/mcavoy/FIDL/cifti/build/Nifti)
#FIND_LIBRARY(COMMON NAMES Common libCommon PATHS /home/hannah/mcavoy/FIDL/cifti/build/Common)
#FIND_LIBRARY(GSL NAMES gsl libgsl PATHS /home/hannah/mcavoy/FIDL/gsl1.16/lib)
#FIND_LIBRARY(GSLCBLAS NAMES gslcblas libgslcblas PATHS /home/hannah/mcavoy/FIDL/gsl1.16/lib)
#START150424
add_library(Cifti STATIC IMPORTED)
set_property(TARGET Cifti PROPERTY IMPORTED_LOCATION /home/hannah/mcavoy/FIDL/cifti/build/Cifti/libCifti.a)
add_library(Nifti STATIC IMPORTED)
set_property(TARGET Nifti PROPERTY IMPORTED_LOCATION /home/hannah/mcavoy/FIDL/cifti/build/Nifti/libNifti.a)
add_library(Common STATIC IMPORTED)
set_property(TARGET Common PROPERTY IMPORTED_LOCATION /home/hannah/mcavoy/FIDL/cifti/build/Common/libCommon.a)
add_library(Gsl STATIC IMPORTED)
set_property(TARGET Gsl PROPERTY IMPORTED_LOCATION /home/hannah/mcavoy/FIDL/gsl1.16/lib/libgsl.a)
add_library(Gslcblas STATIC IMPORTED)
set_property(TARGET Gslcblas PROPERTY IMPORTED_LOCATION /home/hannah/mcavoy/FIDL/gsl1.16/lib/libgslcblas.a)
 
#START151113
add_library(cifti SHARED IMPORTED)
set_property(TARGET cifti PROPERTY IMPORTED_LOCATION /home/hannah/mcavoy/FIDL/cifti/build/Cifti/libcifti.so)
add_library(nifti SHARED IMPORTED)
set_property(TARGET nifti PROPERTY IMPORTED_LOCATION /home/hannah/mcavoy/FIDL/cifti/build/Nifti/libnifti.so)
add_library(common SHARED IMPORTED)
set_property(TARGET common PROPERTY IMPORTED_LOCATION /home/hannah/mcavoy/FIDL/cifti/build/Common/libcommon.so)

#START151113
add_library(gsl SHARED IMPORTED)
set_property(TARGET gsl PROPERTY IMPORTED_LOCATION /home/hannah/mcavoy/FIDL/gsl1.16/lib/libgsl.so)
add_library(gslcblas SHARED IMPORTED)
set_property(TARGET gslcblas PROPERTY IMPORTED_LOCATION /home/hannah/mcavoy/FIDL/gsl1.16/lib/libgslcblas.so)

ADD_LIBRARY(fidl SHARED
/home/hannah/mcavoy/idl/clib/gauss_smoth2.c
/home/hannah/mcavoy/idl/clib/t4_atlas.c
/home/hannah/mcavoy/idl/clib/t_to_z.c
/home/hannah/mcavoy/idl/clib/f_to_z.c
/home/hannah/mcavoy/idl/clib/zoom_by_2n.c
/home/hannah/mcavoy/idl/clib/shouldiswap.c
/home/hannah/mcavoy/idl/clib/timesframesoffsets.c
/home/hannah/mcavoy/idl/clib/subs_util.c
/home/hannah/mcavoy/idl/clib/strutil.c
/home/hannah/mcavoy/idl/clib/read_data.c
/home/hannah/mcavoy/idl/clib/d2double.c
/home/hannah/mcavoy/idl/clib/trend.c
/home/hannah/mcavoy/idl/clib/spatial_extent2.c
/home/hannah/mcavoy/idl/clib/Acond_norm1.c
/home/hannah/mcavoy/idl/clib/cond_norm1.c
/home/hannah/mcavoy/idl/clib/boynton_model.c
/home/hannah/mcavoy/idl/clib/boynton_contrast.c
/home/hannah/mcavoy/idl/clib/count_tab.c
/home/hannah/mcavoy/idl/clib/assign_region_names2.c
/home/hannah/mcavoy/idl/clib/get_limits.c
/home/hannah/mcavoy/idl/clib/gsl_svd.c
/home/hannah/mcavoy/idl/clib/Agsl_svd.c
/home/hannah/mcavoy/idl/clib/make_timestr2.c
/home/hannah/mcavoy/idl/clib/sphere_nvox.c
/home/hannah/mcavoy/idl/clib/count_estimates.c
/home/hannah/mcavoy/idl/clib/orthogonalize.c
/home/hannah/mcavoy/idl/clib/interfile.c
/home/hannah/mcavoy/idl/clib/center_of_mass2.c
/home/hannah/mcavoy/idl/clib/get_atlas_coor.c
/home/hannah/mcavoy/idl/clib/d2charvar.c
/home/hannah/mcavoy/idl/clib/get_atlas.c
/home/hannah/mcavoy/idl/clib/r_to_z.c
/home/hannah/mcavoy/idl/clib/read_xform.c
/home/hannah/mcavoy/idl/clib/get_atlas_param.c
/home/hannah/mcavoy/idl/clib/twoA_or_twoB.c
/home/hannah/mcavoy/idl/clib/AtoATAgsl_svd.c
/home/hannah/mcavoy/idl/clib/checkOS.c
/home/hannah/mcavoy/idl/clib/subs_mask.c)
TARGET_LINK_LIBRARIES(fidl gsl gslcblas)

ADD_LIBRARY(Fidl
/home/hannah/mcavoy/GITHUB/c/interfile.c

#START180316
#/home/hannah/mcavoy/GITHUB/c/extract_regions.c

/home/hannah/mcavoy/GITHUB/c/utilities.c
/home/hannah/mcavoy/GITHUB/c/twoA_or_twoB.c
/home/hannah/mcavoy/GITHUB/c/get_atlas_param.c
/home/hannah/mcavoy/GITHUB/c/get_atlas.c

#START180316
#/home/hannah/mcavoy/GITHUB/c/find_regions_by_file_cover.c

/home/hannah/mcavoy/GITHUB/c/d2charvar.c
/home/hannah/mcavoy/GITHUB/c/files_struct.c
/home/hannah/mcavoy/GITHUB/c/shouldiswap.c
/home/hannah/mcavoy/GITHUB/c/machine_epsilon.c
/home/hannah/mcavoy/GITHUB/c/checkOS.c
/home/hannah/mcavoy/GITHUB/c/make_timestr2.c
/home/hannah/mcavoy/GITHUB/c/read_xform.c
/home/hannah/mcavoy/GITHUB/c/subs_util.c
/home/hannah/mcavoy/GITHUB/c/minmax.c
/home/hannah/mcavoy/GITHUB/c/d2intvar.c
/home/hannah/mcavoy/GITHUB/c/strutil.c)

ADD_LIBRARY(fidlc SHARED
/home/hannah/mcavoy/idl/clib/interfile.c
/home/hannah/mcavoy/idl/clib/map_disk.c
/home/hannah/mcavoy/idl/clib/extract_regions.c
/home/hannah/mcavoy/idl/clib/utilities.c
/home/hannah/mcavoy/idl/clib/twoA_or_twoB.c
/home/hannah/mcavoy/idl/clib/get_atlas_param.c
/home/hannah/mcavoy/idl/clib/get_atlas.c
/home/hannah/mcavoy/idl/clib/find_regions_by_file_cover.c
/home/hannah/mcavoy/idl/clib/d2charvar.c
/home/hannah/mcavoy/idl/clib/files_struct.c
/home/hannah/mcavoy/idl/clib/subs_mask.c
/home/hannah/mcavoy/idl/clib/shouldiswap.c
/home/hannah/mcavoy/idl/clib/get_limits.c
/home/hannah/mcavoy/idl/clib/machine_epsilon.c
/home/hannah/mcavoy/idl/clib/checkOS.c
/home/hannah/mcavoy/idl/clib/make_timestr2.c
/home/hannah/mcavoy/idl/clib/subs_glm.c
/home/hannah/mcavoy/idl/clib/read_xform.c
/home/hannah/mcavoy/idl/clib/subs_util.c
/home/hannah/mcavoy/idl/clib/minmax.c
/home/hannah/mcavoy/idl/clib/d2double.c
/home/hannah/mcavoy/idl/clib/d2intvar.c
/home/hannah/mcavoy/idl/clib/strutil.c
/home/hannah/mcavoy/idl/clib/d2float.c)
TARGET_LINK_LIBRARIES(fidlc gsl gslcblas)

ADD_EXECUTABLE(fidl_sleep /home/hannah/mcavoy/idl/clib/fidl_sleep.c)
TARGET_LINK_LIBRARIES(fidl_sleep m Fidl)
#TARGET_LINK_LIBRARIES(fidl_sleep fidlc)

ADD_EXECUTABLE(fidl_power120 /home/hannah/mcavoy/idl/clib/fidl_power120.c)
TARGET_LINK_LIBRARIES(fidl_power120 fidlc)

#ADD_EXECUTABLE(fidl_chloe /home/hannah/mcavoy/idl/clib/fidl_chloe.c)
ADD_EXECUTABLE(fidl_chloe /home/hannah/mcavoy/GITHUB/c/fidl_chloe.c)
TARGET_LINK_LIBRARIES(fidl_chloe m Fidl)

ADD_EXECUTABLE(fidl_zstat2
/home/hannah/mcavoy/GITHUB/c/get_grand_mean_struct.c
/home/hannah/mcavoy/GITHUB/c/d2double.c
/home/hannah/mcavoy/GITHUB/c/d2float.c
/home/hannah/mcavoy/GITHUB/c/subs_glm.c
/home/hannah/mcavoy/GITHUB/c/t4_atlas.c
/home/hannah/mcavoy/GITHUB/c/gauss_smoth2.c
/home/hannah/mcavoy/GITHUB/c/tags_tc_glmstack.c
/home/hannah/mcavoy/GITHUB/c/boynton_contrast.c
/home/hannah/mcavoy/GITHUB/c/boynton_model.c
/home/hannah/mcavoy/GITHUB/c/t_to_z.c
/home/hannah/mcavoy/GITHUB/c/make_scratchdir.c
/home/hannah/mcavoy/GITHUB/c/filetype.c
/home/hannah/mcavoy/GITHUB/c/subs_nifti.cxx
/home/hannah/mcavoy/GITHUB/c/subs_cifti.cxx
/home/hannah/mcavoy/GITHUB/c/write1.c
/home/hannah/mcavoy/GITHUB/c/nifti_gethdr.cxx
/home/hannah/mcavoy/GITHUB/c/get_atlas_param.c
/home/hannah/mcavoy/GITHUB/c/read_tc_string_new.c
/home/hannah/mcavoy/GITHUB/c/header.cxx
/home/hannah/mcavoy/GITHUB/c/region.cxx
/home/hannah/mcavoy/GITHUB/c/get_limits.c
/home/hannah/mcavoy/GITHUB/c/subs_mask.c
/home/hannah/mcavoy/GITHUB/c/find_regions_by_file_cover.c
/home/hannah/mcavoy/GITHUB/c/get_indbase.c
/home/hannah/mcavoy/GITHUB/c/check_dimensions.cxx
/home/hannah/mcavoy/GITHUB/c/fidl_zstat2.c)
TARGET_LINK_LIBRARIES(fidl_zstat2 Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})

ADD_EXECUTABLE(fidl_bolds
/home/hannah/mcavoy/GITHUB/c/d2double.c
/home/hannah/mcavoy/GITHUB/c/d2float.c
/home/hannah/mcavoy/GITHUB/c/t4_atlas.c
/home/hannah/mcavoy/GITHUB/c/gauss_smoth2.c
/home/hannah/mcavoy/GITHUB/c/read_frames_file.c
/home/hannah/mcavoy/GITHUB/c/make_scratchdir.c
/home/hannah/mcavoy/GITHUB/c/filetype.c
/home/hannah/mcavoy/GITHUB/c/dim_param2.c
/home/hannah/mcavoy/GITHUB/c/read_tc_string_new.c
/home/hannah/mcavoy/GITHUB/c/cifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/subs_nifti.cxx
/home/hannah/mcavoy/GITHUB/c/subs_cifti.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_gethdr.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_flipslice.cxx
/home/hannah/mcavoy/GITHUB/c/write1.c
/home/hannah/mcavoy/GITHUB/c/nifti_class.cxx
/home/hannah/mcavoy/GITHUB/c/header.cxx
/home/hannah/mcavoy/GITHUB/c/subs_glm.c
/home/hannah/mcavoy/GITHUB/c/subs_mask.cxx
/home/hannah/mcavoy/GITHUB/c/find_regions_by_file_cover.c
/home/hannah/mcavoy/GITHUB/c/make_timestr2.c
/home/hannah/mcavoy/GITHUB/c/map_disk.c
/home/hannah/mcavoy/GITHUB/c/region.cxx
/home/hannah/mcavoy/GITHUB/c/check_dimensions.cxx
/home/hannah/mcavoy/GITHUB/c/fidl_bolds.cxx)
TARGET_LINK_LIBRARIES(fidl_bolds Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})


ADD_EXECUTABLE(fidl_junk
#/home/hannah/mcavoy/GITHUB/c/d2double.c
#/home/hannah/mcavoy/GITHUB/c/t4_atlas.c
#/home/hannah/mcavoy/GITHUB/c/gauss_smoth2.c
#/home/hannah/mcavoy/GITHUB/c/read_frames_file.c
#/home/hannah/mcavoy/GITHUB/c/make_scratchdir.c
#/home/hannah/mcavoy/GITHUB/c/dim_param2.c
#/home/hannah/mcavoy/GITHUB/c/read_tc_string_new.c
#/home/hannah/mcavoy/GITHUB/c/cifti_getDimensions.cxx
#/home/hannah/mcavoy/GITHUB/c/nifti_getDimensions.cxx
#/home/hannah/mcavoy/GITHUB/c/subs_nifti.cxx
#/home/hannah/mcavoy/GITHUB/c/subs_cifti.cxx
#/home/hannah/mcavoy/GITHUB/c/nifti_gethdr.cxx
#/home/hannah/mcavoy/GITHUB/c/write1.c
#/home/hannah/mcavoy/GITHUB/c/nifti_class.cxx
#/home/hannah/mcavoy/GITHUB/c/fidl_junk.c)
/home/hannah/mcavoy/GITHUB/c/fidl_junk.cxx)
TARGET_LINK_LIBRARIES(fidl_junk Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})



#ADD_EXECUTABLE(fidl_sumconcpair
#/home/hannah/mcavoy/idl/clib/d2double.c
#/home/hannah/mcavoy/idl/clib/t4_atlas.c
#/home/hannah/mcavoy/idl/clib/dim_param2.c
#/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
#/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx
#/home/hannah/mcavoy/idl/clib/subs_nifti.cxx
#/home/hannah/mcavoy/idl/clib/subs_cifti.cxx
#/home/hannah/mcavoy/idl/clib/write1.c
#/home/hannah/mcavoy/idl/clib/fidl_sumconcpair.c)
#TARGET_LINK_LIBRARIES(fidl_sumconcpair Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})
#START170512
ADD_EXECUTABLE(fidl_sumconcpair
/home/hannah/mcavoy/GITHUB/c/d2double.c
/home/hannah/mcavoy/GITHUB/c/t4_atlas.c
/home/hannah/mcavoy/GITHUB/c/dim_param2.c
/home/hannah/mcavoy/GITHUB/c/cifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/subs_nifti.cxx
/home/hannah/mcavoy/GITHUB/c/subs_cifti.cxx
/home/hannah/mcavoy/GITHUB/c/write1.c
/home/hannah/mcavoy/GITHUB/c/fidl_sumconcpair.c)
TARGET_LINK_LIBRARIES(fidl_sumconcpair Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})

#ADD_EXECUTABLE(fidl_pcat
#/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
#/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx
#/home/hannah/mcavoy/idl/clib/dim_param2.c
#/home/hannah/mcavoy/idl/clib/d2double.c
#/home/hannah/mcavoy/idl/clib/subs_util.c
#/home/hannah/mcavoy/idl/clib/read_frames_file.c
#/home/hannah/mcavoy/idl/clib/subs_nifti.cxx
#/home/hannah/mcavoy/idl/clib/subs_cifti.cxx
#/home/hannah/mcavoy/idl/clib/gsl_svd.c
#/home/hannah/mcavoy/idl/clib/fidl_pcat.c)
#TARGET_LINK_LIBRARIES(fidl_pcat Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})
ADD_EXECUTABLE(fidl_pcat
/home/hannah/mcavoy/GITHUB/c/cifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/dim_param2.c
/home/hannah/mcavoy/GITHUB/c/d2double.c
#/home/hannah/mcavoy/GITHUB/c/subs_util.c
/home/hannah/mcavoy/GITHUB/c/read_frames_file.c
/home/hannah/mcavoy/GITHUB/c/subs_nifti.cxx
/home/hannah/mcavoy/GITHUB/c/subs_cifti.cxx
/home/hannah/mcavoy/GITHUB/c/gsl_svd.c
/home/hannah/mcavoy/GITHUB/c/fidl_pcat.c)
TARGET_LINK_LIBRARIES(fidl_pcat Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})

ADD_EXECUTABLE(compute_glm2
/home/hannah/mcavoy/idl/clib/d2double.c
/home/hannah/mcavoy/idl/clib/dim_param2.c
/home/hannah/mcavoy/idl/clib/subs_glm.c
/home/hannah/mcavoy/idl/clib/gsl_svd.c
/home/hannah/mcavoy/idl/clib/cond_norm1.c
/home/hannah/mcavoy/idl/clib/nrmlrv.c
/home/hannah/mcavoy/idl/clib/write_glm.c
/home/hannah/mcavoy/idl/clib/t4_atlas.c
/home/hannah/mcavoy/idl/clib/gauss_smoth2.c
/home/hannah/mcavoy/idl/clib/f_to_z.c
/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/subs_cifti.cxx
/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/nifti_gethdr.cxx
/home/hannah/mcavoy/idl/clib/subs_nifti.cxx
/home/hannah/mcavoy/idl/clib/nifti_flipslice.cxx
/home/hannah/mcavoy/idl/clib/linmod6.c
/home/hannah/mcavoy/idl/clib/compute_glm2.c)
TARGET_LINK_LIBRARIES(compute_glm2 Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})

#ADD_EXECUTABLE(fidl_anova4
#/home/hannah/mcavoy/idl/clib/d2double.c
#/home/hannah/mcavoy/idl/clib/get_grand_mean_struct.c
#/home/hannah/mcavoy/idl/clib/box_correction.c
#/home/hannah/mcavoy/idl/clib/read_driver2.c
#/home/hannah/mcavoy/idl/clib/gsl_svd.c
#/home/hannah/mcavoy/idl/clib/cond_norm1.c
#/home/hannah/mcavoy/idl/clib/dmatrix_mult.c
#/home/hannah/mcavoy/idl/clib/t4_atlas.c
#/home/hannah/mcavoy/idl/clib/nrutil.c
#/home/hannah/mcavoy/idl/clib/get_atlas_coor.c
#/home/hannah/mcavoy/idl/clib/f_to_z.c
#/home/hannah/mcavoy/idl/clib/inverse.c
#/home/hannah/mcavoy/idl/clib/monte_carlo_mult_comp.c
#/home/hannah/mcavoy/idl/clib/spatial_extent2.c
#/home/hannah/mcavoy/idl/clib/center_of_mass2.c
#/home/hannah/mcavoy/idl/clib/assign_region_names2.c
#/home/hannah/mcavoy/idl/clib/subs_cifti.cxx
#/home/hannah/mcavoy/idl/clib/subs_nifti.cxx
#/home/hannah/mcavoy/idl/clib/write1.c
#/home/hannah/mcavoy/idl/clib/filetype.c
#/home/hannah/mcavoy/idl/clib/read_tc_string_new.c
#/home/hannah/mcavoy/idl/clib/fidl_anova4.c)
#TARGET_LINK_LIBRARIES(fidl_anova4 Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})
#START170706
ADD_EXECUTABLE(fidl_anova4
/home/hannah/mcavoy/GITHUB/c/d2double.c
/home/hannah/mcavoy/GITHUB/c/get_grand_mean_struct.c
/home/hannah/mcavoy/GITHUB/c/box_correction.c
/home/hannah/mcavoy/GITHUB/c/read_driver2.c
/home/hannah/mcavoy/GITHUB/c/gsl_svd.c
/home/hannah/mcavoy/GITHUB/c/cond_norm1.c
/home/hannah/mcavoy/GITHUB/c/dmatrix_mult.c
/home/hannah/mcavoy/GITHUB/c/t4_atlas.c
/home/hannah/mcavoy/GITHUB/c/nrutil.c
/home/hannah/mcavoy/GITHUB/c/get_atlas_coor.c
/home/hannah/mcavoy/GITHUB/c/f_to_z.c
/home/hannah/mcavoy/GITHUB/c/inverse.c
/home/hannah/mcavoy/GITHUB/c/monte_carlo_mult_comp.c
/home/hannah/mcavoy/GITHUB/c/spatial_extent2.c
/home/hannah/mcavoy/GITHUB/c/center_of_mass2.c
/home/hannah/mcavoy/GITHUB/c/assign_region_names2.c
/home/hannah/mcavoy/GITHUB/c/subs_cifti.cxx
/home/hannah/mcavoy/GITHUB/c/subs_nifti.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_gethdr.cxx
/home/hannah/mcavoy/GITHUB/c/write1.c
/home/hannah/mcavoy/GITHUB/c/filetype.c
/home/hannah/mcavoy/GITHUB/c/read_tc_string_new.c
/home/hannah/mcavoy/GITHUB/c/subs_glm.c
/home/hannah/mcavoy/GITHUB/c/subs_mask.c
/home/hannah/mcavoy/GITHUB/c/map_disk.c
/home/hannah/mcavoy/GITHUB/c/d2float.c
/home/hannah/mcavoy/GITHUB/c/fidl_anova4.c)
TARGET_LINK_LIBRARIES(fidl_anova4 Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})

#START150424
ADD_LIBRARY(fidlcifti2 SHARED
/home/hannah/mcavoy/idl/clib/subs_cifti.cxx)
FIND_LIBRARY(CIFTISO NAMES cifti libcifti PATHS /home/hannah/mcavoy/FIDL/cifti/build/Cifti)
FIND_LIBRARY(NIFTISO NAMES nifti libnifti PATHS /home/hannah/mcavoy/FIDL/cifti/build/Nifti)
FIND_LIBRARY(COMMONSO NAMES common libcommon PATHS /home/hannah/mcavoy/FIDL/cifti/build/Common)
#TARGET_LINK_LIBRARIES(fidlcifti2 ${CIFTISO} ${NIFTISO} ${COMMONSO} ${QT_LIBRARIES} ${LIBS})
#START150429
TARGET_LINK_LIBRARIES(fidlcifti2 ${CIFTISO} ${NIFTISO} ${COMMONSO} ${LIBS})
#ADD_EXECUTABLE(fidl_anova_new2
#/home/hannah/mcavoy/idl/clib/fidl_anova_new.c
#/home/hannah/mcavoy/idl/clib/box_correction.c
#/home/hannah/mcavoy/idl/clib/read_driver2.c
#/home/hannah/mcavoy/idl/clib/gsl_svd.c
#/home/hannah/mcavoy/idl/clib/cond_norm1.c
#/home/hannah/mcavoy/idl/clib/dmatrix_mult.c
#/home/hannah/mcavoy/idl/clib/t4_atlas.c
#/home/hannah/mcavoy/idl/clib/nrutil.c
#/home/hannah/mcavoy/idl/clib/get_atlas_coor.c
#/home/hannah/mcavoy/idl/clib/f_to_z.c
#/home/hannah/mcavoy/idl/clib/inverse.c
#/home/hannah/mcavoy/idl/clib/monte_carlo_mult_comp.c
#/home/hannah/mcavoy/idl/clib/spatial_extent2.c
#/home/hannah/mcavoy/idl/clib/center_of_mass2.c
#/home/hannah/mcavoy/idl/clib/assign_region_names2.c)
#TARGET_LINK_LIBRARIES(fidl_anova_new2 Fidl Gsl Gslcblas ${CIFTI} ${NIFTI} ${COMMON} ${LIBS} fidlcifti2)

#ADD_EXECUTABLE(fidl_ttest
#/home/hannah/mcavoy/idl/clib/read_ttest_driver.c 
#/home/hannah/mcavoy/idl/clib/t_to_z.c
#/home/hannah/mcavoy/idl/clib/get_atlas_coor.c
#/home/hannah/mcavoy/idl/clib/monte_carlo_mult_comp.c 
#/home/hannah/mcavoy/idl/clib/spatial_extent2.c
#/home/hannah/mcavoy/idl/clib/center_of_mass2.c
#/home/hannah/mcavoy/idl/clib/assign_region_names2.c
#/home/hannah/mcavoy/idl/clib/subs_cifti.cxx
#/home/hannah/mcavoy/idl/clib/subs_nifti.cxx
#/home/hannah/mcavoy/idl/clib/write1.c
#/home/hannah/mcavoy/idl/clib/dim_param2.c
#/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
#/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx
#/home/hannah/mcavoy/idl/clib/filetype.c
#/home/hannah/mcavoy/idl/clib/d2double.c
#/home/hannah/mcavoy/idl/clib/fidl_ttest.c)
ADD_EXECUTABLE(fidl_ttest
/home/hannah/mcavoy/GITHUB/c/read_ttest_driver.c
/home/hannah/mcavoy/GITHUB/c/t_to_z.c
/home/hannah/mcavoy/GITHUB/c/get_atlas_coor.c
/home/hannah/mcavoy/GITHUB/c/monte_carlo_mult_comp.c
/home/hannah/mcavoy/GITHUB/c/spatial_extent2.c
/home/hannah/mcavoy/GITHUB/c/center_of_mass2.c
/home/hannah/mcavoy/GITHUB/c/assign_region_names2.c
/home/hannah/mcavoy/GITHUB/c/subs_cifti.cxx
/home/hannah/mcavoy/GITHUB/c/subs_nifti.cxx
/home/hannah/mcavoy/GITHUB/c/write1.c
/home/hannah/mcavoy/GITHUB/c/dim_param2.c
/home/hannah/mcavoy/GITHUB/c/cifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/filetype.c
/home/hannah/mcavoy/GITHUB/c/d2double.c
/home/hannah/mcavoy/GITHUB/c/fidl_ttest.c)
TARGET_LINK_LIBRARIES(fidl_ttest Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})

ADD_EXECUTABLE(fidl_avg_zstat2
/home/hannah/mcavoy/GITHUB/c/get_grand_mean_struct.c
/home/hannah/mcavoy/GITHUB/c/d2double.c
/home/hannah/mcavoy/GITHUB/c/subs_glm.c
/home/hannah/mcavoy/GITHUB/c/tags_tc_glmstack.c
/home/hannah/mcavoy/GITHUB/c/boynton_contrast.c
/home/hannah/mcavoy/GITHUB/c/boynton_model.c
/home/hannah/mcavoy/GITHUB/c/t_to_z.c
/home/hannah/mcavoy/GITHUB/c/t4_atlas.c
/home/hannah/mcavoy/GITHUB/c/gauss_smoth2.c
/home/hannah/mcavoy/GITHUB/c/count_tab.c
/home/hannah/mcavoy/GITHUB/c/read_data.c
/home/hannah/mcavoy/GITHUB/c/write1.c
/home/hannah/mcavoy/GITHUB/c/subs_nifti.cxx
/home/hannah/mcavoy/GITHUB/c/subs_cifti.cxx
/home/hannah/mcavoy/GITHUB/c/filetype.c
/home/hannah/mcavoy/GITHUB/c/read_tc_string_new.c
/home/hannah/mcavoy/GITHUB/c/fidl_avg_zstat2.c)
TARGET_LINK_LIBRARIES(fidl_avg_zstat2 Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})

#ADD_EXECUTABLE(compute_residuals 
#/home/hannah/mcavoy/idl/clib/get_grand_mean_struct.c
#/home/hannah/mcavoy/idl/clib/d2double.c
#/home/hannah/mcavoy/idl/clib/t4_atlas.c
#/home/hannah/mcavoy/idl/clib/gauss_smoth2.c
#/home/hannah/mcavoy/idl/clib/nrutil.c
#/home/hannah/mcavoy/idl/clib/tred2.c
#/home/hannah/mcavoy/idl/clib/eigsrt.c
#/home/hannah/mcavoy/idl/clib/tqli.c
#/home/hannah/mcavoy/idl/clib/pythag.c
#/home/hannah/mcavoy/idl/clib/pca.c
#/home/hannah/mcavoy/idl/clib/read_frames_file.c
#/home/hannah/mcavoy/idl/clib/make_scratchdir.c
#/home/hannah/mcavoy/idl/clib/subs_mask.c
#/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
#/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx
#/home/hannah/mcavoy/idl/clib/dim_param2.c
##/home/hannah/mcavoy/idl/clib/write_conc.c
#/home/hannah/mcavoy/idl/clib/read_tc_string_new.c
#/home/hannah/mcavoy/idl/clib/write1.c
#/home/hannah/mcavoy/idl/clib/subs_nifti.cxx
#/home/hannah/mcavoy/idl/clib/subs_cifti.cxx
#/home/hannah/mcavoy/idl/clib/filetype.c
#/home/hannah/mcavoy/idl/clib/compute_residuals.c)
##TARGET_LINK_LIBRARIES(compute_residuals m Fidl Gsl Gslcblas)
##TARGET_LINK_LIBRARIES(compute_residuals fidlc cifti nifti common gsl gslcblas ${LIBS})
##TARGET_LINK_LIBRARIES(compute_residuals Fidl Cifti Nifti Common gsl gslcblas ${LIBS})
#TARGET_LINK_LIBRARIES(compute_residuals Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})

ADD_EXECUTABLE(fidl_residuals
/home/hannah/mcavoy/idl/clib/get_grand_mean_struct.c
/home/hannah/mcavoy/idl/clib/d2double.c
/home/hannah/mcavoy/idl/clib/t4_atlas.c
/home/hannah/mcavoy/idl/clib/gauss_smoth2.c
/home/hannah/mcavoy/idl/clib/nrutil.c
/home/hannah/mcavoy/idl/clib/tred2.c
/home/hannah/mcavoy/idl/clib/eigsrt.c
/home/hannah/mcavoy/idl/clib/tqli.c
/home/hannah/mcavoy/idl/clib/pythag.c
/home/hannah/mcavoy/idl/clib/pca.c
/home/hannah/mcavoy/idl/clib/read_frames_file.c
/home/hannah/mcavoy/idl/clib/make_scratchdir.c
/home/hannah/mcavoy/idl/clib/subs_mask.c
/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/dim_param2.c
#/home/hannah/mcavoy/idl/clib/write_conc.c
/home/hannah/mcavoy/idl/clib/read_tc_string_new.c
/home/hannah/mcavoy/idl/clib/write1.c
/home/hannah/mcavoy/idl/clib/subs_nifti.cxx
/home/hannah/mcavoy/idl/clib/subs_cifti.cxx
/home/hannah/mcavoy/idl/clib/filetype.c
/home/hannah/mcavoy/idl/clib/fidl_residuals.c)
TARGET_LINK_LIBRARIES(fidl_residuals Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})

ADD_EXECUTABLE(fidl_mvpa4
/home/hannah/mcavoy/idl/clib/subs_mask.c
/home/hannah/mcavoy/idl/clib/dim_param2.c
/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/get_atlas_coor.c
/home/hannah/mcavoy/idl/clib/svm.cpp
/home/hannah/mcavoy/idl/clib/cond_norm1.c
/home/hannah/mcavoy/idl/clib/gsl_svd.c
/home/hannah/mcavoy/idl/clib/d2double.c
/home/hannah/mcavoy/idl/clib/read_driver2.c
/home/hannah/mcavoy/idl/clib/fidl_mvpa4.c)
TARGET_LINK_LIBRARIES(fidl_mvpa4 Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})


ADD_EXECUTABLE(fidl_gauss 
/home/hannah/mcavoy/idl/clib/d2double.c
/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/dim_param2.c
/home/hannah/mcavoy/idl/clib/subs_mask.c
/home/hannah/mcavoy/idl/clib/gauss_smoth2.c
/home/hannah/mcavoy/idl/clib/minmax.c
#/home/hannah/mcavoy/idl/clib/subs_util.c
/home/hannah/mcavoy/idl/clib/fidl_gauss.c)
TARGET_LINK_LIBRARIES(fidl_gauss Fidl Cifti Nifti Common ${LIBS})

ADD_EXECUTABLE(fidl_sim 
/home/hannah/mcavoy/idl/clib/fidl_sim.c
/home/hannah/mcavoy/idl/clib/spatial_extent2.c
/home/hannah/mcavoy/idl/clib/gauss_smoth2.c
/home/hannah/mcavoy/idl/clib/t_to_z.c
/home/hannah/mcavoy/idl/clib/f_to_z.c
/home/hannah/mcavoy/idl/clib/subs_mask.c
/home/hannah/mcavoy/idl/clib/subs_util.c)
#TARGET_LINK_LIBRARIES(fidl_sim m Fidl Gsl Gslcblas)
TARGET_LINK_LIBRARIES(fidl_sim m fidlc gsl gslcblas)

ADD_EXECUTABLE(fidl_glmcond
/home/hannah/mcavoy/idl/clib/fidl_glmcond.c
/home/hannah/mcavoy/idl/clib/gsl_svd.c
/home/hannah/mcavoy/idl/clib/cond_norm1.c)
#TARGET_LINK_LIBRARIES(fidl_glmcond Fidl ${LIBS} ${GSL} ${GSLCBLAS})
#START150429
#TARGET_LINK_LIBRARIES(fidl_glmcond m Fidl Gsl Gslcblas)
#TARGET_LINK_LIBRARIES(fidl_glmcond m Fidl gsl gslcblas)
TARGET_LINK_LIBRARIES(fidl_glmcond m Fidl Gsl Gslcblas)

ADD_EXECUTABLE(fidl_timestr2
/home/hannah/mcavoy/idl/clib/fidl_timestr2.c)
TARGET_LINK_LIBRARIES(fidl_timestr2 Fidl)

#ADD_EXECUTABLE(fidl_scratchdir
#/home/hannah/mcavoy/idl/clib/fidl_scratchdir.c)
#TARGET_LINK_LIBRARIES(fidl_scratchdir Fidl ${LIBS})
#START150423
ADD_EXECUTABLE(fidl_scratchdir
/home/hannah/mcavoy/idl/clib/fidl_scratchdir.c
/home/hannah/mcavoy/idl/clib/make_scratchdir.c
/home/hannah/mcavoy/idl/clib/make_timestr2.c)

ADD_EXECUTABLE(compute_tc_reg_stats
/home/hannah/mcavoy/idl/clib/compute_tc_reg_stats.c
/home/hannah/mcavoy/idl/clib/t4_atlas.c
/home/hannah/mcavoy/idl/clib/dim_param2.c
/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx)
#TARGET_LINK_LIBRARIES(compute_tc_reg_stats Fidl ${LIBS} ${GSL} ${GSLCBLAS})
#START150429
#TARGET_LINK_LIBRARIES(compute_tc_reg_stats m Fidl Gsl Gslcblas)
#TARGET_LINK_LIBRARIES(compute_tc_reg_stats Fidl gsl gslcblas)
#TARGET_LINK_LIBRARIES(compute_tc_reg_stats Fidl Cifti Nifti Common ${LIBS} gsl gslcblas)
TARGET_LINK_LIBRARIES(compute_tc_reg_stats Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})

ADD_EXECUTABLE(fidl_cov
/home/hannah/mcavoy/idl/clib/fidl_cov.c 
/home/hannah/mcavoy/idl/clib/globcov.c 
/home/hannah/mcavoy/idl/clib/trend.c
/home/hannah/mcavoy/idl/clib/t4_atlas.c
/home/hannah/mcavoy/idl/clib/count_tab.c
/home/hannah/mcavoy/idl/clib/read_data.c)
#TARGET_LINK_LIBRARIES(fidl_cov Fidl ${LIBS} ${GSL} ${GSLCBLAS})
#START150429
TARGET_LINK_LIBRARIES(fidl_cov m Fidl Gsl Gslcblas)

ADD_EXECUTABLE(compute_boynton
/home/hannah/mcavoy/idl/clib/compute_boynton.c 
/home/hannah/mcavoy/idl/clib/boynton_model.c)
#TARGET_LINK_LIBRARIES(compute_boynton Fidl ${LIBS})
#START150429
TARGET_LINK_LIBRARIES(compute_boynton m Fidl)

ADD_EXECUTABLE(compute_region_clusters
/home/hannah/mcavoy/idl/clib/compute_region_clusters.c
/home/hannah/mcavoy/idl/clib/assign_region_names2.c
/home/hannah/mcavoy/idl/clib/read_tc_string_new.c)
#TARGET_LINK_LIBRARIES(compute_region_clusters Fidl ${LIBS} ${GSL} ${GSLCBLAS})
#START150429
#TARGET_LINK_LIBRARIES(compute_region_clusters m Fidl Gsl Gslcblas)
#START160107
#TARGET_LINK_LIBRARIES(compute_region_clusters fidlc)
TARGET_LINK_LIBRARIES(compute_region_clusters m Fidl Gsl)

ADD_EXECUTABLE(compute_voxel_value
/home/hannah/mcavoy/idl/clib/compute_voxel_value.c 
/home/hannah/mcavoy/idl/clib/read_af3d.c)
#TARGET_LINK_LIBRARIES(compute_voxel_value Fidl ${LIBS})
#START150429
TARGET_LINK_LIBRARIES(compute_voxel_value m Fidl)

ADD_EXECUTABLE(fidl_flipreg
/home/hannah/mcavoy/idl/clib/fidl_flipreg.c
/home/hannah/mcavoy/idl/clib/assign_region_names2.c
/home/hannah/mcavoy/idl/clib/get_atlas_coor.c)
#TARGET_LINK_LIBRARIES(fidl_flipreg Fidl ${LIBS} ${GSL} ${GSLCBLAS})
#START150429
TARGET_LINK_LIBRARIES(fidl_flipreg m Fidl Gsl Gslcblas)

ADD_EXECUTABLE(atlas_transform
/home/hannah/mcavoy/idl/clib/atlas_transform.c
/home/hannah/mcavoy/idl/clib/t4_atlas.c)
#TARGET_LINK_LIBRARIES(atlas_transform Fidl ${LIBS} ${GSL} ${GSLCBLAS})
#START150429
TARGET_LINK_LIBRARIES(atlas_transform m Fidl Gsl Gslcblas)

ADD_EXECUTABLE(compute_region_stats
/home/hannah/mcavoy/idl/clib/compute_region_stats.c
/home/hannah/mcavoy/idl/clib/center_of_mass2.c
/home/hannah/mcavoy/idl/clib/get_atlas_coor.c)
#TARGET_LINK_LIBRARIES(compute_region_stats Fidl ${LIBS} ${GSL} ${GSLCBLAS})
#START150429
TARGET_LINK_LIBRARIES(compute_region_stats m Fidl Gsl Gslcblas)

ADD_EXECUTABLE(fidl_region_stats
/home/hannah/mcavoy/GITHUB/c/center_of_mass2.c
/home/hannah/mcavoy/GITHUB/c/get_atlas_coor.c
/home/hannah/mcavoy/GITHUB/c/header.cxx
/home/hannah/mcavoy/GITHUB/c/stack.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_gethdr.cxx
/home/hannah/mcavoy/GITHUB/c/subs_nifti.cxx
/home/hannah/mcavoy/GITHUB/c/filetype.c
/home/hannah/mcavoy/GITHUB/c/nifti_flipslice.cxx
/home/hannah/mcavoy/GITHUB/c/write1.c
/home/hannah/mcavoy/GITHUB/c/subs_cifti.cxx
/home/hannah/mcavoy/GITHUB/c/fidl_region_stats.cxx)
TARGET_LINK_LIBRARIES(fidl_region_stats m Fidl Cifti Nifti Common Gsl ${LIBS})

ADD_EXECUTABLE(fidl_motionreg
/home/hannah/mcavoy/GITHUB/c/d2double.c
/home/hannah/mcavoy/GITHUB/c/read_data.c
/home/hannah/mcavoy/GITHUB/c/count_tab.c
/home/hannah/mcavoy/GITHUB/c/gsl_svd.c
/home/hannah/mcavoy/GITHUB/c/cond_norm1.c
/home/hannah/mcavoy/GITHUB/c/fidl_motionreg.c)
TARGET_LINK_LIBRARIES(fidl_motionreg m Fidl Gsl Gslcblas)

ADD_EXECUTABLE(fidl_split
/home/hannah/mcavoy/idl/clib/fidl_split.c
/home/hannah/mcavoy/idl/clib/get_atlas_coor.c)
#TARGET_LINK_LIBRARIES(fidl_split Fidl ${LIBS})
#START150429
TARGET_LINK_LIBRARIES(fidl_split m Fidl)

ADD_EXECUTABLE(fidl_threshclus
/home/hannah/mcavoy/idl/clib/fidl_threshclus.c
/home/hannah/mcavoy/idl/clib/spatial_extent2.c
/home/hannah/mcavoy/idl/clib/center_of_mass2.c
/home/hannah/mcavoy/idl/clib/assign_region_names2.c
/home/hannah/mcavoy/idl/clib/get_atlas_coor.c)
TARGET_LINK_LIBRARIES(fidl_threshclus m Fidl)

ADD_EXECUTABLE(fidl_combinations
/home/hannah/mcavoy/idl/clib/fidl_combinations.c)
TARGET_LINK_LIBRARIES(fidl_combinations m Fidl Gsl)

ADD_EXECUTABLE(fidl_p_to_z
/home/hannah/mcavoy/idl/clib/fidl_p_to_z.c)
TARGET_LINK_LIBRARIES(fidl_p_to_z m Gsl)

ADD_EXECUTABLE(fidl_and
/home/hannah/mcavoy/idl/clib/fidl_and.c
/home/hannah/mcavoy/idl/clib/dim_param2.c
/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx)
TARGET_LINK_LIBRARIES(fidl_and Fidl Cifti Nifti Common ${LIBS})

ADD_EXECUTABLE(fidl_mask
/home/hannah/mcavoy/GITHUB/c/filetype.c
/home/hannah/mcavoy/GITHUB/c/dim_param2.c
/home/hannah/mcavoy/GITHUB/c/d2float.c
/home/hannah/mcavoy/GITHUB/c/d2double.c
/home/hannah/mcavoy/GITHUB/c/subs_glm.c
/home/hannah/mcavoy/GITHUB/c/subs_nifti.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_gethdr.cxx
/home/hannah/mcavoy/GITHUB/c/header.cxx
/home/hannah/mcavoy/GITHUB/c/mask.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_flipslice.cxx
/home/hannah/mcavoy/GITHUB/c/stack.cxx
/home/hannah/mcavoy/GITHUB/c/cifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/subs_cifti.cxx
/home/hannah/mcavoy/GITHUB/c/write1.c
/home/hannah/mcavoy/GITHUB/c/spatial_extent2.c
/home/hannah/mcavoy/GITHUB/c/extract_regions.c
/home/hannah/mcavoy/GITHUB/c/fidl_mask.cxx)
TARGET_LINK_LIBRARIES(fidl_mask Fidl Cifti Nifti Common ${LIBS})

ADD_EXECUTABLE(fidl_avg
/home/hannah/mcavoy/idl/clib/fidl_avg.c
/home/hannah/mcavoy/idl/clib/dim_param2.c
/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx)
TARGET_LINK_LIBRARIES(fidl_avg Fidl Cifti Nifti Common ${LIBS})

ADD_EXECUTABLE(fidl_logic
/home/hannah/mcavoy/idl/clib/fidl_logic.c)
TARGET_LINK_LIBRARIES(fidl_logic m Fidl)

ADD_EXECUTABLE(make_save
/home/hannah/mcavoy/idl/clib/make_save.c)
TARGET_LINK_LIBRARIES(make_save ${LIBS})

ADD_EXECUTABLE(fidl_tc_ss
/home/hannah/mcavoy/idl/clib/read_frames_file.c
/home/hannah/mcavoy/idl/clib/dim_param2.c
/home/hannah/mcavoy/idl/clib/get_atlas_coor.c
/home/hannah/mcavoy/idl/clib/t4_atlas.c
/home/hannah/mcavoy/idl/clib/read_tc_string_new.c
/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/fidl_tc_ss.c)
#TARGET_LINK_LIBRARIES(fidl_tc_ss m Fidl)
#TARGET_LINK_LIBRARIES(fidl_tc_ss fidlc cifti nifti common ${LIBS})
TARGET_LINK_LIBRARIES(fidl_tc_ss Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})

ADD_EXECUTABLE(fidl_assemble
/home/hannah/mcavoy/idl/clib/dim_param2.c
/home/hannah/mcavoy/idl/clib/write1.c
/home/hannah/mcavoy/idl/clib/subs_nifti.cxx
/home/hannah/mcavoy/idl/clib/subs_cifti.cxx
/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/fidl_assemble.c)
#TARGET_LINK_LIBRARIES(fidl_assemble fidlc cifti nifti common ${LIBS})
TARGET_LINK_LIBRARIES(fidl_assemble Fidl Cifti Nifti Common ${LIBS})

ADD_EXECUTABLE(fidl_chunkstack
/home/hannah/mcavoy/idl/clib/d2double.c
/home/hannah/mcavoy/idl/clib/dim_param2.c
/home/hannah/mcavoy/idl/clib/write1.c
/home/hannah/mcavoy/idl/clib/subs_nifti.cxx
/home/hannah/mcavoy/idl/clib/subs_cifti.cxx
/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/filetype.c
/home/hannah/mcavoy/idl/clib/get_atlas_coor.c
/home/hannah/mcavoy/idl/clib/monte_carlo_mult_comp.c
/home/hannah/mcavoy/idl/clib/spatial_extent2.c
/home/hannah/mcavoy/idl/clib/center_of_mass2.c
/home/hannah/mcavoy/idl/clib/assign_region_names2.c
/home/hannah/mcavoy/idl/clib/fidl_chunkstack.c)
#TARGET_LINK_LIBRARIES(fidl_chunkstack fidlc cifti nifti common ${LIBS})
TARGET_LINK_LIBRARIES(fidl_chunkstack Fidl Cifti Nifti Common ${LIBS})

ADD_EXECUTABLE(fidl_checkglm
/home/hannah/mcavoy/idl/clib/gsl_svd.c
/home/hannah/mcavoy/idl/clib/cond_norm1.c
/home/hannah/mcavoy/idl/clib/fidl_checkglm.c)
#TARGET_LINK_LIBRARIES(fidl_checkglm fidlc gsl gslcblas)
TARGET_LINK_LIBRARIES(fidl_checkglm m Fidl Gsl Gslcblas)

ADD_EXECUTABLE(fidl_flipglm
/home/hannah/mcavoy/idl/clib/subs_mask.c
/home/hannah/mcavoy/idl/clib/get_atlas_param.c
/home/hannah/mcavoy/idl/clib/get_atlas_coor.c
/home/hannah/mcavoy/idl/clib/atlas_to_index.c
/home/hannah/mcavoy/idl/clib/write_glm.c
/home/hannah/mcavoy/idl/clib/fidl_flipglm.c)
#TARGET_LINK_LIBRARIES(fidl_flipglm fidlc)
TARGET_LINK_LIBRARIES(fidl_flipglm m Fidl)

ADD_EXECUTABLE(fidl_fdr
/home/hannah/mcavoy/idl/clib/read_data.c
/home/hannah/mcavoy/idl/clib/count_tab.c
/home/hannah/mcavoy/idl/clib/dim_param2.c
/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/nifti_getDimensions.cxx
/home/hannah/mcavoy/idl/clib/spatial_extent2.c
/home/hannah/mcavoy/idl/clib/subs_cifti.cxx
/home/hannah/mcavoy/idl/clib/subs_nifti.cxx
/home/hannah/mcavoy/idl/clib/write1.c
/home/hannah/mcavoy/idl/clib/fdr_guts.c
/home/hannah/mcavoy/idl/clib/center_of_mass2.c
/home/hannah/mcavoy/idl/clib/assign_region_names2.c
/home/hannah/mcavoy/idl/clib/get_atlas_coor.c
/home/hannah/mcavoy/idl/clib/fidl_fdr.c)
TARGET_LINK_LIBRARIES(fidl_fdr Fidl Cifti Nifti Common Gsl ${LIBS})

ADD_EXECUTABLE(fidl_rename_effects
/home/hannah/mcavoy/idl/clib/d2double.c
/home/hannah/mcavoy/idl/clib/read_tc_string_new.c
/home/hannah/mcavoy/idl/clib/write_glm.c
/home/hannah/mcavoy/idl/clib/fidl_rename_effects.c)
TARGET_LINK_LIBRARIES(fidl_rename_effects m Fidl)

ADD_EXECUTABLE(fidl_conc
/home/hannah/mcavoy/idl/clib/fidl_conc.c)
TARGET_LINK_LIBRARIES(fidl_conc m Fidl)

ADD_EXECUTABLE(fidl_logreg
/home/hannah/mcavoy/GITHUB/c/read_data.c
/home/hannah/mcavoy/GITHUB/c/x2_to_z.c
/home/hannah/mcavoy/GITHUB/c/d2double.c
/home/hannah/mcavoy/GITHUB/c/count_tab.c
/home/hannah/mcavoy/GITHUB/c/find_regions_by_file_cover.c
/home/hannah/mcavoy/GITHUB/c/header.cxx
/home/hannah/mcavoy/GITHUB/c/subs_glm.c
/home/hannah/mcavoy/GITHUB/c/subs_nifti.cxx
/home/hannah/mcavoy/GITHUB/c/filetype.c
/home/hannah/mcavoy/GITHUB/c/nifti_gethdr.cxx
/home/hannah/mcavoy/GITHUB/c/d2float.c
/home/hannah/mcavoy/GITHUB/c/map_disk.c
/home/hannah/mcavoy/GITHUB/c/read_tc_string_new.c
/home/hannah/mcavoy/GITHUB/c/read_frames_file.c
/home/hannah/mcavoy/GITHUB/c/dim_param2.c
/home/hannah/mcavoy/GITHUB/c/cifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/mask.cxx
/home/hannah/mcavoy/GITHUB/c/write1.c
/home/hannah/mcavoy/GITHUB/c/subs_cifti.cxx
/home/hannah/mcavoy/GITHUB/c/get_limits.c
/home/hannah/mcavoy/GITHUB/c/cond_norm1.c
/home/hannah/mcavoy/GITHUB/c/gsl_svd.c
/home/hannah/mcavoy/GITHUB/c/check_dimensions.cxx
/home/hannah/mcavoy/GITHUB/c/region.cxx
/home/hannah/mcavoy/GITHUB/c/logreg.c
/home/hannah/mcavoy/GITHUB/c/fidl_logreg.cxx)
TARGET_LINK_LIBRARIES(fidl_logreg m Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})

#START180205
ADD_EXECUTABLE(fidl_logreg_montecarlo
/home/hannah/mcavoy/GITHUB/c/count_tab.c
/home/hannah/mcavoy/GITHUB/c/d2double.c
/home/hannah/mcavoy/GITHUB/c/d2float.c
/home/hannah/mcavoy/GITHUB/c/subs_glm.c
/home/hannah/mcavoy/GITHUB/c/nifti_gethdr.cxx
/home/hannah/mcavoy/GITHUB/c/filetype.c
/home/hannah/mcavoy/GITHUB/c/subs_nifti.cxx
/home/hannah/mcavoy/GITHUB/c/header.cxx
/home/hannah/mcavoy/GITHUB/c/mask.cxx
/home/hannah/mcavoy/GITHUB/c/read_data.c
/home/hannah/mcavoy/GITHUB/c/logreg.c
/home/hannah/mcavoy/GITHUB/c/map_disk.c
/home/hannah/mcavoy/GITHUB/c/dim_param2.c
/home/hannah/mcavoy/GITHUB/c/write1.c
/home/hannah/mcavoy/GITHUB/c/cifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/subs_cifti.cxx
/home/hannah/mcavoy/GITHUB/c/x2_to_z.c
/home/hannah/mcavoy/GITHUB/c/spatial_extent2.c
/home/hannah/mcavoy/GITHUB/c/fidl_logreg_montecarlo.cxx)
TARGET_LINK_LIBRARIES(fidl_logreg_montecarlo m Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})

#START180221
ADD_EXECUTABLE(fidl_logreg_fdr
/home/hannah/mcavoy/GITHUB/c/count_tab.c
/home/hannah/mcavoy/GITHUB/c/d2double.c
/home/hannah/mcavoy/GITHUB/c/d2float.c
/home/hannah/mcavoy/GITHUB/c/subs_glm.c
/home/hannah/mcavoy/GITHUB/c/nifti_gethdr.cxx
/home/hannah/mcavoy/GITHUB/c/filetype.c
/home/hannah/mcavoy/GITHUB/c/subs_nifti.cxx
/home/hannah/mcavoy/GITHUB/c/header.cxx
/home/hannah/mcavoy/GITHUB/c/mask.cxx
/home/hannah/mcavoy/GITHUB/c/read_data.c
/home/hannah/mcavoy/GITHUB/c/logreg.c
/home/hannah/mcavoy/GITHUB/c/map_disk.c
/home/hannah/mcavoy/GITHUB/c/dim_param2.c
/home/hannah/mcavoy/GITHUB/c/write1.c
/home/hannah/mcavoy/GITHUB/c/cifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/subs_cifti.cxx
/home/hannah/mcavoy/GITHUB/c/x2_to_z.c
/home/hannah/mcavoy/GITHUB/c/stack.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_flipslice.cxx
/home/hannah/mcavoy/GITHUB/c/atlas.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_getmni.cxx
/home/hannah/mcavoy/GITHUB/c/spatial_extent2.c
/home/hannah/mcavoy/GITHUB/c/fidl_logreg_fdr.cxx)
#TARGET_LINK_LIBRARIES(fidl_logreg_fdr m Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})
TARGET_LINK_LIBRARIES(fidl_logreg_fdr m Fidl Cifti Nifti Common Gsl ${LIBS})

#START180402
ADD_EXECUTABLE(fidl_logreg_collinear
/home/hannah/mcavoy/GITHUB/c/count_tab.c
/home/hannah/mcavoy/GITHUB/c/d2double.c
/home/hannah/mcavoy/GITHUB/c/read_data.c
/home/hannah/mcavoy/GITHUB/c/cond_norm1.c
/home/hannah/mcavoy/GITHUB/c/gsl_svd.c
/home/hannah/mcavoy/GITHUB/c/fidl_logreg_collinear.cxx)
TARGET_LINK_LIBRARIES(fidl_logreg_collinear m Fidl Cifti Nifti Common Gsl Gslcblas ${LIBS})


ADD_EXECUTABLE(fidl_lesion
/home/hannah/mcavoy/GITHUB/c/cifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/d2double.c
/home/hannah/mcavoy/GITHUB/c/d2float.c
/home/hannah/mcavoy/GITHUB/c/subs_glm.c
/home/hannah/mcavoy/GITHUB/c/nifti_flipslice.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_gethdr.cxx
/home/hannah/mcavoy/GITHUB/c/filetype.c
/home/hannah/mcavoy/GITHUB/c/subs_nifti.cxx
/home/hannah/mcavoy/GITHUB/c/header.cxx
/home/hannah/mcavoy/GITHUB/c/stack.cxx
/home/hannah/mcavoy/GITHUB/c/mask.cxx
/home/hannah/mcavoy/GITHUB/c/dim_param2.c
/home/hannah/mcavoy/GITHUB/c/spatial_extent2.c
/home/hannah/mcavoy/GITHUB/c/fidl_lesion.cxx)
TARGET_LINK_LIBRARIES(fidl_lesion Fidl Cifti Nifti Common ${LIBS})


ADD_EXECUTABLE(fidl_fix_region_file
/home/hannah/mcavoy/GITHUB/c/fidl_fix_region_file.c)
TARGET_LINK_LIBRARIES(fidl_fix_region_file m Fidl)

ADD_EXECUTABLE(fidl_chisquare_to_z
/home/hannah/mcavoy/GITHUB/c/x2_to_z.c
/home/hannah/mcavoy/GITHUB/c/fidl_chisquare_to_z.c)
TARGET_LINK_LIBRARIES(fidl_chisquare_to_z m Fidl)

ADD_EXECUTABLE(fidl_copy_contrasts
/home/hannah/mcavoy/GITHUB/c/subs_glm.c
/home/hannah/mcavoy/GITHUB/c/d2float.c
/home/hannah/mcavoy/GITHUB/c/d2double.c
/home/hannah/mcavoy/GITHUB/c/write_glm.c
/home/hannah/mcavoy/GITHUB/c/fidl_copy_contrasts.c)
TARGET_LINK_LIBRARIES(fidl_copy_contrasts m Fidl)

#ADD_LIBRARY(fidlcifti SHARED
#/home/hannah/mcavoy/idl/clib/cifti_getDimensions.cxx
#/home/hannah/mcavoy/idl/clib/nifti_gethdr.cxx
#/home/hannah/mcavoy/idl/clib/nifti_getslice.cxx
#/home/hannah/mcavoy/idl/clib/nifti_flipslice.cxx
#/home/hannah/mcavoy/idl/clib/nifti_getmni.cxx
#/home/hannah/mcavoy/idl/clib/nifti_getmni2.cxx
#/home/hannah/mcavoy/idl/clib/nifti_getorient.cxx
#/home/hannah/mcavoy/idl/clib/cifti_getColumn.cxx)
#TARGET_LINK_LIBRARIES(fidlcifti cifti nifti common ${LIBS})
#START170613
ADD_LIBRARY(fidlcifti SHARED
/home/hannah/mcavoy/GITHUB/c/cifti_getDimensions.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_gethdr.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_getslice.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_flipslice.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_getmni.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_getmni2.cxx
/home/hannah/mcavoy/GITHUB/c/nifti_getorient.cxx
/home/hannah/mcavoy/GITHUB/c/cifti_getColumn.cxx)
TARGET_LINK_LIBRARIES(fidlcifti cifti nifti common ${LIBS})

#START171113
ADD_EXECUTABLE(mpm_str_path
/home/hannah/mcavoy/GITHUB/c/mpm_str_path.cxx)
TARGET_LINK_LIBRARIES(mpm_str_path Fidl)
ADD_EXECUTABLE(mpm_str_root
/home/hannah/mcavoy/GITHUB/c/mpm_str_root.cxx)
TARGET_LINK_LIBRARIES(mpm_str_root Fidl)

INCLUDE_DIRECTORIES(
/home/hannah/mcavoy/FIDL/cifti/CiftiLib-master/Cifti
/home/hannah/mcavoy/FIDL/cifti/CiftiLib-master/Nifti
/home/hannah/mcavoy/FIDL/cifti/CiftiLib-master/Common
/home/hannah/mcavoy/FIDL/gsl1.16/include
${CMAKE_SOURCE_DIR}/fidl
)
