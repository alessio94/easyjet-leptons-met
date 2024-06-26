runConfig="vbshiggsAnalysis/RunConfig-fullLep.yaml"
executable="vbshiggs-ntupler"
campaignName="VBSHiggs_21June2024_v02"

#signal
easyjet-gridsubmit --mc-list  ../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_signal_DAOD_PHYS_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput


#ttbar+single-top+ttH+tW
easyjet-gridsubmit --mc-list ../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_top_DAOD_PHYS_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#WZjj
easyjet-gridsubmit --mc-list ../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_WZjj_DAOD_PHYS_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput

#VH (bb, WW)
easyjet-gridsubmit --mc-list ../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_VH_DAOD_PHYS_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput

#data
easyjet-gridsubmit --data-list ../easyjet/vbshiggsAnalysis/datasets/PHYS/data_Run2_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput

