#ifndef _0207D4_VEC_XZ_H
#define _0207D4_VEC_XZ_H

#include <shinobi.h>

typedef struct {
    float x;
    float z;
} PointXZ;

/* (a - origin) . (b - origin) */
float VecXZDot_8c0207d4(NJS_POINT3 *origin, NJS_POINT3 *a, PointXZ *b);

/* (a - origin) x (b - origin); the sign is b's side of the origin->a line. */
float VecXZCross_8c0207fa(NJS_POINT3 *origin, PointXZ *a, PointXZ *b);

#endif // _0207D4_VEC_XZ_H
