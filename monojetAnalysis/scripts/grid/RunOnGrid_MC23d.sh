runConfig="monojetAnalysis/RunConfig-Monojet.yaml"
executable="monojet-ntupler"
campaignName="mc23d_monojet_v0"

#Diboson
easyjet-gridsubmit --mc-list ../easyjet/monojetAnalysis/datasets/PHYS/MC23d/Diboson_mc23d_p6697.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag >> mc23d_prod.log


#SingleTop
easyjet-gridsubmit --mc-list ../easyjet/monojetAnalysis/datasets/PHYS/MC23d/SingleTop_mc23d_p6697.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
   --noTag >> mc23d_prod.log

#ttbar
easyjet-gridsubmit --mc-list ../easyjet/monojetAnalysis/datasets/PHYS/MC23d/TTbar_mc23d_p6697.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag >> mc23d_prod.log

#ttbarX
easyjet-gridsubmit --mc-list ../easyjet/monojetAnalysis/datasets/PHYS/MC23d/TTbarX_mc23d_p6697.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag >> mc23d_prod.log

#V+jets(lep)
easyjet-gridsubmit --mc-list ../easyjet/monojetAnalysis/datasets/PHYS/MC23d/Vjets_mc23d_p6697.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag >> mc23d_prod.log

#V+jets(quarks)
easyjet-gridsubmit --mc-list ../easyjet/monojetAnalysis/datasets/PHYS/MC23d/Vjets_qq_mc23d_p6697.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag >> mc23d_prod.log