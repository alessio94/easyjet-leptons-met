import os
import pathlib
import yaml
from argparse import ArgumentTypeError


def combine_config_files(local, config_path, fragment_key="include"):

    # if this isn't an iterable there's nothing to combine
    if isinstance(local, dict):
        to_combine = local.values()
    elif isinstance(local, list):
        to_combine = local
    else:
        return

    # otherwise descend into all the entries here
    for sub in to_combine:
        combine_config_files(sub, config_path)

    # if there are no fragments to include we're done
    if fragment_key not in local:
        return

    fragment_path = _find_fragment(
        pathlib.Path(local[fragment_key]),
        config_path)

    with open(fragment_path) as fragment_file:
        fragment = yaml.safe_load(fragment_file)

    # fill out any sub-fragments, looking in the parent path of the
    # fragment for local sub-fragments.
    combine_config_files(fragment, fragment_path.parent)

    # merge the fragment with this one
    _merge_dicts(local, fragment)

    # delete the fragment so we don't stumble over it again
    del local[fragment_key]


def run_config_arg(rawpath):
    fpath = pathlib.Path(rawpath)
    for dirpath in [""] + os.environ["DATAPATH"].split(":"):
        fullpath = dirpath / fpath
        if fullpath.exists():
            try:
                with open(fullpath) as cfgfile:
                    local = yaml.safe_load(cfgfile)
                    combine_config_files(local, fullpath.parent)
                    return local
            except FileNotFoundError as err:
                raise ArgumentTypeError(
                    f"Couldn't load run config: {fullpath}, "
                    f"looking for {str(err.args[0])}")

    raise ArgumentTypeError(f"Couldn't find config: {fpath}")


def _find_fragment(fragment_path, config_path):
    paths_to_check = [
        fragment_path,
        config_path / fragment_path,
        *[x / fragment_path for x in os.environ["DATAPATH"].split(":")]
    ]
    for path in paths_to_check:
        if path.exists():
            return path

    raise FileNotFoundError(fragment_path)


def _merge_dicts(local, fragment):
    # in the list case append the fragment to the local list
    if isinstance(local, list):
        local += fragment
        return
    # In the dict case, append only missing values to local: the local
    # values take precidence over the fragment ones.
    if isinstance(local, dict):
        for key, value in fragment.items():
            if key in local:
                _merge_dicts(local[key], value)
            else:
                local[key] = value
        return
