# Declare the package
atlas_subdir(jjjjAnalysis)

# Build the Athena component library
atlas_add_component(jjjjAnalysis
  src/*.cxx
  src/components/jjjjAnalysis_entries.cxx
  LINK_LIBRARIES
  AthenaBaseComps
  AsgTools
  AthContainers
  xAODEventInfo
  xAODJet
  xAODTracking
  xAODTruth
  SystematicsHandlesLib
  FourMomUtils
  TruthUtils
  TriggerMatchingToolLib
  EventBookkeeperToolsLib
  EasyjetHubLib
)

# Install python modules, joboptions, and share content
atlas_install_scripts(
  bin/jjjj-ntupler
)

atlas_install_python_modules(
  python/*.py
)
atlas_install_data(
  share/*
  )

