# Declare the package
atlas_subdir(EasyjetPlus)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core RIO Tree Hist REQUIRED)

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Build the Athena component library
atlas_add_component(EasyjetPlus
  src/*.cxx
  src/components/EasyjetPlus_entries.cxx
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} GaudiKernel AthenaBaseComps AsgTools PMGAnalysisInterfacesLib
)

atlas_install_runtime( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )



