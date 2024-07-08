# Declare the package
atlas_subdir(XbbCalib)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core Tree Hist REQUIRED)

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Build the Athena component library
atlas_add_component(XbbCalib
  src/BaselineVarsXbbCalibAlg.cxx
  src/XbbCalibSelectorAlg.cxx
  src/components/XbbCalib_entries.cxx
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
  TriggerMatchingToolLib
)

# Install python modules, joboptions, and share content
atlas_install_scripts(
  bin/xbbcalib-ntupler
)
atlas_install_python_modules(
  python/XbbCalib_config.py
)
atlas_install_data(
  share/*.yaml
)
