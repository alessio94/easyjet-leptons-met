from enum import Enum


class SampleTypes(Enum):
    mc20a = "r13167"
    mc20d = "r13144"
    mc20e = "r13145"


def pileupConfigFiles(fileMD):
    """Return the PRW (Pileup ReWeighting) config files and lumicalc files"""
    tags = fileMD.get("AMITag", "")
    dsid = fileMD.get("mcChannelNumber", 0)
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
    if subcampaign == SampleTypes.mc20d:
        actual_mu.append(
            "GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root"  # noqa
        )
    if subcampaign == SampleTypes.mc20e:
        actual_mu.append(
            "GoodRunsLists/data18_13TeV/20190318/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root"  # noqa
        )

    prw_files = [
        f"dev/PileupReweighting/share/DSID{dsid[:3]}xxx/pileup_{subcampaign.name}_dsid{dsid}_{'AFII' if 'a' in tags else 'FS'}.root"  # noqa
    ]

    return prw_files + actual_mu
