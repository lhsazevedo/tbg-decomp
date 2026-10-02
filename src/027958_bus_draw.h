/* 8c027958 */
#ifndef _027958_BUS_DRAW_H
#define _027958_BUS_DRAW_H

#include "1ba1c8_globals.h" /* BusState */
#include "026710_traffic.h" /* TrafficEntry */

/* Pushes one frame of animation state onto a vehicle's model nodes: wheel spin
 * and steering angle, suspension pitch/roll on the body, and each
 * blinkerLights_0x02c entry shown or hidden from its bit of blinker_0x080.
 * The switch on typeCode_0x000 has the same arms as VehiclePartsBind_8c02786c
 * (02786c_vehicle_parts) -- it only drives the extra wheel and turn-lamp nodes
 * that binder cached for this vehicle type.
 *
 * Called with the player's BusState by BusCameraDrawBusModel_8c024bb8 (024b4c), and with a
 * TrafficEntry by this unit's own near-draw callbacks. */
void BusDrawUpdateModels_8c027958(BusState *bus);

/* Called by BusTask_8c022bdc (022bdc) with the player's BusState and by
 * TrafficDriveVehicleTask_8c025b98 (025b98) with a CPU vehicle, each frame at
 * night. Forces blinker_0x080's two running-light bits on, then crossfades
 * lightCoeffRow_0x0c4 between the two CourseSceneParams rows 026710 cached
 * (var_nightLightIntensityOff_8c1bbda8 and ...On_8c1bbdb0, 1ba1c8_globals.h) over 20
 * frames. lightFadeGate_0x2dc drives the direction: it is attribute word [1]
 * of the CPU-grid polygon under the vehicle, so the fade follows the road. */
void BusDrawFadeLights_8c028022(BusState *bus);

/* DrawFn2 (022464_render.h, hence the (int, int) signature) for a type-1
 * TrafficSignal: shows the frames_0x10 lamp named by frame_0x0c and hides the
 * other two, then draws model_0xb8 at matrixArg. objArg is the TrafficSignal*,
 * matrixArg an NJS_MATRIX*. */
void BusDrawSignal_8c0281ac(int objArg, int matrixArg);

/* DrawFn2 for a type 2/3/4 TrafficSignal's attachment: same draw as
 * BusDrawSignal_8c0281ac, but frames_0x10[0] is the only lamp and drawA_0xc8
 * decides whether it shows. */
void BusDrawSignalAttachment_8c028206(int objArg, int matrixArg);

/* Called once per frame per traffic entity by 025b98: by
 * TrafficDriveVehicleTask_8c025b98 for a moving CPU vehicle, passing its 4-frame
 * speed-delta sum as `accel`, and by TrafficDriveParkedTask_8c02656a for a
 * fixed one, passing 0. Two jobs:
 *
 * 1. Queues the entity's draw for this frame through
 *    RenderQueueDraw2_8c022420. Two independent cone tests against
 *    var_busState_8c1bb9d0, either or both of which can fire: the forward one
 *    (200m, ~80 degrees around the bus's last move delta) takes the
 *    detailed path, the mirror one (50m, ~15 degrees around the rear-view
 *    camera at mirrorWorldOffsetX_0x318.._0x330) the cheap one. Each picks its
 *    own LOD from the distance.
 *
 * 2. Advances the entity's wheel spin and suspension lean, then re-probes the
 *    ground under it through probeFn_0x2c8 and re-aligns worldMatrix_0x84 via
 *    VehicleModelPlace_8c020594. Skipped entirely for an entity that is
 *    neither drawn nor near the bus, and skipped again for one already aligned
 *    and standing still. */
void BusDrawPlaceEntity_8c027c3c(TrafficEntry *entity, float accel);

#endif // _027958_BUS_DRAW_H
