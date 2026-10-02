/* Bus stops: which segments have one, the passengers waiting at them, and the
 * arrival/departure state machine. */
#ifndef _02C884_STOP_H
#define _02C884_STOP_H

#include "013ae8_route.h" /* CourseSegment */

/* A stop's spawn strip: origin (x_0x04, z_0x08) and direction (dx_0x0c,
 * dz_0x10) that waiting passengers are laid out along. */
typedef struct {
    void *ukn_0x00;
    float x_0x04;
    float z_0x08;
    float dx_0x0c;
    float dz_0x10;
} StopAreaRecord;

/* Frees the bus-stop task group and marks it unallocated. */
void StopKillTaskGroup_8c02ca96(void);

/* Per-run setup, called once the course has loaded: decides which segments
 * get a stop, then primes the first one and its waiting passengers. */
void StopSetup_8c02caba(void);

/* Locks in the current stop's heading, then finds the next segment with a
 * stop and primes its position and heading. */
void StopAdvance_8c02ccc6(void);

CourseSegment *StopGetSegment_8c02cd6a(int segmentIndex);

/* Returns the stop-area record for segmentIndex's segment
 * (the course's lineBus_0x08 table, selected by the segment's stopAreaId_0x02). */
StopAreaRecord *StopGetArea_8c02cd7a(int segmentIndex);

/* Per-frame arrival state machine, driven by 02b464_grading. Phases run
 * 0 cruising -> 2 approach -> 3 stopped -> 4 finishing -> 1 post-departure ->
 * 0; this function never enters 3 or 4 on its own, 02b464 does. See the
 * function for what each phase watches. */
void StopUpdateArrival_8c02ce48(void);

/* index of the stop the run starts from: 0 for a normal course start, or the
 * debug menu's per-entry startStopIndex_0x04 to begin partway along the route */
extern int var_startStopIndex_8c228704;

extern int var_currentSegment_8c228708;

extern int var_prevStopSegment_8c22870c; // segment index of the previous stop
extern int var_nextStopSegment_8c228710; // segment index of the upcoming stop

/* upcoming stop's heading angle (njArcTan2 of its stop-area record's
 * direction vector, see NinjaApi.h), sign-extended from the low 16 bits by
 * StopAdvance_8c02ccc6 */
extern int var_nextStopHeading_8c228714;

/* fixed 31-slot table of scripted/special waiting-passenger schedule entries,
 * one per fixed stop position along the route; -1 = unused. Read by
 * StopSpawnInit_8c02d968 to spawn each slot's passenger task. */
extern int var_seats_8c228718[31];

/* number of slots filled in var_waitingPassengers_8c228798 by pickWaitingPassengers_8c02c8ae
 * (0-16); read by 02d06c/02d968 to spawn that many passenger tasks. */
extern int var_waitingPassengerCount_8c228794;

/* one waiting passenger picked for the upcoming stop by
 * pickWaitingPassengers_8c02c8ae */
typedef struct {
    void *spot_0x00;  // chosen candidate-stop entry (segment record's list at +8)
    NJS_POINT3 pos_0x04; // world position; y filled in by the ground snap
    float index_0x10; // 0, 1, 2, ... in pick order
} WaitingPassengerSlot;
extern WaitingPassengerSlot var_waitingPassengers_8c228798[16];

/* The one NJS_SPRITE every passenger is drawn through, waiting at the stop
 * (02d06c) or inside the bus (02d19c). Only sx/sy/ang/tanim are reset up
 * front; p and tlist are set per draw. */
extern NJS_SPRITE var_passengerSprite_8c2288d8;

/* Task group for the bus-stop subsystem's waiting-passenger/departure tasks
 * (see StopSpawnInit_8c02d968); -1 means not currently allocated. */
extern void* var_stopTaskGroup_8c2288f8;

#endif // _02C884_STOP_H
