/* Bus stops: which segments have one, the passengers waiting at them, and the
 * arrival/departure state machine. */
#ifndef _02C884_STOP_H
#define _02C884_STOP_H

#include "013ae8_route.h" /* CourseSegment */

/* A point and direction on the course: where the bus pulls up (lineBus_0x08)
 * or the strip waiting passengers line up along (lineHum_0x2c). */
typedef struct {
    void *ukn_0x00;
    float x_0x04;
    float z_0x08;
    float dx_0x0c;
    float dz_0x10;
} StopAreaRecord;

void StopKillTaskGroup_8c02ca96(void);

/* Per-run: decides which segments get a stop, primes the first one and
 * resets driver points and the run clock. */
void StopSetup_8c02caba(void);

/* Makes the next stop's segment current and finds the stop after it. */
void StopAdvance_8c02ccc6(void);

CourseSegment *StopGetSegment_8c02cd6a(int segmentIndex);

StopAreaRecord *StopGetArea_8c02cd7a(int segmentIndex);

/* Steps RunState.stopPhase_0x20 once per frame. */
void StopUpdateArrival_8c02ce48(void);

/* Segment the run starts from: 0, or a debug menu entry's to start partway. */
extern int var_startStopIndex_8c228704;

extern int var_currentSegment_8c228708;

extern int var_prevStopSegment_8c22870c;
extern int var_nextStopSegment_8c228710;

/* Sign-extended from 16 bits. */
extern int var_nextStopHeading_8c228714;

/* The passenger in each bus seat, as their stop-candidate entry; -1 = empty.
 * Indexes init_seatPositions_8c04c3e4. */
extern int var_seats_8c228718[31];

extern int var_waitingPassengerCount_8c228794;

typedef struct {
    /* An entry of CourseSegment.stopCandidates_0x08: {pedestrian type,
     * destination segment} bytes. */
    void *spot_0x00;
    NJS_POINT3 pos_0x04;
    float index_0x10;
} WaitingPassengerSlot;
extern WaitingPassengerSlot var_waitingPassengers_8c228798[16];

/* Shared by every passenger draw; p and tlist are set per draw. */
extern NJS_SPRITE var_passengerSprite_8c2288d8;

/* -1 when not allocated. */
extern void* var_stopTaskGroup_8c2288f8;

#endif // _02C884_STOP_H
