#
# This is a template for a CMakeLists.txt file that can be used in a client
# project (work area) to set up building ATLAS packages against the configured
# release.
#

# Set the minimum required CMake version:
cmake_minimum_required( VERSION 3.7 FATAL_ERROR )
project( HH4bAnalysis VERSION 1.0.0 )

find_package( AthAnalysis 22.2 REQUIRED )

# Set up CTest:
atlas_ctest_setup()

# Set up a work directory project:
atlas_project( HH4bAnalysis 1.0.0
   USE AthAnalysis ${AthAnalysis_VERSION} )

# Set up the runtime environment setup script(s):
lcg_generate_env( SH_FILE ${CMAKE_BINARY_DIR}/${ATLAS_PLATFORM}/env_setup.sh )
install( FILES ${CMAKE_BINARY_DIR}/${ATLAS_PLATFORM}/env_setup.sh
   DESTINATION . )

# Set up CPack:
atlas_cpack_setup()
