from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory

from EasyjetHub.steering.utils.name_helper import drop_sys


def thinning_sequence(flags):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    # Add whatever collections are active in the job to the
    # mapping of type to name
    objflags = {x: f'do_{x}' for x in ['electrons', 'photons', 'muons', 'taus']}
    selections = dict(
        electrons=f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}',
        photons=f'{flags.Analysis.Photon.ID}_{flags.Analysis.Photon.Iso}',
        muons=f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}',
        taus=flags.Analysis.Tau.ID,
    )

    for objtype, objflag in objflags.items():
        if flags.Analysis[objflag]:
            configSeq += makeConfig('Thinning')
            configSeq.setOptionValue('.containerName', drop_sys(
                flags.Analysis.container_names.output[objtype]))
            configSeq.setOptionValue(
                '.selectionName', 'selectPtEta&&' + selections[objtype])
            configSeq.setOptionValue('.postfix', 'thin')

    if flags.Analysis.do_small_R_jets and flags.Analysis.Small_R_jet.do_thinning:
        jet_type = flags.Analysis.Small_R_jet.jet_type
        selection_string = 'selectPtEta'
        if jet_type != 'reco4EMTopoJet':
            selection_string += '&&baselineJvt'
            if flags.Analysis.Small_R_jet.useFJvt:
                selection_string += '&&baselineFJvt'
        configSeq += makeConfig('Thinning')
        configSeq.setOptionValue('.containerName', drop_sys(
            flags.Analysis.container_names.output[jet_type]))
        configSeq.setOptionValue('.selectionName', selection_string)
        configSeq.setOptionValue('.postfix', 'thin')
        if flags.Analysis.Small_R_jet.save_all_jets:
            # Dump inclusive collection
            # Save thinned collection with a different name
            configSeq.setOptionValue(
                '.outputName',
                flags.Analysis.container_names.output[jet_type].replace(
                    '_%SYS%', '_thin'))

    if flags.Analysis.do_large_R_Topo_jets:
        configSeq += makeConfig('Thinning')
        configSeq.setOptionValue('.containerName', drop_sys(
            flags.Analysis.container_names.output.reco10TopoJet))
        configSeq.setOptionValue('.selectionName', 'selectPtEta')
        configSeq.setOptionValue('.postfix', 'thin')

    if flags.Analysis.do_large_R_UFO_jets and flags.Analysis.Large_R_jet.do_thinning:
        configSeq += makeConfig('Thinning')
        configSeq.setOptionValue('.containerName', drop_sys(
            flags.Analysis.container_names.output.reco10UFOJet))
        configSeq.setOptionValue('.selectionName', 'selectPtEta')
        configSeq.setOptionValue('.postfix', 'thin')

    return configSeq
