from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory
from EasyjetHub.steering.sample_metadata import get_prw_files, get_lumicalc_files
from EasyjetHub.steering.utils.log_helper import log


def pileup_sequence(flags):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    # Workaround for mc21 courtesy of
    # https://its.cern.ch/jira/browse/ATLASG-1628?focusedCommentId=4297949&page=com.atlassian.jira.plugin.system.issuetabpanels%3Acomment-tabpanel#comment-4297949
    """
    Check if we still need this. Not so trivial to hack in ConfigBlock
    tags = flags.Input.AMITag
    mc21mc23 = (SampleTypes.mc21a.value in tags) or(SampleTypes.mc23a.value in tags)
    for alg in pileup_sequence.getGaudiConfig2Components():
        if mc21mc23 and "PileupReweightingAlg" in alg.getName():
            alg.pileupReweightingTool.PeriodAssignments = []
            alg.pileupReweightingTool.DataScaleFactor = 1
        cfg.addEventAlgo(alg, pileup_sequence.getName())
    """

    configSeq += makeConfig('PileupReweighting')
    configSeq.setOptionValue('.campaign', flags.Input.MCCampaign, noneAction='ignore')
    configSeq.setOptionValue('.files', flags.Input.Files, noneAction='ignore')
    if 'prw_files' in flags.Analysis:
        configSeq.setOptionValue('.userPileupConfigs', get_prw_files(flags))
    else:
        # only use default config if we're not using custom PRW
        configSeq.setOptionValue('.useDefaultConfig', True)
    if 'lumicalc_files' in flags.Analysis:
        configSeq.setOptionValue('.userLumicalcFiles', get_lumicalc_files(flags))

    return configSeq


def generator_sequence(flags):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

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
    # Include, and then set up the generator analysis sequence:
    configSeq += makeConfig('GeneratorLevelAnalysis')
    configSeq.setOptionValue('.saveCutBookkeepers', doCBK)
    configSeq.setOptionValue('.runNumber', flags.Input.RunNumbers[0])
    configSeq.setOptionValue('.cutBookkeepersSystematics', doCBK)

    return configSeq
