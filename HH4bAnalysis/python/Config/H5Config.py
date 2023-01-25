from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def getH5Cfg(flags):
    ca = ComponentAccumulator()
    output = CompFactory.H5FileSvc(path='out.h5')
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
            output=output
        )
    )
    return ca
