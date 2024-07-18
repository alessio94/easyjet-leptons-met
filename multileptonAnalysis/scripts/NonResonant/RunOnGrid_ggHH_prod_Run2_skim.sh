runConfig="bbllAnalysis/RunConfig-bbll.yaml"
executable="bbll-ntupler"
campaignName="XHHbbll_v00"

#data 
easyjet-gridsubmit --data-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/data_Run2_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag

#XHH signal
easyjet-gridsubmit --mc-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/mc20_ggFXHH_samples_p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag

#Z+jet
easyjet-gridsubmit --mc-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/mc20_Zjet_background_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag

#W+jet
easyjet-gridsubmit --mc-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/mc20_Wjet_background_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag

#ttbar+single-top
easyjet-gridsubmit --mc-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/mc20_top_background_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag
