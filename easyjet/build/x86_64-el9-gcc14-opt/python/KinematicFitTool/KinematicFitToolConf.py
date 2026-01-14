#Mon Jan 12 15:13:36 2026"""Automatically generated. DO NOT EDIT please"""
from GaudiKernel.GaudiHandles import *
from AthenaCommon.Configurable import *

class KinematicFitTool( ConfigurableAlgTool ) :
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
    'mode' : 'bbyy',
    'JetCollection' : 'AntiKt4EMPFlow',
    'bTagWPDecorName' : 'ftag_select_GN2v00LegacyWP_FixedCutBEff_77',
    'JetMinPt' : 20000.000,
    'FixAnglesFit' : True,
    'isRun3' : True,
    'LambdaMomConstraint' : 0.0000000,
    'LambdaMassConstraint' : 0.0000000,
    'EnRespFilename' : '',
    'PtRespFilename' : '',
    'pXconstrFilename' : '',
    'pYconstrFilename' : '',
    'log_pt_binning' : [  ],
    'Barrel_E_binning' : [  ],
    'Crack_E_binning' : [  ],
    'Endcap_E_binning' : [  ],
    'No_Track_E_binning' : [  ],
    'Barrel_pT_binning' : [  ],
    'Crack_pT_binning' : [  ],
    'Endcap_pT_binning' : [  ],
    'No_Track_pT_binning' : [  ],
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
    'mode' : """ corresponding final state [KinematicFitTool] """,
    'JetCollection' : """  [KinematicFitTool] """,
    'bTagWPDecorName' : """  [KinematicFitTool] """,
    'JetMinPt' : """  [KinematicFitTool] """,
    'FixAnglesFit' : """  [KinematicFitTool] """,
    'isRun3' : """  [KinematicFitTool] """,
    'LambdaMomConstraint' : """ Lambda parameter for momentum constraint in kinematic fit [KinematicFitTool] """,
    'LambdaMassConstraint' : """ Lambda parameter for mass constraint in kinematic fit [KinematicFitTool] """,
    'EnRespFilename' : """ corresponding final name [KinematicFitTool] """,
    'PtRespFilename' : """ corresponding final name [KinematicFitTool] """,
    'pXconstrFilename' : """ corresponding final name [KinematicFitTool] """,
    'pYconstrFilename' : """ corresponding final name [KinematicFitTool] """,
    'log_pt_binning' : """  [KinematicFitTool] """,
    'Barrel_E_binning' : """  [KinematicFitTool] """,
    'Crack_E_binning' : """  [KinematicFitTool] """,
    'Endcap_E_binning' : """  [KinematicFitTool] """,
    'No_Track_E_binning' : """  [KinematicFitTool] """,
    'Barrel_pT_binning' : """  [KinematicFitTool] """,
    'Crack_pT_binning' : """  [KinematicFitTool] """,
    'Endcap_pT_binning' : """  [KinematicFitTool] """,
    'No_Track_pT_binning' : """  [KinematicFitTool] """,
  }
  __declaration_location__ = 'KinematicFitTool_entries.cxx:11'
  def __init__(self, name = Configurable.DefaultName, **kwargs):
      super(KinematicFitTool, self).__init__(name)
      for n,v in kwargs.items():
         setattr(self, n, v)
  def getDlls( self ):
      return 'KinematicFitTool'
  def getType( self ):
      return 'KinematicFitTool'
  pass # class KinematicFitTool
