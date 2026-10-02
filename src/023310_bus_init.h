/* 8c023310 */
#ifndef _023310_BUS_INIT_H
#define _023310_BUS_INIT_H

#include "023938_bus_drive.h" /* LineBusSegment, LineBusNode */

/* =======================
 * Non-initialized Globals
 * =======================
 */

/* The active course's route line: its segments and the nodes linking them. */
extern LineBusSegment *var_lineSegments_8c227d84;
extern LineBusNode *var_lineNodes_8c227d88;

/* =========
 * Functions
 * =========
 */

/* Places the player's bus at the segment's stop and spawns its task. */
void BusInitStart_8c023610(void);

#endif // _023310_BUS_INIT_H
