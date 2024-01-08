from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units


def bbtt_cfg(
        flags, smalljetkey, muonkey, electronkey,
        taukey, float_variables=[], int_variables=[]):

    cfg = ComponentAccumulator()

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=MuonWPLabel + muonkey,
            containerOutKey="bbttAnalysisMuons_%SYS%",
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=ElectronWPLabel + electronkey,
            containerOutKey="bbttAnalysisElectrons_%SYS%",
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.TauSelectorAlg(
            "TauSelectorAlg",
            # Baseline always needed for anti-taus
            containerInKey='baseline' + taukey,
            containerOutKey="bbttAnalysisTaus_%SYS%",
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallJetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="bbttAnalysisJets_%SYS%",
            minPt=20 * Units.GeV,
            bTagWPDecorName="",  # empty string: "" ignores btagging
            minimumAmount=2,  # -1 means ignores this
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    # Selection
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in flags.Analysis.TriggerChains
    ]

    TightMuonWP = flags.Analysis.Muon.extra_wps[0]
    TightMuonWPLabel = f'{TightMuonWP[0]}_{TightMuonWP[1]}'
    TightEleWP = flags.Analysis.Electron.extra_wps[0]
    TightEleWPLabel = f'{TightEleWP[0]}_{TightEleWP[1]}'
    cfg.addEventAlgo(
        CompFactory.HHBBTT.HHbbttSelectorAlg(
            "HHbbttSelectorAlg",
            jets="bbttAnalysisJets_%SYS%",
            muons="bbttAnalysisMuons_%SYS%",
            electrons="bbttAnalysisElectrons_%SYS%",
            taus="bbttAnalysisTaus_%SYS%",
            met="AnalysisMET_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            tauWP=flags.Analysis.Tau.ID,
            muonWP=TightMuonWPLabel,
            eleWP=TightEleWPLabel,
            eventDecisionOutputDecoration="bbtt_pass_sr_noMMC_%SYS%",
            triggerLists=trigger_branches,
            channel=flags.Analysis.channel,
            isMC=flags.Input.isMC,
            Years=flags.Analysis.Years,
            bypass=flags.Analysis.bypass,
            saveCutFlow=flags.Analysis.save_bbtt_cutflow,
            cutList=flags.Analysis.CutList,
        )
    )

    # MMC decoration
    if flags.Analysis.do_baseline:
        baseline = "_baseline_"
    else:
        baseline = "_"
    if flags.Analysis.do_mmc:
        cfg.addEventAlgo(
            CompFactory.HHBBTT.MMCDecoratorAlg(
                "MMCDecoratorAlg",
                jets="bbttAnalysisJets_%SYS%",
                muons="bbttAnalysisMuons_%SYS%",
                electrons="bbttAnalysisElectrons_%SYS%",
                taus="bbttAnalysisTaus_%SYS%",
                met="AnalysisMET_%SYS%",
                passSLT="pass" + baseline + "SLT_%SYS%",
                passLTT="pass" + baseline + "LTT_%SYS%",
                passSTT="pass" + baseline + "STT_%SYS%",
                passDTT="pass" + baseline + "DTT_%SYS%",
                channel=flags.Analysis.channel,
            )
        )

        cfg.addEventAlgo(
            CompFactory.HHBBTT.MMCSelectorAlg(
                "MMCSelectorAlg",
                MMC_min=60 * Units.GeV,
                eventDecisionOutputDecoration="bbtt_pass_sr_%SYS%",
                bypass=flags.Analysis.bypass,
            )
        )

    # calculate final bbtt vars
    cfg.addEventAlgo(
        CompFactory.HHBBTT.BaselineVarsbbttAlg(
            "FinalVarsbbttAlg",
            jets="bbttAnalysisJets_%SYS%",
            muons="bbttAnalysisMuons_%SYS%",
            electrons="bbttAnalysisElectrons_%SYS%",
            taus="bbttAnalysisTaus_%SYS%",
            met="AnalysisMET_%SYS%",
            tauWP=flags.Analysis.Tau.ID,
            muonWP=TightMuonWPLabel,
            eleWP=TightEleWPLabel,
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            storeHighLevelVariables=flags.Analysis.store_high_level_variables,
            floatVariableList=float_variables,
            intVariableList=int_variables
        )
    )

    return cfg


def get_BaselineVarsbbttAlg_variables(flags):
    float_variable_names = []

    particles = [
        "Lepton",
        "Leading_Tau",
        "Sublead_Tau",
        "Leading_Bjet",
        "Sublead_Bjet",
    ]

    for particle in particles:
        for var in ["pt", "eta", "phi"]:
            float_variable_names.append(f"{particle}_{var}")

    float_variable_names += ["Leading_Tau_effSF", "Sublead_Tau_effSF",
                             "Lepton_SF"]

    int_variable_names = [
        "Lepton_charge",
        "Lepton_pdgid",
        "Leading_Tau_charge",
        "Sublead_Tau_charge",
    ]

    if flags.Analysis.do_mmc:
        combined_particles = [
            "H_bb",
            "H_vis_tautau",
            "HH",
            "HH_vis",
        ]

        for particle in combined_particles:
            for var in ["pt", "eta", "phi", "m"]:
                float_variable_names.append(f"{particle}_{var}")

    return float_variable_names, int_variable_names


def get_BaselineVarsbbttAlg_highlevelvariables(flags):
    high_level_float_variables = []
    high_level_int_variables = []

    if flags.Analysis.do_mmc:
        high_level_float_variables += [
            "HH_delta_phi",
            "HH_vis_delta_phi",
        ]
    return high_level_float_variables, high_level_int_variables


def bbtt_branches(flags):
    # a list of strings which maps the variable name in the c++ code
    # to the outputname:
    # "EventInfo.<var> -> <outputvarname>"
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsbbttAlg algorithm
    all_baseline_variable_names = []

    # these are the variables that will always be stored by easyjet
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    float_variable_names, int_variable_names = get_BaselineVarsbbttAlg_variables(flags)

    if flags.Analysis.do_mmc:
        # do not append mmc variables to float_variable_names
        # or int_variable_names as they are stored by the
        # mmc algortithm not BaselineVarsbbttAlg
        for var in ["status", "pt", "eta", "phi", "m"]:
            all_baseline_variable_names.append(f"mmc_{var}")

    if flags.Analysis.store_high_level_variables:
        high_level_float_variables, high_level_int_variables \
            = get_BaselineVarsbbttAlg_highlevelvariables(flags)
        float_variable_names += high_level_float_variables
        int_variable_names += high_level_int_variables

    all_baseline_variable_names += float_variable_names
    all_baseline_variable_names += int_variable_names

    for tree_flags in flags.Analysis.ttree_output:
        for var in all_baseline_variable_names:
            if tree_flags['write_object_systs_only_for_pt'] and \
               "pt" not in var and "SF" not in var:
                branches += [f"EventInfo.{var}_NOSYS -> bbtt_{var}"]
            else:
                branches += [f"EventInfo.{var}_%SYS% -> bbtt_{var}_%SYS%"]

    branches += ["EventInfo.bbtt_pass_sr_%SYS% -> bbtt_pass_SR_%SYS%"]

    if flags.Input.isMC:
        branches += ["EventInfo.ftag_effSF_"
                     f"{flags.Analysis.small_R_jet.btag_wp}_%SYS%"
                     " -> weight_ftag_effSF_"
                     f"{flags.Analysis.small_R_jet.btag_wp}_%SYS%",]

        # jvt is effSF is now centrally calculated by CP tools
        branches += ["EventInfo.jvt_effSF_%SYS% -> weight_jvt_effSF_%SYS%"]

    # trigger variables do not need to be added to variable_names
    # as it is written out in HHbbttSelectorAlg
    for var in ["_trigger_", "_baseline_", "_"]:
        for cat in ["SLT", "LTT", "STT", "DTT",
                    "DTT_2016", "DTT_4J12", "DTT_L1Topo"]:
            branches += [f"EventInfo.pass{var}{cat}_%SYS% -> bbtt_pass{var}{cat}_%SYS%"]

    return branches,float_variable_names,int_variable_names
