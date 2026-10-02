/* 8c013ae8 */
#ifndef _013AE8_ROUTE_H
#define _013AE8_ROUTE_H

#include <shinobi.h>
#include "011120_asset_queues.h" /* ModelFiles */
#include "014a9c_tasks.h"
#include "02171c_tile_stream.h" /* TileIndex */

/* =================
 * Type Declarations
 * =================
 */

enum ROUTE {
    ROUTE_SHINJUKU = 0,
    ROUTE_WANGAN   = 1,
    ROUTE_OME      = 2,
};

typedef enum {
    ROUTE_LOAD_STATE_INIT      = 0,
    ROUTE_LOAD_STATE_POST_LOAD = 1,
    ROUTE_LOAD_STATE_WAIT      = 2,
    ROUTE_LOAD_STATE_IDLE      = 3,
    ROUTE_LOAD_STATE_DONE      = 4,
} RouteLoadState;

/* Task layout as seen by the course load tasks. */
typedef struct {
    TaskAction action;
    void *state;
    RouteLoadState stage_0x08;
    int frame_0x0c;
    int field_0x10;
    int field_0x14;
    void *queuedItem_0x18;
    int field_0x1c;
} RouteLoadTask;

enum TIME_OF_DAY {
    TIME_OF_DAY_DAY     = 0,
    TIME_OF_DAY_EVENING = 1,
    TIME_OF_DAY_NIGHT   = 2,
};

typedef struct {
    int tileWindowSpan_0x00;
    float farClipDepth_0x04;
    Uint8 fogBlue_0x08;
    Uint8 fogGreen_0x09;
    Uint8 fogRed_0x0a;
    Uint8 fogAlpha_0x0b;
    float fogN_0x0c;
    float fogF_0x10;
} ViewParams;

typedef struct {
    int requested_0x00;
    int needsLoad_0x04;
    NJS_TEXLIST *texlist_0x08; // -1 when unloaded
    void *nj_0x0c;
} ModelSlot;

/* Probably directional lights, each followed by color/coefficient records. */
typedef struct {
    float dir0_0x00[3];
    float rec0_0x0c[3][5];
    float dir1_0x48[3];
    float rec1_0x54[5];
    float dir2_0x68[3];
    float rec2_0x74[5];
} CourseSceneParams;

/* Asset handles for the loaded course, one per CourseConfig filename field of
 * the same name. The names follow the course's files (Shinjuku day:
 * "s_atari_bus.dat", "sd_road_x.dat", ...): atari = collision, line = route
 * path, attr = attributes, mac = machine/actor, hum = pedestrian. */
typedef struct {
    int courseId_0x00;
    void *atariBus_0x04;
    /* also an 8-byte-stride stop-area table, indexed by a segment record's
     * stopAreaId_0x02; each slot holds a StopAreaRecord* at +0, the trailing
     * 4 bytes unknown. */
    void *lineBus_0x08;
    /* the route's LineBusNode table, borrowed from the config */
    void *lineNodes_0x0c;
    void *attrBus_0x10;
    void *attrMark_0x14;
    void *atariCpu_0x18;
    void *lineCpu_0x1c;
    void *attrCpu_0x20;
    /* once relocated, a table of placed-instance lists per scene-object type,
     * indexed by a CourseSegment.sceneObjectTypeIds_0x14 entry */
    void *macCpu1_0x24;
    void *atariHum_0x28;
    /* also a 12-byte-stride stop spawn-area table, indexed by a segment
     * record's ukn_0x06, whose records start with a spawn-area pointer */
    void *lineHum_0x2c;
    void *macHumG0_0x30;
    void *macHumM0_0x34;
    void *macSignal_0x38; /* TrafficSignalDef table, terminated by type_0x00 == 0 */
    /* road_x, machi_x (town), uv_x, shadow_x -- Ome splits uv into uv1/uv2,
     * Shinjuku and Wangan repeat shadow_x for the model-only 5th layer. */
    TileIndex *tileLayers_0x3c[5];
} CurrentCourse;

/* =====================
 * External Declarations
 * =====================
 */

extern enum ROUTE var_route_8c18ad1c;
extern enum TIME_OF_DAY var_timeOfDay_8c18ad20;
extern ViewParams *var_viewParams_8c18ad28;

extern CourseSceneParams *var_sceneParams_8c18ad24;

/* One per route segment. */
typedef struct {
    // 0 = terminator; 2 = stop forced here; 3 = excluded from the random-stop
    // pick
    Uint16 type_0x00;
    // id, 0..0x16c
    Uint16 stopAreaId_0x02;
    // always 0
    Uint16 ukn_0x04;
    // id, 0..0xa9
    Uint16 ukn_0x06;
    // candidate stop spots, {count,rec*}-style entries
    void *stopCandidates_0x08;
    // tile-region list (gates dat-file requests); element layout unconfirmed
    void *tileRegionList_0x0c;
    Sint8 *routeModelIndexes_0x10;
    // terminated by 0xff
    Uint8 *sceneObjectTypeIds_0x14;
    Sint8 *pedestrianModelList_0x18;
    // scene objects and their nj/pvm/dat (e.g. O_FUMI railroad crossing)
    void *sceneObjectList_0x1c;
    char **datFilenames_0x20;
    ViewParams *viewParams_0x24;
    ModelFiles *modelFiles_0x28;
} CourseSegment;

typedef struct {
    // enum ROUTE
    int route_0x00;
    // enum TIME_OF_DAY
    int timeOfDay_0x04;
    CourseSegment *segments_0x08;
    void *ukn_0x0c;
    CourseSceneParams *sceneParams_0x10;
    // [min, max) of the run's total stop count
    int randomStopCountMin_0x14;
    int randomStopCountMax_0x18;
    /* asset filenames for the CurrentCourse field of the same name */
    char *atariBusFile_0x1c;
    char *lineBusFile_0x20;
    /* not a filename: the route's LineBusNode table */
    void *lineNodes_0x24;
    char *attrBusFile_0x28;
    char *attrMarkFile_0x2c;
    char *atariCpuFile_0x30;
    char *lineCpuFile_0x34;
    char *attrCpuFile_0x38;
    char *macCpu1File_0x3c;
    char *atariHumFile_0x40;
    char *lineHumFile_0x44;
    char *macHumG0File_0x48;
    char *macHumM0File_0x4c;
    char *macSignalFile_0x50;
    char *tileLayerFiles_0x54[5];
} CourseConfig;

extern CourseConfig *var_currentCourseConfig_8c18ad18;

/* BUP_HEADER.game_name -- the VMS sort key, not text. */
extern Sint8 init_bupGameName_8c04410c[16];

extern char var_pvrDir_8c18ad4c[0x20];
extern char var_commonDir_8c18ad6c[0x20];
extern char var_commonDirCopy_8c18ad8c[0x20];
extern void *var_datFiles_8c18adb4[4]; /* one per texel layer, freed after TileStreamLoad_8c021810 */

/* =========
 * Functions
 * =========
 */

void RouteInstallLoad_8c0144fc(void);
void RouteInstallSegmentReload_8c01468e(void);
void RouteSetLatch_8c014330(void);
void RouteClearLatch_8c014322(void);
void RouteFreeVehicleAssets_8c013b5a(void);
void RouteClearModelSlots_8c013bbc(ModelSlot *slots, int count);
void RouteStartModelLoadPass_8c013d78(void);
void RouteFreeAllModels_8c013dae(void);
void RouteFreePedestrianAssets_8c013ee4(void);
int RouteGetLatch_8c01432a(void);
void RouteUnusedTask_8c014784(RouteLoadTask *task, void *state);

#endif // _013AE8_ROUTE_H
