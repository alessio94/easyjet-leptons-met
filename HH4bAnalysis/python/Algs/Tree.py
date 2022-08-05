from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def AnalysisTreeAlgCfg(flags, branches):
    cfg = ComponentAccumulator()
    # Create analysis mini-ntuple
    treeMaker = CompFactory.getComp("CP::TreeMakerAlg")("TreeMaker")
    treeMaker.TreeName = "AnalysisMiniTree"
    cfg.addEventAlgo(treeMaker)

    # Add branches
    ntupleMaker = CompFactory.getComp("CP::AsgxAODNTupleMakerAlg")("NTupleMaker")
    ntupleMaker.TreeName = "AnalysisMiniTree"
    ntupleMaker.Branches = branches
    cfg.addEventAlgo(ntupleMaker)

    # Fill tree
    treeFiller = CompFactory.getComp("CP::TreeFillerAlg")("TreeFiller")
    treeFiller.TreeName = "AnalysisMiniTree"
    cfg.addEventAlgo(treeFiller)

    return cfg
