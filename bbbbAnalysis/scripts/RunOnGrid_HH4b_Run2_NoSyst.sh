runConfig="bbbbAnalysis/RunConfig-HH4b-All-NoSyst.yaml"
executable="bbbb-ntupler"
campaignName="EJ_%Y_%m_%d_T%H%M%S_HH4b_NoSyst_v00"

dir_samples="../easyjet/bbbbAnalysis/datasets/HH4b/"
mc_list=(
    "$dir_samples/mc20_DAOD_PHYS_HH4b_nonres_p6490.txt"
    "$dir_samples/mc20_DAOD_PHYS_HH4b_resonant_p6490.txt"
    "$dir_samples/mc20_DAOD_PHYS_HH4b_background_p6490.txt"
    #"$dir_samples/mc20_DAOD_PHYS_HH4b_validation_p6490.txt"
)

#data
easyjet-gridsubmit --data-list $dir_samples/data_DAOD_PHYS_Run2_p6479.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName} \
    --noTag

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName} \
    --noTag