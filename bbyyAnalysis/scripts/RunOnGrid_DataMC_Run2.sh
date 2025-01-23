ptag=p6266
campaign=v8
dir_samples="../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal"
mc_campaign="mc20_13TeV"
mc_list=(
    "$dir_samples/$mc_campaign.ggFHH_bbyy_SM.$ptag.txt"
    "$dir_samples/$mc_campaign.ggFHH_bbyy_SM_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.ggFHH_bbyy_kl0.$ptag.txt"
    "$dir_samples/$mc_campaign.ggFHH_bbyy_kl5.$ptag.txt"
    "$dir_samples/$mc_campaign.ggFHH_bbyy_klm1.$ptag.txt"
    "$dir_samples/$mc_campaign.ggFHH_bbyy_kl2p5.$ptag.txt"
    "$dir_samples/$mc_campaign.ggFHH_bbyy_kl10.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_SM.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_SM_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl0kvv1kv1.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl2kvv1kv1.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl10kvv1kv1.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl1kvv0kv1.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl1kvv0p5kv1.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl1kvv1p5kv1.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl1kvv2kv1.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl1kvv3kv1.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl1kvv1kv0p5.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl1kvv1kv1p5.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl0kvv0kv1.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_klm5kvv1kv0p5.$ptag.txt"
    "$dir_samples/$mc_campaign.ggFH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.WpH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.WmH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.qqZH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.ggZH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.ttH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.tHjb.$ptag.txt"
    "$dir_samples/$mc_campaign.tWHyy.$ptag.txt"
    "$dir_samples/$mc_campaign.bbH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.yyjets.$ptag.txt"
    "$dir_samples/$mc_compaign.yybb.$ptag.txt"
    "$dir_samples/$mc_campaign.ttyy_nonallhad.p6490.txt" #p6266 are not available
    "$dir_samples/$mc_campaign.ttyy_allhad.6490.txt" #p6266 are not available
)

#data 
easyjet-gridsubmit --data-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/data_13TeV.Run2.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose-syst.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 100 \
    --campaign ${campaign}

#mc
easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list[@]}") \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose-syst.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 10 \
    --campaign ${campaign}

