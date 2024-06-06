runConfig="ssWWAnalysis/RunConfig-ssWW-VBS.yaml"
executable="ssWW-ntupler"
campaignName="ssWWVBS_v00"

#data 
easyjet-gridsubmit --data-list ../easyjet/ssWWAnalysis/datasets/PHYSLITE/data_Run3_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#Z+jet
easyjet-gridsubmit --mc-list ../easyjet/ssWWAnalysis/datasets/PHYSLITE/mc23_Zjet_background_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#W+jet
easyjet-gridsubmit --mc-list ../easyjet/ssWWAnalysis/datasets/PHYSLITE/mc23_Wjet_background_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#ttbar+single-top
easyjet-gridsubmit --mc-list ../easyjet/ssWWAnalysis/datasets/PHYSLITE/mc23_top_background_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag
