runConfig="ssWWAnalysis/RunConfig-ssWW-VBS.yaml"
executable="ssWW-ntupler"
campaignName="ssWWVBS_v00"

dir_samples="../easyjet/ssWWAnalysis/datasets/PHYSLITE/"
mc_list=(
    "$dir_samples/mc20_Zjet_background_p6026.txt"
    "$dir_samples/mc20_Wjet_background_p6026.txt"
    "$dir_samples/mc20_top_background_p6026.txt"
)

#data 
easyjet-gridsubmit --data-list $dir_samples/data_Run2_p6026.txt \
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
