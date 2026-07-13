#ifndef _02AF78_H
#define _02AF78_H

#include <shinobi.h>

/* =================
 * Type Declarations
 * =================
 */

/* course/segment unlock rule, 16 bytes, arrays terminated by timeOfDay_0x00 ==
 * 0xffff. dayMask_0x04 and conditions_0x08 each pack a list of 5/10-bit codes
 * (see scanEventCandidates_8c02b03c). */
typedef struct {
    Uint16 timeOfDay_0x00;
    Uint16 segmentId_0x02;
    Uint32 dayMask_0x04;

    /* packed list of 10-bit {mode:2,flag:8} codes, terminated by a 0 (or
     * padding) slot. mode 0/1 (must-not-have/must-have a progress flag) is
     * checked by scanEventCandidates_8c02b03c; mode 2/3 (must-not-have/
     * must-have a var_runEventFlags_8c1ba2b4 bit) is checked by pickSegmentEvent_8c02b170. */
    Uint32 conditions_0x08;

    /* packed list of 10-bit {mode:1,flag:8} codes applied by
     * applyEventFlags_8c02b292 once this entry is chosen: mode clear
     * sets a progress flag, mode set (0x200) sets a var_runEventFlags_8c1ba2b4 bit. */
    Uint32 actions_0x0c;
} EventEntry;

void setProgressFlag_8c02af78(int index);
int hasProgressFlag_8c02afbe(int index);
int hasProgressFlagAlt_8c02aff0(int index);
void setRunEventFlag_8c02b022(int index);
int hasRunEventFlag_8c02b030(int index);
void scanEventCandidates_8c02b03c(void);
void pickSegmentEvent_8c02b170(void);
void applyEventFlags_8c02b292(void);

#endif // _02AF78_H
