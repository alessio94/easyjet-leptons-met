from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory

from EasyjetHub.algs.calibration.view_select import makeViewSelectionConfig
from EasyjetHub.steering.utils.name_helper import drop_sys


def electron_sequence(flags, configAcc):

    # Previous configuration, to be reproduced
    #     flags.Analysis.DataType,
    #     workingPoint="LooseLHElectron.NonIso",
    #     postfix="loose",
    #     deepCopyOutput=False,
    #     shallowViewOutput=True,
    #     recomputeLikelihood=False,
    #     chargeIDSelection=False,
    #     isolationCorrection=False,
    #     crackVeto=False,
    #     ptSelectionOutput=True,
    #     enableCutflow=False,
    #     enableKinematicHistograms=False,

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    output_name = drop_sys(flags.Analysis.container_names.output.electrons)
    configSeq += makeConfig('Electrons', containerName=output_name)
    configSeq.setOptionValue('.crackVeto', True)

    # PID configuration
    configSeq += makeConfig('Electrons.WorkingPoint', containerName=output_name,
                            selectionName=ElectronWPLabel)
    configSeq.setOptionValue('.likelihoodWP', flags.Analysis.Electron.ID)
    configSeq.setOptionValue('.isolationWP', flags.Analysis.Electron.Iso)
    configSeq.setOptionValue('.recomputeLikelihood', False)
    if 'extra_wps' in flags.Analysis.Electron:
        for wp in flags.Analysis.Electron.extra_wps:
            id = wp[0]
            iso = wp[1]
            wpLabel = f'{id}_{iso}'
            configSeq += makeConfig('Electrons.WorkingPoint', containerName=output_name,
                                    selectionName=wpLabel)
            configSeq.setOptionValue('.likelihoodWP', id)
            configSeq.setOptionValue('.isolationWP', iso)
            configSeq.setOptionValue('.recomputeLikelihood', False)

    # Kinematic selection
    configSeq += makeConfig('Electrons.PtEtaSelection', containerName=output_name)
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', 4.5e3)
    configSeq.setOptionValue('.maxEta', 2.47)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', containerName=output_name)

    # Apply selection as view container
    makeViewSelectionConfig(configSeq, output_name)
    # Add working point selection
    makeViewSelectionConfig(
        configSeq,
        ElectronWPLabel + output_name,
        input=output_name,
        original=flags.Analysis.container_names.input.electrons,
        selection=ElectronWPLabel
    )

    return configSeq
