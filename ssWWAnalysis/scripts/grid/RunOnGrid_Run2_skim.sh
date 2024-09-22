runConfig="ssWWAnalysis/RunConfig-Signal-ssWW.yaml"
executable="ssWW-ntupler"
campaignName="ssWWVBS_v00"

dir_samples="../easyjet/ssWWAnalysis/datasets/PHYS/"
mc_list=(
    "$dir_samples/mc20_ssWW_MGHW_p6266.txt"
    "$dir_samples/mc20_ssWW_Sh222_p6266.txt"
    "$dir_samples/mc20_ssWW_Sh3_p6266.txt"
    "$dir_samples/mc20_EWWZ_p6266.txt"
    "$dir_samples/mc20_QCDWZ_p6266.txt"
    "$dir_samples/mc20_TopV_p6266.txt"
    "$dir_samples/mc20_Vgamma_p6266.txt"
    "$dir_samples/mc20_Vjets_p6266.txt"
    "$dir_samples/mc20_ZZ_p6266.txt"
)

#data 
easyjet-gridsubmit --data-list $dir_samples/data_Run2_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag
