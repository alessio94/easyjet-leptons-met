runConfig="ZCharmAnalysis/RunConfig-ZCharm.yaml"
executable="ZCharm-ntupler"
campaignName="ZCharm_v00"

dir_samples="../easyjet/ZCharmAnalysis/datasets/PHYS/MC20e"
mc_list=(
    "$dir_samples/Zjets_Sh_Run2_p6266.txt"
    "$dir_samples/Diboson_Run2_p6266.txt"
    "$dir_samples/ZH_Run2_p6266.txt"
    "$dir_samples/Ttbar_Run2_p6266.txt"
)
#   "$dir_samples/Zjets_MG_Run2_p6266.txt"
#   "$dir_samples/Stop_Run2_p6266.txt"

# #data 
# easyjet-gridsubmit --data-list ../easyjet/ZCharmAnalysis/datasets/PHYS/MC20e/Data_Run2_p6266.txt \
#     --run-config ${runConfig} \
#     --exec ${executable} \
#     --nGBperJob 5 \
#     --campaign ${campaignName} \
#     --noTag

# mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag
