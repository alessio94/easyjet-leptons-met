from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from EasyjetHub.steering.container_names import get_container_names


def container(typename, key, items=[]):
    itemstring = '.'.join(items)
    return [
        f'xAOD::{typename}Container#{key}',
        f'xAOD::{typename}AuxContainer#{key}Aux.{itemstring}',
        f'xAOD::AuxContainerBase#{key}Aux.{itemstring}',
    ]


def get_xaod_cfg(flags):
    ca = ComponentAccumulator()
    # first we make copies of the jet collection, because reasons
    jetcol = get_container_names(flags)["inputs"]["reco4PFlowJet"]
    ca.addEventAlgo(CompFactory.Easyjet.JetDeepCopyAlg(
        name="jetdeepcopy",
        jetsIn=jetcol.replace("%SYS%","NOSYS"),
        jetsOut="EasyJets",
    ))
    item_list = [
        "xAOD::EventInfo#EventInfo",
        "xAOD::AuxInfoBase#EventInfoAux."
    ] + container("Jet", "EasyJets")
    ca.merge(OutputStreamCfg(flags, "AOD", ItemList=item_list))
    return ca
