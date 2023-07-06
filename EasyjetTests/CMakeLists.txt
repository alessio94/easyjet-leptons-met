# Declare the package
atlas_subdir(EasyjetTests)

# We don't want any warnings in compilation
add_compile_options(-Werror)

atlas_install_scripts(
  bin/easyjet-test
  bin/metadata-check
)

atlas_install_generic(
  tab-complete.bash
  DESTINATION .
)
