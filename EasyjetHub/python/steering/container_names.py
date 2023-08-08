from EasyjetHub.steering.argument_parser import dict_to_flags
from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def define_output_container_name_flags(flags: AthConfigFlags) -> AthConfigFlags:
    """
    Retrieve the input container names for calibration from
    the yaml configuration. Different yaml configs can be
    provided to support different data formats.
    The input containers are then mapped to output names
    corresponding to the post-calibration containers.
    """
    inputs = flags.Analysis.container_names.input

    if flags.Analysis.disable_calib:
        # If not running calibration algs in PHYSLITE, we can just
        # clone the inputs to the outputs
        # Re-convert to make a copy
        # Truth is not calibrated, so skip
        outputs = dict_to_flags(
            {k:getattr(inputs,k) for k in inputs.dict_keys if 'truth' not in k}
        )
    else:
        # Convert the input to output containers as defined in the
        # CP alg sequences
        output_dict = dict(
            reco4PFlowJet=f"Analysis{inputs.reco4PFlowJet}_%SYS%",
            reco10UFOJet=f"Analysis{inputs.reco10UFOJet}_%SYS%",
            reco10TopoJet=f"Analysis{inputs.reco10TopoJet}_%SYS%",
            vrJet=f"Analysis{inputs.vrJet}_%SYS%",
            muons=f"Analysis{inputs.muons}_%SYS%",
            electrons=f"Analysis{inputs.electrons}_%SYS%",
            photons=f"Analysis{inputs.photons}_%SYS%",
            taus=f"Analysis{inputs.taus}_%SYS%",
            #
            met="AnalysisMET_%SYS%",
        )
        # Disable collections unavailable in PHYSLITE
        if flags.Input.isPHYSLITE:
            output_dict.update(dict(
                reco10TopoJet='',
                vrJet='',
            ))
        outputs = dict_to_flags(output_dict)

    # This is a truth collection created by easyjets
    outputs.addFlag("truthHHParticles", "TruthDiHiggsParticles")

    return outputs
