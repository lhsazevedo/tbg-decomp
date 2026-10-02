/* @unit VecXZ */
#include "0207d4_vec_xz.h"

/* ====================
 * Functions
 * ====================
 */

float VecXZDot_8c0207d4(NJS_POINT3 *origin, NJS_POINT3 *a, PointXZ *b)
{
    float ax = a->x - origin->x;
    float az = a->z - origin->z;
    float bx = b->x - origin->x;
    float bz = b->z - origin->z;

    return az * bz + bx * ax;
}

float VecXZCross_8c0207fa(NJS_POINT3 *origin, PointXZ *a, PointXZ *b)
{
    float ax = a->x - origin->x;
    float az = a->z - origin->z;
    float bx = b->x - origin->x;
    float bz = b->z - origin->z;

    return ax * bz - az * bx;
}
