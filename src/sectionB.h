/* 8c0fcd20: undecompiled data section */
#ifndef _0FCD20_SECTIONB_H
#define _0FCD20_SECTIONB_H

#include <shinobi.h>
#include "013ae8_route_load.h"
#include "02af78_event.h"

/* =================
 * Type Declarations
 * =================
 */

typedef struct {
    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int field_0x10;
    int field_0x14;
    int field_0x18;
} Struct8c2264b8;

extern int var_8c22606c;
extern int var_8c2260a8;
extern void* var_8c2260ac;
extern void* var_8c2260b8;
extern void* var_8c2260c4;
extern void* var_8c226434;
extern void* var_8c226438;
extern Struct8c2264b8 var_8c2264b8;
extern Bool var_8c22655c;
extern Uint32 var_8c227da0;
extern int var_8c227da8;
extern int var_8c227dd4;
extern void* var_8c227e20;
extern void* var_8c227e24;
extern void* var_8c228234;
extern int var_8c22847c;

/* unlock-candidate scratch list built by EventScanCandidates_8c02b03c;
 * var_routeEvents_8c22851c points at the active route's EventEntry table */
extern EventEntry* var_routeEvents_8c22851c;
extern int var_eventCandidates_8c228520[];
extern int var_eventCandidateCount_8c228560;

extern int var_8c2285c4[];

/* gate for EventPickForSegment_8c02b170: only runs while
 * var_8c2285dc <= var_8c2285d8 (role of each side unclear) */
extern int var_8c2285d8;
extern int var_8c2285dc;

extern int var_8c228704;
extern void* var_8c2288f8;
extern Sint8 var_coursesToUnlock_8c225fd4[];
extern int var_currentSegment_8c228708;

/* table index (into the EventEntry array pointed to by var_routeEvents_8c22851c)
 * chosen by EventPickForSegment_8c02b170, consumed by
 * EventApplyFlags_8c02b292 */
extern int var_selectedEventEntry_8c228478;
extern void* var_currentSysResGroupInfo_8c225fb0;
extern int var_currentTileRegionList_8c226534;
extern int var_8c22640c;
extern int var_demoEntryValue_8c227e14;
extern int var_demoEntryValue_8c22822c;
extern int var_dialogQueue_8c225fbc[4]; // TODO: Confirm length
extern int var_dialogSequenceIsActive_8c225fb4;
extern int var_fogParam_8c226504;
extern int var_fogParam_8c226508;
extern float var_fogParam_8c227dd0;
extern Bool var_isFading_8c226568;
extern int var_menuTextboxCharLimit_8c225fb8;
extern ResourceGroup* var_resourceGroup_8c2263a8;
extern Sint8 var_soundMode_8c226070;
extern float var_uknVol_8c226468; // real type is 0100bc_sound.c's local UnknownVolStructB {float}
extern int var_vmuStatus_8c226048[9];

#endif // _0FCD20_SECTIONB_H
