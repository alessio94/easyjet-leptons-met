# Declare the package
atlas_subdir(ExampleAnalysis)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core REQUIRED)

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Build the Athena component library
atlas_add_component(ExampleAnalysis
  src/BaselineVarsExampleAlg.cxx
  src/MjjExampleAlg.cxx
  src/components/ExampleAnalysis_entries.cxx
  LINK_LIBRARIES
  AthenaBaseComps
  SystematicsHandlesLib
  xAODJet
  xAODMuon
  xAODEgamma
  xAODEventInfo
)

# Install python modules, joboptions, and share content
atlas_install_scripts(
  bin/example-ntupler
  )
#atlas_install_runtime( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_install_python_modules(
  python/example_config.py
)
atlas_install_data(
  #data/*
  share/*
)

# atlas_install_data( data/* )
# You can access your data from code using path resolver, e.g.
# PathResolverFindCalibFile("JetMETCommon/file.txt")
