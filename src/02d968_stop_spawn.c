/* @unit StopSpawn */

#include <shinobi.h>
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "013ae8_route.h"
#include "011120_asset_queues.h"
#include "015034_text.h"
#include "1ba1c8_globals.h"
#include "01e27c_practice_menu.h"
#include "014a9c_tasks.h"
#include "02d19c_passenger.h"
#include "025870_cinematic.h"
#include "02c884_stop.h"
#include "02b464_grading.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define SEAT_COUNT 31

/* ====================
 * Type Declarations
 * ====================
 */

typedef struct {
    void *spot_0x00;
    int seat_0x04;
} ExitingSeat;

/* ====================
 * Initialized Globals
 * ====================
 */

/* Per pedestrian type; added to 0x32 (boarding) or 0x36 (exiting). */
STATIC Uint8 init_passengerVoiceVariant_8c04c4dc[] = {
    0x03, 0x03, 0x02, 0x01, 0x00, 0x00, 0x02, 0x01,
    0x02, 0x01, 0x00, 0x03, 0x02, 0x00, 0x00, 0x00,
    0x02, 0x03, 0x01, 0x00, 0x01, 0x01, 0x02, 0x00,
    0x01, 0x03, 0x03, 0x03, 0x03, 0x02, 0x02, 0x00,
    0x00, 0x02, 0x02, 0x03, 0x02, 0x02, 0x01, 0x02,
    0x02, 0x03, 0x03, 0x03, 0x01, 0x01, 0x02, 0x01,
    0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01,
    0x03, 0x00, 0x00, 0x00,
};

/* ====================
 * Functions
 * ====================
 */

void StopSpawnInit_8c02d968(void)
{
    Task *task;
    PassengerState *state;
    PassengerStopSceneState *rawState;
    int i, n, count, midiSlot, randIdx;
    void *spot;
    void *matchedBuf;
    ExitingSeat *matched;
    ExitingSeat tmp;

    if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE && (var_practiceRules_8c226410 & 8) != 8) {
        TaskInstall_8c014ae8(var_tasks_8c1ba5e8, &PassengerSkipStopTask_8c02d8f0, &task, (void **)&state, 0);
        var_runState_8c2285c4.runPhase_0x00 = 2;
        return;
    }

    TaskInstall_8c014ae8(var_tasks_8c1ba5e8, &PassengerStopSceneTask_8c02d644, &task, (void **)&rawState, 8);
    rawState->phase_0x00 = 0;
    rawState->subPhase_0x04 = 0;

    njSetTexture(var_interiorTexlist_8c1bc438);
    njLoadCacheTexture(var_interiorTexlist_8c1bc438);
    CinematicBoardingCamera_8c025870();

    if (var_route_8c18ad1c == ROUTE_SHINJUKU || var_route_8c18ad1c == ROUTE_WANGAN) {
        var_boardSpot1_8c228928.x = -1.6f;
        var_boardSpot1_8c228928.y = 0.0f;
        var_boardSpot1_8c228928.z = 0.9f;
        njCalcPoint(&var_busState_8c1bb9d0.worldMatrix_0x084, &var_boardSpot1_8c228928, &var_boardSpot1_8c228928);

        var_boardSpot2_8c228910.x = 0.15f;
        var_boardSpot2_8c228910.y = 0.68f;
        var_boardSpot2_8c228910.z = 0.27f;

        var_boardSpot3_8c22891c.x = 0.128f;
        var_boardSpot3_8c22891c.y = 0.68f;
        var_boardSpot3_8c22891c.z = 0.76f;

        var_exitSpot1_8c228934.x = 0.35f;
        var_exitSpot1_8c228934.y = 0.68f;
        var_exitSpot1_8c228934.z = 3.0f;

        var_exitSpot2_8c228940.x = 1.0f;
        var_exitSpot2_8c228940.y = 0.35f;
        var_exitSpot2_8c228940.z = 2.8f;

        var_exitSpot3_8c22894c.x = -1.39f;
        var_exitSpot3_8c22894c.y = 0.0f;
        var_exitSpot3_8c22894c.z = 2.63f;
        njCalcPoint(&var_busState_8c1bb9d0.worldMatrix_0x084, &var_exitSpot3_8c22894c, &var_exitSpot3_8c22894c);
    } else if (var_route_8c18ad1c == ROUTE_OME) {
        var_boardSpot1_8c228928.x = -1.39f;
        var_boardSpot1_8c228928.y = 0.0f;
        var_boardSpot1_8c228928.z = 2.63f;
        njCalcPoint(&var_busState_8c1bb9d0.worldMatrix_0x084, &var_boardSpot1_8c228928, &var_boardSpot1_8c228928);

        var_boardSpot2_8c228910.x = 1.0f;
        var_boardSpot2_8c228910.y = 0.35f;
        var_boardSpot2_8c228910.z = 2.8f;

        var_boardSpot3_8c22891c.x = 0.35f;
        var_boardSpot3_8c22891c.y = 0.68f;
        var_boardSpot3_8c22891c.z = 3.0f;

        var_exitSpot1_8c228934.x = 0.128f;
        var_exitSpot1_8c228934.y = 0.68f;
        var_exitSpot1_8c228934.z = 0.76f;

        var_exitSpot2_8c228940.x = 0.15f;
        var_exitSpot2_8c228940.y = 0.68f;
        var_exitSpot2_8c228940.z = 0.27f;

        var_exitSpot3_8c22894c.x = -1.6f;
        var_exitSpot3_8c22894c.y = 0.0f;
        var_exitSpot3_8c22894c.z = 0.9f;
        njCalcPoint(&var_busState_8c1bb9d0.worldMatrix_0x084, &var_exitSpot3_8c22894c, &var_exitSpot3_8c22894c);
    }

    var_passengersFadedOut_8c22895c = 0;
    var_passengerFadeColor_8c228960[0] = 0.0f;
    var_passengerFadeColor_8c228960[1] = 1.0f;
    var_passengerFadeColor_8c228960[2] = 1.0f;
    var_passengerFadeColor_8c228960[3] = 1.0f;

    n = 0;
    for (i = 0; i < SEAT_COUNT; i++) {
        if (var_seats_8c228718[i] != -1) {
            n++;
        }
    }

    var_stopTaskGroup_8c2288f8 = (void *)syMalloc((var_waitingPassengerCount_8c228794 + n + 1) * sizeof(Task));
    TaskInitGroup_8c014a9c(var_stopTaskGroup_8c2288f8, var_waitingPassengerCount_8c228794 + n);

    for (i = 0; i < var_waitingPassengerCount_8c228794; i++) {
        TaskInstall_8c014ae8((Task *)var_stopTaskGroup_8c2288f8, &PassengerBoardTask_8c02d21c, &task,
                           (void **)&state, sizeof(PassengerState));
        state->spot_0x00 = var_waitingPassengers_8c228798[i].spot_0x00;
        state->state_0x04 = 1;
        state->pos_0x08 = var_waitingPassengers_8c228798[i].pos_0x04;
        state->jitterX_0x18 = ((float)rand() / 32768.0f) * 0.2f;
        state->jitterZ_0x1c = ((float)rand() / 32768.0f) * 0.2f;
        state->delay_0x20 = i;
        state->isSeated_0x28 = 0;
        midiSlot = i % 4;
        state->voice_0x2c = midiSlot;
        state->soundIdA_0x30 = init_passengerVoiceVariant_8c04c4dc[*(char *)state->spot_0x00] + 0x32;
        state->soundIdB_0x34 = state->soundIdA_0x30 + 4;
        var_passengerCount_8c1bb8e4++;
    }

    /* Passengers whose destination is this segment get off, in random order. */
    matchedBuf = (void *)syMalloc(SEAT_COUNT * sizeof(ExitingSeat));
    matched = (ExitingSeat *)matchedBuf;
    count = 0;
    for (i = 0; i < SEAT_COUNT; i++) {
        if (var_seats_8c228718[i] == -1) {
            continue;
        }
        spot = (void *)var_seats_8c228718[i];
        if (*((char *)spot + 1) == (char)var_currentSegment_8c228708) {
            matched[count].spot_0x00 = spot;
            matched[count].seat_0x04 = i;
            count++;
        } else {
            TaskInstall_8c014ae8((Task *)var_stopTaskGroup_8c2288f8, &PassengerSeatedTask_8c02d5ca, &task,
                               (void **)&state, sizeof(PassengerState));
            state->spot_0x00 = spot;
            state->pos_0x08.x = init_seatPositions_8c04c3e4[i].x;
            state->pos_0x08.y = var_exitSpot1_8c228934.y;
            state->pos_0x08.z = init_seatPositions_8c04c3e4[i].z;
            if (i < 0x14) {
                state->spriteNo_0x14 = 0x20;
                state->pos_0x08.y -= 0.18f;
            } else if (i < 0x1a) {
                state->spriteNo_0x14 = 0x22;
            } else {
                state->spriteNo_0x14 = 0x23;
            }
            state->isSeated_0x28 = 1;
        }
    }

    for (i = 0; i < count; i++) {
        randIdx = AsqGetRandomInRangeA_8c012178(count);
        tmp = matched[i];
        matched[i] = matched[randIdx];
        matched[randIdx] = tmp;
    }

    /* Drawn at the i-th seat, not their own; seat_0x24 is the one to free. */
    for (i = 0; i < count; i++) {
        TaskInstall_8c014ae8((Task *)var_stopTaskGroup_8c2288f8, &PassengerExitTask_8c02d46c, &task,
                           (void **)&state, sizeof(PassengerState));
        state->spot_0x00 = matched[i].spot_0x00;
        state->state_0x04 = 0;
        state->pos_0x08.x = init_seatPositions_8c04c3e4[i].x;
        state->pos_0x08.y = var_exitSpot1_8c228934.y;
        state->pos_0x08.z = init_seatPositions_8c04c3e4[i].z;
        if (i < 0x14) {
            state->spriteNo_0x14 = 0x20;
            state->pos_0x08.y -= 0.18f;
        } else if (i < 0x1a) {
            state->spriteNo_0x14 = 0x22;
        } else {
            state->spriteNo_0x14 = 0x23;
        }
        state->jitterX_0x18 = ((float)rand() / 32768.0f) * 0.2f;
        state->jitterZ_0x1c = ((float)rand() / 32768.0f) * 0.2f;
        state->delay_0x20 = i;
        state->seat_0x24 = matched[i].seat_0x04;
        state->isSeated_0x28 = 1;
        midiSlot = i % 4;
        state->voice_0x2c = midiSlot;
        state->soundIdA_0x30 = init_passengerVoiceVariant_8c04c4dc[*(char *)state->spot_0x00] + 0x36;
        state->soundIdB_0x34 = state->soundIdA_0x30 - 8;
    }

    syFree(matchedBuf);
}
