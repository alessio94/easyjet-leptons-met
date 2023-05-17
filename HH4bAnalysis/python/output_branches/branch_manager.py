from dataclasses import dataclass, field
from enum import IntEnum


class SystOption(IntEnum):
    NONE = 0
    NO_SYST = 1
    ALL_SYST = 2


@dataclass
class BranchManager(object):
    """Class for handling container to output ntuple branch mapping"""
    input_container:    str
    output_prefix:      str
    do_overlap_removal: bool = False
    systematics_option: SystOption = SystOption.NONE
    required_flags:     list = field(default_factory=list)
    variables:          list = field(default_factory=list)

    def or_str(self):
        if self.do_overlap_removal:
            return "_OR"
        else:
            return ""

    def syst_str(self):
        return {
            SystOption.NONE:     "",
            SystOption.NO_SYST:  "_NOSYS",
            SystOption.ALL_SYST: "_%SYS%",
        }[self.systematics_option]

    def full_output_prefix(self):
        _output_prefix = f"{self.output_prefix}{self.or_str()}{self.syst_str()}"
        if _output_prefix:
            _output_prefix += '_'
        return _output_prefix

    def full_input_container(self):
        _input_container = f"{self.input_container}{self.or_str()}"
        if (
            "%SYS%" in _input_container
            and self.systematics_option == SystOption.NO_SYST
        ):
            _input_container = _input_container.replace("_%SYS%",self.syst_str())
        return _input_container

    def output_string(self,var):
        # Handle case where output prefix is ''
        return (
            f"{self.full_input_container()}.{var} -> "
            f"{self.full_output_prefix()}{var}"
        )

    def add_four_mom_branches(self,do_mass):
        self.variables += ["pt","eta","phi"]
        if do_mass:
            self.variables += ["m"]

    def get_output_list(self):
        for req in self.required_flags:
            if not req:
                raise RuntimeError(
                    f"Required condition not satisfied"
                    f" for branches from {self.full_input_container()}"
                    f" to {self.full_output_prefix()}"
                )
        return [
            self.output_string(var) for var in self.variables
        ]
