# Declare the package
atlas_subdir( HH4bAnalysis )

# Find an external package (i.e. ROOT)
find_package( ROOT COMPONENTS Core Tree Hist REQUIRED )

# We don't want any warnings in compilation
# disabling for now until framework is more mature
# add_compile_options(-Werror) 

# Install the interface headers
atlas_add_library ( HH4bAnalysisLib
  INTERFACE
  PUBLIC_HEADERS HH4bAnalysis
)

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
    xAODEgamma
    SystematicsHandlesLib
    FTagAnalysisInterfacesLib
 )

# Install python modules, joboptions, and share content
atlas_install_scripts( share/*.py )
# atlas_install_python_modules( python/*.py )

# atlas_install_data( data/* )
# You can access your data from code using path resolver, e.g.
# PathResolverFindCalibFile("JetMETCommon/file.txt")
