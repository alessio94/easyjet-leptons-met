runConfig="bbyyAnalysis/RunConfig-Resonant-Default.yaml"
executable="bbyy-ntupler"
campaignName="SHbbyy_vXXX"

#data 
easyjet-gridsubmit --data-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/data_13p6TeV.Run3.p5858.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}

#SH signal
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYS/nominal/mc23_13p6TeV.XHS.p6026.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}

#bbH(yy) 
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.bbH_yy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}

#ggF H(yy) 
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ggFH_yy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}

#ggZH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ggZH_yy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}

#qqZH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.qqZH_yy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}

#ttH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ttH_yy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}

#VBF HH(bbyy) SM
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_SM.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}

#VBF H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFH_yy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}

#W-H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.WmH_yy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}

#W+H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.WpH_yy.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}

#yy+jets
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc21_13p6TeV.yyjets.p5855.txt \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName}
