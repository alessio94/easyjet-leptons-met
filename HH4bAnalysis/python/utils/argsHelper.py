import sys


def checkArgs(flags, args, parser):
    if "StreamDAOD_PHYSLITE" in flags.Input.ProcessingTags:
        input_stream_format = "DAOD_PHYSLITE"
    elif "StreamDAOD_PHYS" in flags.Input.ProcessingTags:
        input_stream_format = "DAOD_PHYS"
    else:
        input_stream_format = "other"

    if args.daod_physlite and input_stream_format != "DAOD_PHYSLITE":
        print(
            f"Input file is {input_stream_format} but --daod-physlite flag was "
            f"{'given' if args.daod_physlite else 'not given'}. Usage: {parser.prog} -h"
        )
        sys.exit(-1)

    is_input_mc = flags.Input.isMC
    if args.mc is not is_input_mc:
        print(
            f"Input file is {'MC' if is_input_mc else 'Data'} but --mc flag was "
            f"{'given' if args.mc else 'not given'}. Usage: {parser.prog} -h"
        )
        sys.exit(-1)
