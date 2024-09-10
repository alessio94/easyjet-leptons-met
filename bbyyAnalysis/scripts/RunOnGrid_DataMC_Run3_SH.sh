runConfig="bbyyAnalysis/RunConfig-Resonant-Default.yaml"
executable="bbyy-ntupler"
campaignName="SHbbyy_vXXX"
dir_PHYS="../easyjet/bbyyAnalysis/datasets/PHYS/nominal"
dir_PHYSLITE="../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal"
mc_campaign="mc23_13p6TeV"
ptag=p5855
mc_list=(
    "$dir_PHYS/$mc_campaign.XHS.p6026.txt"
    "$dir_PHYSLITE/$mc_campaign.VBFHH_bbyy_SM.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.ggFH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.ggZH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.qqZH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.ttH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.VBFH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.WmH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.WpH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.bbH_yy.$ptag.txt"
    "$dir_PHYSLITE/mc21_13p6TeV.yyjets.$ptag.txt"
)
#data 
easyjet-gridsubmit --data-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/data_13p6TeV.Run3.p5858.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}


#mc
easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

