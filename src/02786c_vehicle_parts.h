#ifndef _02786C_VEHICLE_PARTS_H
#define _02786C_VEHICLE_PARTS_H

#include "026710_traffic.h" /* TrafficEntry */

/* Also takes the player's BusState*, which shares TrafficEntry's 0x00-0x60
 * prefix. */
void VehiclePartsBind_8c02786c(TrafficEntry *entry, Uint32 typeCode);

#endif // _02786C_VEHICLE_PARTS_H
