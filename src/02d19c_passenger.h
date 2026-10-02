#include "shinobi.h"

#ifndef _02D19C_PASSENGER_H_
#define _02D19C_PASSENGER_H_

#include "014a9c_tasks.h"

/* TaskSpawn_8c014ae8 state, exactly the 0x38 bytes all three spawn paths
 * (PassengerBoardTask_8c02d21c, PassengerSeatedTask_8c02d5ca, PassengerExitTask_8c02d46c) ask for. The three
 * callbacks agree on every field's type. */
typedef struct {
    void *entry_0x00;
    Uint32 state_0x04;
    NJS_POINT3 pos_0x08;
    Uint32 spriteNo_0x14;
    float jitterX_0x18;
    float jitterZ_0x1c;
    Sint32 delay_0x20;
    Sint32 slotIndex_0x24;
    Uint32 isSeated_0x28;
    Uint32 voice_0x2c;
    Uint32 soundIdA_0x30;
    Uint32 soundIdB_0x34;
} StopScheduleState;

/* Per-scripted-stop schedule table, 31 (x, z) seat positions indexed by the
 * var_seats_8c228718 slot index. */
typedef struct {
    float x;
    float z;
} SeatPos;
extern SeatPos init_seatPositions_8c04c3e4[31];

/* TaskSpawn_8c014ae8 state for PassengerStopSceneTask_8c02d644, exactly the
 * 8 bytes StopSpawnInit_8c02d968 asks for -- a private 2-int layout distinct
 * from StopScheduleState above. */
typedef struct {
    int phase_0x00;
    int subPhase_0x04;
} PassengerStopSceneState;

/* Task action for a scripted-stop slot whose segment differs from the bus's
 * current one -- spawned immediately (no shuffle) by StopSpawnInit_8c02d968. */
void PassengerSeatedTask_8c02d5ca(Task *task, void *state);

/* Task action for an already-picked waiting passenger (state->state_0x04 == 1
 * always, from 02d968); handles positioning, the stop-bell sound and
 * despawn. */
void PassengerBoardTask_8c02d21c(Task *task, StopScheduleState *state);

/* Task action for a scripted-stop slot that matches the bus's current
 * segment (spawned after the Fisher-Yates shuffle). */
void PassengerExitTask_8c02d46c(Task *task, StopScheduleState *state);

/* Per-frame countdown/animation task action shared by every waiting-
 * passenger spawn path once positioned; drives the wave/board animation and
 * frees itself when done. */
void PassengerStopSceneTask_8c02d644(Task *task, PassengerStopSceneState *state);

/* Task action spawned instead of the normal per-passenger tasks when
 * var_playMode_8c1bb8d0 == 1 (VM/replay mode) and course-restart flag
 * var_practiceRules_8c226410 bit 3 is clear -- resets state on first run. */
void PassengerSkipStopTask_8c02d8f0(Task *task, void *state);

/* Six consecutive NJS_POINT3 waypoints (0x228910-0x22894c, 12 bytes apart),
 * filled by StopSpawnInit_8c02d968 at course start and stepped through by
 * 02d19c's passenger tasks, three per sequence in the numbered order. Passengers do
 * not walk between them: each step happens on the one frame
 * var_passengersFadedOut_8c22895c is set, so the sprite vanishes at one spot and
 * reappears at the next.
 *
 * Named by position in the sequence rather than by what is there, because
 * what is there moves: StopSpawnInit_8c02d968 fills the six slots from the
 * same six coordinates in REVERSED order on Ome, swapping which door each
 * sequence uses. (0.15, 0.68, 0.27) is hard against the driver -- the fare
 * box -- and lands in boardSpot2 on flat-fare Shinjuku/Wangan but in
 * exitSpot2 on distance-fare Ome, which is also why 02d19c gates the
 * boarding voice lines on non-Ome and the exiting ones on Ome: the
 * greeting plays wherever the passenger passes the driver.
 *
 * Geometry, against init_seatPositions_8c04c3e4: the +x wall has no seat
 * slot between z = -0.33 and 3.95, and every waypoint falls in that gap, in
 * two z clusters (~0.3-0.9 and ~2.6-3.0) -- two doors. (1.0, 0.35, 2.8) is
 * the only point at half floor height and sits alone in the gap: the
 * doorwell step.
 *
 * Open: the two y=0 points, (-1.6, 0, 0.9) and (-1.39, 0, 2.63), are on -x,
 * the side with the unbroken seat row, though their z matches the doors.
 * They are also the only two rewritten in place by njCalcPoint against
 * var_busState_8c1bb9d0.worldMatrix_0x084, and StopSpawnInit_8c02d968 runs once at
 * course start -- so that transform, not the coordinate, is the thing to
 * check. */
extern NJS_POINT3 var_boardSpot2_8c228910;
extern NJS_POINT3 var_boardSpot3_8c22891c;
extern NJS_POINT3 var_boardSpot1_8c228928;
extern NJS_POINT3 var_exitSpot1_8c228934;
extern NJS_POINT3 var_exitSpot2_8c228940;
extern NJS_POINT3 var_exitSpot3_8c22894c;

/* Nonzero for the single frame the passenger fade reaches full black.
 * Passenger tasks only advance a step while it is set, so they move unseen. */
extern int var_passengersFadedOut_8c22895c;

/* Constant material for the pass the passenger sprites draw in, passed to
 * njSetConstantMaterial by StopDrawLightBegin_8c02d0fc (02d06c). [0] is the
 * alpha 02d19c's scene task fades in and out at 1/15 per frame, [1..3] the
 * rgb, held at 1.0. [4] is past the NJS_ARGB and is not part of the color.
 * The interior model itself sets no constant material. */
extern float var_passengerFadeColor_8c228960[5];

#endif /* _02D19C_PASSENGER_H_ */
