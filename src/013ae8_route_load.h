/* 8c013ae8 */
#ifndef _013AE8_ROUTE_LOAD_H
#define _013AE8_ROUTE_LOAD_H

#include <shinobi.h>
#include "014a9c_tasks.h"

/* =================
 * Type Declarations
 * =================
 */

enum ROUTE {
    ROUTE_SHINJUKU = 0,
    ROUTE_WANGAN   = 1,
    ROUTE_OME      = 2,
};

enum TIME_OF_DAY {
    TIME_OF_DAY_DAY     = 0,
    TIME_OF_DAY_EVENING = 1,
    TIME_OF_DAY_NIGHT   = 2,
};

typedef struct {
    int field_0x00;
    float field_0x04;
    Uint8 field_0x08;
    Uint8 field_0x09;
    Uint8 field_0x0a;
    Uint8 field_0x0b;
    float fogN_0x0c;
    float fogF_0x10;
} FogParams;

/* One entry of var_routeModelSlots_8c1bbddc. */
typedef struct {
    int requested_0x00;
    int needsLoad_0x04;
    NJS_TEXLIST *texlist_0x08; // -1 when unloaded
    void *nj_0x0c;
} ModelSlot;

typedef struct {
    int courseId_0x00;
    // asset handles filled by loadRouteModels
    void *slots_0x04[19];
} CurrentCourse;

/* =====================
 * External Declarations
 * =====================
 */

extern enum ROUTE var_route_8c18ad1c;
extern enum TIME_OF_DAY var_timeOfDay_8c18ad20;
extern FogParams *var_fogParams_8c18ad28;

/* =========
 * Functions
 * =========
 */

void pushRouteLoadTask_8c0144fc(void);
void pushUnknownSegmentReloadTask_8c01468e(void);
void setPvmReady_8c014330(void);
void resetPvmReady_8c014322(void);
void requestVehicleAssets_8c013ae8(void);
void freeVehicleAssets_8c013b5a(void);
void clearModelSlots_8c013bbc(ModelSlot *slots, int count);
void syncRouteModelAssets_8c013c34(Sint8 *models);
void finishAssetLoad_8c013d42(void);
void startRouteModelLoadPass_8c013d78(void);
void freeAllRouteModels_8c013dae(void);
void syncPedestrianAssets_8c013df6(Sint8 *models);
void freePedestrianAssets_8c013ee4(void);
void freeSegmentModels_8c013f22(void);
void syncSegmentModels_8c013f78(void);
void loadRouteModels_8c014088(void);
int isPvmReady_8c01432a(void);
void routeLoadTask_8c014338(Task *task, void *state);
void unknownSegmentReloadTask_8c014550(Task *task, void *state);
void unknownRouteLoadTask_8c014784(Task *task, void *state);

#endif // _013AE8_ROUTE_LOAD_H
