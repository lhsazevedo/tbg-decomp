#ifndef _02E51C_ATTR_QUERY_H
#define _02E51C_ATTR_QUERY_H

#include "014a9c_tasks.h"

/* =======================
 * Non-initialized Globals
 * =======================
 */

/* Set alongside var_activeGroundGrid_8c2264d4 from the course's pair:
 * atari* -> ground, attr* -> here. */
extern void *var_activeAttrGrid_8c228b3c;

/* The four lookups share one callback shape: find the polygon containing
 * (x, z) and return &polys[*slot].attr_0x08, or NULL on a miss.
 *
 * out is an AttrQueryResult {int *slot; int *vertexIds; int count;}. It
 * carries the previous hit, which is re-tested first; zero it to reset. */

/* Convex polygons only; y unused. */
void *AttrQueryFindConvexPolygon_8c02e51c(float x, float y, float z, void *out);

/* Convex polygons only, and the polygon's height must be near y, to tell an
 * elevated road from the street beneath it. */
void *AttrQueryFindConvexPolygonAtHeight_8c02eab4(float x, float y, float z, void *out);

/* Convex and concave polygons; y unused. */
void *AttrQueryFindPolygon_8c02e69c(float x, float y, float z, void *out);

/* Convex and concave polygons, height-filtered as in 8c02eab4. */
void *AttrQueryFindPolygonAtHeight_8c02ec50(float x, float y, float z, void *out);

/* Reports whether anyone other than self stands on attr id `id`: another
 * traffic entry (signalId_0x410), or the player bus
 * (BusState.fallbackTaskMatchId_0x3a0). */
int AttrQueryRegionOccupied_8c02f08a(
    /* Excluded from the scan */
    Task *self,
    /* An attr_0x08, as returned by the lookups above */
    int id
);

#endif // _02E51C_ATTR_QUERY_H
