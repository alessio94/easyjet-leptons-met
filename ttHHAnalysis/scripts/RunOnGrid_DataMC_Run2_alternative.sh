ptag=p6490
campaign=v8-1
dir_samples="../easyjet/ttHHAnalysis/dataset/PHYSLITE"
mc_campaign="mc20_13TeV"

mc_list=(
    "$dir_samples/alternative/$mc_campaign.ttbar.$ptag.txt"
    "$dir_samples/alternative/$mc_campaign.ttH.$ptag.txt"
)

#mc
easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list[@]}") \
    --run-config ttHHAnalysis/RunConfig-ttHH-alternative.yaml \
    --exec ttHH-ntupler \
    --nGBperJob 3 \
    --campaign ${campaign} \
    --mergeOutput \
    #--noSubmit

