from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import makeConfig

from EasyjetHub.algs.calibration.view_select import makeViewSelectionConfig
from EasyjetHub.steering.utils.name_helper import drop_sys


def muon_sequence(flags, configAcc):

    # Previous configuration, to be reproduced
    #     flags.Analysis.DataType,
    #     workingPoint="Loose.NonIso",
    #     postfix="loose",
    #     deepCopyOutput=False,
    #     shallowViewOutput=True,
    #     ptSelectionOutput=True,
    #     qualitySelectionOutput=True,
    #     enableCutflow=False,
    #     enableKinematicHistograms=False,
    #     isRun3Geo=(flags.Analysis.Run == 3),

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'

    configSeq = ConfigSequence()

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    output_name = drop_sys(flags.Analysis.container_names.output.muons)
    configSeq += makeConfig('Muons', output_name)

    # PID configuration
    configSeq += makeConfig('Muons.Selection', output_name + '.' + MuonWPLabel)
    configSeq.setOptionValue('.quality', flags.Analysis.Muon.ID)
    configSeq.setOptionValue('.isolation', flags.Analysis.Muon.Iso)
    if 'extra_wps' in flags.Analysis.Muon:
        for wp in flags.Analysis.Muon.extra_wps:
            id = wp[0]
            iso = wp[1]
            wpLabel = f'{id}_{iso}'
            configSeq += makeConfig('Muons.Selection',
                                    output_name + '.' + wpLabel)
            configSeq.setOptionValue('.quality', id)
            configSeq.setOptionValue('.isolation', iso)

    # TODO: MCP should restore this when the recommendations for Tight WP exist in R23
    # configSeq += makeConfig('Muons.Selection', output_name + '.tight')
    # configSeq.setOptionValue('.quality', 'Tight')
    # configSeq.setOptionValue('.isolation', 'Loose_VarRad')

    # Kinematic selection
    configSeq += makeConfig('Selection.PtEta', output_name)
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', 3e3)
    configSeq.setOptionValue('.maxEta', 2.7)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', f'SystObjectLink.{output_name}')

    # Apply kinematic selection as view container
    makeViewSelectionConfig(configSeq, output_name)

    # Add working point selection
    makeViewSelectionConfig(
        configSeq,
        MuonWPLabel + output_name,
        input=output_name,
        original=flags.Analysis.container_names.input.muons,
        selection=MuonWPLabel
    )

    return configSeq
