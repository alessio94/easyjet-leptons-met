# Declare the package
atlas_subdir(bbbbAnalysis)

# Find an external package (i.e. ROOT)
find_package(ROOT COMPONENTS Core Tree Hist REQUIRED)

find_package( Boost )

find_package(HDF5 1.10.1 COMPONENTS HL C CXX)

# We don't want any warnings in compilation
add_compile_options(-Werror)

# Build the Athena component library
atlas_add_component(bbbbAnalysis
  src/TruthParticleInformationAlg.cxx
  src/JetPairingAlg.cxx
  src/BaselineVarsResolvedAlg.cxx
  src/BaselineVarsBoostedAlg.cxx
  src/JetBoostHistograms.cxx
  src/JetBoostHistogramsAlg.cxx
  src/components/HH4bAnalysis_entries.cxx
  LINK_LIBRARIES
  AthenaBaseComps
  AsgTools
  AthContainers
  xAODEventInfo
  xAODEgamma
  xAODJet
  xAODTracking
  xAODTruth
  SystematicsHandlesLib
  FourMomUtils
  TruthUtils
  IH5GroupSvc
  ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES}
)

# Install python modules, joboptions, and share content
atlas_install_scripts(
  bin/bbbb-ntupler
  bin/bbbb-hists
)

atlas_install_python_modules(
  python/config
  python/utils
)
atlas_install_data(
  share/RunConfig-Resolved.yaml
  share/RunConfig-SH4b.yaml
  share/run-config-boosted-minimal.yaml
  share/histonly-config.yaml
)

# atlas_install_data( data/* )
# You can access your data from code using path resolver, e.g.
# PathResolverFindCalibFile("JetMETCommon/file.txt")
