# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory
from AthenaConfiguration.Enums import LHCPeriod
from AthenaCommon.Utils.unixtools import find_datafile

from EasyjetHub.steering.utils.name_helper import drop_sys
from EasyjetHub.steering.analysis_configuration import get_trigger_chains_scale_factor


def jet_sequence(
    flags,
    configAcc,
):

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig
    jet_flags = flags.Analysis.Small_R_jet

    # We define the basic sequence to produce all calibrated jets
    jet_type = jet_flags.jet_type

    # Need to keep DAOD_PHYS collection name regardless of input
    # due to CP algs configs in Athena
    jetColl = (
        "AntiKt4EMPFlowJets"
        if jet_type == "reco4PFlowJet"
        else "AntiKt4EMTopoJets"
    )

    output_name = drop_sys(flags.Analysis.container_names.output[jet_type])

    configSeq += makeConfig("Jets", containerName=output_name,
                            jetCollection=jetColl)
    # don't run JVT only for EMTopo jets
    configSeq.setOptionValue(".runNNJvtUpdate", jet_type != "reco4EMTopoJet")
    configSeq.setOptionValue(".runJvtSelection", jet_type != "reco4EMTopoJet")
    # Forward JVT
    configSeq.setOptionValue(".runFJvtSelection", jet_flags.useFJvt)
    configSeq.setOptionValue(".runFJvtEfficiency", jet_flags.useFJvt)
    # JES/JER scheme
    configSeq.setOptionValue(".systematicsModelJES", jet_flags.systModelJES)
    configSeq.setOptionValue(".systematicsModelJER", jet_flags.systModelJER)

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

    # Do not recalibrate PHYSLITE samples yet due to missing variables.
    if flags.Input.isPHYSLITE and flags.GeoModel.Run is LHCPeriod.Run3:
        print("WARNING! Event shape variables not present in mc23 PHYSLITE.")
        print("Jets won't be recalibrated.")
        print("Please check the JET/Etmiss twiki for more info:")
        print("https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/ApplyJetCalibrationR22#R_0_4_particle_flow_jet_cali_AN1")  # noqa: E501
        configSeq.setOptionValue(
            ".recalibratePhyslite",
            False
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
        configSeq += makeConfig('Jets.JVT')
        configSeq.setOptionValue('.containerName', output_name)
        configSeq.setOptionValue('.enableFJvt', jet_flags.useFJvt)

        btag_wps = []
        if jet_flags.btag_wp != "":
            btag_wps += [jet_flags.btag_wp]
        if 'btag_extra_wps' in jet_flags:
            btag_wps += jet_flags.btag_extra_wps

        # Make sure PCBT is scheduled to get SF
        if btag_wps and "GN2v01_Continuous" not in btag_wps:
            btag_wps += ["GN2v01_Continuous"]

        tagger_set = set()

        for tagger_wp in btag_wps:
            tagger, btag_wp = tagger_wp.split("_", 1)
            configSeq += makeConfig('Jets.FlavourTagging')
            configSeq.setOptionValue('.containerName', output_name)
            configSeq.setOptionValue('.selectionName', tagger_wp)
            configSeq.setOptionValue('.btagger', tagger)
            configSeq.setOptionValue('.btagWP', btag_wp)
            # save pb / pc / pu / ptau
            if tagger not in tagger_set:
                tree_flags = flags.Analysis.ttree_output
                if tree_flags.collection_options.small_R_jets.btag_details:
                    configSeq.setOptionValue('.saveScores', 'All')
                tagger_set.add(tagger)

            bTagCalibFile = None
            if 'btagCDI' in jet_flags:
                bTagCalibFile = jet_flags.btagCDI

            if tagger_wp == "GN2v01_Continuous2D":
                from AthenaCommon.Utils.unixtools import find_datafile
                bTagCalibFile = find_datafile(
                    'EasyjetHub/2023-22-13p6TeV-MC21-CDI_GN2v01_Test_2024-07-ctag_noSF_NewCutValues_fTau_Ctag.root')  # noqa

            if bTagCalibFile:
                configSeq.setOptionValue('.bTagCalibFile', bTagCalibFile)

        for tagger in tagger_set:
            tagger_wp = tagger + "_Continuous"
            # Note: this is going to run post overlap removal
            configSeq += config.makeConfig('Jets.FlavourTaggingEventSF')
            configSeq.setOptionValue('.containerName', output_name + '.baselineJvt')
            configSeq.setOptionValue('.selectionName', tagger_wp)
            configSeq.setOptionValue('.btagger', tagger)
            # set the MC/MC SF to default for now, this was broken by
            # https://gitlab.cern.ch/atlas/athena/-/merge_requests/66729
            configSeq.setOptionValue('.generator', 'default')
            configSeq.setOptionValue('.eigenvectorReductionB',
                                     jet_flags.btag_egReductionB)
            configSeq.setOptionValue('.eigenvectorReductionC',
                                     jet_flags.btag_egReductionC)
            configSeq.setOptionValue('.eigenvectorReductionLight',
                                     jet_flags.btag_egReductionLight)

            bTagCalibFile = None
            if 'btagCDI' in jet_flags:
                bTagCalibFile = jet_flags.btagCDI

            if bTagCalibFile:
                configSeq.setOptionValue('.bTagCalibFile', bTagCalibFile)

            trigSF_flags = flags.Analysis.Trigger.scale_factor
            if trigSF_flags.doSF and hasattr(trigSF_flags, 'bjet'):
                bTagCalibTriggerFile = None
                if 'btagTriggerCDI' in trigSF_flags.bjet:
                    bTagCalibTriggerFile = trigSF_flags.bjet.btagTriggerCDI

                if bTagCalibTriggerFile:
                    configSeq.setOptionValue('.bTagCalibTriggerFile',
                                             bTagCalibTriggerFile)

                configSeq.setOptionValue('.triggerChainsPerYear',
                                         get_trigger_chains_scale_factor(flags, 'bjet'))
                configSeq.setOptionValue('.removeHLTPrefix', False)

        if jet_flags.runBJetPtCalib:
            configSeq += makeConfig('Jets.BJetCalib')
            configSeq.setOptionValue('.containerName', output_name)
            configSeq.setOptionValue('.muonContainerName', drop_sys(
                flags.Analysis.container_names.output.muons))
            configSeq.setOptionValue('.jetPreselection', jet_flags.btag_wp)
            configSeq.setOptionValue('.muonPreselection', "forBJetCalib")

    # Apply kinematic selection
    configSeq += makeConfig('Jets.PtEtaSelection')
    configSeq.setOptionValue('.containerName', output_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta')
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', jet_flags.min_pT)
    configSeq.setOptionValue('.maxEta', jet_flags.max_eta)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink')
    configSeq.setOptionValue('.containerName', output_name)

    return configSeq


# lr = large-R
def lr_jet_sequence(flags, lr_jet_type, configAcc):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig
    jet_flags = flags.Analysis.Large_R_jet

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    output_name = drop_sys(
        flags.Analysis.container_names.output[f"reco10{lr_jet_type}Jet"]
    )

    # Need to keep DAOD_PHYS collection name regardless of input
    # due to CP algs configs in Athena
    jetColl = "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"

    configSeq += makeConfig('Jets', containerName=output_name,
                            jetCollection=jetColl)
    configSeq.setOptionValue('.containerName', output_name)
    configSeq.setOptionValue('.jetCollection', jetColl)

    for GN2X_wp in flags.Analysis.Large_R_jet.GN2X_hbb_wps:
        jSONCalibFile = find_datafile(
            "EasyjetHub/Xbb_lookup_table_prelim_Oct30_2024.json")
        configSeq += config.makeConfig('Jets.XbbTagging')
        configSeq.setOptionValue('.containerName', output_name)
        # no eff SF provided for the moment
        configSeq.setOptionValue('.noEffSF', True)
        configSeq.setOptionValue('.calibFile', jSONCalibFile)
        configSeq.setOptionValue('.Xbbtagger', "GN2Xv01")
        configSeq.setOptionValue('.XbbWP', GN2X_wp)

    # Optional muon-in-jet correction for large-R jets
    if flags.Analysis.Large_R_jet.runMuonJetPtCorr:
        configSeq += makeConfig('Jets.BJetCalib')
        configSeq.setOptionValue('.containerName', output_name)
        configSeq.setOptionValue('.muonContainerName', drop_sys(
            flags.Analysis.container_names.output.muons))
        configSeq.setOptionValue('.muonPreselection', "forBJetCalib")
        # Disable small-R b-jet pT reco
        configSeq.setOptionValue('.doPtCorr', False)

    configSeq += makeConfig('Jets.PtEtaSelection')
    configSeq.setOptionValue('.containerName', output_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta')
    configSeq.setOptionValue('.minPt', jet_flags.min_pT)
    if jet_flags.max_pT > 0:
        configSeq.setOptionValue('.maxPt', jet_flags.max_pT)
    configSeq.setOptionValue('.maxEta', jet_flags.max_eta)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink')
    configSeq.setOptionValue('.containerName', output_name)

    return configSeq


def rc_jet_sequence(flags, configAcc):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    input_name = flags.Analysis.container_names.input["reco10RCJet"]
    output_name = drop_sys(
        flags.Analysis.container_names.output["reco10RCJet"]
    )
    configSeq += makeConfig('ReclusteredJetCalibration')
    configSeq.setOptionValue('.containerName', output_name)
    configSeq.setOptionValue('.jetCollection', input_name)
    configSeq.setOptionValue('.jetInput', drop_sys(
        flags.Analysis.container_names.output["reco4EMTopoJet"]))

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink')
    configSeq.setOptionValue('.containerName', output_name)

    return configSeq
