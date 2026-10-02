/* 8c02e2dc */
#ifndef _02E2DC_BUS_COLLISION_H
#define _02E2DC_BUS_COLLISION_H

#include <shinobi.h>
#include "026710_traffic.h" /* TrafficEntry */

/* Local-space bounding boxes, indexed by variantIndex_0x2e0. */
extern NJS_POINT3 *init_variantBoxes_8c04c940[16];

/* Leaves the bus's world-space box in var_collisionSelfBox_8c228978.
 * busDistance_0x490 lies past BusState's 0x3cc end, so the bus never hits
 * itself. */
TrafficEntry *BusCollisionFindHit_8c02e2dc(void);

#endif // _02E2DC_BUS_COLLISION_H
