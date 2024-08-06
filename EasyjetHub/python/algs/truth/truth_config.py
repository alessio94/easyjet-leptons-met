from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.steering.utils.log_helper import log
from EasyjetHub.algs.truth.jet_parent_decorator_config import jet_parent_decorator_cfg
from EasyjetHub.algs.truth.tau_parent_decorator_config import tau_parent_decorator_cfg
from EasyjetHub.algs.truth.muon_parent_decorator_config import muon_parent_decorator_cfg
from EasyjetHub.algs.truth.electron_parent_decorator_config import (
    electron_parent_decorator_cfg
)
from EasyjetHub.algs.truth_particle_info_config import truth_particle_info_cfg


def truth_info_cfg(
    flags,
):
    cfg = ComponentAccumulator()

    if flags.Analysis.do_jet_parent_decoration and not flags.Input.isPHYSLITE:
        # truth record seems to be broken in physlite
        if flags.Analysis.do_small_R_jets:
            cfg.merge(jet_parent_decorator_cfg(
                flags,
                jet_collection=flags.Analysis.container_names.input[
                    flags.Analysis.small_R_jet.jet_type
                ],
                name_prefix="smallR",
                match_dr=0.3
            ))

        if flags.Analysis.do_large_R_UFO_jets:
            cfg.merge(jet_parent_decorator_cfg(
                flags,
                jet_collection=flags.Analysis.container_names.input.reco10UFOJet,
                name_prefix="largeR",
                match_dr=1.0,
            ))

        if flags.Analysis.do_large_R_Topo_jets:
            cfg.merge(jet_parent_decorator_cfg(
                flags,
                jet_collection=flags.Analysis.container_names.input.reco10TopoJet,
                name_prefix="largeRTopo",
                match_dr=1.0,
            ))

    if flags.Analysis.do_tau_parent_decoration and not flags.Input.isPHYSLITE:
        cfg.merge(tau_parent_decorator_cfg(
            flags,
            tau_collection=flags.Analysis.container_names.input.taus,
            name_prefix="Tau",
            match_dr=0.3,
        ))

    if flags.Analysis.do_muon_parent_decoration and not flags.Input.isPHYSLITE:
        cfg.merge(muon_parent_decorator_cfg(
            flags,
            muon_collection=flags.Analysis.container_names.input.muons,
            name_prefix="Muon",
            match_dr=0.3,
        ))

    if flags.Analysis.do_electron_parent_decoration and not flags.Input.isPHYSLITE:
        cfg.merge(electron_parent_decorator_cfg(
            flags,
            electron_collection=flags.Analysis.container_names.input.electrons,
            name_prefix="Electron",
            match_dr=0.3,
        ))

    log.info("Adding truth particle info seq")
    cfg.merge(
        truth_particle_info_cfg(flags)
    )

    return cfg


def sumofweightsalg_cfg(flags):
    cfg = ComponentAccumulator()

    cfg.addEventAlgo(
        CompFactory.Easyjet.SumOfWeightsAlg(
            "SumOfWeightsAlg",
        ),
    )

    return cfg
