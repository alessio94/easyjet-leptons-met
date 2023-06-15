# Declare the package
atlas_subdir(EasyjetHub)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core Tree Hist REQUIRED)

find_package(nlohmann_json REQUIRED)

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Build the Athena component library
atlas_add_component(EasyjetHub
  src/EventCounterAlg.cxx
  src/GhostAssocVRJetGetterAlg.cxx
  src/JetSelectorAlg.cxx
  src/PhotonSelectorAlg.cxx
  src/LargeJetGhostVRJetAssociationAlg.cxx
  src/TruthParentDecoratorAlg.cxx
  src/H5FileSvc.cxx
  src/components/*.cxx
  LINK_LIBRARIES
  #GaudiKernel
  AthenaBaseComps
  AthContainers
  AsgTools
  xAODBase
  xAODEventInfo
  xAODJet
  xAODTruth
  StoreGateLib
  SystematicsHandlesLib
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
  python/*.py
  python/algs
  python/output
  python/steering
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
