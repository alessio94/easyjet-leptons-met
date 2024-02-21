import json
import pickle
from pathlib import Path
from PathResolver import PathResolver
from Campaigns.Utils import Campaign


MCSampleYears = {
    Campaign.MC20a: (2015, 2016),
    Campaign.MC20d: (2017,),
    Campaign.MC20e: (2018,),
    Campaign.MC21a: (2022,),
    Campaign.MC23a: (2022,),
    Campaign.MC23c: (2023,),
}


def cache_metadata(path):
    from AthenaConfiguration.AutoConfigFlags import _fileMetaData

    all_md = {}
    for f, m in _fileMetaData.items():
        all_md[f] = {
            "metadata": m.metadata,
            "level": m.metAccessLevel,
        }
    try:
        with open(path, "w") as cached:
            json.dump(all_md, cached, indent=2)
    except TypeError:
        # if json fails, try pickle
        path.unlink(missing_ok=True)
        with open(path.with_suffix(".pkl"), "wb") as cached:
            pickle.dump(all_md, cached)


def _load_metadata(path):
    pkl_path = path.with_suffix('.pkl')
    if not path.exists() and pkl_path.exists():
        path = pkl_path
    elif not path.exists():
        return None
    with open(path, "rb") as cached_file:
        if path.suffix == '.pkl':
            return pickle.load(cached_file)
        else:
            return json.load(cached_file)


def update_metadata(path):
    from AthenaConfiguration.AutoConfigFlags import _fileMetaData
    all_cached = _load_metadata(path)
    if all_cached is None:
        return
    for f, m in _fileMetaData.items():
        cached = all_cached.get(f)
        if cached:
            md = _fileMetaData[f]
            md.metadata.update(cached["metadata"])
            md.filename = f
            md.metAccessLevel = cached["level"]


def has_metadata(flags, path=Path("metadata.json")):
    metadict = _load_metadata(path)
    if metadict is None:
        return False
    for infile in flags.Input.Files:
        if infile not in metadict:
            return False
    return True


def get_valid_ami_tag(tags, check_tag="p", min_valid_tag="p5657"):
    is_valid_tag = False
    for tag in tags:
        if check_tag in tag:
            is_valid_tag = int(tag[1:]) > int(min_valid_tag[1:])
    return is_valid_tag


def get_lumicalc_files(flags):
    """Return the lumicalc files"""

    lumicalc_files = set()
    for year in flags.Analysis.Years:
        year = str(year)
        lumicalc_dir = flags.Analysis.grl_years[year]
        lumicalc_file = flags.Analysis.lumicalc_files[year]
        if lumicalc_dir and lumicalc_file:
            lumicalc_files.add(str(Path(lumicalc_dir) / lumicalc_file))
        else:
            raise RuntimeError(
                f"Could not find Lumicalc for year {year}. "
                "Specify Lumicalc files in the config file."
            )
    return list(lumicalc_files)


def get_prw_files(flags):
    """Return the PRW (Pileup ReWeighting) config files."""
    campaign = flags.Input.MCCampaign

    prw_files = set()
    for year in flags.Analysis.Years:
        year = str(year)
        prw_dir = flags.Analysis.grl_years[year]
        # because we don't get PRW from GRL folders from all years
        if year in flags.Analysis.prw_files:
            prw_file = flags.Analysis.prw_files[year]
            prw_files.add(str(Path(prw_dir) / prw_file))
        elif campaign in flags.Analysis.prw_files:
            prw_file = flags.Analysis.prw_files[campaign]
            prw_files.add(prw_file)
        else:
            raise RuntimeError(
                f"Could not find PRW for year {year} or campaign {campaign}. "
                "Specify PRW files in the config file."
            )

    if flags.Input.MCChannelNumber:
        dsid = str(flags.Input.MCChannelNumber)
        data_type = get_data_type(flags, is_prw=True)
        if data_type == "afii":
            simulation_type = "AFII"
        else:
            simulation_type = "FS"
        config = f"dev/PileupReweighting/share/DSID{dsid[:3]}xxx/pileup_{campaign}_dsid{dsid}_{simulation_type}.root"  # noqa
        if PathResolver.FindCalibFile(config):
            prw_files.add(config)

    return list(prw_files)


def get_run_years(flags):
    years = []
    if flags.Analysis.DataType != "data":
        years += MCSampleYears[flags.Input.MCCampaign]
    else:
        years.append(flags.Input.DataYear)
    return years


def get_data_type(flags, is_prw=False):
    data_type = ""
    if flags.Input.SimulationFlavour in [
        "",
        "FullG4",
        "FullG4_QS",
        "FullG4_LongLived",
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


def get_grl_files(flags):
    grl_files = set()
    if not flags.Input.isMC:
        for year in flags.Analysis.Years:
            year = str(year)
            grl_dir = flags.Analysis.grl_years[year]
            grl_file = flags.Analysis.grl_files[year]
            if grl_dir and grl_file:
                grl_files.add(str(Path(grl_dir) / grl_file))
            else:
                raise RuntimeError(
                    f"Could not find GRL for year {year}. "
                    "Specify GRL files in the config file."
                )
    return list(grl_files)
