#
# Cmake for H5Writer
#

# Set the name of the package:
atlas_subdir( H5Writer )

# We don't want any warnings in compilation
add_compile_options(-Werror)

# common requirements
atlas_add_library(H5WriterLib
  src/EventInfoWriter.cxx
  LINK_LIBRARIES
  AthenaBaseComps
  xAODEventInfo
  HDF5Utils
  PUBLIC_HEADERS
  H5Writer
  )
atlas_add_component(H5Writer
  src/H5FileSvc.cxx
  src/EventInfoWriterAlg.cxx
  src/components/H5Writer_entries.cxx
  LINK_LIBRARIES
  H5WriterLib
  )


