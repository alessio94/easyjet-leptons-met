# Declare the package
atlas_subdir(bbVVAnalysis)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core Tree Hist REQUIRED)

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Build the Athena component library
atlas_add_component(bbVVAnalysis
  src/*.cxx
  src/components/bbVVAnalysis_entries.cxx
  LINK_LIBRARIES
  AthenaBaseComps
  AsgTools
  AthContainers
  xAODEventInfo
  xAODEgamma
  xAODMuon
  xAODJet
  xAODTracking
  xAODTruth
  SystematicsHandlesLib
  FourMomUtils
  TriggerMatchingToolLib
  TruthUtils
  EventBookkeeperToolsLib
  EasyjetHubLib
)

# Install python modules, joboptions, and share content
atlas_install_scripts(
  bin/bbVV-ntupler
)

atlas_install_python_modules(
  python/bbVV_config.py
  python/VBF_config.py
)
atlas_install_data(
  share/*.yaml
  datasets/*
)

# atlas_install_data( data/* )
# You can access your data from code using path resolver, e.g.
# PathResolverFindCalibFile("JetMETCommon/file.txt")
