runConfig="bbllAnalysis/RunConfig-bbll-bypass.yaml"
executable="bbll-ntupler"
campaignName="XHHbbll_v00"

dir_samples="../easyjet/bbllAnalysis/datasets/PHYSLITE/"
mc_list=(
    "$dir_samples/mc23_Zjet_background_p6026.txt"
    "$dir_samples/mc23_Wjet_background_p6026."
    "$dir_samples/mc23_top_background_p6026.txt"
)
#No Run3 ggH samples yet

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
