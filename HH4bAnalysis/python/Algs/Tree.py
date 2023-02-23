from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def AnalysisTreeAlgCfg(flags, branches, treename="AnalysisMiniTree"):
    cfg = ComponentAccumulator()
    # Create analysis mini-ntuple
    treeMaker = CompFactory.getComp("CP::TreeMakerAlg")("TreeMaker")
    treeMaker.TreeName = treename
    cfg.addEventAlgo(treeMaker)

    # Add branches
    ntupleMaker = CompFactory.getComp("CP::AsgxAODNTupleMakerAlg")("NTupleMaker")
    ntupleMaker.TreeName = treename
    ntupleMaker.Branches = branches
    ntupleMaker.systematicsService = 'SystematicsSvc'
    cfg.addEventAlgo(ntupleMaker)

    # Fill tree
    treeFiller = CompFactory.getComp("CP::TreeFillerAlg")("TreeFiller")
    treeFiller.TreeName = treename
    cfg.addEventAlgo(treeFiller)

    return cfg
