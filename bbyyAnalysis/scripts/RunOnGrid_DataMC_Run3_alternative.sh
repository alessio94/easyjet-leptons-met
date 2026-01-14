ptag=p6491
campaign=v9
dir_samples="../easyjet/bbyyAnalysis/datasets/PHYSLITE/alternative"
mc_campaign="mc23_13p6TeV"
mc_list=(
    "$dir_samples/$mc_campaign.ggFH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.VBFH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.WpH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.WmH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.qqZH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.ggZH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.ttH_yy.$ptag.txt"
    "$dir_samples/$mc_campaign.bbH_yy.$ptag.txt"    
)

#mc
easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list[@]}") \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 10 \
    --campaign ${campaign}


