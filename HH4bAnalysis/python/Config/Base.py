import json
import sys
from enum import Enum

import yaml
from HH4bAnalysis.utils.logHelper import log
from HH4bAnalysis.utils.inputsHelper import get_dataType


class DataSampleYears(Enum):
    data16 = (2016,)
    data18 = (2018,)
    data22 = (2022,)


class MCSampleYears(Enum):
    r13167 = (2015, 2016)
    r13144 = (2017,)
    r13145 = (2018,)
    r13829 = (2022,)


class SampleTypes(Enum):
    mc20a = "r13167"  # run2, 2015-16
    mc20d = "r13144"  # run2, 2017
    mc20e = "r13145"  # run2, 2018
    mc21a = "r13829"  # run3, 2022
    # ptag
    mc20 = "p5057"


def cache_metadata(path):
    from AthenaConfiguration.AutoConfigFlags import _fileMetaData

    all_md = {}
    for f, m in _fileMetaData.items():
        all_md[f] = {
            "metadata": m.metadata,
            "level": m.metAccessLevel,
        }
    with open(path, "w") as cached:
        json.dump(all_md, cached)


def update_metadata(path):
    from AthenaConfiguration.AutoConfigFlags import _fileMetaData

    if not path.exists():
        return
    with open(path) as cached_file:
        all_cached = json.load(cached_file)
    for f, m in _fileMetaData.items():
        cached = all_cached.get(f)
        if cached:
            md = _fileMetaData[f]
            md.metadata.update(cached["metadata"])
            md.filename = f
            md.metAccessLevel = cached["level"]


def get_valid_ami_tag(tags, check_tag="p", min_valid_tag=SampleTypes.mc20):
    is_valid_tag = False
    for tag in tags:
        if check_tag in tag:
            is_valid_tag = int(tag[1:]) > int(min_valid_tag.value[1:])
    return is_valid_tag


def pileupConfigFiles(flags):
    """Return the PRW (Pileup ReWeighting) config files and lumicalc files"""

    dsid = flags.Input.MCChannelNumber
    tags = flags.Input.AMITag
    dataType = get_dataType(flags, isPRW=True)

    # Figure out which MC we are using
    if SampleTypes.mc20a.value in tags:
        subcampaign = SampleTypes.mc20a
    elif SampleTypes.mc20d.value in tags:
        subcampaign = SampleTypes.mc20d
    elif SampleTypes.mc20e.value in tags:
        subcampaign = SampleTypes.mc20e
    elif SampleTypes.mc21a.value in tags:
        subcampaign = SampleTypes.mc21a
    else:
        raise LookupError(f"Cannot determine subcampaign for DSID {dsid}")

    lumicalc_files = getLumicalcFiles(subcampaign)
    prw_files = getPrwFiles(dsid, subcampaign, dataType)

    return prw_files, lumicalc_files


def getLumicalcFiles(subcampaign):
    list = {
        SampleTypes.mc20a: [
            "GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root",  # noqa
            "GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root",  # noqa
        ],
        SampleTypes.mc20d: [
            "GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root",  # noqa
        ],
        SampleTypes.mc20e: [
            "GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root"  # noqa
        ],
        SampleTypes.mc21a: [
            "GoodRunsLists/data22_13p6TeV/20220902/ilumicalc_histograms_None_427882-430648_OflLumi-Run3-001.root"  # noqa
        ],
    }

    return list.get(subcampaign, [])


def getPrwFiles(dsid, subcampaign, dataType):
    prw_files = []
    actual_mu = {
        SampleTypes.mc20d: [
            "GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root"  # noqa
        ],
        SampleTypes.mc20e: [
            "GoodRunsLists/data18_13TeV/20190318/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root"  # noqa
        ],
    }

    if dsid:
        dsid_as_str = str(dsid)
        if dataType == "mc":
            simulation_type = "FS"
        else:
            simulation_type = "AFII"

        prw_files.append(
            f"dev/PileupReweighting/share/DSID{dsid_as_str[:3]}xxx/pileup_{subcampaign.name}_dsid{dsid_as_str}_{simulation_type}.root"  # noqa
        )

    return prw_files + actual_mu.get(subcampaign, [])


def getRunYears(flags, dataType):
    years = []
    if dataType != "data":
        # use rtag for figuring out year in MC
        tags = flags.Input.AMITag
        for mc_campaign in MCSampleYears:
            if mc_campaign.name in tags:
                years += mc_campaign.value
                break
    else:
        # Use projet_name for figuring out which year in data
        project_name = flags.Input.ProjectName
        for data_campaign in DataSampleYears:
            if data_campaign.name in project_name:
                years += data_campaign.value
                break
    return years


def ConfigFlagsAdder(args, flags):
    # load user config
    try:
        with open(args.runConfig) as file:
            runConfig = yaml.safe_load(file)
            log.info("Loaded run config: " + str(args.runConfig))
    except FileNotFoundError:
        log.error(
            "Couldn't load run config: "
            + str(args.runConfig)
            + "\n"
            + "Please give a valid run config file path to  --runConfig"
        )
        sys.exit(1)

    # removing standard athena flags from args
    athFlags = [
        "debug",
        "evtMax",
        "skipEvents",
        "filesInput",
        "loglevel",
        "configOnly",
        "threads",
        "nprocs",
    ]

    # args contain the flags, overwrite runconfig file values with values from flags
    for key in vars(args):
        # exclude standard athena flags
        if key not in athFlags:
            value = getattr(args, key)
            # don't overwrite if flags not given (None), not given bools are False
            if value is not None and value is not False:
                runConfig[key] = getattr(args, key)

    # add them to ConfigFlags
    for key, value in runConfig.items():
        flags.addFlag("Analysis." + key, value)
        if key != "runConfig":
            log.info("User configured: " + str(key) + ": " + str(value))

    return flags
