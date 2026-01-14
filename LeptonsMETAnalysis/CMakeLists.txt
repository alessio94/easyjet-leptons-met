# Declare the package
atlas_subdir(LeptonsMETAnalysis)

# Install share content (configuration files)
atlas_install_data(
  share/*
)

# Note: This is a minimal configuration-only package
# Add C++ algorithms, python modules, and executables as needed:
#
# atlas_add_component(LeptonsMETAnalysis
#   src/YourAlgorithm.cxx
#   src/components/LeptonsMETAnalysis_entries.cxx
#   LINK_LIBRARIES
#   AthenaBaseComps
#   SystematicsHandlesLib
#   xAODJet
#   xAODMuon
#   xAODEgamma
#   xAODMissingET
#   xAODEventInfo
# )
#
# atlas_install_scripts(
#   bin/leptonsmet-ntupler
# )
#
# atlas_install_python_modules(
#   python/leptonsmet_config.py
# )
