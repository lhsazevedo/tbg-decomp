/* @unit Geom */
#include <shinobi.h>

#include "02081c_geom.h"

/* ====================
 * Functions
 * ====================
 */

float GeomDistanceXZ_8c02081c(void *a, void *b)
{
    Float *pa = (Float *)a;
    Float *pb = (Float *)b;
    Float dx = pa[0] - pb[0];
    Float dz = pa[2] - pb[1];

    return njSqrt(dz * dz + dx * dx);
}

Bool GeomQuadOverlap_8c020842(NJS_POINT3 *a, NJS_POINT3 *b)
{
    Sint32 i, j;

    if (a[1].y - b[1].y > 5.0f) {
        return FALSE;
    }

    for (i = 0; i < 4; i++) {
        Float prevDx = b[3].x - a[i].x;
        Float prevDz = b[3].z - a[i].z;

        for (j = 0; j < 4; j++) {
            Float curDx = b[j].x - a[i].x;
            Float curDz = b[j].z - a[i].z;

            if (prevDx * curDz - prevDz * curDx < 0.0f) {
                break;
            }

            prevDx = curDx;
            prevDz = curDz;
        }

        if (j == 4) {
            return TRUE;
        }
    }

    for (i = 0; i < 4; i++) {
        Float prevDx = a[3].x - b[i].x;
        Float prevDz = a[3].z - b[i].z;

        for (j = 0; j < 4; j++) {
            Float curDx = a[j].x - b[i].x;
            Float curDz = a[j].z - b[i].z;

            if (prevDx * curDz - prevDz * curDx < 0.0f) {
                break;
            }

            prevDx = curDx;
            prevDz = curDz;
        }

        if (j == 4) {
            return TRUE;
        }
    }

    return FALSE;
}
