runConfig="monojetAnalysis/RunConfig-Monojet.yaml"
executable="monojet-ntupler"
campaignName="mc23a_monojet_v0"

#Diboson
easyjet-gridsubmit --mc-list ../easyjet/monojetAnalysis/datasets/PHYS/MC23a/Diboson_mc23a_p6697.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#SingleTop
easyjet-gridsubmit --mc-list ../easyjet/monojetAnalysis/datasets/PHYS/MC23a/SingleTop_mc23a_p6697.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#ttbar
easyjet-gridsubmit --mc-list ../easyjet/monojetAnalysis/datasets/PHYS/MC23a/TTbar_mc23a_p6697.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#ttbarX
easyjet-gridsubmit --mc-list ../easyjet/monojetAnalysis/datasets/PHYS/MC23a/TTbarX_mc23a_p6697.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#V+jets(lep)
easyjet-gridsubmit --mc-list ../easyjet/monojetAnalysis/datasets/PHYS/MC23a/Vjets_mc23a_p6697.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#V+jets(quarks)
easyjet-gridsubmit --mc-list ../easyjet/monojetAnalysis/datasets/PHYS/MC23a/Vjets_qq_mc23a_p6697.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag