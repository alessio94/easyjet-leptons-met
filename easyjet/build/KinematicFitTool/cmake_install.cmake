# Install script for directory: /afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/easyjet/1.0.0/InstallArea/x86_64-el9-gcc14-opt")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/cvmfs/sft.cern.ch/lcg/releases/binutils/2.40-acaab/x86_64-el9/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/src/KinematicFitTool" TYPE DIRECTORY FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/" USE_SOURCE_PERMISSIONS REGEX "/\\.svn$" EXCLUDE REGEX "/\\.git$" EXCLUDE REGEX "/[^/]*\\~$" EXCLUDE)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  
      set( _destdir "$ENV{DESTDIR}")
      if( NOT _destdir STREQUAL "" )
         set( _destdir "${_destdir}/" )
      endif()
      execute_process( COMMAND ${CMAKE_COMMAND}
         -E make_directory
         ${_destdir}${CMAKE_INSTALL_PREFIX}/include )
      unset( _destdir )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  
         set( _destdir "$ENV{DESTDIR}")
         if( NOT _destdir STREQUAL "" )
            set( _destdir "${_destdir}/" )
         endif()
         execute_process( COMMAND ${CMAKE_COMMAND}
            -E create_symlink ../src/KinematicFitTool/KinematicFitTool
            ${_destdir}${CMAKE_INSTALL_PREFIX}/include/KinematicFitTool )
         unset( _destdir )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY OPTIONAL FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/x86_64-el9-gcc14-opt/lib/libKinematicFitToolLib.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libKinematicFitToolLib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libKinematicFitToolLib.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/cvmfs/sft.cern.ch/lcg/releases/binutils/2.40-acaab/x86_64-el9/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libKinematicFitToolLib.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE MODULE OPTIONAL FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/x86_64-el9-gcc14-opt/lib/libKinematicFitTool.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libKinematicFitTool.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libKinematicFitTool.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/cvmfs/sft.cern.ch/lcg/releases/binutils/2.40-acaab/x86_64-el9/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libKinematicFitTool.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/python/KinematicFitTool" TYPE FILE OPTIONAL FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/x86_64-el9-gcc14-opt/python/KinematicFitTool/KinematicFitToolConf.py")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  execute_process( COMMAND ${CMAKE_COMMAND} -E touch
      $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/python/KinematicFitTool/__init__.py )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/KinematicFitTool" TYPE FILE RENAME "E_AntiKt4EMPFlow_TaOgataParameters_Run2_FastSim_PeakCentered.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/data/E_AntiKt4EMPFlow_TaOgataParameters_Run2_FastSim_PeakCentered.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/KinematicFitTool" TYPE FILE RENAME "E_AntiKt4EMPFlow_TaOgataParameters_Run2_bb4l.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/data/E_AntiKt4EMPFlow_TaOgataParameters_Run2_bb4l.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/KinematicFitTool" TYPE FILE RENAME "E_AntiKt4EMPFlow_TaOgataParameters_Run3_FastSim_PeakCentered.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/data/E_AntiKt4EMPFlow_TaOgataParameters_Run3_FastSim_PeakCentered.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/KinematicFitTool" TYPE FILE RENAME "E_AntiKt4EMPFlow_TaOgataParameters_Run3_bb4l.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/data/E_AntiKt4EMPFlow_TaOgataParameters_Run3_bb4l.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/KinematicFitTool" TYPE FILE RENAME "lnE_AntiKt4EMPFlow_TaOgataParameters_Run2_bb4l.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/data/lnE_AntiKt4EMPFlow_TaOgataParameters_Run2_bb4l.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/KinematicFitTool" TYPE FILE RENAME "ln_pT_AntiKt4EMPFlow_TaOgataParameters_Run2_bb4l.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/data/ln_pT_AntiKt4EMPFlow_TaOgataParameters_Run2_bb4l.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/KinematicFitTool" TYPE FILE RENAME "pT_AntiKt4EMPFlow_TaOgataParameters_Run2_bb4l.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/data/pT_AntiKt4EMPFlow_TaOgataParameters_Run2_bb4l.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/KinematicFitTool" TYPE FILE RENAME "pT_AntiKt4EMPFlow_TaOgataParameters_Run3_bb4l.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/data/pT_AntiKt4EMPFlow_TaOgataParameters_Run3_bb4l.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/KinematicFitTool" TYPE FILE RENAME "pXconstr_AntiKt4EMPFlow_DSCBParameters.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/data/pXconstr_AntiKt4EMPFlow_DSCBParameters.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/KinematicFitTool" TYPE FILE RENAME "pXconstr_AntiKt4EMPFlow_DSCBParameters_bb4l.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/data/pXconstr_AntiKt4EMPFlow_DSCBParameters_bb4l.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/KinematicFitTool" TYPE FILE RENAME "pXconstr_AntiKt4EMPFlow_TaOTaParameters_updated0925.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/data/pXconstr_AntiKt4EMPFlow_TaOTaParameters_updated0925.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/KinematicFitTool" TYPE FILE RENAME "pYconstr_AntiKt4EMPFlow_DSCBParameters.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/data/pYconstr_AntiKt4EMPFlow_DSCBParameters.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/KinematicFitTool" TYPE FILE RENAME "pYconstr_AntiKt4EMPFlow_DSCBParameters_bb4l.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/data/pYconstr_AntiKt4EMPFlow_DSCBParameters_bb4l.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/KinematicFitTool" TYPE FILE RENAME "pYconstr_AntiKt4EMPFlow_TaOTaParameters_updated0925.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/data/pYconstr_AntiKt4EMPFlow_TaOTaParameters_updated0925.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/python/KinematicFitTool" TYPE FILE RENAME "KinematicFit_config.py" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/KinematicFitTool/python/KinematicFit_config.py")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/KinematicFitTool/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
