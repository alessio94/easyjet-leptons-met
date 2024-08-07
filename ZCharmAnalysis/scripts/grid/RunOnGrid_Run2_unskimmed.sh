runConfig="ZCharmAnalysis/RunConfig-ZCharm-bypass.yaml"
executable="ZCharm-ntupler"
campaignName="ZCharm_v00"

dir_samples="../easyjet/ZCharmAnalysis/datasets/PHYS/"
mc_list=(
    "$dir_samples/Zjets_Run2_p6266.txt"
    "$dir_samples/Diboson_Run2_p6266.txt"
    "$dir_samples/ZH_Run2_p6266.txt"
    "$dir_samples/Top_Run2_p6266.txt"
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
