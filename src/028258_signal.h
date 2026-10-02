/* 8c028258 */
#ifndef _028258_SIGNAL_H
#define _028258_SIGNAL_H

#include <shinobi.h>
#include "014a9c_tasks.h" /* Task */

/* Task state of a signal head (type 1) or an attachment (types 2/3/4).
 * A head's frame_0x0c cycles 0->1->2->0, holding each for
 * durations_0x08[frame] frames, and is published per id to
 * var_trafficSignalFrames_8c227e24 for pedestrians and CPU vehicles. An
 * attachment hangs off a head through attachedTo_0xd0, has only lamps_0x10[0],
 * and uses drawA_0xc8 as that lamp's visibility. */
typedef struct TrafficSignal {
    int id_0x00;
    int counter_0x04;
    int *durations_0x08;
    int frame_0x0c;
    NJS_OBJECT *lamps_0x10[3];
    NJS_POINT3 posA_0x1c;
    NJS_POINT3 posB_0x28;
    NJS_MATRIX mtxA_0x34;
    NJS_MATRIX mtxB_0x74;
    NJS_TEXLIST *tlist_0xb4;
    NJS_OBJECT *model_0xb8;
    struct TrafficSignal *attached_0xbc[3]; /* one per type 2/3/4 */
    int drawA_0xc8;
    int drawB_0xcc;
    struct TrafficSignal *attachedTo_0xd0;
} TrafficSignal;

/* One entry of the course's macSignal_0x38 table, terminated by a zero
 * type_0x00. Types as in TrafficSignal. A rot/pos pair applies only when its
 * position is non-zero. */
typedef struct {
    int type_0x00;
    int id_0x04;
    int linkedId_0x08;           /* type 1: initial counter instead */
    int rotADeg_0x0c;
    NJS_POINT3 posA_0x10;
    int rotBDeg_0x1c;
    NJS_POINT3 posB_0x20;
    int durations_0x2c[3];
} TrafficSignalDef;

extern Task *var_trafficSignalTasks_8c227e20;
extern int *var_trafficSignalFrames_8c227e24; /* per-id frame_0x0c */

void SignalInit_8c02845a(void);
TrafficSignal *SignalGet_8c0288b2(int index);
void SignalFree_8c0288be(void);
int SignalGetFrame_8c028900(int index);
void SignalClearPedCrossing_8c02890c(void);
void SignalClearCrossingOccupied_8c028958(void);
void SignalMarkPedCrossing_8c02897a(int index);
void SignalMarkCrossingOccupied_8c028984(int index);
int SignalIsCrossingOccupied_8c02898e(int index);
int SignalIsPedCrossing_8c028998(int index);

#endif /* _028258_SIGNAL_H */
