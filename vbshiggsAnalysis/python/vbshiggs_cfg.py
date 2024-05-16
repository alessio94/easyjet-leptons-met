from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from vbshiggsAnalysis.fullLep_config import fullLep_cfg
from vbshiggsAnalysis.fullLep_config import fullLep_branches

from vbshiggsAnalysis.semiLep_config import semiLep_cfg
from vbshiggsAnalysis.semiLep_config import semiLep_branches

from vbshiggsAnalysis.fullHad_config import fullHad_cfg
from vbshiggsAnalysis.fullHad_config import fullHad_branches

from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units


def vbshiggs_cfg(flags, smalljetkey, muonkey, electronkey):

    cfg = ComponentAccumulator()

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=MuonWPLabel + muonkey,
            containerOutKey="vbshiggsAnalysisMuons_%SYS%",
            muon_WP=MuonWPLabel,
            isMC=flags.Input.isMC,
            checkOR=flags.Analysis.do_overlap_removal,
            minPt=9 * Units.GeV,
        )
    )

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=ElectronWPLabel + electronkey,
            containerOutKey="vbshiggsAnalysisElectrons_%SYS%",
            checkOR=flags.Analysis.do_overlap_removal,
            ele_WP=ElectronWPLabel,
            isMC=flags.Input.isMC,
            minPt=9 * Units.GeV,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallJetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="vbshiggsAnalysisJets_%SYS%",
            bTagWPDecorName="",
            minPt=20 * Units.GeV,
            minimumAmount=2,  # -1 means ignores this
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    if flags.Analysis.Channel == "FullLep":
        extra_vbshiggs_branches, float_variable_names, \
            int_variable_names = fullLep_branches(flags)
        cfg.merge(
            fullLep_cfg(
                flags,
                float_variables=float_variable_names,
                int_variables=int_variable_names)
        )

    if flags.Analysis.Channel == "SemiLep":
        extra_vbshiggs_branches, float_variable_names, \
            int_variable_names = semiLep_branches(flags)
        cfg.merge(
            semiLep_cfg(
                flags,
                float_variables=float_variable_names,
                int_variables=int_variable_names)
        )

    if flags.Analysis.Channel == "FullHad":
        extra_vbshiggs_branches, float_variable_names, int_variable_names \
            = fullHad_branches(flags)
        cfg.merge(
            fullHad_cfg(
                flags,
                float_variables=float_variable_names,
                int_variables=int_variable_names)
        )
    return cfg
