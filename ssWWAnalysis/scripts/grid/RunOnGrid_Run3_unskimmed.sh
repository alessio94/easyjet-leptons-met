runConfig="ssWWAnalysis/RunConfig-Signal-ssWW-bypass.yaml"
executable="ssWW-ntupler"
campaignName="ssWWVBS_v00"

dir_samples="../easyjet/ssWWAnalysis/datasets/PHYSLITE/"
mc_list=(
    "$dir_samples/mc23_Zjet_background_p6026.txt"
    "$dir_samples/mc23_Wjet_background_p6026.txt"
    "$dir_samples/mc23_top_background_p6026.txt"
)

#data 
easyjet-gridsubmit --data-list $dir_samples/data_Run3_p6026.txt \
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
