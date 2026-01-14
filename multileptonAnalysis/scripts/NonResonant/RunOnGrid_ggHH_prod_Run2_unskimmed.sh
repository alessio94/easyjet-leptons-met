runConfig="multileptonAnalysis/RunConfig-multilepton-bypass.yaml"
executable="hhml-ntupler"
campaignName="HHML_test_v00"

dir_samples="../easyjet/multileptonAnalysis/datasets/PHYS/prod_v1"
mc_list=(
    "$dir_samples/mc20_13TeV.HHML.txt"
)

#data 
#easyjet-gridsubmit --data-list $dir_samples/data_Run2.txt \
#    --run-config ${runConfig} \
#    --exec ${executable} \
#    --campaign ${campaignName} \
#    --noTag

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag
