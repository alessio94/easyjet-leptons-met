ptag=p6266
campaign=v4

#data 
#easyjet-gridsubmit --data-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/data_13p6TeV.Run3.${ptag}.txt \
#    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
#    --exec bbyy-ntupler \
##    --campaign ${campaign}

#ggF HH(bbyy) SM
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ggFHH_bbyy_SM.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#ggF HH(bbyy) SM FS
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ggFHH_bbyy_SM_FS.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#ggF HH(bbyy) kl=0
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ggFHH_bbyy_kl0.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#ggF HH(bbyy) kl=5
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ggFHH_bbyy_kl5.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#ggF HH(bbyy) kl=-1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ggFHH_bbyy_klm1.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#ggF HH(bbyy) kl=2.5
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ggFHH_bbyy_kl2p5.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#ggF HH(bbyy) kl=10
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ggFHH_bbyy_kl10.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#VBF HH(bbyy) SM
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_SM.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#VBF HH(bbyy) kl=0, k2V=1, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl0kvv1kv1.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#VBF HH(bbyy) kl=2, k2V=1, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl2kvv1kv1.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#VBF HH(bbyy) kl=10, k2V=1, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl10kvv1kv1.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#VBF HH(bbyy) kl=1, k2V=0, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl1kvv0kv1.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#VBF HH(bbyy) kl=1, k2V=0.5, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl1kvv0p5kv1.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#VBF HH(bbyy) kl=1, k2V=1.5, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl1kvv1p5kv1.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#VBF HH(bbyy) kl=1, k2V=2, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl1kvv2kv1.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#VBF HH(bbyy) kl=1, k2V=3, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl1kvv3kv1.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#VBF HH(bbyy) kl=1, k2V=1, kv=0.5
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl1kvv1kv0p5.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#VBF HH(bbyy) kl=1, k2V=1, kv=1.5
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl1kvv1kv1p5.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#VBF HH(bbyy) kl=0, k2V=0, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl0kvv0kv1.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#VBF HH(bbyy) kl=-5, k2V=1, kv=0.5
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_klm5kvv1kv0p5.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#ggF H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ggFH_yy.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#VBF H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFH_yy.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#W+H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.WpH_yy.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#W-H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.WmH_yy.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#qqZH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.qqZH_yy.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#ggZH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ggZH_yy.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#ttH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ttH_yy.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#bbH(yy) 
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.bbH_yy.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#yy+jets MadGraph
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.yyjets_MadGraph.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#yy+jets Sherpa
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.yyjets.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}

#yy+jets Sherpa FS
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.yyjets_FS.${ptag}.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --campaign ${campaign}
