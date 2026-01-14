from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import LHCPeriod
import AthenaCommon.SystemOfUnits as Units
from EasyjetHub.steering.main_sequence_config import metadata_cfg


def get_filenames_bb4l(jet_algo, data_taking_period):
    """Generate calibration filenames for bb4l mode"""
    return {
        "EnRespFilename": (
            f"KinematicFitTool/E_{jet_algo}_TaOgataParameters_"
            f"{data_taking_period}_bb4l.root"
        ),
        "PtRespFilename": (
            f"KinematicFitTool/pT_{jet_algo}_TaOgataParameters_"
            f"{data_taking_period}_bb4l.root"
        ),
        "pXconstrFilename": (
            f"KinematicFitTool/pXconstr_{jet_algo}_DSCBParameters_bb4l.root"
        ),
        "pYconstrFilename": (
            f"KinematicFitTool/pYconstr_{jet_algo}_DSCBParameters_bb4l.root"
        )
    }


def get_filenames_bbyy(jet_algo, data_taking_period):
    """Generate calibration filenames for bbyy mode"""
    return {
        "EnRespFilename": (
            f"KinematicFitTool/E_{jet_algo}_TaOgataParameters_"
            f"{data_taking_period}_FastSim_PeakCentered.root"
        ),
        "pXconstrFilename": (
            f"KinematicFitTool/pXconstr_{jet_algo}_TaOTaParameters_updated0925.root"
        ),
        "pYconstrFilename": (
            f"KinematicFitTool/pYconstr_{jet_algo}_TaOTaParameters_updated0925.root"
        )
    }


def KinematicFitTool_bb4l_Cfg(flags, name="KinematicFitTool_bb4l", **kwargs):
    """Configure KinematicFitTool for bb4l analysis with sensible defaults"""
    cfg = ComponentAccumulator()

    # Set default parameters for bb4l mode
    kwargs.setdefault("mode", "bb4l")
    kwargs.setdefault("JetCollection", "AntiKt4EMPFlow")

    # Set lambda parameters for bb4l
    kwargs.setdefault("LambdaMomConstraint", 2.)
    kwargs.setdefault("LambdaMassConstraint", 0.25)

    # Get calibration filenames
    jet_algo = kwargs.get("JetCollection", "AntiKt4EMPFlow")
    data_taking_period = "Run3" if flags.GeoModel.Run is LHCPeriod.Run3 else "Run2"
    calib_files = get_filenames_bb4l(jet_algo, data_taking_period)
    for key, filename in calib_files.items():
        kwargs.setdefault(key, filename)

    # Default binning parameters
    kwargs.setdefault("Barrel_E_binning", [2.9, 3.8, 4.3, 4.6, 4.9, 5.3, 6.9])
    kwargs.setdefault("Crack_E_binning", [2.9, 4.4, 4.7, 5.0, 5.4, 5.7, 6.9])
    kwargs.setdefault("Endcap_E_binning", [2.9, 4.8, 5.1, 5.4, 5.7, 6.0, 6.9])
    kwargs.setdefault("No_Track_E_binning", [2.9, 5.7, 6.0, 6.4, 6.7, 7.0, 8.2])
    kwargs.setdefault("Barrel_pT_binning", [2.9, 3.6, 4.0, 4.4, 4.6, 5.0, 6.2])
    kwargs.setdefault("Crack_pT_binning", [2.9, 3.6, 3.9, 4.3, 4.5, 4.9, 6.2])
    kwargs.setdefault("Endcap_pT_binning", [2.9, 3.5, 3.8, 4.1, 4.4, 4.8, 6.2])
    kwargs.setdefault("No_Track_pT_binning", [2.9, 3.5, 3.6, 3.8, 4.0, 4.4, 6.1])

    kwargs.setdefault("isRun3", flags.GeoModel.Run is LHCPeriod.Run3)

    cfg.setPrivateTools(CompFactory.KinematicFitTool(name, **kwargs))
    return cfg


def KinematicFitTool_bbyy_Cfg(flags, name="KinematicFitTool_bbyy", **kwargs):
    """Configure KinematicFitTool for bbyy analysis with sensible defaults"""
    cfg = ComponentAccumulator()

    # Set default parameters for bbyy mode
    kwargs.setdefault("mode", "bbyy")
    kwargs.setdefault("JetCollection", "AntiKt4EMPFlow")
    kwargs.setdefault("JetMinPt", 25. * Units.GeV)

    # Set lambda parameters for bbyy
    kwargs.setdefault("LambdaMomConstraint", 1.15)
    kwargs.setdefault("LambdaMassConstraint", 0.07)

    # Default binning parameters
    kwargs.setdefault("log_pt_binning", [2.0, 3.7, 4.0, 4.5, 5.0, 5.3, 6.0])

    # Get calibration filenames
    jet_algo = kwargs.get("JetCollection", "AntiKt4EMPFlow")
    data_taking_period = "Run3" if flags.GeoModel.Run is LHCPeriod.Run3 else "Run2"
    calib_files = get_filenames_bbyy(jet_algo, data_taking_period)
    for key, filename in calib_files.items():
        kwargs.setdefault(key, filename)

    kwargs.setdefault("isRun3", flags.GeoModel.Run is LHCPeriod.Run3)

    cfg.setPrivateTools(CompFactory.KinematicFitTool(name, **kwargs))
    return cfg
