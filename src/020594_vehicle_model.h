/* 8c020594 */
#ifndef _020594_VEHICLE_MODEL_H
#define _020594_VEHICLE_MODEL_H

#include <shinobi.h>

#include "1ba1c8_globals.h" /* BusState */

/* Pitch from the forward ground probe, roll from the lateral pair;
 * width_0x23c and height_0x244 are their spacings. Also caches the pitch for
 * the camera. `bus` may be a TrafficEntry*, which shares this field prefix. */
void VehicleModelPlace_8c020594(NJS_MATRIX *matrix, BusState *bus);

#endif // _020594_VEHICLE_MODEL_H
