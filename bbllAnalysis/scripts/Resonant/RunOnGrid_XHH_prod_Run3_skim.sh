runConfig="bbllAnalysis/RunConfig-bbll-Resonant.yaml"
executable="bbll-ntupler"
campaignName="XHHbbll_v00"

#data 
easyjet-gridsubmit --data-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/data_Run3_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#XHH signal
easyjet-gridsubmit --mc-list ../easyjet/bbllAnalysis/datasets/PHYS/mc23_resonantXHH_samples_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#Z+jet
easyjet-gridsubmit --mc-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/mc23_Zjet_background_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#W+jet
easyjet-gridsubmit --mc-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/mc23_Wjet_background_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#ttbar+single-top
easyjet-gridsubmit --mc-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/mc23_top_background_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag
