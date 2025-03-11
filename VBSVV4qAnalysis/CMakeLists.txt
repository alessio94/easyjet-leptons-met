# Declare the package
atlas_subdir(VBSVV4qAnalysis)

# Build the Athena component library
atlas_add_component(VBSVV4qAnalysis
  src/*.cxx
  src/components/VBSVV4qAnalysis_entries.cxx
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
  TriggerMatchingToolLib
  EventBookkeeperToolsLib
  EasyjetHubLib
  BoostedJetTaggersLib
)

# Install python modules, joboptions, and share content
atlas_install_scripts(
  bin/VBSVV4q-ntupler
)

atlas_install_python_modules(
  python/*.py
)
atlas_install_data(
  share/*
  data/*
  )

# atlas_install_data( data/* )
# You can access your data from code using path resolver, e.g.
# PathResolverFindCalibFile("JetMETCommon/file.txt")
