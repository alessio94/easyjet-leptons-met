# Declare the package
atlas_subdir(HH4bAnalysis)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core Tree Hist REQUIRED)

find_package(nlohmann_json REQUIRED)

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Build the Athena component library
atlas_add_component(HH4bAnalysis
  src/EventCounterAlg.cxx
  src/TruthParticleInformationAlg.cxx
  src/LargeJetGhostVRJetAssociationAlg.cxx
  src/JetSelectorAlg.cxx
  src/JetPairingAlg.cxx
  src/BaselineVarsResolvedAlg.cxx
  src/BaselineVarsBoostedAlg.cxx
  src/GhostAssocVRJetGetterAlg.cxx
  src/H5FileSvc.cxx
  src/TruthParentDecoratorAlg.cxx
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
  TruthUtils
  FlavorTagDiscriminants
)

# Install python modules, joboptions, and share content
atlas_install_scripts(
  bin/easyjet-ntupler
  bin/easyjet-gridsubmit
  bin/easyjet-test
  bin/metadata-check
)
atlas_install_python_modules(
  python/cpalgs
  python/config
  python/output_branches
  python/utils
)
atlas_install_data(
  share/*.yaml
)
atlas_install_generic(
  tab-complete.bash
  DESTINATION .
)

# atlas_install_data( data/* )
# You can access your data from code using path resolver, e.g.
# PathResolverFindCalibFile("JetMETCommon/file.txt")
