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
    jet_flags = flags.Analysis.small_R_jet

    # We define the basic sequence to produce all calibrated jets
    # Filtering on kinematics and JVT is done later
    # We need to make the filtered jet container explicitly different
    # because for MET we need the unfiltered container

    jet_type = jet_flags.jet_type
    allcalib_name = flags.Analysis.container_names.allcalib[jet_type]
    # Need to keep DAOD_PHYS collection name regardless of input
    # due to CP algs configs in Athena
    jetColl = (
        "AntiKt4EMPFlowJets"
        if jet_type == "reco4PFlowJet"
        else "AntiKt4EMTopoJets"
    )

    configSeq += makeConfig("Jets", drop_sys(allcalib_name), jetCollection=jetColl)
    # don't run JVT only for EMTopo jets
    configSeq.setOptionValue(".runNNJvtUpdate", jet_type != "reco4EMTopoJet")
    configSeq.setOptionValue(".runJvtSelection", jet_type != "reco4EMTopoJet")

    # Set options for calibration tool if given
    if jet_flags.calibToolConfigFile and jet_flags.calibToolCalibArea:
        configSeq.setOptionValue(
            ".calibToolConfigFile",
            jet_flags.calibToolConfigFile
        )
        configSeq.setOptionValue(
            ".calibToolCalibArea",
            jet_flags.calibToolCalibArea)
    if jet_flags.calibToolCalibSeq:
        configSeq.setOptionValue(
            ".calibToolCalibSeq",
            jet_flags.calibToolCalibSeq
        )

    # Set options for uncertainties tool if given
    if jet_flags.uncertToolConfigPath and jet_flags.uncertToolCalibArea:
        configSeq.setOptionValue(
            ".uncertToolConfigPath",
            jet_flags.uncertToolConfigPath
        )
        configSeq.setOptionValue(
            ".uncertToolCalibArea",
            jet_flags.uncertToolCalibArea
        )
    if jet_flags.uncertToolMCType:
        configSeq.setOptionValue(
            ".uncertToolMCType",
            jet_flags.uncertToolMCType
        )

    if jet_type != "reco4EMTopoJet":
        configSeq += makeConfig('Jets.Jvt', drop_sys(allcalib_name))

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

    if jet_type != "reco4EMTopoJet":
        btag_wps = [jet_flags.btag_wp]
        if 'btag_extra_wps' in jet_flags:
            btag_wps += jet_flags.btag_extra_wps

        for tagger_wp in btag_wps:
            tagger, btag_wp = tagger_wp.split("_", 1)
            configSeq += makeConfig(
                'FlavourTagging',
                f'{drop_sys(allcalib_name)}.{tagger_wp}'
            )
            configSeq.setOptionValue('.btagger', tagger)
            # set the MC/MC SF to default for now, this was broken by
            # https://gitlab.cern.ch/atlas/athena/-/merge_requests/66729
            configSeq.setOptionValue('.generator', 'default')
            configSeq.setOptionValue('.btagWP', btag_wp)
            configSeq.setOptionValue('.kinematicSelection', True)
            if 'btagCDI' in jet_flags:
                configSeq.setOptionValue(
                    '.bTagCalibFile',
                    jet_flags.btagCDI
                )
            # if GN2 in tagger name overwrite the CDI
            if "GN2" in tagger:
                configSeq.setOptionValue(
                    '.bTagCalibFile',
                    'xAODBTaggingEfficiency/13p6TeV/'
                    '2023-22-13p6TeV-MC21-CDI_Test_2023-08-1_v1.root'
                ) # noqa

        # Run this by default, but will fail if muon and btag calib sequences not run
        # TODO: Add a toggle?
        if flags.Analysis.do_muons:
            # Pick a reasonable b-tag selection?
            makeBJetPtCalibrationConfig(
                configSeq,
                drop_sys(allcalib_name),
            )
            configSeq.setOptionValue(
                '.muonName',
                drop_sys(flags.Analysis.container_names.output.muons),
            )
            configSeq.setOptionValue(
                '.btagSelDecor',
                "ftag_select_" + jet_flags.btag_wp,
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
    configSeq.setOptionValue('.maxEta', jet_flags.max_eta)

    # Apply selection as view container

    # Declare the connections between the allcalib and output containers
    output_name = flags.Analysis.container_names.output[jet_type]
    input_name = flags.Analysis.container_names.input[jet_type]

    makeViewSelectionConfig(
        configSeq,
        drop_sys(output_name),
        input=drop_sys(allcalib_name),
        original=input_name,
        selection='selectPtEta&&jvt',
    )

    return configSeq


# lr = large-R
def lr_jet_sequence(flags, lr_jet_type, configAcc):
    configSeq = ConfigSequence()

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    input_name = flags.Analysis.container_names.input[
        f"reco10{lr_jet_type}Jet"]
    output_name = flags.Analysis.container_names.output[
        f"reco10{lr_jet_type}Jet"].replace('_%SYS%','')
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
    # for tagger_wp in flags.Analysis.large_R_jet.vr_btag_wps:
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
    for tagger_wp in flags.Analysis.large_R_jet.vr_btag_wps:
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
            LargeJetInKey=flags.Analysis.container_names.input[
                f"reco10{lr_jet_type}Jet"].replace("%SYS%", "NOSYS"),
            workingPoints=flags.Analysis.large_R_jet.vr_btag_wps,
            EventInfoDecorSuffix=lr_jet_type,
        )
    )

    return cfg
