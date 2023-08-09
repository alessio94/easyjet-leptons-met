def validate_hh4b_analysis_prerequisites(flags):
    if flags.Analysis.do_resolved_dihiggs:
        assert flags.Analysis.do_small_R_jets
        assert not flags.Analysis.disable_calib

    if flags.Analysis.do_boosted_dihiggs:
        assert flags.Analysis.do_large_R_Topo_jets
        assert not flags.Analysis.disable_calib
