from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import makeConfig

from EasyjetHub.steering.utils.name_helper import drop_sys


def overlap_sequence(flags):
    configSeq = ConfigSequence()

    if (
        flags.Analysis.do_large_R_Topo_jets
        and flags.Analysis.do_large_R_UFO_jets
    ):
        raise ValueError('Overlap removal only works with one Large R collection')

    # TODO: May want to make these configurable
    container_names = flags.Analysis.container_names

    # Add whatever collections are active in the job to the
    # mapping of type to name
    preOR_collections = {}
    objflags = {x:f'do_{x}' for x in ['electrons','photons','muons','taus']}
    # Reproduced from MET for now
    # TODO: Make configurable
    ORselections = dict(
        electrons='loose',
        photons='tight',
        muons='medium',
        taus='baseline',
    )
    # Construct the names of the view containers with working point selection
    # We need to use the '.' style so that the algs operate on the full
    # container, and avoid incomplete decorations
    for objtype, objflag in objflags.items():
        if flags.Analysis[objflag]:
            collname = drop_sys(container_names.output[objtype])
            selection = ORselections[objtype]
            preOR_collections[objtype] = f'{collname}.{selection}'

    # Jets have different flag naming conventions
    if flags.Analysis.do_small_R_jets:
        preOR_collections['jets'] = drop_sys(container_names.output.reco4PFlowJet)

    # For the benefit of the view container creation
    original_names = {
        objtype:flags.Analysis.container_names.input[objtype]
        for objtype in ['electrons','photons','muons','taus']
    }
    original_names['jets'] = flags.Analysis.container_names.input.reco4PFlowJet

    # Large-R jets need more special handling
    if flags.Analysis.do_large_R_Topo_jets:
        preOR_collections['fatJets'] = (
            drop_sys(flags.Analysis.container_names.output.reco10TopoJet)
        )
        original_names['fatJets'] = flags.Analysis.container_names.input.reco10TopoJet
    if flags.Analysis.do_large_R_UFO_jets:
        preOR_collections['fatJets'] = (
            drop_sys(flags.Analysis.container_names.output.reco10UFOJet)
        )
        original_names['fatJets'] = flags.Analysis.container_names.input.reco10TopoJet

    # Include, and then set up the overlap analysis algorithm config:
    configSeq += makeConfig('OverlapRemoval', None)
    configSeq.setOptionValue('.inputLabel',  'preselectOR')
    configSeq.setOptionValue('.outputLabel', 'passesOR')
    for objtype, coll in preOR_collections.items():
        configSeq.setOptionValue(f'.{objtype}', coll)

    # config for TauAntiTauOR
    # configSeq.setOptionValue('.antiTauIDTauLabel', 'isIDTau')
    configSeq.setOptionValue('.antiTauBJetLabel', 'ftag_select_DL1dv01_FixedCutBEff_77')
    configSeq.setOptionValue('.antiTauLabel', 'isAntiTau')
    configSeq.setOptionValue('.doTauAntiTauJetOR', True)

    # Define output view containers after OR
    # Need to loop again because of the ConfigSequence convention
    # that you add and configure blocks one by one
    # TODO: This doesn't work now because the ConfigBlock setup
    # attaches a _%SYS% suffix to the passesOR decoration, which
    # then cannot be read by the view creator alg
    """
    for objtype,coll in preOR_collections.items():
        # Add working point selection
        makeViewSelectionConfig(
            configSeq,
            coll+'_OR',
            input=coll,
            original=original_names[objtype],
            selection='passesOR',
        )
    """

    # Old configuration, for reference
    '''
    overlapSequence = makeOverlapAnalysisSequence(
        flags.Analysis.DataType,
        inputLabel='',
        outputLabel='passesOR',
        linkOverlapObjects=False,
        doEleEleOR=False,
        doTaus=flags.Analysis.do_taus,
        enableUserPriority=False,
        antiTauBJetLabel="ftag_select_DL1dv01_FixedCutBEff_77",
        antiTauLabel="isAntiTau",
        doTauAntiTauJetOR=True,
        boostedLeptons=False,
        postfix='',
        shallowViewOutput=True,
        enableCutflow=False,
        doJets=flags.Analysis.do_small_R_jets,
        doMuons=flags.Analysis.do_muons,
        doElectrons=flags.Analysis.do_electrons,
        doPhotons=flags.Analysis.do_photons,
        doFatJets=do_fatJet_OR
    )
    '''

    return configSeq
