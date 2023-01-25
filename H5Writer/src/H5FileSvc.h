#ifndef H5_FILE_SVC_H
#define H5_FILE_SVC_H

#include "AthenaBaseComps/AthService.h"
#include "Gaudi/Property.h"

#include <memory>

namespace H5 {
  class H5File;
}

class H5FileSvc : public AthService
{
public:
  DeclareInterfaceID( H5FileSvc, 1, 0 );
  H5FileSvc(const std::string& name, ISvcLocator* pSvcLocator);
  ~H5FileSvc();
  StatusCode initialize() override;
  StatusCode finalize() override;
  H5::H5File* file();
private:
  StatusCode queryInterface(const InterfaceID& riid,
                            void** ppvInterface) override;

  std::unique_ptr<H5::H5File> m_file;
  Gaudi::Property<std::string> m_file_path {this, "path", "", "path to file"};
};

#endif
