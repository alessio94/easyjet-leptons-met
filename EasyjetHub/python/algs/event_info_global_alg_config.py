from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def event_info_global_alg_cfg(flags, **kwargs):
    ca = ComponentAccumulator()

    kwargs.setdefault("isMC", flags.Input.isMC)
    kwargs.setdefault("Years", flags.Analysis.Years)
    kwargs.setdefault("doRetrievePV", flags.Analysis.do_primary_vertex_pos)

    ca.addEventAlgo(CompFactory.Easyjet.EventInfoGlobalAlg(**kwargs))
    return ca
