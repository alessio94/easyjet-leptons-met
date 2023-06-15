from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.steering.sample_metadata import SampleTypes
from EasyjetHub.steering.utils.log_helper import log


def pileup_sequence_cfg(flags, prwfiles, lumicalcfiles):
    cfg = ComponentAccumulator()
    from AsgAnalysisAlgorithms.PileupAnalysisSequence import makePileupAnalysisSequence

    tags = flags.Input.AMITag
    mc21mc23 = (SampleTypes.mc21a.value in tags) or (SampleTypes.mc23a.value in tags)
    pileup_sequence = makePileupAnalysisSequence(
        flags.Analysis.DataType,
        files=flags.Input.Files,
        useDefaultConfig=mc21mc23,
    )
    pileup_sequence.configure(inputName={}, outputName={})

    cfg.addSequence(CompFactory.AthSequencer(pileup_sequence.getName()))
    for alg in pileup_sequence.getGaudiConfig2Components():
        # Workaround for mc21 courtesy of
        # https://its.cern.ch/jira/browse/ATLASG-1628?focusedCommentId=4297949&page=com.atlassian.jira.plugin.system.issuetabpanels%3Acomment-tabpanel#comment-4297949
        if mc21mc23 and "PileupReweightingAlg" in alg.getName():
            alg.pileupReweightingTool.PeriodAssignments = []
            alg.pileupReweightingTool.DataScaleFactor = 1
        cfg.addEventAlgo(alg, pileup_sequence.getName())

    return cfg


def generator_sequence_cfg(flags):
    cfg = ComponentAccumulator()
    from AsgAnalysisAlgorithms.GeneratorAnalysisSequence import (
        makeGeneratorAnalysisSequence,
    )

    tags = flags.Input.AMITag.split("_")
    ptag = ""
    for tag in reversed(tags):
        if tag.startswith("p"):
            ptag = tag
            break
    if not ptag:
        log.warning(f"Did not find p-tag in AMI tags: {flags.Input.AMITag}")

    # we have to disable cutbookkeepers if there's no output file, or
    # if we're looking at one of several broken tags
    is_bad_tag = ptag in ["p5226", "p5278", "p5334"]
    doCBK = not is_bad_tag and flags.Analysis.out_file
    generator_sequence = makeGeneratorAnalysisSequence(
        flags.Analysis.DataType,
        saveCutBookkeepers=doCBK,
        runNumber=flags.Input.RunNumber[0],
        cutBookkeepersSystematics=doCBK,
    )

    cfg.addSequence(CompFactory.AthSequencer(generator_sequence.getName()))
    for alg in generator_sequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, generator_sequence.getName())

    return cfg
