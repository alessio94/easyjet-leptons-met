//
// Created by Yulei on 2024/9/12.
//

#ifndef EASYJET_MULTILEPTONENUM_H
#define EASYJET_MULTILEPTONENUM_H

namespace MULTILEPTON
{
  enum class CH_ID
  {
    unknown = -1,
    hhbb4l = 1,
    hh3l1tau = 2,
    hh3l = 3,
    hh2l2tau = 4,
    hh2lsc1tau = 5,
    hh1l2tau = 6,
    hh1l3tau = 7,
    hh2lsc = 8,

    // extra
    hh2loc = 9,
    hh2loc1tau = 10,
  };

  enum class FLAVOR_1L{
    unknown = -1,
    e = 1,
    m = 2,
  };

  enum class FlAVOR_2L
  {
    unknown = -1,
    ee = 1,
    em = 2,
    me = 3,
    mm = 4,
  };

  enum class FLAVOR_3L
  {
    unknown = -1,
    eee = 1,
    eem = 2,
    eme = 3,
    emm = 4,
    mee = 5,
    mem = 6,
    mme = 7,
    mmm = 8,
  };

  // ZZ->4l
  enum class FLAVOR_4L
  {
    unknown = -1,
    eeee = 1,
    eemm = 2,
    mmee = 3,
    mmmm = 4,
  };
}

#endif // EASYJET_MULTILEPTONENUM_H
