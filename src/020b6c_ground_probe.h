#ifndef _020B6C_GROUND_PROBE_H
#define _020B6C_GROUND_PROBE_H

#include "020914_ground_query.h"

/* Track* re-test the previous match in *out before searching the cell.
 * *AtHeight reject polygons far from y, telling elevated roads from the
 * streets beneath. */

void GroundProbeTrackPolygon_8c020b6c(float x, float y, float z, GroundQueryResult *out);
void GroundProbeFindPolygonAtHeight_8c020fe4(float x, float y, float z, GroundQueryResult *out);
void GroundProbeTrackPolygonAtHeight_8c021290(float x, float y, float z, GroundQueryResult *out);

/* Sets point[1] on the matched polygon's plane; a miss leaves it alone. */
void GroundProbeInterpolateHeight_8c020f7e(GroundQueryResult *result, float *point);

#endif // _020B6C_GROUND_PROBE_H
