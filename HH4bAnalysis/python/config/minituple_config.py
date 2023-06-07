from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from HH4bAnalysis.config.boosted_config import boosted_branches
from HH4bAnalysis.config.container_names import get_container_names
from HH4bAnalysis.config.resolved_config import resolved_branches
from HH4bAnalysis.config.yybb_config import yybb_branches
from HH4bAnalysis.cpalgs.tree import tree_cfg
from HH4bAnalysis.output_branches.electrons import get_electron_branches
from HH4bAnalysis.output_branches.eventinfo import get_event_info_branches
from HH4bAnalysis.output_branches.large_R_jets import get_large_R_jet_branches
from HH4bAnalysis.output_branches.muons import get_muon_branches
from HH4bAnalysis.output_branches.photons import get_photon_branches
from HH4bAnalysis.output_branches.small_R_jets import (
    get_small_R_bjet_branches,
    get_small_R_jet_branches,
)
from HH4bAnalysis.output_branches.truth_jets import (
    get_large_R_truthjet_branches,
    get_small_R_truthjet_branches,
)
from HH4bAnalysis.utils.log_helper import log


def minituple_cfg(flags, trigger_chains, do_PRW=False):
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

    tree_branches += get_event_info_branches(flags, do_PRW, trigger_chains)

    objects_out = {
        "electrons": ("el", get_electron_branches),
        "photons": ("ph", get_photon_branches),
        "muons": ("mu", get_muon_branches),
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
        # Note that the indices can get out of sync with
        # the small-R jet container if jet thinning is
        # done -- see issue #67
        tree_branches += get_small_R_bjet_branches(
            flags,
            input_container="BTagging_AntiKt4EMPFlow",
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

    if flags.Analysis.do_resolved_dihiggs_analysis and not flags.Analysis.disable_calib:
        tree_branches += resolved_branches(flags)

    if flags.Analysis.do_boosted_dihiggs_analysis and not flags.Analysis.disable_calib:
        tree_branches += boosted_branches(flags)

    if flags.Analysis.do_yybb_analysis:
        tree_branches += yybb_branches(flags)

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
