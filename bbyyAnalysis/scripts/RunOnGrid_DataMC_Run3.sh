ptag=p6266
campaign=v5
dir_samples="../easyjet/bbyyAnalysis/datasets/PHYS/nominal"
mc_compaign="mc23_13p6TeV"
mc_list=(
    "$dir_samples/$mc_compaign.ggFHH_bbyy_SM.$ptag.txt"
    "$dir_samples/$mc_compaign.ggFHH_bbyy_SM_FS.$ptag.txt"
    "$dir_samples/$mc_compaign.ggFHH_bbyy_kl0.$ptag.txt"
    "$dir_samples/$mc_compaign.ggFHH_bbyy_kl5.$ptag.txt"
    "$dir_samples/$mc_compaign.ggFHH_bbyy_klm1.$ptag.txt"
    "$dir_samples/$mc_compaign.ggFHH_bbyy_kl2p5.$ptag.txt"
    "$dir_samples/$mc_compaign.ggFHH_bbyy_kl10.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_SM.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl0kvv1kv1.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl2kvv1kv1.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl10kvv1kv1.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl1kvv0kv1.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl1kvv0p5kv1.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl1kvv1p5kv1.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl1kvv2kv1.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl1kvv3kv1.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl1kvv1kv0p5.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl1kvv1kv1p5.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl0kvv0kv1.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_klm5kvv1kv0p5.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_SM_FS.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl0kvv1kv1_FS.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl2kvv1kv1_FS.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl10kvv1kv1_FS.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl1kvv0kv1_FS.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl1kvv0p5kv1_FS.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl1kvv1p5kv1_FS.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl1kvv2kv1_FS.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl1kvv3kv1_FS.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl1kvv1kv0p5_FS.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl1kvv1kv1p5_FS.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_kl0kvv0kv1_FS.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFHH_bbyy_klm5kvv1kv0p5_FS.$ptag.txt"
    "$dir_samples/$mc_compaign.ggFH_yy.$ptag.txt"
    "$dir_samples/$mc_compaign.VBFH_yy.$ptag.txt"
    "$dir_samples/$mc_compaign.WpH_yy.$ptag.txt"
    "$dir_samples/$mc_compaign.WmH_yy.$ptag.txt"
    "$dir_samples/$mc_compaign.qqZH_yy.$ptag.txt"
    "$dir_samples/$mc_compaign.ggZH_yy.$ptag.txt"
    "$dir_samples/$mc_compaign.ttH_yy.$ptag.txt"
    "$dir_samples/$mc_compaign.bbH_yy.$ptag.txt"
    "$dir_samples/$mc_compaign.yyjets.$ptag.txt"
)
#data 
easyjet-gridsubmit --data-list ../easyjet/bbyyAnalysis/datasets/PHYS/nominal/data_13p6TeV.Run3.p6269.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign ${campaign}

