from AthenaCommon.Constants import INFO
from EasyjetHub.algs.event_counter_config import METADATA_FILE


def run_job(flags,args,cfg):

    # Print the full job configuration
    if flags.Exec.OutputLevel <= INFO:
        cfg.printConfig(summariseProps=False)

    # clear the cutflow
    METADATA_FILE.unlink(missing_ok=True)

    # Execute the job defined in the ComponentAccumulator.
    # The number of events is specified by `args.evtMax`
    return cfg.run(args.evtMax)
