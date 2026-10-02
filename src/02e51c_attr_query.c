/* @unit AttrQuery */

#include <shinobi.h>
#include "includes.h" /* TWO_PI */

#include "02e51c_attr_query.h"
#include "026710_traffic.h"  /* TrafficEntry */
#include "1ba1c8_globals.h"

/* ====================
 * Type Declarations
 * ====================
 */

typedef struct {
    float x, y, z;
} AttrVertex;

/* The polygons whose footprint touches this cell. */
typedef struct {
    int count_0x00;
    int *polyIds_0x04;
} AttrCell;

/* No plane normal: only used for flat (x, z) containment tests. */
typedef struct {
    int vertexCount_0x00;
    int *vertexIds_0x04;
    int attr_0x08;
    int field_0x0c;
    int shapeFlag_0x10;     /* 0 = convex (cross-product test), else concave (angle-sum test) */
    int field_0x14;
} AttrPoly;

typedef struct {
    int cellsX_0x00;
    int cellsZ_0x04;
    int field_0x08;
    int field_0x0c;
    AttrCell *cells_0x10;
    AttrPoly *polys_0x14;
    AttrVertex *verts_0x18;
} AttrGrid;

/* count == 0 is a miss. */
typedef struct {
    int *slot;
    int *vertexIds;
    int count;
} AttrQueryResult;

#define CELL_SIZE 150.0f

/* Max gap between a candidate's first-vertex y and the query y. */
#define HEIGHT_TOLERANCE 20.0f

/* =======================
 * Non-initialized Globals
 * =======================
 */

void *var_activeAttrGrid_8c228b3c;


/* ====================
 * Functions
 * ====================
 */

void *AttrQueryFindConvexPolygon_8c02e51c(float x, float y, float z, void *outParam)
{
    AttrQueryResult *out = (AttrQueryResult *)outParam;
    AttrGrid *grid = (AttrGrid *)var_activeAttrGrid_8c228b3c;
    AttrCell *cell;
    int cx, cz;
    int i;

    if (out->count != 0) {
        int *ids = out->vertexIds;
        int *slot = out->slot;
        AttrVertex *v;
        float pdx, pdz;
        int n = out->count;
        int hit;
        int j;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

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

        if (hit) {
            out->slot = slot;
            out->vertexIds = grid->polys_0x14[*slot].vertexIds_0x04;
            out->count = n;
            return &grid->polys_0x14[*slot].attr_0x08;
        }
    }

    cx = (int)(x / CELL_SIZE);
    if (cx >= grid->cellsX_0x00) {
        cx = grid->cellsX_0x00 - 1;
    }

    cz = (int)(z / CELL_SIZE);
    if (cz >= grid->cellsZ_0x04) {
        cz = grid->cellsZ_0x04 - 1;
    }

    cell = &grid->cells_0x10[cz * grid->cellsX_0x00 + cx];

    for (i = 0; i < cell->count_0x00; i++) {
        int *slot = &cell->polyIds_0x04[i];
        int *ids = grid->polys_0x14[*slot].vertexIds_0x04;
        int n;
        AttrVertex *v;
        float pdx, pdz;
        int hit;
        int j;

        if (ids == out->vertexIds) {
            continue;
        }

        n = grid->polys_0x14[*slot].vertexCount_0x00;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

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

        if (hit) {
            out->slot = slot;
            out->vertexIds = ids;
            out->count = n;
            return &grid->polys_0x14[*slot].attr_0x08;
        }
    }

    out->vertexIds = 0;
    out->count = 0;
    return NULL;
}

/* The re-test of the previous hit skips the height check. */
void *AttrQueryFindConvexPolygonAtHeight_8c02eab4(float x, float y, float z, void *outParam)
{
    AttrQueryResult *out = (AttrQueryResult *)outParam;
    AttrGrid *grid = (AttrGrid *)var_activeAttrGrid_8c228b3c;
    AttrCell *cell;
    int cx, cz;
    int i;

    if (out->count != 0) {
        int *ids = out->vertexIds;
        int *slot = out->slot;
        AttrVertex *v;
        float pdx, pdz;
        int n = out->count;
        int hit;
        int j;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

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

        if (hit) {
            out->slot = slot;
            out->vertexIds = grid->polys_0x14[*slot].vertexIds_0x04;
            out->count = n;
            return &grid->polys_0x14[*slot].attr_0x08;
        }
    }

    cx = (int)(x / CELL_SIZE);
    if (cx >= grid->cellsX_0x00) {
        cx = grid->cellsX_0x00 - 1;
    }

    cz = (int)(z / CELL_SIZE);
    if (cz >= grid->cellsZ_0x04) {
        cz = grid->cellsZ_0x04 - 1;
    }

    cell = &grid->cells_0x10[cz * grid->cellsX_0x00 + cx];

    for (i = 0; i < cell->count_0x00; i++) {
        int *slot = &cell->polyIds_0x04[i];
        int *ids = grid->polys_0x14[*slot].vertexIds_0x04;
        int n;
        AttrVertex *v;
        float pdx, pdz;
        int hit;
        int j;

        if (fabsf(grid->verts_0x18[ids[0]].y - y) > HEIGHT_TOLERANCE ||
            ids == out->vertexIds) {
            continue;
        }

        n = grid->polys_0x14[*slot].vertexCount_0x00;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

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

        if (hit) {
            out->slot = slot;
            out->vertexIds = ids;
            out->count = n;
            return &grid->polys_0x14[*slot].attr_0x08;
        }
    }

    out->vertexIds = 0;
    out->count = 0;
    return NULL;
}

void *AttrQueryFindPolygon_8c02e69c(float x, float y, float z, void *outParam)
{
    AttrQueryResult *out = (AttrQueryResult *)outParam;
    AttrGrid *grid = (AttrGrid *)var_activeAttrGrid_8c228b3c;
    AttrCell *cell;
    int cx, cz;
    int i;

    if (out->count != 0) {
        int *ids = out->vertexIds;
        int *slot = out->slot;
        AttrPoly *poly = &grid->polys_0x14[*slot];
        AttrVertex *v;
        float pdx, pdz;
        int n = out->count;
        int hit;
        int j;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        if (poly->shapeFlag_0x10 == 0) {
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

        /* Unlike the convex-only variants, *out is left as is. */
        if (hit) {
            return &poly->attr_0x08;
        }
    }

    cx = (int)(x / CELL_SIZE);
    if (cx >= grid->cellsX_0x00) {
        cx = grid->cellsX_0x00 - 1;
    }

    cz = (int)(z / CELL_SIZE);
    if (cz >= grid->cellsZ_0x04) {
        cz = grid->cellsZ_0x04 - 1;
    }

    cell = &grid->cells_0x10[cz * grid->cellsX_0x00 + cx];

    for (i = 0; i < cell->count_0x00; i++) {
        int *slot = &cell->polyIds_0x04[i];
        AttrPoly *poly = &grid->polys_0x14[*slot];
        int *ids = poly->vertexIds_0x04;
        int n;
        AttrVertex *v;
        float pdx, pdz;
        int hit;
        int j;

        if (ids == out->vertexIds) {
            continue;
        }

        n = poly->vertexCount_0x00;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        if (poly->shapeFlag_0x10 == 0) {
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
            out->slot = slot;
            out->vertexIds = poly->vertexIds_0x04;
            out->count = n;
            return &poly->attr_0x08;
        }
    }

    out->vertexIds = 0;
    out->count = 0;
    return NULL;
}

/* The re-test of the previous hit skips the height check. */
void *AttrQueryFindPolygonAtHeight_8c02ec50(float x, float y, float z, void *outParam)
{
    AttrQueryResult *out = (AttrQueryResult *)outParam;
    AttrGrid *grid = (AttrGrid *)var_activeAttrGrid_8c228b3c;
    AttrCell *cell;
    int cx, cz;
    int i;

    if (out->count != 0) {
        int *ids = out->vertexIds;
        int *slot = out->slot;
        AttrPoly *poly = &grid->polys_0x14[*slot];
        AttrVertex *v;
        float pdx, pdz;
        int n = out->count;
        int hit;
        int j;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        if (poly->shapeFlag_0x10 == 0) {
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

        /* Unlike the convex-only variants, *out is left as is. */
        if (hit) {
            return &poly->attr_0x08;
        }
    }

    cx = (int)(x / CELL_SIZE);
    if (cx >= grid->cellsX_0x00) {
        cx = grid->cellsX_0x00 - 1;
    }

    cz = (int)(z / CELL_SIZE);
    if (cz >= grid->cellsZ_0x04) {
        cz = grid->cellsZ_0x04 - 1;
    }

    cell = &grid->cells_0x10[cz * grid->cellsX_0x00 + cx];

    for (i = 0; i < cell->count_0x00; i++) {
        int *slot = &cell->polyIds_0x04[i];
        AttrPoly *poly = &grid->polys_0x14[*slot];
        int *ids = poly->vertexIds_0x04;
        int n;
        AttrVertex *v;
        float pdx, pdz;
        int hit;
        int j;

        if (fabsf(grid->verts_0x18[ids[0]].y - y) > HEIGHT_TOLERANCE ||
            ids == out->vertexIds) {
            continue;
        }

        n = poly->vertexCount_0x00;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        if (poly->shapeFlag_0x10 == 0) {
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
            out->slot = slot;
            out->vertexIds = poly->vertexIds_0x04;
            out->count = n;
            return &poly->attr_0x08;
        }
    }

    out->vertexIds = 0;
    out->count = 0;
    return NULL;
}

int AttrQueryRegionOccupied_8c02f08a(Task *self, int id)
{
    Task *t;

    for (t = var_tasks_8c1bac28; t->action != NULL; t++) {
        if ((void *)t->action != (void *)-1 && t != self &&
            ((TrafficEntry *)t->state)->signalId_0x410 == id) {
            return 1;
        }
    }

    return var_busState_8c1bb9d0.fallbackTaskMatchId_0x3a0 == id;
}
