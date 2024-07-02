runConfig="ZCharmAnalysis/RunConfig-ZCharm.yaml"
executable="ZCharm-ntupler"
campaignName="ZCharm_v00"

# #data 
# easyjet-gridsubmit --data-list ../easyjet/ZCharmAnalysis/datasets/PHYS/MC20e/Data_Run2_p6266.txt \
#     --run-config ${runConfig} \
#     --exec ${executable} \
#     --nGBperJob 5 \
#     --campaign ${campaignName} \
#     --noTag

#Z+jet (Sherpa)
easyjet-gridsubmit --mc-list ../easyjet/ZCharmAnalysis/datasets/PHYS/MC20e/Zjets_Sh_Run2_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag


# #Z+jet (MG)
# easyjet-gridsubmit --mc-list ../easyjet/ZCharmAnalysis/datasets/PHYS/MC20e/Zjets_MG_Run2_p6266.txt \
#     --run-config ${runConfig} \
#     --exec ${executable} \
#     --nGBperJob 5 \
#     --campaign ${campaignName} \
#     --noTag

#Diboson
easyjet-gridsubmit --mc-list ../easyjet/ZCharmAnalysis/datasets/PHYS/MC20e/Diboson_Run2_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#ZH
easyjet-gridsubmit --mc-list ../easyjet/ZCharmAnalysis/datasets/PHYS/MC20e/ZH_Run2_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

#ttbar
easyjet-gridsubmit --mc-list ../easyjet/ZCharmAnalysis/datasets/PHYS/MC20e/Ttbar_Run2_p6266.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 5 \
    --campaign ${campaignName} \
    --noTag

# #single-top
# easyjet-gridsubmit --mc-list ../easyjet/ZCharmAnalysis/datasets/PHYS/MC20e/Stop_Run2_p6266.txtt \
#     --run-config ${runConfig} \
#     --exec ${executable} \
#     --nGBperJob 5 \
#     --campaign ${campaignName} \
#     --noTag