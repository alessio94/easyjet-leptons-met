submit_jobs() {
    local list_path=$1
    local list_type=${2:---mc-list} # Either --data-list or --mc-list (default)

    easyjet-gridsubmit $list_type $list_path \
        --run-config ../easyjet/ttHHAnalysis/share/RunConfig-ttHH.yaml \
        --exec ttHH-ntupler \
        --nGBperJob 2 \
        --campaign 001
}

folder_name_PHYSLITE="../easyjet/ttHHAnalysis/datasets/PHYSLITE"
folder_name_PHYS="../easyjet/ttHHAnalysis/datasets/PHYS"

# Data
submit_jobs "${folder_name_PHYSLITE}/data_13TeV.Run2.p5855.txt" "--data-list"

# ttHH
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttHH_dilep.p5855.txt"
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttHH_semilep.p5855.txt"
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttHH_allhad.p5855.txt"
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttHH_SSML.p5855.txt"

# ttH
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttH_dilep.p5855.txt"
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttH_semilep.p5855.txt"
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttH_allhad.p5855.txt"

# ttbar hdamp258p75
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttbar_allhad.p5855.txt"
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttbar_dilep.p5855.txt"
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttbar_nonallhad.p5855.txt"

# ttZ (ttZqq, ttZnunu, tttautau, ttmumu, ttee)
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttZ.p5855.txt"

# ttZZ
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttZZ.p5855.txt"

# ttW 700168.Sh_2210
# missing PHYSLITE for 700706.Sh_2212, also Sherpa 2.2.4 and Sherpa 2.2.12 versions are deprecated, need to migrate to Sherpa 2.2.14
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttW.p5855.txt"

# ttWW
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttWW.p5855.txt"

# ttWH
# missing PHYSLITE so using PHYS for the moment
submit_jobs "${folder_name_PHYS}/mc20_13TeV.ttWH.p5855.txt"

# ttWZ
# missing PHYSLITE so using PHYS for the moment
submit_jobs "${folder_name_PHYS}/mc20_13TeV.ttWZ.p5855.txt"

# tttt
# missing PHYSLITE so using PHYS for the moment
# submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.tttt.p5855.txt"
submit_jobs "${folder_name_PHYS}/mc20_13TeV.tttt_MGPy8EG_A14NNPDF31.p5855.txt"
submit_jobs "${folder_name_PHYS}/mc20_13TeV.tttt_aMcAtNloHerwig7EvtGen_H7UE.p5855.txt"
submit_jobs "${folder_name_PHYS}/mc20_13TeV.tttt_aMcAtNloPythia8EvtGen_A14NNPDF31.p5855.txt"

# ttt
submit_jobs "${folder_name_PHYSLITE}/mc20_13TeV.ttt.p5855.txt"