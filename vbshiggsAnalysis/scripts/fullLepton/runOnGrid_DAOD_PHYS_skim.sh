runConfig="vbshiggsAnalysis/RunConfig-fullLep.yaml"
executable="vbshiggs-ntupler"
campaignName="VBSHiggs_vXX_skim"

mc_list=(
    "../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_FullLep_signal_DAOD_PHYS_p6266.txt"
    "../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_EWVVjj_DAOD_PHYS_p6266.txt"
    "../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_VH_DAOD_PHYS_p6266.txt"
    "../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_Wjets_DAOD_PHYS_p6266.txt"
    "../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_Zjets_DAOD_PHYS_p6266.txt"
    "../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_stop_DAOD_PHYS_p6266.txt"
    "../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_ttH_DAOD_PHYS_p6266.txt"
    "../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_ttV_DAOD_PHYS_p6266.txt"
    "../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_ttbar_DAOD_PHYS_p6266.txt"
    "../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_VVV_DAOD_PHYS_p6266.txt"
)

#mc
easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable}  \
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
    --mergeOutput \
    --noEmail \
    --HDBSProductionRole