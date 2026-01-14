ptag=p7018 #tag for Run 2 data, mc20 bkg, mc20 sig
ptag=p7017 #tag for mc23 signal and mc23 bkg
ptag=p7019 #tag for Run3 data

campaign=v1

dir_samples="../easyjet/HZallyyAnalysis/datasets/PHYS"
dir_config="../easyjet/HZallyyAnalysis/share"

mc_list=(
    "$dir_samples/mc20_SIG_$ptag.txt"  
    "$dir_samples/mc20_BKG_$ptag.txt"
    "$dir_samples/data_Run2_$ptag.txt"
    "$dir_samples/mc23_SIG_$ptag.txt" 
    "$dir_samples/mc23_BKG_$ptag.txt" 
    "$dir_samples/data_Run3_$ptag.txt" 
)

easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config "$dir_config/RunConfig-llyy.yaml" \
    --exec llyy-ntupler \
    --nGBperJob MAX \
    --campaign ${campaign} \
    --mergeWithHadd \
    --mergeOutput \
    --noTag \

