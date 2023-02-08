# Declare the package
atlas_subdir(HH4bAnalysis)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core Tree Hist REQUIRED)

# We don't want any warnings in compilation
# disabling for now until framework is more mature
# add_compile_options(-Werror)

# Install the interface headers
atlas_add_library(HH4bAnalysisLib
  INTERFACE
  PUBLIC_HEADERS HH4bAnalysis
)

# Build the Athena component library
atlas_add_component(HH4bAnalysis
  src/VariableDumperAlg.cxx
  src/TruthParticleInformationAlg.cxx
  src/LargeJetGhostVRJetAssociationAlg.cxx
  src/LargeUFOJetGhostVRJetAssociationAlg.cxx
  src/JetSelectorAlg.cxx
  src/JetPairingAlg.cxx
  src/FinalVarsResolvedAlg.cxx
  src/FinalVarsBoostedAlg.cxx
  src/JetTruthMatcherAlg.cxx
  src/GhostAssocVRJetGetterAlg.cxx
  src/tools/NTrkVertexCounter.cxx
  src/H5FileSvc.cxx
  src/components/*.cxx
  LINK_LIBRARIES
  GaudiKernel
  AthenaBaseComps
  AsgTools
  xAODEventInfo
  xAODTracking
  xAODJet
  xAODMuon
  xAODEgamma
  SystematicsHandlesLib
  FTagAnalysisInterfacesLib
  xAODBTagging
  FourMomUtils
)

# Install python modules, joboptions, and share content
atlas_install_scripts(
  share/VariableDumperConfig.py
  share/grid_submit.py
  share/hh4b-test
)
atlas_install_python_modules(
  python/Algs
  python/Config
  python/utils
)
atlas_install_data(
  share/RunConfig.yaml
)

# atlas_install_data( data/* )
# You can access your data from code using path resolver, e.g.
# PathResolverFindCalibFile("JetMETCommon/file.txt")
