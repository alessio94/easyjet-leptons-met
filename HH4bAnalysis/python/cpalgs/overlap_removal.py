from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def overlap_sequence_cfg(
    flags,
    containers,
):
    cfg = ComponentAccumulator()
    from AsgAnalysisAlgorithms.OverlapAnalysisSequence import (
        makeOverlapAnalysisSequence,
    )

    if (
        flags.Analysis.write_large_R_Topo_jets
        and flags.Analysis.write_large_R_UFO_jets
    ):
        raise ValueError("Overlap removal only works with one Large R collection")
    overlapInputNames = {}
    for objtype in ["muons", "electrons", "photons"]:
        if flags(f"Analysis.do_{objtype}"):
            overlapInputNames[objtype] = containers["outputs"][objtype]

    if flags.Analysis.do_small_R_jets:
        overlapInputNames["jets"] = containers["outputs"]["reco4Jet"]

    do_fatJet_OR = False
    if flags.Analysis.write_large_R_Topo_jets:
        overlapInputNames["fatJets"] = containers["outputs"]["reco10TopoJet"]
        do_fatJet_OR = True
    if flags.Analysis.write_large_R_UFO_jets:
        overlapInputNames["fatJets"] = containers["outputs"]["reco10UFOJet"]
        do_fatJet_OR = True

    overlapOutputNames = {k: f"{v}_OR" for k, v in overlapInputNames.items()}

    overlapSequence = makeOverlapAnalysisSequence(
        flags.Analysis.DataType,
        inputLabel="",
        outputLabel="passesOR",
        linkOverlapObjects=False,
        doEleEleOR=False,
        doTaus=False,
        enableUserPriority=False,
        bJetLabel="",
        boostedLeptons=False,
        postfix="",
        shallowViewOutput=True,
        enableCutflow=False,
        doJets=flags.Analysis.do_small_R_jets,
        doMuons=flags.Analysis.do_muons,
        doElectrons=flags.Analysis.do_electrons,
        doPhotons=flags.Analysis.do_photons,
        doFatJets=do_fatJet_OR
    )
    overlapSequence.configure(
        inputName=overlapInputNames,
        outputName=overlapOutputNames,
    )
    # print(overlapSequence)  # For debugging

    cfg.addSequence(CompFactory.AthSequencer(overlapSequence.getName()))
    for alg in overlapSequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, overlapSequence.getName())

    return cfg
