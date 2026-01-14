// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME TruthWeightToolsDictReflexDict
#define R__NO_DEPRECATION

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "ROOT/RConfig.hxx"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Header files passed as explicit arguments
#include "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/TruthWeightTools/TruthWeightTools/TruthWeightToolsDict.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static TClass *TruthWeightToolscLcLIHiggsWeightTool_Dictionary();
   static void TruthWeightToolscLcLIHiggsWeightTool_TClassManip(TClass*);
   static void delete_TruthWeightToolscLcLIHiggsWeightTool(void *p);
   static void deleteArray_TruthWeightToolscLcLIHiggsWeightTool(void *p);
   static void destruct_TruthWeightToolscLcLIHiggsWeightTool(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::TruthWeightTools::IHiggsWeightTool*)
   {
      ::TruthWeightTools::IHiggsWeightTool *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::TruthWeightTools::IHiggsWeightTool));
      static ::ROOT::TGenericClassInfo 
         instance("TruthWeightTools::IHiggsWeightTool", "TruthWeightTools/IHiggsWeightTool.h", 22,
                  typeid(::TruthWeightTools::IHiggsWeightTool), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &TruthWeightToolscLcLIHiggsWeightTool_Dictionary, isa_proxy, 4,
                  sizeof(::TruthWeightTools::IHiggsWeightTool) );
      instance.SetDelete(&delete_TruthWeightToolscLcLIHiggsWeightTool);
      instance.SetDeleteArray(&deleteArray_TruthWeightToolscLcLIHiggsWeightTool);
      instance.SetDestructor(&destruct_TruthWeightToolscLcLIHiggsWeightTool);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::TruthWeightTools::IHiggsWeightTool*)
   {
      return GenerateInitInstanceLocal(static_cast<::TruthWeightTools::IHiggsWeightTool*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::TruthWeightTools::IHiggsWeightTool*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *TruthWeightToolscLcLIHiggsWeightTool_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::TruthWeightTools::IHiggsWeightTool*>(nullptr))->GetClass();
      TruthWeightToolscLcLIHiggsWeightTool_TClassManip(theClass);
   return theClass;
   }

   static void TruthWeightToolscLcLIHiggsWeightTool_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *TruthWeightToolscLcLHiggsWeightTool_Dictionary();
   static void TruthWeightToolscLcLHiggsWeightTool_TClassManip(TClass*);
   static void delete_TruthWeightToolscLcLHiggsWeightTool(void *p);
   static void deleteArray_TruthWeightToolscLcLHiggsWeightTool(void *p);
   static void destruct_TruthWeightToolscLcLHiggsWeightTool(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::TruthWeightTools::HiggsWeightTool*)
   {
      ::TruthWeightTools::HiggsWeightTool *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::TruthWeightTools::HiggsWeightTool));
      static ::ROOT::TGenericClassInfo 
         instance("TruthWeightTools::HiggsWeightTool", "TruthWeightTools/HiggsWeightTool.h", 23,
                  typeid(::TruthWeightTools::HiggsWeightTool), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &TruthWeightToolscLcLHiggsWeightTool_Dictionary, isa_proxy, 4,
                  sizeof(::TruthWeightTools::HiggsWeightTool) );
      instance.SetDelete(&delete_TruthWeightToolscLcLHiggsWeightTool);
      instance.SetDeleteArray(&deleteArray_TruthWeightToolscLcLHiggsWeightTool);
      instance.SetDestructor(&destruct_TruthWeightToolscLcLHiggsWeightTool);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::TruthWeightTools::HiggsWeightTool*)
   {
      return GenerateInitInstanceLocal(static_cast<::TruthWeightTools::HiggsWeightTool*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::TruthWeightTools::HiggsWeightTool*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *TruthWeightToolscLcLHiggsWeightTool_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::TruthWeightTools::HiggsWeightTool*>(nullptr))->GetClass();
      TruthWeightToolscLcLHiggsWeightTool_TClassManip(theClass);
   return theClass;
   }

   static void TruthWeightToolscLcLHiggsWeightTool_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrapper around operator delete
   static void delete_TruthWeightToolscLcLIHiggsWeightTool(void *p) {
      delete (static_cast<::TruthWeightTools::IHiggsWeightTool*>(p));
   }
   static void deleteArray_TruthWeightToolscLcLIHiggsWeightTool(void *p) {
      delete [] (static_cast<::TruthWeightTools::IHiggsWeightTool*>(p));
   }
   static void destruct_TruthWeightToolscLcLIHiggsWeightTool(void *p) {
      typedef ::TruthWeightTools::IHiggsWeightTool current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::TruthWeightTools::IHiggsWeightTool

namespace ROOT {
   // Wrapper around operator delete
   static void delete_TruthWeightToolscLcLHiggsWeightTool(void *p) {
      delete (static_cast<::TruthWeightTools::HiggsWeightTool*>(p));
   }
   static void deleteArray_TruthWeightToolscLcLHiggsWeightTool(void *p) {
      delete [] (static_cast<::TruthWeightTools::HiggsWeightTool*>(p));
   }
   static void destruct_TruthWeightToolscLcLHiggsWeightTool(void *p) {
      typedef ::TruthWeightTools::HiggsWeightTool current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::TruthWeightTools::HiggsWeightTool

namespace ROOT {
   // Registration Schema evolution read functions
   int RecordReadRules_libTruthWeightToolsDict() {
      return 0;
   }
   static int _R__UNIQUE_DICT_(ReadRules_libTruthWeightToolsDict) = RecordReadRules_libTruthWeightToolsDict();R__UseDummy(_R__UNIQUE_DICT_(ReadRules_libTruthWeightToolsDict));
} // namespace ROOT
namespace {
  void TriggerDictionaryInitialization_libTruthWeightToolsDict_Impl() {
    static const char* headers[] = {
"0",
nullptr
    };
    static const char* includePaths[] = {
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "libTruthWeightToolsDict dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
namespace TruthWeightTools{class __attribute__((annotate("$clingAutoload$TruthWeightTools/IHiggsWeightTool.h")))  IHiggsWeightTool;}
namespace TruthWeightTools{class __attribute__((annotate("$clingAutoload$TruthWeightTools/HiggsWeightTool.h")))  HiggsWeightTool;}
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "libTruthWeightToolsDict dictionary payload"

#ifndef XAOD_ANALYSIS
  #define XAOD_ANALYSIS 1
#endif
#ifndef XAOD_DEPRECATE_AUXDATA
  #define XAOD_DEPRECATE_AUXDATA 1
#endif
#ifndef ATLAS_PACKAGE_NAME
  #define ATLAS_PACKAGE_NAME "TruthWeightTools"
#endif
#ifndef EIGEN_DONT_VECTORIZE
  #define EIGEN_DONT_VECTORIZE 1
#endif
#ifndef BOOST_FIBER_NO_LIB
  #define BOOST_FIBER_NO_LIB 1
#endif
#ifndef BOOST_FIBER_DYN_LINK
  #define BOOST_FIBER_DYN_LINK 1
#endif
#ifndef BOOST_CONTEXT_NO_LIB
  #define BOOST_CONTEXT_NO_LIB 1
#endif
#ifndef BOOST_CONTEXT_DYN_LINK
  #define BOOST_CONTEXT_DYN_LINK 1
#endif
#ifndef BOOST_FILESYSTEM_NO_LIB
  #define BOOST_FILESYSTEM_NO_LIB 1
#endif
#ifndef BOOST_FILESYSTEM_DYN_LINK
  #define BOOST_FILESYSTEM_DYN_LINK 1
#endif
#ifndef BOOST_SYSTEM_NO_LIB
  #define BOOST_SYSTEM_NO_LIB 1
#endif
#ifndef BOOST_SYSTEM_DYN_LINK
  #define BOOST_SYSTEM_DYN_LINK 1
#endif
#ifndef BOOST_THREAD_NO_LIB
  #define BOOST_THREAD_NO_LIB 1
#endif
#ifndef BOOST_THREAD_DYN_LINK
  #define BOOST_THREAD_DYN_LINK 1
#endif
#ifndef BOOST_REGEX_NO_LIB
  #define BOOST_REGEX_NO_LIB 1
#endif
#ifndef BOOST_REGEX_DYN_LINK
  #define BOOST_REGEX_DYN_LINK 1
#endif
#ifndef BOOST_CHRONO_NO_LIB
  #define BOOST_CHRONO_NO_LIB 1
#endif
#ifndef BOOST_CHRONO_DYN_LINK
  #define BOOST_CHRONO_DYN_LINK 1
#endif
#ifndef FMT_SHARED
  #define FMT_SHARED 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#ifndef TRUTHWEIGHTTOOLS_TRUTHWEIGHTTOOLSDICT
#define TRUTHWEIGHTTOOLS_TRUTHWEIGHTTOOLSDICT

#include "TruthWeightTools/IHiggsWeightTool.h"
#include "TruthWeightTools/HiggsWeightTool.h"

#endif

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"TruthWeightTools::HiggsWeightTool", payloadCode, "@",
"TruthWeightTools::IHiggsWeightTool", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("libTruthWeightToolsDict",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_libTruthWeightToolsDict_Impl, {{"namespace DataVector_detail { template <typename B1, typename B2, typename B3> class VirtBases; }", 1},{"template <typename T> class DataVectorBase;", 1},{"template <typename T, typename BASE> class DataVector;", 1},{"namespace DataVector_detail { template <typename B> class DVLEltBase_init; }", 1}}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_libTruthWeightToolsDict_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_libTruthWeightToolsDict() {
  TriggerDictionaryInitialization_libTruthWeightToolsDict_Impl();
}
