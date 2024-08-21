runConfig="vbshiggsAnalysis/RunConfig-semiLep-bypass.yaml"
executable="vbshiggs-ntupler"
campaignName="VBSHiggs_v4_unskimmed"

mc_list=(
    "../../datasets/PHYS/mc20_semilep_signal_single_DAOD_PHYS_p6266.txt"
)

#mc_list=(
#    "/afs/cern.ch/user/j/jaburles/public/VBSHiggs/easyjet/easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_semilep_WpWmhjj_signal_DAOD_PHYS_p6266.txt",
#    "/afs/cern.ch/user/j/jaburles/public/VBSHiggs/easyjet/easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_semilep_WpWphjj_signal_DAOD_PHYS_p6266.txt",
#    "/afs/cern.ch/user/j/jaburles/public/VBSHiggs/easyjet/easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_semilep_WmWmhjj_signal_DAOD_PHYS_p6266.txt",
#    "/afs/cern.ch/user/j/jaburles/public/VBSHiggs/easyjet/easyjet/vbshiggsAnalysis/datasets/PHYS/mc20_semilep_WmWphjj_signal_DAOD_PHYS_p6266.txt"
#)


#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable}  \
    --campaign ${campaignName} \
    --noTag \
    --mergeOutput \
    --noEmail 
