from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import makeConfig

from EasyjetHub.steering.utils.name_helper import drop_sys


def met_sequence(flags, configAcc):

    assert flags.Analysis.do_small_R_jets, (
        "Small-R jets are strictly necessary for MET"
    )

    configSeq = ConfigSequence()

    container_names = flags.Analysis.container_names

    preMET_collections = {}
    objflags = {x:f'do_{x}' for x in ['electrons','photons','muons','taus']}
    METselections = dict(
        electrons=f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}',
        photons=f'{flags.Analysis.Photon.ID}_{flags.Analysis.Photon.Iso}',
        muons=f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}',
        taus=flags.Analysis.Tau.ID,
    )
    # Construct the names of the view containers with working point selection
    # We need to use the '.' style so that the algs operate on the full
    # container, and avoid incomplete decorations
    for objtype, objflag in objflags.items():
        if flags.Analysis[objflag]:
            collname = drop_sys(container_names.output[objtype])
            selection = METselections[objtype]
            preMET_collections[objtype] = f'{collname}.{selection}'

    configSeq += makeConfig('MissingET', drop_sys(container_names.output.met))
    # Pass all the calibrated jets
    configSeq.setOptionValue(
        '.jets',
        drop_sys(container_names.allcalib[flags.Analysis.small_R.jet_type])
    )
    # Add whatever collections are active in the job
    for objtype, coll in preMET_collections.items():
        configSeq.setOptionValue(f'.{objtype}', coll)

    return configSeq
