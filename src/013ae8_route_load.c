#include <shinobi.h>
#include "011120_asset_queues.h"
#include "012504_input.h"
#include "012f44.h"
#include "013ae8_route_load.h"
#include "014a9c_tasks.h"
#include "014f54_text.h"
#include "02171c.h"
#include "026710.h"
#include "028258.h"
#include "02af78_event.h"
#include "02c884.h"
#include "03bd80_sectionD.h"
#include "014f54_text_pre_data.h"
#include "0fcd20_sectionB.h"
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
        (task)->field_0x08 = x; \
        LOG_DEBUG(("[ROUTE_LOAD] State changed: %s\n", DEBUG_routeLoadStateNames[x])); \
    } while (0)

#define CHANGE_SEGMENT_RELOAD_STATE(task, x) \
    do { \
        (task)->field_0x08 = x; \
        LOG_DEBUG(("[ROUTE_LOAD] State changed: %s\n", DEBUG_segmentReloadStateNames[x])); \
    } while (0)

/* ====================
 * Type Declarations
 * ====================
 */

enum ROUTE_LOAD_STATE {
    ROUTE_LOAD_STATE_INIT      = 0,
    ROUTE_LOAD_STATE_POST_LOAD = 1,
    ROUTE_LOAD_STATE_WAIT      = 2,
    ROUTE_LOAD_STATE_IDLE      = 3,
    ROUTE_LOAD_STATE_DONE      = 4,
};

enum SEGMENT_RELOAD_STATE {
    SEGMENT_RELOAD_STATE_POST_LOAD = 0,
    SEGMENT_RELOAD_STATE_WAIT      = 1,
    SEGMENT_RELOAD_STATE_IDLE      = 2,
    SEGMENT_RELOAD_STATE_DONE      = 3,
};

typedef struct {
    // 0..3
    Uint16 ukn_0x00;
    // id, 0..0x16c
    Uint16 ukn_0x02;
    // always 0
    Uint16 ukn_0x04;
    // id, 0..0xa9
    Uint16 ukn_0x06;
    void *ukn_0x08;
    // tile-region list (gates dat-file requests); element layout unconfirmed
    void *tileRegionList_0x0c;
    Sint8 *routeModelIndexes_0x10;
    void *ukn_0x14;
    Sint8 *pedestrianModelList_0x18;
    // scene object list (FUN_8c029ad4 streams nj/pvm/dat; e.g. O_FUMI railroad crossing)
    void *sceneObjectList_0x1c;
    char **datFilenames_0x20;
    FogParams *fog_0x24;
    ModelFiles *modelFiles_0x28;
} CourseSegment;

/* Appears to be directional lighting followed by color/coefficient records.
 * Roles inferred from value ranges + consumers 023310 */
typedef struct {
    float dir0_0x00[3];
    float rec0_0x0c[3][5];
    float dir1_0x48[3];
    float rec1_0x54[5];
    float dir2_0x68[3];
    float rec2_0x74[5];
} CourseSceneParams;

typedef struct {
    // enum ROUTE
    int route_0x00;
    // enum TIME_OF_DAY
    int timeOfDay_0x04;
    CourseSegment *segments_0x08;
    void *ukn_0x0c;
    CourseSceneParams *sceneParams_0x10;
    int ukn_0x14;
    int ukn_0x18;
    // nj/dat asset names loaded by loadRouteModels.
    char *filenames_0x1c[19];
} CourseConfig;

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

FogParams *var_fogParams_8c18ad28;

char var_datDir_8c18ad2c[0x20];

enum TIME_OF_DAY var_timeOfDay_8c18ad20;

// Read by driving/render units 023310, 026710, 021b9c, 0222dc, 024b4c
CourseSceneParams *var_sceneParams_8c18ad24;

int var_pvmReady_8c18adac;

Sint8 *var_routeModelIndexes_8c18adb0;

void *var_datFiles_8c18adb4[4];

/* ======================
 * Initialized Globals
 * ======================
 */

#include "013ae8_route_load.data.inc"

/* ======================
 * Forward Declarations
 * ======================
 */

void setPvmReady_8c014330(void);
void resetPvmReady_8c014322(void);

/* ==========
 * Functions
 * ==========
 */

STATIC void requestVehicleAssets_8c013ae8(void)
{
    LOG_DEBUG(("[ROUTE_LOAD] requesting vehicle assets\n"));

    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, "front.njd", &var_frontNj_8c1bc434, 0);
    AsqRequestPvm_8c011ac0(var_commonDir_8c18ad6c, "front.pvm", &var_frontTexlist_8c1bc430, 0xf, 0);

    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, "syanai.njd", &var_interiorNj_8c1bc43c, 0);
    AsqRequestPvm_8c011ac0(var_commonDir_8c18ad6c, "syanai.pvm", &var_interiorTexlist_8c1bc438, 0x40, 0x80000000);

    AsqRequestPvm_8c011ac0(var_commonDir_8c18ad6c, "mark.pvm", &var_markTexlist_8c1bc418, 3, 0);
    AsqRequestPvm_8c011ac0(var_commonDir_8c18ad6c, "busstop.pvm", &var_busStopTexlist_8c1bc424, 1, 0);

    var_trafficModels_8c1bc3f4 = AsqRequestModels_8c012030(var_commonDir_8c18ad6c, init_trafficModelFiles_8c043d64, 0);
}

void freeVehicleAssets_8c013b5a(void)
{
    Uint32 i;

    if (var_interiorTexlist_8c1bc438 != (NJS_TEXLIST *) -1) {
        LOG_DEBUG(("[ROUTE_LOAD] freeing vehicle assets\n"));

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

void clearModelSlots_8c013bbc(ModelSlot *slots, int count)
{
    ModelSlot *slot = &slots[--count];

    for (; slot >= slots; slot--) {
        slot->texlist_0x08 = (NJS_TEXLIST *) -1;
    }
}

/* Reconcile loaded route-model assets with a -1-terminated list of model
 * indices: request files for newly-wanted models, free the ones dropped. */
STATIC void syncRouteModelAssets_8c013c34(Sint8 *models)
{
    ModelSlot *slot;
    char **names;
    int i;

    LOG_DEBUG(("[ROUTE_LOAD] reconciling route-model assets (models=%p)\n", models));

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
                AsqRequestNj_8c011492(var_commonDir_8c18ad6c, init_8c043ecc[i * 2], 0, &var_routeModelSlots_8c1bbddc[i].nj_0x0c);
                names = init_8c043ecc;
            } else {
                AsqRequestNj_8c011492(var_commonDir_8c18ad6c, init_8c043dc4[i * 2], 0, &var_routeModelSlots_8c1bbddc[i].nj_0x0c);
                names = init_8c043dc4;
            }
            AsqRequestPvm_8c011ac0(var_commonDir_8c18ad6c, names[i * 2 + 1], &var_routeModelSlots_8c1bbddc[i].texlist_0x08, 0x60, 0);
        } else if (var_routeModelSlots_8c1bbddc[i].requested_0x00 == 0 && var_routeModelSlots_8c1bbddc[i].texlist_0x08 != (NJS_TEXLIST *) -1) {
            AsqReleaseAndFreeTexlist_8c011e3c(var_routeModelSlots_8c1bbddc[i].texlist_0x08);
            syFree(var_routeModelSlots_8c1bbddc[i].nj_0x0c);
            var_routeModelSlots_8c1bbddc[i].texlist_0x08 = (NJS_TEXLIST *) -1;
        }
    }
}

/* Callback for an asset-load pass. */
STATIC void finishAssetLoad_8c013d42(void)
{
    LOG_DEBUG(("[ROUTE_LOAD] asset load finished\n"));

    setPvmReady_8c014330();
    AsqFreeQueues_8c011f7e();
}

/* Initialize the asset queues and kick off one
 * load pass for the currently-wanted route models. */
void startRouteModelLoadPass_8c013d78(void)
{
    LOG_DEBUG(("[ROUTE_LOAD] starting route-model load pass\n"));

    AsqInitQueues_8c011f36(0, 0x40, 0, 0x40);
    AsqResetQueues_8c011f6c();
    resetPvmReady_8c014322();
    syncRouteModelAssets_8c013c34(var_routeModelIndexes_8c18adb0);
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, finishAssetLoad_8c013d42);
}

void freeAllRouteModels_8c013dae(void)
{
    ModelSlot *slot;

    LOG_DEBUG(("[ROUTE_LOAD] releasing all route-model assets\n"));

    for (slot = var_routeModelSlots_8c1bbddc; slot < &var_routeModelSlots_8c1bbddc[0x20]; slot++) {
        if (slot->texlist_0x08 != (NJS_TEXLIST *) -1) {
            AsqReleaseAndFreeTexlist_8c011e3c(slot->texlist_0x08);
            syFree(slot->nj_0x0c);
            slot->texlist_0x08 = (NJS_TEXLIST *) -1;
        }
    }
}

/* Unlike syncRouteModelAssets there is no njd, so no syFree. */
STATIC void syncPedestrianAssets_8c013df6(Sint8 *models)
{
    ModelSlot *slot;
    int i;

    LOG_DEBUG(("[ROUTE_LOAD] reconciling pedestrian textures (models=%p)\n", models));

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

/* texlist only */
void freePedestrianAssets_8c013ee4(void)
{
    ModelSlot *slot;

    LOG_DEBUG(("[ROUTE_LOAD] releasing pedestrian textures\n"));

    for (slot = var_pedestrianAssets_8c1bbfdc; slot < &var_pedestrianAssets_8c1bbfdc[0x41]; slot++) {
        if (slot->texlist_0x08 != (NJS_TEXLIST *) -1) {
            AsqReleaseAndFreeTexlist_8c011e3c(slot->texlist_0x08);
            slot->texlist_0x08 = (NJS_TEXLIST *) -1;
        }
    }
}

STATIC void freeSegmentModels_8c013f22(void)
{
    CourseSegment *entry;

    LOG_DEBUG(("[ROUTE_LOAD] freeing pairs for segment (index=%d)\n", var_currentSegment_8c228708));

    entry = &var_currentCourseConfig_8c18ad18->segments_0x08[var_currentSegment_8c228708];
    if (entry->modelFiles_0x28 != 0) {
        AsqFreeModels_8c0120fe(&var_segmentModels_8c1bc3f0);
    }
    if (entry->tileRegionList_0x0c != 0) {
        FUN_8c021a24();
    }
}

/* Bring the current segment's assets in line: publish its fog params,
 * request its models, sync both model tables, and request its dat files.
 * Practice/attract modes skip this. */
STATIC void syncSegmentModels_8c013f78(void)
{
    CourseSegment *entry;
    int i;

    LOG_DEBUG(("[ROUTE_LOAD] syncing assets for segment (index=%d)\n", var_currentSegment_8c228708));

    entry = &var_currentCourseConfig_8c18ad18->segments_0x08[var_currentSegment_8c228708];

    if (entry->fog_0x24 != 0) {
        var_fogParams_8c18ad28 = entry->fog_0x24;
        var_fogParam_8c226504 = var_fogParams_8c18ad28->field_0x00 - 1;
        var_fogParam_8c226508 = var_fogParam_8c226504 / 2;
        var_fogParam_8c227dd0 = var_fogParams_8c18ad28->field_0x04;
    }

    if (entry->modelFiles_0x28 != 0) {
        var_segmentModels_8c1bc3f0 = AsqRequestModels_8c012030(var_commonDir_8c18ad6c, entry->modelFiles_0x28, 0x10);
    }

    /* When pickSegmentEvent_8c02b170 armed a cutscene for this segment,
     * load the cutscene's actor set (init_8c043fd4) instead of the segment's
     * normal traffic models. */
    if (var_cutsceneActive_8c1bb900 == 0 || var_playMode_8c1bb8d0 != PLAY_MODE_NORMAL) {
        if (entry->routeModelIndexes_0x10 != 0) {
            var_routeModelIndexes_8c18adb0 = entry->routeModelIndexes_0x10;
            syncRouteModelAssets_8c013c34(entry->routeModelIndexes_0x10);
        }
    } else {
        if (entry->routeModelIndexes_0x10 != 0) {
            var_routeModelIndexes_8c18adb0 = entry->routeModelIndexes_0x10;
        }
        syncRouteModelAssets_8c013c34(init_8c043fd4);
    }

    if (entry->pedestrianModelList_0x18 != 0) {
        syncPedestrianAssets_8c013df6(entry->pedestrianModelList_0x18);
    }

    FUN_8c029ad4(entry->sceneObjectList_0x1c);

    if (entry->tileRegionList_0x0c == 0) {
        var_currentTileRegionList_8c226534 = -1;
    } else {
        var_currentTileRegionList_8c226534 = (int)entry->tileRegionList_0x0c;
        for (i = 0; i < 4; i++) {
            AsqRequestDat_8c011182(var_datDir_8c18ad2c, entry->datFilenames_0x20[i], &var_datFiles_8c18adb4[i]);
        }
    }

    /* Cutscene armed for this segment: run its setup. */
    if (var_cutsceneActive_8c1bb900 != 0 && var_playMode_8c1bb8d0 == PLAY_MODE_NORMAL) {
        FUN_8c02aa36();
    }
}

/* Select the route config record for the current
 * route id, and request all of its files. */
STATIC void loadRouteModels_8c014088(void)
{
    LOG_DEBUG(("[ROUTE_LOAD] requesting course %d models\n", var_currentCourse_8c1bb868.courseId_0x00));

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

    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[0], &var_currentCourse_8c1bb868.slots_0x04[0], 0);
    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[1], &var_currentCourse_8c1bb868.slots_0x04[1], 0);
    var_currentCourse_8c1bb868.slots_0x04[2] = var_currentCourseConfig_8c18ad18->filenames_0x1c[2];
    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[3], &var_currentCourse_8c1bb868.slots_0x04[3], 0);
    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[4], &var_currentCourse_8c1bb868.slots_0x04[4], 0);
    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[5], &var_currentCourse_8c1bb868.slots_0x04[5], 0);
    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[6], &var_currentCourse_8c1bb868.slots_0x04[6], 0);
    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[7], &var_currentCourse_8c1bb868.slots_0x04[7], 0);
    AsqRequestDat_8c011182(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[8], &var_currentCourse_8c1bb868.slots_0x04[8]);
    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[9], &var_currentCourse_8c1bb868.slots_0x04[9], 0);
    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[10], 0, &var_currentCourse_8c1bb868.slots_0x04[10]);
    AsqRequestDat_8c011182(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[11], &var_currentCourse_8c1bb868.slots_0x04[11]);
    AsqRequestDat_8c011182(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[12], &var_currentCourse_8c1bb868.slots_0x04[12]);
    AsqRequestDat_8c011182(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[13], &var_currentCourse_8c1bb868.slots_0x04[13]);
    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[14], &var_currentCourse_8c1bb868.slots_0x04[14], 0);
    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[15], &var_currentCourse_8c1bb868.slots_0x04[15], 0);
    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[16], &var_currentCourse_8c1bb868.slots_0x04[16], 0);
    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[17], &var_currentCourse_8c1bb868.slots_0x04[17], 0);
    AsqRequestNj_8c011492(var_commonDir_8c18ad6c, var_currentCourseConfig_8c18ad18->filenames_0x1c[18], &var_currentCourse_8c1bb868.slots_0x04[18], 0);

    requestVehicleAssets_8c013ae8();
    var_routeModels_8c1bc3ec = AsqRequestModels_8c012030(var_commonDir_8c18ad6c, init_8c0440dc, 0x10);
}

void resetPvmReady_8c014322(void)
{
    LOG_DEBUG(("[ROUTE_LOAD] pvm ready reset\n"));

    var_pvmReady_8c18adac = 0;
}

int isPvmReady_8c01432a(void)
{
    return var_pvmReady_8c18adac;
}

void setPvmReady_8c014330(void)
{
    LOG_DEBUG(("[ROUTE_LOAD] pvm ready set\n"));

    var_pvmReady_8c18adac = 1;
}

/* The route loads in stages. Full load runs
 * once at course entry via routeLoadTask. */
STATIC void routeLoadTask_8c014338(Task *task, void *state)
{
    int frame;

    switch (task->field_0x08) {
        case ROUTE_LOAD_STATE_INIT: {
            AsqResetQueues_8c011f6c();
            njSetTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
            njLoadCacheTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
            loadRouteModels_8c014088();
            resetPvmReady_8c014322();
            AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, setPvmReady_8c014330);
            CHANGE_LOAD_STATE(task, ROUTE_LOAD_STATE_POST_LOAD);
            return;
        }

        case ROUTE_LOAD_STATE_POST_LOAD: {
            if (isPvmReady_8c01432a() != 0) {
                FUN_8c02175a();
                FUN_8c026da4(var_currentCourse_8c1bb868.slots_0x04[8]);
                FUN_8c028de8(var_currentCourse_8c1bb868.slots_0x04[11]);
                FUN_8c028dd0(var_currentCourse_8c1bb868.slots_0x04[12]);
                FUN_8c02caba();
                // Arm this segment's cutscene first: syncSegmentModels reads cutsceneActive.
                pickSegmentEvent_8c02b170();
                AsqResetQueues_8c011f6c();
                syncSegmentModels_8c013f78();
                resetPvmReady_8c014322();
                AsqProcessQueues_8c011fe0(AsqNop_8c011120, FUN_8c021810, FUN_8c02190a, 0, setPvmReady_8c014330);
                CHANGE_LOAD_STATE(task, ROUTE_LOAD_STATE_WAIT);
            }
            break;
        }

        case ROUTE_LOAD_STATE_WAIT: {
            if (isPvmReady_8c01432a() != 0) {
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
            freeTask_8c014b66(task);
            AsqFreeQueues_8c011f7e();
            var_loadScreenActive_8c157a6c = 0;
            njReleaseTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
            FUN_8c01306e();
            dispatchInputTask_8c012970();
            return;
        }
    }

    drawSprite_8c014f54(&var_loadingResourceGroup_8c1bc3f8, 0, 0.0f, 0.0f, -5.0f);
    // Loading animation
    frame = (int) task->field_0x0c;
    task->field_0x0c = (void *) (frame + 1);
    drawSprite_8c014f54(&var_loadingResourceGroup_8c1bc3f8, (frame >> 2) % 6 + 1, 0.0f, 0.0f, -4.0f);
}

void pushRouteLoadTask_8c0144fc(void)
{
    Task *task;
    void *state;

    LOG_DEBUG(("[ROUTE_LOAD] pushing routeLoadTask_8c014338\n"));

    njSetBackColor(0xff418dff, 0xff418dff, 0xff418dff);
    var_loadScreenActive_8c157a6c = 1;

    pushTask_8c014ae8(var_tasks_8c1ba3c8, (void *) routeLoadTask_8c014338, &task, &state, 0);
    CHANGE_LOAD_STATE(task, ROUTE_LOAD_STATE_INIT);
    task->field_0x0c = 0;

    njGarbageTexture(var_tex_8c157af8, 0xc00);

    AsqInitQueues_8c011f36(0x20, 0x800, 0x800, 0x40);
}

/* Segment-boundary reload: load the new segment's assets,
 * rebind the interior texture, then hand off to the input task. */
STATIC void unknownSegmentReloadTask_8c014550(Task *task, void *state)
{
    int frame;

    switch (task->field_0x08) {
        case SEGMENT_RELOAD_STATE_POST_LOAD: {
            // Arm this segment's cutscene first: syncSegmentModels reads cutsceneActive.
            pickSegmentEvent_8c02b170();
            AsqResetQueues_8c011f6c();
            syncSegmentModels_8c013f78();
            resetPvmReady_8c014322();
            AsqProcessQueues_8c011fe0(AsqNop_8c011120, FUN_8c021810, FUN_8c02190a, 0, setPvmReady_8c014330);
            CHANGE_SEGMENT_RELOAD_STATE(task, SEGMENT_RELOAD_STATE_WAIT);
            break;
        }

        case SEGMENT_RELOAD_STATE_WAIT: {
            if (isPvmReady_8c01432a() != 0) {
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
            freeTask_8c014b66(task);
            AsqFreeQueues_8c011f7e();
            var_loadScreenActive_8c157a6c = 0;
            njReleaseTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
            njSetTexture(var_interiorTexlist_8c1bc438);
            njLoadCacheTexture(var_interiorTexlist_8c1bc438);
            FUN_8c01306e();
            dispatchInputTask_8c012970();
            return;
        }
    }

    drawSprite_8c014f54(&var_loadingResourceGroup_8c1bc3f8, 0, 0.0f, 0.0f, -5.0f);
    // Loading animation
    frame = (int) task->field_0x0c;
    task->field_0x0c = (void *) (frame + 1);
    drawSprite_8c014f54(&var_loadingResourceGroup_8c1bc3f8, (frame >> 2) % 6 + 1, 0.0f, 0.0f, -4.0f);
}

void pushUnknownSegmentReloadTask_8c01468e(void)
{
    Task *task;
    void *state;

    if (var_progress_8c1ba1cc.field_0xc4 < 2 || var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE) {
        var_8c2285c4[3] += 0x1e;
        if (var_8c2285c4[4] < var_8c2285c4[3]) {
            var_8c2285c4[3] = var_8c2285c4[4];
        }
    }

    LOG_DEBUG(("[ROUTE_LOAD] pushing unknownSegmentReloadTask_8c014550\n"));

    var_loadScreenActive_8c157a6c = 1;
    pushTask_8c014ae8(var_tasks_8c1ba3c8, (void *) unknownSegmentReloadTask_8c014550, &task, &state, 0);
    CHANGE_SEGMENT_RELOAD_STATE(task, SEGMENT_RELOAD_STATE_POST_LOAD);
    task->field_0x0c = 0;
    freeSegmentModels_8c013f22();

    njGarbageTexture(var_tex_8c157af8, 0xc00);
    AsqInitQueues_8c011f36(0x20, 0x800, 0x800, 0x40);
    njSetTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
    njLoadCacheTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
    njSetBackColor(0xff418dff, 0xff418dff, 0xff418dff);
}

/* Like routeLoadTask_8c014338, but on completion binds the interior texture and
 * hands off to the input task (as unknownSegmentReloadTask_8c014550 does). */
void unknownRouteLoadTask_8c014784(Task *task, void *state)
{
    int frame;

    switch (task->field_0x08) {
        case ROUTE_LOAD_STATE_INIT: {
            AsqResetQueues_8c011f6c();
            njSetTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
            njLoadCacheTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
            loadRouteModels_8c014088();
            resetPvmReady_8c014322();
            AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, setPvmReady_8c014330);
            CHANGE_LOAD_STATE(task, ROUTE_LOAD_STATE_POST_LOAD);
            break;
        }

        case ROUTE_LOAD_STATE_POST_LOAD: {
            if (isPvmReady_8c01432a() != 0) {
                FUN_8c02175a();
                FUN_8c026da4(var_currentCourse_8c1bb868.slots_0x04[8]);
                FUN_8c028de8(var_currentCourse_8c1bb868.slots_0x04[11]);
                FUN_8c028dd0(var_currentCourse_8c1bb868.slots_0x04[12]);
                FUN_8c02caba();
                // Arm this segment's cutscene first: syncSegmentModels reads cutsceneActive.
                pickSegmentEvent_8c02b170();
                AsqResetQueues_8c011f6c();
                syncSegmentModels_8c013f78();
                resetPvmReady_8c014322();
                AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, FUN_8c02190a, 0, setPvmReady_8c014330);
                CHANGE_LOAD_STATE(task, ROUTE_LOAD_STATE_WAIT);
            }
            break;
        }

        case ROUTE_LOAD_STATE_WAIT: {
            if (isPvmReady_8c01432a() != 0) {
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
            freeTask_8c014b66(task);
            AsqFreeQueues_8c011f7e();
            var_loadScreenActive_8c157a6c = 0;
            njReleaseTexture(var_loadingResourceGroup_8c1bc3f8.tlist_0x00);
            njSetTexture(var_interiorTexlist_8c1bc438);
            njLoadCacheTexture(var_interiorTexlist_8c1bc438);
            FUN_8c01306e();
            dispatchInputTask_8c012970();
            return;
        }
    }

    drawSprite_8c014f54(&var_loadingResourceGroup_8c1bc3f8, 0, 0.0f, 0.0f, -5.0f);
    // Loading animation
    frame = (int) task->field_0x0c;
    task->field_0x0c = (void *) (frame + 1);
    drawSprite_8c014f54(&var_loadingResourceGroup_8c1bc3f8, (frame >> 2) % 6 + 1, 0.0f, 0.0f, -4.0f);
}
