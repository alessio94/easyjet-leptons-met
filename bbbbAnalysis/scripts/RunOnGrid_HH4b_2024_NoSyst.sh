runConfig="bbbbAnalysis/RunConfig-HH4b-All-NoSyst-Run3-2024.yaml"
executable="bbbb-ntupler"
campaignName="EJ_v0_43_HH4b_NoSyst"

dir_samples="../easyjet/bbbbAnalysis/datasets/HH4b/"
mc_list=(
    "$dir_samples/mc23_DAOD_PHYS_HH4b_nonres_2024.txt"
)

#data
easyjet-gridsubmit --data-list $dir_samples/data_DAOD_PHYS_Run3_2024.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag \
    --dest-se CERN-PROD_LOCALGROUPDISK

rm -rf co*

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag \
    --dest-se CERN-PROD_LOCALGROUPDISK

rm -rf co*
