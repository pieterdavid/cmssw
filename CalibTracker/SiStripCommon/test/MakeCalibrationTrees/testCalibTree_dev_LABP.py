from __future__ import print_function

## adapted from produceCalibrationTree_template_cfg.py

import FWCore.ParameterSet.Config as cms
##from CalibTracker.SiStripCommon.shallowTree_test_template import * ## TODO get rid of this one

process = cms.Process('CALIB')
process.load('Configuration/StandardSequences/MagneticField_cff')
process.load('Configuration.Geometry.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, "auto:run2_data")

process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.StandardSequences.Services_cff')

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(-1))

process.source = cms.Source("PoolSource",
        ##fileNames = cms.untracked.vstring("file:/afs/cern.ch/user/d/davidp/private/tkdpg/CT20/CMSSW_11_2_0_pre3/src/1311.0_MinBias_13+MinBias_13+DIGIUP15+RECOMINUP15+HARVESTMINUP15+ALCAMINUP15/SiStripCalMinBias.root"),
        fileNames = cms.untracked.vstring("file:/eos/cms/store/express/Run2018E/StreamExpress/ALCARECO/SiStripCalMinBias-Express-v1/000/325/310/00000/FCD0C1A1-06A4-274B-8373-119CE5C5DF37.root")
     )

process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )
process.MessageLogger.cerr.FwkReport.reportEvery = 10000

inTracks = cms.InputTag("ALCARECOSiStripCalMinBias")

process.load('CalibTracker.SiStripCommon.prescaleEvent_cfi')
process.load('CalibTracker.Configuration.Filter_Refit_cff')
## use CalibrationTracks (for clusters) and CalibrationTracksRefit (for tracks)
process.CalibrationTracks.src = inTracks

tracksForCalib = cms.InputTag("CalibrationTracksRefit")

process.prescaleEvent.prescale = 1

process.TkCalSeq = cms.Sequence(process.prescaleEvent*process.MeasurementTrackerEvent*process.trackFilterRefit)

process.load("PhysicsTools.NanoAOD.nano_cff")
process.load("PhysicsTools.NanoAOD.NanoAODEDMEventContent_cff")

## as a test: it should be possible to add tracks fully at configuration level (+ declaring the plugin)
from PhysicsTools.NanoAOD.common_cff import *
## this is equivalent to ShallowTrackProducer as configured for the gain calibration
process.tracksTable = cms.EDProducer("SimpleTrackFlatTableProducer",
        src=tracksForCalib,
        cut=cms.string(""),
        name=cms.string("track"),
        doc=cms.string("SiStripCalMinBias ALCARECO tracks"),
        singleton=cms.bool(False),
        extension=cms.bool(False),
        variables=cms.PSet(
            chi2ndof=Var("chi2()/ndof", float),
            pt=Var("pt()", float),
            hitsvalid=Var("numberOfValidHits()", int), ## unsigned?
            phi=Var("phi()", float),
            eta=Var("eta()", float),
            )
        )
process.load("CalibTracker.SiStripCommon.siStripPositionCorrectionsTable_cfi")
process.siStripPositionCorrectionsTable.Tracks = tracksForCalib
process.load("CalibTracker.SiStripCommon.siStripLorentzAngleRunInfoTable_cfi")

process.nanoCTPath = cms.Path(process.TkCalSeq*
        process.nanoMetadata
        *process.tracksTable
        *process.siStripPositionCorrectionsTable
        *process.siStripLorentzAngleRunInfoTable
        )

process.out = cms.OutputModule("NanoAODOutputModule",
        fileName=cms.untracked.string("CalibTreeMC_nano_LABP.root"),
        outputCommands=process.NANOAODEventContent.outputCommands+[
            "drop edmTriggerResults_*_*_*"
            ]
        )

process.end = cms.EndPath(process.out)
