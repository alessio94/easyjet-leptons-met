# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# This script is used to tweak the exported file created by CMake for
# describing the installed targets of a release, not to treat it as a
# fatal error if one of the targets is missing. Which can very much
# be the case in a nightly, if a package fails building. But it should
# not stop the downstream projects from building all together.
#
# This is a tweaked version of the script, for AthAnalysisExternals.
#

# Look for specific target files:
set( _fileName "$ENV{DESTDIR}/${CMAKE_INSTALL_PREFIX}/" )
set( _fileName "${_fileName}cmake/" )
set( _fileName "${_fileName}easyjetConfig-targets-*.cmake" )
file( GLOB _targetFiles ${_fileName} )
foreach( _file ${_targetFiles} )
   # Add a line at the end of each file that clears out the
   # _IMPORT_CHECK_TARGETS variable. So that no targets would be checked
   # at the build configuration time.
   file( APPEND "${_file}"
      "# Don't check the imported targets:\n"
      "set(_IMPORT_CHECK_TARGETS)\n" )
endforeach()

# The name of the main file to sanitize:
set( _fileName "$ENV{DESTDIR}/${CMAKE_INSTALL_PREFIX}/" )
set( _fileName "${_fileName}cmake/" )
set( _fileName "${_fileName}easyjetConfig-targets.cmake" )

# Read in the generated targets file:
file( READ ${_fileName} _targetsContent )

# Do all the necessary string replacements. It has to be done in this
# complicated way, as semicolons in the string are interpreted by string(...)
# as list delimeters. And are removed if I give the content as a single
# value.
set( _outputContent )
foreach( _element ${_targetsContent} )
   # Lower the level of fatal messages to warning. To make nightlies with
   # package failures still useful for code development.
   string( REPLACE "FATAL_ERROR" "WARNING" _newContent ${_element} )
   # Make the paths coming from LCG be treated as absolute paths, and not
   # relative ones.
   string( REPLACE "\${_IMPORT_PREFIX}/\\\${LCG_RELEASE_BASE}"
      "\\\${LCG_RELEASE_BASE}" _newContent ${_newContent} )
   string( REPLACE "\\\${LCG_RELEASE_BASE}"
      "\${LCG_RELEASE_BASE}" _newContent ${_newContent} )
   # Make the directories pointing at AthAnalysisExternals be relative paths:
   string( REPLACE
      "/cvmfs/atlas.cern.ch/repo/sw/software/25.2/AthAnalysisExternals/25.2.76/InstallArea/x86_64-el9-gcc14-opt/"
      "\${easyjet_INSTALL_DIR}/../../../../AthAnalysisExternals/\${AthAnalysisExternals_VERSION}/InstallArea/\${AthAnalysisExternals_PLATFORM}/"
      _newContent ${_newContent} )
   # Append it to the full string:
   if( NOT "${_outputContent}" STREQUAL "" )
      set( _outputContent "${_outputContent}\;${_newContent}" )
   else()
      set( _outputContent "${_newContent}" )
   endif()
endforeach()

# Look for direct references to /cvmfs, /eos or /afs in the exported
# configuration. Just to make sure none are there.
foreach( _path "/cvmfs" "/eos" "/afs" )
   if( "${_outputContent}" MATCHES "${_path}" )
      message( WARNING
         "Direct reference to \"${_path}\" found in \"${_fileName}\"" )
   endif()
endforeach()

# Overwrite the original file with the new content:
file( WRITE ${_fileName} ${_outputContent} )

# Unset all created variables:
unset( _targetsContent )
unset( _newContent )
unset( _outputContent )
unset( _fileName )
