#!/bin/bash
runConfig="/usatlas/u/ivelisce/Xbb/easyjet/XbbCalib/share/RunConfig-ZbbyCalib.yaml"
executable="xbbcalib-ntupler"
campaignName="ZbbyCalib_250217"

dir_samples="/usatlas/u/ivelisce/Xbb/easyjet/XbbCalib/datasets/Zbby"
mc_list=("$dir_samples/MC/yjets.txt" "$dir_samples/MC/Wqqgamma_pTW140.txt" "$dir_samples/MC/Zbbgamma_pTZ100.txt" "$dir_samples/MC/Zqqgamma_pTZ100.txt")
#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

