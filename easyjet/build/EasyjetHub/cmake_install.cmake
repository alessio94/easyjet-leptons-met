# Install script for directory: /afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/src/EasyjetHub" TYPE DIRECTORY FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/" USE_SOURCE_PERMISSIONS REGEX "/\\.svn$" EXCLUDE REGEX "/\\.git$" EXCLUDE REGEX "/[^/]*\\~$" EXCLUDE)
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
            -E create_symlink ../src/EasyjetHub/EasyjetHub
            ${_destdir}${CMAKE_INSTALL_PREFIX}/include/EasyjetHub )
         unset( _destdir )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY OPTIONAL FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/x86_64-el9-gcc14-opt/lib/libEasyjetHubLib.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libEasyjetHubLib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libEasyjetHubLib.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/cvmfs/sft.cern.ch/lcg/releases/binutils/2.40-acaab/x86_64-el9/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libEasyjetHubLib.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE MODULE OPTIONAL FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/x86_64-el9-gcc14-opt/lib/libEasyjetHub.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libEasyjetHub.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libEasyjetHub.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/cvmfs/sft.cern.ch/lcg/releases/binutils/2.40-acaab/x86_64-el9/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libEasyjetHub.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/python/EasyjetHub" TYPE FILE OPTIONAL FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/x86_64-el9-gcc14-opt/python/EasyjetHub/EasyjetHubConf.py")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  execute_process( COMMAND ${CMAKE_COMMAND} -E touch
      $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/python/EasyjetHub/__init__.py )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM RENAME "easyjet-ntupler" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/bin/easyjet-ntupler")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM RENAME "easyjet-gridsubmit" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/bin/easyjet-gridsubmit")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM RENAME "easyjet-merge-configs" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/bin/easyjet-merge-configs")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM RENAME "easyjet-create-git-tag" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/bin/easyjet-create-git-tag")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM RENAME "merge-with-hadd" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/bin/merge-with-hadd")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/python/EasyjetHub" TYPE FILE RENAME "__init__.py" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/python/__init__.py")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/python/EasyjetHub" TYPE FILE RENAME "hub.py" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/python/hub.py")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/python/EasyjetHub" TYPE DIRECTORY FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/python/algs" USE_SOURCE_PERMISSIONS REGEX "/\\.svn$" EXCLUDE)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/python/EasyjetHub" TYPE DIRECTORY FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/python/output" USE_SOURCE_PERMISSIONS REGEX "/\\.svn$" EXCLUDE)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/python/EasyjetHub" TYPE DIRECTORY FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/python/steering" USE_SOURCE_PERMISSIONS REGEX "/\\.svn$" EXCLUDE)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/python/EasyjetHub" TYPE DIRECTORY FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/test" USE_SOURCE_PERMISSIONS REGEX "/\\.svn$" EXCLUDE)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "AnalysisMiniTree-config.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/AnalysisMiniTree-config.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "DSID_samples.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/DSID_samples.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "DalitzDataset.txt" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/DalitzDataset.txt")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "RunConfig-EMTopo.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/RunConfig-EMTopo.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "RunConfig.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/RunConfig.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "SpecialWeightIndices.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/SpecialWeightIndices.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "awkward-config-noEle.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/awkward-config-noEle.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "awkward-config.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/awkward-config.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "base-config.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/base-config.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "bjet_grl.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/bjet_grl.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "bjet_grl_years.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/bjet_grl_years.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "container-names-DAOD_PHYS.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/container-names-DAOD_PHYS.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "container-names-DAOD_PHYSLITE.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/container-names-DAOD_PHYSLITE.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "grl_years_no2015.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/grl_years_no2015.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "lumicalc_bjet.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/lumicalc_bjet.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "lumicalc_no2015.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/lumicalc_no2015.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "orth-config.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/orth-config.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "prw_bjet.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/prw_bjet.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "trigger.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/trigger.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "xbb-vars-in-ptag.yaml" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/share/xbb-vars-in-ptag.yaml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "2023-22-13p6TeV-MC21-CDI_GN2v01_Test_2024-07-ctag_noSF_NewCutValues_fTau_Ctag.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/data/2023-22-13p6TeV-MC21-CDI_GN2v01_Test_2024-07-ctag_noSF_NewCutValues_fTau_Ctag.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "Xbb_lookup_table_prelim_Oct30_2024.json" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/data/Xbb_lookup_table_prelim_Oct30_2024.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "jet_trigger_scale_factors_2223.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/data/jet_trigger_scale_factors_2223.root")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/EasyjetHub" TYPE FILE RENAME "jet_trigger_scale_factors_run2.root" FILES "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/EasyjetHub/data/jet_trigger_scale_factors_run2.root")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/EasyjetHub/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
