#ifndef SiPixelDetId_PixelModuleName_H
#define SiPixelDetId_PixelModuleName_H

#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"

/** \class PixelModuleName
 * Base class to Pixel modules naming, provides a name as in PixelDatabase
 */
class PixelModuleName {
public:

  enum ModuleType { v1x2, v1x5, v1x8, v2x3, v2x4, v2x5, v2x8 } ;

  explicit PixelModuleName(DetId detId) : m_detId(detId) {}
  virtual ~PixelModuleName();

  /// return the DetId
  DetId getDetId() const { return m_detId; }

  /// true for barrel modules
  bool isBarrel() const { return isBarrel(m_detId); }
  static bool isBarrel(DetId detId) { return ( PixelSubdetector::PixelBarrel == detId.subdetId() ); }


  /// associated name
  virtual std::string name() const = 0;

  /// module type
  virtual ModuleType moduleType() const = 0;


  /// check equality of modules
  bool operator== (const PixelModuleName& other) const { return m_detId == other.m_detId; }

private:
  DetId m_detId;
};

std::ostream & operator<<(std::ostream& out, const PixelModuleName::ModuleType& t);
#endif
