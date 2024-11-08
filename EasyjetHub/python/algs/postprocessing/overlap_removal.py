from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory

from EasyjetHub.steering.utils.name_helper import drop_sys


def overlap_sequence(flags):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    if (
        flags.Analysis.do_large_R_Topo_jets
        and flags.Analysis.do_large_R_UFO_jets
    ):
        raise ValueError('Overlap removal only works with one Large R collection')

    if flags.Analysis.do_small_R_jets:
        if (
            flags.Analysis.Small_R_jet.jet_type not in
            {'reco4PFlowJet', 'reco4EMTopoJet'}
        ):
            raise ValueError(
                "Specified small-R jet type is invalid"
            )

    container_names = flags.Analysis.container_names

    # Add whatever collections are active in the job to the
    # mapping of type to name
    preOR_collections = {}
    objflags = {x: f'do_{x}' for x in ['electrons', 'photons', 'muons', 'taus']}
    ORselections = dict(
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
            selection = ORselections[objtype]
            preOR_collections[objtype] = f'{collname}.{selection}'

    # For the benefit of the view container creation
    original_names = {
        objtype: container_names.input[objtype]
        for objtype in ['electrons', 'photons', 'muons', 'taus']
    }

    # Jets have different flag naming conventions
    if flags.Analysis.do_small_R_jets:
        preOR_collections['jets'] = drop_sys(
            container_names.output[flags.Analysis.Small_R_jet.jet_type]
        ) + '.selectPtEta'
        original_names['jets'] = container_names.input[
            flags.Analysis.Small_R_jet.jet_type
        ]

    # Large-R jets need more special handling
    if flags.Analysis.OverlapRemoval.do_large_R_jets:
        if flags.Analysis.do_large_R_Topo_jets:
            preOR_collections['fatJets'] = (
                drop_sys(container_names.output.reco10TopoJet)
            )
            original_names['fatJets'] = container_names.input.reco10TopoJet
        if flags.Analysis.do_large_R_UFO_jets:
            preOR_collections['fatJets'] = (
                drop_sys(container_names.output.reco10UFOJet)
            )
            original_names['fatJets'] = container_names.input.reco10UFOJet

    # Include, and then set up the overlap analysis algorithm config:
    configSeq += makeConfig('OverlapRemoval')
    configSeq.setOptionValue('.inputLabel', 'preselectOR')
    configSeq.setOptionValue('.outputLabel', 'passesOR')
    configSeq.setOptionValue('.addToAllSelections', True)
    configSeq.setOptionValue('.doJetFatJetOR',
                             flags.Analysis.OverlapRemoval.do_small_R_jet_large_R_jet)
    for objtype, coll in preOR_collections.items():
        configSeq.setOptionValue(f'.{objtype}', coll)

    # Config for TauAntiTauOR
    # Should be equivalent to regular tau-jet OR for taus + b-jets
    # Differences expected for light-jets
    # => light jets should not be used for HH orthogonality
    if flags.Analysis.OverlapRemoval.doTauAntiTauJet:
        configSeq.setOptionValue('.antiTauIDTauLabel', 'isIDTau')
        configSeq.setOptionValue('.antiTauBJetLabel',
                                 "ftag_select_" + flags.Analysis.Small_R_jet.btag_wp)
        configSeq.setOptionValue('.antiTauLabel', 'isAntiTau')
        configSeq.setOptionValue('.doTauAntiTauJetOR', True)
    return configSeq
