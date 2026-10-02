/* @unit GroundQuery */

#include <shinobi.h>
#include "includes.h" /* TWO_PI */
#include "020914_ground_query.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define CELL_SIZE 150.0f

/* ====================
 * Non-initialized Globals
 * ====================
 */

GroundGrid* var_activeGroundGrid_8c2264d4;

/* ====================
 * Functions
 * ====================
 */

void GroundQueryFindPolygon_8c020914(float x, float y, float z, GroundQueryResult *out)
{
    GroundGrid *grid = var_activeGroundGrid_8c2264d4;
    GroundCell *cell;
    int cx, cz;
    int i;

    cx = (int)(x / CELL_SIZE);
    if (cx >= grid->cellsX_0x00) {
        cx = grid->cellsX_0x00 - 1;
    }

    cz = (int)(z / CELL_SIZE);
    if (cz >= grid->cellsZ_0x04) {
        cz = grid->cellsZ_0x04 - 1;
    }

    /* Original: only the upper bound is clamped. */
    cell = &grid->cells_0x10[cz * grid->cellsX_0x00 + cx];

    for (i = 0; i < cell->count_0x00; i++) {
        int *slot = &cell->polyIds_0x04[i];
        GroundPoly *poly = &grid->polys_0x14[*slot];
        int n = poly->vertexCount_0x00;
        int *ids = poly->vertexIds_0x04;
        GroundVertex *v;
        float pdx, pdz;
        int hit;
        int j;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        if (poly->attr_0x14 >= 0) {
            for (j = 0; j < n; j++) {
                float dx, dz;

                v = &grid->verts_0x18[ids[j]];
                dx = v->x - x;
                dz = v->z - z;

                if (0.0f > pdx * dz - pdz * dx) {
                    break;
                }

                pdx = dx;
                pdz = dz;
            }

            hit = (j == n);
        } else {
            int acc = 0;

            for (j = 0; j < n; j++) {
                float dx, dz, cross, cosT;
                int angle;

                v = &grid->verts_0x18[ids[j]];
                dx = v->x - x;
                dz = v->z - z;

                cross = pdx * dz - pdz * dx;
                cosT = (pdx * dx + pdz * dz)
                     / (njSqrt(pdx * pdx + pdz * pdz) * njSqrt(dx * dx + dz * dz));

                if (cosT > 1.0f) {
                    cosT = 1.0f;
                } else if (-1.0f > cosT) {
                    cosT = -1.0f;
                }

                angle = (int)(acosf(cosT) * 65536.0f / TWO_PI);
                if (0.0f > cross) {
                    acc -= angle;
                } else {
                    acc += angle;
                }

                pdx = dx;
                pdz = dz;
            }

            hit = acc > 0x8000;
        }

        if (hit) {
            out->attr_0x00 = poly->attr_0x14 & 0x7fffffff;
            out->polyIdSlot_0x04 = slot;
            out->vertexIds_0x08 = poly->vertexIds_0x04;
            out->count_0x0c = n;
            return;
        }
    }

    out->vertexIds_0x08 = 0;
    out->count_0x0c = 0;
}
