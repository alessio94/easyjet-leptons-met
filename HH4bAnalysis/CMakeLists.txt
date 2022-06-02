# Declare the package
atlas_subdir( HH4bAnalysis )

# Find an external package (i.e. ROOT)
find_package( ROOT COMPONENTS Core Tree Hist REQUIRED )

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Build the Athena component library
atlas_add_component( HH4bAnalysis
  src/*.cxx 
  src/tools/*.cxx 
  src/components/*.cxx
  LINK_LIBRARIES
    GaudiKernel
    AthenaBaseComps
    AsgTools
    # PathResolver
    xAODEventInfo
    xAODTracking
    xAODJet
    xAODMuon
    FTagAnalysisInterfacesLib
 )

# Install python modules, joboptions, and share content
atlas_install_scripts( share/*.py )
# atlas_install_python_modules( python/*.py )

# atlas_install_data( data/* )
# You can access your data from code using path resolver, e.g.
# PathResolverFindCalibFile("JetMETCommon/file.txt")
