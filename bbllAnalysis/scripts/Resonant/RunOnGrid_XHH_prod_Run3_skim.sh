runConfig="bbllAnalysis/RunConfig-bbll-Resonant-NW.yaml"
executable="bbll-ntupler"
campaignName="XHHbbll_v00"

dir_samples="../easyjet/bbllAnalysis/datasets/PHYS/"
mc_list=(
    "$dir_samples/mc23_resonantXHH_samples_p6266.txt"
    "$dir_samples/mc23_Zjet_background_p6266.txt"
    "$dir_samples/mc23_Wjet_background_p6266."
    "$dir_samples/mc23_top_background_p6266.txt"
    "$dir_samples/mc23_diboson_background_p6266.txt"
    "$dir_samples/mc23_singleH_background_p6226.txt"
)

#data 
easyjet-gridsubmit --data-list $dir_samples/data_Run3_p6266.txt \
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

