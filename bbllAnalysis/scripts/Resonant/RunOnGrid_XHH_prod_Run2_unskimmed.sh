runConfig="bbllAnalysis/RunConfig-bbll-Resonant-bypass.yaml"
executable="bbll-ntupler"
campaignName="XHHbbll_v00"

#data 
easyjet-gridsubmit --data-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/data_Run2_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#XHH signal
easyjet-gridsubmit --mc-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/mc20_resonantXHH_samples_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#Z+jet
easyjet-gridsubmit --mc-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/mc20_Zjet_background_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#W+jet
easyjet-gridsubmit --mc-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/mc20_Wjet_background_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#ttbar+single-top
easyjet-gridsubmit --mc-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/mc20_top_background_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#Diboson
easyjet-gridsubmit --mc-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/mc20_diboson_background_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#Single Higgs
easyjet-gridsubmit --mc-list ../easyjet/bbllAnalysis/datasets/PHYSLITE/mc20_singleH_background_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag
