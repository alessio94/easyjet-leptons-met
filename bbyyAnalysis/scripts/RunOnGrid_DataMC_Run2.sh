ptag=p6697
campaign=v1p999bgt
dir_samples="../easyjet/bbyyAnalysis/datasets/PHYS/nominal/"
mc_campaign="mc20_13TeV"
yaml="RunConfig-HHandHyy-skimming-loosest.yaml"
mc_list=(
    "$mc_campaign.HH_bbyy_SM.$ptag_mc.txt"
    "$mc_campaign.ggFHH_bbyy_BSM.$ptag_mc.txt"
    "$mc_campaign.VBFHH_bbyy_BSM.$ptag_mc.txt"
    "$mc_campaign.SingleHyy.$ptag_mc.txt"
    "$mc_campaign.yyBkg.$ptag_mc.txt"
)

# --nFiles 200 to avoid gigantic yyjets
easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list[@]/#/${dir_samples}}") \
    --run-config bbyyAnalysis/${yaml} \
    --exec bbyy-ntupler \
    --nGBperJob 20 \
    --campaign ${campaign} \
    --nFiles 200 \
    --noEmail

easyjet-gridsubmit --data-list ${dir_samples}/data_13TeV.Run2.${ptag}.txt \
    --run-config bbyyAnalysis/${yaml} \
    --exec bbyy-ntupler \
    --nGBperJob 50 \
    --campaign ${campaign}