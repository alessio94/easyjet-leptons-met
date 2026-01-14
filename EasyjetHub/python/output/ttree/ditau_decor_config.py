from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ditau_decor_cfg(flags, **kwargs):
    ditaucoll = flags.Analysis.container_names.input.ditaus

    cfg = ComponentAccumulator()

    if flags.Analysis.DiTau.do_score_decoration:
        cfg.addEventAlgo(
            CompFactory.Easyjet.DiTauDecoratorAlg(
                f"DiTauDecor_{ditaucoll}",
                diTausIn=ditaucoll,
                DiTauIDVarCalculator=CompFactory.DiTauRecTools.DiTauIDVarCalculator(),
                DiTauOnnxDiscriminantTool=(
                    CompFactory.DiTauRecTools.DiTauOnnxDiscriminantTool()),
                **kwargs
            )
        )

    return cfg
