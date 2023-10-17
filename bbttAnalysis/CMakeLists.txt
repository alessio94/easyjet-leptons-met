# Declare the package
atlas_subdir(bbttAnalysis)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core Tree Hist REQUIRED)

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Build the Athena component library
atlas_add_component(bbttAnalysis
  src/BaselineVarsbbttAlg.cxx
  src/MMCDecoratorAlg.cxx
  src/HHbbttSelectorAlg.cxx
  src/components/bbttAnalysis_entries.cxx
  LINK_LIBRARIES
  AthenaBaseComps
  AsgTools
  AthContainers
  xAODEventInfo
  xAODEgamma
  xAODMuon
  xAODTau
  xAODJet
  xAODTracking
  xAODTruth
  SystematicsHandlesLib
  FourMomUtils
  TruthUtils
  DiTauMassToolsLib
  EventBookkeeperToolsLib
)

# Install python modules, joboptions, and share content
atlas_install_scripts(
  bin/bbtt-ntupler
)

atlas_install_python_modules(
  python/bbtt_config.py
)
atlas_install_data(
  share/*.yaml
)

# atlas_install_data( data/* )
# You can access your data from code using path resolver, e.g.
# PathResolverFindCalibFile("JetMETCommon/file.txt")
