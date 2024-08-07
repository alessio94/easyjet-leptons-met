runConfig="multileptonAnalysis/RunConfig-multilepton-bypass.yaml"
executable="hhml-ntupler"
campaignName="HHML_v00"

dir_samples="../easyjet/multileptonAnalysis/datasets/PHYSLITE/"
mc_list=(
    "$dir_samples/mc20_ggFXHH_samples_p5855.txt"
    "$dir_samples/mc20_Zjet_background_p6026.txt"
    "$dir_samples/mc20_Wjet_background_p6026."
    "$dir_samples/mc20_top_background_p6026.txt"
)

#data 
easyjet-gridsubmit --data-list $dir_samples/data_Run2_p6026.txt \
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
