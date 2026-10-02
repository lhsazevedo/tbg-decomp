/* @unit Route */
#include <shinobi.h>
#include "011120_asset_queues.h"
#include "012324_input.h"
#include "0129cc_game.h"
#include "013ae8_route.h"
#include "014a9c_tasks.h"
#include "014f54_sprite.h"
#include "015034_text.h"
#include "02171c_tile_stream.h"
#include "021b9c_tile_draw.h"
#include "026710_traffic.h"
#include "0289ac_objects.h"
#include "02a9fc_message_box.h"
#include "02af78_event.h"
#include "02c884_stop.h"
#include "02b464_grading.h"
#include "04ce10_line_nodes.h"
#include "024b4c_bus_camera.h"
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#ifdef SERIAL_DEBUG
char *DEBUG_routeLoadStateNames[] = {
    "INIT",
    "POST_LOAD",
    "WAIT",
    "IDLE",
    "DONE",
};

char *DEBUG_segmentReloadStateNames[] = {
    "POST_LOAD",
    "WAIT",
    "IDLE",
    "DONE",
};
#endif

#define CHANGE_LOAD_STATE(task, x) \
    do { \
        (task)->stage_0x08 = x; \
        LOG_DEBUG(("[ROUTE] State changed: %s\n", DEBUG_routeLoadStateNames[x])); \
    } while (0)

#define CHANGE_SEGMENT_RELOAD_STATE(task, x) \
    do { \
        (task)->stage_0x08 = x; \
        LOG_DEBUG(("[ROUTE] State changed: %s\n", DEBUG_segmentReloadStateNames[x])); \
    } while (0)

/* ====================
 * Type Declarations
 * ====================
 */

typedef enum {
    SEGMENT_RELOAD_STATE_POST_LOAD = 0,
    SEGMENT_RELOAD_STATE_WAIT      = 1,
    SEGMENT_RELOAD_STATE_IDLE      = 2,
    SEGMENT_RELOAD_STATE_DONE      = 3,
} SegmentReloadState;

/* RouteLoadTask with its own state enum. */
typedef struct {
    TaskAction action;
    void *state;
    SegmentReloadState stage_0x08;
    int frame_0x0c;
    int field_0x10;
    int field_0x14;
    void *queuedItem_0x18;
    int field_0x1c;
} SegmentReloadTask;

/* Placeholder types for the nested table/record
   tree hung off the CourseSegment pointer slots. */
typedef struct { int count; void *ptr; } IntPtr;           /* {count, rec*} lists, {-1,NULL}-terminated */
typedef struct { char *a, *b; int n0, n1; } Rec_ssii;
typedef struct { char *a, *b; int n; float f0, f1; } Rec_ssiff;
typedef struct { char *a, *b; int n; char *c; } Rec_ssis;
typedef struct { char *a, *b; int n; char *c; Uint8 flags[4]; } Rec_ssisb;

/* ==========================
 * Non-initialized Globals
 * ==========================
 */

char var_commonDir_8c18ad6c[0x20];
char var_pvrDir_8c18ad4c[0x20];
char var_commonDirCopy_8c18ad8c[0x20];

CourseConfig *var_currentCourseConfig_8c18ad18;

enum ROUTE var_route_8c18ad1c;

ViewParams *var_viewParams_8c18ad28;

char var_datDir_8c18ad2c[0x20];

enum TIME_OF_DAY var_timeOfDay_8c18ad20;

CourseSceneParams *var_sceneParams_8c18ad24;

int var_loadLatch_8c18adac;

Sint8 *var_routeModelIndexes_8c18adb0;

void *var_datFiles_8c18adb4[4];

/* ======================
 * Initialized Globals
 * ======================
 */

#include "013ae8_route.data.inc"

/* ======================
 * Forward Declarations
 * ======================
 */

void RouteSetLatch_8c014330(void);
void RouteClearLatch_8c014322(void);

/* ==========
 * Functions
 * ==========
 */

STATIC void requestVehicleAssets_8c013ae8(void)
{
    LOG_DEBUG(("[ROUTE] requesting vehicle assets\n"));

    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, "front.njd", &var_frontNj_8c1bc434, 0);
    AsqRequestPvm_8c011ac0(var_commonDir_8c18ad6c, "front.pvm", &var_frontTexlist_8c1bc430, 0xf, 0);

    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, "syanai.njd", &var_interiorNj_8c1bc43c, 0);
    AsqRequestPvm_8c011ac0(var_commonDir_8c18ad6c, "syanai.pvm", &var_interiorTexlist_8c1bc438, 0x40, 0x80000000);

    AsqRequestPvm_8c011ac0(var_commonDir_8c18ad6c, "mark.pvm", &var_markTexlist_8c1bc418, 3, 0);
    AsqRequestPvm_8c011ac0(var_commonDir_8c18ad6c, "busstop.pvm", &var_busStopTexlist_8c1bc424, 1, 0);

    var_trafficModels_8c1bc3f4 = AsqRequestModels_8c012030(var_commonDir_8c18ad6c, init_trafficModelFiles_8c043d64, 0);
}

void RouteFreeVehicleAssets_8c013b5a(void)
{
    Uint32 i;

    if (var_interiorTexlist_8c1bc438 != (NJS_TEXLIST *) -1) {
        LOG_DEBUG(("[ROUTE] freeing vehicle assets\n"));

        njSetTexture(var_interiorTexlist_8c1bc438);
        for (i = 0; i < var_interiorTexlist_8c1bc438->nbTexture; i++) {
            njReleaseCacheTextureNum(i);
        }
        AsqReleaseAndFreeTexlist_8c011e3c(var_interiorTexlist_8c1bc438);
        syFree(var_interiorNj_8c1bc43c);

        AsqReleaseAndFreeTexlist_8c011e3c(var_frontTexlist_8c1bc430);
        syFree(var_frontNj_8c1bc434);

        AsqReleaseAndFreeTexlist_8c011e3c(var_markTexlist_8c1bc418);
        AsqReleaseAndFreeTexlist_8c011e3c(var_busStopTexlist_8c1bc424);

        var_interiorTexlist_8c1bc438 = (NJS_TEXLIST *) -1;
    }
}

void RouteClearModelSlots_8c013bbc(ModelSlot *slots, int count)
{
    ModelSlot *slot = &slots[--count];

    for (; slot >= slots; slot--) {
        slot->texlist_0x08 = (NJS_TEXLIST *) -1;
    }
}

/* models: -1-terminated slot indices. */
STATIC void syncRouteModelAssets_8c013c34(Sint8 *models)
{
    ModelSlot *slot;
    char **names;
    int i;

    LOG_DEBUG(("[ROUTE] reconciling route-model assets (models=%p)\n", models));

    for (slot = var_routeModelSlots_8c1bbddc; slot < &var_routeModelSlots_8c1bbddc[0x20]; slot++) {
        slot->requested_0x00 = 0;
        slot->needsLoad_0x04 = 0;
    }

    while ((i = *models) != -1) {
        var_routeModelSlots_8c1bbddc[i].requested_0x00 = 1;
        models++;
        if (var_routeModelSlots_8c1bbddc[i].texlist_0x08 == (NJS_TEXLIST *) -1) {
            var_routeModelSlots_8c1bbddc[i].needsLoad_0x04 = 1;
        }
    }

    for (i = 0; i < 0x20; i++) {
        if (var_routeModelSlots_8c1bbddc[i].needsLoad_0x04 != 0) {
            if (var_timeOfDay_8c18ad20 == TIME_OF_DAY_NIGHT) {
                AsqRequestNj_8c011492(var_commonDir_8c18ad6c, init_routeModelFilesNight_8c043ecc[i * 2], 0, &var_routeModelSlots_8c1bbddc[i].nj_0x0c);
                names = init_routeModelFilesNight_8c043ecc;
            } else {
                AsqRequestNj_8c011492(var_commonDir_8c18ad6c, init_routeModelFilesDay_8c043dc4[i * 2], 0, &var_routeModelSlots_8c1bbddc[i].nj_0x0c);
                names = init_routeModelFilesDay_8c043dc4;
            }
            AsqRequestPvm_8c011ac0(var_commonDir_8c18ad6c, names[i * 2 + 1], &var_routeModelSlots_8c1bbddc[i].texlist_0x08, 0x60, 0);
        } else if (var_routeModelSlots_8c1bbddc[i].requested_0x00 == 0 && var_routeModelSlots_8c1bbddc[i].texlist_0x08 != (NJS_TEXLIST *) -1) {
            AsqReleaseAndFreeTexlist_8c011e3c(var_routeModelSlots_8c1bbddc[i].texlist_0x08);
            syFree(var_routeModelSlots_8c1bbddc[i].nj_0x0c);
            var_routeModelSlots_8c1bbddc[i].texlist_0x08 = (NJS_TEXLIST *) -1;
        }
    }
}

STATIC void finishAssetLoad_8c013d42(void)
{
    LOG_DEBUG(("[ROUTE] asset load finished\n"));

    RouteSetLatch_8c014330();
    AsqFreeQueues_8c011f7e();
}

void RouteStartModelLoadPass_8c013d78(void)
{
    LOG_DEBUG(("[ROUTE] starting route-model load pass\n"));

    AsqInitQueues_8c011f36(0, 0x40, 0, 0x40);
    AsqResetQueues_8c011f6c();
    RouteClearLatch_8c014322();
    syncRouteModelAssets_8c013c34(var_routeModelIndexes_8c18adb0);
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, finishAssetLoad_8c013d42);
}

void RouteFreeAllModels_8c013dae(void)
{
    ModelSlot *slot;

    LOG_DEBUG(("[ROUTE] releasing all route-model assets\n"));

    for (slot = var_routeModelSlots_8c1bbddc; slot < &var_routeModelSlots_8c1bbddc[0x20]; slot++) {
        if (slot->texlist_0x08 != (NJS_TEXLIST *) -1) {
            AsqReleaseAndFreeTexlist_8c011e3c(slot->texlist_0x08);
            syFree(slot->nj_0x0c);
            slot->texlist_0x08 = (NJS_TEXLIST *) -1;
        }
    }
}

/* Textures only, no njd. */
STATIC void syncPedestrianAssets_8c013df6(Sint8 *models)
{
    ModelSlot *slot;
    int i;

    LOG_DEBUG(("[ROUTE] reconciling pedestrian textures (models=%p)\n", models));

    for (slot = var_pedestrianAssets_8c1bbfdc; slot < &var_pedestrianAssets_8c1bbfdc[0x41]; slot++) {
        slot->requested_0x00 = 0;
        slot->needsLoad_0x04 = 0;
    }

    while ((i = *models) != -1) {
        var_pedestrianAssets_8c1bbfdc[i].requested_0x00 = 1;
        models++;
        if (var_pedestrianAssets_8c1bbfdc[i].texlist_0x08 == (NJS_TEXLIST *) -1) {
            var_pedestrianAssets_8c1bbfdc[i].needsLoad_0x04 = 1;
        }
    }

    for (i = 0; i < 0x41; i++) {
        if (var_pedestrianAssets_8c1bbfdc[i].needsLoad_0x04 != 0) {
            AsqRequestPvm_8c011ac0(var_commonDir_8c18ad6c, init_pedestrianPvmNames_8c043fd8[i], &var_pedestrianAssets_8c1bbfdc[i].texlist_0x08, 2, 0);
        } else if (var_pedestrianAssets_8c1bbfdc[i].requested_0x00 == 0 && var_pedestrianAssets_8c1bbfdc[i].texlist_0x08 != (NJS_TEXLIST *) -1) {
            AsqReleaseAndFreeTexlist_8c011e3c(var_pedestrianAssets_8c1bbfdc[i].texlist_0x08);
            var_pedestrianAssets_8c1bbfdc[i].texlist_0x08 = (NJS_TEXLIST *) -1;
        }
    }
}

void RouteFreePedestrianAssets_8c013ee4(void)
{
    ModelSlot *slot;

    LOG_DEBUG(("[ROUTE] releasing pedestrian textures\n"));

    for (slot = var_pedestrianAssets_8c1bbfdc; slot < &var_pedestrianAssets_8c1bbfdc[0x41]; slot++) {
        if (slot->texlist_0x08 != (NJS_TEXLIST *) -1) {
            AsqReleaseAndFreeTexlist_8c011e3c(slot->texlist_0x08);
            slot->texlist_0x08 = (NJS_TEXLIST *) -1;
        }
    }
}

STATIC void freeSegmentAssets_8c013f22(void)
{
    CourseSegment *entry;

    LOG_DEBUG(("[ROUTE] freeing pairs for segment (index=%d)\n", var_currentSegment_8c228708));

    entry = &var_currentCourseConfig_8c18ad18->segments_0x08[var_currentSegment_8c228708];
    if (entry->modelFiles_0x28 != 0) {
        AsqFreeModels_8c0120fe(&var_segmentModels_8c1bc3f0);
    }
    if (entry->tileRegionList_0x0c != 0) {
        TileStreamReleaseAll_8c021a24();
    }
}

STATIC void syncSegmentAssets_8c013f78(void)
{
    CourseSegment *entry;
    int i;

    LOG_DEBUG(("[ROUTE] syncing assets for segment (index=%d)\n", var_currentSegment_8c228708));

    entry = &var_currentCourseConfig_8c18ad18->segments_0x08[var_currentSegment_8c228708];

    if (entry->viewParams_0x24 != 0) {
        var_viewParams_8c18ad28 = entry->viewParams_0x24;
        var_tileDrawSpan_8c226504 = var_viewParams_8c18ad28->tileWindowSpan_0x00 - 1;
        var_tileDrawRadius_8c226508 = var_tileDrawSpan_8c226504 / 2;
        var_farClipDepth_8c227dd0 = var_viewParams_8c18ad28->farClipDepth_0x04;
    }

    if (entry->modelFiles_0x28 != 0) {
        var_segmentModels_8c1bc3f0 = AsqRequestModels_8c012030(var_commonDir_8c18ad6c, entry->modelFiles_0x28, 0x10);
    }

    /* A cutscene's own model set replaces the segment's traffic models.
     * Cutscenes only run in PLAY_MODE_NORMAL. */
    if (var_cutsceneActive_8c1bb900 == 0 || var_playMode_8c1bb8d0 != PLAY_MODE_NORMAL) {
        if (entry->routeModelIndexes_0x10 != 0) {
            var_routeModelIndexes_8c18adb0 = entry->routeModelIndexes_0x10;
            syncRouteModelAssets_8c013c34(entry->routeModelIndexes_0x10);
        }
    } else {
        if (entry->routeModelIndexes_0x10 != 0) {
            var_routeModelIndexes_8c18adb0 = entry->routeModelIndexes_0x10;
        }
        syncRouteModelAssets_8c013c34(init_cutsceneRouteModels_8c043fd4);
    }

    if (entry->pedestrianModelList_0x18 != 0) {
        syncPedestrianAssets_8c013df6(entry->pedestrianModelList_0x18);
    }

    ObjectsStartAssetRequests_8c029ad4(entry->sceneObjectList_0x1c);

    if (entry->tileRegionList_0x0c == 0) {
        var_currentTileRegionList_8c226534 = (void *)-1;
    } else {
        var_currentTileRegionList_8c226534 = entry->tileRegionList_0x0c;
        for (i = 0; i < 4; i++) {
            AsqRequestDat_8c011182(var_datDir_8c18ad2c, entry->datFilenames_0x20[i], &var_datFiles_8c18adb4[i]);
        }
    }

    if (var_cutsceneActive_8c1bb900 != 0 && var_playMode_8c1bb8d0 == PLAY_MODE_NORMAL) {
        MessageBoxRequestAssets_8c02aa36();
    }
}

/* Also selects the course's config. */
STATIC void requestCourseAssets_8c014088(void)
{
    CourseConfig *config;
    CurrentCourse *course;
    char *dir;
    int i;

    LOG_DEBUG(("[ROUTE] requesting course %d models\n", var_currentCourse_8c1bb868.courseId_0x00));

    var_currentCourseConfig_8c18ad18 = init_courseTable_8c043ca4[var_currentCourse_8c1bb868.courseId_0x00];
    var_route_8c18ad1c = var_currentCourseConfig_8c18ad18->route_0x00;
    var_timeOfDay_8c18ad20 = var_currentCourseConfig_8c18ad18->timeOfDay_0x04;
    var_sceneParams_8c18ad24 = var_currentCourseConfig_8c18ad18->sceneParams_0x10;

    switch (var_route_8c18ad1c) {
        case ROUTE_SHINJUKU: {
            switch (var_timeOfDay_8c18ad20) {
                case TIME_OF_DAY_DAY:
                case TIME_OF_DAY_EVENING: {
                    strcpy(var_commonDir_8c18ad6c, "\\SD_COMMON");
                    strcpy(var_pvrDir_8c18ad4c, "\\SD_PVR");
                    break;
                }
                case TIME_OF_DAY_NIGHT: {
                    strcpy(var_commonDir_8c18ad6c, "\\SN_COMMON");
                    strcpy(var_pvrDir_8c18ad4c, "\\SN_PVR");
                    break;
                }
            }
            break;
        }
        case ROUTE_WANGAN: {
            switch (var_timeOfDay_8c18ad20) {
                case TIME_OF_DAY_DAY:
                case TIME_OF_DAY_EVENING: {
                    strcpy(var_commonDir_8c18ad6c, "\\WD_COMMON");
                    strcpy(var_pvrDir_8c18ad4c, "\\WD_PVR");
                    break;
                }
                case TIME_OF_DAY_NIGHT: {
                    strcpy(var_commonDir_8c18ad6c, "\\WN_COMMON");
                    strcpy(var_pvrDir_8c18ad4c, "\\WN_PVR");
                    break;
                }
            }
            break;
        }
        case ROUTE_OME: {
            switch (var_timeOfDay_8c18ad20) {
                case TIME_OF_DAY_DAY:
                case TIME_OF_DAY_EVENING: {
                    strcpy(var_commonDir_8c18ad6c, "\\OD_COMMON");
                    strcpy(var_pvrDir_8c18ad4c, "\\OD_PVR");
                    break;
                }
                case TIME_OF_DAY_NIGHT: {
                    strcpy(var_commonDir_8c18ad6c, "\\ON_COMMON");
                    strcpy(var_pvrDir_8c18ad4c, "\\ON_PVR");
                    break;
                }
            }
            break;
        }
    }

    strcpy(var_commonDirCopy_8c18ad8c, var_commonDir_8c18ad6c);
    strcpy(var_datDir_8c18ad2c, var_commonDir_8c18ad6c);

    config = var_currentCourseConfig_8c18ad18;
    course = &var_currentCourse_8c1bb868;
    dir = var_commonDir_8c18ad6c;

    AsqRequestNj_8c011492(dir, config->atariBusFile_0x1c, &course->atariBus_0x04, 0);
    AsqRequestNj_8c011492(dir, config->lineBusFile_0x20, &course->lineBus_0x08, 0);
    course->lineNodes_0x0c = config->lineNodes_0x24;
    AsqRequestNj_8c011492(dir, config->attrBusFile_0x28, &course->attrBus_0x10, 0);
    AsqRequestNj_8c011492(dir, config->attrMarkFile_0x2c, &course->attrMark_0x14, 0);
    AsqRequestNj_8c011492(dir, config->atariCpuFile_0x30, &course->atariCpu_0x18, 0);
    AsqRequestNj_8c011492(dir, config->lineCpuFile_0x34, &course->lineCpu_0x1c, 0);
    AsqRequestNj_8c011492(dir, config->attrCpuFile_0x38, &course->attrCpu_0x20, 0);
    AsqRequestDat_8c011182(dir, config->macCpu1File_0x3c, &course->macCpu1_0x24);
    AsqRequestNj_8c011492(dir, config->atariHumFile_0x40, &course->atariHum_0x28, 0);
    AsqRequestNj_8c011492(dir, config->lineHumFile_0x44, 0, &course->lineHum_0x2c);
    AsqRequestDat_8c011182(dir, config->macHumG0File_0x48, &course->macHumG0_0x30);
    AsqRequestDat_8c011182(dir, config->macHumM0File_0x4c, &course->macHumM0_0x34);
    AsqRequestDat_8c011182(dir, config->macSignalFile_0x50, &course->macSignal_0x38);
    for (i = 0; i < 5; i++) {
        AsqRequestNj_8c011492(dir, config->tileLayerFiles_0x54[i], &course->tileLayers_0x3c[i], 0);
    }

    requestVehicleAssets_8c013ae8();
    var_routeModels_8c1bc3ec = AsqRequestModels_8c012030(var_commonDir_8c18ad6c, init_signalModelFiles_8c0440dc, 0x10);
}

/* One bit with no fixed sense: a caller primes it one way, hands
 * AsqProcessQueues_8c011fe0 the other accessor as its completion callback, and
 * polls until it flips. The load tasks here wait for 1, most menus for 0. */
void RouteClearLatch_8c014322(void)
{
    LOG_DEBUG(("[ROUTE] load latch cleared\n"));

    var_loadLatch_8c18adac = 0;
}

int RouteGetLatch_8c01432a(void)
{
    return var_loadLatch_8c18adac;
}

void RouteSetLatch_8c014330(void)
{
    LOG_DEBUG(("[ROUTE] load latch set\n"));

    var_loadLatch_8c18adac = 1;
}

STATIC void routeLoadTask_8c014338(RouteLoadTask *task, void *state)
{
    int frame;

    switch (task->stage_0x08) {
        case ROUTE_LOAD_STATE_INIT: {
            AsqResetQueues_8c011f6c();
            njSetTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
            njLoadCacheTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
            requestCourseAssets_8c014088();
            RouteClearLatch_8c014322();
            AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteSetLatch_8c014330);
            CHANGE_LOAD_STATE(task, ROUTE_LOAD_STATE_POST_LOAD);
            return;
        }

        case ROUTE_LOAD_STATE_POST_LOAD: {
            if (RouteGetLatch_8c01432a() != 0) {
                TileStreamInit_8c02175a();
                TrafficRelocatePlacementTable_8c026da4(var_currentCourse_8c1bb868.macCpu1_0x24);
                ObjectsRelocatePedGroupDefs_8c028de8(var_currentCourse_8c1bb868.macHumG0_0x30);
                ObjectsRelocatePedGroupLists_8c028dd0(var_currentCourse_8c1bb868.macHumM0_0x34);
                StopSetup_8c02caba();
                // Arm this segment's cutscene first: syncSegmentAssets reads cutsceneActive.
                EventPickForSegment_8c02b170();
                AsqResetQueues_8c011f6c();
                syncSegmentAssets_8c013f78();
                RouteClearLatch_8c014322();
                AsqProcessQueues_8c011fe0(AsqNop_8c011120, TileStreamLoad_8c021810, TileStreamRequestUpload_8c02190a, 0, RouteSetLatch_8c014330);
                CHANGE_LOAD_STATE(task, ROUTE_LOAD_STATE_WAIT);
            }
            break;
        }

        case ROUTE_LOAD_STATE_WAIT: {
            if (RouteGetLatch_8c01432a() != 0) {
                CHANGE_LOAD_STATE(task, ROUTE_LOAD_STATE_IDLE);
                return;
            }
            break;
        }

        case ROUTE_LOAD_STATE_IDLE: {
            CHANGE_LOAD_STATE(task, ROUTE_LOAD_STATE_DONE);
            return;
        }

        case ROUTE_LOAD_STATE_DONE: {
            TaskKill_8c014b66((Task *) task);
            AsqFreeQueues_8c011f7e();
            var_loadScreenActive_8c157a6c = 0;
            njReleaseTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
            GameEnterDrive_8c01306e();
            InputPollDrive_8c012970();
            return;
        }
    }

    SpriteDraw_8c014f54(&var_loadingResourceGroup_8c1bc3f8, 0, 0.0f, 0.0f, -5.0f);
    frame = task->frame_0x0c;
    task->frame_0x0c = frame + 1;
    SpriteDraw_8c014f54(&var_loadingResourceGroup_8c1bc3f8, (frame >> 2) % 6 + 1, 0.0f, 0.0f, -4.0f);
}

void RouteSpawnLoadTask_8c0144fc(void)
{
    RouteLoadTask *task;
    void *state;

    LOG_DEBUG(("[ROUTE] spawning routeLoadTask_8c014338\n"));

    njSetBackColor(0xff418dff, 0xff418dff, 0xff418dff);
    var_loadScreenActive_8c157a6c = 1;

    TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, (void *) routeLoadTask_8c014338, (Task **) &task, &state, 0);
    CHANGE_LOAD_STATE(task, ROUTE_LOAD_STATE_INIT);
    task->frame_0x0c = 0;

    njGarbageTexture(var_tex_8c157af8, 0xc00);

    AsqInitQueues_8c011f36(0x20, 0x800, 0x800, 0x40);
}

STATIC void segmentReloadTask_8c014550(SegmentReloadTask *task, void *state)
{
    int frame;

    switch (task->stage_0x08) {
        case SEGMENT_RELOAD_STATE_POST_LOAD: {
            // Arm this segment's cutscene first: syncSegmentAssets reads cutsceneActive.
            EventPickForSegment_8c02b170();
            AsqResetQueues_8c011f6c();
            syncSegmentAssets_8c013f78();
            RouteClearLatch_8c014322();
            AsqProcessQueues_8c011fe0(AsqNop_8c011120, TileStreamLoad_8c021810, TileStreamRequestUpload_8c02190a, 0, RouteSetLatch_8c014330);
            CHANGE_SEGMENT_RELOAD_STATE(task, SEGMENT_RELOAD_STATE_WAIT);
            break;
        }

        case SEGMENT_RELOAD_STATE_WAIT: {
            if (RouteGetLatch_8c01432a() != 0) {
                CHANGE_SEGMENT_RELOAD_STATE(task, SEGMENT_RELOAD_STATE_IDLE);
                return;
            }
            break;
        }

        case SEGMENT_RELOAD_STATE_IDLE: {
            CHANGE_SEGMENT_RELOAD_STATE(task, SEGMENT_RELOAD_STATE_DONE);
            return;
        }

        case SEGMENT_RELOAD_STATE_DONE: {
            TaskKill_8c014b66((Task *) task);
            AsqFreeQueues_8c011f7e();
            var_loadScreenActive_8c157a6c = 0;
            njReleaseTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
            njSetTexture(var_interiorTexlist_8c1bc438);
            njLoadCacheTexture(var_interiorTexlist_8c1bc438);
            GameEnterDrive_8c01306e();
            InputPollDrive_8c012970();
            return;
        }
    }

    SpriteDraw_8c014f54(&var_loadingResourceGroup_8c1bc3f8, 0, 0.0f, 0.0f, -5.0f);
    frame = task->frame_0x0c;
    task->frame_0x0c = frame + 1;
    SpriteDraw_8c014f54(&var_loadingResourceGroup_8c1bc3f8, (frame >> 2) % 6 + 1, 0.0f, 0.0f, -4.0f);
}

void RouteSpawnSegmentReloadTask_8c01468e(void)
{
    SegmentReloadTask *task;
    void *state;

    /* Below hard, and in practice, each segment boundary refunds 30 driver
     * points. */
    if (var_progress_8c1ba1cc.difficulty_0xc4 < 2 || var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE) {
        var_runState_8c2285c4.driverPoints_0x0c += 0x1e;
        if (var_runState_8c2285c4.driverPointsMax_0x10 < var_runState_8c2285c4.driverPoints_0x0c) {
            var_runState_8c2285c4.driverPoints_0x0c = var_runState_8c2285c4.driverPointsMax_0x10;
        }
    }

    LOG_DEBUG(("[ROUTE] spawning segmentReloadTask_8c014550\n"));

    var_loadScreenActive_8c157a6c = 1;
    TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, (void *) segmentReloadTask_8c014550, (Task **) &task, &state, 0);
    CHANGE_SEGMENT_RELOAD_STATE(task, SEGMENT_RELOAD_STATE_POST_LOAD);
    task->frame_0x0c = 0;
    freeSegmentAssets_8c013f22();

    njGarbageTexture(var_tex_8c157af8, 0xc00);
    AsqInitQueues_8c011f36(0x20, 0x800, 0x800, 0x40);
    njSetTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
    njLoadCacheTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
    njSetBackColor(0xff418dff, 0xff418dff, 0xff418dff);
}

/* routeLoadTask_8c014338 without TileStreamLoad_8c021810, rebinding the
 * interior texture at the end. Only UnusedLoadSpawnTask_8c014934 spawns it, and
 * nothing calls that. */
void RouteUnusedTask_8c014784(RouteLoadTask *task, void *state)
{
    int frame;

    switch (task->stage_0x08) {
        case ROUTE_LOAD_STATE_INIT: {
            AsqResetQueues_8c011f6c();
            njSetTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
            njLoadCacheTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
            requestCourseAssets_8c014088();
            RouteClearLatch_8c014322();
            AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteSetLatch_8c014330);
            CHANGE_LOAD_STATE(task, ROUTE_LOAD_STATE_POST_LOAD);
            break;
        }

        case ROUTE_LOAD_STATE_POST_LOAD: {
            if (RouteGetLatch_8c01432a() != 0) {
                TileStreamInit_8c02175a();
                TrafficRelocatePlacementTable_8c026da4(var_currentCourse_8c1bb868.macCpu1_0x24);
                ObjectsRelocatePedGroupDefs_8c028de8(var_currentCourse_8c1bb868.macHumG0_0x30);
                ObjectsRelocatePedGroupLists_8c028dd0(var_currentCourse_8c1bb868.macHumM0_0x34);
                StopSetup_8c02caba();
                // Arm this segment's cutscene first: syncSegmentAssets reads cutsceneActive.
                EventPickForSegment_8c02b170();
                AsqResetQueues_8c011f6c();
                syncSegmentAssets_8c013f78();
                RouteClearLatch_8c014322();
                AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, TileStreamRequestUpload_8c02190a, 0, RouteSetLatch_8c014330);
                CHANGE_LOAD_STATE(task, ROUTE_LOAD_STATE_WAIT);
            }
            break;
        }

        case ROUTE_LOAD_STATE_WAIT: {
            if (RouteGetLatch_8c01432a() != 0) {
                CHANGE_LOAD_STATE(task, ROUTE_LOAD_STATE_IDLE);
                return;
            }
            break;
        }

        case ROUTE_LOAD_STATE_IDLE: {
            CHANGE_LOAD_STATE(task, ROUTE_LOAD_STATE_DONE);
            return;
        }

        case ROUTE_LOAD_STATE_DONE: {
            TaskKill_8c014b66((Task *) task);
            AsqFreeQueues_8c011f7e();
            var_loadScreenActive_8c157a6c = 0;
            njReleaseTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
            njSetTexture(var_interiorTexlist_8c1bc438);
            njLoadCacheTexture(var_interiorTexlist_8c1bc438);
            GameEnterDrive_8c01306e();
            InputPollDrive_8c012970();
            return;
        }
    }

    SpriteDraw_8c014f54(&var_loadingResourceGroup_8c1bc3f8, 0, 0.0f, 0.0f, -5.0f);
    frame = task->frame_0x0c;
    task->frame_0x0c = frame + 1;
    SpriteDraw_8c014f54(&var_loadingResourceGroup_8c1bc3f8, (frame >> 2) % 6 + 1, 0.0f, 0.0f, -4.0f);
}
