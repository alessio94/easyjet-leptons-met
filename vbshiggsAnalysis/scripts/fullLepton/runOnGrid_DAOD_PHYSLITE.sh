runConfig="vbshiggsAnalysis/RunConfig-fullLep.yaml"
executable="vbshiggs-ntupler"
campaignName="VBSHiggs_12July2024_v03"

#signal
easyjet-gridsubmit --mc-list  ../easyjet/vbshiggsAnalysis/datasets/PHYSLITE/mc20_signal_DAOD_PHYSLITE_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput


#mc20a bkg
easyjet-gridsubmit --mc-list ../easyjet/vbshiggsAnalysis/datasets/PHYSLITE/mc20a_DAOD_PHYSLITE_p6266_bkg.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag

easyjet-gridsubmit --mc-list ../easyjet/vbshiggsAnalysis/datasets/PHYSLITE/mc20d_DAOD_PHYSLITE_p6266_bkg.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag

easyjet-gridsubmit --mc-list ../easyjet/vbshiggsAnalysis/datasets/PHYSLITE/mc20e_DAOD_PHYSLITE_p6266_bkg.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag

#data
easyjet-gridsubmit --data-list ../easyjet/vbshiggsAnalysis/datasets/PHYSLITE/data_Run2_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput

