runConfig="bbbbAnalysis/RunConfig-HH4b-All.yaml"
executable="bbbb-ntupler"
campaignName="EJ_v0_43_HH4b_Syst"

dir_samples="../easyjet/bbbbAnalysis/datasets/HH4b/"
mc_list=(
    "$dir_samples/mc20_DAOD_PHYS_HH4b_nonres.txt"
    "$dir_samples/mc20_DAOD_PHYS_HH4b_resonant.txt"
)

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName} \
    --noTag \
    --dest-se CERN-PROD_LOCALGROUPDISK

rm -rf co*