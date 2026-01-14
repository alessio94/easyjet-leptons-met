ptag=p6697
campaign=v1p999bgt
dir_samples="../easyjet/bbyyAnalysis/datasets/PHYS/nominal/"
mc_campaign="mc20_13TeV"
yaml="RunConfig-HH_H-skimming-loosest.yaml"
mc_list=(
    "$mc_campaign.HH_bbyy_SM.$ptag.txt"
    "$mc_campaign.ggFHH_bbyy_BSM.$ptag.txt"
    "$mc_campaign.VBFHH_bbyy_BSM.$ptag.txt"
    "$mc_campaign.SingleHyy.$ptag.txt"
    "$mc_campaign.yyBkg.$ptag.txt"
)

easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list[@]/#/${dir_samples}}") \
    --run-config bbyyAnalysis/${yaml} \
    --exec bbyy-ntupler \
    --nGBperJob 20 \
    --campaign ${campaign} \
    --noEmail

easyjet-gridsubmit --data-list ${dir_samples}/data_13TeV.Run2.${ptag}.txt \
    --run-config bbyyAnalysis/${yaml} \
    --exec bbyy-ntupler \
    --nGBperJob 50 \
    --campaign ${campaign}