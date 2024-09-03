from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.steering.utils.log_helper import log
from EasyjetHub.algs.truth.parent_decorator_config import parent_decorator_cfg
from EasyjetHub.algs.truth_particle_info_config import truth_particle_info_cfg

import pathlib
import os


def truth_info_cfg(
    flags,
):
    cfg = ComponentAccumulator()

    if not flags.Input.isPHYSLITE:
        # truth record seems to be broken in physlite
        if flags.Analysis.do_small_R_jets \
           and flags.Analysis.Small_R_jet.do_parent_decoration:
            cfg.merge(parent_decorator_cfg(
                flags,
                targetContainer=flags.Analysis.container_names.input[
                    flags.Analysis.Small_R_jet.jet_type
                ],
                prefix="smallR"
            ))

        if flags.Analysis.do_large_R_UFO_jets and \
           flags.Analysis.Large_R_jet.do_parent_decoration:
            cfg.merge(parent_decorator_cfg(
                flags,
                targetContainer=flags.Analysis.container_names.input.reco10UFOJet,
                prefix="largeR",
                matchDeltaR=1.0,
            ))

        if flags.Analysis.do_large_R_Topo_jets and \
           flags.Analysis.Large_R_jet.do_parent_decoration:
            cfg.merge(parent_decorator_cfg(
                flags,
                targetContainer=flags.Analysis.container_names.input.reco10TopoJet,
                prefix="largeRTopo",
                matchDeltaR=1.0,
            ))

        if flags.Analysis.Tau.do_parent_decoration:
            cfg.merge(parent_decorator_cfg(
                flags, targetContainer=flags.Analysis.container_names.input.taus,
                prefix="Tau"
            ))

        if flags.Analysis.Muon.do_parent_decoration:
            cfg.merge(parent_decorator_cfg(
                flags, targetContainer=flags.Analysis.container_names.input.muons,
                prefix="Muon"
            ))

        if flags.Analysis.Electron.do_parent_decoration:
            cfg.merge(parent_decorator_cfg(
                flags, targetContainer=flags.Analysis.container_names.input.electrons,
                prefix="Electron"
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


def contain_dalitz(input):
    # Determine if input is flags or an integer DSID
    if isinstance(input, int):
        dsid = str(input)
    else:
        dsid = str(input.Input.MCChannelNumber)

    def FullPath(rawpath):
        fpath = pathlib.Path(rawpath)
        for dirpath in [""] + os.environ["DATAPATH"].split(":"):
            fullpath = dirpath / fpath
            if fullpath.exists():
                return fullpath

    # file name hard-coded
    with open(FullPath("EasyjetHub/DalitzDataset.txt"), 'r') as file_in:
        dataset_list = file_in.readlines()
        for dataset in dataset_list:
            if dsid in dataset:
                return True
    return False


def bbyy_filter_dalitz_cfg(flags):
    cfg = ComponentAccumulator()

    cfg.addEventAlgo(
        CompFactory.HHBBYY.bbyyFilterDalitzAlg(
            "bbyyFilterDalitzAlg",
            TruthParticleSMInKey=(
                flags.Analysis.container_names.input.truthSMParticles
            ),
            TruthParticleBSMInKey=(
                flags.Analysis.container_names.input.truthBSMParticles
            ),
        ),
    )

    return cfg
