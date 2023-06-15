from AthenaCommon import Logging

log = Logging.logging.getLogger("easyjet-ntupler")

_rogue_loggers = [
    "Athena",
    "AutoConfigFlags",
    "MetaReader",
    "makePileupAnalysisSequence",
    "ConfigurableDb",
]


def setRogueLoggers(level):
    for x in _rogue_loggers:
        Logging.logging.getLogger(x).setLevel(level)
