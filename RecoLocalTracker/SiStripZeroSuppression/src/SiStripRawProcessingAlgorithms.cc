#include "RecoLocalTracker/SiStripZeroSuppression/interface/SiStripRawProcessingAlgorithms.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/SiStripDigi/interface/SiStripDigi.h"
#include "DataFormats/SiStripDigi/interface/SiStripRawDigi.h"
#include "DataFormats/SiStripDetId/interface/SiStripDetId.h"
#include "RecoLocalTracker/SiStripZeroSuppression/interface/SiStripRawProcessingFactory.h"
#include <memory>

SiStripRawProcessingAlgorithms::SiStripRawProcessingAlgorithms(std::unique_ptr<SiStripPedestalsSubtractor> ped,
                                                               std::unique_ptr<SiStripCommonModeNoiseSubtractor> cmn,
                                                               std::unique_ptr<SiStripFedZeroSuppression> zs,
                                                               std::unique_ptr<SiStripAPVRestorer> res,
                                                               bool doAPVRest,
                                                               bool useCMMap,
                                                               std::vector<uint32_t> debugModules)
    : subtractorPed(std::move(ped)),
      subtractorCMN(std::move(cmn)),
      suppressor(std::move(zs)),
      restorer(std::move(res)),
      doAPVRestore(doAPVRest),
      useCMMeanMap(useCMMap),
      debugModules_(debugModules) {

  std::sort(debugModules_.begin(), debugModules_.end());
}

bool SiStripRawProcessingAlgorithms::shouldDebug(uint32_t id) const {
  const auto dbg_it = std::lower_bound(debugModules_.begin(), debugModules_.end(), id);
  return ( debugModules_.end() != dbg_it ) && ( *dbg_it == id );
}

void SiStripRawProcessingAlgorithms::initialize(const edm::EventSetup& es) {
  subtractorPed->init(es);
  subtractorCMN->init(es);
  suppressor->init(es);
  if (restorer.get())
    restorer->init(es);

  edm::ESHandle<TrackerGeometry> tracker;
  es.get<TrackerDigiGeometryRecord>().get(tracker);
  trGeo = tracker.product();
}

void SiStripRawProcessingAlgorithms::initialize(const edm::EventSetup& es, const edm::Event& e) {
  initialize(es);
  if (restorer.get() && doAPVRestore && useCMMeanMap)
    restorer->loadMeanCMMap(e);
}

/**
 * Zero-suppress "hybrid" raw data
 *
 * Subtracts common-mode noise, and inspects the digis then.
 * If flagged by the APV inspector, the zero-suppression is performed as usual;
 * otherwise the positive inputs are copied.
 *
 * @param id module DetId
 * @param firstAPV index of the first APV considered
 * @param procRawDigis input (processed raw) ADCs. Note that this means that
 * ADCs for all strips are expected, so zero-suppressed data should be filled with zeros for the suppressed strips.
 * Output: the ADCs after all subtractions, but before zero-suppression.
 * @param output zero-suppressed digis
 * @return number of restored APVs
 */
//IMPORTANT: don't forget the conversion from  hybrids on the bad APVs (*2 -1024)
uint16_t SiStripRawProcessingAlgorithms::suppressHybridData(uint32_t id,
                                                            uint16_t firstAPV,
                                                            digivector_t& procRawDigis,
                                                            edm::DetSet<SiStripDigi>& suppressedDigis) {
  const auto debug = shouldDebug(id);
  digivector_t procRawDigisPedSubtracted(procRawDigis);

  subtractorCMN->subtract(id, firstAPV, procRawDigis);
  if ( debug ) {
    std::stringstream cmS;
    for ( const auto itm : subtractorCMN->getAPVsCM() ) {
      cmS << "" << itm.first << ": " << itm.second << ", ";
    }
    LogDebug("SiStripZeroSuppression") << "Common modes (starting with APV#" << firstAPV << "): " << cmS.str();
  }

  const auto nAPVFlagged =
      restorer->inspectAndRestore(id, firstAPV, procRawDigisPedSubtracted, procRawDigis, subtractorCMN->getAPVsCM());

  const std::vector<bool>& apvf = getAPVFlags();
  const std::size_t nAPVs = procRawDigis.size() / 128;
  for (uint16_t iAPV = firstAPV; iAPV < firstAPV + nAPVs; ++iAPV) {
    if (apvf[iAPV]) {
      const auto firstDigiIt = std::begin(procRawDigis) + 128 * (iAPV - firstAPV);
      std::vector<int16_t> singleAPVdigi(firstDigiIt, firstDigiIt + 128);
      suppressor->suppress(singleAPVdigi, iAPV, suppressedDigis);
    } else {
      for (uint16_t i = 0; i < 128; ++i) {
        const int16_t digi = procRawDigisPedSubtracted[128 * (iAPV - firstAPV) + i];
        if (digi > 0) {
          suppressedDigis.push_back(SiStripDigi(iAPV * 128 + i, suppressor->truncate(digi)));
        }
      }
    }
  }

  return nAPVFlagged;
}

/**
 * Zero-suppress "hybrid" raw data
 *
 * Subtracts common-mode noise, and inspects the digis then.
 * If flagged by the APV inspector, the zero-suppression is performed as usual.
 * Otherwise, the positive inputs are copied.
 *
 * @param hybridDigis input ADCs in ZS format (regular ZS or "hybrid", i.e. processed as x->(x+1024-ped)/2)
 * @param output zero-suppressed digis
 * @param RawDigis processed ADCs
 * @return number of restored APVs
 */
uint16_t SiStripRawProcessingAlgorithms::suppressHybridData_faster(const edm::DetSet<SiStripDigi>& hybridDigis,
                                                                   edm::DetSet<SiStripDigi>& suppressedDigis) {
  if (hybridDigis.empty()) {
    // TODO check if this ever happens - will anything be unpacked then?
    return 0;
  }
  const auto debug = shouldDebug(hybridDigis.id);
  const auto stripModuleGeom = dynamic_cast<const StripGeomDetUnit*>(trGeo->idToDetUnit(hybridDigis.id));
  const std::size_t nStrips = stripModuleGeom->specificTopology().nstrips();
  const std::size_t nAPVs = nStrips / 128;

  std::vector<bool> apvFlags(nAPVs, false);

  uint16_t nAPVFlagged = 0;
  auto beginAPV = std::cbegin(hybridDigis);
  for (std::size_t iAPV{0}; iAPV != nAPVs; ++iAPV) {
    const auto endAPV = (iAPV + 1 != nAPVs)
                            ? std::lower_bound(beginAPV, std::cend(hybridDigis), SiStripDigi((iAPV + 1) * 128, 0))
                            : std::cend(hybridDigis);
    const auto nDigisInAPV = std::distance(beginAPV, endAPV);
    if (nDigisInAPV > 64) {
      apvFlags[iAPV] = true;
      digivector_t procRawDigis(128, -1024);
      for (auto it = beginAPV; it != endAPV; ++it) {
        procRawDigis[it->strip() - 128 * iAPV] = it->adc() * 2 - 1024;
      }
      if (debug) {
        std::stringstream rawDigiS;
        for ( const auto rd : procRawDigis ) {
          rawDigiS << rd << " ";
        }
        LogDebug("SiStripZeroSuppression") << "[faster] Raw digis for APV#" << iAPV << ": " << rawDigiS.str();
      }
      // hybrid reusing previous code - my be able to sve a bit more
      const auto nFlag = suppressHybridData(hybridDigis.id, iAPV, procRawDigis, suppressedDigis);
      if ( nFlag != 1 )
        LogDebug("SiStripZeroSuppression") << nDigisInAPV << " digis but not flagged by hybrid inspector!!!";
      nAPVFlagged += nFlag;
    } else {  // already zero-suppressed, copy and truncate
      std::transform(beginAPV, endAPV, std::back_inserter(suppressedDigis), [this](const SiStripDigi inDigi) {
        return SiStripDigi(inDigi.strip(), suppressor->truncate(inDigi.adc()));
      });
    }
    beginAPV = endAPV;
  }
  if (debug) {
    std::stringstream flagsS;
    for ( const auto flag : apvFlags ) {
      if (flag)
        flagsS << "T";
      else
        flagsS << "F";
    }
    LogDebug("SiStripZeroSuppression") << "[faster] Det " << hybridDigis.id << " APV flags: " << flagsS.str();
  }
  return nAPVFlagged;
}

uint16_t SiStripRawProcessingAlgorithms::suppressHybridData(const edm::DetSet<SiStripDigi>& hybridDigis,
                                                            edm::DetSet<SiStripDigi>& suppressedDigis,
                                                            digivector_t& rawDigis) {
  convertHybridDigiToRawDigiVector(hybridDigis, rawDigis);
  return suppressHybridData(hybridDigis.id, 0, rawDigis, suppressedDigis);
}

/**
 * Convert hybrid digis to a list of processed raw ADCs
 *
 * Non-zero-suppressed APVs are identified by the number of ADCs found (above 64),
 * and the ADCs converted into normal processed format (x->x*2-1024).
 * For zero-supppressed APVs the absent strips are set to zero ADC.
 *
 * @param inDigis input (non-ZS hybrid or ZS) data
 * @param RawDigis processed raw (or zero-filled ZS) ADCs
 */
void SiStripRawProcessingAlgorithms::convertHybridDigiToRawDigiVector(const edm::DetSet<SiStripDigi>& inDigis,
                                                                      digivector_t& rawDigis) {
  const auto debug = shouldDebug(inDigis.id);
  const auto stripModuleGeom = dynamic_cast<const StripGeomDetUnit*>(trGeo->idToDetUnit(inDigis.id));
  const std::size_t nStrips = stripModuleGeom->specificTopology().nstrips();
  const std::size_t nAPVs = nStrips / 128;

  rawDigis.assign(nStrips, 0);
  std::vector<uint16_t> stripsPerAPV(nAPVs, 0);

  for (SiStripDigi digi : inDigis) {
    rawDigis[digi.strip()] = digi.adc();
    ++stripsPerAPV[digi.strip() / 128];
  }

  for (uint16_t iAPV = 0; iAPV < nAPVs; ++iAPV) {
    if (stripsPerAPV[iAPV] > 64) {
      for (uint16_t strip = iAPV * 128; strip < (iAPV + 1) * 128; ++strip)
        rawDigis[strip] = rawDigis[strip] * 2 - 1024;
    }
    if (debug) {
      std::stringstream rawDigiS;
      for (uint16_t strip = iAPV * 128; strip != (iAPV + 1) * 128; ++strip) {
        rawDigiS << rawDigis[strip] << " ";
      }
      LogDebug("SiStripZeroSuppression") << "[legacy] Raw digis for APV#" << iAPV << ": " << rawDigiS.str();
    }
  }
  if (debug) {
    std::stringstream flagsS;
    for ( const auto nAPVDigis : stripsPerAPV ) {
      if (nAPVDigis > 64)
        flagsS << "T";
      else
        flagsS << "F";
    }
    LogDebug("SiStripZeroSuppression") << "[legacy] Det " << inDigis.id << " APV flags: " << flagsS.str();
  }
}

/**
 * Zero-suppress virgin raw data.
 *
 * Subtracts pedestals and common-mode noise, and (optionally, if doAPVRestore)
 * re-evaluates and subtracts the baseline.
 *
 * @param id module DetId
 * @param firstAPV index of the first APV to consider
 * @param procRawDigis input (virgin raw) ADCs. Output: the ADCs after all subtractions, but before zero-suppression
 * @param output zero-suppressed digis
 * @return number of restored APVs
 */
uint16_t SiStripRawProcessingAlgorithms::suppressVirginRawData(uint32_t id,
                                                               uint16_t firstAPV,
                                                               digivector_t& procRawDigis,
                                                               edm::DetSet<SiStripDigi>& output) {
  subtractorPed->subtract(id, firstAPV * 128, procRawDigis);
  return suppressProcessedRawData(id, firstAPV, procRawDigis, output);
}

/**
 * Zero-suppress virgin raw data.
 *
 * Subtracts pedestals and common-mode noise, and (optionally, if doAPVRestore)
 * re-evaluates and subtracts the baseline.
 *
 * @param rawDigis input (virgin) raw digis
 * @param output zero-suppressed digis
 * @return number of restored APVs
 */
uint16_t SiStripRawProcessingAlgorithms::suppressVirginRawData(const edm::DetSet<SiStripRawDigi>& rawDigis,
                                                               edm::DetSet<SiStripDigi>& output) {
  digivector_t rawdigis;
  rawdigis.reserve(rawDigis.size());
  std::transform(std::begin(rawDigis), std::end(rawDigis), std::back_inserter(rawdigis), [](SiStripRawDigi digi) {
    return digi.adc();
  });
  return suppressVirginRawData(rawDigis.id, 0, rawdigis, output);
}

/**
 * Zero-suppress processed (pedestals-subtracted) raw data.
 *
 * Subtracts common-mode noise and (optionally, if doAPVRestore)
 * re-evaluates and subtracts the baseline.
 *
 * @param id module DetId
 * @param firstAPV index of the first APV to consider
 * @param procRawDigis input (processed raw) ADCs. Output: the ADCs after all subtractions, but before zero-suppression
 * @param output zero-suppressed digis
 * @return number of restored APVs
 */
uint16_t SiStripRawProcessingAlgorithms::suppressProcessedRawData(uint32_t id,
                                                                  uint16_t firstAPV,
                                                                  digivector_t& procRawDigis,
                                                                  edm::DetSet<SiStripDigi>& output) {
  digivector_t procRawDigisPedSubtracted;

  int16_t nAPVFlagged = 0;
  if (doAPVRestore)
    procRawDigisPedSubtracted.assign(procRawDigis.begin(), procRawDigis.end());
  subtractorCMN->subtract(id, firstAPV, procRawDigis);
  if (doAPVRestore)
    nAPVFlagged =
        restorer->inspectAndRestore(id, firstAPV, procRawDigisPedSubtracted, procRawDigis, subtractorCMN->getAPVsCM());
  suppressor->suppress(procRawDigis, firstAPV, output);
  return nAPVFlagged;
}

/**
 * Zero-suppress processed (pedestals-subtracted) raw data.
 *
 * Subtracts common-mode noise and (optionally, if doAPVRestore)
 * re-evaluates and subtracts the baseline.
 *
 * @param rawDigis input (processed) raw digis
 * @param output zero-suppressed digis
 * @return number of restored APVs
 */
uint16_t SiStripRawProcessingAlgorithms::suppressProcessedRawData(const edm::DetSet<SiStripRawDigi>& rawDigis,
                                                                  edm::DetSet<SiStripDigi>& output) {
  digivector_t rawdigis;
  rawdigis.reserve(rawDigis.size());
  std::transform(std::begin(rawDigis), std::end(rawDigis), std::back_inserter(rawdigis), [](SiStripRawDigi digi) {
    return digi.adc();
  });
  return suppressProcessedRawData(rawDigis.id, 0, rawdigis, output);
}

/**
 * Zero-suppress virgin raw data in "hybrid" mode
 *
 * Subtracts pedestals (in 11bit mode, x->(x+1024-ped)/2) and common-mode noise, and inspects the digis then.
 * If not flagged by the hybrid APV inspector, the zero-suppression is performed as usual
 * (evaluation and subtraction of the baseline, truncation).
 * Otherwise, the pedestal-subtracted digis (as above) are saved in one 128-strip cluster.
 * Note: the APV restorer is used, it must be configured with APVInspectMode='HybridEmulation' if this method is called.
 *
 * @param id module DetId
 * @param firstAPV index of the first APV considered
 * @param procRawDigis input (virgin raw) ADCs. Output: the ADCs after all subtractions, but before zero-suppression
 * @param output zero-suppressed digis (or pedestal-subtracted digis, see above)
 * @return number of restored APVs
 */
uint16_t SiStripRawProcessingAlgorithms::convertVirginRawToHybrid(uint32_t id,
                                                                  uint16_t firstAPV,
                                                                  digivector_t& procRawDigis,
                                                                  edm::DetSet<SiStripDigi>& output) {
  digivector_t procRawDigisPedSubtracted;

  for (auto& digi : procRawDigis) {
    digi += 1024;
  }  // adding one MSB

  subtractorPed->subtract(id, firstAPV * 128, procRawDigis);  // all strips are pedestals subtracted

  for (auto& digi : procRawDigis) {
    digi /= 2;
  }

  procRawDigisPedSubtracted.assign(procRawDigis.begin(), procRawDigis.end());

  subtractorCMN->subtract(id, firstAPV, procRawDigis);

  const auto nAPVFlagged = restorer->inspect(id, firstAPV, procRawDigis, subtractorCMN->getAPVsCM());

  for (auto& digi : procRawDigis) {
    digi *= 2;
  }

  const std::vector<bool>& apvf = getAPVFlags();
  const std::size_t nAPVs = procRawDigis.size() / 128;
  for (uint16_t iAPV = firstAPV; iAPV < nAPVs + firstAPV; ++iAPV) {
    if (apvf[iAPV]) {
      //GB 23/6/08: truncation should be done at the very beginning
      for (uint16_t i = 0; i < 128; ++i) {
        const int16_t digi = procRawDigisPedSubtracted[128 * (iAPV - firstAPV) + i];
        output.push_back(SiStripDigi(128 * iAPV + i, (digi < 0 ? 0 : suppressor->truncate(digi))));
      }
    } else {
      const auto firstDigiIt = std::begin(procRawDigis) + 128 * (iAPV - firstAPV);
      std::vector<int16_t> singleAPVdigi(firstDigiIt, firstDigiIt + 128);
      suppressor->suppress(singleAPVdigi, iAPV, output);
    }
  }

  return nAPVFlagged;
}

/**
 * Zero-suppress virgin raw data in "hybrid" mode
 *
 * Subtracts pedestals (in 11bit mode, x->(x+1024-ped)/2) and common-mode noise, and inspects the digis then.
 * If flagged by the hybrid APV inspector, the zero-suppression is performed as usual
 * (evaluation and subtraction of the baseline, truncation).
 * Otherwise, the pedestal-subtracted digis are saved in one 128-strip cluster.
 *
 * @param rawDigis input (virgin) raw digis
 * @param output zero-suppressed digis (or pedestal-subtracted digis, see above)
 * @return number of restored APVs
 */
uint16_t SiStripRawProcessingAlgorithms::convertVirginRawToHybrid(const edm::DetSet<SiStripRawDigi>& rawDigis,
                                                                  edm::DetSet<SiStripDigi>& suppressedDigis) {
  digivector_t rawdigis;
  rawdigis.reserve(rawDigis.size());
  std::transform(std::begin(rawDigis), std::end(rawDigis), std::back_inserter(rawdigis), [](SiStripRawDigi digi) {
    return digi.adc();
  });
  return convertVirginRawToHybrid(rawDigis.id, 0, rawdigis, suppressedDigis);
}
