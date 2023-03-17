def is_physlite(flags):
    return flags.Input.ProcessingTags == ["StreamDAOD_PHYSLITE"]


def is_mc_phys(flags):
    return flags.Input.isMC and not is_physlite(flags)
