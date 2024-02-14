# Declare the package
atlas_subdir(EasyjetHub)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core Tree Hist REQUIRED)

find_package(nlohmann_json REQUIRED)

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Component(s) in the package:
atlas_add_library( EasyjetHubLib
		   Easyjet/CutManager.h
		   src/CutManager.cxx
                   PUBLIC_HEADERS EasyjetHub
                   LINK_LIBRARIES
                   AthenaBaseComps
                   AthContainers
                   AthLinks
                   AsgTools
                   xAODBase
                   xAODBTagging
                   xAODEventInfo
                   xAODJet
                   xAODTau
                   xAODTruth
                   egammaUtils
                   StoreGateLib
                   SystematicsHandlesLib
                   FourMomUtils
                   TruthUtils
                   FlavorTagDiscriminants
)


# Build the Athena component library
atlas_add_component(EasyjetHub
  src/BTaggingDecoratorAlg.cxx
  src/EventCounterAlg.cxx
  src/GhostAssocVRJetGetterAlg.cxx
  src/JetSelectorAlg.cxx
  src/PhotonSelectorAlg.cxx
  src/MuonSelectorAlg.cxx
  src/ElectronSelectorAlg.cxx
  src/TauSelectorAlg.cxx
  src/LargeJetGhostVRJetAssociationAlg.cxx
  src/TruthParentDecoratorAlg.cxx
  src/TruthParticleInformationAlg.cxx
  src/H5FileSvc.cxx
  src/JetDeepCopyAlg.cxx
  src/TauDecoratorAlg.cxx
  src/EventInfoGlobalAlg.cxx
  src/OrthogonalityAlg.cxx
  src/components/*.cxx
  LINK_LIBRARIES EasyjetHubLib
)

# Install python modules, joboptions, and share content
atlas_install_scripts(
  bin/easyjet-ntupler
  bin/easyjet-gridsubmit
  bin/easyjet-merge-configs
)
atlas_install_python_modules(
  python/*.py
  python/algs
  python/output
  python/steering
  python/utils
  test
)
atlas_install_data(
  share/*.yaml
)

# atlas_install_data( data/* )
# You can access your data from code using path resolver, e.g.
# PathResolverFindCalibFile("JetMETCommon/file.txt")
