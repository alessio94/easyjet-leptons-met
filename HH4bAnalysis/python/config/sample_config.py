import json
from enum import Enum
from pathlib import Path


class DataSampleYears(Enum):
    data15 = (2015,)
    data16 = (2016,)
    data17 = (2017,)
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
    # ptag for Xbb tagger
    mc20x = "p5657"


def cache_metadata(path):
    from AthenaConfiguration.AutoConfigFlags import _fileMetaData

    all_md = {}
    for f, m in _fileMetaData.items():
        all_md[f] = {
            "metadata": m.metadata,
            "level": m.metAccessLevel,
        }
    with open(path, "w") as cached:
        json.dump(all_md, cached, indent=2)


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


def has_metadata(flags,path=Path('metadata.json')):
    if not path.is_file():
        return False
    with open(path) as meta:
        metadict = json.load(meta)
        for infile in flags.Input.Files:
            if infile not in metadict:
                return False
        return True


def get_valid_ami_tag(tags, check_tag="p", min_valid_tag=SampleTypes.mc20):
    is_valid_tag = False
    for tag in tags:
        if check_tag in tag:
            is_valid_tag = int(tag[1:]) > int(min_valid_tag.value[1:])
    return is_valid_tag


def get_pileup_config_files(flags):
    """Return the PRW (Pileup ReWeighting) config files and lumicalc files"""

    dsid = flags.Input.MCChannelNumber
    tags = flags.Input.AMITag
    data_type = get_data_type(flags, is_prw=True)

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

    lumicalc_files = _get_lumicalc_files(subcampaign)
    prw_files = _get_prw_files(dsid, subcampaign, data_type)

    return prw_files, lumicalc_files


def _get_lumicalc_files(subcampaign):
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


def _get_prw_files(dsid, subcampaign, data_type):
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
        if data_type == "mc":
            simulation_type = "FS"
        else:
            simulation_type = "AFII"

        prw_files.append(
            f"dev/PileupReweighting/share/DSID{dsid_as_str[:3]}xxx/pileup_{subcampaign.name}_dsid{dsid_as_str}_{simulation_type}.root"  # noqa
        )

    return prw_files + actual_mu.get(subcampaign, [])


def get_run_years(flags):
    years = []
    if flags.Analysis.DataType != "data":
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


def get_data_type(flags, is_prw=False):
    data_type = ""
    if flags.Input.SimulationFlavour in [
        "",
        "FullG4",
        "FullG4_QS",
        "FullG4_Longlived",
    ]:
        data_type = "mc"
    if flags.Input.SimulationFlavour in ["ATLFAST3_QS"] and not is_prw:
        # in R22 there are no calibrations for af3 yet,
        # using FullSim calibrations for now
        data_type = "mc"
    if flags.Input.SimulationFlavour in ["ATLFAST3_QS"] and is_prw:
        # there are no PRW files for af3 yet, except for the SH samples.
        # however, they are hard-coded in the dev group as AFII.root,
        # so for now setting af3 to afii to get correct PRW files from dev
        data_type = "afii"
    if not flags.Input.isMC:
        data_type = "data"

    if not data_type:
        raise AssertionError("Data type cannot be determined from inputs!")

    return data_type
