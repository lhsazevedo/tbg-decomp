/* 8c025b98 */
#ifndef _025B98_TRAFFIC_DRIVE_H
#define _025B98_TRAFFIC_DRIVE_H

#include <shinobi.h>

/* TaskSpawn_8c014ae8 action for a moving traffic entry (installed by
 * spawnEntry_8c0272b8, 026710_traffic.h). */
void TrafficDriveVehicleTask_8c025b98();

/* TaskSpawn_8c014ae8 action for a fixed-position traffic entry (script
 * type 10; installed by spawnEntry_8c0272b8, 026710_traffic.h). */
void TrafficDriveParkedTask_8c02656a();

#endif // _025B98_TRAFFIC_DRIVE_H
