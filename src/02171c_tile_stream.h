/* 8c02171c */
#ifndef _02171C_TILE_STREAM_H
#define _02171C_TILE_STREAM_H

#include <shinobi.h>
#include "011120_asset_queues.h" /* LoadedModel */

/* =================
 * Type Declarations
 * =================
 */

/* Tile layer index file: tileOffsets are from the file base, 0 = no data.
 * The copies in CurrentCourse.tileLayers_0x3c are only read for their dims. */
typedef struct {
    int width;
    int height;
    int tileOffsets[1]; /* width * height */
} TileIndex;

/* Inclusive, in tiles. A list ends at a rect with endCol and endRow both 0. */
typedef struct {
    Sint8 startCol;
    Sint8 startRow;
    Sint8 endCol;
    Sint8 endRow;
} TileRect;

/* =========
 * Functions
 * =========
 */

void TileStreamClearSlots_8c02171c(void);
void TileStreamTeardown_8c021724(void);
void TileStreamInit_8c02175a(void);
void TileStreamLoad_8c021810(void);
void TileStreamRequestUpload_8c02190a(void);
void TileStreamReleaseAll_8c021a24(void);
void TileStreamDrawTile_8c021b34(LoadedModel *slot);

#endif // _02171C_TILE_STREAM_H
