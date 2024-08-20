ptag="p6266"
runConfig="ttHHAnalysis/RunConfig-ttHH.yaml"
executable="ttHH-ntupler"
campaignName="001"

mc_compaign="mc20_13TeV" # or "mc23_13p6TeV"
folder_name_PHYSLITE="../easyjet/ttHHAnalysis/dataset/PHYSLITE"

mc_list=(
    "${folder_name_PHYSLITE}/nominal/${mc_compaign}.ttHH.signal.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/${mc_compaign}.ttbar.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/${mc_compaign}.ttH.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/${mc_compaign}.ttV.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/${mc_compaign}.ttVV.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/${mc_compaign}.tttt.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/${mc_compaign}.ttt.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/${mc_compaign}.singleTop.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/${mc_compaign}.VVV.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/${mc_compaign}.VH.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/${mc_compaign}.Vjets.${ptag}.txt"
    "${folder_name_PHYSLITE}/variations/${mc_compaign}.ttbar.${ptag}.txt"
    "${folder_name_PHYSLITE}/variations/${mc_compaign}.ttH.${ptag}.txt"
    "${folder_name_PHYSLITE}/variations/${mc_compaign}.tW.${ptag}.txt"
)

# check data campaign
if [ "$mc_campaign" == "mc20_13TeV" ]; then
    data="data_13TeV.Run2.${ptag}.txt"
elif [ "$mc_campaign" == "mc23_13p6TeV" ]; then
    data="data_13p6TeV.Run3.${ptag}.txt"
else
    echo "Unknown mc_campaign: $mc_campaign"
    data=""
fi

#data 
easyjet-gridsubmit --data-list ${folder_name_PHYSLITE}/nominal/${data} \
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

