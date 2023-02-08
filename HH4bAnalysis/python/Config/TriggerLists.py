TriggerLists = {
    "HH4bResolved": {
        2015: [
            # 2015 (not used in the resolved analysis):
            "HLT_2j35_btight_2j35_L13J25.0ETA23",
            "HLT_j100_2j55_bmedium",
            "HLT_j225_bloose",
        ],
        2016: [
            "HLT_j175_bmv2c2040_split",
            "HLT_j225_bmv2c2060_split",
            "HLT_2j35_bmv2c2060_split_2j35_L14J15.0ETA25",
            "HLT_2j55_bmv2c2060_split_ht300_L14J15",
            "HLT_j100_2j55_bmv2c2060_split",
            "HLT_j150_bmv2c2060_split_j50_bmv2c2060_split",
        ],
        2017: [
            "HLT_j175_gsc225_bmv2c1040_split",  # also 2018
            "HLT_j225_gsc275_bmv2c1060_split",
            "HLT_j225_gsc300_bmv2c1070_split",
            "HLT_j150_gsc175_bmv2c1060_split_j45_gsc60_bmv2c1060_split",  # and 2018
            "HLT_j110_gsc150_boffperf_split_2j35_gsc55_bmv2c1070_split_L1J85_3J30",
            "HLT_2j15_gsc35_bmv2c1040_split_2j15_gsc35_boffperf_split_L14J15.0ETA25",
            "HLT_2j35_gsc55_bmv2c1050_split_ht300_L1HT190-J15s5.ETA21",
            "HLT_2j35_gsc55_bmv2c1060_split_ht300_L1HT190-J15s5.ETA21",  # (below 1.5e34) # noqa
        ],
        2018: [
            "HLT_j225_gsc275_bhmv2c1060_split",
            "HLT_j225_gsc300_bmv2c1070_split",
            "HLT_j110_gsc150_boffperf_split_2j45_gsc55_bmv2c1070_split_L1J85_3J30",
            "HLT_2j35_bmv2c1060_split_2j35_L14J15.0ETA25",
            "HLT_2j45_gsc55_bmv2c1050_split_ht300_L1HT190-J15s5.ETA21",
            "HLT_j175_gsc225_bmv2c1040_split",  # also 2017
            "HLT_j150_gsc175_bmv2c1060_split_j45_gsc60_bmv2c1060_split",  # and 2017
        ],
        2022: [
            "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25",  # noqa
            "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1r85_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25",  # noqa
            "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25",  # noqa
            "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1r77_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25",  # noqa
            "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20",  # noqa
            "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1r77_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20",  # noqa
            "HLT_2j35c_020jvt_bdl1d60_2j35c_020jvt_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25",  # noqa
            "HLT_2j35c_020jvt_bdl1r60_2j35c_020jvt_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25",  # noqa
            "HLT_j150_2j55_0eta290_020jvt_bdl1d70_pf_ftf_preselj80XX2j45b90_L1J85_3J30",  # noqa
            "HLT_j150_2j55_0eta290_020jvt_bdl1r70_pf_ftf_preselj80XX2j45b90_L1J85_3J30",  # noqa
        ],
    },
    "HH4bBoosted": {
        2015: [
            "HLT_j360_a10_lcw_sub_L1J100",
        ],
        2016: [
            "HLT_j420_a10_lcw_L1J100",
            "HLT_j420_a10r_L1J100",
        ],
        2017: [
            "HLT_j420_a10t_lcw_jes_40smcINF_L1J100",
            "HLT_j390_a10t_lcw_jes_30smcINF_L1J100",  # also 2018
            "HLT_j460_a10t_lcw_jes_L1J100",
        ],
        2018: [
            "HLT_j420_a10t_lcw_jes_35smcINF_L1J100",
            "HLT_j390_a10t_lcw_jes_30smcINF_L1J100",  # also 2017
            "HLT_j420_a10t_lcw_jes_35smcINF_L1SC111",  # add. triggers to test
            "HLT_j460_a10r_L1SC111",
            "HLT_j460_a10r_L1J100",
            "HLT_j460_a10_lcw_subjes_L1SC111",
            "HLT_j460_a10_lcw_subjes_L1J100",
            "HLT_j460_a10t_lcw_jes_L1SC111",
            "HLT_j460_a10t_lcw_jes_L1J100",
        ],
        2022: [
            # LCTopo/PFlow without mass cut
            "HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1J100",
            "HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15",
            "HLT_j460_a10t_lcw_jes_L1J100",
            "HLT_j460_a10t_lcw_jes_L1SC111-CJ15",
            # LCTopo/PFlow with mass cut
            "HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1J100",
            "HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15",
            "HLT_j420_35smcINF_a10t_lcw_jes_L1J100",
            "HLT_j420_35smcINF_a10t_lcw_jes_L1SC111-CJ15",
        ],
    },
    "JetTrigEffStudy": {
        2015: [
            # Reference single electron trigger
            "HLT_e26_lhtight_nod0_ivarloose",
            # Reference single muon trigger
            "HLT_mu26_ivarmedium",
        ],
        2016: [
            # Reference single electron trigger
            "HLT_e26_lhtight_nod0_ivarloose",
            # Reference single muon trigger
            "HLT_mu26_ivarmedium",
        ],
        2017: [
            # Reference single electron trigger
            "HLT_e26_lhtight_nod0_ivarloose",
            # Reference single muon trigger
            "HLT_mu26_ivarmedium",
        ],
        2018: [
            # Reference single electron trigger
            "HLT_e26_lhtight_nod0_ivarloose",
            # Reference single muon trigger
            "HLT_mu26_ivarmedium",
        ],
        2022: [
            # Reference single electron trigger
            "HLT_e26_lhtight_ivarloose_L1EM22VHI",
            # Reference single muon trigger
            "HLT_mu24_ivarmedium_L1MU14FCH",
            # Various multijet triggers
            "HLT_4j120_L13J50",
            "HLT_4j115_pf_ftf_presel4j85_L13J50",
            "HLT_5j70c_pf_ftf_presel5c50_L14J15",
            "HLT_5j85_pf_ftf_presel5j50_L14J15",
            "HLT_6j35c_020jvt_pf_ftf_presel6c25_L14J15",
            "HLT_6j55c_pf_ftf_presel6j40_L14J15",
            "HLT_6j70_pf_ftf_presel6j40_L14J15",
            "HLT_7j45_pf_ftf_presel7j30_L14J15",
            "HLT_10j40_pf_ftf_presel7j30_L14J15",
        ],
    },
}
