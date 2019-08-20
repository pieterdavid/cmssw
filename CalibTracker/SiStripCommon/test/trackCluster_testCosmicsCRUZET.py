from CalibTracker.SiStripCommon.shallowTree_test_template_cruzet import *
process.TFileService.fileName = 'bla2.root'

#process.source.fileNames = cms.untracked.vstring('file:step3CRUZET.root') #0T gen
process.source.fileNames = cms.untracked.vstring('file:CTcruzet_0.805step3.root') #0T gen
#process.source.fileNames = cms.untracked.vstring('root://cms-xrd-global.cern.ch//store/data/Run2017B/Cosmics/RECO/PromptReco-v1/000/298/211/00000/46EABE1D-5663-E711-B53F-02163E0142C5.root') #2017 runB data
inputStr = sys.argv[2]
found = inputStr.find("root")

if found > 0:
    process.source.fileNames = cms.untracked.vstring("file:"+inputStr)
    process.TFileService.fileName = 'test_shallowTrackAndClusterFullInfo.root' + inputStr
    print(input)
else:
    process.TFileService.fileName = 'test_shallowTrackAndClusterFullInfoCRUZET_tests.root'


#process.source.fileNames = cms.untracked.vstring('root://cms-xrd-global.cern.ch//store/mc/CosmicFall16PhaseIDR/TKCosmics_38T/GEN-SIM-RECO/DECO_90X_upgrade2017cosmics_realistic_deco_v18-v1/00000/0A229457-9122-E711-8E68-0CC47A78A4A6.root') #3.8 cosmics MC 28300ev

#fileNames = cms.untracked.vstring('root://cms-xrd-global.cern.ch//store/data/Commissioning2017/Cosmics/RECO/PromptReco-v1/000/293/492/00000/1A059B34-6135-E711-8D9D-02163E011D9A.root')

#fileNames = cms.untracked.vstring('root://cms-xrd-global.cern.ch//store/data/Commissioning2017/MinimumBias/RECO/PromptReco-v1/000/293/492/00000/7A3E3E7F-6B35-E711-8D8E-02163E011E08.root')

#fileNames = cms.untracked.vstring('root://cms-xrd-global.cern.ch//store/relval/CMSSW_9_0_0/RelValCosmics_UP17/GEN-SIM-RECO/90X_upgrade2017cosmics_realistic_deco_v18_resub-v1/00000/461E2891-7614-E711-A90B-0025905B858A.root')

#fileNames = cms.untracked.vstring('root://cms-xrd-global.cern.ch//store/data/Commissioning2017/MinimumBias/RECO/PromptReco-v1/000/293/492/00000/7A3E3E7F-6B35-E711-8D8E-02163E011E08.root')



process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
    )

#from RecoTracker.TrackProducer.TrackRefitter_cfi import TrackRefitter

process.load('RecoTracker.TrackProducer.TrackRefitters_cff')
process.ShallowTrackClustersCombined = cms.EDProducer("ShallowTrackClustersProducerCombined",
                                      #Tracks=cms.InputTag("generalTracks",""),
                                      Tracks=cms.InputTag("ctfWithMaterialTracksP5",""),
                                      Clusters=cms.InputTag("siStripClusters"),
                                      vertices=cms.InputTag("offlinePrimaryVertices"),
                                      LorentzAngle = cms.string(''),
                                      Prefix=cms.string("cluster"),
                                      Suffix=cms.string("tsos"),
                                      lowBound=cms.int32(0),
                                      highBound=cms.int32(1000),
                                      filename=cms.string("lowPUlogMC.txt"),
                                      isData=cms.bool(False))

#process.load('CalibTracker.SiStripCommon.ShallowTrackClustersProducer_cfi')
#process.tracksRefit = TrackRefitter.clone()


process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase1_2017_cosmics')
#process.GlobalTag = GlobalTag(process.GlobalTag, '92X_dataRun2_Prompt_v4')

#"global_tag": "90X_dataRun2_Prompt_v3" for run 293492 (from 902)
#"global_tag": "90X_dataRun2_Prompt_v2" for data (before 902)
#MC global tag: 90X_upgrade2017_realistic_v20
#

process.TrackRefitter.src = "ctfWithMaterialTracksP5"
#process.TrackRefitter.src = "generalTracks"
process.TrackRefitter.TTRHBuilder = "WithTrackAngle"
process.TrackRefitter.NavigationSchool = ""
process.ShallowTrackClustersCombined.Tracks = 'TrackRefitter'


#process.load('CalibTracker.SiStripCommon.ShallowTrackClustersProducer_cfi')
process.testTree = cms.EDAnalyzer(
   "ShallowTree",
   outputCommands = cms.untracked.vstring(
      'drop *',
      'keep *_ShallowTrackClustersCombined_*_*',
      )
   )


process.p = cms.Path(#process.MeasurementTrackerEvent*
                     process.TrackRefitter*
                     process.ShallowTrackClustersCombined*process.testTree)



#print process.dumpPython() 
