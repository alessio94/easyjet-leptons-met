runConfig="bbllAnalysis/RunConfig-bbll-Resonant-NW-Run3.yaml"
executable="bbll-ntupler"
campaignName="XHHbbll_v08_systprod"

dir_samples="../easyjet/bbllAnalysis/datasets/PHYSLITE/p6697/"
mc_list=(
    "$dir_samples/mc23_resonantXHH_samples_p6697.txt"
    "$dir_samples/mc23_ggFXHH_samples_p6697.txt"
    "$dir_samples/mc23_Zjet_background_p6697.txt"
    "$dir_samples/mc23_Wjet_background_p6697.txt"
    "$dir_samples/mc23_diboson_background_p6697.txt"
    "$dir_samples/mc23_singleH_background_p6697.txt"
    "$dir_samples/mc23_top_background_p6697.txt"
    "$dir_samples/mc23_alternative_samples_p6697.txt"
)

#data 
easyjet-gridsubmit --data-list $dir_samples/data_Run3_p6697.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag \
    --noEmail

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag \
    --noEmail

