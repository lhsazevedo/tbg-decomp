/* 8c021b9c */
#ifndef _021B9C_TILE_DRAW_H
#define _021B9C_TILE_DRAW_H

#include "011120_asset_queues.h" /* LoadedModel */
#include "02171c_tile_stream.h"  /* TileIndex, TileRect */

/* =======================
 * Non-initialized Globals
 * =======================
 */

extern float var_simpleLightDir_8c2264d8[3];
extern float var_mirrorSimpleLightDir_8c2264e4[3];
extern float var_simpleLightIntensity_8c2264f0[2];
extern float var_simpleLightColor_8c2264f8[3];
extern int var_tileDrawSpan_8c226504;
extern int var_tileDrawRadius_8c226508;
extern TileIndex *var_tileLayerIndexes_8c22650c[5];
/* Layers 0-3 hold texture+model pairs, layer 4 models only. [0] is -1 while
 * unallocated. */
extern LoadedModel *var_tileLayerSlots_8c226520[5];
extern TileRect *var_currentTileRegionList_8c226534; /* -1 when unset */

/* =========
 * Functions
 * =========
 */

void TileDrawSpawnTask_8c0222dc(void);

#endif // _021B9C_TILE_DRAW_H
