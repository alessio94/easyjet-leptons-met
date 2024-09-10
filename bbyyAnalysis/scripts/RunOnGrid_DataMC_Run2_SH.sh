runConfig="bbyyAnalysis/RunConfig-Resonant-Default.yaml"
executable="bbyy-ntupler"
campaignName="SHbbyy_vXXX"
dir_PHYS="../easyjet/bbyyAnalysis/datasets/PHYS/nominal"
dir_PHYSLITE="../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal"
mc_campaign="mc20_13TeV"
ptag=p5855
mc_list=(
    "$dir_PHYS/$mc_campaign.XHS.p6026.txt"
    "$dir_PHYSLITE/$mc_campaign.ggFHH_bbyy_SM.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.VBFHH_bbyy_kl1kvv1kv1.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.ggFH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.VBFH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.WpH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.WmH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.qqZH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.ggZH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.ttH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.tHjb.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.tWHyy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.yyjets.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.ttyy_nonallhad.$ptag.txt"
)
#data 
easyjet-gridsubmit --data-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/data_13TeV.Run2.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}

#mc
easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

