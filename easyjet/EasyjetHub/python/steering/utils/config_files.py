import os
import pathlib
import yaml
from argparse import ArgumentTypeError

from AnalysisAlgorithmsConfig.ConfigText import combineConfigFiles


def run_config_arg(rawpath):
    fpath = pathlib.Path(rawpath)
    for dirpath in [""] + os.environ["DATAPATH"].split(":"):
        fullpath = dirpath / fpath
        if fullpath.exists():
            try:
                with open(fullpath) as cfgfile:
                    local = yaml.safe_load(cfgfile)
                    combineConfigFiles(
                        local,
                        fullpath.parent,
                        fragment_key='include'
                    )
                    return local
            except FileNotFoundError as err:
                raise ArgumentTypeError(
                    f"Couldn't load run config: {fullpath}, "
                    f"looking for {str(err.args[0])}")

    raise ArgumentTypeError(f"Couldn't find config: {fpath}")
