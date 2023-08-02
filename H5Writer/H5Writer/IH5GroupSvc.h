#ifndef I_H5_FILE_SVC_H
#define I_H5_FILE_SVC_H

#include "GaudiKernel/IService.h"

namespace H5 {
  class Group;
}

class IH5GroupSvc : virtual public IService
{
public:
  virtual ~IH5GroupSvc() {};
  static const InterfaceID& interfaceID();
  virtual H5::Group* group() = 0;
};

inline const InterfaceID& IH5GroupSvc::interfaceID() {
  static const InterfaceID id("IH5GroupSvc", 1, 0);
  return id;
}

#endif
