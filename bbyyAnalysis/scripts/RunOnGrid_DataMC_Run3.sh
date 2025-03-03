ptag=p6491
ptag_mc23e=p6522
campaign=v9
dir_samples="../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal"
dir_mc_samples_mc23e="../easyjet/bbyyAnalysis/datasets/PHYS/nominal"
mc_campaign="mc23_13p6TeV"
mc_list=(
    "$dir_samples/$mc_campaign.ggFHH_bbyy_SM.$ptag.txt"
    "$dir_samples/$mc_campaign.ggFHH_bbyy_SM_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.ggFHH_bbyy_kl0.$ptag.txt"
    "$dir_samples/$mc_campaign.ggFHH_bbyy_kl5.$ptag.txt"
    "$dir_samples/$mc_campaign.ggFHH_bbyy_klm1.$ptag.txt"
    "$dir_samples/$mc_campaign.ggFHH_bbyy_kl2p5.$ptag.txt"
    "$dir_samples/$mc_campaign.ggFHH_bbyy_kl10.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_SM.$ptag.txt"
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
    "$dir_samples/$mc_campaign.VBFHH_bbyy_SM_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl0kvv1kv1_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl2kvv1kv1_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl10kvv1kv1_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl1kvv0kv1_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl1kvv0p5kv1_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl1kvv1p5kv1_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl1kvv2kv1_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl1kvv3kv1_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl1kvv1kv0p5_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl1kvv1kv1p5_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_kl0kvv0kv1_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFHH_bbyy_klm5kvv1kv0p5_FS.$ptag.txt"
    "$dir_samples/$mc_campaign.ggFH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.WpH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.WmH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.qqZH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.ggZH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.ttH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.bbH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.tHjb.$ptag.txt"
    "$dir_samples/$mc_campaign.tWHyy.$ptag.txt"
    "$dir_samples/$mc_campaign.yyjets.$ptag.txt"
    "$dir_samples/$mc_campaign.yybb.$ptag.txt"
    "$dir_samples/$mc_campaign.ttyy_nonallhad.$ptag.txt" 
    "$dir_samples/$mc_campaign.ttyy_allhad.$ptag.txt"
)
mc_list_mc23e=(
    "$dir_mc_samples_mc23e/$mc_campaign.ggFHH_bbyy_SM.$ptag_mc23e.txt"
    "$dir_mc_samples_mc23e/$mc_campaign.ggFHH_bbyy_BSM.$ptag_mc23e.txt"
    "$dir_mc_samples_mc23e/$mc_campaign.VBFHH_bbyy_SM.$ptag_mc23e.txt"
    "$dir_mc_samples_mc23e/$mc_campaign.VBFHH_bbyy_BSM.$ptag_mc23e.txt"
    "$dir_mc_samples_mc23e/$mc_campaign.VBFHH_bbyy_FS.$ptag_mc23e.txt"
    "$dir_mc_samples_mc23e/$mc_campaign.SingleHyy.$ptag_mc23e.txt"
    "$dir_mc_samples_mc23e/$mc_campaign.yyjets.$ptag_mc23e.txt"
    "$dir_mc_samples_mc23e/$mc_campaign.yybb.$ptag_mc23e.txt"
    "$dir_mc_samples_mc23e/$mc_campaign.ttyy.$ptag_mc23e.txt"
)

#data 
easyjet-gridsubmit --data-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/data_13p6TeV.Run3.p6482.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-legacy.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 50 \
    --campaign ${campaign}

#mc
easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list[@]}") \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-legacy.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 20 \
    --campaign ${campaign}

#data24
easyjet-gridsubmit --data-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/data_23p6TeV_2024.Run3.p6515.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-legacy.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 50 \
    --campaign ${campaign}

#mc23e
easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list_mc23e[@]}") \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-legacy.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 20 \
    --campaign ${campaign}
