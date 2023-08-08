from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def histograms_cfg(flags, jets=None, out_path=None):
    if not jets:
        jets = flags.Analysis.container_names.output.reco4PFlowJet
    cfg = ComponentAccumulator()
    if path := out_path or flags.Analysis.output_hists:
        output = CompFactory.H5FileSvc(path=str(path))
        cfg.addService(output)
        cfg.addEventAlgo(
            CompFactory.HH4B.JetBoostHistogramsAlg(
                name="JetBoostHistogramsAlg",
                jetsIn=str(jets),
                output=output,
            )
        )
        cfg.addEventAlgo(
            CompFactory.HH4B.MassPlaneBoostHistogramsAlg(
                name="MassPlaneBoostHistogramAlg",
                matchingMethods=["2301_03212"],
                jetsIn=str(jets),
                output=output,
            )
        )
        if flags.Input.isMC:
            cfg.addEventAlgo(
                CompFactory.HH4B.MassPlaneBoostHistogramsAlg(
                    name="MassPlaneTruthMatchedBoostHistogramAlg",
                    matchingMethods=["2301_03212"],
                    truthMatchingPrefix="parentHiggs",
                    jetsIn=str(jets),
                    output=output,
                )
            )
    return cfg
