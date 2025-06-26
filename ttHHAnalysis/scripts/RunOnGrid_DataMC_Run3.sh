ptag=p6491
campaign=v8-1
dir_samples="../easyjet/ttHHAnalysis/dataset/PHYSLITE"
mc_campaign="mc23_13p6TeV"

mc_list=(
    "$dir_samples/nominal/$mc_campaign.ttHH.signal.$ptag.txt"
    "$dir_samples/nominal/$mc_campaign.ttbar.$ptag.txt"
    "$dir_samples/nominal/$mc_campaign.ttH.$ptag.txt"
    "$dir_samples/nominal/$mc_campaign.ttV.$ptag.txt"
    "$dir_samples/nominal/$mc_campaign.ttVV.$ptag.txt"
    "$dir_samples/nominal/$mc_campaign.tttt.$ptag.txt"
    "$dir_samples/nominal/$mc_campaign.ttt.$ptag.txt"
    "$dir_samples/nominal/$mc_campaign.singleTop.$ptag.txt"
    "$dir_samples/nominal/$mc_campaign.VH.$ptag.txt"
    "$dir_samples/nominal/$mc_campaign.Vjets.$ptag.txt"
    "$dir_samples/nominal/$mc_campaign.VV.$ptag.txt"
    "$dir_samples/nominal/$mc_campaign.VVV.$ptag.txt"
)

#data 
easyjet-gridsubmit --data-list "$dir_samples/nominal/data_13TeV.Run2.p6479.txt" \
    --run-config ttHHAnalysis/RunConfig-ttHH-nominal.yaml \
    --exec ttHH-ntupler \
    --nGBperJob 50 \
    --campaign ${campaign} \
    --mergeOutput \
    #--noSubmit

#mc
easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list[@]}") \
    --run-config ttHHAnalysis/RunConfig-ttHH-nominal.yaml \
    --exec ttHH-ntupler \
    --nGBperJob 3 \
    --campaign ${campaign} \
    --mergeOutput \
    #--noSubmit
