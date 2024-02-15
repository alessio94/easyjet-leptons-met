# Declare the package
atlas_subdir(ttHHAnalysis)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core Tree Hist REQUIRED)

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Build the Athena component library
atlas_add_component(ttHHAnalysis
  src/JetPairingAlgttHH.cxx
  src/BaselineVarsttHHAlg.cxx
  src/ttHHSelectorAlg.cxx
  src/components/ttHHAnalysis_entries.cxx
  LINK_LIBRARIES
  AthenaBaseComps
  AsgTools
  AthContainers
  xAODEventInfo
  xAODEgamma
  xAODMuon
  xAODJet
  SystematicsHandlesLib
  FourMomUtils
  TruthUtils
  EasyjetHubLib
)

# Install python modules, joboptions, and share content
atlas_install_scripts(
  bin/ttHH-ntupler
)
atlas_install_runtime( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_install_python_modules(
  python/ttHH_config.py
)
atlas_install_data(
  share/*.yaml
)

# atlas_install_data( data/* )
# You can access your data from code using path resolver, e.g.
# PathResolverFindCalibFile("JetMETCommon/file.txt")
