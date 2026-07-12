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

    /* packed list of 10-bit {mode:2,flag:8} codes, terminated by a 0 (or
     * padding) slot. mode 0/1 (must-not-have/must-have a progress flag) is
     * checked by scanUnlockCandidates_8c02b03c; mode 2/3 (must-not-have/
     * must-have a var_8c1ba2b4 bit) is checked by pickUnlockCandidate_8c02b170. */
    Uint32 conditions_0x08;

    /* packed list of 10-bit {mode:1,flag:8} codes applied by
     * applyUnlockCandidate_8c02b292 once this entry is chosen: mode clear
     * sets a progress flag, mode set (0x200) sets a var_8c1ba2b4 bit. */
    Uint32 actions_0x0c;
} UnlockEntry;

/* Packed 10-bit unlock codes ({mode:2,flag:8}), LSB slot first, terminated by
 * a 0x3ff (empty/padding) slot. Shared layout for conditions_0x08 (all 4
 * modes) and actions_0x0c (only bit 0x200 is meaningful there). */
#define UNLOCK_CODE_BITS  10
#define UNLOCK_CODE_MASK  0x3ff
#define UNLOCK_CODE_EMPTY 0x3ff
#define UNLOCK_CODE_FLAG  0x0ff
#define UNLOCK_CODE_MODE  0x300

#define UNLOCK_MODE_FORBID_PROGRESS   0x000 /* fail if progress flag set */
#define UNLOCK_MODE_REQUIRE_PROGRESS  0x100 /* fail if progress flag clear */
#define UNLOCK_MODE_FORBID_EPHEMERAL  0x200 /* fail if var_8c1ba2b4 bit set */
#define UNLOCK_MODE_REQUIRE_EPHEMERAL 0x300 /* fail if var_8c1ba2b4 bit clear */

/* actions_0x0c reuses the same 10-bit slot layout but only a 1-bit mode
 * (bit 0x200): clear sets a progress flag, set sets a var_8c1ba2b4 bit. */
#define UNLOCK_ACTION_MODE 0x200

/* Packed 5-bit day-value slots in dayMask_0x04, terminated by a 0 slot. */
#define UNLOCK_DAY_BITS 5
#define UNLOCK_DAY_MASK 0x1f

void setProgressFlag_8c02af78(int index);
int hasProgressFlag_8c02afbe(int index);
int hasProgressFlagAlt_8c02aff0(int index);
void FUN_8c02b022(int index);
int FUN_8c02b030(int index);
void scanUnlockCandidates_8c02b03c(void);
void pickUnlockCandidate_8c02b170(void);
void applyUnlockCandidate_8c02b292(void);

#endif // _02AF78_H
