// -*- C++ -*-
//
// Package:     SiStripMonitorSummary
// Class  :     SiStripClassToMonitorCondData
//
// Original Author:  Evelyne Delmeire
//

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/Common/interface/Handle.h"

#include "DQM/SiStripCommon/interface/ExtractTObject.h"
#include "DQM/SiStripCommon/interface/SiStripFolderOrganizer.h"
#include "DQM/SiStripCommon/interface/SiStripHistoId.h"
#include "DQMServices/Core/interface/DQMStore.h"

#include "DQM/SiStripMonitorSummary/interface/SiStripClassToMonitorCondData.h"

#include "DQM/SiStripMonitorSummary/interface/SiStripApvGainsDQM.h"
#include "DQM/SiStripMonitorSummary/interface/SiStripBackPlaneCorrectionDQM.h"
#include "DQM/SiStripMonitorSummary/interface/SiStripCablingDQM.h"
#include "DQM/SiStripMonitorSummary/interface/SiStripLorentzAngleDQM.h"
#include "DQM/SiStripMonitorSummary/interface/SiStripNoisesDQM.h"
#include "DQM/SiStripMonitorSummary/interface/SiStripPedestalsDQM.h"
#include "DQM/SiStripMonitorSummary/interface/SiStripQualityDQM.h"
#include "DQM/SiStripMonitorSummary/interface/SiStripThresholdDQM.h"

#include "CondFormats/SiStripObjects/interface/SiStripApvGain.h"
#include "CondFormats/SiStripObjects/interface/SiStripBackPlaneCorrection.h"
#include "CondFormats/SiStripObjects/interface/SiStripLorentzAngle.h"
#include "CondFormats/SiStripObjects/interface/SiStripNoises.h"
#include "CondFormats/SiStripObjects/interface/SiStripPedestals.h"

#include "CondFormats/DataRecord/interface/SiStripApvGainRcd.h"
#include "CondFormats/DataRecord/interface/SiStripNoisesRcd.h"
#include "CondFormats/DataRecord/interface/SiStripPedestalsRcd.h"

#include "CalibFormats/SiStripObjects/interface/SiStripQuality.h"
#include "CalibTracker/Records/interface/SiStripQualityRcd.h"

// std
#include <cstdlib>
#include <algorithm>

SiStripClassToMonitorCondData::SiStripClassToMonitorCondData(edm::ParameterSet const &iConfig,
                                                             edm::ConsumesCollector iC)
    : conf_(iConfig) {
  monitorPedestals_ = iConfig.getParameter<bool>("MonitorSiStripPedestal");
  monitorNoises_ = iConfig.getParameter<bool>("MonitorSiStripNoise");
  monitorQuality_ = iConfig.getParameter<bool>("MonitorSiStripQuality");
  monitorApvGains_ = iConfig.getParameter<bool>("MonitorSiStripApvGain");
  monitorLorentzAngle_ = iConfig.getParameter<bool>("MonitorSiStripLorentzAngle");
  monitorBackPlaneCorrection_ = iConfig.getParameter<bool>("MonitorSiStripBackPlaneCorrection");
  monitorLowThreshold_ = iConfig.getParameter<bool>("MonitorSiStripLowThreshold");
  monitorHighThreshold_ = iConfig.getParameter<bool>("MonitorSiStripHighThreshold");
  monitorCabling_ = iConfig.getParameter<bool>("MonitorSiStripCabling");
}

SiStripClassToMonitorCondData::~SiStripClassToMonitorCondData() {}

void SiStripClassToMonitorCondData::beginRun(edm::RunNumber_t iRun, edm::EventSetup const &eSetup) {
  if (monitorPedestals_) {
    pedestalsDQM_ =
        std::make_unique<SiStripPedestalsDQM>(eSetup,
                                              iRun,
                                              conf_.getParameter<edm::ParameterSet>("SiStripPedestalsDQM_PSet"),
                                              conf_.getParameter<edm::ParameterSet>("FillConditions_PSet"));
  }

  if (monitorNoises_) {
    noisesDQM_ = std::make_unique<SiStripNoisesDQM>(eSetup,
                                                    iRun,
                                                    conf_.getParameter<edm::ParameterSet>("SiStripNoisesDQM_PSet"),
                                                    conf_.getParameter<edm::ParameterSet>("FillConditions_PSet"));
  }

  if (monitorQuality_) {
    qualityDQM_ = std::make_unique<SiStripQualityDQM>(eSetup,
                                                      iRun,
                                                      conf_.getParameter<edm::ParameterSet>("SiStripQualityDQM_PSet"),
                                                      conf_.getParameter<edm::ParameterSet>("FillConditions_PSet"));
  }

  if (monitorApvGains_) {
    apvgainsDQM_ =
        std::make_unique<SiStripApvGainsDQM>(eSetup,
                                             iRun,
                                             conf_.getParameter<edm::ParameterSet>("SiStripApvGainsDQM_PSet"),
                                             conf_.getParameter<edm::ParameterSet>("FillConditions_PSet"));
  }

  if (monitorLorentzAngle_) {
    lorentzangleDQM_ =
        std::make_unique<SiStripLorentzAngleDQM>(eSetup,
                                                 iRun,
                                                 conf_.getParameter<edm::ParameterSet>("SiStripLorentzAngleDQM_PSet"),
                                                 conf_.getParameter<edm::ParameterSet>("FillConditions_PSet"));
  }

  if (monitorBackPlaneCorrection_) {
    bpcorrectionDQM_ = std::make_unique<SiStripBackPlaneCorrectionDQM>(
        eSetup,
        iRun,
        conf_.getParameter<edm::ParameterSet>("SiStripBackPlaneCorrectionDQM_PSet"),
        conf_.getParameter<edm::ParameterSet>("FillConditions_PSet"));
  }

  if (monitorLowThreshold_) {
    lowthresholdDQM_ =
        std::make_unique<SiStripThresholdDQM>(eSetup,
                                              iRun,
                                              conf_.getParameter<edm::ParameterSet>("SiStripLowThresholdDQM_PSet"),
                                              conf_.getParameter<edm::ParameterSet>("FillConditions_PSet"));
  }

  if (monitorHighThreshold_) {
    highthresholdDQM_ =
        std::make_unique<SiStripThresholdDQM>(eSetup,
                                              iRun,
                                              conf_.getParameter<edm::ParameterSet>("SiStripHighThresholdDQM_PSet"),
                                              conf_.getParameter<edm::ParameterSet>("FillConditions_PSet"));
  }

  if (monitorCabling_) {
    cablingDQM_ = std::make_unique<SiStripCablingDQM>(eSetup,
                                                      iRun,
                                                      conf_.getParameter<edm::ParameterSet>("SiStripCablingDQM_PSet"),
                                                      conf_.getParameter<edm::ParameterSet>("FillConditions_PSet"));
  }
}

void SiStripClassToMonitorCondData::getModMEsOnDemand(edm::EventSetup const &eSetup, uint32_t requestedDetId) {
  if (monitorPedestals_) {
    pedestalsDQM_->analysisOnDemand(eSetup, requestedDetId);
  }
  if (monitorNoises_) {
    noisesDQM_->analysisOnDemand(eSetup, requestedDetId);
  }
  if (monitorQuality_) {
    qualityDQM_->analysisOnDemand(eSetup, requestedDetId);
    qualityDQM_->fillGrandSummaryMEs(eSetup);
  }  // fillGrand. for SiStripquality
  if (monitorApvGains_) {
    apvgainsDQM_->analysisOnDemand(eSetup, requestedDetId);
  }
  if (monitorLorentzAngle_) {
    lorentzangleDQM_->analysisOnDemand(eSetup, requestedDetId);
  }
  if (monitorBackPlaneCorrection_) {
    bpcorrectionDQM_->analysisOnDemand(eSetup, requestedDetId);
  }
  if (monitorCabling_) {
    cablingDQM_->analysisOnDemand(eSetup, requestedDetId);
  }
  if (monitorLowThreshold_) {
    lowthresholdDQM_->analysisOnDemand(eSetup, requestedDetId);
  }
  if (monitorHighThreshold_) {
    highthresholdDQM_->analysisOnDemand(eSetup, requestedDetId);
  }
}
// -----

//
// ----- getlayerMEsOnDemand
//
void SiStripClassToMonitorCondData::getLayerMEsOnDemand(edm::EventSetup const &eSetup,
                                                        std::string requestedSubDetector,
                                                        uint32_t requestedSide,
                                                        uint32_t requestedLayer) {
  if (monitorPedestals_) {
    pedestalsDQM_->analysisOnDemand(eSetup, requestedSubDetector, requestedSide, requestedLayer);
  }
  if (monitorNoises_) {
    noisesDQM_->analysisOnDemand(eSetup, requestedSubDetector, requestedSide, requestedLayer);
  }
  if (monitorQuality_) {
    qualityDQM_->analysisOnDemand(eSetup, requestedSubDetector, requestedSide, requestedLayer);
    qualityDQM_->fillGrandSummaryMEs(eSetup);
  }
  if (monitorApvGains_) {
    apvgainsDQM_->analysisOnDemand(eSetup, requestedSubDetector, requestedSide, requestedLayer);
  }
  if (monitorLorentzAngle_) {
    lorentzangleDQM_->analysisOnDemand(eSetup, requestedSubDetector, requestedSide, requestedLayer);
  }
  if (monitorBackPlaneCorrection_) {
    bpcorrectionDQM_->analysisOnDemand(eSetup, requestedSubDetector, requestedSide, requestedLayer);
  }
  if (monitorCabling_) {
    cablingDQM_->analysisOnDemand(eSetup, requestedSubDetector, requestedSide, requestedLayer);
  }
  if (monitorLowThreshold_) {
    lowthresholdDQM_->analysisOnDemand(eSetup, requestedSubDetector, requestedSide, requestedLayer);
  }
  if (monitorHighThreshold_) {
    highthresholdDQM_->analysisOnDemand(eSetup, requestedSubDetector, requestedSide, requestedLayer);
  }
}

//
// ----- Analyze
//
void SiStripClassToMonitorCondData::analyseCondData(edm::EventSetup const &eSetup) {
  if (monitorPedestals_) {
    pedestalsDQM_->analysis(eSetup);
  }
  if (monitorNoises_) {
    noisesDQM_->analysis(eSetup);
  }
  if (monitorQuality_) {
    qualityDQM_->analysis(eSetup);
    qualityDQM_->fillGrandSummaryMEs(eSetup);
  }  // fillGrand. for SiStripquality
  if (monitorApvGains_) {
    apvgainsDQM_->analysis(eSetup);
  }
  if (monitorLorentzAngle_) {
    lorentzangleDQM_->analysis(eSetup);
  }
  if (monitorBackPlaneCorrection_) {
    bpcorrectionDQM_->analysis(eSetup);
  }
  if (monitorCabling_) {
    cablingDQM_->analysis(eSetup);
  }
  if (monitorLowThreshold_) {
    lowthresholdDQM_->analysis(eSetup);
  }
  if (monitorHighThreshold_) {
    highthresholdDQM_->analysis(eSetup);
  }

}  // analyze
// -----

void SiStripClassToMonitorCondData::end() {
  if (monitorPedestals_) {
    pedestalsDQM_->end();
  }
  if (monitorNoises_) {
    noisesDQM_->end();
  }
  if (monitorLowThreshold_) {
    lowthresholdDQM_->end();
  }
  if (monitorHighThreshold_) {
    highthresholdDQM_->end();
  }
  if (monitorApvGains_) {
    apvgainsDQM_->end();
  }
  if (monitorLorentzAngle_) {
    lorentzangleDQM_->end();
  }
  if (monitorBackPlaneCorrection_) {
    bpcorrectionDQM_->end();
  }
  if (monitorQuality_) {
    qualityDQM_->end();
  }
  if (monitorCabling_) {
    cablingDQM_->end();
  }
}

void SiStripClassToMonitorCondData::save() {
  bool outputMEsInRootFile = conf_.getParameter<bool>("OutputMEsInRootFile");
  std::string outputFileName = conf_.getParameter<std::string>("OutputFileName");

  DQMStore *dqmStore_ = edm::Service<DQMStore>().operator->();

  if (outputMEsInRootFile) {
    dqmStore_->save(outputFileName);
  }
}
