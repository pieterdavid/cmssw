#ifndef TIBORTOBPARAMETERS_H
#define TIBORTOBPARAMETERS_H

#include "DataFormats/SiStripDetId/interface/SiStripDetId.h"

// WARNING: This header file is created on the assumption
// that the subDetector Id (TIB or TOB) is already known.
// It is used inside CalibTracker/SiStripLorentzAngle only.


uint32_t getTIBOrTOBLayer( DetId detId )
{
  return ((detId.rawId()>>14) & 0x7);
};

bool getTIBorTOBStereo( DetId detId )
{
  return ( ((detId.rawId()>>0) & 0x3) == 1 ) ? 1 : 0;
};

#endif
