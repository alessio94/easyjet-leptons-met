from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from HH4bAnalysis.utils.containerNameHelper import get_container_names


def getH5Cfg(flags):
    ca = ComponentAccumulator()
    output = CompFactory.H5FileSvc(path=flags.Analysis.h5_output.as_posix())
    ca.addService(output)
    ca.addEventAlgo(
        CompFactory.EventInfoWriterAlg(
            'infowriter',
            primitives=[
                'eventNumber',
                'lumiBlock',
                'averageInteractionsPerCrossing',
                'actualInteractionsPerCrossing'
            ],
            primitiveToType={
                'eventNumber':'ULL',
                'mcEventNumber':'ULL',
                'lumiBlock':'UINT',
                'averageInteractionsPerCrossing':'HALF',
                'actualInteractionsPerCrossing':'HALF'
            },
            datasetName='event',
            output=output
        )
    )
    jetcol = get_container_names(flags)['outputs']['reco4Jet']
    types = {'valid':'CUSTOM'}
    associations = {}
    kinematics = ['ptGeV', 'eta', 'phi', 'massGeV']
    types |= {x: 'CUSTOM' for x in kinematics}
    btagging = [f'DL1dv00_p{x}' for x in 'cub']
    types |= {x: 'HALF' for x in btagging}
    associations = {x:'btaggingLink' for x in btagging}
    primitives = ['valid'] + kinematics + btagging
    ca.addEventAlgo(
        CompFactory.IParticleWriterAlg(
            'jetwriter',
            primitives=primitives,
            primitiveToType=types,
            primitiveToAssociation=associations,
            datasetName='jets',
            maximumSize=6,
            container=jetcol.replace("_%SYS%", "_NOSYS"),
            output=output
        )
    )
    return ca
