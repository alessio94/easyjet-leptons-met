#data 
easyjet-gridsubmit --data-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/data_13p6TeV.Run3.p5858.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#bbH(yy) 
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.bbH_yy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#ggF H(yy) 
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ggFH_yy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#ggZH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ggZH_yy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#qqZH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.qqZH_yy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#ttH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.ttH_yy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=0, k2V=0, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl0kvv0kv1.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=0, k2V=1, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl0kvv1kv1.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=10, k2V=1, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl10kvv1kv1.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=1, k2V=0, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl1kvv0kv1.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=1, k2V=0.5, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl1kvv0p5kv1.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=1, k2V=1, kv=0.5
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl1kvv1kv0p5.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=1, k2V=1, kv=1.5
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl1kvv1kv1p5.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=1, k2V=1.5, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl1kvv1p5kv1.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=1, k2V=2, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl1kvv2kv1.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=1, k2V=3, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl1kvv3kv1.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=2, k2V=1, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_kl2kvv1kv1.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=1.5, k2V=1, kv=0.5
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_klm5kvv1kv0p5.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) SM
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFHH_bbyy_SM.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.VBFH_yy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#W-H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.WmH_yy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#W+H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc23_13p6TeV.WpH_yy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#yy+jets
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc21_13p6TeV.yyjets.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2
