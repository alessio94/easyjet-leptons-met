from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.steering.container_names import get_container_names
from EasyjetHub.steering.utils.log_helper import log
from EasyjetHub.output.ttree.eventinfo import get_event_info_branches
from EasyjetHub.output.ttree.electrons import get_electron_branches
from EasyjetHub.output.ttree.photons import get_photon_branches
from EasyjetHub.output.ttree.muons import get_muon_branches
from EasyjetHub.output.ttree.taus import get_tau_branches
from EasyjetHub.output.ttree.small_R_jets import (
    get_small_R_jet_branches,
    get_small_R_bjet_branches,
)
from EasyjetHub.output.ttree.large_R_jets import (
    get_large_R_jet_branches,
)
from EasyjetHub.output.ttree.truth_jets import (
    get_large_R_truthjet_branches,
    get_small_R_truthjet_branches,
)
from EasyjetHub.output.ttree.met import get_met_branches


def tree_cfg(flags, branches, treename="AnalysisMiniTree"):
    cfg = ComponentAccumulator()
    # Create analysis mini-ntuple
    treeMaker = CompFactory.getComp("CP::TreeMakerAlg")("TreeMaker")
    treeMaker.TreeName = treename
    cfg.addEventAlgo(treeMaker)

    # Add branches
    ntupleMaker = CompFactory.getComp("CP::AsgxAODNTupleMakerAlg")("NTupleMaker")
    ntupleMaker.TreeName = treename
    ntupleMaker.Branches = branches
    ntupleMaker.systematicsService = "SystematicsSvc"
    cfg.addEventAlgo(ntupleMaker)

    # Fill tree
    treeFiller = CompFactory.getComp("CP::TreeFillerAlg")("TreeFiller")
    treeFiller.TreeName = treename
    cfg.addEventAlgo(treeFiller)

    return cfg


def minituple_cfg(flags):
    cfg = ComponentAccumulator()

    containers = get_container_names(flags)["outputs"]
    log.debug(f"Containers requested in dataset: {containers}")

    ########################################################################
    # Create analysis mini-ntuple
    ########################################################################

    # Add an instance of THistSvc, to create the output file and associated stream.
    # This is needed so that the alg can register its output TTree.
    # The syntax for the output is:
    #   Stream name: "ANALYSIS" (default assumed by AthHistogramAlgorithm)
    #   Output file name: specified by setting "DATAFILE"
    #   File I/O option: specified by setting "OPT" and passed to the TFile constructor
    #      "RECREATE" will (over)write the specified file name with a new file
    cfg.addService(
        CompFactory.THistSvc(
            Output=[f"ANALYSIS DATAFILE='{flags.Analysis.out_file}', OPT='RECREATE'"]
        )
    )

    tree_branches = []

    tree_branches += get_event_info_branches(
        flags, flags.Analysis.doPRW, flags.Analysis.TriggerChains
    )

    objects_out = {
        "electrons": ("el", get_electron_branches),
        "photons": ("ph", get_photon_branches),
        "muons": ("mu", get_muon_branches),
        "taus": ("tau", get_tau_branches),
    }
    for objtype, (prefix, branch_getter) in objects_out.items():
        if flags(f"Analysis.write_{objtype}"):
            tree_branches += branch_getter(
                flags,
                input_container=containers[objtype],
                output_prefix=prefix,
            )

    if flags.Analysis.write_small_R_jets:
        tree_branches += get_small_R_jet_branches(
            flags,
            input_container=containers["reco4PFlowJet"],
            output_prefix="recojet_antikt4PFlow",
        )

        # Use this to directly read b-tagging information
        # Needs to be decorated onto the jet container
        # to handle jet selection (thinning)
        tree_branches += get_small_R_bjet_branches(
            flags,
            input_container=containers["reco4PFlowJet"],
            output_prefix="recojet_antikt4PFlow",
        )

    if flags.Analysis.write_large_R_Topo_jets:
        tree_branches += get_large_R_jet_branches(
            flags,
            input_container=containers["reco10TopoJet"],
            output_prefix="recojet_antikt10Topo",
            lr_jet_type="Topo",
        )

    if flags.Analysis.write_large_R_UFO_jets:
        tree_branches += get_large_R_jet_branches(
            flags,
            input_container=containers["reco10UFOJet"],
            output_prefix="recojet_antikt10UFO",
            lr_jet_type="UFO",
        )

    if flags.Analysis.write_met:
        tree_branches += get_met_branches(
            flags,
            input_container=containers["met"],
            output_prefix="met"
        )

    if flags.Input.isMC and flags.Analysis.write_truth_small_R_jets:
        tree_branches += get_small_R_truthjet_branches(
            flags,
            input_container=containers["truth4Jet"],
            output_prefix="truthjet_antikt4PFlow",
        )

    if flags.Input.isMC and flags.Analysis.write_truth_large_R_jets:
        if flags.Analysis.write_large_R_Topo_jets:
            tree_branches += get_large_R_truthjet_branches(
                flags,
                input_container=containers["truth10TrimmedJet"],
                output_prefix="truthjet_antikt10Trimmed",
            )
        if flags.Analysis.write_large_R_UFO_jets:
            tree_branches += get_large_R_truthjet_branches(
                flags,
                input_container=containers["truth10SoftDropJet"],
                output_prefix="truthjet_antikt10SoftDrop",
            )

    if flags.Analysis.extra_output_branches:
        log.info(
            f"Appending {len(flags.Analysis.extra_output_branches)} extra branches"
        )
        tree_branches += flags.Analysis.extra_output_branches

    log.info("Add tree seq")
    cfg.merge(tree_cfg(flags, branches=tree_branches))

    if flags.Analysis.dump_output_branchlist:
        outf_sub = flags.Analysis.out_file.replace("root", "txt")
        if "/" in outf_sub:
            outf_dir, outf_sub = outf_sub.rsplit("/", 1)
            branches_fname = f"{outf_dir}/output-branches-{outf_sub}"
        else:
            branches_fname = f"output-branches-{outf_sub}"

        with open(branches_fname, "w") as branches_f:
            for b in tree_branches:
                branches_f.write(f"{b}\n")

    return cfg
