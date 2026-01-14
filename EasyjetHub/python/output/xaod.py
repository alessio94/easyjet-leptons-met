from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg


def container(typename, key, items=None):
    if items is None:
        items = []
    itemstring = '.'.join(items)
    return [
        f'xAOD::{typename}Container#{key}',
        f'xAOD::{typename}AuxContainer#{key}Aux.{itemstring}',
        f'xAOD::AuxContainerBase#{key}Aux.{itemstring}',
    ]


def get_xaod_cfg(flags, seqname):
    ca = ComponentAccumulator()
    # first we make copies of the jet collection, because reasons
    jetcol = flags.Analysis.container_names.input[flags.Analysis.Small_R_jet.jet_type]
    AcceptAlgs = [seqname]
    ca.addEventAlgo(CompFactory.Easyjet.JetDeepCopyAlg(
        name="jetdeepcopy",
        jetsIn=jetcol.replace("%SYS%", "NOSYS"),
        jetsOut="EasyJets",
    ))
    item_list = [
        "xAOD::EventInfo#EventInfo",
        "xAOD::AuxInfoBase#EventInfoAux."
    ] + container("Jet", "EasyJets")
    ca.merge(OutputStreamCfg(
        flags,
        "AOD",
        ItemList=item_list,
        AcceptAlgs=AcceptAlgs,
    ))
    ca.merge(SetupMetaDataForStreamCfg(
        flags,
        streamName="AOD",
        AcceptAlgs=AcceptAlgs
    ))
    return ca
