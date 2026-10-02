/* 8c027958 */
#ifndef _027958_BUS_DRAW_H
#define _027958_BUS_DRAW_H

#include "1ba1c8_globals.h" /* BusState */
#include "026710_traffic.h" /* TrafficEntry */

/* Poses a vehicle's model nodes from its state: wheels, body lean and lamps.
 * Only drives the extra nodes VehiclePartsBind_8c02786c bound for its type. */
void BusDrawUpdateModels_8c027958(BusState *bus);

/* Run each frame at night. Forces the two running-light bits on and
 * crossfades lightCoeffRow_0x0c4 over 20 frames; lightFadeGate_0x2dc, an
 * attribute of the CPU-grid polygon under the vehicle, picks the direction. */
void BusDrawFadeLights_8c028022(BusState *bus);

/* DrawFn2: objArg is a type-1 TrafficSignal*, matrixArg an NJS_MATRIX*. */
void BusDrawSignal_8c0281ac(int objArg, int matrixArg);

/* DrawFn2 for a type 2/3/4 TrafficSignal's attachment, whose only lamp
 * drawA_0xc8 shows. */
void BusDrawSignalAttachment_8c028206(int objArg, int matrixArg);

/* accel is a moving vehicle's 4-frame speed-delta sum, 0 for a parked one.
 * Queues the vehicle's forward and mirror draws, then advances its wheels and
 * lean and re-aligns it to the ground. */
void BusDrawPlaceEntity_8c027c3c(TrafficEntry *entity, float accel);

#endif // _027958_BUS_DRAW_H
