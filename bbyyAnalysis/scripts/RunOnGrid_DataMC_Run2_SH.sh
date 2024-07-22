runConfig="bbyyAnalysis/RunConfig-Resonant-Default.yaml"
executable="bbyy-ntupler"
campaignName="SHbbyy_vXXX"
dir_PHYS="../easyjet/bbyyAnalysis/datasets/PHYS/nominal"
dir_PHYSLITE="../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal"
mc_compaign="mc20_13TeV"
ptag=p5855
mc_list=(
    "$dir_PHYS/$mc_compaign.XHS.p6026.txt"
    "$dir_PHYSLITE/$mc_compaign.ggFHH_bbyy_SM.$ptag.txt"
    "$dir_PHYSLITE/$mc_compaign.VBFHH_bbyy_kl1kvv1kv1.$ptag.txt"
    "$dir_PHYSLITE/$mc_compaign.ggFH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_compaign.VBFH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_compaign.WpH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_compaign.WmH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_compaign.qqZH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_compaign.ggZH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_compaign.ttH_yy.$ptag.txt"
    "$dir_PHYSLITE/$mc_compaign.tHjb.$ptag.txt"
    "$dir_PHYSLITE/$mc_compaign.tWHyy.$ptag.txt"
    "$dir_PHYSLITE/$mc_compaign.yyjets.$ptag.txt"
    "$dir_PHYSLITE/$mc_compaign.ttyy_nonallhad.$ptag.txt"
)
#data 
easyjet-gridsubmit --data-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/data_13TeV.Run2.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

