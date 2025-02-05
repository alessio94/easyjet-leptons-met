executable="hhml-ntupler"

dir_samples="../easyjet/multileptonAnalysis/datasets/PHYSLITE/prod_v2"
mc_list=(
    "$dir_samples/mc20_13TeV.ggF_HHML.txt"
    "$dir_samples/mc20_13TeV.VBF_HHML.txt"
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

campaignTag="v03"
configTags=(
    "2l"
    "3l"
    "bb4l"
    "tau"
    "VeryLooseTau"
)

for configTag in "${configTags[@]}"; do
    runConfig="multileptonAnalysis/RunConfig-multilepton-${configTag}.yaml"
    campaignName="HHML_${configTag}_${campaignTag}"
    echo "runConfig: $runConfig. campaignName: $campaignName"

    #data 
    easyjet-gridsubmit --data-list $dir_samples/data_Run2.txt \
        --run-config ${runConfig} \
        --exec ${executable} \
        --campaign ${campaignName} \
        --noTag \
        --HDBSProductionRole

    #mc pure_lep
    for mc_file in "${mc_list[@]}"; do
        cat "$mc_file"
        echo # This adds a newline after each file's content
    done | easyjet-gridsubmit --mc-list /dev/stdin \
        --run-config ${runConfig} \
        --exec ${executable} \
        --campaign ${campaignName} \
        --noTag \
        --HDBSProductionRole # --noSubmit
done
