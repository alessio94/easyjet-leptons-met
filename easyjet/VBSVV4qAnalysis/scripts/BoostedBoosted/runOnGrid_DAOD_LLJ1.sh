runConfig="VBSVV4qAnalysis/RunConfig-JJ-discojet.yaml"
executable="VBSVV4q-ntupler"
campaignName="VBSVV4q_20250507"

mc_list=(
    "../easyjet/VBSVV4qAnalysis/datasets/LLJ1/mc20_dijets_p6453.txt"
    "../easyjet/VBSVV4qAnalysis/datasets/LLJ1/mc20_ewkvv_p6453.txt"
    "../easyjet/VBSVV4qAnalysis/datasets/LLJ1/mc20_ttbar_p6453.txt"
    "../easyjet/VBSVV4qAnalysis/datasets/LLJ1/mc20_vjets_p6453.txt"
#    "../easyjet/VBSVV4qAnalysis/datasets/PHYS/mc20_ewkvv_p6026.txt"
)

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable}  \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail \

#data
easyjet-gridsubmit --data-list ../easyjet/VBSVV4qAnalysis/datasets/LLJ1/data_Run2_p6453.txt --run-config ${runConfig} --exec ${executable} --campaign ${campaignName} --noTag --mergeOutput --noEmail 
