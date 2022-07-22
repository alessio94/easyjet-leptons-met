from AthenaConfiguration.ComponentFactory import CompFactory


def makeAndAddAnalysisTreeAlg(compAcc, branches):
    # Create analysis mini-ntuple
    treeMaker = CompFactory.getComp("CP::TreeMakerAlg")("TreeMaker")
    treeMaker.TreeName = "AnalysisMiniTree"
    compAcc.addEventAlgo(treeMaker)

    # Add branches
    ntupleMaker = CompFactory.getComp("CP::AsgxAODNTupleMakerAlg")("NTupleMaker")
    ntupleMaker.TreeName = "AnalysisMiniTree"
    ntupleMaker.Branches = branches
    compAcc.addEventAlgo(ntupleMaker)

    # Fill tree
    treeFiller = CompFactory.getComp("CP::TreeFillerAlg")("TreeFiller")
    treeFiller.TreeName = "AnalysisMiniTree"
    compAcc.addEventAlgo(treeFiller)

    return treeMaker, ntupleMaker, treeFiller
