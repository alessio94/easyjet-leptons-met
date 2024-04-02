from pprint import pformat
from .log_helper import log
from .config_files import run_config_arg
from .option_validation import validate_flags
from ..argument_parser import validate_args
from argparse import Namespace

from AthenaConfiguration.AthConfigFlags import AthConfigFlags


##########################################################
# Convert everything back to standard python stuff
##########################################################
#
# Everything in here should be better behaved
# than AthConfigFlags
class ConfigItem:
    """Frozen attributes class

    This class provides access to its data via attribute
    access, as well as the familiar dict interface, while
    remaining immutable. We convert locked AthConfigFlags
    to ConfigItem, keeping the flag values frozen, but
    gain some nicer iteration capabilities in the process.
    """

    def __init__(self, **args):
        fields = []
        for name, val in args.items():
            fields.append((name, val))
        try:
            fset = frozenset(fields)
        except TypeError as e:
            errmsg = (
                'ConfigItem must be constructed from immutable, hashable inputs.'
                ' Use to_immutable for recursive conversion.'
            )
            raise TypeError(errmsg) from e
        super().__setattr__("_fields", frozenset(x for x, y in fset))
        super().__setattr__("_hash", hash(fset))
        for name, val in fset:
            super().__setattr__(name, val)

    # we <3 iteration
    def __iter__(self):
        for k in self._fields:
            yield k

    def __contains__(self, key):
        return key in self._fields

    # and why not <3 hashing too?
    def __hash__(self):
        return self._hash

    def __eq__(self, other):
        kother = set(other)
        kself = set(self)
        if kself != kother:
            return False
        for k in kself:
            if k not in other:
                return False
            if self[k] != other[k]:
                return False
        return True

    # we also <3 getitem syntax
    def __getitem__(self, key):
        return getattr(self, key)

    # disable setting and deleting of attributes
    def __setattr__(self, attr, value):
        name = type(self).__name__
        raise TypeError(f"{name} doesn't support setting attributes")

    def __delattr__(self, attr):
        name = type(self).__name__
        raise TypeError(f"{name} doesn't support deleting attributes")

    # Fill out dict-like interface
    def keys(self):
        for k in self._fields:
            yield k

    def values(self):
        for k in self._fields:
            yield self[k]

    def items(self):
        for k in self:
            yield k, self[k]

    # Convenient back-conversion to a dict
    # Will convert tuples to lists
    # As it's not possible to unambiguously decide
    # if we should convert a sequence into a list
    # or a tuple, we pick the mutable option,
    # which is also the only thing yaml supports
    def as_mutable(self):

        def to_mutable(v):
            if isinstance(v, ConfigItem):
                return v.as_mutable()
            elif isinstance(v, tuple):
                return [to_mutable(e) for e in v]
            return v

        return {
            k: to_mutable(v)
            for k, v in self.items()
        }

    # Provide a nice string representation
    def __repr__(self):
        return pformat(self.as_mutable())


def to_immutable(keydict):
    """Convert a dictionary / list struct to ConfigItem / tuple

    The resulting data structure is immutable and behaves sort of like
    AthConfigFlags, only better.

    """
    if isinstance(keydict, dict):
        # Avoid clobbering these keys if they exist. For now just
        # throw an exception.
        for k in ['items', 'keys', 'values']:
            if k in keydict:
                raise ValueError(
                    f"field '{k}' isn't allowed in configuration files")
        subtuples = {k: to_immutable(v) for k, v in keydict.items()}
        return ConfigItem(**subtuples)
    if isinstance(keydict, list):
        return tuple(to_immutable(x) for x in keydict)
    return keydict

##########################################################
# Convert yaml configuration to AthConfigFlags
# and transform Analysis sub-flags into ConfigItem
# for nicer iteration
##########################################################


def dict_to_flags(d: dict) -> AthConfigFlags:
    _flags = AthConfigFlags()
    for k, v in d.items():
        if isinstance(v, dict):
            _flags.addFlagsCategory(
                k,
                lambda v=v: dict_to_flags(v),
                prefix=True,
            )
        else:
            assert v is not None, f"Flag value {k} not assigned"
            _flags.addFlag(k, v)
    return _flags


def fill_flags_from_runconfig(
    args: Namespace,
    flags: AthConfigFlags,
    overwrites: dict,
) -> AthConfigFlags:
    # Collect all the run config values from config file and flags
    run_config_all = run_config_arg(args.run_config)
    validate_args(run_config_all, overwrites)

    # args contain the flags, overwrite run_config file values with values from flags
    # and add flags that are not in run_config file
    for key in vars(args):
        # exclude standard athena flags
        if key in overwrites:
            value = getattr(args, key)
            if value is not None or key not in run_config_all:
                run_config_all[key] = value

    # add them to athena's ConfigFlags
    for key, value in run_config_all.items():
        log.info("User configured: " + str(key) + ": " + str(value))
        # if we find a dict, make a flag category
        if isinstance(value, dict):
            flags.addFlagsCategory(
                f"Analysis.{key}",
                lambda value=value: dict_to_flags(value),
                prefix=True,
            )
        else:
            assert value is not None, f"Flag value {key} not assigned"
            flags.addFlag(f"Analysis.{key}", value)

    return flags


def lock_merged_config_flags(flags, subflag_name='Analysis'):
    """Main function to lock configuration flags

    This should be the only thing that users need to call.
    """

    flag_dict = flags[subflag_name].asdict()
    tup_flags = to_immutable(flag_dict)
    del flags[subflag_name]
    flags.addFlag(subflag_name, tup_flags)
    flags.lock()

    validate_flags(flags)
