from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
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


def tree_cfg(
    flags: AthConfigFlags,
    branches: list[str],
    treename: str = "AnalysisMiniTree",
    outfile:  str = "output.root",
    stream:   str = "ANALYSIS",
    treedir:  str = "",
) -> ComponentAccumulator:
    """
    Configures output of a single TTree
    Different calls can output different trees and/or root files
    The 'stream' provides a mapping to the output file
    One stream can only route to one root file, but can write
    multiple trees (and histograms etc)
    """

    cfg = ComponentAccumulator()

    # Add an instance of THistSvc, to create the output file and associated stream.
    # This is needed so that the alg can register its output TTree.
    # The syntax for the output is:
    #   Stream name: (default is "ANALYSIS" assumed by AthHistogramAlgorithm)
    #   Output file name: specified by setting "DATAFILE"
    #   File I/O option: specified by setting "OPT" and passed to the TFile constructor
    #      "RECREATE" will (over)write the specified file name with a new file
    cfg.addService(
        CompFactory.THistSvc(
            Output=[f"{stream} DATAFILE='{outfile}', OPT='RECREATE'"]
        )
    )

    log.info(
        f"Writing tree '{treedir}/{treename}'"
        f" to '{outfile}' via stream '{stream}'"
    )

    # Create analysis mini-ntuple
    treeMaker = CompFactory.CP.TreeMakerAlg(
        f"TreeMaker_{treename}",
        RootStreamName=f"/{stream}",
        RootDirName=treedir,
    )
    treeMaker.TreeName = treename
    cfg.addEventAlgo(treeMaker)

    # Add branches
    ntupleMaker = CompFactory.CP.AsgxAODNTupleMakerAlg(
        f"NTupleMaker_{treename}",
        TreeName=treename,
        Branches=branches,
        systematicsService="SystematicsSvc",
        RootStreamName=f"/{stream}",
        RootDirName=treedir,
    )
    cfg.addEventAlgo(ntupleMaker)

    # Fill tree
    treeFiller = CompFactory.CP.TreeFillerAlg(
        f"TreeFiller_{treename}",
        TreeName=treename,
        RootStreamName=f"/{stream}",
        RootDirName=treedir,
    )
    cfg.addEventAlgo(treeFiller)

    return cfg


def minituple_cfg(
    flags: AthConfigFlags,
    tree_name: str,
    outfile_name: str,
) -> ComponentAccumulator:
    """
    This is the template output TTree configuration, steered via yaml config.
    It uses the branch managers to configure writing out the standard object
    collections, producing /AnalysisMiniTree in the output ROOT file set
    in the RunConfig.
    """
    cfg = ComponentAccumulator()

    ########################################################################
    # Create analysis mini-ntuple
    ########################################################################

    # Extract the set of flags pertaining to this tree
    tree_flags = getattr(flags.Analysis.ttree_output,tree_name)

    tree_branches = []

    tree_branches += get_event_info_branches(
        flags, tree_flags, flags.Analysis.doPRW, flags.Analysis.TriggerChains
    )

    objects_out = {
        "electrons": ("el", get_electron_branches),
        "photons": ("ph", get_photon_branches),
        "muons": ("mu", get_muon_branches),
        "taus": ("tau", get_tau_branches),
    }
    for objtype, (prefix, branch_getter) in objects_out.items():
        if getattr(tree_flags.reco_outputs,f'{objtype}'):
            tree_branches += branch_getter(
                flags,
                tree_flags,
                input_container=getattr(flags.Analysis.container_names.output,objtype),
                output_prefix=prefix,
            )

    if tree_flags.reco_outputs.small_R_jets:
        tree_branches += get_small_R_jet_branches(
            flags, tree_flags,
            input_container=flags.Analysis.container_names.output.reco4PFlowJet,
            output_prefix="recojet_antikt4PFlow",
        )

        # Use this to directly read b-tagging information
        # Needs to be decorated onto the jet container
        # to handle jet selection (thinning)
        tree_branches += get_small_R_bjet_branches(
            flags, tree_flags,
            input_container=flags.Analysis.container_names.output.reco4PFlowJet,
            output_prefix="recojet_antikt4PFlow",
        )

    if tree_flags.reco_outputs.large_R_Topo_jets:
        tree_branches += get_large_R_jet_branches(
            flags, tree_flags,
            input_container=flags.Analysis.container_names.output.reco10TopoJet,
            output_prefix="recojet_antikt10Topo",
            lr_jet_type="Topo",
        )

    if tree_flags.reco_outputs.large_R_UFO_jets:
        tree_branches += get_large_R_jet_branches(
            flags, tree_flags,
            input_container=flags.Analysis.container_names.output.reco10UFOJet,
            output_prefix="recojet_antikt10UFO",
            lr_jet_type="UFO",
        )

    if tree_flags.reco_outputs.met:
        tree_branches += get_met_branches(
            flags,
            input_container=flags.Analysis.container_names.output.met,
            output_prefix="met"
        )

    if flags.Input.isMC and tree_flags.truth_outputs.small_R_jets:
        tree_branches += get_small_R_truthjet_branches(
            flags,
            input_container=flags.Analysis.container_names.input.truth4Jet,
            output_prefix="truthjet_antikt4PFlow",
        )

    if flags.Input.isMC and tree_flags.truth_outputs.large_R_jets:
        if tree_flags.reco_outputs.large_R_Topo_jets:
            tree_branches += get_large_R_truthjet_branches(
                flags,
                input_container=flags.Analysis.container_names.input.truth10TrimmedJet,
                output_prefix="truthjet_antikt10Trimmed",
            )
        if tree_flags.reco_outputs.large_R_UFO_jets:
            tree_branches += get_large_R_truthjet_branches(
                flags,
                input_container=flags.Analysis.container_names.input.truth10SoftDropJet,
                output_prefix="truthjet_antikt10SoftDrop",
            )

    if tree_flags.extra_output_branches:
        log.info(
            f"Appending {len(tree_flags.extra_output_branches)} extra branches"
        )
        tree_branches += tree_flags.extra_output_branches

    log.info("Add tree seq")
    cfg.merge(tree_cfg(
        flags,
        branches=tree_branches,
        outfile=outfile_name,
        stream=tree_flags.stream_name,
        treedir=tree_flags.directory_name,
    ))

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
