/* 8c02081c */
#ifndef _02081C_GEOM_H
#define _02081C_GEOM_H

#include <shinobi.h>

/* a is an NJS_POINT3, b a PointXZ. */
float GeomDistanceXZ_8c02081c(void *a, void *b);

/* Convex quads, x/z only. TRUE only if one holds a vertex of the other, so a
 * cross-shaped overlap misses. A one-sided height cull on vertex 1 runs
 * first. */
Bool GeomQuadOverlap_8c020842(NJS_POINT3 *a, NJS_POINT3 *b);

#endif // _02081C_GEOM_H
