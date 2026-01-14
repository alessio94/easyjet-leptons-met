# Declare the package
atlas_subdir(STXSAlgorithm)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core Tree Hist REQUIRED)

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Build the Athena component library
atlas_add_component(STXSAlgorithm
  src/*.cxx
  src/components/*.cxx
  LINK_LIBRARIES
  AthenaBaseComps
  AthContainers
  AthLinks
  AsgTools
  AsgDataHandlesLib
  xAODBase
  xAODEventInfo
  TruthWeightToolsLib
  ${ROOT_LIBRARIES}
)

