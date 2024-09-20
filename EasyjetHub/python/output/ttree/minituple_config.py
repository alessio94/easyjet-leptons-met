from operator import attrgetter
from typing import List, Optional

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import seqAND, parOR

from EasyjetHub.steering.utils.log_helper import log
from EasyjetHub.output.ttree.eventinfo import get_event_info_branches
from EasyjetHub.output.ttree.electrons import get_electron_branches
from EasyjetHub.output.ttree.photons import get_photon_branches
from EasyjetHub.output.ttree.muons import get_muon_branches
from EasyjetHub.output.ttree.taus import get_tau_branches
from EasyjetHub.output.ttree.small_R_jets import (
    get_small_R_jet_branches
)
from EasyjetHub.output.ttree.large_R_jets import (
    get_large_R_jet_branches,
)
from EasyjetHub.output.ttree.truth_jets import (
    get_large_R_truthjet_branches,
    get_small_R_truthjet_branches,
)
from EasyjetHub.output.ttree.truth_taus import get_truthtau_branches
from EasyjetHub.output.ttree.met import get_met_branches
from EasyjetHub.steering.utils.config_flags import ConfigItem

from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory


# Permit aliasing of the container names
# The full path under Analysis is substituted if possible
def substitute_container_name(flags: AthConfigFlags, name: str) -> str:
    _name = name
    analysis_flags = flags.Analysis
    try:
        # With attrgetter we can retrieve a name containing '.'
        # which works for names a few levels deeper in the flags
        _name = attrgetter(name)(analysis_flags)
    except AttributeError:
        pass
    return _name


def output_analysis_sequence(
        flags: AthConfigFlags,
        branches: list[str],
        met_branches: list[str],
        treename: str = "AnalysisMiniTree",
        streamname: str = "ANALYSIS"
) -> ConfigSequence:
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    configSeq += makeConfig('Output')
    configSeq.setOptionValue('.vars', branches)
    configSeq.setOptionValue('.metVars', met_branches)
    configSeq.setOptionValue('.treeName', treename)
    configSeq.setOptionValue('.streamName', streamname)
    configSeq.setOptionValue('.postfix', treename)

    return configSeq


def minituple_output_cfg(
    flags: AthConfigFlags,
    tree_flags: ConfigItem,
    outfile_name: str,
    stream_name: str,
    tree_name: str = "AnalysisMiniTree",
    extra_output_branches: Optional[List[str]] = None,
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

    tree_branches = []

    tree_branches += get_event_info_branches(
        flags, tree_flags, flags.Analysis.TriggerChains
    )

    objects_out = {
        "electrons": ("el", get_electron_branches),
        "photons": ("ph", get_photon_branches),
        "muons": ("mu", get_muon_branches),
        "taus": ("tau", get_tau_branches),
    }
    for objtype, (prefix, branch_getter) in objects_out.items():
        write_container_flag = tree_flags.reco_outputs[objtype]
        if write_container_flag:
            write_container = substitute_container_name(flags, write_container_flag)
            tree_branches += branch_getter(
                flags,
                tree_flags,
                input_container=write_container,
                output_prefix=prefix,
            )

    if tree_flags.reco_outputs.small_R_jets:
        small_R_name = substitute_container_name(
            flags,
            tree_flags.reco_outputs.small_R_jets
        )
        small_R_prefix = (
            "recojet_antikt4"
            + flags.Analysis.Small_R_jet.jet_type.replace("reco4", "").replace("Jet", "")  # noqa: E501
        )
        tree_branches += get_small_R_jet_branches(
            flags, tree_flags,
            input_container=small_R_name,
            output_prefix=small_R_prefix,
        )

    if tree_flags.reco_outputs.large_R_Topo_jets:
        large_R_jet_name = substitute_container_name(
            flags,
            tree_flags.reco_outputs.large_R_Topo_jets
        )
        tree_branches += get_large_R_jet_branches(
            flags, tree_flags,
            input_container=large_R_jet_name,
            output_prefix="recojet_antikt10Topo",
            lr_jet_type="Topo",
        )

    if tree_flags.reco_outputs.large_R_UFO_jets:
        large_R_jet_name = substitute_container_name(
            flags,
            tree_flags.reco_outputs.large_R_UFO_jets
        )
        tree_branches += get_large_R_jet_branches(
            flags, tree_flags,
            input_container=large_R_jet_name,
            output_prefix="recojet_antikt10UFO",
            lr_jet_type="UFO",
        )

    met_branches = []
    if tree_flags.reco_outputs.met:
        met_branches += get_met_branches(
            flags,
            input_container=substitute_container_name(
                flags, tree_flags.reco_outputs.met
            ),
            output_prefix="met"
        )

    if flags.Input.isMC and tree_flags.truth_outputs.small_R_jets:
        tree_branches += get_small_R_truthjet_branches(
            flags,
            input_container=substitute_container_name(
                flags, tree_flags.truth_outputs.small_R_jets
            ),
            output_prefix="truthjet_antikt4",
        )

    large_R_truth_flags = tree_flags.truth_outputs.large_R_jets
    if flags.Input.isMC and large_R_truth_flags:
        def add_large_R_truth(flags, large_R_name):
            large_R_name = substitute_container_name(flags, large_R_name)
            large_R_type = ''
            if 'Trimmed' in large_R_name:
                large_R_type = 'Trimmed'
            if 'SoftDrop' in large_R_name:
                large_R_type = 'SoftDrop'
            return get_large_R_truthjet_branches(
                flags,
                input_container=large_R_name,
                output_prefix="truthjet_antikt10" + large_R_type,
            )
        if isinstance(large_R_truth_flags, tuple):
            for coll in large_R_truth_flags:
                tree_branches += add_large_R_truth(flags, coll)
        else:
            tree_branches += add_large_R_truth(flags, large_R_truth_flags)

    if flags.Input.isMC and tree_flags.truth_outputs.taus:
        tree_branches += get_truthtau_branches(
            flags,
            input_container=substitute_container_name(
                flags, tree_flags.truth_outputs.taus
            ),
            output_prefix="truthtau",
        )

    if extra_output_branches:
        log.info(
            f"Appending {len(extra_output_branches)} branches from args"
        )
        tree_branches += extra_output_branches

    if tree_flags.extra_output_branches:
        log.info(
            f"Appending {len(tree_flags.extra_output_branches)} branches from yaml"
        )
        tree_branches += list(tree_flags.extra_output_branches)

    log.info("Add tree seq")

    # Add an instance of THistSvc, to create the output file and associated stream.
    # This is needed so that the alg can register its output TTree.
    # The syntax for the output is:
    #   Stream name: (default is "ANALYSIS" assumed by AthHistogramAlgorithm)
    #   Output file name: specified by setting "DATAFILE"
    #   File I/O option: specified by setting "OPT" and passed to the TFile constructor
    #      "RECREATE" will (over)write the specified file name with a new file
    cfg.addService(
        CompFactory.THistSvc(
            Output=[f"{stream_name} DATAFILE='{outfile_name}',"
                    " OPT='RECREATE'"]
        )
    )

    log.info(
        f"Writing tree AnalysisMiniTree"
        f" to '{outfile_name}' via stream '{stream_name}'"
    )

    outputSeq = CompFactory.AthSequencer('OutputSequence' + tree_name)
    outputConfigAccumulator = ConfigAccumulator(
        outputSeq,
        autoconfigFromFlags=flags,
    )
    outputConfigSeq = output_analysis_sequence(flags, branches=tree_branches,
                                               met_branches=met_branches,
                                               treename=tree_name,
                                               streamname=stream_name)
    outputConfigSeq.fullConfigure(outputConfigAccumulator)
    cfg.merge(outputConfigAccumulator.CA)

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


def minituple_cfg(
    flags: AthConfigFlags,
    tree_flags: ConfigItem,
    outfile_name: str,
    tree_name: str = "AnalysisMiniTree",
    extra_output_branches: Optional[List[str]] = None,
) -> ComponentAccumulator:
    """
    Extra config layer on top of minituple_output_cfg to allow splitting between
    different output trees per channel
    """
    cfg = ComponentAccumulator()

    seqname = "OutputSeq"
    cfg.addSequence(parOR(seqname))

    if flags.Analysis.splitCBK:
        # Define output stream for cut bookkeeper
        cbk_outfile = flags.Analysis.out_file.replace(".root", "_cbk.root")
        cfg.addService(
            CompFactory.THistSvc(
                Output=[f"CBK DATAFILE='{cbk_outfile}',"
                        " OPT='RECREATE'"]
            )
        )

    channelList = [""]
    if flags.Analysis.splitOutputTree:
        channelList = flags.Analysis.channels

    for channel in channelList:
        channelSeqName = seqname
        outfile = flags.Analysis.out_file

        if flags.Analysis.splitOutputTree:
            channelSeqName = seqname + channel
            outfile = outfile.replace(".root", "_" + channel + ".root")
            cfg.addSequence(seqAND(channelSeqName), parentName=seqname)
            cfg.addEventAlgo(
                CompFactory.Easyjet.EventSelectorAlg(
                    "EventSelectorAlg_" + channel,
                    filterDecoration="pass_" + channel + "_%SYS%",
                    # Dummy output decoration, required for all selector alg
                    eventDecisionOutputDecoration="out_" + channel + "_%SYS%"),
                sequenceName=channelSeqName)

        cfg.merge(
            minituple_output_cfg(
                flags, tree_flags,
                outfile_name=outfile,
                stream_name=tree_flags.stream_name + channel,
                tree_name=tree_flags.tree_name + channel,
                extra_output_branches=extra_output_branches
            ),
            channelSeqName,
        )

    return cfg
