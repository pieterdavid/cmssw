#include "DataFormats/SiPixelDetId/interface/PixelEndcapName.h"

#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"

#include <sstream>
#include "FWCore/MessageLogger/interface/MessageLogger.h"

namespace {
  constexpr bool pilot_blade = false;
}

// Decodes the pixel name from the cmssw DetID, uses tracker topology
PixelEndcapName::PixelEndcapName(DetId id, const TrackerTopology* tt, bool phase)
  : PixelModuleName(id)
  , phase1(phase)
{
  const int side = tt->pxfSide(id);
  int tmpBlade   = tt->pxfBlade(id);
  theDisk        = tt->pxfDisk(id);
  thePannel      = tt->pxfPanel(id);

  bool outer = false;   // outer means with respect to the LHC ring (x - axis) Bm(p)I/O
  if(phase1) {  // phase 1
    int ring=0; // ring number , according to radius, 1-lower, 2- higher

    if (tmpBlade>=7 && tmpBlade<=17) {  // ring 1, O
      outer = true;
      theBlade = tmpBlade-6;                      //7...17-->1...11
      ring = 1;   // lower radius

    } else if( tmpBlade<=6 ) {  // ring 1, I
      theBlade = 7-tmpBlade;                      //1...6-->6...1
      ring=1;

    } else if( tmpBlade>=18 && tmpBlade<=22 ) {  // ring 1, I
      theBlade = 29-tmpBlade;                     //18...22-->11...7
      ring=1;

    } else if (tmpBlade>=32 && tmpBlade<=48) {  // ring 2, O
      outer = true;
      theBlade = tmpBlade-31;                     //32...48-->28...12
      ring=2;  // higher radius

    } else if( tmpBlade>=23 && tmpBlade<=31 ) { // ring 2, I
      theBlade = 32-tmpBlade;                     //23...31-->9...1
      ring=2;

    } else if( tmpBlade>=49 && tmpBlade<=56 ) { // ring 2, I
      theBlade = 66-tmpBlade;                     //49...56-->28...21
      ring=2;

    } //  end
    thePlaquette = ring;

  } else { // phase0

    // hack for the pilot blade
    if(pilot_blade && theDisk==3 ) { // do only for disk 3
      //cout<<tmpBlade<<" "<<theDisk<<endl;
      if(tmpBlade>=1 && tmpBlade<=4) {
	// convert the sequential counting of pilot blades to std. cmssw convention
	if(tmpBlade<3) tmpBlade +=3;
	else  tmpBlade +=13;
      } else {
	edm::LogError ("Bad PilotBlade blade number ")
	  << tmpBlade;
      }
    }

    if (tmpBlade >= 7 && tmpBlade <= 18) {
      outer = true;
      theBlade = tmpBlade-6;
    } else if( tmpBlade <=6 ) {
      theBlade = 7-tmpBlade;
    } else if( tmpBlade >= 19) {
      theBlade = 31-tmpBlade;
    }

    thePlaquette = tt->pxfModule(id);  // ring number is coded as plaqutte (plaqs are unused)
  } // end phase1

       if( side == 1 &&  outer ) thePart = mO;
  else if( side == 1 && !outer ) thePart = mI;
  else if( side == 2 &&  outer ) thePart = pO;
  else if( side == 2 && !outer ) thePart = pI;
}

PixelEndcapName::~PixelEndcapName() {}

PixelModuleName::ModuleType PixelEndcapName::moduleType() const
{
  ModuleType type = v1x2;

  if(phase1) { // phase1

    type = v2x8;

  } else { // phase 0

    if (pannelName() == 1) {
      if (plaquetteName() == 1)      { type = v1x2; }
      else if (plaquetteName() == 2) { type = v2x3; }
      else if (plaquetteName() == 3) { type = v2x4; }
      else if (plaquetteName() == 4) { type = v1x5; }
    } else {
      if (plaquetteName() == 1)      { type = v2x3; }
      else if (plaquetteName() == 2) { type = v2x4; }
      else if (plaquetteName() == 3) { type = v2x5; }
    }

    // hack for the pilot blade
    if(pilot_blade && theDisk==3 ) {type=v2x8;} // do only for disk 3

  } // end phase1

  return type;
}

std::string PixelEndcapName::name() const
{
  std::ostringstream stm;

  if(phase1)
    stm <<"FPix_B"<<thePart<<"_D"<<theDisk<<"_BLD"<<theBlade<<"_PNL"<<thePannel<<"_RNG"<<thePlaquette;
  else
    stm <<"FPix_B"<<thePart<<"_D"<<theDisk<<"_BLD"<<theBlade<<"_PNL"<<thePannel<<"_PLQ"<<thePlaquette;

  return stm.str();
}

// constructor from name string
PixelEndcapName PixelEndcapName::fromName(std::string name, const TrackerTopology* tt, bool phase1)
{
  // parse the name string
  // first, check to make sure this is an FPix name, should start with "FPix_"
  // also check to make sure the needed parts are present
  if ( (name.substr(0, 5) != "FPix_") ||
       (name.find("_B") == std::string::npos) ||
       (name.find("_D") == std::string::npos) ||
       (name.find("_BLD") == std::string::npos) ||
       (name.find("_PNL") == std::string::npos) ||
       ( (phase1  && name.find("_RNG") == std::string::npos) ) ||
       ( (!phase1 && name.find("_PLQ") == std::string::npos) ) ) {
    throw std::invalid_argument("Bad name string in PixelEndcapName::PixelEndcapName(std::string): "+name);
  }

  // results of parsing
  HalfCylinder thePart;
  uint32_t disk, tmpBlade, pannel, tmpPlaquette;

  // strip off ROC part if it's there
  if (name.find("_ROC") != std::string::npos)
    name = name.substr(0, name.find("_ROC"));

  // get the half cylinder
  const std::string hcString = name.substr(name.find("_B")+2, name.find("_D")-name.find("_B")-2);
  if (hcString == "mO") thePart = mO;
  else if (hcString == "mI") thePart = mI;
  else if (hcString == "pO") thePart = pO;
  else if (hcString == "pI") thePart = pI;
  else {
    throw std::invalid_argument("Unable to determine half cylinder in PixelEndcapName::PixelEndcapName(std::string): "+name);
  }

  // get the disk
  const std::string diskString = name.substr(name.find("_D")+2, name.find("_BLD")-name.find("_D")-2);
  if (diskString == "1") disk = 1;
  else if (diskString == "2") disk = 2;
  else if (diskString == "3") disk = 3;
  else {
    throw std::invalid_argument("Unable to determine disk number in PixelEndcapName::PixelEndcapName(std::string): "+name);
  }

  // get the blade
  const std::string bladeString = name.substr(name.find("_BLD")+4, name.find("_PNL")-name.find("_BLD")-4);
  // since atoi() doesn't report errors, do it the long way
  if (bladeString == "1") tmpBlade = 1;
  else if (bladeString == "2") tmpBlade = 2;
  else if (bladeString == "3") tmpBlade = 3;
  else if (bladeString == "4") tmpBlade = 4;
  else if (bladeString == "5") tmpBlade = 5;
  else if (bladeString == "6") tmpBlade = 6;
  else if (bladeString == "7") tmpBlade = 7;
  else if (bladeString == "8") tmpBlade = 8;
  else if (bladeString == "9") tmpBlade = 9;
  else if (bladeString == "10") tmpBlade = 10;
  else if (bladeString == "11") tmpBlade = 11;
  else if (bladeString == "12") tmpBlade = 12;
  else if (bladeString == "13") tmpBlade = 13;
  else if (bladeString == "14") tmpBlade = 14;
  else if (bladeString == "15") tmpBlade = 15;
  else if (bladeString == "16") tmpBlade = 16;
  else if (bladeString == "17") tmpBlade = 17;
  else {
    throw std::invalid_argument("Unable to determine blade number in PixelEndcapName::PixelEndcapName(std::string): "+name);
  }

  // find the panel
  const std::string panelString = ( phase1
      ? name.substr(name.find("_PNL")+4, name.find("_RNG")-name.find("_PNL")-4)
      : name.substr(name.find("_PNL")+4, name.find("_PLQ")-name.find("_PNL")-4) );
  if      (panelString == "1") pannel = 1;
  else if (panelString == "2") pannel = 2;
  else {
    throw std::invalid_argument("Unable to determine panel number in PixelEndcapName::PixelEndcapName(std::string): "+name);
  }

  // find the plaquette, for phase 1 this is the rung number
  if (phase1) { // phase1

    const std::string ringString = name.substr(name.find("_RNG")+4, name.size()-name.find("_RNG")-4);
    if (ringString == "1")      tmpPlaquette = 1; // code ring in the plaquette
    else if (ringString == "2") tmpPlaquette = 2;
    else {
      throw std::invalid_argument("Unable to determine ring number in PixelEndcapName::PixelEndcapName(std::string): "+name);
    }

  } else { // phase 0

    // find the plaquette
    const std::string plaquetteString = name.substr(name.find("_PLQ")+4, name.size()-name.find("_PLQ")-4);
    if (plaquetteString == "1") tmpPlaquette = 1;
    else if (plaquetteString == "2") tmpPlaquette = 2;
    else if (plaquetteString == "3") tmpPlaquette = 3;
    else if (plaquetteString == "4") tmpPlaquette = 4;
    else {
      throw std::invalid_argument("Unable to determine plaquette number in PixelEndcapName::PixelEndcapName(std::string): "+name);
    }
  } // end phase1


  // interpret further
  uint32_t side{0}, blade{0}, module{0};

  // figure out the side
  if (thePart == mO || thePart == mI) side = 1;
  else if (thePart == pO || thePart == pI) side = 2;

  // convert blade numbering to cmssw convention
  const bool outer = (thePart == mO) || (thePart == pO);

  if(phase1) { // phase1
    module=1;
    switch (tmpPlaquette) { // this is ring for phase1
      case 1: // ring 1, lower radius
        if (outer) {
          if          (tmpBlade>=1 && tmpBlade<=11)    blade = tmpBlade+6;
        } else { //inner
          if          (tmpBlade<=6 )                   blade = 7-tmpBlade;
          else                                         blade = 29-tmpBlade;
        }
        break;
      case 2: //ring 2, upper radius
        if (outer) {
          if          (tmpBlade>=1 && tmpBlade<=17)    blade = tmpBlade+31;
        } else { //inner
          if          (tmpBlade<=9 )                   blade = 32-tmpBlade;
          else                                         blade = 66-tmpBlade;
        }
        break;
    }
  } else { // phase 0
    if (outer) {
      blade = tmpBlade + 6;
    } else { // inner
      if (tmpBlade <= 6) blade = 7 - tmpBlade;
      else if (tmpBlade <= 12) blade = 31 - tmpBlade;
    }

    // hack for the pilot blade
    if(pilot_blade && disk==3 ) { // do only for disk 3
      //cout<<tmpBlade<<" "<<blade<<endl;
      if(blade<=5) blade -=3;
      else         blade -=13;
      //cout<<tmpBlade<<" "<<blade<<endl;
    }

    module = tmpPlaquette;
  } // end phase1

  return PixelEndcapName(tt->pxfDetId(side, disk, blade, pannel, module), tt, phase1);
}


std::ostream & operator<<( std::ostream& out, const PixelEndcapName::HalfCylinder& t)
{
  switch (t) {
    case(PixelEndcapName::pI) : {out << "pI"; break;}
    case(PixelEndcapName::pO) : {out << "pO"; break;}
    case(PixelEndcapName::mI) : {out << "mI"; break;}
    case(PixelEndcapName::mO) : {out << "mO"; break;}
    default: out << "unknown";
  };
  return out;
}
