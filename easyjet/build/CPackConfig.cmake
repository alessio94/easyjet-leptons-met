# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


set(CPACK_BUILD_SOURCE_DIRS "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet;/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build")
set(CPACK_CMAKE_GENERATOR "Unix Makefiles")
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_FILE "/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/Cmake/4.0.1/Linux-x86_64/share/cmake-4.0/Templates/CPack.GenericDescription.txt")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_SUMMARY "easyjet built using CMake")
set(CPACK_GENERATOR "RPM")
set(CPACK_INNOSETUP_ARCHITECTURE "x64")
set(CPACK_INSTALL_CMAKE_PROJECTS "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build;easyjet;ALL;/")
set(CPACK_INSTALL_PREFIX "usr/easyjet/1.0.0/InstallArea/x86_64-el9-gcc14-opt")
set(CPACK_INSTALL_SCRIPT "/cvmfs/atlas.cern.ch/repo/sw/software/25.2/AthAnalysis/25.2.76/InstallArea/x86_64-el9-gcc14-opt/cmake/modules/scripts/cpack_install.cmake")
set(CPACK_MODULE_PATH "/cvmfs/atlas.cern.ch/repo/sw/software/25.2/AthAnalysis/25.2.76/InstallArea/x86_64-el9-gcc14-opt/cmake/modules;/cvmfs/atlas.cern.ch/repo/sw/software/25.2/AthAnalysisExternals/25.2.76/InstallArea/x86_64-el9-gcc14-opt/cmake/modules;/cvmfs/atlas.cern.ch/repo/sw/software/25.2/sw/lcg/releases/ROOT/6.36.04-6ef4f/x86_64-el9-gcc14-opt/cmake/modules")
set(CPACK_NSIS_DISPLAY_NAME "easyjet/1.0.0/InstallArea/x86_64-el9-gcc14-opt")
set(CPACK_NSIS_INSTALLER_ICON_CODE "")
set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
set(CPACK_NSIS_PACKAGE_NAME "easyjet/1.0.0/InstallArea/x86_64-el9-gcc14-opt")
set(CPACK_NSIS_UNINSTALL_NAME "Uninstall")
set(CPACK_OBJCOPY_EXECUTABLE "/cvmfs/sft.cern.ch/lcg/releases/binutils/2.40-acaab/x86_64-el9/bin/objcopy")
set(CPACK_OBJDUMP_EXECUTABLE "/cvmfs/sft.cern.ch/lcg/releases/binutils/2.40-acaab/x86_64-el9/bin/objdump")
set(CPACK_OUTPUT_CONFIG_FILE "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/CPackConfig.cmake")
set(CPACK_PACKAGE_CONTACT "atlas-sw-core@cern.ch")
set(CPACK_PACKAGE_DEFAULT_LOCATION "/usr")
set(CPACK_PACKAGE_DESCRIPTION "easyjet - 1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_FILE "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/CMakeFiles/README.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "easyjet - 1.0.0")
set(CPACK_PACKAGE_FILE_NAME "easyjet_1.0.0_x86_64-el9-gcc14-opt")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "easyjet/1.0.0/InstallArea/x86_64-el9-gcc14-opt")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "easyjet/1.0.0/InstallArea/x86_64-el9-gcc14-opt")
set(CPACK_PACKAGE_NAME "easyjet")
set(CPACK_PACKAGE_RELOCATABLE "true")
set(CPACK_PACKAGE_VENDOR "ATLAS Collaboration")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PROJECT_CONFIG_FILE "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/CMakeFiles/CPackOptions.cmake")
set(CPACK_READELF_EXECUTABLE "/cvmfs/sft.cern.ch/lcg/releases/binutils/2.40-acaab/x86_64-el9/bin/readelf")
set(CPACK_RESOURCE_FILE_LICENSE "/cvmfs/atlas.cern.ch/repo/sw/software/25.2/AthAnalysis/25.2.76/InstallArea/x86_64-el9-gcc14-opt/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/CMakeFiles/README.txt")
set(CPACK_RESOURCE_FILE_WELCOME "/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/Cmake/4.0.1/Linux-x86_64/share/cmake-4.0/Templates/CPack.GenericWelcome.txt")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "noarch")
set(CPACK_RPM_PACKAGE_AUTOREQ " no")
set(CPACK_RPM_PACKAGE_AUTOREQPROV " no")
set(CPACK_RPM_PACKAGE_GROUP "ATLAS Software")
set(CPACK_RPM_PACKAGE_LICENSE "Apache License Version 2.0")
set(CPACK_RPM_PACKAGE_NAME "easyjet_1.0.0_x86_64-el9-gcc14-opt")
set(CPACK_RPM_PACKAGE_PROVIDES "/bin/sh")
set(CPACK_RPM_PACKAGE_REQUIRES "AthAnalysis_25.2.76_x86_64-el9-gcc14-opt, LCG_108a_ATLAS_1_googletest_1.16.0_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_Boost_1.88.0_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_sqlite_3320300_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_tbb_2022.2.0_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_cppgsl_3.1.0_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_AIDA_3.2.1_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_HepPDT_2.06.01_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_CppUnit_1.14.0_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_libunwind_1.5.0_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_vdt_0.4.4_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_Python_3.12.11_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_fmt_10.2.1_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_jsonmcpp_3.11.3_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_XercesC_3.3.0_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_ROOT_6.36.04_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_libxml2_2.10.4_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_eigen_3.4.1_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_rangev3_0.12.0_x86_64_el9_gcc14_opt, LCG_108a_ATLAS_1_clhep_2.4.7.1_x86_64_el9_gcc14_opt")
set(CPACK_RPM_PACKAGE_VERSION "1.0.0")
set(CPACK_RPM_SPEC_MORE_DEFINE "
%global __os_install_post %{nil}
%define _unpackaged_files_terminate_build 0
%define _binaries_in_noarch_packages_terminate_build 0
%define _source_payload w2.xzdio
%define _binary_payload w2.xzdio
%undefine __brp_mangle_shebangs")
set(CPACK_SET_DESTDIR "OFF")
set(CPACK_SOURCE_GENERATOR "RPM")
set(CPACK_SOURCE_OUTPUT_CONFIG_FILE "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/CPackSourceConfig.cmake")
set(CPACK_SYSTEM_NAME "Linux")
set(CPACK_THREADS "1")
set(CPACK_TOPLEVEL_TAG "Linux")
set(CPACK_WIX_SIZEOF_VOID_P "8")

if(NOT CPACK_PROPERTIES_FILE)
  set(CPACK_PROPERTIES_FILE "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/CPackProperties.cmake")
endif()

if(EXISTS ${CPACK_PROPERTIES_FILE})
  include(${CPACK_PROPERTIES_FILE})
endif()
