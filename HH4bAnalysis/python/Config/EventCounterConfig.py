from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from pathlib import Path

METADATA_FILE = Path('userJobMetadata.json')


def eventCounterCfg(step, filename=METADATA_FILE, alg_name=None):
    ca = ComponentAccumulator()
    ca.addEventAlgo(
        CompFactory.EventCounterAlg(
            name=(alg_name or f'EventCounterAlg_{step}'),
            countName=step,
            output=filename.as_posix(),
        )
    )
    return ca
