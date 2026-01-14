# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# Configuration file for an ATLAS CMake release installation.
# It defines the following variables:
#
#  easyjet_INSTALL_DIR  - Base install directory for the release
#  easyjet_INCLUDE_DIR  - Include directory for the release
#  easyjet_LIBRARY_DIR  - Library directory for the release
#  easyjet_BINARY_DIR   - Runtime directory for the release
#  easyjet_CMAKE_DIR    - Directory holding CMake files
#  easyjet_PYTHON_DIR   - Directory holding python code
#  easyjet_VERSION      - The version number of the release
#  easyjet_TARGET_NAMES - The names of the targets provided by the
#                                      release
#  easyjet_PLATFORM     - The name of the platform of the release
#
# Note however that most of the time you should not be using any of these
# variables, but the imported targets of the project instead. Even more, in most
# cases you will want to use atlas_project(...) to handle the imported targets
# in a "smart way".

cmake_minimum_required( VERSION 3.25 )

# Add the CMake provided initialisation code.

####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was ProjectConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

####################################################################################

# Set various directory variables.
set( easyjet_INSTALL_DIR "${PACKAGE_PREFIX_DIR}/" )
set( easyjet_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include" )
set( easyjet_LIBRARY_DIR "${PACKAGE_PREFIX_DIR}/lib" )
set( easyjet_BINARY_DIR "${PACKAGE_PREFIX_DIR}/bin" )
set( easyjet_CMAKE_DIR "${PACKAGE_PREFIX_DIR}/cmake" )
set( easyjet_PYTHON_DIR "${PACKAGE_PREFIX_DIR}/python" )

# Set the version of the project.
set( easyjet_VERSION "1.0.0" )

# Set the names of the targets that the project exported.
set( easyjet_TARGET_NAMES "EasyjetHubLib;EasyjetHub;compareOutputNtuples;KinematicFitToolLib;KinematicFitTool;TruthWeightToolsLib;TruthWeightTools;VBFTaggerLib;VBFTagger;test_vbfrnn" )

# Print what project/release was found just now.
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( easyjet
   REQUIRED_VARS CMAKE_CURRENT_LIST_FILE
   VERSION_VAR easyjet_VERSION )

# The base projects that this project was built on top of:
set( easyjet_BASE_PROJECTS AthAnalysis;25.2.76 )

# The platform name used for the build.
set( easyjet_PLATFORM "x86_64-el9-gcc14-opt" )
set( ATLAS_PLATFORM "x86_64-el9-gcc14-opt" )

# Include the project-specific pre-include file, if it exists:
include( "${easyjet_CMAKE_DIR}/PreConfig.cmake" OPTIONAL )

# Include the base projects, in the order in which they were given to
# the atlas_project call.
set( _baseProjectseasyjet
   ${easyjet_BASE_PROJECTS} )
while( _baseProjectseasyjet )
   # Extract the release name and version, and then remove these entries
   # from the list:
   list( GET _baseProjectseasyjet 0
      _base_projecteasyjet )
   list( GET _baseProjectseasyjet 1
      _base_versioneasyjet )
   list( REMOVE_AT _baseProjectseasyjet 0 1 )
   # Make sure that the project version is a regular version number:
   if( NOT _base_versioneasyjet MATCHES "^[0-9]+[0-9.]*" )
      # Let's not specify a version in this case...
      message( STATUS "Using base project "
         "\"${_base_projecteasyjet}\" without "
         "its \"${_base_versioneasyjet}\" version name/number" )
      set( _base_versioneasyjet )
   endif()
   # Find the base release:
   if( easyjet_FIND_QUIETLY )
      find_package( ${_base_projecteasyjet}
         ${_base_versioneasyjet} EXACT QUIET )
   else()
      find_package( ${_base_projecteasyjet}
         ${_base_versioneasyjet} EXACT )
   endif()
endwhile()
unset( _baseProjectseasyjet )
unset( _base_projecteasyjet )
unset( _base_versioneasyjet )

# Make CMake find the release's installed modules. Optionally append the module
# library instead of prepending it. To allow the user to override the
# modules packaged with the release.
if( ATLAS_DONT_PREPEND_PROJECT_MODULES )
   list( APPEND CMAKE_MODULE_PATH "${easyjet_CMAKE_DIR}/modules" )
else()
   list( INSERT CMAKE_MODULE_PATH 0 "${easyjet_CMAKE_DIR}/modules" )
endif()
list( REMOVE_DUPLICATES CMAKE_MODULE_PATH )

# Pull in the ATLAS code:
include( AtlasFunctions )

# Include the file listing all the imported targets and options:
include(
   "${easyjet_CMAKE_DIR}/easyjetConfig-targets.cmake"
   OPTIONAL )

# Check what build mode the release was built with. And set CMAKE_BUILD_TYPE
# to that value by default. While there should only be one build mode in
# a given install area, provide an explicit preference order to the different
# build modes:
foreach( _type "Debug" "RelWithDebInfo" "Release" "MinSizeRel" "Default" )
   string( TOLOWER "${_type}" _typeLower )
   set( _fileName "${easyjet_CMAKE_DIR}/" )
   set( _fileName "${_fileName}easyjetConfig-targets" )
   set( _fileName "${_fileName}-${_typeLower}.cmake" )
   if( EXISTS "${_fileName}" )
      # Set the build type forcefully. CMake (at least at the time of writing
      # [3.19]) needs to be handled in this weird way. (One would think that if
      # the variable is deemed not-yet-set, then you wouldn't need to use the
      # FORCE keyword. But you do...)
      if( NOT CMAKE_BUILD_TYPE )
         set( CMAKE_BUILD_TYPE "${_type}"
            CACHE STRING "Build mode for the release" FORCE )
      endif()
      break()
   endif()
   unset( _fileName )
   unset( _typeLower )
endforeach()

# If the libraries need to be set up...
if( easyjet_FIND_COMPONENTS )

   # A sanity check.
   if( NOT ${easyjet_FIND_COMPONENTS} STREQUAL "INCLUDE" )
      message( WARNING "Only the 'INCLUDE' component is understood. "
         "Continuing as if 'INCLUDE' would have been specified..." )
   endif()

   # Tell the user what's happening.
   message( STATUS "Including the packages from project "
      "easyjet - 1.0.0..." )

   # Loop over the targets that this project has.
   foreach( _target ${easyjet_TARGET_NAMES} )
      # If the target exists already, then don't do aything else.
      if( TARGET ${_target} )
         continue()
      endif()
      # Check whether the target in question is known in this release.
      if( NOT TARGET easyjet::${_target} )
         message( SEND_ERROR
            "Target with name easyjet::${_target} not found" )
         continue()
      endif()
      # And now create a deep copy of it.
      atlas_copy_target( easyjet ${_target} )
   endforeach()

   # Include the base projects, in reverse order. So that the components from
   # the end of the list would get precedence over the components from the
   # front.
   set( _baseProjectseasyjet
      ${easyjet_BASE_PROJECTS} )
   while( _baseProjectseasyjet )
      # Get the last project from the list:
      list( LENGTH _baseProjectseasyjet
         _lengtheasyjet )
      math( EXPR _projNameIdxeasyjet
         "${_lengtheasyjet} - 2" )
      math( EXPR _projVersIdxeasyjet
         "${_lengtheasyjet} - 1" )
      list( GET _baseProjectseasyjet
         ${_projNameIdxeasyjet}
         _base_projecteasyjet )
      list( GET _baseProjectseasyjet
         ${_projVersIdxeasyjet}
         _base_versioneasyjet )
      list( REMOVE_AT _baseProjectseasyjet
         ${_projNameIdxeasyjet}
         ${_projVersIdxeasyjet} )
      # Find the base release:
      find_package( ${_base_projecteasyjet}
         ${_base_versioneasyjet} EXACT COMPONENTS INCLUDE QUIET )
   endwhile()
   unset( _baseProjectseasyjet )
   unset( _projNameIdxeasyjet )
   unset( _projVersIdxeasyjet )
   unset( _base_projecteasyjet )
   unset( _base_versioneasyjet )
   unset( _lengtheasyjet )

endif()

# Only do this if necessary:
if( NOT ATLAS_DONT_PREPEND_PROJECT_MODULES )
   # Make sure that after all of this, we still have this release's module
   # directory at the front of the list:
   list( INSERT CMAKE_MODULE_PATH 0 "${easyjet_CMAKE_DIR}/modules" )
   list( REMOVE_DUPLICATES CMAKE_MODULE_PATH )
endif()

# Include the project-specific post-include file, if it exists:
include( "${easyjet_CMAKE_DIR}/PostConfig.cmake" OPTIONAL )
