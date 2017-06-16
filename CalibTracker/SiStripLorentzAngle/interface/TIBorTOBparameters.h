#ifndef TIBORTOBPARAMETERS_H
#define TIBORTOBPARAMETERS_H

#include "DataFormats/DetId/interface/DetId.h"

//WARNING: these methods assume that the DetId is a
//TIBDetId or TOBDetId (and don't check this).
//Please use the corresponding TrackerTopology
//methods whenever possible.
//Code copied from TIBDetId and TOBDetId.

uint32_t getTIBOrTOBLayer( DetId detId )
{
  return ((detId.rawId()>>14) & 0x7);
};

bool getTIBorTOBStereo( DetId detId )
{
  return (detId.rawId() & 0x3) == 1;
};

#endif
