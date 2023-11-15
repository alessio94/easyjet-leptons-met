# Declare the package
atlas_subdir(bbyyAnalysis)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core Tree Hist REQUIRED)

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Build the Athena component library
atlas_add_component(bbyyAnalysis
  src/BaselineVarsyybbAlg.cxx
  src/SelectionFlagsyybbAlg.cxx
  src/components/bbyyAnalysis_entries.cxx
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
  EasyjetHubLib
)

# Install python modules, joboptions, and share content
atlas_install_scripts(
  bin/bbyy-ntupler
)

atlas_install_python_modules(
  python/yybb_config.py
)
atlas_install_data(
  share/yybb-base-config.yaml
  share/RunConfig-PHYS-yybb.yaml
  share/RunConfig-PHYSLITE-yybb.yaml
  share/trigger.yaml
)

# atlas_install_data( data/* )
# You can access your data from code using path resolver, e.g.
# PathResolverFindCalibFile("JetMETCommon/file.txt")
