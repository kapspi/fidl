;Copyright 11/6/99 Washington University.  All Rights Reserved.
;get_atlas_labels.pro  $Revision: 1.7 $

pro get_atlas_labels,labels,files,natlas,THIS_SPACE_ONLY=this_space_only
labels = [ $
    'Standard adult (711-2B 222_t88)', $
    'Young and old adults combined (711-2C 222_t88)', $
    'Old adults (711-2O 222_t88)', $
    'Young adults (711-2Y 222_t88)', $
    'Colin (711-2B 222_t88)', $
    'TRIO_Y_NDC (222_t88)', $
    'TRIO_KY_NDC (222_t88)', $
    !BLANK, $
    'Standard adult (711-2B 333_t88)', $
    'TRIO_Y_NDC (333_t88)', $
    'TRIO_KY_NDC (333_t88)', $
    !BLANK, $
    'Standard adult (711-2B 111_t88)', $
    'Young and old adults combined (711-2C 111_t88)', $
    'Old adults (711-2O 111_t88)', $
    'Young adults (711-2Y 111_t88)', $
    'Kids (711-2K 111_t88)', $
    'TRIO_Y_NDC (111_t88)', $

    ;'TRIO_KY_NDC (111_t88)' $
    ;START180118
    'TRIO_KY_NDC (111_t88)', $
    'mni_icbm152_t1_tal_nlin_sym_09a' $

    ]
files = [ $
    !ATLAS_FILE_222, $
    !ATLAS_711_2C_222, $
    !ATLAS_711_2O_222, $
    !ATLAS_711_2Y_222, $
    !COLIN_222, $
    !TRIO_Y_NDC_222, $
    !TRIO_KY_NDC_222, $
    '', $
    !ATLAS_FILE_333, $
    !TRIO_Y_NDC_333, $
    !TRIO_KY_NDC_333, $
    '', $
    !ATLAS_FILE_111, $
    !ATLAS_711_2C_111, $
    !ATLAS_711_2O_111, $
    !ATLAS_711_2Y_111, $
    !ATLAS_711_2K_111, $
    !TRIO_Y_NDC_111, $

    ;!TRIO_KY_NDC_111 $
    ;START180118
    !TRIO_KY_NDC_111, $
    !mni_icbm152_t1_tal_nlin_sym_09a $

    ]
if keyword_set(THIS_SPACE_ONLY) then begin
    case this_space_only of
        !SPACE_222: begin
            labels = labels[[0,1,2,3,4,5,6]]
            files = files[[0,1,2,3,4,5,6]]
        end
        !SPACE_333: begin
            labels = labels[[8,9,10]]
            files = files[[8,9,10]]
        end
        !SPACE_111: begin

            ;labels = labels[[12,13,14,15,16,17,18]]
            ;files = files[[12,13,14,15,16,17,18]]
            ;START180118
            labels = labels[[12,13,14,15,16,17,18,19]]
            files = files[[12,13,14,15,16,17,18,19]]

        end
        else: ;do nothing
    endcase
endif
natlas = n_elements(files)
end
