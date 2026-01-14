runConfig="ZQGAnalysis/RunConfig-ZQG.yaml"
executable="ZQG-ntupler"
campaignName="ZQG_v01"

dir_samples="../easyjet/ZQGAnalysis/datasets/PHYS/"
mc_list=(
    "$dir_samples/mc23_Zjets_MG_p6697.txt"
    "$dir_samples/mc23_Zjets_Sh_p6697.txt"
    "$dir_samples/mc23_VV_p6697.txt"
    "$dir_samples/mc23_ttbar_p6697.txt"
    "$dir_samples/mc23_stop_p6697.txt"
)

#data 
easyjet-gridsubmit --data-list $dir_samples/data_Run3_p6700.txt \
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
