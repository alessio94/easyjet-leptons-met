from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import makeConfig

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

    configSeq = ConfigSequence()

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    output_name = drop_sys(flags.Analysis.container_names.output.electrons)
    configSeq += makeConfig('Electrons', output_name)

    # PID configuration
    configSeq += makeConfig('Electrons.Selection', output_name + '.loose')
    configSeq.setOptionValue('.likelihoodWP', 'LooseBLayerLH')
    configSeq.setOptionValue('.isolationWP', 'NonIso')
    configSeq.setOptionValue('.recomputeLikelihood', False)

    # Kinematic selection
    configSeq += makeConfig('Selection.PtEta', output_name)
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', 4.5e3)
    configSeq.setOptionValue('.maxEta', 2.47)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', f'SystObjectLink.{output_name}')

    # Apply selection as view container
    makeViewSelectionConfig(configSeq, output_name)
    # Add working point selection
    makeViewSelectionConfig(
        configSeq,
        'loose' + output_name,
        input=output_name,
        original=flags.Analysis.container_names.input.electrons,
        selection='loose'
    )

    return configSeq
