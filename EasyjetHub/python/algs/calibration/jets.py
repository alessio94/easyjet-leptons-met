from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory

from BJetCalibrationTool.BJetPtCorrectionConfig import makeBJetPtCalibrationConfig

from EasyjetHub.algs.calibration.FTagEventSFConfig import makeFTagEventSFConfig
from EasyjetHub.steering.utils.name_helper import drop_sys


def jet_sequence(
    flags,
    configAcc,
):

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig
    jet_flags = flags.Analysis.small_R_jet

    # We define the basic sequence to produce all calibrated jets
    jet_type = jet_flags.jet_type

    # Need to keep DAOD_PHYS collection name regardless of input
    # due to CP algs configs in Athena
    jetColl = (
        "AntiKt4EMPFlowJets"
        if jet_type == "reco4PFlowJet"
        else "AntiKt4EMTopoJets"
    )

    calib_name = drop_sys(flags.Analysis.container_names.allcalib[jet_type])

    configSeq += makeConfig("Jets", containerName=calib_name,
                            jetCollection=jetColl)
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
        configSeq += makeConfig('Jets.JVT', containerName=calib_name)

    btag_wps = [jet_flags.btag_wp]
    if 'btag_extra_wps' in jet_flags:
        btag_wps += jet_flags.btag_extra_wps

    if jet_type != "reco4EMTopoJet":

        for tagger_wp in btag_wps:
            tagger, btag_wp = tagger_wp.split("_", 1)
            configSeq += makeConfig('Jets.FlavourTagging',
                                    containerName=calib_name,
                                    selectionName=tagger_wp)
            configSeq.setOptionValue('.btagger', tagger)
            # set the MC/MC SF to default for now, this was broken by
            # https://gitlab.cern.ch/atlas/athena/-/merge_requests/66729
            configSeq.setOptionValue('.generator', 'default')
            configSeq.setOptionValue('.btagWP', btag_wp)
            configSeq.setOptionValue('.kinematicSelection', True)

            # Keep using the Easyjet version for now, while the Athena
            # config for globalSF is being adjusted
            configSeq.setOptionValue('.globalSF', False)

            if 'btagCDI' in jet_flags:
                configSeq.setOptionValue(
                    '.bTagCalibFile',
                    jet_flags.btagCDI
                )
            # if GN2 in tagger name overwrite the CDI
            if "GN2v00" in tagger:
                configSeq.setOptionValue(
                    '.bTagCalibFile',
                    'xAODBTaggingEfficiency/13p6TeV/'
                    '2023-22-13p6TeV-MC21-CDI_Test_2023-08-1_v1.root'
                ) # noqa
            elif "GN2v01" in tagger:
                configSeq.setOptionValue(
                    '.bTagCalibFile',
                    'xAODBTaggingEfficiency/13p6TeV/'
                    '2023-02_MC23_CDI_GN2v01-noSF.root'
                ) # noqa
                configSeq.setOptionValue('.noEffSF', True)

        # Run this by default, but will fail if muon and btag calib sequences not run
        # TODO: Add a toggle?
        if flags.Analysis.do_muons:
            # Pick a reasonable b-tag selection?
            makeBJetPtCalibrationConfig(
                configSeq,
                calib_name,
            )
            configSeq.setOptionValue(
                '.muonName',
                flags.Analysis.container_names.output.muons
            )
            configSeq.setOptionValue(
                '.btagSelDecor',
                "ftag_select_" + jet_flags.btag_wp,
            )

    # Apply kinematic selection
    configSeq += makeConfig('Jets.PtEtaSelection',
                            containerName=calib_name)
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', 20e3)
    configSeq.setOptionValue('.maxEta', jet_flags.max_eta)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', containerName=calib_name)

    output_name = drop_sys(flags.Analysis.container_names.output[jet_type])
    configSeq += makeConfig('Thinning', containerName=calib_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta&&jvt')
    configSeq.setOptionValue('.outputName', output_name)

    # Event-level FTAG scale factor
    if jet_type != "reco4EMTopoJet":

        for tagger_wp in btag_wps:
            tagger, btag_wp = tagger_wp.split("_", 1)

            if "GN2v01" in tagger:
                continue

            makeFTagEventSFConfig(
                configSeq,
                flags.Analysis.container_names.output[jet_type],
                tagger_wp,
            )
            configSeq.setOptionValue('.btagger', tagger)
            configSeq.setOptionValue('.btagWP', btag_wp)

    return configSeq


# lr = large-R
def lr_jet_sequence(flags, lr_jet_type, configAcc):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    input_name = flags.Analysis.container_names.input[
        f"reco10{lr_jet_type}Jet"]
    output_name = flags.Analysis.container_names.output[
        f"reco10{lr_jet_type}Jet"].replace('_%SYS%', '')
    configSeq += makeConfig('Jets', containerName=output_name,
                            jetCollection=input_name)
    configSeq.setOptionValue('.postfix', f'largeR_{lr_jet_type}jets')

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', containerName=output_name)

    configSeq += makeConfig('Thinning', containerName=output_name)

    return configSeq


# vr = variable R
def vr_jet_sequence(flags, configAcc):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    # There is no output container, we just operate on the input one
    input_name = flags.Analysis.container_names.input.vrJet
    for tagger_wp in flags.Analysis.large_R_jet.vr_btag_wps:
        tagger, btag_wp = tagger_wp.split("_", 1)
        # Default CDI in FTag config which is:
        #   "xAODBTaggingEfficiency/13TeV/2022-22-13TeV-MC20-CDI-2022-07-28_v1.root"
        # supports only DL1dv00 and GN2 in PFlow jets, for testing
        # minPt defaults to 10 GeV for VR
        # kinematic selection is on by default
        configSeq += makeConfig('FlavourTagging', containerName=input_name,
                                selectionName=tagger_wp)
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
