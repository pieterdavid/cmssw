#ifndef TRACKER_TOPOLOGY_LEGACY_H
#define TRACKER_TOPOLOGY_LEGACY_H

#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "DataFormats/DetId/interface/DetId.h"

namespace LegacyTrackerTopology {
  
  inline static std::unique_ptr<TrackerTopology> getTrackerTopology()
  {
        TrackerTopology::PixelBarrelValues pxb;
        pxb.layerStartBit_ =  16;
        pxb.ladderStartBit_ = 8;
        pxb.moduleStartBit_ = 2;
        pxb.layerMask_ =  0xF;
        pxb.ladderMask_ = 0xFF;
        pxb.moduleMask_ = 0x3F;

        TrackerTopology::PixelEndcapValues pxf;
        pxf.sideStartBit_ =   23;
        pxf.diskStartBit_ =   16;
        pxf.bladeStartBit_ =  10;
        pxf.panelStartBit_ =  8;
        pxf.moduleStartBit_ = 2;
        pxf.sideMask_ =   0x3;
        pxf.diskMask_ =   0xF;
        pxf.bladeMask_ =  0x3F;
        pxf.panelMask_ =  0x3;
        pxf.moduleMask_ = 0x3F;

        TrackerTopology::TECValues tecVals;
        tecVals.sideStartBit_ =        18;
        tecVals.wheelStartBit_ =       14;
        tecVals.petal_fw_bwStartBit_ = 12;
        tecVals.petalStartBit_ =       8;
	tecVals.ringStartBit_ =        5;
	tecVals.moduleStartBit_ =      2;
	tecVals.sterStartBit_ =        0;
	tecVals.sideMask_ =        0x3;
	tecVals.wheelMask_ =       0xF;
	tecVals.petal_fw_bwMask_ = 0x3;
	tecVals.petalMask_ =       0xF;
	tecVals.ringMask_ =        0x7;
	tecVals.moduleMask_ =      0x7;
	tecVals.sterMask_ =        0x3;

        TrackerTopology::TIBValues tibVals;
        tibVals.layerStartBit_ =       14;
	tibVals.str_fw_bwStartBit_ =   12;
	tibVals.str_int_extStartBit_ = 10;
	tibVals.strStartBit_ =         4;
	tibVals.moduleStartBit_ =      2;
	tibVals.sterStartBit_ =        0;
	tibVals.layerMask_ =       0x7;
	tibVals.str_fw_bwMask_ =   0x3;
	tibVals.str_int_extMask_ = 0x3;
	tibVals.strMask_ =         0x3F;
	tibVals.moduleMask_ =      0x3;
	tibVals.sterMask_ =        0x3;
    
	TrackerTopology::TIDValues tidVals;
	tidVals.sideStartBit_ =         13;
	tidVals.wheelStartBit_ =        11;
	tidVals.ringStartBit_ =         9;
	tidVals.module_fw_bwStartBit_ = 7;
	tidVals.moduleStartBit_ =       2;
	tidVals.sterStartBit_ =         0;
	tidVals.sideMask_ =         0x3;
	tidVals.wheelMask_ =        0x3;
	tidVals.ringMask_ =         0x3;
	tidVals.module_fw_bwMask_ = 0x3;
	tidVals.moduleMask_ =       0x1F;
	tidVals.sterMask_ =         0x3;
	
        TrackerTopology::TOBValues tobVals;
	tobVals.layerStartBit_ =     14;
        tobVals.rod_fw_bwStartBit_ = 12;
        tobVals.rodStartBit_ =       5;
        tobVals.moduleStartBit_ =    2;
        tobVals.sterStartBit_ =      0;
        tobVals.layerMask_ =     0x7;
        tobVals.rod_fw_bwMask_ = 0x3;
        tobVals.rodMask_ =       0x7F;
        tobVals.moduleMask_ =    0x7;
        tobVals.sterMask_ =      0x3;

        std::unique_ptr<TrackerTopology> tTopo(new TrackerTopology(pxb, pxf, tecVals, tibVals, tidVals, tobVals));
        
        return tTopo;
  }
};

#endif //
