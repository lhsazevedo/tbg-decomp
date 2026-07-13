#ifndef _EVENT_H
#define _EVENT_H

#include <shinobi.h>

/* =================
 * Type Declarations
 * =================
 */

typedef struct {
    Uint16 timeOfDay_0x00;
    Uint16 segmentId_0x02;

    /*
     * Six 5-bit day slots, stored least-significant slot first.
     *
     * Day 0 uses bits 0-4. Day 5 uses bits 25-29.
     * Zero-valued trailing slots are omitted and terminate the scan.
     */
    Uint32 dayMask_0x04;

    /*
     * Three 10-bit code slots, stored least-significant slot first.
     *
     * Slot 0 uses bits 0-9. Slot 2 uses bits 20-29.
     * Bits 30-31 are unused.
     *
     * Each slot contains an 8-bit flag and a 2-bit mode.
     * A value of 0x3ff represents UNLOCK_NONE.
     */
    Uint32 conditions_0x08;
    Uint32 actions_0x0c;
} EventEntry;

int hasProgressFlagAlt_8c02aff0(int index);
void scanEventCandidates_8c02b03c(void);
void pickSegmentEvent_8c02b170(void);
void applyEventFlags_8c02b292(void);

#endif // _EVENT_H
