ptag=p7077 #tag for Run 2 data, mc20 bkg, mc23 nkg
ptag=p7079 #tag for Run3 data
ptag=p7106 #tag for signal mc20 and mc23

campaign=v1

dir_samples="../easyjet/HZallyyAnalysis/datasets/LLP1"
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
    --run-config "$dir_config/RunConfig-llyy_LLP1.yaml" \
    --exec llyy-ntupler \
    --nGBperJob MAX \
    --campaign ${campaign} \
    --mergeWithHadd \
    --mergeOutput \
    --noTag \
