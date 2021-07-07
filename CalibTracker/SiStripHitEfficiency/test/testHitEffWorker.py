import FWCore.ParameterSet.Config as cms

process = cms.Process("HitEff")
process.load("Configuration/StandardSequences/MagneticField_cff")
process.load("Configuration.StandardSequences.GeometryRecoDB_cff")
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_data', '')  

process.source = cms.Source("PoolSource", fileNames=cms.untracked.vstring(
    # 10 random files, will need the rest later
    "file:/eos/cms/store/express/Run2018D/StreamExpress/ALCARECO/SiStripCalMinBias-Express-v1/000/325/172/00000/E3D6AECF-3F12-6540-97DC-4A6994CFEBF3.root",
    "file:/eos/cms/store/express/Run2018D/StreamExpress/ALCARECO/SiStripCalMinBias-Express-v1/000/325/172/00000/242566C3-0540-8C43-8D6E-BB42C1FE0BB5.root",
    "file:/eos/cms/store/express/Run2018D/StreamExpress/ALCARECO/SiStripCalMinBias-Express-v1/000/325/172/00000/BC8C0839-F645-B948-9040-15FCB5D50472.root",
    "file:/eos/cms/store/express/Run2018D/StreamExpress/ALCARECO/SiStripCalMinBias-Express-v1/000/325/172/00000/3A806401-2CBC-4345-A5CB-593AABD4BE4E.root",
    "file:/eos/cms/store/express/Run2018D/StreamExpress/ALCARECO/SiStripCalMinBias-Express-v1/000/325/172/00000/852C3C1E-2BD4-A843-A65B-51110A503FBD.root",
    "file:/eos/cms/store/express/Run2018D/StreamExpress/ALCARECO/SiStripCalMinBias-Express-v1/000/325/172/00000/B795F9A0-4681-A34A-B879-E33A0DEC8720.root",
    "file:/eos/cms/store/express/Run2018D/StreamExpress/ALCARECO/SiStripCalMinBias-Express-v1/000/325/172/00000/3A0884F2-A395-C541-8EFB-740C45A57CCE.root",
    "file:/eos/cms/store/express/Run2018D/StreamExpress/ALCARECO/SiStripCalMinBias-Express-v1/000/325/172/00000/D274E7C1-5A9D-A544-B9B3-6A30166FC16C.root",
    "file:/eos/cms/store/express/Run2018D/StreamExpress/ALCARECO/SiStripCalMinBias-Express-v1/000/325/172/00000/C4D243DC-2A09-CF42-A050-7678EF4A90D7.root",
    "file:/eos/cms/store/express/Run2018D/StreamExpress/ALCARECO/SiStripCalMinBias-Express-v1/000/325/172/00000/7946A89D-8AC5-6B4F-BAD2-AE3B971865C5.root",
    ))

#process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(15))
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1000))

process.load("RecoVertex.BeamSpotProducer.BeamSpot_cfi")

process.load("RecoTracker.TrackProducer.TrackRefitters_cff")
process.refitTracks = process.TrackRefitter.clone(src=cms.InputTag("ALCARECOSiStripCalMinBias"))
tracks = cms.InputTag("refitTracks")

process.hiteff = cms.EDProducer("SiStripHitEfficiencyWorker",
    lumiScalers=cms.InputTag("scalersRawToDigi"),
    addLumi = cms.untracked.bool(False),
    commonMode=cms.InputTag("siStripDigis", "CommonMode"),
    addCommonMode=cms.untracked.bool(False),
    combinatorialTracks = tracks,
    trajectories        = tracks,
    siStripClusters     = cms.InputTag("siStripClusters"),
    siStripDigis        = cms.InputTag("siStripDigis"),
    trackerEvent        = cms.InputTag("MeasurementTrackerEvent"),
    # part 2
    Layer = cms.int32(0), # =0 means do all layers
    Debug = cms.bool(True),
    # do not cut on the total number of tracks
    cutOnTracks = cms.untracked.bool(True),
    trackMultiplicity = cms.untracked.uint32(100),
    # use or not first and last measurement of a trajectory (biases), default is false
    useFirstMeas = cms.untracked.bool(False),
    useLastMeas = cms.untracked.bool(False),
    useAllHitsFromTracksWithMissingHits = cms.untracked.bool(False)
    )

process.load("DQM.SiStripCommon.TkHistoMap_cff")

## OLD HITEFF
from CalibTracker.SiStripHitEfficiency.SiStripHitEff_cff import anEff
process.anEff = anEff
process.anEff.Debug = True
process.anEff.combinatorialTracks = tracks
process.anEff.trajectories = tracks
process.TFileService = cms.Service("TFileService",
        fileName = cms.string('HitEffTree.root')
)
process.load("CalibTracker.SiStripCommon.ShallowEventDataProducer_cfi")
process.eventInfo = cms.EDAnalyzer(
        "ShallowTree",
        outputCommands = cms.untracked.vstring(
            'drop *',
            'keep *_shallowEventRun_*_*',
            )
        )
## END OLD HITEFF

## TODO double-check in main CalibTree config if hiteff also takes refitted tracks
process.allPath = cms.Path(process.MeasurementTrackerEvent*process.offlineBeamSpot*process.refitTracks
        *process.anEff*process.shallowEventRun*process.eventInfo
        *process.hiteff)

# save the DQM plots in the DQMIO format
process.dqmOutput = cms.OutputModule("DQMRootOutputModule",
            fileName = cms.untracked.string("DQM.root")
            )
# also save in legacy format, for easier comparison
process.dqmSaver = cms.EDAnalyzer("DQMFileSaver",
        convention = cms.untracked.string('Offline'),
        fileFormat = cms.untracked.string('ROOT'),
        producer = cms.untracked.string('DQM'),
        workflow = cms.untracked.string('/Harvesting/SiStripHitEfficiency/All'),
        dirName = cms.untracked.string('.'),
        saveByRun = cms.untracked.int32(1),
        saveAtJobEnd = cms.untracked.bool(False),
        )
process.HitEffOutput = cms.EndPath(process.dqmOutput*process.dqmSaver)

process.MessageLogger = cms.Service(
    "MessageLogger",
    destinations = cms.untracked.vstring(
        "log_tkhistomap"
        ),
    log_tkhistomap = cms.untracked.PSet(
        threshold = cms.untracked.string("DEBUG"),
        default = cms.untracked.PSet(
        limit = cms.untracked.int32(-1)
        )
    ),
    debugModules = cms.untracked.vstring("hiteff", "anEff"),
    categories=cms.untracked.vstring("TkHistoMap", "SiStripHitEfficiency:HitEff", "SiStripHitEfficiency")
)
# Run the rest of the CT-based sequence with
# cmsRun test/testSiStripHitEffFromCalibTree_cfg.py inputFiles=HitEffTree.root runNumber=325172
