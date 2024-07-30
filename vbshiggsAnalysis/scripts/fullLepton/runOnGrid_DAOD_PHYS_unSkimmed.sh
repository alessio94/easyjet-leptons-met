runConfig="vbshiggsAnalysis/RunConfig-fullLep-bypass.yaml"
executable="vbshiggs-ntupler"
campaignName="VBSHiggs_v4_unskim"

#signal
easyjet-gridsubmit --mc-list  ../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_signal_DAOD_PHYS_p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail \
    --HDBSProductionRole

#data
easyjet-gridsubmit --data-list ../easyjet/vbshiggsAnalysis/datasets/PHYS/data_Run2_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput

# W+QCD VVjj
easyjet-gridsubmit --mc-list  ../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_EWVVjj_DAOD_PHYS_p6262.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail \
    --HDBSProductionRole

#VH
easyjet-gridsubmit --mc-list ../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_VH_DAOD_PHYS_p6262.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail \
    --HDBSProductionRole

#Wjets
easyjet-gridsubmit --mc-list ../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_Wjets_DAOD_PHYS_p6262.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail \
    --HDBSProductionRole

#Zjets
easyjet-gridsubmit --mc-list ../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_Zjets_DAOD_PHYS_p6262.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail \
    --HDBSProductionRole

#single top
easyjet-gridsubmit --mc-list ../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_stop_DAOD_PHYS_p6262.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail \
    --HDBSProductionRole

#ttH
easyjet-gridsubmit --mc-list ../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_ttH_DAOD_PHYS_p6262.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail \
    --HDBSProductionRole

#ttV
easyjet-gridsubmit --mc-list ../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_ttV_DAOD_PHYS_p6262.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail \
    --HDBSProductionRole

#ttbar
easyjet-gridsubmit --mc-list ../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_ttbar_DAOD_PHYS_p6262.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail \
    --HDBSProductionRole


