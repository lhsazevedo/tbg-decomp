#include "shinobi.h"

#ifndef _02D19C_PASSENGER_H_
#define _02D19C_PASSENGER_H_

#include "014a9c_tasks.h"

typedef struct {
    void *spot_0x00;
    Uint32 state_0x04;
    NJS_POINT3 pos_0x08;
    Uint32 spriteNo_0x14;
    float jitterX_0x18;
    float jitterZ_0x1c;
    Sint32 delay_0x20;
    Sint32 seat_0x24;
    Uint32 isSeated_0x28;
    Uint32 voice_0x2c;
    Uint32 soundIdA_0x30;
    Uint32 soundIdB_0x34;
} PassengerState;

/* Bus-local, indexed like var_seats_8c228718. */
typedef struct {
    float x;
    float z;
} SeatPos;
extern SeatPos init_seatPositions_8c04c3e4[31];

typedef struct {
    int phase_0x00;
    int subPhase_0x04;
} PassengerStopSceneState;

/* A seated passenger who isn't getting off at this stop. */
void PassengerSeatedTask_8c02d5ca(Task *task, void *state);

void PassengerBoardTask_8c02d21c(Task *task, PassengerState *state);

void PassengerExitTask_8c02d46c(Task *task, PassengerState *state);

void PassengerStopSceneTask_8c02d644(Task *task, PassengerStopSceneState *state);

/* Replaces the stop scene in practice lessons without passengers. */
void PassengerSkipStopTask_8c02d8f0(Task *task, void *state);

/* Passenger waypoints, bus-local except boardSpot1 and exitSpot3 (outside
 * the bus, in world space). Passengers jump from one to the next on the frame
 * var_passengersFadedOut_8c22895c is set.
 *
 * Named by sequence, not place: Ome fills them in reverse, swapping doors.
 * (0.15, 0.68, 0.27) is the fare box by the driver -- boardSpot2 on
 * flat-fare Shinjuku/Wangan, exitSpot2 on distance-fare Ome -- which is why
 * the voice lines play on boarding off Ome and on exiting on Ome. */
extern NJS_POINT3 var_boardSpot2_8c228910;
extern NJS_POINT3 var_boardSpot3_8c22891c;
extern NJS_POINT3 var_boardSpot1_8c228928;
extern NJS_POINT3 var_exitSpot1_8c228934;
extern NJS_POINT3 var_exitSpot2_8c228940;
extern NJS_POINT3 var_exitSpot3_8c22894c;

/* Set for the one frame the passenger fade is fully out. */
extern int var_passengersFadedOut_8c22895c;

/* NJS_ARGB for the in-bus passengers; the alpha fades at 1/15 per frame.
 * [4] is not part of the color. */
extern float var_passengerFadeColor_8c228960[5];

#endif /* _02D19C_PASSENGER_H_ */
