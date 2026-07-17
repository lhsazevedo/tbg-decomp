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

void RouteLoadPushTask_8c0144fc(void);
void RouteLoadPushTask_8c0144fc(void);
void RouteLoadPushSegmentReloadTask_8c01468e(void);
void RouteLoadSetPvmReady_8c014330(void);
void RouteLoadResetPvmReady_8c014322(void);
void RouteLoadFreeVehicleAssets_8c013b5a(void);
void RouteLoadClearModelSlots_8c013bbc(ModelSlot *slots, int count);
void RouteLoadStartRouteModelLoadPass_8c013d78(void);
void RouteLoadFreeAllRouteModels_8c013dae(void);
void RouteLoadFreePedestrianAssets_8c013ee4(void);
int RouteLoadIsPvmReady_8c01432a(void);
void RouteLoadUnusedTask_8c014784(Task *task, void *state);

#endif // _013AE8_ROUTE_LOAD_H
