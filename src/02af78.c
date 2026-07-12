#include <shinobi.h>

#include "014f54_text_pre_data.h"
#include "0fcd20_sectionB.h"
#include "03bd80_sectionD.h"
#include "serial_debug.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

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

/* ====================
 * Functions
 * ====================
 */

/* Marks unlock flag `index` in both paired bitsets of the progress struct. */
void setProgressFlag_8c02af78(int index)
{
    int word = index >> 5;
    int mask = 1 << (index & 0x1f);

    var_progress_8c1ba1cc.field_0x04[word] |= mask;
    var_progress_8c1ba1cc.field_0x18[word] |= mask;
}

/* Tests unlock flag `index` in the field_0x04 bitset (see setProgressFlag_8c02af78). */
int hasProgressFlag_8c02afbe(int index)
{
    int word = index >> 5;
    int mask = 1 << (index & 0x1f);

    return var_progress_8c1ba1cc.field_0x04[word] & mask;
}

/* Tests unlock flag `index` in the field_0x18 bitset (see setProgressFlag_8c02af78). */
int hasProgressFlagAlt_8c02aff0(int index)
{
    int word = index >> 5;
    int mask = 1 << (index & 0x1f);

    return var_progress_8c1ba1cc.field_0x18[word] & mask;
}

/* Sets bit `index` of var_8c1ba2b4 (single-word bitset, role unclear). */
void FUN_8c02b022(int index)
{
    var_8c1ba2b4 |= 1 << (index & 0x1f);
}

/* Tests bit `index` of var_8c1ba2b4 (see FUN_8c02b022). */
int FUN_8c02b030(int index)
{
    return var_8c1ba2b4 & (1 << (index & 0x1f));
}

/* Scans the active route's UnlockEntry table for entries matching the
 * current time of day whose prerequisites are met, and collects their
 * indices into var_8c228520/var_8c228560 as unlock candidates. Skipped
 * entirely (other than resetting the candidate count) during practice
 * mode. */
void scanUnlockCandidates_8c02b03c(void)
{
    UnlockEntry *entry;
    int index;
    Uint32 dayMask;
    Uint32 conditions;

    var_8c228560 = 0;

    if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE) {
        LOG_DEBUG(("[UNLOCK] scanUnlockCandidates_8c02b03c: skipped (practice mode)\n"));
        return;
    }

    var_8c1ba2b4 = 0;

    switch (var_route_8c18ad1c) {
        case ROUTE_SHINJUKU: {
            var_8c22851c = init_8c04b1f0;
            break;
        }
        case ROUTE_WANGAN: {
            var_8c22851c = init_8c04abb0;
            break;
        }
        case ROUTE_OME: {
            var_8c22851c = init_8c04b920;
            break;
        }
    }

    for (index = 0; var_8c22851c[index].timeOfDay_0x00 != 0xffff; index++) {
        entry = &var_8c22851c[index];

        if (entry->timeOfDay_0x00 != var_timeOfDay_8c18ad20) {
            continue;
        }

        for (dayMask = entry->dayMask_0x04;
             dayMask != 0 && var_progress_8c1ba1cc.days_0x00 != (int)(dayMask & UNLOCK_DAY_MASK);
             dayMask >>= UNLOCK_DAY_BITS) {
        }

        if (dayMask == 0) {
            continue;
        }

        for (conditions = entry->conditions_0x08; conditions != 0; conditions >>= UNLOCK_CODE_BITS) {
            int code = conditions & UNLOCK_CODE_MASK;

            if (code == UNLOCK_CODE_EMPTY) {
                continue;
            }

            if ((code & UNLOCK_CODE_MODE) == UNLOCK_MODE_FORBID_PROGRESS) {
                if (hasProgressFlag_8c02afbe(code & UNLOCK_CODE_FLAG) != 0) {
                    break;
                }
                continue;
            }

            if ((code & UNLOCK_CODE_MODE) == UNLOCK_MODE_REQUIRE_PROGRESS &&
                hasProgressFlag_8c02afbe(code & UNLOCK_CODE_FLAG) == 0) {
                break;
            }
        }

        if (conditions == 0) {
            var_8c228520[var_8c228560] = index;
            var_8c228560++;
        }
    }

    LOG_DEBUG(("[UNLOCK] scanUnlockCandidates_8c02b03c: found %d candidate(s) for timeOfDay=%d\n",
               var_8c228560, var_timeOfDay_8c18ad20));
}

/* Narrows scanUnlockCandidates_8c02b03c's candidates to the ones whose
 * segmentId_0x02 matches the current segment and whose conditions_0x08
 * var_8c1ba2b4-bit checks (mode 2/3) also pass, then randomly picks one into
 * var_selectedUnlockEntry_8c228478 and arms var_cutsceneActive_8c1bb900.
 * Skipped (cutsceneActive cleared) outside normal play, during the course
 * menu (var_gameMode_8c1bb8fc), or while var_8c2285dc > var_8c2285d8. */
void pickUnlockCandidate_8c02b170(void)
{
    UnlockEntry *entry;
    int i;
    int count;
    int tableIndex;
    Uint32 conditions;
    int candidates[5];

    if (!(var_playMode_8c1bb8d0 == PLAY_MODE_NORMAL && var_gameMode_8c1bb8fc == 0 &&
          var_8c2285dc <= var_8c2285d8)) {
        var_cutsceneActive_8c1bb900 = 0;
        LOG_DEBUG(("[UNLOCK] pickUnlockCandidate_8c02b170: skipped (guard not open)\n"));
        return;
    }

    count = 0;

    for (i = 0; i < var_8c228560; i++) {
        tableIndex = var_8c228520[i];
        entry = var_8c22851c + tableIndex;

        if (var_currentSegment_8c228708 != entry->segmentId_0x02) {
            continue;
        }

        for (conditions = entry->conditions_0x08; conditions != 0; conditions >>= UNLOCK_CODE_BITS) {
            int code = conditions & UNLOCK_CODE_MASK;

            if (code == UNLOCK_CODE_EMPTY) {
                continue;
            }

            if ((code & UNLOCK_CODE_MODE) == UNLOCK_MODE_FORBID_EPHEMERAL) {
                if (FUN_8c02b030(code & UNLOCK_CODE_FLAG) != 0) {
                    break;
                }
                continue;
            }

            if ((code & UNLOCK_CODE_MODE) == UNLOCK_MODE_REQUIRE_EPHEMERAL &&
                FUN_8c02b030(code & UNLOCK_CODE_FLAG) == 0) {
                break;
            }
        }

        if (conditions == 0) {
            candidates[count] = tableIndex;
            count++;
        }
    }

    if (count == 0) {
        var_cutsceneActive_8c1bb900 = 0;
        LOG_DEBUG(("[UNLOCK] pickUnlockCandidate_8c02b170: no eligible candidates for segment=%d\n",
                   var_currentSegment_8c228708));
        return;
    }

    var_cutsceneActive_8c1bb900 = 1;
    var_selectedUnlockEntry_8c228478 = candidates[AsqGetRandomInRangeB_8c0121be(count)];
    LOG_DEBUG(("[UNLOCK] pickUnlockCandidate_8c02b170: selected entry %d from %d candidate(s)\n",
               var_selectedUnlockEntry_8c228478, count));
}

/* Applies var_selectedUnlockEntry_8c228478's actions_0x0c codes: mode clear
 * sets a progress flag, mode 0x200 sets a var_8c1ba2b4 bit. */
void applyUnlockCandidate_8c02b292(void)
{
    UnlockEntry *entry;
    Uint32 actions;

    entry = var_8c22851c + var_selectedUnlockEntry_8c228478;

    LOG_DEBUG(("[UNLOCK] applyUnlockCandidate_8c02b292: applying entry %d\n",
               var_selectedUnlockEntry_8c228478));

    for (actions = entry->actions_0x0c; actions != 0; actions >>= UNLOCK_CODE_BITS) {
        int code = actions & UNLOCK_CODE_MASK;

        if (code == UNLOCK_CODE_EMPTY) {
            continue;
        }

        if ((code & UNLOCK_ACTION_MODE) == 0) {
            setProgressFlag_8c02af78(code & UNLOCK_CODE_FLAG);
            continue;
        }

        FUN_8c02b022(code & UNLOCK_CODE_FLAG);
    }
}
