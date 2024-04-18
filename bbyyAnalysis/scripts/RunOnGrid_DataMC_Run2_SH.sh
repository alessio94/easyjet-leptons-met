runConfig="bbyyAnalysis/RunConfig-Resonant-Default.yaml"
executable="bbyy-ntupler"
campaignName="SHbbyy_vXXX"

#data 
easyjet-gridsubmit --data-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/data_13TeV.Run2.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

#SH signal
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYS/nominal/mc20_13TeV.XHS.p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

#ggF HH(bbyy) SM
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.ggFHH_bbyy_SM.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

#VBF HH(bbyy) SM
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.VBFHH_bbyy_kl1kvv1kv1.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

#ggF H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.ggFH_yy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

#VBF H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.VBFH_yy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

#W+H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.WpH_yy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

#W-H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.WmH_yy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

#qqZH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.qqZH_yy.p5855.txt\
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

#ggZH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.ggZH_yy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

#ttH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.ttH_yy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

#yy+jets
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.yyjets.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

#tHjb
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.tHjb.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

#tWHyy
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.tWHyy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}

#ttyy non all had 
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.ttyy_nonallhad.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --nGBperJob 2 \
    --campaign ${campaignName}