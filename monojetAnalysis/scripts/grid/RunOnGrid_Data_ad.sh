runConfig="monojetAnalysis/RunConfig-Monojet.yaml"
executable="monojet-ntupler"
campaignName="dataA_monojet_v0"

#data 23a
easyjet-gridsubmit --data-list ../easyjet/monojetAnalysis/datasets/PHYS/MC23a/Data_Run3_p6697.txt \
     --run-config ${runConfig} \
     --exec ${executable} \
     --campaign ${campaignName} \
     --noTag
#data23d
easyjet-gridsubmit --data-list ../easyjet/monojetAnalysis/datasets/PHYS/MC23d/Data_Run3_p6697.txt \
     --run-config ${runConfig} \
     --exec ${executable} \
     --campaign ${campaignName} \
     --noTag
