from HH4bAnalysis.Config.Base import SampleTypes


def is_physlite(flags):
    return flags.Input.ProcessingTags == ["StreamDAOD_PHYSLITE"]


def get_valid_ami_tag(tags, check_tag="p", min_valid_tag=SampleTypes.mc20):
    is_valid_tag = False
    for tag in tags:
        if check_tag in tag:
            is_valid_tag = int(tag[1:]) > int(min_valid_tag.value[1:])
    return is_valid_tag
