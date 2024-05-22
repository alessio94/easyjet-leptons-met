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
    systematics_suffix_separator: str = "_"
    required_flags:     list = field(default_factory=list)
    variables:          list = field(default_factory=list)

    # Apply systs only for given vars (takes priority) or not for some vars
    syst_only_for:      list = field(default_factory=list)
    syst_not_for:       list = field(default_factory=list)

    # In the ConfigBlock OR setup, we cannot actually make OR-ed
    # view containers for output without some more work.
    # So instead we have to add a 'passesOR_%SYS% branch
    def or_str(self):
        return ""
        # if self.do_overlap_removal:
        #     return "_OR"
        # else:
        #     return ""

    def syst_str(self):
        return {
            SystOption.NONE:     "",
            SystOption.NO_SYST:  self.systematics_suffix_separator + "NOSYS",
            SystOption.ALL_SYST: self.systematics_suffix_separator + "%SYS%",
        }[self.systematics_option]

    def not_apply_syst_for_var(self, var):
        """
        Check if we are applying systematics for a given variable.
        If syst_only_for is set, apply only for those vars, else
        if syst_not_for is set, don't apply for those.
        """
        if "%SYS%" in var:
            return False
        return ((self.syst_only_for and var not in self.syst_only_for)
                or (not self.syst_only_for and var in self.syst_not_for))

    def full_output_var(self, var):
        _output_prefix = f"{self.output_prefix}{self.or_str()}"
        if _output_prefix:
            _output_prefix += '_'

        _output_suffix = self.syst_str()
        if self.not_apply_syst_for_var(var):
            _output_suffix = ''

        _output_var = _output_prefix + var + _output_suffix

        # Handle decorations which have already _%SYS% in their name
        if "_%SYS%" in var:
            _output_var = _output_var.replace(
                "_%SYS%" + _output_suffix, _output_suffix)

        return _output_var

    def full_input_container(self, var):
        _input_container = f"{self.input_container}{self.or_str()}"
        if (
            "%SYS%" in _input_container
            and self.systematics_option == SystOption.NO_SYST
        ):
            _input_container = _input_container.replace("_%SYS%", self.syst_str())

        if self.not_apply_syst_for_var(var):
            _input_container = _input_container.replace("_%SYS%", "_NOSYS")

        return _input_container

    def output_string(self, var):
        incont = self.full_input_container(var)
        outvar = self.full_output_var(var)
        outstr = f"{incont}.{var} -> {outvar}"
        return outstr

    def add_four_mom_branches(self, do_mass):
        self.variables += ["pt", "eta", "phi"]
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
