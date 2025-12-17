ptag_mc=p6697
ptag_data=p6700
campaign=v1p999bgt
dir_samples="../easyjet/bbyyAnalysis/datasets/PHYS/nominal/"
yaml="RunConfig-HH_H-skimming-loosest.yaml"
mc_campaign="mc23_13p6TeV"
mc_list_sample_names=(
    "$mc_campaign.HH_bbyy_SM.$ptag_mc.txt"
    "$mc_campaign.ggFHH_bbyy_BSM.$ptag_mc.txt"
    "$mc_campaign.VBFHH_bbyy_BSM.$ptag_mc.txt"
    "$mc_campaign.SingleHyy.$ptag_mc.txt"
    "$mc_campaign.yyBkg.$ptag_mc.txt"
)

easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list_sample_names[@]/#/${dir_samples}}") \
    --run-config bbyyAnalysis/${yaml} \
    --exec bbyy-ntupler \
    --nGBperJob 20 \
    --campaign ${campaign} \
    --noEmail

easyjet-gridsubmit --data-list ${dir_samples}/data_13p6TeV.Run3.${ptag_data}.txt \
    --run-config bbyyAnalysis/${yaml} \
    --exec bbyy-ntupler \
    --nGBperJob 50 \
    --campaign ${campaign}