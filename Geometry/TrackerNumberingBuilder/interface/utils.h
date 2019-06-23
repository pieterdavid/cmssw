#ifndef GEOMETRY_TRACKERNUMBERINGBUILDER_UTILS_H
#define GEOMETRY_TRACKERNUMBERINGBUILDER_UTILS_H

#include "Geometry/TrackerNumberingBuilder/interface/GeometricDet.h"

/**
 * A helper method to get the full list of SiStrip DetIds from the GeometricDet
 *
 * The DetIds are sorted by subdetector, but otherwise keep the ordering from
 * GeometricDet::deepComponents (for compatibility with SiStripDetInfoFileReader)
 */
std::vector<uint32_t> getSiStripDetIds(const GeometricDet& geomDet);

#endif  // GEOMETRY_TRACKERNUMBERINGBUILDER_UTILS_H