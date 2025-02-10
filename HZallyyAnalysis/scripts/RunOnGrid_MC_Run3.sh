#ptag=p6266
ptag=p5855
campaign=v1
dir_samples="../easyjet/llyyAnalysis/datasets/PHYS/nominal"
#mc_campaign="mc23_13p6TeV"
mc_campaign="mc23"
mc_list=(
     "$dir_samples/$mc_campaign.Zallyy_Signal_prompt_$ptag.txt"
)

easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config RunConfig-llyy.yaml \
    --exec llyy-ntupler \
    --nGBperJob 2 \ 
    --noTag \
    --campaign ${campaign}
