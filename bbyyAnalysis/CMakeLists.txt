# Declare the package
atlas_subdir(bbyyAnalysis)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core Tree Hist Matrix MathCore REQUIRED)
find_package( lwtnn )

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Build the Athena component library
atlas_add_component(bbyyAnalysis
  src/MbbKinFitDecoratorAlg.cxx
  src/BaselineVarsbbyyAlg.cxx
  src/LeptonVarsbbyyAlg.cxx
  src/bbyySelectorAlg.cxx
  src/bbyyFilterDalitzAlg.cxx
  src/components/bbyyAnalysis_entries.cxx
  src/ResonantPNNbbyyAlg.cxx
  src/BoostedVarsbbyyAlg.cxx
  LINK_LIBRARIES ${ROOT_LIBRARIES}
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
  EasyjetHubLib
  PathResolver
  EventBookkeeperToolsLib
  TriggerMatchingToolLib
  MVAUtils
  KinematicFitToolLib
)

# Install python modules, joboptions, and share content
atlas_install_scripts(
  bin/bbyy-ntupler
  )
atlas_install_runtime( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_install_python_modules(
  python/bbyy_config.py
)
atlas_install_data(
  data/*
  share/*
)

# atlas_install_data( data/* )
# You can access your data from code using path resolver, e.g.
# PathResolverFindCalibFile("JetMETCommon/file.txt")
