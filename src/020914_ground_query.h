#ifndef _020914_GROUND_QUERY_H
#define _020914_GROUND_QUERY_H

typedef struct {
    float x, y, z;
} GroundVertex;

/* One 150-unit grid cell: the polygons whose footprint touches it. */
typedef struct {
    int count_0x00;
    int *polyIds_0x04;
} GroundCell;

typedef struct {
    int vertexCount_0x00;
    int *vertexIds_0x04;
    float normalX_0x08;     /* precomputed plane normal */
    float normalY_0x0c;
    float normalZ_0x10;
    int attr_0x14;  /* sign bit selects the concave containment test */
} GroundPoly;

typedef struct {
    int cellsX_0x00;
    int cellsZ_0x04;
    float cellSizeX_0x08;
    float cellSizeZ_0x0c;
    GroundCell *cells_0x10;
    GroundPoly *polys_0x14;
    GroundVertex *verts_0x18;
} GroundGrid;

extern GroundGrid* var_activeGroundGrid_8c2264d4;

/* A miss zeroes count_0x0c and vertexIds_0x08 only; the rest goes stale. */
typedef struct {
    int attr_0x00;          /* polygon attribute word, sign bit stripped */
    int *polyIdSlot_0x04;   /* entry in the cell's polygon list */
    int *vertexIds_0x08;    /* indices into verts_0x18 */
    int count_0x0c;         /* vertex count; 0 = no polygon under the point */
} GroundQueryResult;

/* Searches var_activeGroundGrid_8c2264d4; y is unused. */
void GroundQueryFindPolygon_8c020914(float x, float y, float z, GroundQueryResult *out);

#endif // _020914_GROUND_QUERY_H
