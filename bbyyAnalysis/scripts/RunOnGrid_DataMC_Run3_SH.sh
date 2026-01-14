runConfig="bbyyAnalysis/RunConfig-Resonant-Default-oneAndtwobtag.yaml"
executable="bbyy-ntupler"
campaignName="EBReview_v2_Run3"
dir_PHYS="../easyjet/bbyyAnalysis/datasets/PHYS/nominal"
mc_campaign="mc23_13p6TeV"
ptag="p6266"
mc_list=(
    "$dir_PHYS/$mc_campaign.SHbbyy_1btag.$ptag.txt"
    "$dir_PHYS/$mc_campaign.SHbbyy_2btag.$ptag.txt"
    "$dir_PHYS/$mc_campaign.ggFHH_bbyy_SM.$ptag.txt"
    "$dir_PHYS/$mc_campaign.VBFHH_bbyy_SM.$ptag.txt"
    "$dir_PHYS/$mc_campaign.ggFH_yy.$ptag.txt"
    "$dir_PHYS/$mc_campaign.VBFH_yy.$ptag.txt"
    "$dir_PHYS/$mc_campaign.WpH_yy.$ptag.txt"
    "$dir_PHYS/$mc_campaign.WmH_yy.$ptag.txt"
    "$dir_PHYS/$mc_campaign.qqZH_yy.$ptag.txt"
    "$dir_PHYS/$mc_campaign.ggZH_yy.$ptag.txt"
    "$dir_PHYS/$mc_campaign.ttH_yy.$ptag.txt"
    "$dir_PHYS/$mc_campaign.bbH_yy.$ptag.txt"
    "$dir_PHYS/$mc_campaign.tHjb.$ptag.txt"
    "$dir_PHYS/$mc_campaign.tWHyy.$ptag.txt"
    "$dir_PHYS/$mc_campaign.ttyy_nonallhad.p6491.txt" #p6266 are not available
    "$dir_PHYS/$mc_campaign.ttyy_allhad.p6491.txt" #p6266 are not available
    "$dir_PHYS/$mc_campaign.yyjets.$ptag.txt"
    "$dir_PHYS/$mc_campaign.yybb.$ptag.txt"
)

#data 
easyjet-gridsubmit --data-list ../easyjet/bbyyAnalysis/datasets/PHYS/nominal/data_13p6TeV.Run3.p6269.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 50 \
    --campaign ${campaignName}

#mc
easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 10 \
    --campaign ${campaignName}

