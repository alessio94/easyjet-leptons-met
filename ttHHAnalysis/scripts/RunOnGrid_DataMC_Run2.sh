ptag="p5855"
runConfig="ttHHAnalysis/RunConfig-ttHH.yaml"
executable="ttHH-ntupler"
campaignName="001"

folder_name_PHYSLITE="../easyjet/ttHHAnalysis/dataset/PHYSLITE"
folder_name_PHYS="../easyjet/ttHHAnalysis/dataset/PHYS"

mc_list=(
    "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttHH_dilep.523073.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttHH_semilep.523072.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttHH_allhad.523074.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttHH_SSML.525963.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttH_dilep.346345.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttH_semilep.346344.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttH_allhad.346343.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttbar_allhad.410471.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttbar_nonallhad.410470.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttZ.all_nominal.${ptag}.txt"
    "${folder_name_PHYS}/nominal/mc20_13TeV.ttW.700706.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttWW.410081.${ptag}.txt"
    "${folder_name_PHYS}/nominal/mc20_13TeV.ttWZ.500463.${ptag}.txt"
    "${folder_name_PHYS}/nominal/mc20_13TeV.tttt.412043.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttt.516978.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/mc20_13TeV.Vjets.all_nominal.${ptag}.txt"
    "${folder_name_PHYSLITE}/nominal/mc20_13TeV.singleTop.all_nominal.${ptag}.txt"
)
# "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttbar_dilep.410472.${ptag}.txt"
# "${folder_name_PHYSLITE}/variations/mc20_13TeV.ttW.700168.${ptag}.txt"
# "${folder_name_PHYSLITE}/mc20_13TeV.tttt.${ptag}.txt"
# "${folder_name_PHYS}/variations/mc20_13TeV.tttt.412044.${ptag}.txt"
# "${folder_name_PHYS}/variations/mc20_13TeV.tttt.500326.${ptag}.txt"
# "${folder_name_PHYSLITE}/variations/mc20_13TeV.ttt.304014.${ptag}.txt"

#data 
easyjet-gridsubmit --data-list ${folder_name_PHYSLITE}/nominal/data_13TeV.Run2.${ptag}.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName} \
    --noSubmit

#mc
easyjet-gridsubmit --mc-list <(cat "${mc_list[@]}") \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName} \
    --noSubmit
