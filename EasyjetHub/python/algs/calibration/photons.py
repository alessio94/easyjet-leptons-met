from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import makeConfig

from EasyjetHub.algs.calibration.view_select import makeViewSelectionConfig
from EasyjetHub.steering.utils.name_helper import drop_sys


def photon_sequence(flags, configAcc):

    # Previous configuration, to be reproduced
    #     flags.Analysis.DataType,
    #     workingPoint=flags.Analysis.PhotonWP,
    #     postfix="loose",
    #     deepCopyOutput=False,
    #     shallowViewOutput=True,
    #     crackVeto=False,
    #     enableCleaning=True,
    #     cleaningAllowLate=False,
    #     recomputeIsEM=False,
    #     ptSelectionOutput=True,
    #     enableCutflow=False,
    #     enableKinematicHistograms=False,

    PhotonWPLabel = f'{flags.Analysis.Photon.ID}_{flags.Analysis.Photon.Iso}'

    configSeq = ConfigSequence()

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    output_name = drop_sys(flags.Analysis.container_names.output.photons)
    configSeq += makeConfig('Photons', output_name)
    configSeq.setOptionValue('.recomputeIsEM', False)
    configSeq.setOptionValue('.crackVeto', True)

    # PID configuration
    configSeq += makeConfig('Photons.Selection', output_name + '.' + PhotonWPLabel)
    configSeq.setOptionValue('.qualityWP', flags.Analysis.Photon.ID)
    configSeq.setOptionValue('.isolationWP', flags.Analysis.Photon.Iso)
    if (flags.Analysis.Photon.Iso == "NonIso"):
        configSeq.setOptionValue('.noEffSF', True)

    # Kinematic selection
    configSeq += makeConfig('Selection.PtEta', output_name)
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', 10e3)
    configSeq.setOptionValue('.maxEta', 2.37)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', f'SystObjectLink.{output_name}')

    # Apply selection as view container
    makeViewSelectionConfig(configSeq, output_name)
    # Add working point selection
    makeViewSelectionConfig(
        configSeq,
        PhotonWPLabel + output_name,
        input=output_name,
        original=flags.Analysis.container_names.input.photons,
        selection=PhotonWPLabel
    )

    return configSeq
