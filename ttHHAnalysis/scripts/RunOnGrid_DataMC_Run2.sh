submit_jobs() {
    local list_path=$1
    local list_type=${2:---mc-list} # Either --data-list or --mc-list (default)

    easyjet-gridsubmit $list_type $list_path \
        --run-config ../easyjet/ttHHAnalysis/share/RunConfig-ttHH.yaml \
        --exec ttHH-ntupler \
        --nGBperJob 2 \
        --campaign 001 \
        --noSubmit
}

folder_name_PHYSLITE="../easyjet/ttHHAnalysis/dataset/PHYSLITE"
folder_name_PHYS="../easyjet/ttHHAnalysis/dataset/PHYS"

# Data
submit_jobs "${folder_name_PHYSLITE}/nominal/data_13TeV.Run2.p5855.txt" "--data-list"

# ttHH
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttHH_dilep.523073.p5855.txt"
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttHH_semilep.523072.p5855.txt"
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttHH_allhad.523074.p5855.txt"
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttHH_SSML.525963.p5855.txt"

# ttH
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttH_dilep.346345.p5855.txt"
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttH_semilep.346344.p5855.txt"
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttH_allhad.346343.p5855.txt"

# ttbar hdamp258p75
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttbar_allhad.410471.p5855.txt"
#submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttbar_dilep.410472.p5855.txt"
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttbar_nonallhad.410470.p5855.txt"

# ttZ (ttZqq, ttZnunu, tttautau, ttmumu, ttee)
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttZ.all_nominal.p5855.txt"

# ttZZ
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttZZ.500462.p5855.txt"

# ttW 700168.Sh_2210
# missing PHYSLITE for 700706.Sh_2212
submit_jobs "${folder_name_PHYS}/nominal/mc20_13TeV.ttW.700706.p5855.txt"
#submit_jobs "${folder_name_PHYSLITE}/variations/mc20_13TeV.ttW.700168.p5855.txt"

# ttWW
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttWW.410081.p5855.txt"

# ttWH
# missing PHYSLITE so using PHYS for the moment
submit_jobs "${folder_name_PHYS}/nominal/mc20_13TeV.ttWH.500461.p5855.txt"

# ttWZ
# missing PHYSLITE so using PHYS for the moment
submit_jobs "${folder_name_PHYS}/nominal/mc20_13TeV.ttWZ.500463.p5855.txt"

# tttt
# missing PHYSLITE so using PHYS for the moment
# submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.tttt.p5855.txt"
submit_jobs "${folder_name_PHYS}/nominal/mc20_13TeV.tttt.412043.p5855.txt"
#submit_jobs "${folder_name_PHYS}/variations/mc20_13TeV.tttt.412044.p5855.txt"
#submit_jobs "${folder_name_PHYS}/variations/mc20_13TeV.tttt.500326.p5855.txt"

# ttt
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.ttt.516978.p5855.txt"
#submit_jobs "${folder_name_PHYSLITE}/variations/mc20_13TeV.ttt.304014.p5855.txt"

# V+jets
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.Vjets.all_nominal.p5855.txt"

# single top
submit_jobs "${folder_name_PHYSLITE}/nominal/mc20_13TeV.singleTop.all_nominal.p5855.txt"
