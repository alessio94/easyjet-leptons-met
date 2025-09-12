runConfig="VBSVV4qAnalysis/RunConfig-JJ.yaml"
executable="VBSVV4q-ntupler"
campaignName="VBSVV4q_$1"

mc_list=(
    "../easyjet/VBSVV4qAnalysis/datasets/LLJ1/mc20_dijets_p6453.txt"
    "../easyjet/VBSVV4qAnalysis/datasets/LLJ1/mc20_ewkvv_p6453.txt"
    "../easyjet/VBSVV4qAnalysis/datasets/LLJ1/mc20_ttbar_p6453.txt"
    "../easyjet/VBSVV4qAnalysis/datasets/LLJ1/mc20_vjets_p6453.txt"
)

easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable}  \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail \

