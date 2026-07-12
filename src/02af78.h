#ifndef _02AF78_H
#define _02AF78_H

#include <shinobi.h>

/* =================
 * Type Declarations
 * =================
 */

/* course/segment unlock rule, 16 bytes, arrays terminated by timeOfDay_0x00 ==
 * 0xffff. dayMask_0x04 and conditions_0x08 each pack a list of 5/10-bit codes
 * (see scanUnlockCandidates_8c02b03c). */
typedef struct {
    Uint16 timeOfDay_0x00;
    Uint16 segmentId_0x02;
    Uint32 dayMask_0x04;
    Uint32 conditions_0x08;
    Uint32 field_0x0c;
} UnlockEntry;

void setProgressFlag_8c02af78(int index);
int hasProgressFlag_8c02afbe(int index);
int hasProgressFlagAlt_8c02aff0(int index);
void FUN_8c02b022(int index);
int FUN_8c02b030(int index);
void scanUnlockCandidates_8c02b03c(void);
void FUN_8c02b170(void);

#endif // _02AF78_H
