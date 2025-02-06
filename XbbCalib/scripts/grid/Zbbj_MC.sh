#!/bin/bash
runConfig="../easyjet/XbbCalib/share/RunConfig_Zbbj-MC.yaml"
executable="xbbcalib-ntupler"
campaignName="ZbbjCalib_250203"

dir_samples="../easyjet/XbbCalib/datasets/Zbbj"
mc_list=(
    "$dir_samples/MC/dijets.txt"
)

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag
