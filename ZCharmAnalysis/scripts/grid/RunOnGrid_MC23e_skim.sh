runConfig="ZCharmAnalysis/RunConfig-ZCharm.yaml"
executable="ZCharm-ntupler"
campaignName="ZCharm_v00"

dir_samples="../easyjet/ZCharmAnalysis/datasets/PHYS/MC23e"
mc_list=(
    "$dir_samples/mc23e_Zjets_Sh_p6522.txt"
    "$dir_samples/mc23e_VV_p6522.txt"
    "$dir_samples/mc23e_ttbar_p6522.txt"
    "$dir_samples/mc23_stop_p6522.txt"
)

#data 
easyjet-gridsubmit --data-list $dir_samples/data_Run3_p6700.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 10 \
    --campaign ${campaignName} \
    --noTag

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 10 \
    --campaign ${campaignName} \
    --noTag