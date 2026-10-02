/* @unit TileDraw */
#include <shinobi.h>

#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "1ba1c8_globals.h"
#include "011120_asset_queues.h" /* LoadedModel */
#include "013ae8_route.h"   /* CourseSceneParams */
#include "014a9c_tasks.h"        /* Task */
#include "021b9c_tile_draw.h"
#include "02171c_tile_stream.h"  /* TileIndex, TileStreamDrawTile_8c021b34 */
#include "022464_render.h"         /* RenderQueueDraw, RenderQueueDraw2 */

/* ====================
 * Non-initialized Globals
 * ====================
 */

float var_simpleLightDir_8c2264d8[3];
float var_mirrorSimpleLightDir_8c2264e4[3];
float var_simpleLightIntensity_8c2264f0[2];
float var_simpleLightColor_8c2264f8[3];
int var_tileDrawSpan_8c226504;
int var_tileDrawRadius_8c226508;
TileIndex *var_tileLayerIndexes_8c22650c[5];
LoadedModel *var_tileLayerSlots_8c226520[5];
TileRect *var_currentTileRegionList_8c226534;
STATIC float var_easyLightDir_8c226538[3];
STATIC float var_easyLightIntensity_8c226544[2];
STATIC float var_easyLightColor_8c22654c[3];

/* ====================
 * Forward Declarations
 * ====================
 */
STATIC void drawTileGrid_8c021b9c(int width, int height);
STATIC void drawTileGridMirror_8c021ec4(int width, int height);

/* ====================
 * Functions
 * ====================
 */

/* Layers 0, 2 and 3 use the Easy light model, layer 1 Simple. */
STATIC void drawTileGrid_8c021b9c(int width, int height)
{
    int colStart, colEnd, rowStart, rowEnd;
    int row, col, flatIndex, rowBase;
    LoadedModel *slot;

    colStart = (int)var_busState_8c1bb9d0.posX_0x2fc / 150 - var_tileDrawRadius_8c226508;
    colEnd = colStart + var_tileDrawSpan_8c226504;
    if (colStart < 0) {
        colStart = 0;
    }
    if (colEnd >= width) {
        colEnd = width - 1;
    }

    rowStart = (int)var_busState_8c1bb9d0.posZ_0x304 / 150 - var_tileDrawRadius_8c226508;
    rowEnd = rowStart + var_tileDrawSpan_8c226504;
    if (rowStart < 0) {
        rowStart = 0;
    }
    if (rowEnd >= height) {
        rowEnd = height - 1;
    }

    njControl3D(NJD_CONTROL_3D_MODEL_CLIP | NJD_CONTROL_3D_SHADOW | NJD_CONTROL_3D_TRANS_MODIFIER);

    rowBase = rowStart * width;
    for (row = rowStart; row <= rowEnd; row++) {
        for (col = colStart; col <= colEnd; col++) {
            flatIndex = rowBase + col;

            slot = &var_tileLayerSlots_8c226520[0][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_drawCamera_8c226558);
                njCnkSetEasyLight(var_easyLightDir_8c226538[0], var_easyLightDir_8c226538[1], var_easyLightDir_8c226538[2]);
                njCnkSetEasyLightIntensity(var_easyLightIntensity_8c226544[0], var_easyLightIntensity_8c226544[1]);
                njCnkSetEasyLightColor(var_easyLightColor_8c22654c[0], var_easyLightColor_8c22654c[1], var_easyLightColor_8c22654c[2]);
                njSetTexture(slot->texlist);
                njCnkEasyDrawObject(slot->njDest);
            }

            slot = &var_tileLayerSlots_8c226520[1][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_drawCamera_8c226558);
                njCnkSetSimpleLight(var_simpleLightDir_8c2264d8[0], var_simpleLightDir_8c2264d8[1], var_simpleLightDir_8c2264d8[2]);
                njCnkSetSimpleLightIntensity(var_simpleLightIntensity_8c2264f0[0], var_simpleLightIntensity_8c2264f0[1]);
                njCnkSetSimpleLightColor(var_simpleLightColor_8c2264f8[0], var_simpleLightColor_8c2264f8[1], var_simpleLightColor_8c2264f8[2]);
                njSetTexture(slot->texlist);
                njCnkSimpleDrawObject(slot->njDest);
            }

            slot = &var_tileLayerSlots_8c226520[2][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_drawCamera_8c226558);
                njCnkSetEasyLight(var_easyLightDir_8c226538[0], var_easyLightDir_8c226538[1], var_easyLightDir_8c226538[2]);
                njCnkSetEasyLightIntensity(var_easyLightIntensity_8c226544[0], var_easyLightIntensity_8c226544[1]);
                njCnkSetEasyLightColor(var_easyLightColor_8c22654c[0], var_easyLightColor_8c22654c[1], var_easyLightColor_8c22654c[2]);
                njSetTexture(slot->texlist);
                njCnkEasyDrawObject(slot->njDest);
            }

            slot = &var_tileLayerSlots_8c226520[3][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_drawCamera_8c226558);
                njCnkSetEasyLight(var_easyLightDir_8c226538[0], var_easyLightDir_8c226538[1], var_easyLightDir_8c226538[2]);
                njCnkSetEasyLightIntensity(var_easyLightIntensity_8c226544[0], var_easyLightIntensity_8c226544[1]);
                njCnkSetEasyLightColor(var_easyLightColor_8c22654c[0], var_easyLightColor_8c22654c[1], var_easyLightColor_8c22654c[2]);
                njSetTexture(slot->texlist);
                njCnkEasyDrawObject(slot->njDest);
            }
        }
        rowBase += width;
    }

    njControl3D(NJD_CONTROL_3D_MODEL_CLIP);
}

/* Differs from drawTileGrid_8c021b9c only in layer 1's light direction. */
STATIC void drawTileGridMirror_8c021ec4(int width, int height)
{
    int colStart, colEnd, rowStart, rowEnd;
    int row, col, flatIndex, rowBase;
    LoadedModel *slot;

    colStart = (int)var_busState_8c1bb9d0.posX_0x2fc / 150 - var_tileDrawRadius_8c226508;
    colEnd = colStart + var_tileDrawSpan_8c226504;
    if (colStart < 0) {
        colStart = 0;
    }
    if (colEnd >= width) {
        colEnd = width - 1;
    }

    rowStart = (int)var_busState_8c1bb9d0.posZ_0x304 / 150 - var_tileDrawRadius_8c226508;
    rowEnd = rowStart + var_tileDrawSpan_8c226504;
    if (rowStart < 0) {
        rowStart = 0;
    }
    if (rowEnd >= height) {
        rowEnd = height - 1;
    }

    njControl3D(NJD_CONTROL_3D_MODEL_CLIP | NJD_CONTROL_3D_SHADOW | NJD_CONTROL_3D_TRANS_MODIFIER);

    rowBase = rowStart * width;
    for (row = rowStart; row <= rowEnd; row++) {
        for (col = colStart; col <= colEnd; col++) {
            flatIndex = rowBase + col;

            slot = &var_tileLayerSlots_8c226520[0][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_drawCamera_8c226558);
                njCnkSetEasyLight(var_easyLightDir_8c226538[0], var_easyLightDir_8c226538[1], var_easyLightDir_8c226538[2]);
                njCnkSetEasyLightIntensity(var_easyLightIntensity_8c226544[0], var_easyLightIntensity_8c226544[1]);
                njCnkSetEasyLightColor(var_easyLightColor_8c22654c[0], var_easyLightColor_8c22654c[1], var_easyLightColor_8c22654c[2]);
                njSetTexture(slot->texlist);
                njCnkEasyDrawObject(slot->njDest);
            }

            slot = &var_tileLayerSlots_8c226520[1][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_drawCamera_8c226558);
                njCnkSetSimpleLight(var_mirrorSimpleLightDir_8c2264e4[0], var_mirrorSimpleLightDir_8c2264e4[1], var_mirrorSimpleLightDir_8c2264e4[2]);
                njCnkSetSimpleLightIntensity(var_simpleLightIntensity_8c2264f0[0], var_simpleLightIntensity_8c2264f0[1]);
                njCnkSetSimpleLightColor(var_simpleLightColor_8c2264f8[0], var_simpleLightColor_8c2264f8[1], var_simpleLightColor_8c2264f8[2]);
                njSetTexture(slot->texlist);
                njCnkSimpleDrawObject(slot->njDest);
            }

            slot = &var_tileLayerSlots_8c226520[2][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_drawCamera_8c226558);
                njCnkSetEasyLight(var_easyLightDir_8c226538[0], var_easyLightDir_8c226538[1], var_easyLightDir_8c226538[2]);
                njCnkSetEasyLightIntensity(var_easyLightIntensity_8c226544[0], var_easyLightIntensity_8c226544[1]);
                njCnkSetEasyLightColor(var_easyLightColor_8c22654c[0], var_easyLightColor_8c22654c[1], var_easyLightColor_8c22654c[2]);
                njSetTexture(slot->texlist);
                njCnkEasyDrawObject(slot->njDest);
            }

            slot = &var_tileLayerSlots_8c226520[3][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_drawCamera_8c226558);
                njCnkSetEasyLight(var_easyLightDir_8c226538[0], var_easyLightDir_8c226538[1], var_easyLightDir_8c226538[2]);
                njCnkSetEasyLightIntensity(var_easyLightIntensity_8c226544[0], var_easyLightIntensity_8c226544[1]);
                njCnkSetEasyLightColor(var_easyLightColor_8c22654c[0], var_easyLightColor_8c22654c[1], var_easyLightColor_8c22654c[2]);
                njSetTexture(slot->texlist);
                njCnkEasyDrawObject(slot->njDest);
            }
        }
        rowBase += width;
    }

    njControl3D(NJD_CONTROL_3D_MODEL_CLIP);
}

/* Each light direction broadcasts one scene-param component to all three
 * axes before the camera transform. Original behaviour. */
STATIC void enqueueTask_8c0221d0(Task *task, void *state)
{
    njSetCamera(&var_camera_8c1bb904);

    var_easyLightDir_8c226538[0] = var_sceneParams_8c18ad24->dir1_0x48[0];
    var_easyLightDir_8c226538[1] = var_sceneParams_8c18ad24->dir1_0x48[0];
    var_easyLightDir_8c226538[2] = var_sceneParams_8c18ad24->dir1_0x48[0];
    njCalcVector(NULL, (NJS_VECTOR *)var_easyLightDir_8c226538, (NJS_VECTOR *)var_easyLightDir_8c226538);

    var_simpleLightDir_8c2264d8[0] = var_sceneParams_8c18ad24->dir2_0x68[0];
    var_simpleLightDir_8c2264d8[1] = var_sceneParams_8c18ad24->dir2_0x68[0];
    var_simpleLightDir_8c2264d8[2] = var_sceneParams_8c18ad24->dir2_0x68[0];
    njCalcVector(NULL, (NJS_VECTOR *)var_simpleLightDir_8c2264d8, (NJS_VECTOR *)var_simpleLightDir_8c2264d8);

    njSetCamera(&var_mirrorCamera_8c1bb944);

    var_mirrorSimpleLightDir_8c2264e4[0] = var_sceneParams_8c18ad24->dir2_0x68[0];
    var_mirrorSimpleLightDir_8c2264e4[1] = var_sceneParams_8c18ad24->dir2_0x68[0];
    var_mirrorSimpleLightDir_8c2264e4[2] = var_sceneParams_8c18ad24->dir2_0x68[0];
    njCalcVector(NULL, (NJS_VECTOR *)var_mirrorSimpleLightDir_8c2264e4, (NJS_VECTOR *)var_mirrorSimpleLightDir_8c2264e4);

    RenderQueueDraw2_8c022420(RENDER_LAYER_MAIN, drawTileGrid_8c021b9c, var_tileLayerIndexes_8c22650c[0]->width, var_tileLayerIndexes_8c22650c[0]->height);
    RenderQueueDraw2_8c022420(RENDER_LAYER_MIRROR, drawTileGridMirror_8c021ec4, var_tileLayerIndexes_8c22650c[0]->width, var_tileLayerIndexes_8c22650c[0]->height);
    RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, (DrawFn)TileStreamDrawTile_8c021b34, (int)state);
    RenderQueueDraw_8c0223ea(RENDER_LAYER_MIRROR, (DrawFn)TileStreamDrawTile_8c021b34, (int)state);
}

void TileDrawSpawnTask_8c0222dc(void)
{
    Task *task;
    LoadedModel *state;

    TaskSpawn_8c014ae8(var_tasks_8c1ba5e8, enqueueTask_8c0221d0, &task, (void **)&state, 8);
    state->texlist = var_segmentModels_8c1bc3f0->texlist;
    state->njDest = var_segmentModels_8c1bc3f0->njDest;

    var_easyLightIntensity_8c226544[0] = var_sceneParams_8c18ad24->rec1_0x54[0];
    var_easyLightIntensity_8c226544[1] = var_sceneParams_8c18ad24->rec1_0x54[1];
    var_easyLightColor_8c22654c[0] = var_sceneParams_8c18ad24->rec1_0x54[2];
    var_easyLightColor_8c22654c[1] = var_sceneParams_8c18ad24->rec1_0x54[3];
    var_easyLightColor_8c22654c[2] = var_sceneParams_8c18ad24->rec1_0x54[4];

    var_simpleLightIntensity_8c2264f0[0] = var_sceneParams_8c18ad24->rec2_0x74[0];
    var_simpleLightIntensity_8c2264f0[1] = var_sceneParams_8c18ad24->rec2_0x74[1];
    var_simpleLightColor_8c2264f8[0] = var_sceneParams_8c18ad24->rec2_0x74[2];
    var_simpleLightColor_8c2264f8[1] = var_sceneParams_8c18ad24->rec2_0x74[3];
    var_simpleLightColor_8c2264f8[2] = var_sceneParams_8c18ad24->rec2_0x74[4];
}
