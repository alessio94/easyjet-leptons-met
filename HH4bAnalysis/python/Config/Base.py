from enum import Enum
import json


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
    mc20a = MCSampleYears.r13167.name  # run2, 2015-16
    mc20d = MCSampleYears.r13144.name  # run2, 2017
    mc20e = MCSampleYears.r13145.name  # run2, 2018
    mc21a = MCSampleYears.r13829.name  # run3, 2022
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


def pileupConfigFiles(flags):
    """Return the PRW (Pileup ReWeighting) config files and lumicalc files"""
    dsid = flags.Input.MCChannelNumber
    tags = flags.Input.AMITag
    simulation_flavor = flags.Input.SimulationFlavour
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
    prw_files = getPrwFiles(dsid, subcampaign, simulation_flavor)

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
            "GoodRunsLists/data22_13p6TeV/20220820/ilumicalc_histograms_None_427882-428855_OflLumi-Run3-001.root"  # noqa
        ],
    }

    return list.get(subcampaign, [])


def getPrwFiles(dsid, subcampaign, simulation_flavor):
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
        if simulation_flavor in ["", "FullG4", "FullG4_QS", "FullG4_Longlived"]:
            simulation_type = "FS"
        else:
            simulation_type = "AFII"

        prw_files.append(
            f"dev/PileupReweighting/share/DSID{dsid_as_str[:3]}xxx/pileup_{subcampaign.name}_dsid{dsid_as_str}_{simulation_type}.root"  # noqa
        )

    return prw_files + actual_mu.get(subcampaign, [])


def getRunYears(flags):
    years = []
    if flags.Input.isMC:
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
