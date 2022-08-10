from enum import Enum
import json


class SampleTypes(Enum):
    mc20a = "r13167"
    mc20d = "r13144"
    mc20e = "r13145"


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


def pileupConfigFiles(fileMD):
    """Return the PRW (Pileup ReWeighting) config files and lumicalc files"""
    tags = fileMD["AMITag"]
    dsid = fileMD["mc_channel_number"]
    split_tags = tags.split("_")
    # Figure out which MC we are using
    if SampleTypes.mc20a.value in split_tags:
        subcampaign = SampleTypes.mc20a
    elif SampleTypes.mc20d.value in split_tags:
        subcampaign = SampleTypes.mc20d
    elif SampleTypes.mc20e.value in split_tags:
        subcampaign = SampleTypes.mc20e
    else:
        raise LookupError(f"Cannot determine subcampaign for DSID {dsid}")

    lumicalc_files = getLumicalcFiles(subcampaign)
    prw_files = getPrwFiles(dsid, subcampaign, tags)

    return prw_files, lumicalc_files


def getLumicalcFiles(subcampaign):
    list = []
    if subcampaign == SampleTypes.mc20a:
        list.extend(
            (
                "GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root",  # noqa
                "GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root",  # noqa
            )
        )
    if subcampaign == SampleTypes.mc20d:
        list.append(
            "GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root",  # noqa
        )
    if subcampaign == SampleTypes.mc20e:
        list.append(
            "GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root"  # noqa
        )

    return list


def getPrwFiles(dsid, subcampaign, tags):
    actual_mu = []
    prw_files = []
    if subcampaign == SampleTypes.mc20d:
        actual_mu.append(
            "GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root"  # noqa
        )
    if subcampaign == SampleTypes.mc20e:
        actual_mu.append(
            "GoodRunsLists/data18_13TeV/20190318/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root"  # noqa
        )

    if dsid:
        dsid_as_str = str(dsid)
        prw_files.append(
            f"dev/PileupReweighting/share/DSID{dsid_as_str[:3]}xxx/pileup_{subcampaign.name}_dsid{dsid_as_str}_{'AFII' if 'a' in tags else 'FS'}.root"  # noqa
        )

    return prw_files + actual_mu
