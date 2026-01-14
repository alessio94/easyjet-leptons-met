runConfig="jjjjAnalysis/RunConfig-jjjj-common.yaml"
executable="jjjj-ntupler"

mc_list=(
    "../easyjet/jjjjAnalysis/datasets/mc20_13TeV_dijets_p6697.txt"
    "../easyjet/jjjjAnalysis/datasets/mc20_13TeV_MG_4jet_p6490.txt"
)

# mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable}  \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail \

# data
easyjet-gridsubmit --data-list ../easyjet/jjjjAnalysis/datasets/data_Run2_p6697.txt --run-config ${runConfig} \
    --exec ${executable} --campaign ${campaignName} --noTag --mergeOutput --noEmail 