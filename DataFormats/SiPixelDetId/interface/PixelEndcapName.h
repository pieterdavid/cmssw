#ifndef SiPixelDetId_PixelEndcapName_H
#define SiPixelDetId_PixelEndcapName_H

#include "DataFormats/SiPixelDetId/interface/PixelModuleName.h"

class TrackerTopology;

/** \class PixelEndcapName
 * Endcap Module name (as in PixelDatabase) for endcaps
 */
class PixelEndcapName : public PixelModuleName {
public:

  enum HalfCylinder { mO = 1, mI = 2 , pO =3 , pI =4 };

  PixelEndcapName(DetId, const TrackerTopology*, bool phase);
  virtual ~PixelEndcapName();

  /// ctor for defined name
  static PixelEndcapName fromName(std::string, const TrackerTopology*, bool phase);

  // accessors

  HalfCylinder halfCylinder() const { return thePart; }
  /// disk id
  int diskName() const { return theDisk; }
  /// blade id
  int bladeName() const { return theBlade; }
  /// pannel id
  int pannelName() const { return thePannel; }
  /// plaquetteId (in pannel)
  int plaquetteName() const { return thePlaquette; }
  /// ring Id
  int ringName() const { return thePlaquette; }

  // interface methods

  virtual std::string name() const override;
  /// module Type
  virtual PixelModuleName::ModuleType moduleType() const override;

private:
  bool phase1; // TODO make this an enum?
  HalfCylinder thePart;
  int theDisk, theBlade, thePannel, thePlaquette;
};

std::ostream & operator<<( std::ostream& out, const PixelEndcapName::HalfCylinder & t);
#endif
