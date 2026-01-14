runConfig="ssWWAnalysis/RunConfig-Signal-ssWW-bypass.yaml"
executable="ssWW-ntupler"
campaignName="ssWWVBS_v00"

dir_samples="../easyjet/ssWWAnalysis/datasets/PHYS/"
mc_list=(
    "$dir_samples/mc23_Vjets_p6266.txt"
    "$dir_samples/mc23_Vgamma_p6266.txt"
    "$dir_samples/mc23_Top_p6266.txt"
    "$dir_samples/mc23_ZZ_p6266.txt"
    "$dir_samples/mc23_QCDWZ_p6266.txt"
)

#data 
easyjet-gridsubmit --data-list $dir_samples/data_Run3_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag
