from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory
from EasyjetHub.steering.sample_metadata import get_prw_files, get_lumicalc_files
from EasyjetHub.steering.utils.log_helper import log


def pileup_sequence(flags):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    # Nominal config will be used to define RandomRunNumber, if it does not exist yet
    # in the input (PHYSLITE stores it already)
    PRW_config = [flags.Analysis.PileupReweighting]
    # Extra PRW does not alter the RandomRunNumber at this point
    PRW_config += flags.Analysis.PileupReweighting.extra_prw

    altConfig = False
    for prw in PRW_config:
        postfix = ("_" + prw.postfix) if altConfig else ""
        configSeq += makeConfig('PileupReweighting')
        configSeq.setOptionValue('.postfix', postfix)
        configSeq.setOptionValue('.campaign', flags.Input.MCCampaign)
        configSeq.setOptionValue('.files', flags.Input.Files)
        configSeq.setOptionValue('.alternativeConfig', altConfig)
        if 'prw_files' in prw:
            configSeq.setOptionValue('.userPileupConfigs', get_prw_files(flags, prw))
        else:
            # only use default config if we're not using custom PRW
            configSeq.setOptionValue('.useDefaultConfig', True)
        if 'lumicalc_files' in prw:
            configSeq.setOptionValue('.userLumicalcFiles',
                                     get_lumicalc_files(flags, prw))
        altConfig = True

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
    doCBK = not is_bad_tag and bool(flags.Analysis.out_file)
    # Include, and then set up the generator analysis sequence:
    configSeq += makeConfig('GeneratorLevelAnalysis')
    configSeq.setOptionValue('.saveCutBookkeepers', doCBK)
    if hasattr(flags.Analysis, "ttree_output"):
        configSeq.setOptionValue('.streamName', 'CBK' if flags.Analysis.splitCBK else
                                 flags.Analysis.ttree_output.stream_name)
    configSeq.setOptionValue('.runNumber', flags.Input.RunNumbers[0])
    configSeq.setOptionValue('.cutBookkeepersSystematics', doCBK
                             and flags.Analysis.do_CP_systematics)
    configSeq.setOptionValue('.histPattern', flags.Analysis.cbkHistPattern)

    return configSeq
