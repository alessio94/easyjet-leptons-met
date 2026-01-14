#Mon Jan 12 15:13:36 2026"""Automatically generated. DO NOT EDIT please"""
from GaudiKernel.GaudiHandles import *
from GaudiKernel.DataHandle import DataHandle
from AthenaCommon.Configurable import *

class VBFTagger( ConfigurableAlgTool ) :
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
    'IsDebug' : False,
    'IsPureRNN' : True,
    'PreProcJetsMethod' : '',
    'pTCut' : 30000.000,
    'modelTag' : '',
    'nMaxJets' : 2,
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
    'IsDebug' : """  [VBFTagger] """,
    'IsPureRNN' : """  [VBFTagger] """,
    'PreProcJetsMethod' : """  [VBFTagger] """,
    'pTCut' : """  [VBFTagger] """,
    'modelTag' : """ VBF RNN model version [VBFTagger] """,
    'nMaxJets' : """  [VBFTagger] """,
  }
  __declaration_location__ = 'VBFTagger_entries.cxx:13'
  def __init__(self, name = Configurable.DefaultName, **kwargs):
      super(VBFTagger, self).__init__(name)
      for n,v in kwargs.items():
         setattr(self, n, v)
  def getDlls( self ):
      return 'VBFTagger'
  def getType( self ):
      return 'VBFTagger'
  pass # class VBFTagger

class VBFTaggerAlg( ConfigurableAlgorithm ) :
  __slots__ = { 
    'ExtraInputs' : set(),
    'ExtraOutputs' : set(),
    'OutputLevel' : 0,
    'Enable' : True,
    'ErrorMax' : 1,
    'AuditInitialize' : False,
    'AuditReinitialize' : False,
    'AuditRestart' : False,
    'AuditExecute' : False,
    'AuditFinalize' : False,
    'AuditStart' : False,
    'AuditStop' : False,
    'Timeline' : True,
    'MonitorService' : 'MonitorSvc',
    'RegisterForContextService' : False,
    'Cardinality' : 1,
    'NeededResources' : [  ],
    'Asynchronous' : False,
    'FilterCircularDependencies' : True,
    'EvtStore' : ServiceHandle('StoreGateSvc'),
    'DetStore' : ServiceHandle('StoreGateSvc/DetectorStore'),
    'THistSvc' : ServiceHandle('THistSvc/THistSvc'),
    'RootStreamName' : '/ANALYSIS',
    'RootDirName' : '',
    'HistNamePrefix' : '',
    'HistNamePostfix' : '',
    'HistTitlePrefix' : '',
    'HistTitlePostfix' : '',
    'VBFTagger' : PrivateToolHandle('VBFTagger/VBFTagger'),
    'EventInfoKey' : DataHandle('StoreGateSvc+EventInfo','R','xAOD::EventInfo',False),
    'containerAllJetsKey' : DataHandle('StoreGateSvc+','R','xAOD::JetContainer',False),
    'containerSigJetsKey' : DataHandle('StoreGateSvc+','R','xAOD::JetContainer',False),
    'containerSigLargeRJetsKey' : DataHandle('StoreGateSvc+','R','xAOD::JetContainer',False),
    'RNNJetsDec' : DataHandle('StoreGateSvc+','W','xAOD::JetContainer',False),
    'pTCut' : 30000.0,
    'nMaxJets' : 2.00000,
    'DRCut' : 1.40000,
    'OnlyFirstLargeRJet' : False,
    'DecTag' : '',
  }
  _propertyDocDct = { 
    'ExtraInputs' : """  [DataHandleHolderBase<PropertyHolder<CommonMessaging<implements<IAlgorithm,IDataHandleHolder,IProperty,IStateful> > > >] """,
    'ExtraOutputs' : """  [DataHandleHolderBase<PropertyHolder<CommonMessaging<implements<IAlgorithm,IDataHandleHolder,IProperty,IStateful> > > >] """,
    'OutputLevel' : """ output level [Gaudi::Algorithm] """,
    'Enable' : """ should the algorithm be executed or not [Gaudi::Algorithm] """,
    'ErrorMax' : """ [[deprecated]] max number of errors [Gaudi::Algorithm] """,
    'AuditInitialize' : """ trigger auditor on initialize() [Gaudi::Algorithm] """,
    'AuditReinitialize' : """ trigger auditor on reinitialize() [Gaudi::Algorithm] """,
    'AuditRestart' : """ trigger auditor on restart() [Gaudi::Algorithm] """,
    'AuditExecute' : """ trigger auditor on execute() [Gaudi::Algorithm] """,
    'AuditFinalize' : """ trigger auditor on finalize() [Gaudi::Algorithm] """,
    'AuditStart' : """ trigger auditor on start() [Gaudi::Algorithm] """,
    'AuditStop' : """ trigger auditor on stop() [Gaudi::Algorithm] """,
    'Timeline' : """ send events to TimelineSvc [Gaudi::Algorithm] """,
    'MonitorService' : """ name to use for Monitor Service [Gaudi::Algorithm] """,
    'RegisterForContextService' : """ flag to enforce the registration for Algorithm Context Service [Gaudi::Algorithm] """,
    'Cardinality' : """ how many clones to create - 0 means algo is reentrant [Gaudi::Algorithm] """,
    'NeededResources' : """ named resources needed during event looping [Gaudi::Algorithm] """,
    'Asynchronous' : """ whether algorithm is asynchronous and uses Boost Fiber to suspend while offloaded code is running. [Gaudi::Algorithm] """,
    'FilterCircularDependencies' : """ filter out circular data dependencies [Gaudi::Algorithm] """,
    'EvtStore' : """ Handle to a StoreGateSvc instance: it will be used to retrieve data during the course of the job [unknown owner type] """,
    'DetStore' : """ Handle to a StoreGateSvc/DetectorStore instance: it will be used to retrieve data during the course of the job [unknown owner type] """,
    'THistSvc' : """ Handle to a THistSvc instance: it will be used to write ROOT objects to ROOT files [unknown owner type] """,
    'RootStreamName' : """ Name of the output ROOT stream (file) that the THistSvc uses [unknown owner type] """,
    'RootDirName' : """ Name of the ROOT directory inside the ROOT file where the histograms will go [unknown owner type] """,
    'HistNamePrefix' : """ The prefix for the histogram THx name [unknown owner type] """,
    'HistNamePostfix' : """ The postfix for the histogram THx name [unknown owner type] """,
    'HistTitlePrefix' : """ The prefix for the histogram THx title [unknown owner type] """,
    'HistTitlePostfix' : """ The postfix for the histogram THx title [unknown owner type] """,
    'VBFTagger' : """ Tool implementing the VBF RNN score retrieve [VBFTaggerAlg] """,
    'EventInfoKey' : """ EventInfo container to dump [VBFTaggerAlg] """,
    'containerAllJetsKey' : """ containerName to read [VBFTaggerAlg] """,
    'containerSigJetsKey' : """ containerName to read [VBFTaggerAlg] """,
    'containerSigLargeRJetsKey' : """ containerName to read [VBFTaggerAlg] """,
    'RNNJetsDec' : """ rnn jets container to write [VBFTaggerAlg] """,
    'pTCut' : """ Minimum pT of jets to be used [VBFTaggerAlg] """,
    'nMaxJets' : """ Maximum number of jets to be used [VBFTaggerAlg] """,
    'DRCut' : """ DR thr between large-R and small-R jets [VBFTaggerAlg] """,
    'OnlyFirstLargeRJet' : """ Consider only the first large-R jet for the removal [VBFTaggerAlg] """,
    'DecTag' : """ Additional tag for decorator [VBFTaggerAlg] """,
  }
  __declaration_location__ = 'VBFTagger_entries.cxx:14'
  def __init__(self, name = Configurable.DefaultName, **kwargs):
      super(VBFTaggerAlg, self).__init__(name)
      for n,v in kwargs.items():
         setattr(self, n, v)
  def getDlls( self ):
      return 'VBFTagger'
  def getType( self ):
      return 'VBFTaggerAlg'
  pass # class VBFTaggerAlg

class VBFTaggerAlgSys( ConfigurableAlgorithm ) :
  __slots__ = { 
    'ExtraInputs' : set(),
    'ExtraOutputs' : set(),
    'OutputLevel' : 0,
    'Enable' : True,
    'ErrorMax' : 1,
    'AuditInitialize' : False,
    'AuditReinitialize' : False,
    'AuditRestart' : False,
    'AuditExecute' : False,
    'AuditFinalize' : False,
    'AuditStart' : False,
    'AuditStop' : False,
    'Timeline' : True,
    'MonitorService' : 'MonitorSvc',
    'RegisterForContextService' : False,
    'Cardinality' : 1,
    'NeededResources' : [  ],
    'Asynchronous' : False,
    'FilterCircularDependencies' : True,
    'EvtStore' : ServiceHandle('StoreGateSvc'),
    'DetStore' : ServiceHandle('StoreGateSvc/DetectorStore'),
    'THistSvc' : ServiceHandle('THistSvc/THistSvc'),
    'RootStreamName' : '/ANALYSIS',
    'RootDirName' : '',
    'HistNamePrefix' : '',
    'HistNamePostfix' : '',
    'HistTitlePrefix' : '',
    'HistTitlePostfix' : '',
    'VBFTagger' : PrivateToolHandle('VBFTagger/VBFTagger'),
    'systematicsService' : ServiceHandle('SystematicsSvc'),
    'affectingSystematicsFilter' : '',
    'EventInfoKey' : 'EventInfo',
    'containerAllJetsKey' : '',
    'containerSigJetsKey' : '',
    'containerSigLargeRJetsKey' : '',
    'RNNScoreDec' : '',
    'nRNNJetsDec' : '',
    'RNNJetsDec' : '',
    'pTCut' : 30000.0,
    'nMaxJets' : 2.00000,
    'DRCut' : 1.40000,
    'OnlyFirstLargeRJet' : False,
    'DecTag' : '',
  }
  _propertyDocDct = { 
    'ExtraInputs' : """  [DataHandleHolderBase<PropertyHolder<CommonMessaging<implements<IAlgorithm,IDataHandleHolder,IProperty,IStateful> > > >] """,
    'ExtraOutputs' : """  [DataHandleHolderBase<PropertyHolder<CommonMessaging<implements<IAlgorithm,IDataHandleHolder,IProperty,IStateful> > > >] """,
    'OutputLevel' : """ output level [Gaudi::Algorithm] """,
    'Enable' : """ should the algorithm be executed or not [Gaudi::Algorithm] """,
    'ErrorMax' : """ [[deprecated]] max number of errors [Gaudi::Algorithm] """,
    'AuditInitialize' : """ trigger auditor on initialize() [Gaudi::Algorithm] """,
    'AuditReinitialize' : """ trigger auditor on reinitialize() [Gaudi::Algorithm] """,
    'AuditRestart' : """ trigger auditor on restart() [Gaudi::Algorithm] """,
    'AuditExecute' : """ trigger auditor on execute() [Gaudi::Algorithm] """,
    'AuditFinalize' : """ trigger auditor on finalize() [Gaudi::Algorithm] """,
    'AuditStart' : """ trigger auditor on start() [Gaudi::Algorithm] """,
    'AuditStop' : """ trigger auditor on stop() [Gaudi::Algorithm] """,
    'Timeline' : """ send events to TimelineSvc [Gaudi::Algorithm] """,
    'MonitorService' : """ name to use for Monitor Service [Gaudi::Algorithm] """,
    'RegisterForContextService' : """ flag to enforce the registration for Algorithm Context Service [Gaudi::Algorithm] """,
    'Cardinality' : """ how many clones to create - 0 means algo is reentrant [Gaudi::Algorithm] """,
    'NeededResources' : """ named resources needed during event looping [Gaudi::Algorithm] """,
    'Asynchronous' : """ whether algorithm is asynchronous and uses Boost Fiber to suspend while offloaded code is running. [Gaudi::Algorithm] """,
    'FilterCircularDependencies' : """ filter out circular data dependencies [Gaudi::Algorithm] """,
    'EvtStore' : """ Handle to a StoreGateSvc instance: it will be used to retrieve data during the course of the job [unknown owner type] """,
    'DetStore' : """ Handle to a StoreGateSvc/DetectorStore instance: it will be used to retrieve data during the course of the job [unknown owner type] """,
    'THistSvc' : """ Handle to a THistSvc instance: it will be used to write ROOT objects to ROOT files [unknown owner type] """,
    'RootStreamName' : """ Name of the output ROOT stream (file) that the THistSvc uses [unknown owner type] """,
    'RootDirName' : """ Name of the ROOT directory inside the ROOT file where the histograms will go [unknown owner type] """,
    'HistNamePrefix' : """ The prefix for the histogram THx name [unknown owner type] """,
    'HistNamePostfix' : """ The postfix for the histogram THx name [unknown owner type] """,
    'HistTitlePrefix' : """ The prefix for the histogram THx title [unknown owner type] """,
    'HistTitlePostfix' : """ The postfix for the histogram THx title [unknown owner type] """,
    'VBFTagger' : """ Tool implementing the VBF RNN score retrieve [VBFTaggerAlgSys] """,
    'systematicsService' : """ systematics to evaluate [unknown owner type] """,
    'affectingSystematicsFilter' : """ an (optional) filter to remove affecting systematics [unknown owner type] """,
    'EventInfoKey' : """ EventInfo container to read [unknown owner type] """,
    'containerAllJetsKey' : """ containerName to read [unknown owner type] """,
    'containerSigJetsKey' : """ containerName to read [unknown owner type] """,
    'containerSigLargeRJetsKey' : """ containerName to read [unknown owner type] """,
    'RNNScoreDec' : """ decorator name for RNN score [unknown owner type] """,
    'nRNNJetsDec' : """ decorator name for nRNN jets [unknown owner type] """,
    'RNNJetsDec' : """ rnn jets container to write [unknown owner type] """,
    'pTCut' : """ Minimum pT of jets to be used [VBFTaggerAlgSys] """,
    'nMaxJets' : """ Maximum number of jets to be used [VBFTaggerAlgSys] """,
    'DRCut' : """ DR thr between large-R and small-R jets [VBFTaggerAlgSys] """,
    'OnlyFirstLargeRJet' : """ Consider only the first large-R jet for the removal [VBFTaggerAlgSys] """,
    'DecTag' : """ Additional tag for decorator [VBFTaggerAlgSys] """,
  }
  __declaration_location__ = 'VBFTagger_entries.cxx:15'
  def __init__(self, name = Configurable.DefaultName, **kwargs):
      super(VBFTaggerAlgSys, self).__init__(name)
      for n,v in kwargs.items():
         setattr(self, n, v)
  def getDlls( self ):
      return 'VBFTagger'
  def getType( self ):
      return 'VBFTaggerAlgSys'
  pass # class VBFTaggerAlgSys
