#Mon Jan 12 15:13:36 2026"""Automatically generated. DO NOT EDIT please"""
from GaudiKernel.GaudiHandles import *
from AthenaCommon.Configurable import *

class TruthWeightTools__HiggsWeightTool( ConfigurableAlgTool ) :
  __slots__ = { 
    'ExtraInputs' : set(),
    'ExtraOutputs' : set(),
    'OutputLevel' : 0,
    'MonitorService' : 'MonitorSvc',
    'AuditInitialize' : False,
    'AuditStart' : False,
    'AuditStop' : False,
    'AuditFinalize' : False,
    'AuditReinitialize' : False,
    'AuditRestart' : False,
    'EvtStore' : ServiceHandle('StoreGateSvc'),
    'DetStore' : ServiceHandle('StoreGateSvc/DetectorStore'),
    'RequireFinite' : False,
    'WeightCutOff' : -1.0000000,
    'ForceNNLOPS' : False,
    'ForceVBF' : False,
    'ForceVH' : False,
    'ForceTTH' : False,
    'ProdMode' : 'ggF',
  }
  _propertyDocDct = { 
    'ExtraInputs' : """  [DataHandleHolderBase<PropertyHolder<CommonMessaging<implements<IAlgTool,IDataHandleHolder,IProperty,IStateful> > > >] """,
    'ExtraOutputs' : """  [DataHandleHolderBase<PropertyHolder<CommonMessaging<implements<IAlgTool,IDataHandleHolder,IProperty,IStateful> > > >] """,
    'OutputLevel' : """ output level [AlgTool] """,
    'MonitorService' : """ name to use for Monitor Service [AlgTool] """,
    'AuditInitialize' : """ trigger auditor on initialize() [AlgTool] """,
    'AuditStart' : """ trigger auditor on start() [AlgTool] """,
    'AuditStop' : """ trigger auditor on stop() [AlgTool] """,
    'AuditFinalize' : """ trigger auditor on finalize() [AlgTool] """,
    'AuditReinitialize' : """ trigger auditor on reinitialize() [AlgTool] """,
    'AuditRestart' : """ trigger auditor on restart() [AlgTool] """,
    'EvtStore' : """ Handle to a StoreGateSvc instance: it will be used to retrieve data during the course of the job [unknown owner type] """,
    'DetStore' : """ Handle to a StoreGateSvc/DetectorStore instance: it will be used to retrieve data during the course of the job [unknown owner type] """,
  }
  __declaration_location__ = 'TruthWeightTools_entries.cxx:3'
  def __init__(self, name = Configurable.DefaultName, **kwargs):
      super(TruthWeightTools__HiggsWeightTool, self).__init__(name)
      for n,v in kwargs.items():
         setattr(self, n, v)
  def getDlls( self ):
      return 'TruthWeightTools'
  def getType( self ):
      return 'TruthWeightTools::HiggsWeightTool'
  pass # class TruthWeightTools__HiggsWeightTool
