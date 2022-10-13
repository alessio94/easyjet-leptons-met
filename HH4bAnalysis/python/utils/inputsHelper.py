def is_physlite(flags):
    return flags.Input.ProcessingTags == ["StreamDAOD_PHYSLITE"]


def get_dataType(flags, isPRW=False):
    dataType = ""
    if flags.Input.SimulationFlavour in [
        "",
        "FullG4",
        "FullG4_QS",
        "FullG4_Longlived",
    ]:
        dataType = "mc"
    if flags.Input.SimulationFlavour in ["ATLFAST3_QS"] and not isPRW:
        # in R22 there are no calibrations for af3 yet,
        # using FullSim calibrations for now
        dataType = "mc"
    if flags.Input.SimulationFlavour in ["ATLFAST3_QS"] and isPRW:
        # there are no PRW files for af3 yet, except for the SH samples.
        # however, they are hard-coded in the dev group as AFII.root,
        # so for now setting af3 to afii to get correct PRW files from dev
        dataType = "afii"
    if not flags.Input.isMC:
        dataType = "data"

    if not dataType:
        raise AssertionError("dataType cannot be determined from inputs!")

    return dataType
