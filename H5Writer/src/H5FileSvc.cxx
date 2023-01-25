#include "H5FileSvc.h"
#include "H5Cpp.h"

H5FileSvc::H5FileSvc(const std::string& name, ISvcLocator* pSvcLocator):
  AthService(name, pSvcLocator)
{
}

H5FileSvc::~H5FileSvc(){};

StatusCode H5FileSvc::initialize() {
  if (m_file_path.empty()) {
    return StatusCode::FAILURE;
  }
  m_file.reset(new H5::H5File(m_file_path, H5F_ACC_TRUNC));
  return StatusCode::SUCCESS;
}
StatusCode H5FileSvc::finalize() {
  return StatusCode::SUCCESS;
}

H5::H5File* H5FileSvc::file() {
  return m_file.get();
}

// weird magic, sort of copied from
// Control/AthenaBaseComps/src/AthCnvSvc.cxx
// There's a better example under PerfMonComps
// https://gitlab.cern.ch/atlas/athena/-/tree/master/Control/PerformanceMonitoring/PerfMonComps
//
StatusCode H5FileSvc::queryInterface(const InterfaceID& riid,
                                     void** ppvInterface)
{
  if ( interfaceID().versionMatch(riid) )  {
    // This thing seems to get called when we run the code.
    //
    // For now the dynamic cast won't do anything, but if we ever make
    // a base class for H5FileSvc this cast should go to IH5FileSvc.
    *ppvInterface = dynamic_cast<H5FileSvc*>(this);
  } else {
    // And this thing gets called when the code is being compiled
    return AthService::queryInterface(riid, ppvInterface);
  }
  addRef();
  return StatusCode::SUCCESS;
}
