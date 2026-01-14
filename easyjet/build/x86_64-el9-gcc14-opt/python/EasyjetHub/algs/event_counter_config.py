from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from pathlib import Path

METADATA_FILE = Path("userJobMetadata.json")


def event_counter_cfg(step, filename=METADATA_FILE, alg_name=None):
    ca = ComponentAccumulator()
    ca.addEventAlgo(
        CompFactory.EventCounterAlg(
            name=(alg_name or f"EventCounterAlg_{step}"),
            countName=step,
            output=filename.as_posix(),
        )
    )
    return ca


# Define a ConfigBlocks version so that we can integrate this easily
# with CP alg preselection configuration
class EventCounterConfig(ConfigBlock):
    def __init__(self, step):
        super(EventCounterConfig, self).__init__()
        self.step = step
        self.addOption('alg_name', '', type=str)
        self.addOption('output', METADATA_FILE, type=None)

    def makeAlgs(self, config):
        alg = config.createAlgorithm(
            'EventCounterAlg',
            self.alg_name or f'EventCounterAlg_{self.step}'
        )
        alg.countName = self.step
        alg.output = self.output.as_posix()


def makeEventCounterConfig(seq, step):
    config = EventCounterConfig(step)
    seq.append(config)
