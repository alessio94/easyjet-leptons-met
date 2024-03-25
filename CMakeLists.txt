#
# This is a template for a CMakeLists.txt file that can be used in a client
# project (work area) to set up building ATLAS packages against the configured
# release.
#

# Set the minimum required CMake version:
cmake_minimum_required( VERSION 3.7 FATAL_ERROR )
project( easyjet VERSION 1.0.0 )

find_package( AthAnalysis 25.2 REQUIRED )

# Set up CTest:
atlas_ctest_setup()

# Generate a compile_commands.json file, which VS Code can use to interpret
# our code correctly.
set( CMAKE_EXPORT_COMPILE_COMMANDS TRUE CACHE BOOL
   "Create compile_commands.json" FORCE )

# Set up a work directory project:
atlas_project( easyjet 1.0.0
   USE AthAnalysis ${AthAnalysis_VERSION}
   PROJECT_ROOT ${CMAKE_SOURCE_DIR}/
)

# Set up the runtime environment setup script(s):
lcg_generate_env( SH_FILE ${CMAKE_BINARY_DIR}/${ATLAS_PLATFORM}/env_setup.sh )
message(STATUS "Appending EasyJet tab complete to setup")
file(APPEND ${CMAKE_BINARY_DIR}/${ATLAS_PLATFORM}/env_setup.sh.in
  "# custom EasyJet scripts:\n"
  "TC=\${easyjet_DIR}/tab-complete.bash\n"
  "[ -f $TC ] && . \${easyjet_DIR}/tab-complete.bash\n"
  "unset TC\n"
  )
install( FILES ${CMAKE_BINARY_DIR}/${ATLAS_PLATFORM}/env_setup.sh
   DESTINATION . )

# Setup IDE integration:
set( ATLAS_ENABLE_IDE_HELPERS OFF CACHE BOOL "Enable IDE helpers" )
if( ATLAS_ENABLE_IDE_HELPERS )
   set( ATLAS_IDEHELPERSCRIPTS_SETUP
      "${CMAKE_SOURCE_DIR}/.vscode/IDEHelperScripts/Setup.cmake"
      CACHE FILEPATH "Setup file for the IDE / VS Code helpers" )
   include( "${ATLAS_IDEHELPERSCRIPTS_SETUP}" )
endif()

# Set up CPack:
atlas_cpack_setup()
