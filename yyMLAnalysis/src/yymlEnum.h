
#ifndef YYMLANALYSIS_YYMLENUM_H
#define YYMLANALYSIS_YYMLENUM_H

namespace HHYYML {

  enum class CH_ID {
    unknown = -1,
    hh1l0tau = 1,
    hh0l1tau = 2,
    hh2l0tau = 3,
    hh1l1tau = 4,
    hh0l2tau = 5,
  };

  enum class FLAVOR_1L {
    unknown = -1,
    e = 1,
    m = 2,
  };

  enum class FLAVOR_2L
  {
    unknown = -1,
    ee = 1,
    em = 2,
    me = 3,
    mm = 4,
  };

}

#endif // YYMLANALYSIS_YYMLENUM_H
