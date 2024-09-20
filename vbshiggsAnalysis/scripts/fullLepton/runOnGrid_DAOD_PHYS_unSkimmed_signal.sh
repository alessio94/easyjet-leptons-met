runConfig="vbshiggsAnalysis/RunConfig-fullLep-bypass.yaml"
executable="vbshiggs-ntupler"
campaignName="VBSHiggs_v4_unskimmed"

mc_list=(
    "../easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_signal_single_DAOD_PHYS_p6026.txt"
)

#mc_list=(
#    "/srv/easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_signal_DAOD_PHYS_p6026.txt"
#)

#mc
easyjet-gridsubmit --mc-list <(sed -e '$a\' "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable}  \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail 
