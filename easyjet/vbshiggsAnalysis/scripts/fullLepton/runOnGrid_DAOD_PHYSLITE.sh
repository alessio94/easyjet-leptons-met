runConfig="vbshiggsAnalysis/RunConfig-fullLep.yaml"
executable="vbshiggs-ntupler"
campaignName="VBSHiggs_vXXX"

mc_list=(
    "../easyjet/vbshiggsAnalysis/datasets/PHYSLITE/mc20_signal_DAOD_PHYSLITE_p6026.txt"
    "../easyjet/vbshiggsAnalysis/datasets/PHYSLITE/mc20a_DAOD_PHYSLITE_p6266_bkg.txt"
    "../easyjet/vbshiggsAnalysis/datasets/PHYSLITE/mc20d_DAOD_PHYSLITE_p6266_bkg.txt"
    "../easyjet/vbshiggsAnalysis/datasets/PHYSLITE/mc20e_DAOD_PHYSLITE_p6266_bkg.txt"
)
#mc
easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable}  \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail \
    --ProductionRole HMBS

#data
easyjet-gridsubmit --data-list ../easyjet/vbshiggsAnalysis/datasets/PHYSLITE/data_Run2_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail \
    --ProductionRole HMBS

