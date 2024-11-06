regime="2btag" #choose between 1btag and 2btag
runConfig="bbyyAnalysis/RunConfig-Resonant-Default-$regime.yaml"
executable="bbyy-ntupler"
campaignName="SHbbyy_vXXX"
dir_PHYS="../easyjet/bbyyAnalysis/datasets/PHYS/nominal"
dir_PHYSLITE="../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal"
mc_campaign="mc23_13p6TeV"
ptag="p6266"
mc_list=(
    "$dir_PHYSLITE/$mc_campaign.SHbbyy_$regime.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.ggFHH_bbyy_SM.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.VBFHH_bbyy_SM.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.ggFH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.VBFH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.WpH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.WmH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.qqZH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.ggZH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.ttH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.bbH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.tHjb.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.tWHyy.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.yyjets.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.yybb.$ptag.txt"
    "$dir_PHYSLITE/$mc_campaign.ttyy_nonallhad.$ptag.txt"
)
#data 
easyjet-gridsubmit --data-list ../easyjet/bbyyAnalysis/datasets/PHYS/nominal/data_13p6TeV.Run3.p6269.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 100 \
    --nCore 3 \
    --campaign ${campaignName}


#mc
easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 10 \
    --nCore 3 \
    --campaign ${campaignName}

