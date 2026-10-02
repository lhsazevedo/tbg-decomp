#ifndef _02DF3C_TRAFFIC_LOOKAHEAD_H
#define _02DF3C_TRAFFIC_LOOKAHEAD_H

#include "014a9c_tasks.h"       /* Task */
#include "026710_traffic.h"     /* TrafficEntry */

/* Both functions below fill entry->lookaheadPoints_0x49c: the path ahead of
 * the entry sampled every 5 units as world (x, z) pairs, 9999.0 terminated.
 * The cache is an odometer -- lookaheadCacheLen_0x4ec shrinks as the entry
 * drives, and the scan refills it. */

/* Seeds the cache from scratch, out to 25 units. Call once the entry's path
 * cursor is resolved; the four cache fields must still be 0. */
void TrafficLookaheadInit_8c02df3c(TrafficEntry *entry);

/* Reports what is standing on the entry's path within `lookahead` units:
 * var_playerBus_8c1bbd9c for the player's bus, another entry's state, or NULL.
 * Refills the cache to 20 units first if driving has run it short. */
void *TrafficLookaheadScan_8c02dfca(
    /* The entry's own task, excluded from the scan */
    Task *self,
    TrafficEntry *entry,
    float lookahead
);

#endif // _02DF3C_TRAFFIC_LOOKAHEAD_H
