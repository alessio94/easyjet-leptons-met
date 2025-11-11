# Declare the package
atlas_subdir(XbbCalib)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core Tree Hist REQUIRED)

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Build the Athena component library
atlas_add_component(XbbCalib
  src/FourVectorOutBlock.cxx
  src/BaselineVarsttCalibAlg.cxx
  src/BaselineVarsZbbjCalibAlg.cxx
  src/BaselineVarsZlljCalibAlg.cxx
  src/BaselineVarsZllyCalibAlg.cxx
  src/ZbbjCalibSelectorAlg.cxx
  src/ZlljCalibSelectorAlg.cxx
  src/ZllyCalibSelectorAlg.cxx
  src/ttCalibSelectorAlg.cxx
  src/components/XbbCalib_entries.cxx
  src/ZbbyCalibSelectorAlg.cxx
  src/BaselineVarsZbbyCalibAlg.cxx
  src/ZcandSelectorAlg.cxx

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
  python/ttCalib_config.py
  python/ZbbyCalib_config.py
  python/ZbbjCalib_config.py
  python/ZlljCalib_config.py
  python/ZllyCalib_config.py
)
atlas_install_runtime( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data(
  share/*
)
