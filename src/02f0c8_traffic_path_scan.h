#ifndef _02F0C8_TRAFFIC_PATH_SCAN_H
#define _02F0C8_TRAFFIC_PATH_SCAN_H

#include "014a9c_tasks.h"
#include "026710_traffic.h"

/* =======================
 * Non-initialized Globals
 * =======================
 */

/** The route's signal ids grouped by intersection */
extern Sint32 *var_signalGroups_8c228b40;

/** The group whose intersection a vehicle occupies */
extern Sint32 *var_occupiedGroup_8c228b44;

/* ===================
 * Initialized Globals
 * ===================
 */

/** Per-route signal groups, selected into var_signalGroups_8c228b40 by TrafficInit_8c02769e */
extern Sint32 init_signalGroupsWangan_8c04c980[91];
extern Sint32 init_signalGroupsShinjuku_8c04caec[147];
extern Sint32 init_signalGroupsOme_8c04cd38[54];

/* =========
 * Functions
 * =========
 */

/**
 * Walks the remaining samples and returns the state of the
 * first task whose entry sits within 2.5 units of a sample.
 */
void *TrafficPathScanNext_8c02f212(void);

/**
 * Reports what occupies the stretch of path an entry is about to drive.
 */
void *TrafficPathScanBuild_8c02f0c8(
    /* The spawning task, excluded from the scan */
    Task *self,
    /* Supplies the successor records once firstRecord's run ends */
    TrafficEntry *entry,
    /* Path record the walk starts from */
    PathRecord *firstRecord,
    /* FirstRecord's index in entry->pathBlocks_0x304 */
    Sint32 argIndex,
    /* Distance along the path to skip before sampling */
    float startProgress,
    /* Distance sampled past startProgress */
    float window
);

/**
 * Reports whether signalId is one of the signals
 * at the intersection where a vehicle currently stands.
 */
Sint32 TrafficPathScanJunctionOccupied_8c02f28a(
    /* Signal id, as in TrafficEntry.signalWaitId_0x450 */
    Sint32 signalId
);

#endif // _02F0C8_TRAFFIC_PATH_SCAN_H
