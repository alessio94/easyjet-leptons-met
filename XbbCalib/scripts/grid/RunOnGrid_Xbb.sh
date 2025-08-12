ptag="p6266"
runConfig="XbbCalib/RunConfig-XbbCalib.yaml"
executable="xbbcalib-ntupler"
campaignName="001"

mc_compaign="mc20_13TeV" # or "mc23_13p6TeV"
path="../easyjet/XbbCalib/datasets/ttbar/"

mc_list=(
    "${path}/mc20_ttbar_${ptag}.txt"
    "${path}/mc20_singletop_${ptag}.txt"
    "${path}/mc20_Wjet_${ptag}.txt"
    "${path}/mc20_Zjet_${ptag}.txt"
)

# check data campaign
if [ "$mc_campaign" == "mc20_13TeV" ]; then
    data="data_Run2_${ptag}.txt"
elif [ "$mc_campaign" == "mc23_13p6TeV" ]; then
    data="data_13p6TeV.Run3.${ptag}.txt"
else
    echo "Unknown mc_campaign: $mc_campaign"
    data=""
fi

#data 
easyjet-gridsubmit --data-list ${path}/${data} \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName} \
    --noSubmit

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName} \
    --noSubmit

