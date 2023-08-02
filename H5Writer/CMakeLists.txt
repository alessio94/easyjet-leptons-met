#
# Cmake for H5Writer
#

# Set the name of the package:
atlas_subdir( H5Writer )

# We don't want any warnings in compilation
add_compile_options(-Werror)

# interface library
atlas_add_library(IH5GroupSvc
  PUBLIC_HEADERS
  H5Writer
  LINK_LIBRARIES
  GaudiKernel
  )

# common requirements
atlas_add_library(H5WriterLib
  src/EventInfoWriter.cxx
  src/IParticleWriter.cxx
  src/PrimitiveHelpers.cxx
  LINK_LIBRARIES
  AthenaBaseComps
  xAODEventInfo
  xAODBase
  HDF5Utils
  xAODBTagging
  PUBLIC_HEADERS
  H5Writer
  )
atlas_add_component(H5Writer
  src/H5FileSvc.cxx
  src/EventInfoWriterAlg.cxx
  src/IParticleWriterAlg.cxx
  src/AlgHelpers.cxx
  src/components/H5Writer_entries.cxx
  LINK_LIBRARIES
  H5WriterLib
  IH5GroupSvc
  )


