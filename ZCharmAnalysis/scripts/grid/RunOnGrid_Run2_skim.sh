runConfig="ZCharmAnalysis/RunConfig-ZCharm.yaml"
executable="ZCharm-ntupler"
campaignName="ZCharm_v00"

#data 
easyjet-gridsubmit --data-list ../easyjet/ZCharmAnalysis/datasets/PHYS/data_Run2_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#Z+jet
easyjet-gridsubmit --mc-list ../easyjet/ZCharmAnalysis/datasets/PHYS/Zjets_Run2_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#Diboson
easyjet-gridsubmit --mc-list ../easyjet/ZCharmAnalysis/datasets/PHYS/Diboson_Run2_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#ZH
easyjet-gridsubmit --mc-list ../easyjet/ZCharmAnalysis/datasets/PHYS/ZH_Run2_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#ttbar+single-top
easyjet-gridsubmit --mc-list ../easyjet/ZCharmAnalysis/datasets/PHYS/Top_Run2_p6266.txtt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag
