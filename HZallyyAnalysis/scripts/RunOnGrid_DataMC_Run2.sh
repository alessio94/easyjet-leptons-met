ptag=p6266
campaign=v1
dir_samples="../easyjet/HZallyyAnalysis/datasets/PHYS"
mc_campaign="mc20"
mc_list=(
    "$dir_samples/$mc_campaign_diboson_$ptag.txt"
    "$dir_samples/$mc_campaign_Zjet_$ptag.txt"
    "$dir_samples/$mc_campaign_Zy_$ptag.txt"
    "$dir_samples/$mc_campaign_Zyy_$ptag.txt"
    "$dir_samples/data_Run2_$ptag.txt"
)

easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config RunConfig-llyy.yaml \
    --exec llyy-ntupler \
    --nGBperJob 2 \ 
    --noTag \
    --campaign ${campaign}
