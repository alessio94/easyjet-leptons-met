# Declare the package
atlas_subdir(EasyjetTests)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core Tree Hist Gpad Graf MathCore REQUIRED)

# We don't want any warnings in compilation
add_compile_options(-Werror)

atlas_add_executable( compareOutputNtuples
  util/compareOutputNtuples.cxx
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES}
)

atlas_install_scripts(
  bin/easyjet-test
  bin/metadata-check
)

atlas_install_generic(
  tab-complete.bash
  DESTINATION .
)
