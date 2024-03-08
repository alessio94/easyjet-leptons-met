#data 
easyjet-gridsubmit --data-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/data_13TeV.Run2.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#ggF HH(bbyy) SM
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.ggFHH_bbyy_SM.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#ggF HH(bbyy) kl=10
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.ggFHH_bbyy_kl10.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) SM
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.VBFHH_bbyy_kl1kvv1kv1.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=10, k2V=1, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.VBFHH_bbyy_kl10kvv1kv1.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=1, k2V=1, kv=0.5
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.VBFHH_bbyy_kl1kvv1kv0p5.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2    

#VBF HH(bbyy) kl=1, k2V=1.5, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.VBFHH_bbyy_kl1kvv1p5kv1.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF HH(bbyy) kl=2, k2V=1, kv=1
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.VBFHH_bbyy_kl2kvv1kv1.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2 

#VBF HH(bbyy) kl=-5, k2V=1, kv=0.5
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.VBFHH_bbyy_klm5kvv1kv0p5.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#ggF H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.ggFH_yy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#VBF H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.VBFH_yy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#W+H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.WpH_yy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#W-H(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.WmH_yy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#qqZH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.qqZH_yy.p5855.txt\
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#ggZH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.ggZH_yy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#ttH(yy)
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.ttH_yy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#yy+jets
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.yyjets.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#tHjb
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.tHjb.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#tWHyy
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.tWHyy.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2

#ttyy non all had 
easyjet-gridsubmit --mc-list ../easyjet/bbyyAnalysis/datasets/PHYSLITE/nominal/mc20_13TeV.ttyy_nonallhad.p5855.txt \
    --run-config bbyyAnalysis/RunConfig-bbyy-skimming-loose.yaml \
    --exec bbyy-ntupler \
    --nGBperJob 2 \
    --campaign v2