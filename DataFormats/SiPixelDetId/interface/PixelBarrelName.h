#ifndef SiPixelDetId_PixelBarrelName_H
#define SiPixelDetId_PixelBarrelName_H

#include "DataFormats/SiPixelDetId/interface/PixelModuleName.h"

class TrackerTopology;

/** \class PixelBarrelName
 * Module name (as in PixelDatabase) in barrel
 */
class PixelBarrelName final : public PixelModuleName {
public:

  enum Shell { mO = 1, mI = 2 , pO =3 , pI =4 };

  PixelBarrelName(DetId, const TrackerTopology*, bool phase);
  virtual ~PixelBarrelName();

  /// ctor from name string
  static PixelBarrelName fromName(std::string, const TrackerTopology*, bool phase);

  // accessors

  Shell shell() const { return thePart; }
  /// layer id
  int layerName() const { return theLayer; }
  /// module id (index in z)
  int moduleName() const { return theModule; }
  /// ladder id (index in phi)
  int ladderName() const { return theLadder; }
  /// sector id
  int sectorName() const;
  /// full or half module
  bool isHalfModule() const;

  // interface methods

  virtual std::string name() const override;
  /// module Type
  virtual PixelModuleName::ModuleType moduleType() const override;

private:
  bool phase1; // TODO make this an enum?
  Shell thePart;
  int theLayer, theModule, theLadder;

  int convertLadderNumber(int oldLadder);
};

std::ostream & operator<<(std::ostream& out, const PixelBarrelName::Shell& t);
#endif
