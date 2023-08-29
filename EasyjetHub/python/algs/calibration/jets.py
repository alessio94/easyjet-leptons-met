from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import makeConfig

from BJetCalibrationTool.BJetPtCorrectionConfig import makeBJetPtCalibrationConfig

from EasyjetHub.algs.calibration.view_select import makeViewSelectionConfig
from EasyjetHub.steering.utils.name_helper import drop_sys


def jet_sequence(
    flags,
    configAcc,
):

    configSeq = ConfigSequence()

    # We define the basic sequence to produce all calibrated jets
    # Filtering on kinematics and JVT is done later
    # We need to make the filtered jet container explicitly different
    # because for MET we need the unfiltered container
    allcalib_name = flags.Analysis.container_names.allcalib.reco4PFlowJet

    configSeq += makeConfig(
        'Jets',
        drop_sys(allcalib_name),
        jetCollection='AntiKt4EMPFlowJets'
    )
    configSeq.setOptionValue('.runNNJvtUpdate', True)
    configSeq.setOptionValue('.runJvtSelection', True)

    # jet_sequence = makeJetAnalysisSequence(
    #     flags.Analysis.DataType,
    #     jetCollection=flags.Analysis.container_names.input.reco4PFlowJet,
    #     postfix="smallR",
    #     deepCopyOutput=False,
    #     shallowViewOutput=True,
    #     runGhostMuonAssociation=not flags.Input.isPHYSLITE,
    #     enableCutflow=False,
    #     enableKinematicHistograms=False,
    #     runJvtUpdate=not flags.Input.isPHYSLITE,
    #     runNNJvtUpdate=not flags.Input.isPHYSLITE,
    #     runJvtSelection=not flags.Input.isPHYSLITE,
    # )

    for tagger_wp in flags.Analysis.btag_wps:
        tagger, btag_wp = tagger_wp.split("_", 1)
        configSeq += makeConfig(
            'FlavourTagging',
            f'{drop_sys(allcalib_name)}.{tagger_wp}'
        )
        configSeq.setOptionValue('.btagger', tagger)
        configSeq.setOptionValue('.btagWP', btag_wp)
        configSeq.setOptionValue('.kinematicSelection', True)

        # makeFTagAnalysisSequence(
        #     jet_sequence,
        #     flags.Analysis.DataType,
        #     jetCollection=jet_btag_name,
        #     btagWP=btag_wp,
        #     btagger=tagger,
        #     generator="Pythia8",
        #     minPt=20000,
        #     postfix=f"{jet_btag_name}_{tagger_wp}",
        #     preselection=None,
        #     kinematicSelection=True,
        #     noEfficiency=False,
        #     legacyRecommendations=False,
        #     enableCutflow=False,
        # )

    # Run this by default, but will fail if muon and btag calib sequences not run
    # TODO: Add a toggle?
    if flags.Analysis.do_muons:
        # Pick a reasonable b-tag selection?
        makeBJetPtCalibrationConfig(
            configSeq,
            drop_sys(flags.Analysis.container_names.allcalib.reco4PFlowJet),
        )
        configSeq.setOptionValue(
            '.muonName',
            drop_sys(flags.Analysis.container_names.output.muons),
        )
        configSeq.setOptionValue(
            '.btagSelDecor',
            "ftag_select_DL1dv01_FixedCutBEff_77",
        )

    # Add systematic object links
    configSeq += makeConfig(
        'SystObjectLink',
        f'SystObjectLink.{drop_sys(allcalib_name)}'
    )

    # Apply kinematic selection
    configSeq += makeConfig(
        'Selection.PtEta',
        drop_sys(allcalib_name)
    )
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', 20e3)
    configSeq.setOptionValue('.maxEta', 2.5)

    # Apply selection as view container

    # Declare the connections between the allcalib and output containers
    output_name = flags.Analysis.container_names.output.reco4PFlowJet
    input_name = flags.Analysis.container_names.input.reco4PFlowJet
    makeViewSelectionConfig(
        configSeq,
        drop_sys(output_name),
        input=drop_sys(allcalib_name),
        original=input_name,
        selection='selectPtEta'
        # TODO: Restore JVT when we have 24.2.20
        # selection='selectPtEta&&jvt'
    )

    return configSeq


# lr = large-R
def lr_jet_sequence(flags, lr_jet_type, configAcc):
    configSeq = ConfigSequence()

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    input_name = getattr(
        flags.Analysis.container_names.input,
        f"reco10{lr_jet_type}Jet",
    )
    output_name = getattr(
        flags.Analysis.container_names.output,
        f"reco10{lr_jet_type}Jet",
    ).replace('_%SYS%','')
    configSeq += makeConfig('Jets', output_name, jetCollection=input_name)
    configSeq.setOptionValue('.postfix', f'largeR_{lr_jet_type}jets')

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', f'SystObjectLink.{output_name}')

    # Apply selection as view container
    makeViewSelectionConfig(configSeq, output_name)

    return configSeq


# vr = variable R
def vr_jet_sequence(flags, configAcc):

    # Previous configuration, to be reproduced
    # for tagger_wp in flags.Analysis.vr_btag_wps:
    #     tagger, btag_wp = tagger_wp.split("_", 1)
    #     makeFTagAnalysisSequence(
    #         vr_jet_sequence,
    #         flags.Analysis.DataType,
    #         jetCollection=vr_jet_btag_name,
    #         btagWP=btag_wp,
    #         btagger=tagger,
    #         minPt=10e3,
    #         postfix=btag_wp,
    #         preselection=None,
    #         kinematicSelection=True,
    #         noEfficiency=False,
    #         legacyRecommendations=False,
    #         enableCutflow=False,
    #     )
    #
    # cfg.addSequence(CompFactory.AthSequencer(vr_jet_sequence.getName()))
    # # Hack until this is merged:
    # # https://gitlab.cern.ch/atlas/athena/-/merge_requests/54939]
    # for alg in vr_jet_sequence.getGaudiConfig2Components():
    #     if "FTagSelectionAlg" in alg.getName():
    #         alg.selectionTool.FlvTagCutDefinitionsFileName = btag_calib_file
    #     if "FTagEfficiencyScaleFactorAlg" in alg.getName():
    #         alg.efficiencyTool.ScaleFactorFileName = btag_calib_file

    configSeq = ConfigSequence()

    # There is no output container, we just operate on the input one
    input_name = flags.Analysis.container_names.input.vrJet
    for tagger_wp in flags.Analysis.vr_btag_wps:
        tagger, btag_wp = tagger_wp.split("_", 1)
        # Default CDI in FTag config which is:
        #   "xAODBTaggingEfficiency/13TeV/2022-22-13TeV-MC20-CDI-2022-07-28_v1.root"
        # supports only DL1dv00 and GN2 in PFlow jets, for testing
        # minPt defaults to 10 GeV for VR
        # kinematic selection is on by default
        configSeq += makeConfig('FlavourTagging', f'{input_name}.{tagger_wp}')
        configSeq.setOptionValue('.btagger', tagger)
        configSeq.setOptionValue('.btagWP', btag_wp)
        # Set up CDI compatible with DL1r for VR
        configSeq.setOptionValue('.legacyRecommendations', True)

    # If we don't have a (functionally useless) jet sequence
    # preceding the FTag one, the latter just won't configure
    # unless we force the names like this
    # which we need to do because the ghost VR jets need us to
    # decorate the original VR collection
    configAcc.setSourceName(
        containerName=input_name,
        sourceName=input_name,
        originalName=input_name,
    )

    return configSeq


def lr_jet_ghost_vr_jet_association_cfg(
    flags,
    lr_jet_type,
):
    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.Easyjet.LargeJetGhostVRJetAssociationAlg(
            f"Large{lr_jet_type}JetGhostVRJetAssociationAlg",
            isMC=flags.Input.isMC,
            LargeJetInKey=getattr(
                flags.Analysis.container_names.input,
                f"reco10{lr_jet_type}Jet"
            ).replace("%SYS%", "NOSYS"),
            workingPoints=flags.Analysis.vr_btag_wps,
            EventInfoDecorSuffix=lr_jet_type,
        )
    )

    return cfg
