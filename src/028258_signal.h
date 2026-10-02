/* 8c028258 */
#ifndef _028258_SIGNAL_H
#define _028258_SIGNAL_H

#include <shinobi.h>
#include "014a9c_tasks.h" /* Task */

/* Task state for both object kinds; exactly the 0xd4 TaskSpawn_8c014ae8 asks for.
 *
 * A type 1 object is a signal head: `frame_0x0c` cycles 0->1->2->0, holding each
 * for `durations_0x08[frame]` frames, and is published to
 * var_trafficSignalFrames_8c227e24 under id_0x00, where pedestrians and CPU
 * vehicles read it to decide whether they may go. A type 2/3/4 object hangs off
 * one of those through `attachedTo_0xd0`, mirrors its open/closed state, and
 * only ever uses lamps_0x10[0].
 *
 * The 027958_bus_draw draw callbacks light exactly one entry of `lamps_0x10` per frame --
 * NJD_EVAL_HIDE cleared on it, set on the other two -- then draw `model_0xb8`
 * with `tlist_0xb4`. For a type 2/3/4 object BusDrawSignalAttachment_8c028206 reads drawA_0xc8 as
 * plain visibility rather than as a per-placement draw flag. */
typedef struct TrafficSignal {
    int id_0x00; /* indexes var_trafficSignalFrames_8c227e24 / var_trafficSignalStates_8c227e28 */
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
 * type_0x00. A type-1 entry is a signal head in its own right, running its own
 * lamp cycle; types 2/3/4 are attachments naming the type-1 entry they hang off.
 * Each rot/pos pair is only applied when its position is non-zero. */
typedef struct {
    int type_0x00;
    int id_0x04;                 /* indexes var_trafficSignalFrames_8c227e24 / ...States_8c227e28 */
    int linkedId_0x08;           /* type 1: initial counter instead */
    int rotADeg_0x0c;
    NJS_POINT3 posA_0x10;
    int rotBDeg_0x1c;
    NJS_POINT3 posB_0x20;
    int durations_0x2c[3];       /* per-frame hold times, walked by headTask_8c028258 */
} TrafficSignalDef;

extern Task *var_trafficSignalTasks_8c227e20; /* Task array for headTask_8c028258/attachmentTask_8c02833c, sized (count+1) by SignalInit_8c02845a */
extern int *var_trafficSignalFrames_8c227e24; /* per-id current frame index, read by SignalGetFrame_8c028900 */

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
