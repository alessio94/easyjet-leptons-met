executable="hhml-ntupler"

dir_samples="../easyjet/multileptonAnalysis/datasets/PHYS/prod_v3"
mc_list=(
    "$dir_samples/mc20_13TeV.hhml_ana.txt"
    "$dir_samples/mc20_13TeV.hhml.txt"
    "$dir_samples/mc20_13TeV.singleH.txt"
    "$dir_samples/mc20_13TeV.top.txt"
    "$dir_samples/mc20_13TeV.Vjets.txt"
    "$dir_samples/mc20_13TeV.VV.txt"
    "$dir_samples/mc20_13TeV.VVV.txt"
    "$dir_samples/mc20_13TeV.Vy.txt"
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
        --ProductionRole HIGP

    #mc pure_lep
    for mc_file in "${mc_list[@]}"; do
        cat "$mc_file"
        echo # This adds a newline after each file's content
    done | easyjet-gridsubmit --mc-list /dev/stdin \
        --run-config ${runConfig} \
        --exec ${executable} \
        --campaign ${campaignName} \
        --noTag \
        --ProductionRole HIGP
done
