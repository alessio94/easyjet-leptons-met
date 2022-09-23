from HH4bAnalysis.Config.Base import SampleTypes


def is_physlite(flags):
    return flags.Input.ProcessingTags == ["StreamDAOD_PHYSLITE"]


def get_valid_ami_tag(tags, check_tag="p", min_valid_tag=SampleTypes.mc20):
    is_valid_tag = False
    for tag in tags:
        if check_tag in tag:
            is_valid_tag = int(tag[1:]) > int(min_valid_tag.value[1:])
    return is_valid_tag


def get_dataType(flags):
    if flags.Input.SimulationFlavour in [
        "",
        "FullG4",
        "FullG4_QS",
        "FullG4_Longlived",
    ]:
        dataType = "mc"
    if flags.Input.SimulationFlavour in ["ATLFAST3_QS"]:
        # in R22 there are no calibrations for af3 yet,
        # using FullSim calibrations for now
        dataType = "mc"
    if not flags.Input.isMC:
        dataType = "data"

    try:
        dataType
    except NameError:
        raise AssertionError("dataType cannot be determined from inputs!")

    return dataType
