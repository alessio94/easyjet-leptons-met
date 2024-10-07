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
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
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
                   xAODTrigger
                   egammaUtils
                   StoreGateLib
                   SystematicsHandlesLib
                   FourMomUtils
                   TruthUtils
                   FlavorTagDiscriminants
                   FTagAnalysisInterfacesLib
                   TauAnalysisToolsLib
                   TrigDecisionToolLib
                   TrigCompositeUtilsLib
                   ${ROOT_LIBRARIES}
                   BoostedJetTaggersLib 
                   JetAnalysisInterfacesLib

)


# Build the Athena component library
atlas_add_component(EasyjetHub
  src/*.cxx
  src/components/*.cxx
  LINK_LIBRARIES EasyjetHubLib
)

# Install python modules, joboptions, and share content
atlas_install_scripts(
  bin/easyjet-ntupler
  bin/easyjet-gridsubmit
  bin/easyjet-merge-configs
  bin/easyjet-create-git-tag
)
atlas_install_python_modules(
  python/*.py
  python/algs
  python/output
  python/steering
  test
)
atlas_install_data(
  share/*
  data/*
)

# atlas_install_data( data/* )
# You can access your data from code using path resolver, e.g.
# PathResolverFindCalibFile("JetMETCommon/file.txt")
