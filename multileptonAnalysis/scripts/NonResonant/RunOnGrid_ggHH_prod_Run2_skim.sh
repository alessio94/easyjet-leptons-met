runConfig="multileptonAnalysis/RunConfig-multilepton.yaml"
executable="hhml-ntupler"
campaignName="HHML_v01"

dir_samples="../easyjet/multileptonAnalysis/datasets/PHYS/prod_v1"
mc_list=(
    "$dir_samples/mc20_13TeV.HHML.txt"
    "$dir_samples/mc20_13TeV.Diboson.txt"
    "$dir_samples/mc20_13TeV.Triboson.txt"
    "$dir_samples/mc20_13TeV.SingleTop.txt"
    "$dir_samples/mc20_13TeV.VBF_Higgs.txt"
    "$dir_samples/mc20_13TeV.VH.txt"
    "$dir_samples/mc20_13TeV.Vgamma.txt"
    "$dir_samples/mc20_13TeV.Vjets.txt"
    "$dir_samples/mc20_13TeV.ggF_Higgs.txt"
    "$dir_samples/mc20_13TeV.tWZ.txt"
    "$dir_samples/mc20_13TeV.tZ.txt"
    "$dir_samples/mc20_13TeV.ttH.txt"
    "$dir_samples/mc20_13TeV.ttV.txt"
    "$dir_samples/mc20_13TeV.ttVV.txt"
    "$dir_samples/mc20_13TeV.ttbar.txt"
    "$dir_samples/mc20_13TeV.ttt.txt"
    "$dir_samples/mc20_13TeV.tttt.txt"
)

#data 
easyjet-gridsubmit --data-list $dir_samples/data_Run2.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --HDBSProductionRole

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --noTag \
    --HDBSProductionRole
