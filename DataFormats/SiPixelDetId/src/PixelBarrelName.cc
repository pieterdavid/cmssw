#include "DataFormats/SiPixelDetId/interface/PixelBarrelName.h"

#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"

#include <sstream>
#include <iostream>
#include "FWCore/MessageLogger/interface/MessageLogger.h"

namespace {
  // ladder limits
  constexpr int lL[2][12] ={
    // layer 1, layer 2, layer 3 , layer 4
      {5,15,26, 8,24,41, 11,33,56,  0, 0, 0}, // phase0
      {3, 9,16, 7,21,36, 11,33,56, 16,48,81}  // phase1
    };
}

PixelBarrelName::PixelBarrelName(DetId id, const TrackerTopology* tt, bool phase)
  : PixelModuleName(id)
  , phase1(phase)
{
  theLayer = tt->pxbLayer(id);
  const int oldModule = tt->pxbModule(id);  // CMSSW convention
  const int oldLadder = tt->pxbLadder(id);  // CMSSW convention

  const int ladder = convertLadderNumber(oldLadder); // convert to online convention
  int module = oldModule-4; // convert
  if (module<=0) module--;

  // part
  if      (module < 0 && ladder < 0) thePart = mO;
  else if (module > 0 && ladder < 0) thePart = pO;
  else if (module < 0 && ladder > 0) thePart = mI;
  else if (module > 0 && ladder > 0) thePart = pI;

  // ladder
  theLadder = std::abs(ladder);
  // module
  theModule = std::abs(module);
}

PixelBarrelName::~PixelBarrelName() {}

int PixelBarrelName::convertLadderNumber(int oldLadder)
{
  int ladder=-1;
  const int ind = ( phase1 ? 1 : 0 );

  if (theLayer == 1) {
    if       (oldLadder <= lL[ind][0] )
      ladder =(lL[ind][0]+1)-oldLadder;
    else if (oldLadder >= (lL[ind][0]+1) && oldLadder <= lL[ind][1])
      ladder = lL[ind][0]-oldLadder;
    else if (oldLadder >= (lL[ind][1]+1) )
      ladder = lL[ind][2]-oldLadder;

  } else if (theLayer == 2) {

    if      (oldLadder <=  lL[ind][3])
      ladder =(lL[ind][3]+1)-oldLadder;
    else if (oldLadder >= (lL[ind][3]+1) && oldLadder <= lL[ind][4])
      ladder = lL[ind][3]-oldLadder;
    else if (oldLadder >= (lL[ind][4]+1))
      ladder = lL[ind][5]-oldLadder;

  } else if (theLayer == 3) {

    if      (oldLadder <=  lL[ind][6])
      ladder = (lL[ind][6]+1)-oldLadder;
    else if (oldLadder >= (lL[ind][6]+1) && oldLadder <= lL[ind][7])
      ladder = lL[ind][6]-oldLadder;
    else if (oldLadder >= (lL[ind][7]+1))
      ladder = lL[ind][8]-oldLadder;

  } else if (theLayer == 4) {

    if      (oldLadder <=  lL[ind][9])
      ladder =(lL[ind][9]+1)-oldLadder;
    else if (oldLadder >= (lL[ind][9]+1) && oldLadder <= lL[ind][10])
      ladder = lL[ind][9]-oldLadder;
    else if (oldLadder >= (lL[ind][10]+1))
      ladder = lL[ind][11]-oldLadder;

  }

  return ladder;
}

int PixelBarrelName::sectorName() const
{
  int sector = 0;

  switch (theLayer) {
    case 1:
      if (phase1) { // phase 1
        // Layer 1 for phase1 is special, modules on a ladder are shared in different sectors
        // Ladder   Module    Sector
        //   1      2,3,4       1
        //   1        1         2
        //   2       3,4        2
        //   2       1,2        3
        //   3        4         3
        //   3      1,2,3       4
        //   4      2,3,4       5
        //   4        1         6
        //   5       3,4        6
        //   5       1,2        7
        //   6        4         7
        //   6      1,2,3       8

        switch (theLadder) {
        case 1 :  {  // ladder 1
          if(theModule>=2) {sector = 1; break;}  // mods 2,3,4
          else             {sector = 2; break;}  // mods 1
        }
        case 2 :  { // ladder 2
          if(theModule>=3) {sector = 2; break;} // mods 3,4
          else             {sector = 3; break;} // mods 1,2
        }
        case 3 :  { // ladder 3
          if(theModule>=4) {sector = 3; break;}
          else             {sector = 4; break;}
          sector = 3; break;
        }
        case 4 :  { // ladder 4
          if(theModule>=2) {sector = 5; break;}
          else             {sector = 6; break;}
          sector = 6; break;
        }
        case 5 :  { // ladder 5
          if(theModule>=3) {sector = 6; break;}
          else             {sector = 7; break;}
          sector = 7; break;
        }
        case 6 :  { // ladder 6
          if(theModule>=4) {sector = 7; break;} // mods 4
          else             {sector = 8; break;} // mods 1,2,3
          sector = 8; break;
        }
        default: ;
        };
      } else { // phase 0
        switch (theLadder) {
        case 1 : case 2: {sector = 1; break;}
        case 3 :         {sector = 2; break;}
        case 4 :         {sector = 3; break;}
        case 5 :         {sector = 4; break;}
        case 6 :         {sector = 5; break;}
        case 7 :         {sector = 6; break;}
        case 8 :         {sector = 7; break;}
        case 9 : case 10:{sector = 8; break;}
        default: ;
        };
      } // phase0/1
      break;
    case 2:
      if(phase1) { // phase 1
        switch (theLadder) {
        case  1: case  2: {sector = 1; break;}
        case  3: case  4: {sector = 2; break;}
        case  5:          {sector = 3; break;}
        case  6: case  7: {sector = 4; break;}
        case  8: case  9: {sector = 5; break;}
        case 10:          {sector = 6; break;}
        case 11: case 12: {sector = 7; break;}
        case 13: case 14: {sector = 8; break;}
        default: ;
        };
      } else { // phase 0
        switch (theLadder) {
        case  1 : case  2: {sector = 1; break;}
        case  3 : case  4: {sector = 2; break;}
        case  5 : case  6: {sector = 3; break;}
        case  7 : case  8: {sector = 4; break;}
        case  9 : case 10: {sector = 5; break;}
        case 11 : case 12: {sector = 6; break;}
        case 13 : case 14: {sector = 7; break;}
        case 15 : case 16: {sector = 8; break;}
        default: ;
        };
      } // enad phase0/1
      break;
    case 3:
      if(phase1) { // phase 1
        switch (theLadder) {
        case  1: case  2:          {sector = 1; break;}
        case  3: case  4: case  5: {sector = 2; break;}
        case  6: case  7: case  8: {sector = 3; break;}
        case  9: case 10: case 11: {sector = 4; break;}
        case 12: case 13: case 14: {sector = 5; break;}
        case 15: case 16: case 17: {sector = 6; break;}
        case 18: case 19: case 20: {sector = 7; break;}
        case 21: case 22:          {sector = 8; break;}
        default: ;
        };
      } else { // phase 0
        switch (theLadder) {
        case  1 : case  2: case  3: {sector = 1; break;}
        case  4 : case  5: case  6: {sector = 2; break;}
        case  7 : case  8: case  9: {sector = 3; break;}
        case 10 : case 11:          {sector = 4; break;}
        case 12 : case 13:          {sector = 5; break;}
        case 14 : case 15: case 16: {sector = 6; break;}
        case 17 : case 18: case 19: {sector = 7; break;}
        case 20 : case 21: case 22: {sector = 8; break;}
        default: ;
        };
      } // end phase 0/1
      break;
    case 4:
      if(phase1) { // phase 1
        switch (theLadder) {
        case  1 : case  2: case  3:  case  4: {sector = 1; break;}
        case  5 : case  6: case  7:  case  8: {sector = 2; break;}
        case  9 : case 10: case 11:  case 12: {sector = 3; break;}
        case 13 : case 14: case 15:  case 16: {sector = 4; break;}
        case 17 : case 18: case 19:  case 20: {sector = 5; break;}
        case 21 : case 22: case 23:  case 24: {sector = 6; break;}
        case 25 : case 26: case 27:  case 28: {sector = 7; break;}
        case 29 : case 30: case 31:  case 32: {sector = 8; break;}
        default: ;
        };
      } // end phase1
      break;
  } // layer
  return sector;
}

bool PixelBarrelName::isHalfModule() const
{
  bool halfModule = false;

  if ( ! phase1 ) {
    if (theLadder == 1) halfModule = true;
    if (theLayer == 1 && theLadder == 10) halfModule = true;
    if (theLayer == 2 && theLadder == 16) halfModule = true;
    if (theLayer == 3 && theLadder == 22) halfModule = true;
  }
  return halfModule;
}

PixelModuleName::ModuleType PixelBarrelName::moduleType() const
{
  return isHalfModule() ? PixelBarrelName::v1x8 : PixelBarrelName::v2x8;
}

std::string PixelBarrelName::name() const
{
   std::ostringstream stm;

   stm<<"BPix_B"<<thePart<<"_SEC"<<sectorName()<<"_LYR"<<theLayer<<"_LDR"<<theLadder;
   if ( isHalfModule() ) stm <<"H"; else stm <<"F";
   stm << "_MOD" << theModule;

   return stm.str();
}


// constructor from name string
PixelBarrelName PixelBarrelName::fromName(std::string name, const TrackerTopology* tt, bool phase1)
{
  // parse the name string
  // first, check to make sure this is an BPix name, should start with "BPix_"
  // also check to make sure the needed parts are present
  if ( (name.substr(0, 5) != "BPix_") ||
       (name.find("_B") == std::string::npos) ||
       (name.find("_LYR") == std::string::npos) ||
       (name.find("_LDR") == std::string::npos) ||
       (name.find("_MOD") == std::string::npos) ) {
    throw std::invalid_argument("Bad name string in PixelBarrelName::fromName : "+name);
  }

  // results of parsing
  Shell thePart;
  int theLayer, theModule, theLadder;

  // strip off ROC part if it's there
  if (name.find("_ROC") != std::string::npos)
    name = name.substr(0, name.find("_ROC"));

  // find shell
  const std::string shellString = name.substr(name.find("_B")+2, name.find("_SEC")-name.find("_B")-2);
  if (shellString == "mO") thePart = mO;
  else if (shellString == "mI") thePart = mI;
  else if (shellString == "pO") thePart = pO;
  else if (shellString == "pI") thePart = pI;
  else {
    throw std::invalid_argument("Unable to determine shell in PixelBarrelName::fromName(std::string): "+name);
  }

  // find the layer
  const std::string layerString = name.substr(name.find("_LYR")+4, name.find("_LDR")-name.find("_LYR")-4);
  if (layerString == "1") theLayer = 1;
  else if (layerString == "2") theLayer = 2;
  else if (layerString == "3") theLayer = 3;
  else if (phase1 && layerString == "4") theLayer = 4;
  else {
    throw std::invalid_argument("Unable to determine layer in PixelBarrelName::fromName(std::string): "+name);
  }

  // find the ladder
  const std::string ladderString = name.substr(name.find("_LDR")+4, name.find("_MOD")-name.find("_LDR")-4);

  if (phase1) { // phase 1 ladders
    // do we want to kee the "F" for phase1?
    if (ladderString.substr(ladderString.size()-1, 1) == "F") {
      int ladderNum = atoi(ladderString.substr(0, ladderString.size() -1).c_str());
      if (theLayer == 1 && ladderNum >= 1 && ladderNum <= 6) theLadder = ladderNum;
      else if (theLayer == 2 && ladderNum >= 1 && ladderNum <= 14) theLadder = ladderNum;
      else if (theLayer == 3 && ladderNum >= 1 && ladderNum <= 22) theLadder = ladderNum;
      else if (theLayer == 4 && ladderNum >= 1 && ladderNum <= 32) theLadder = ladderNum;
      else {
        throw std::invalid_argument("Unable to determine ladder in PixelBarrelName::fromName(std::string): "+name);
      }
    } // full ladders
    else {
      throw std::invalid_argument("Unable to determine ladder in PixelBarrelName::fromName(std::string): "+name);
    }


  } else { // phase 0 ladders

    if (ladderString == "1H") theLadder = 1;
    else if (ladderString == "10H" && theLayer == 1) theLadder = 10;
    else if (ladderString == "16H" && theLayer == 2) theLadder = 16;
    else if (ladderString == "22H" && theLayer == 3) theLadder = 22;
    else if (ladderString.substr(ladderString.size()-1, 1) == "F") {
      int ladderNum = atoi(ladderString.substr(0, ladderString.size() -1).c_str());
      if (theLayer == 1 && ladderNum > 1 && ladderNum < 10) theLadder = ladderNum;
      else if (theLayer == 2 && ladderNum > 1 && ladderNum < 16) theLadder = ladderNum;
      else if (theLayer == 3 && ladderNum > 1 && ladderNum < 22) theLadder = ladderNum;
      else {
        throw std::invalid_argument("Unable to determine ladder in PixelBarrelName::fromName(std::string): "+name);
      }
    } // full ladders
    else {
      throw std::invalid_argument("Unable to determine ladder in PixelBarrelName::fromName(std::string): "+name);
    }
  } // phase0/1

  // find the module
  const std::string moduleString = name.substr(name.find("_MOD")+4, name.size()-name.find("_MOD")-4);
  if (moduleString == "1") theModule = 1;
  else if (moduleString == "2") theModule = 2;
  else if (moduleString == "3") theModule = 3;
  else if (moduleString == "4") theModule = 4;
  else {
    throw std::invalid_argument("Unable to determine module in PixelBarrelName::fromName(std::string): "+name);
  }

  uint32_t ladder, module;
  // translate the ladder number from the naming convention to the cmssw convention
  const int ind = ( phase1 ? 1 : 0 );
  if ( (thePart == mO) || (thePart == pO) ) { // outer
    switch (theLayer) {
      case 1: ladder = theLadder + lL[ind][0]; break;
      case 2: ladder = theLadder + lL[ind][3]; break;
      case 3: ladder = theLadder + lL[ind][6]; break;
      case 4: ladder = theLadder + lL[ind][9]; break;
    } // theLayer
  } // outer
  else { // inner
    switch (theLayer) {
      case 1:
        if (theLadder <= lL[ind][0]) ladder = (lL[ind][0]+1) - theLadder;
        else                         ladder =  lL[ind][2]    - theLadder;
        break;
      case 2:
        if (theLadder <= lL[ind][3]) ladder = (lL[ind][3]+1) - theLadder;
        else                         ladder =  lL[ind][5]    - theLadder;
        break;
      case 3:
        if (theLadder <= lL[ind][6]) ladder = (lL[ind][6]+1) - theLadder;
        else                         ladder =  lL[ind][8]    - theLadder;
        break;
      case 4:
        if (theLadder <= lL[ind][9]) ladder = (lL[ind][9]+1) - theLadder;
        else                         ladder =  lL[ind][11]   - theLadder;
        break;
    } // theLayer
  } // inner

  // translate the module number from naming convention to cmssw convention
  // numbering starts at positive z
  if (thePart == pO || thePart == pI)
    module = theModule + 4;
  else // negative z side
    module = 5 - theModule;

  return PixelBarrelName(tt->pxbDetId(theLayer, ladder, module), tt, phase1);
}

std::ostream & operator<<( std::ostream& out, const PixelBarrelName::Shell& t)
{
  switch (t) {
    case(PixelBarrelName::pI) : {out << "pI"; break;}
    case(PixelBarrelName::pO) : {out << "pO"; break;}
    case(PixelBarrelName::mI) : {out << "mI"; break;}
    case(PixelBarrelName::mO) : {out << "mO"; break;}
    default: out << "unknown";
  };
  return out;
}
