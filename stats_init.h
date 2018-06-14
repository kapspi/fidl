;Copyright 12/31/99 Washington University.  All Rights Reserved.
;stats_init.h  $Revision: 12.334 $

;widget ids
wd = {  Widget_IDs, $
    repeat_action:0L, $ ; Repeat last action.
    repeat_action_m1:0L, $ ; Repeat second last action.
    exit:0L,     $  ; Exit script.
    leader:0L,  $   ; Lead widget.
    msmooth:0L, $   ; Generate horizontal profile.
    color:0L,   $   ; Generate color table.
    color_scale:0L,   $   ; Generate color table.
    vol:0L,     $   ; Current volume number
;Display
    draw:lonarr(!MAX_WORKING_WINDOWS),    $  ; Draw widget images displayed on.
    wkg_base:lonarr(!MAX_WORKING_WINDOWS),    $  ; Draw widget images displayed on.
    new_wrkg_wdw:0L,    $  ; Create new working window.
    loadct:0L,  $   ; Call color table widget.
    adjust_colortable:0L,  $   ; Adjust color tables.
    cine:0L,     $  ; Display in cine mode.
    zoomin:0L,  $   ; Increase zoom factor.
    zoomout:0L, $   ; Decrease zoom factor.
    zoomtype:0L,    $   ; ALgorithm used for zoom.
    refresh:0L, $   ; Refresh display.
    display_all:0L, $   ; Display all planes.
    view_images:0L, $   ; View selected images.
    view26d4:0L, $
    print_all:0L,   $   ; Print all planes.
    coneqoff:0L, $
    coneqon:0L, $
    local_scl:!TRUE,$   ; Scale each image separately.
    global_scl:!FALSE,$ ; Same scale for entire volume.
    fixed_scl:!FALSE,$  ; User specified scale for entire volume.
    fixed_glbl_scl:!FALSE,$ ; Global scale used for entire volume.
    build_2x2:0L,   $   ; Build 2x2 display
    build_custom:0L, $  ; Build custom display
    build_addimg:0L, $  ; Add image to custom display.
    build_annotate:0L,$ ; Annotate custom display.
    build_draw:0L,  $   ; Draw widget for custom display.
    select_custom:0L,$  ; Select current custom window.
    crop_custom:0L,$        ; Crop images in current custom window.
    color_scale_widget:0L,$ ; Widget number of color scale.
    color_scale_base:0L,$ ; Widget number of color scale base.
    overlay_actmap:0L,$ ; Overlay activation map on image.
    view3d:0L,  $   ; Invoke slicer.
    radview:0L, $   ; View in radiological orientation.
    neuroview:0L,   $   ; View in neurological orientation.
;Status
    error:0L,   $   ; Error display.
;Profiles
    hprof:0L,   $   ; Generate horizontal profile.
    vprof:0L,   $   ; Generate vertical profile.
    zprof:0L,   $   ; Generate time or Z profile.
    roiprof:0L, $   ; Generate region using time profile.
    oprof:0L,   $   ; Overlay profile on existing profiles.
    profsave:0L,    $   ; Save profiles to disk.
    path:'',    $   ; Path to directory where profiles are saved.
    phisto:0L,  $   ; Plot histogram of profile data.Plot.
    prof_fft:0L,    $   ; Plot histogram of profile data.Plot.
    true_scaling:0L,$   ; Plot unscaled time-profile values.
    prop_scaling:0L,$   ; Plot scaled time-profile values.
    profsym:0L, $   ; Plot profiles with symbols only.
    profline:0L,    $   ; Plot profiles with lines only.
    profsymline:0L, $   ; Plot profiles with lines and symbols.
    gaussfit:0L,    $   ; Fit Gaussian to profile.
    profwidth:0L,   $   ; Profile width.
    profhght:0L,    $   ; Profile height.
    print_prof:0L,  $   ; Print profiles.
    select_prof:0L, $   ; Print profiles.
    annotate_prof:0L, $ ; Print profiles.
    kill_all_prof:0L, $ ; Print profiles.
    prbox_smooth:0L,$   ; Width of boxcar for profile smooth.
    prbox_smooth0:0L,$  ; Boxcar width of 0.
    prbox_smooth3:0L,$  ; Boxcar width of 3.
    prbox_smooth5:0L,$  ; Boxcar of width of 5
    prof_wht:0L,    $   ; White profiles.
    prof_ylw:0L,    $   ; Yellow profiles.
    prof_red:0L,    $   ; Red profiles.
    prof_grn:0L,    $   ; Green profiles.
    prof_blu:0L,    $   ; Blue profiles.
    t_box_size:0L,  $   ; Box size for time profiles.
    t_gang:0L,  $   ; Ganged plot for time profile.
    gang_dim:0L,    $   ; Number of plots in gang.
    gang_subdim:0L, $   ; Number of voxels to sum for each gang plot.
    t_prof_plain:0L,    $   ; Sum all pixels within designated box.
    t_prof_ext_thresh:0L,$  ; Sum all pixels within designated box.
    t_prof_max_pix:0L,$ ; Sum pixels above threshold.
    scatter_plot:0L,  $   ; Scatter plot.
    thresh_pct:0L,  $   ; Threshold for time profile.
    thresh_sig:0L,  $   ; Threshold for time profile.
    subtract_trend:0L,$ ; Subtract trend from time profiles?
    no_trend:0L,    $   ; Don't subtract trend from time profiles?
;Process
    plnstats:0L,    $   ; Statistics of displayed image.
    fidl_swap:0L, $
    voxel_values:0L,$   ; Image values at voxel locations in a text file. 
    fidl_4dfp_to_text:0L,$   ; Image values in mask to a text file. 
    fidl_text_to_4dfp:0L,$
    fidl_txt2conc_subcol:0L,$
    fidl_collatecol:0L, $
    sinecorr:0L,    $   ; Correlate with sine function.
    diffimg:0L, $   ; Compute difference image.
    regress:0L, $   ; Compute Gaussianized r for regression analysis.
    actmap:0L,  $   ; Spawn Avi's actmap program.
        smobox:0L,  $   ; Smooth with boxcar filter.
        smogauss:0L,    $   ; IDL - Smooth with Gaussian filter.
        fidl_gauss:0L,    $   ; C - Smooth with Gaussian filter.
        smobutter:0L,   $   ; Smooth with Butterworth filter. mcavoy
        meanvar:0L, $   ; Compute mean and variance.
        linfit:0L,  $   ; Fit line to time course.
        kolmo_smrnov:0L,$   ; Compute Kolmogorov-Smirnov statistic.
        wilcoxon:0L,    $   ; Compute two-tailed Wilcoxon statistic.
        kendalls_tau:0L,$   ; Compute Kendall's tau statistic
        svd:0L,     $   ; SVD analysis.
        define_stimulus:0L,$    ; Define stimulus time-course.
        edit_stimulus:0L,$  ; Edit stimulus time-course.
        show_stimulus:0L,$  ; Plot stimulus time-course.
        save_stimulus:0L,$  ; Save stimulus time-course in interfile hdr.
        covariance:0L,$     ; Compute covariance matrix for a single voxel.
        analyze_cc:0L,$     ; Analyze correlation coefficients.
        create_cc_src:0L,$  ; Create data file listing source voxels.
        pca_cc:0L,$         ; Perform PCA on residual correlation matrix.
        cov_butter:0L,$         ; Compute correlations after filtering.
        smoothness:0L,$     ; Smoothness estimate ala Forman et al.
        time_corr:0L,$      ; Estimate of time-correlation.
        mult_comp_monte_carlo:0L,$      ; Correct for multiple comparisons (IDL).
        mult_comp_monte_carloC:0L,$     
        fidl_fdr:0L,$
    generate_mask:0L,$  ; Generate mask.
    apply_mask:0L,$     ; Apply mask and sum over masked image.
    fidl_mask:0L,$
    fidl_logic:0L,$
    fidl_split:0L,$
;Files
    mldcti:0L,  $   ; Load CTI image.
    ldmri_analyze:0L,$  ; Load Analyze MRI data.
    ldmri_4dfp:0L,  $   ; Load 4D floating point MRI data file.
    ld_gif:0L,  $   ; Load GIF image.
    ld_tif:0L,  $   ; Load TIF image.

    ;START160112
    ld_list:0L, $

    ld_xwd:0L,  $   ; Load xwd image.
    ld_matrix7v:0L, $   ; Load CTI matrix 7 image file.
    ld_t88:0L,  $   ; Load 4D floating point MRI data file.
    ld_optical:0L,$ ; Load optical data.
    ldraw:0L,   $   ; Load unformatted data.
    stitch:0L,  $   ; Stitch together multiple files.
    wrtgif:0L,  $   ; Write GIF file.
    wrttif:0L,  $   ; Write TIFF file.
    wrt4dfp:0L, $   ; Write Raw file.
    printany:0L,    $   ; Print any window
;Help
    help:0L,        $   ; Main help button.
    help_info:0L,   $   ; Display help window.
;Controls
    files:0L,   $    ; File list display.

    nextpln:0L, $    ; Display next plane.
    lastpln:0L, $    ; Display previous plane.
    nextfrm:0L, $    ; Display next frame.
    lastfrm:0L, $    ; Display previous frame.
    scale:0L, $
    conadj:0L, $

    sld_pln:0L, $    ; Plane number slider.
    sld_frm:0L, $    ; Frame number slider.
    delete:0L, $     ; Delete file.
    delete_all:0L, $ ; Delete all files.
        low_bandwidth:0L,$  ; Reduced display I/O.
        nrml_bandwidth:0L,$ ; Normal display I/O.
    roinum:0L, $        ; Displays current roi number.
    roivol:0L, $        ; Display current volume number.
    nxtvol:0L, $        ; Set to next available volume number.
    user1:0L,   $
    user2:0L,   $
    user3:0L,   $
    user4:0L,   $
    user5:0L,   $
    user6:0L,   $
    user7:0L,   $
    user8:0L,   $
    user9:0L,   $
    user10:0L,  $
    test:0L,        $   ; Test
    test1:0L,       $   ; Test
    atlas_transform:0L, $   ; Transform to atlas space.
    atlas_transform_c:0L, $   ; Transform to atlas space.
    logical_not:0L,  $   ; Save logical inverse of an image.
    logical_and:0L, $   ; Perform logical and of two images.
    fidl_and:0L, $
    logical_or:0L,  $   ; Perform logical or of two images.
    logical_xor:0L $
    }

wd1 = { Widget_IDs_1,   $
    mstim:0L,   $   ; Menu
    mload:0L,   $   ; Menu
    mdisp:0L,   $   ; Menu
    mprof:0L,   $   ; Menu
    mhelp:0L,   $   ; Menu
    mlinmod:0L, $   ; Menu
    msmoth:0L,  $   ; Menu
    msmoth_gauss:0L,  $   ; Menu
    malgebra:0L,    $   ; Menu
    mproc:0L,   $   ; Menu
    zoom:0L,    $   ; Zoom image and save in another slot.
    low_bandwidth:0L,  $    ; Low bandwidth network connection.
    high_bandwidth:0L,  $   ; High bandwidth network connection.
    display_comp:0L, $  ; Compare two images.
    view:0L,    $   ; Change view to coronal or sagittal.
    build_macro:0L, $   ; Build macro for batch job.
    macro_comment:0L, $ ; Build macro for batch job.
    compile_macro:0L, $ ; Submit macro for batch job.
    finish_macro:0L, $  ; Finish batch job macro.
    load_conc:0L,   $   ; Load concatenated set of files.
    save_conc:0L,   $   ; Save concatenated set of files.
    generate_conc:0L, $ ; Generate concatenated set of files.
    fidl_genconc:0L, $
    fidl_randskew:0L, $
    load_atlas:0L, $
    load_mask:0L, $
    load_image_filter:0L,   $   ; Filter for load dialog box.
    load_glm_filter:0L,   $   ; Filter for load glm dialog box.
    def_single_trial:0L,$   ; Define single trial paradigm.
    single_trial_resp:0L,$  ; Compute average single trial response.
    def_contrast:0L,$   ; Define contrast vectors.
    def_contrast1:0L,$   ; Define contrast vectors.
    copy_contrasts:0L,$   ; Copy contrasts from one glm file to one or more others.
    subtract_images:0L,$; Add two images.
    divide_images:0L,$  ; Divide two images.
    multiply_images:0L,$; Multiply two images.
    scale_images:0L,$   ; Scale image by a constant.
    add_scalar:0L,  $   ; Add scalar to an image.
    clip_image:0L,  $   ; Clip an image.
    fidl_avg:0L,  $     
    fidl_zeromean:0L,  $     
    fidl_minmax:0L,  $  ; Min and max maps.
    fidl_lminusrdivlplusr:0L,$

    ;START170404
    ;fidl_sumconcpair:0L,$

    power_spectrum:0L,$ ; Compute power spectral density.
    compute_pearson:0L,$ ; Pearson product moment correlation coefficient 
    compute_boynton:0L,$ ; Boynton HRF

    ;START160614
    fidl_flipglm:0L,$ 

    fidl_flip:0L,$ 
    fidl_flipnew:0L,$ 
    fidl_histogram:0L, $
    fidl_hemitxt:0L, $
    fidl_circor:0L, $
    fidl_psd:0L,$ 
    threshold_image:0L,$; Interactively threshold image.
    ;threshold_cluster:0L,$; Interactively threshold and cluster image.
    ;fidl_threshold_cluster:0L,$; C version 
    ;fidl_threshold_cluster2:0L,$;

    ;START140825
    fidl_threshclus_idl:0L,$
    fidl_threshclus:0L,$

    link_image:0L,  $   ; Link image to data file.
    link_model:0L,  $   ; Link model to data file.
    prgauss_smooth:0L,$ ; Gaussian profile kernel
    import_spm:0L,  $   ; Save preferences.
    save_prefs:0L,  $   ; Save preferences.
    preferences:0L, $   ; Set preferences.
    comp_linmod:0L, $   ; Compute general linar model.
    glm_adjusted_df:0L, $   ; Compute adjusted degrees of freedom.
    comp_residuals:0L, $   ; Compute residuals of general linar model.
    compute_residuals:0L, $   ; Compute residuals of general linar model.
    fidl_autocorrelation:0L, $
    fidl_rename_effects:0L, $
    fidl_rename_paths:0L, $
    fidl_eog:0L, $
 
    ;START141001
    fidl_motionreg:0L, $

    ;START140911
    fidl_motion:0L, $

    fidl_anova:0L,  $   ; Compute anova
    fidl_anova2:0L,  $   ; Compute anova
    fidl_anova_ss:0L,  $   ; single subject, widely spaced design 
    compute_avg_zstat:0L, $ ; Compute average z statistics
    fidl_2ndlevelmodel:0L, $ 
    fidl_slopesintercepts:0L, $ 
    fidl_lmerr:0L, $ 
    compute_epsilon:0L, $   ; Correction for the degrees of freedom 
    comp_linmod_old:0L, $   ; Compute general linar model.
    extract_timecourse:0L,$ ; Extract estimated time-course from glm (IDL).
    interleave_timecourse:0L,$ ; Interleave two time-courses.
    sum_timecourse:0L,$ ; Sum estimated time-course from glm.
    ;extract_timecourse_c:0L,$ ; Extract estimated time-course from glm (C).
    compute_tc_max_deflect:0L,$ ; Maximum defection of timecourse from 4dfp.
    compute_avg_zstat_boy:0L,$ ;
    fidl_tc_ss:0L,$ ;Widely spaced designs. Mean timecourses extracted from residual BOLDs. 
    fidl_logreg_ss:0L,$

    ;START170614
    fidl_logreg_ss2:0L,$

    fidl_cc:0L,$ 
    fidl_fano:0L,$ 
    extract_fzstat:0L,$ ; Extract F-derived z-statistic.
    fidl_extract_fzstat:0L,$ 
    extract_mean:0L,$   ; Extract glm grand mean.
    extract_sdev:0L,$   ; Extract glm grand mean.
    extract_trend:0L,$  ; Extract glm trend (ie slope).
    extract_cov:0L,$    ; Extract glm covariance matrix.
    fidl_glmsavecorr:0L,$
    fix_grand_mean:0L,$ ; Patch to fix incorrect grand means in GLMs.
    fidl_ac:0L,$ 
    fidl_crosscorr:0L,$ 
    fidl_crosscov:0L,$ 
    fidl_cov:0L,$ 
    fidl_pca:0L,$ 

    ;START170127
    fidl_pcat:0L,$ 

    fidl_levene:0L,$ 
    cross_corr:0L,$     ; Cross-correlate adjacent time courses.
    apply_linmod:0L,$   ; Apply contrasts to estimates to get t-stats
    compute_zstat:0L,$  ; Apply contrasts to estimates to get t-stats
    extract_mag:0L,$    ; Write magnitudes from GLM to a single file. 
    fidl_mvpa:0L,$

    ;START151111
    fidl_mvpa2:0L,$

    fidl_ttest:0L,$  ; t-tests (C version).
    fidl_ftest_ss:0L,$
    fidl_ttest_ss:0L,$
    correlation_analysis:0L,$  ; Test correlations.
    scatter_plots:0L,$  ; Extract scatter plots.
    apply_linmod_lag:0L,$   ; Apply contrasts to estimates to get t-stats
    hotellingt2:0L,$    ; Compute z-statistics using Hotelling T**2.
    test_linmod:0L,$    ; Test new statistics.
    fidl_checkglm:0L, $
    fidl_glmcond:0L, $
    show_linmod:0L, $   ; Show design matrix.
    save_linmod:0L, $   ; Save linear model and estimates to disk.
    load_linmod:0L, $   ; Load linear models and estimates from disk.
    def_block_linmod:0L,    $   ; Define design matrix.
    def_single_linmod_new:0L,$  ; Define single-trial design matrix.

    ;START170109
    def_single_linmod_new_trials:0L,$

    define_pet_linmod:0L,$  ; Define PET design matrix.
    define_manual_linmod:0L,$  ; Manually define design matrix.
    define_pet_linmod_dflt:0L,$ ; Define PET design matrix.
    define_optical_linmod:0L,$ ; Define optical design matrix..
    define_cov_linmod:0L,$ ; Define design matrix for correlation analysis.
    export_dsgn_matrix:0L,$ ; Export design matrix to an image.
    import_dsgn_matrix:0L,$ ; Import design matrix from a text file.
    dsgn_matrix_metrics:0L,$ ; Print design matrix quality metrics.
    fidl_dsgn_matrix_metrics:0L,$ ; Print design matrix quality metrics.
    event_file_synopsis:0L,$ ; Create synopsis of event file.
    edit_linmod:0L, $   ; Edit design matrix.
        timeover_no:0L, $   ; No overplot for time profiles.
        timeover_para:0L,$  ; Overplot paradigm for time profiles.
        timeover_glm:0L,    $; Overplot gen. linear model for time profiles.
        timeover_hrf:0L,    $; Overplot assumed HRF for time profiles.
        timeover_pca:0L,    $; Overplot principal components.
        timeover_stderr:0L, $; Overplot standard error.
        glm_simulate:0L, $  ; Numerical simulation of linear model.
        glm_boynton_mod:0L, $   ; Modify Boynton model parameters.
        glm_sim_paradigm:0L, $  ; Simulate single trial paradigm.
        timeover_resid:0L,$ ; Overplot gen. lin model residuals.
        ;atlas_2a:0L,    $   ; Target for atlas transformation
        ;atlas_2b:0L,    $   ; Target for atlas transformation
	t_to_z_IDL:0L,  $   ; Convert T to z-statistic (IDL).
	fidl_t_to_z:0L,    $   ; Convert T to z-statistic (C version).
	f_to_z_IDL:0L,  $   ; Convert F to z-statistic (IDL).
	f_to_z_C:0L,    $   ; Convert F to z-statistic (C version).
	fidl_r_to_z:0L, $
	fidl_r_to_zC:0L, $
	fidl_p_to_z:0L, $
        sum_images:0L,  $   ; Sum images.
    mroi:0L, $      ; Roi menu.
    mroitype:0L, $      ; Roi definition button.
    roiirreg:0L, $      ; Define irregular ROI.
    roisq:0L, $     ; Define square ROI.
    roicrc:0L, $        ; Define circular ROI
    roimov:0L, $        ; Move ROI.
    del_roi:0L, $       ; Delete current roi.
    delall_roi:0L, $    ; Delete all rois.
    refresh_roi:0L, $   ; Refresh rois.
    sum_roi:0L, $       ; Sum rois
    roipcpy:0L, $       ; Copy rois.
    load_rois:0L, $     ; Load rois from disk.
    save_rois:0L, $     ; Save rois to disk.
    roicpy:0L, $        ; Copy ROIs from one volume to another.
    roicolor:0L, $      ; Select color.
    roiwht:0L, $        ; White rois.
    roiylw:0L, $        ; Yellow rois.
    roired:0L, $        ; Red Rois.
    roigrn:0L, $        ; Green rois.
    roiblu:0L, $        ; Blue rois.
    edit_mask:0L, $ ; Define voxels in mask.
    define_regions:0L, $          ; Define voxels in mask.
    count_mask:0L, $              ; Count regions defined in mask.
    fidl_reg_ass:0L, $            ; Assign values to regions. 
    convert_af3d_to_fidl:0L, $    ; Convert af3d to fidl region.
    fidl_labelfile:0L, $
    compute_region_stats:0L, $    ; Region center of mass and peak activation 
    compute_tc_reg_stats:0L, $    ; jmo's "sum regions" redone in C 
    compute_region_clusters:0L, $ ; Form region clusters.
    grow_regions:0L, $            ; Grow regions from af3d seeds. 
    delete_regions:0L, $          ; Delete regions. 
    fidl_flipreg:0L, $          
    fix_region_file:0L, $         ; Fix region files that have the first region defined with a value of 1.
    fidl_fix_region_file:0L, $    
    brain_mask:0L, $              ; Generate mask covering brain.
    timecourse_magnitude:0L, $    ; Compute magnitue measure of timecourse.
    frames_to_skip:0L, $          ; Number frames to skip in time profile.
    prof_proc_nothing:0L, $       ; No profile-region processing.
    prof_proc_pct:0L $            ; Use percentile threshold in profile region.
    }

cstm = {Custom_image,   $   ; Structure for custom images.
    xdim:0L,        $   ; Horizontal dimension.
    ydim:0L,        $   ; Vertical dimension.
    id:fltarr(!PROF_NWIN),$ ; IDs of custom windows.
    draw:fltarr(!PROF_NWIN),$ ; widget ids of custom windows.
    base:fltarr(!PROF_NWIN),$ ; base ids of custom windows.
    nblt:0L,        $   ; Number of images in custom display.
    nbltmax:0L, $   ; Maximum number of images in custom display.
    nwindows:0L,    $   ; Number of custom windows.
    crop:fltarr(4), $   ; Coordinates x1,x2,y1,y2 to crop images.
    cur_window:-1,  $   ; Current window number.
    minmax:fltarr(2),$  ; Array containing minimum and maximum.
    init:!FALSE $   ; !TRUE if initialized.
        }

help ={Help,        $
    enable:!FALSE,  $   ; !TRUE: display help window.
    id:0L       $   ; Widget id of help window.
        }

fd = {File_data,    $   ; Structure for each open file.
    file:'',    $   ; File name.
    xdim:0L,        $   ; Horizontal dimension.
    ydim:0L,        $   ; Vertical dimension.
    zdim:0L,        $   ; Depth dimension.
    tdim:0L,        $   ; Time dimension.
    type:0L,        $   ; Number format, !FLOAT, !SHORT.
    dxdy:0.,    $   ; Pixel size.
    dz:0.,      $   ; Slice thickness.
    scl:1.,     $   ; Scale factor for entire space.
    array_type:0L   $   ; Type of array, real or associated I/O
    }

stc1 = {Stitch,     $   ; File that stitches together multiple files.
    name:'',    $   ; Name to be displayed in file window.
    n:0L,       $   ; Number of stitched files.
    tdim_all:0L,    $   ; Time dimension of concatenated set.
    tdim_file:ptr_new(),$  ; Time dimension of each file.
    tdim_sum:ptr_new(),$   ; Cumulative time dimension.
    t_to_file:ptr_new(),$   ; Converts time to file number.
    hdr_ptr:ptr_new(),$    ; Pointers for headers.
    filnam:ptr_new(),$  ; File names
    identify:ptr_new(),$
    tdim_sum_new:ptr_new(),$

    ;START150624
    filetype:ptr_new()$

    }
stc = replicate(stc1,!NUM_FILES)

sng1 = {Eventfile, $
    name:'', $
    init_ctl:0, $
    period:0, $
    factor_labels:ptr_new(), $
    conditions:ptr_new(), $
    num_trials:0L, $
    num_levels:0, $
    frames:ptr_new(), $
    TR:0., $
    offsets:ptr_new(), $
    offsets_gf:ptr_new(), $
    stimlen:ptr_new(), $
    stimlen_vs_t:ptr_new(), $
    stimlenframes_vs_t:ptr_new(), $
    event_file:'', $
    effect_shift_TR:ptr_new(), $
    nbehavcol:0, $
    behav_vs_t:ptr_new(), $
    index_present:ptr_new(), $
    count_present:0, $
    times:ptr_new(), $
    frames_block:ptr_new(), $
    fraction:ptr_new(), $
    frames_floor:ptr_new() $
    }
glm1 = {General_linear_model,$  ; Parameters of general linear model.
    A:ptr_new(),    $   ; Design matrix.
    ATAm1:ptr_new(),$   ; Inverse of design matrix.
    ATAm1vox:ptr_new(),$   
    dfvox:ptr_new(),$   
    b:ptr_new(),    $   ; Estimated parameters
    nF:1,       $   ; Number of F statistics.
    F_names:ptr_new(), $
    F_names2:ptr_new(), $
    fstat:ptr_new(), $ 
    fzstat:ptr_new(), $ 
    sd:ptr_new(),   $   ; Standard Dev. of data
    var:ptr_new(),   $
    mean:ptr_new(), $   ; Pointer to mean of data
    grand_mean:ptr_new(),$  ; Pointer to grand mean over all runs.
    c:ptr_new(),    $   ; Pointer to contrast vector.
    cnorm:ptr_new(), $  ; Pointer to contrast vector for norm calc.
    valid_frms:ptr_new(),$  ; Pointer to vector of valid frames.
                ; valid_frms[i] > 0 if valid.
    spm:ptr_new(),  $   ; Design matrix imported from SPM.
    contrast_labels:ptr_new(),$ ; Labels for contrasts.
    effect_label:ptr_new(),$    ; Labels for all effects.
    effect_length:ptr_new(),$   ; Length of each effect.
    effect_column:ptr_new(),$   ; Column in design matrix for effects.
    effect_TR:ptr_new(),$       ; Effective TR of the condition. 
    effect_shift_TR:ptr_new(),$ ; Condition has been shifted some fraction of a TR from interleaving. 
    n_files:0L, $   ; Number of files.
    xdim:0L,     $   ; Horizontal dimension of image.
    ydim:0L,     $   ; Vertical dimension of image.
    zdim:0L,     $   ; Depth dimension of image.
    tdim:0L,    $   ; Time dimension of images.
    dxdy:1.,    $   ; XY voxel size.
    dz:1.,      $   ; Z voxel size.
    t_valid:0L, $   ; Number of valid time points.
    df:0.,      $   ; Degrees of freedom.
    W:0.,       $   ; Smoothness estimate.
    fwhmx:0.,    $   ; Smoothness estimate in units of FWHM.
    fwhmy:0.,    $   ; Smoothness estimate in units of FWHM.
    fwhmz:0.,    $   ; Smoothness estimate in units of FWHM.
    nc:0L,      $   ; Number of contrasts.
    tot_eff:0L, $   ; Total number of effects of interest.
    all_eff:0L, $   ; Total number of effects (interest + nuisance).
    period:0L,  $   ; Number of estimates per condition. (Length of HTF)
    num_trials:0L,  $   ; Number of stimuli presentations.
    TR:0.,      $   ; Time required to collect one frame.
    Ysim:ptr_new(), $   ; Simulated, ideal data vector for one voxel.
    stimlen:ptr_new(),$ ; Duration of neuronal firing.
    delay:ptr_new(),$   ; Delay of neuronal firing.
    lcfunc:ptr_new(),$  ; TRUE: Encode function into design matrix.
    ;funclen:1,  $   ; Number of elements in basis set.
    ;functype:!BOYNTON,$ ; Basis function type (!BOYNTON or !FRISTON)
    funclen:ptr_new(),  $   ; Number of elements in basis set. dimension = tot_eff
    functype:ptr_new(),$ ; Basis function type (!BOYNTON or !FRISTON) dimension = tot_eff
    effect_group:ptr_new(),$ ; Effect that an estimate belongs to.  This has effect if an effect has multiple stimulus durations and hence is represented by multiple columns in teh design matrix.
    stimlen_vs_t:ptr_new(),$; Stimulus of duration that varies by trial.
    rev:0,                $   ; Revision number of file format.
    N:0L,                 $   ; Total number of estimated parameters.
    n_interest:0L,        $   ; Number of effects of interest in dsgn matrix.
    start_data:0L,        $   ; Offset into file where the first computed
    event_file:ptr_new(), $   ; Event file used in construction of design matrix. 
    cifti_xmldata:ptr_new(), $

    ;START160914
    maskimg:ptr_new(), $
    maski:ptr_new(), $
    nmaski:0 $

    }
glm = replicate(glm1,!NUM_FILES)

trend = { LinearTrend,  $
        t_rest1:intarr(!NUM_FILES,20), $; Number of starting rest files.
    slope_ptr:ptrarr(!NUM_FILES), $; Assoc slope files.
    intcpt_ptr:ptrarr(!NUM_FILES), $; Assoc intercept files.
    hdr_ptr:ptrarr(!NUM_FILES) $ Pointers for image structs.
    }

fi = { Files,       $
    image_filter:'',    $   ; Filter for dialog box.
    glm_filter:'',    $   ; Filter for dialog box.
    path:'',    $   ; Current path.
    write_path:'', $   ; Path to write images to.
    tail:'',    $   ; Current file name less path.
    name:'',    $   ; Current file name (sans the .extension) with path. (If loaded from disk, then has .extension.)
    paths:strarr(!NUM_FILES), $ ; Paths to all filenames loaded.
    names:strarr(!NUM_FILES), $ ; Complete filenames loaded with the .extension and the path.
    tails:strarr( !NUM_FILES), $ ; Filenames less path ($names:t)
    list:strarr( !NUM_FILES),$ ; Name of filename in displayed list.
    rec:strarr( !NUM_FILES),$ ; String describing processing done.
    selected_files:lonarr(!NUM_FILES), $; Currently selected files.
    space:lonarr(!NUM_FILES), $; !SPACE_111, !SPACE_222, !SPACE_333, !SPACE_DATA
    nselected:0l, $ ; Number of selected files.
    paradigm_single:ptrarr(!NUM_FILES),$ ;Pointer for single trial paradigm.
    paradigm:ptrarr(!NUM_FILES),$ ;Stimulus time-course.
    default_paradigm:ptr_new(), $ ;Default stimulus time-course.
    paradigm_code:strarr( !NUM_FILES), $; ASCII encoded paradigm.
    paradigm_loaded:intarr( !NUM_FILES), $; !TRUE: paradigm loaded.
    zoom:fltarr( !NUM_FILES), $; Zoom factor.
    view:intarr( !NUM_FILES), $; !TRANSVERSE, !SAGITAL, or !CORONAL
    nfiles:0L,  $   ; Number of files loaded.
    n:0L,       $   ; Current file being displayed.
    secondary:intarr(!NUM_FILES+1), $ Secondary image index associated with fi.n
    color_scale1:intarr( !NUM_FILES+1),    $ ; Color scale for this file.
    color_min1:intarr( !NUM_FILES+1),  $ ; Parameter set by xloadct
    color_max1:intarr( !NUM_FILES+1),  $ ; Parameter set by xloadct
    color_gamma1:fltarr( !NUM_FILES+1),    $ ; Parameter set by xloadct
    color_lct1:ptrarr(!NUM_FILES+1),  $ ; Color lookup tables.
    color_scale2:intarr( !NUM_FILES+1),    $ ; Color scale for this file.
    color_min2:intarr( !NUM_FILES+1),  $ ; Parameter set by xloadct
    color_max2:intarr( !NUM_FILES+1),  $ ; Parameter set by xloadct
    color_gamma2:fltarr( !NUM_FILES+1),    $ ; Parameter set by xloadct
    color_lct2:ptrarr(!NUM_FILES+1),  $ ; Color lookup tables.
    current_colors:0L,  $ ; Current color table.
    lun:lonarr( !NUM_FILES), $ ; Logical units for data files.
    data_ptr:ptrarr( !NUM_FILES), $; Assoc data files.
    hdr_ptr:ptrarr( !NUM_FILES), $ Pointers for image structs.
    atlas_pix:fltarr(3,!NUM_FILES),$ ; Scale factors for atlas transformation.
    atlas_off:fltarr(3,!NUM_FILES),$ ; Offset for atlas transformation.
    csh_file:'',          $   ; File-name used for shell script.
    printfile:'idl.ps',$ ; Temporary file for printing.
    cw:0L, $ ; Current working window
    ;file_source:intarr(!NUM_FILES), $ ;0:!LOADED   1:!CREATED
    whereisit:intarr(!NUM_FILES),    $ ;0:!INTERNAL 1:!ON_DISK
    min:fltarr(!NUM_FILES), $
    max:fltarr(!NUM_FILES) $
    }

pr = { Profiles,    $
    profcmds:ptrarr( !MAX_PROFILES), $ ; Pointers for profile commands
    true_scl:!FALSE, $ ; !TRUE: Don't scale time profiles.
    prof_on:!FALSE, $ ; !TRUE: A profile window is active.
    prof_proc:!PROF_PROC_NOTHING,$ ; !Processing to be done in profile-region mode.
    mode:!DRAW_OFF, $ ; !DRAW_OFF, !DRAWING, !DRAW_DONE
    coord:intarr(2),$ ; Row,column used in profile mode.
    hprof:!FALSE,   $ ; Horizontal profile
    oprof: -12345,  $ ; Overplotted profile.
    roiprof:!FALSE, $ ; True: Compute sum of profile region only.
    horzprof:0L,    $ ; In horizontal profile mode.
    width:1,    $ ; Width of profile in pixels.
    height:1,   $ ; Height of profile in pixels.
    frames_to_skip:4, $ ; Number of frames to skip at start of time profile.
    symtype:!LINE,  $ ; Symbol type for profile.
    sym:intarr(20), $ ; Symbols used in plots.
    profcmd:strarr(20), $ ; List of commands for current profile.
    plotfiles:strarr(20), $ ; Data in current profile.
        plot_title:'',  $ ; Title of current profile.
    nprof:0L,   $ ; Number of profiles on current plot.
    nwindows:0L,    $ ; Number of profile windows.
    cur_window:-1,  $ ; Current profile window number.
    base:intarr( !PROF_NWIN), $ ; Current profile base widget.
    draw:intarr( !PROF_NWIN), $ ; Current profile draw widget.
    id:intarr( !PROF_NWIN), $ ; Current profile window number.
    krnl_width:0L,  $ ; Width of smoothing kernel.
    krnl_type:0L,  $ ; Width of smoothing kernel.
    regional:!FALSE, $; Average over a region?
    scatter_plot:!FALSE, $; Is this a scatter plot?
    time_type:!PLAIN_TPROF,$ ; !PLAIN_TPROF: sum over region.
;                      !THRESH_TPROF: sum over thresholded region.
    last_type:!PLAIN_TPROF,$ ; ! Last value of pr.time_type.
    time_overplot:!PLOT_PARADIGM,   $ ; Type of overplot for time profiles.
    time_thresh:.5, $ ; Threshold for time profiles.
    time_thresh_type:!SIGNIFICANCE, $ ; Type of threshold, SIGNIFICANCE or PERCENT
    subtract_trend:!FALSE, $ ; !TRUE: Subtract linear trend.
    x:0L,       $ ; x coordinate of profile.
    y:0L,       $ ; y coordinate of profile.
    z:0L,       $ ; z coordinate of profile.
    gang_dim:3, $   ; Number of plots in gang.
    gang_subdim:1,  $   ; Number of voxels to sum for each gang plot.
    path:'',    $ ; Path to save profile file.
    color:0 $ ;!GREEN    $ ; Color of profile
    }

dsp1 = { Display,    $
    sizex:256,  $   ; Size of image display.
    sizey:256,  $   ; Size of image display.
    dsizex:0,  $   ; Size of draw widget.
    dsizey:0,  $   ; Size of draw widget.
    x0:0L,      $
    y0:0L,      $
    file_num:0,     $   ; Currently displayed file number.
    scale:!GLOBAL,  $   ; Type of scaling, global or local.
    conadj:0,  $
    table:!GLOBAL,  $   ; Number of current color table.
    show_color_scale:!FALSE,$    ; TRUE: Color scale is displayed.
    width_color_scale:!FALSE,$ ; Width of color scale in pixels.
    color_scale:0,$   ; Current color table index.
    adjust_color_top:0L, $ ; Top level widget of adjust_color_table widget.
    num_colors:256,$    ; Length of color table.
    len_colortab:251,$  ; Length of color table less plotting colors.
    len_colortab1:251,$  ; Length of color table less plotting colors.
    len_colortab2:251,$  ; Length of color table less plotting colors.
    white:1,    $   ; white color table index.
    yellow:1,   $   ; yellow color table index.
    red:1,      $   ; red color table index.
    green:1,    $   ; green color table index.
    blue:1,     $   ; blue color table index.
    black:0,    $   ; Black color table index.
    draw_mode:!DRAW_LOCATION,$ ;Report location or sum over box (!DRAW_BOX)
    cur_wrkg_wdw:0,$ ;Current working window.
    image_index:lonarr(20), $   ; Index used by wset for draw widget.
    id:intarr(500), $   ; Window ids from "display all" cmd
    draw:intarr(500), $ ; Draw ids from "display all" cmd
    base:intarr(500), $ ; Base ids from "display all" cmd
    cur_window:0L,  $   ; Index to next id to be assigned.
    nwindows:0L,    $   ; Total number of custom windows.
    view:!TRANSVERSE, $ ; Default view.
    orientation:!NEURO, $   ; Orientation of displayed images.
    img_minmax:fltarr(2), $
    min_primary:0., $ ; Primary image minimum.
    max_primary:0., $ ; Primary image maximum.
    min_secondary:0, $ ; Minimum of secondary image.
    max_secondary:0, $ ; Maximum of secondary image.
    fixed_min:0.,   $   ; User specified minimum value.
    fixed_max:0.,   $   ; User specified maximum value.
    xdim:0L,    $   ; Horizontal dimension of disp image.
    ydim:0L,    $   ; Vertical dimension of disp image.
    zdim:0L,    $   ; Depth dimension of disp image.
    tdim:0L,    $   ; Time dimension of disp image.
    type:0L,    $   ; Type of array, ASSOC_ARRAY, FLOAT_ARRAY etc.
    zoom:1.,     $  ; Zoom value for display window.
    zoomtype:!BILINEAR,  $  ; !NEAREST_NEIGHBOR, !BILINEAR, or !CUBIC_SPLINE
    ;atlas_xform_type:!TARGET_2A, $ !TARGET_2A: Display coordinates as if target 2a used. Otherwise TARGET_2B.
    ;    atlas_2a_to_2b:fltarr(3,3), $ Atlas transformation.
    ;    atlas_2a_to_2b_off:fltarr(3),   $ Atlas offset.
    low_bandwidth:!FALSE, $ ; TRUE: reduce display I/O.
    plane:1,    $   ; Current plane displayed.
    frame:1,    $   ; Current frame displayed.
    lct:intarr(256,3) $ ; Current color table.
    }

dsp = replicate(dsp1,!MAX_WORKING_WINDOWS)

sprf1 = {View_slice_pref, $
     sup_to_inf:!TRUE,         $ ; TRUE: order images from superior to inferior
     slice_not_z:!TRUE,        $ ; TRUE: specify slices by number, not by t88 z.
     slice_type:!ANATOMIC,     $ ; !ANATOMIC or !STATISTICAL
     view:!TRANSVERSE,         $ ; !TRANSVERS, !SAGITTAL, or !CORONAL
     sagittal_face_left:!TRUE, $ ; !TRUE: sagittal face left.
     first_slice:0L,           $ ; First slice to be loaded.
     last_slice:0L,            $ ; Last slice to be loaded.
     slice_incr:0L,            $ ; Slice increment.
     mask:!TRUE                $ ; mask image with 222 or 333 mask
     }

prf1 = {View_window_pref, $
     xdim_draw:1152L, $	; X dimension of window.
     ydim_draw:900L, $	; Y dimension of window.
     xgap:0L, $		; Starting x of image 1.
     ygap:0L, $		; Starting y of image 1.
     ;dx:150L, $
     ;dy:150L, $
     ;dz:75L, $
     transverse_width:0L, $
     transverse_height:0L, $
     sagittal_width:0L, $
     sagittal_height:0L, $
     coronal_width:0L, $
     coronal_height:0L, $
     sx:0L, $		; x position of label
     sy:0L, $		; y position of label

     ;START110701
     lcf:0, $

     fx:0L, $		; x position of file name label
     fy:0L, $		; y position of file name label
     font_family:'', $	; Default font family (times, helvetica, courier, etc.)
     font_size:0L, $	; 
     font_bold:0L, $	; 
     font_italic:0L, $	; 
     paint:0L,  $ ; TRUE: paint all cells.
     color_x:0L, $	; x position of color scale.
     color_y:0L, $	; y position of color scale.
     color_bar_type:0L, $; !NONE, !HORIZ_COLOR_BAR, !VERT_COLOR_BAR
     lc_act_type:0L,    $ ; !POSITIVE_ACTIVATIONS, !NEGATIVE_ACTIVATIONS, !ALL_ACTIVATIONS
     lc_auto_scale:0L,  $ ; !TRUE: auto-scale, else manually scale.
     scale_pos_max:0.,  $ ; Scale positives to this maximum.
     scale_pos_min:0.,  $ ; Scale positives to this minimum.
     scale_neg_max:0.,  $ ; Scale negatives to this maximum.
     scale_neg_min:0.,  $ ; Scale negatives to this minimum.
     scale_anat_min:0., $ ; Minimum anatomy value.
     scale_anat_max:0., $ ; Maximum anatomy value.
     anat_local_global:0L, $
     crop_anat:0., $
     color_max1:0.,     $ ; Top value of base color scale.
     color_min1:0.,     $ ; Bottom value of base color scale.
     color_gamma1:0.,   $ ; Gamma correction of base color scale.
     color_max2:0.,     $ ; Top value of overlay color scale.
     color_min2:0.,     $ ; Bottom value of overlay color scale.
     color_gamma2:0.,   $ ; Gamma correction of overlay color scale.
     color_table1:0L,   $ Primary color scale type.
     color_table2:0L,   $ Overlay color scale type.
     display_units:0L,   $ Units for values displayed under cursor control.
     coding:0L, $
     black_background:!TRUE,   $ Type of background, black or white.
     black_background_mask:!FALSE,   $ Type of background, black or white.
     zoom:0L,  $ ; Zoom factor.
     overlay_zoom_type:0L,  $ ;
     orientation:0L,  $ ; !NEURO or !RADIOL, right/left orientation of images.
     printer_name:'',   $ ; Name of printer.
     print_xdim:0.,   $ ; X dimension of output.
     print_ydim:0.,   $ ; Y dimension of output.
     print_x0:0.,   $ ; X origin of output.
     print_y0:0.,   $ ; Y origin of output.
     print_orientation:0L,   $ ; !PORTRAIT or !LANDSCAPE
     print_color:0L,   $ ; !TRUE or !FALSE
     points_shape:0L, $
     points_color:0L, $
     points_size:0., $
     label_color:0L $
     }

af3d1 = {Af3d_symbol_pref, $
    shape:'',       $
    shape_index:0,  $
    psym:0,  $
    color:'',       $
    color_index:0,  $
    fill:'',        $
    size:0., $ 
    thick:0. $
    }
af3dp1 = {Af3d_points, $
    nfiles:0L, $
    files:ptr_new(), $
    coordinates:ptr_new(), $ 
    x:ptr_new(), $
    y:ptr_new(), $
    z:ptr_new(), $
    nindex:0L $
    }
reg1 = {Reg, $
    checked:ptr_new(), $
    color:ptr_new(), $
    color_index:ptr_new(), $
    colorbarlabels:ptr_new(), $
    pixmap_file:ptr_new(), $
    val:ptr_new() $
    }
;START59
label1 = {Label, $
    checked:ptr_new() $
    }


found = {Find_cell, $
     itr:0L,  $       ; Translation table index of cell that was found.
     lnum:0l $       ; Number of label that was found.
     }

cell1 = { View_Window_Cell, $
    x:0L, $           ; X coordinate on screen.
    y:0L, $           ; Y coordinate on screen.
    layer:0L, $
    xdim:0L, $        ; X dimension of the data.
    ydim:0L, $        ; Y dimension of the data.

    ;xpad:0L, $        ; X pad of the data.
    ;ypad:0L, $        ; Y pad of the data.
    ;START180309
    xpad:0., $        ; X pad of the data.
    ypad:0., $        ; Y pad of the data.

    base_min:0., $    ; Minimum of base image.
    base_max:0., $    ; Maximum of base image.
    overlay_min:0., $ ; Minimum of base image.
    overlay_max:0., $ ; Maximum of base image.
    valid:!FALSE, $
    paint:!FALSE, $    ; TRUE: Display this cell
    image_set:-1L, $  ; ID of image set.
    new_row:!FALSE, $  ; !TRUE: Start new row.
    view:!TRANSVERSE, $
    slice:0L, $
    base_image:ptr_new(), $
    base_image_val:ptr_new(), $
    overlay_image:ptr_new(), $
    overlay_image_val:ptr_new(), $
    nlab:0L, $         ; Number of labels.
    labels:ptr_new(), $
    freeze:!FALSE $
    }

;sLabel = { View_label, $
;    x:0L, $ ; X position.
;    y:0L, $ ; Y position.
;    wdth:0L, $ ; Width
;    hgth:0L, $ ; Height
;    font:'', $ ; Font name.
;    s:'' $; Label
;    }
;START59
sLabel = { View_label, $
    x:0L, $ ; X position.
    y:0L, $ ; Y position.
    wdth:0L, $ ; Width
    hgth:0L, $ ; Height
    font:'', $ ; Font name.
    s:'', $; Label
    color:'', $
    color_index:0L, $
    pixmap_file:'' $
    }






win = {View_window, $
    xdim_draw:512L, $
    ydim_draw:512L, $
    xmax:512L, $  ; Largest x pixel number.
    ymax:512L, $  ; Largest y pixel number.
    atlas_space:intarr(!NUM_FILES), $  ; !SPACE_222, !SPACE_333 or !SPACE_DATA
    lastx:0, $
    lasty:0, $
    nlayer:1, $
    black_background:!TRUE, $
    black_background_mask:!FALSE, $
    color_bar:!FALSE, $
    ncell:0, $
    name:'', $  ; Name of window.
    insertion_point:0L, $ ; Point at which new images inserted  into display
    num_idx:0L, $ ; Number of files loaded.
    idxidx:0L,  $ ; Index to current image indices.
    idxidxcur:0L,  $ ; Currently plotted index. 
    resliced:lonarr(!NUM_FILES), $  ; !TRUE: Image set has already been resliced.
    image_idx:lonarr(!NUM_FILES), $
    mask_idx:lonarr(!NUM_FILES), $
    over_idx:lonarr(!NUM_FILES), $
    over_tidx:lonarr(!NUM_FILES), $
    nreg:lonarr(!NUM_FILES), $ ; = #regions if overlay is a region file
    translation_table:lonarr(!MAX_CELLS), $ ; Lists cells to be displayed.
    ntrans:0L, $   ; Length of translation table
    prf:prf1, $    ; Window preferences.
    sprf:replicate(sprf1,!NUM_FILES), $
    cell:replicate(cell1,!MAX_CELLS), $
    naf3dpts:0L, $                           ;shall be deleted
    af3dp:replicate(af3dp1,!NUM_FILES), $    ;shall be deleted
    af3d_new:replicate(af3d1,!NAF3D_FILES), $
    af3dp_new:replicate(af3dp1,!NAF3D_FILES), $
    af3dpts_new:replicate(ptr_new(),!NAF3D_FILES), $ ;replicate(af3dpts1,!NAF3D_FILES), $
    naf3dpts_new:intarr(!NAF3D_FILES), $
    af3d_checked:intarr(!NAF3D_FILES), $
    reg:replicate(reg1,!NUM_FILES), $
    over_max:lonarr(!NUM_FILES), $
    over_min:lonarr(!NUM_FILES), $

    ;START110701
    ;lcf:0, $

    filenamelabels:strarr(!NUM_FILES), $
    nlab:0L, $         ; Number of labels.
    labels:ptr_new(), $
    label:replicate(label1,!NUM_FILES) $
    }


font1 = {View_font,    $
    family:'times', $  	; Font name.
    bold:!FALSE, $	; 
    italic:!FALSE, $	; 
    size:'14', $         ; Size in points.
    current_font_number:'', $  ; Currently used font.
    current_font:'' $
    }

vw = {View_images,    $
    win:ptrarr(!VIEW_WINDOWS_MAX),$ ; Structures defining each window.
    id:lonarr(!VIEW_WINDOWS_MAX), $; IDs of display windows.
    wdw:lonarr(!VIEW_WINDOWS_MAX), $; window number of draw window
    base:lonarr(!VIEW_WINDOWS_MAX), $; IDs of widget bases
    used_name:lonarr(!VIEW_WINDOWS_MAX), $; TRUE: name is used.
    cw:0,             $ ; Index to current window.
    num_windows:0,    $
    view_leader:0L,   $ ; Top level widget
    nclip:0L,   $ ; Number of cells on the clipboard
    clipboard:replicate(cell1,!MAX_CELLS), $
    fnt:font1 $
    }
     
fl = { Flags,       $
	first:!TRUE,    $
	colscl:!NEVER,  $
	custom:!FALSE,  $
	plnplt:!FALSE   $
	}

    st = { State,       $
	fidl_rev:'',    $
	pln_sld_min:0L, $
	pln_sld_max:1,  $
	last_vol_num:-1,$
	last_evid:0L,   $
	second_last_evid:0L,   $
	last_crc_diam:10,$ ; Last diameter of circular roi.
	last_sq_side:10,$ ; Last diameter of circular roi.
	roicolor:0, $ ;!GREEN, $
	ldfilter:'',    $ ; Filter used for reading images.
	error:0L,   $
	error_msg:''    $
	}

    pref = {Preferences,    $
	wilc_sig:1, $   ; Save significance image.
	wilc_prob:1,    $   ; Save log probability image.
	wilc_Z:1,   $   ; Save Z statistic image.
	diff_var:0L,    $   ; Save variance image.
	diff_pct:1, $   ; Save percent change image.
	diff_diff:1,    $   ; Save difference image.
	diff_z:1,   $   ; Save Z statistic image.
	glm_mag:1,  $   ; Save magnitude
	glm_var:1,  $   ; Save variance
	glm_z:1,    $   ; Save Z statistic image.
	glm_t:1,    $   ; Save T statistic image.
	slope:0L,   $   ; Save linfit slope
	intercept:0L,   $   ; Save linfit intercept
	paraskip:4, $   ; Frames to skip.
	parainit:9, $   ; Number of inital control frames.
	paraact:18, $   ; Number of activation frames per cycle.
	paractl:18, $   ; Number of control frames per cycle.
	TR:2.5,     $   ; Frame spacing in seconds.

	;period:7L,  $   ; Assumed duration of hemodynamic response.
        ;START170224
	period:7,  $   ; Assumed duration of hemodynamic response.

	orientation:!NEURO,  $   ; Default image orientation
	orientation_hide_label:0,  $   ; Hide 'Rad' or 'Neuro' 
        printer_name:'',    $   ; Default printer.
        concpath:'', $
        fidlpath:'', $
        rhost:ptr_new(), $
        rpath:ptr_new(), $
        chunk:1, $

        ;START170224
        cpu:'', $

	residuals_avg:!TRUE, $ ; !TRUE: Average across runs.
	residuals_pctchg:!TRUE, $ ; !TRUE: Express as percent change.
	residuals_skip:!FALSE, $ ; !TRUE: Skip first four frames
        residuals_keep_list:strarr(100), $  ! List of effects to include
	output_file:'', $ ; Output file name.
	glm_pause:!FALSE, $ ; !TRUE: Scanner paused during acquisition.
	glm_hipass:!FALSE, $ ; !TRUE: Include hi-pass filter.
	glm_hipass_cutoff:!HIPASS_CUTOFF, $ ; Cutoff frequency of hipass filter.
	glm_interaction:!FALSE, $ ; !TRUE: Include interaction terms.
	glm_basis_class:!BASIS_DELTA, $ ; Regressor: !BASIS_DELTA or !BASIS_FUNC
	glm_basis_model:!BOYNTON, $ ; if glm_basis_class = !BASIS_FUNC, this
    ;                                specifies the model function. 
    ;                                Values: !BOYNTON, !FRISTON, !BOXCAR_STIM.
	glm_basis_length:8, $ ; Length of HRF model in frames.
	glm_eff_pause:0, $ ; Effect number of no-stimulus trials.
	glm_eff_pause_length:4, $ ; Number of frames for T1 to equilibrate.
	glm_pause_per_run:!TRUE, $ ; !TRUE: Estimate T1 effect separately for each run.
	glm_comp_init:!TRUE,   $ ; !TRUE: Initialze script file. 
	glm_comp_xform:!FALSE, $ ; !TRUE: Transform to atlas space.
	glm_comp_t4_file:'',   $ ; T4 file for xform to atlas space.
	glm_comp_smoth:!FALSE, $ ; !TRUE: Smooth with a Gaussian filter.
	glm_comp_fwhm: 2, $      ; FWHM of Gaussian filter.
	glm_comp_get_smoth:!FALSE, $ ; !TRUE: Compute smoothness estimate.
	glm_comp_concfile:'',      $  ; Name of *.conc file specifying time-course data.
	glm_comp_glmfile:'',   $ ; Name of *.glm file.o
	glm_event_file:'',     $ ; Name of event file.
	glm_comp_new:!TRUE,    $ ; !TRUE: Write new script file, else append.
	;glm_hrf_delay:!HRF_DELTA, $ ; Default delay for HRF model
	glm_hrf_delay:0.,     $ ; CHANGED 10/9/01 Boynton has a hard coded delay of 2s. However, this piece of code is $
                                ; used elsewhere. Seems dangerous to let it sit at 2. 
	glm_stim_duration:.1, $ ; Default stimulus duration.
	
	fwhm_hat:1.,    $   ; Estimated FWHM.
	glmsim_tdim:0L, $   ; Number of frames to be simulated.
	glmsim_period:6L,$  ; Simulated duration of hemodynamic response.
	glmsim_minsoa:1L,$  ; Minimum SOA.
	glmsim_maxsoa:2L,$  ; Maximum SOA.
	glmsim_pct_catch:0.,$   ; Percentage catch trials.
	glmsim_seed:0L, $   ; Starting seed for random number generator.
	glmsim_nreal:100, $ ; Number of realizations to compute.
	glmsim_maxchng:1.,$ ; Maximum percent change of signal.
	glmsim_tot_eff:2,$  ; Maximum percent change of signal.
	glmsim_stimlen:fltarr(!GLMSIM_MAXEFF),$ ; Maximum percent change of signal.
	glmsim_sigstr:fltarr(!GLMSIM_MAXEFF),$ ; Maximum percent change of signal.
	glmsim_sig:fltarr(!GLMSIM_MAX_STIMLEN,!GLMSIM_MAXEFF) $ ; Maximum percent change of signal.
        }

    ifh = {InterFile_Header, $
	modality:string('mri'),     $
	orig_system:string('MAGNETOM VISION'),      $
	conversion_program:string('fidl'),  $
	program_version:string(' '),    $
	program_date:string(' '),       $
	original_institution:string('Washington University'),   $
	data_file:string(' '),      $
	patient_ID:string(' '),     $
	study_date:string(' '),     $
	number_format:string('float'),  $
	bytes_per_pixel:4,  $
	orientation:2,      $
	time_series_flag:0, $
	num_dimensions:4,   $
	matrix_size_1:0L,    $
	matrix_size_2:0L,    $
	matrix_size_3:0L,    $
	matrix_size_4:0L,    $
	volume_sf:0.,       $
	scale_1:0.,     $
	scale_2:0.,     $
	scale_3:0.,     $
	slice_thickness:0.,$
	mat_init_element_1:string('right'), $
	mat_init_element_2:string('posterior'), $
	mat_init_element_3:string('inferior'),  $
	mri_param_file:' ', $
	mri_seq_file:' ', $
	mri_seq_description:' ', $
	mri_paradigm_format:' ', $
	global_min:0., $
	center:fltarr(3), $
	mmppix:fltarr(3), $

        ;START150702
	c_orient:ptr_new(),$
	;c_sform:ptr_new(),$
	;cf_flip:0,$
        ;START150714
	cf_flip:0L,$


        fwhm:0., $
        nreg:0L, $
        region_names:ptr_new(), $
        nregfiles:0L, $
        regfiles:ptr_new(), $
        harolds_num:ptr_new(), $
        behavior_names:ptr_new(), $ ; string array of names of behavioral names.  Behavioral data are stored in the first N voxels of the 4dfp stack.
	global_max:0.,$
	smoothness:0.,$
	df1:0., $
	df2:0., $
	bigendian:0,                $
	nsub:0,                $
	datades:'', $
	mask:'', $

	glm_rev:0,              $       ; glm revision number of file format.
	glm_N:0L,               $       ; glm total number of estimated parameters.
	glm_n_interest:0L,      $       ; glm number of effects of interest in dsgn matrix.
	glm_xdim:0L,             $       ; glm horizontal dimension of image.
	glm_ydim:0L,             $       ; glm vertical dimension of image.
	glm_zdim:0L,             $       ; glm depth dimension of image.
	glm_tdim:0L,            $       ; glm time dimension of images.
	glm_t_valid:0L,         $       ; glm number of valid time points.
	glm_df:0.,              $       ; glm degrees of freedom.
	glm_nc:0L,              $       ; glm number of contrasts
	glm_period:0L,          $       ; glm number of estimates per condition. (Length of HTF)
	glm_num_trials:0L,      $       ; glm number of stimuli presentations.
	glm_TR:0.,              $       ; glm time required to collect one frame.
	glm_tot_eff:0L,         $       ; glm number of effects of interest.
	glm_input_data_space:0L, $      ; glm input data space.
	glm_all_eff:0L,         $       ; glm total number of effects.
	glm_effect_label:ptr_new(), $   ; glm effect names
	glm_effect_length:ptr_new(),$   ; glm parameters estimated per effect
	glm_effect_column:ptr_new(),$   ; glm Starting column in dsgn matrix.
        glm_effect_TR:ptr_new(), $      ; Effective TR of the condition.
        glm_effect_shift_TR:ptr_new(),$ ; Condition has been shifted some fraction of a TR from interleaving.
	glm_n_files:0L,          $      ; glm number of files.
        glm_funclen:ptr_new(),   $      ; Number of elements in basis set. dimension = tot_eff
        glm_functype:ptr_new(),  $      ; Basis function type (!BOYNTON or !FRISTON) dimension = tot_eff
	glm_fwhm:0.,                $   ; glm fwhm in voxels. 
	glm_W:0.,                $      ; glm smoothness estimate.
	glm_fwhmx:0.,            $      ; glm smoothness estimate in units of FWHM.
	glm_fwhmy:0.,            $      ; glm smoothness estimate in units of FWHM.
	glm_fwhmz:0.,            $      ; glm smoothness estimate in units of FWHM.
	glm_dxdy:1.,             $      ; glm XY voxel size.
	glm_dz:1.,               $      ; glm Z voxel size.
	glm_nF:1,                $      ; glm number of F statistics.
	glm_F_names:ptr_new(),   $      
	glm_F_names2:ptr_new(),  $      
	glm_event_file:ptr_new(),$      
	glm_mask_file:ptr_new(), $      
        glm_nt4files:0L,         $
	glm_t4files:ptr_new(),   $      
        glm_boldtype:'', $
        glm_cifti_xmlsize:0L, $

        ;START160906
        glm_masked:0, $

        msg:'' $
	}
get_labels_struct = {Get_labels_struct, $
    nscraplabels:0, $
    scraplabels:ptr_new(), $
    cnorm:ptr_new(), $
    ifh:{Interfile_header}, $
    sum_all:ptr_new(), $
    index_glm:ptr_new(), $
    effect_length:ptr_new(), $
    stimlen:ptr_new(), $
    contrastlabels:ptr_new(), $
    ncontrastlabels:0, $
    sum_all_cl:ptr_new(), $
    index_glm_cl:ptr_new(), $
    effectlabels:ptr_new(), $
    neffectlabels:0, $
    sum_all_el:ptr_new(), $
    index_glm_el:ptr_new() $
    }


cop_struct = {Cop_struct, $
    typestr:'', $
    coeff:ptr_new(), $
    coeffstr:ptr_new() $
    }
identify_struct = {Identify_struct, $
    n:0, $
    i:ptr_new(), $
    ni:ptr_new(), $
    s:ptr_new(), $
    starti:ptr_new() $
    }
