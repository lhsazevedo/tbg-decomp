#include <shinobi.h>

#include "014f54_text_pre_data.h"
#include "0fcd20_sectionB.h"
#include "03bd80_sectionD.h"

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
        return;
    }

    var_8c1ba2b4 = 0;

    if (var_route_8c18ad1c == ROUTE_SHINJUKU) {
        var_8c22851c = init_8c04b1f0;
    } else if (var_route_8c18ad1c == ROUTE_WANGAN) {
        var_8c22851c = init_8c04abb0;
    } else if (var_route_8c18ad1c == ROUTE_OME) {
        var_8c22851c = init_8c04b920;
    }

    index = 0;
    for (entry = var_8c22851c; entry->timeOfDay_0x00 != 0xffff; entry++) {
        if (entry->timeOfDay_0x00 == var_timeOfDay_8c18ad20) {
            for (dayMask = entry->dayMask_0x04;
                 dayMask != 0 && var_progress_8c1ba1cc.days_0x00 != (int)(dayMask & 0x1f);
                 dayMask >>= 5) {
            }

            if (dayMask != 0) {
                for (conditions = entry->conditions_0x08; conditions != 0; conditions >>= 10) {
                    if ((conditions & 0x3ff) != 0x3ff) {
                        if ((conditions & 0x300) == 0) {
                            if (hasProgressFlag_8c02afbe(conditions & 0xff) != 0) {
                                // coverage:ignore-next-line -- see docs/lessons_learned.md
                                break;
                            }
                        } else if ((conditions & 0x300) == 0x100 &&
                                   hasProgressFlag_8c02afbe(conditions & 0xff) == 0) {
                            break;
                        }
                    }
                }

                if (conditions == 0) {
                    var_8c228520[var_8c228560] = index;
                    var_8c228560++;
                }
            }
        }
        index++;
    }
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

    if (var_playMode_8c1bb8d0 == PLAY_MODE_NORMAL && var_gameMode_8c1bb8fc == 0 &&
        var_8c2285dc <= var_8c2285d8) {
        count = 0;

        for (i = 0; i < var_8c228560; i++) {
            tableIndex = var_8c228520[i];
            entry = var_8c22851c + tableIndex;

            if (var_currentSegment_8c228708 == entry->segmentId_0x02) {
                for (conditions = entry->conditions_0x08; conditions != 0; conditions >>= 10) {
                    if ((conditions & 0x3ff) != 0x3ff) {
                        if ((conditions & 0x300) == 0x200) {
                            if (FUN_8c02b030(conditions & 0xff) != 0) {
                                // coverage:ignore-next-line -- see docs/lessons_learned.md
                                break;
                            }
                        } else if ((conditions & 0x300) == 0x300 &&
                                   FUN_8c02b030(conditions & 0xff) == 0) {
                            break;
                        }
                    }
                }

                if (conditions == 0) {
                    candidates[count] = tableIndex;
                    count++;
                }
            }
        }

        if (count != 0) {
            var_cutsceneActive_8c1bb900 = 1;
            var_selectedUnlockEntry_8c228478 = candidates[AsqGetRandomInRangeB_8c0121be(count)];
            return;
        }
    }

    var_cutsceneActive_8c1bb900 = 0;
}

/* Applies var_selectedUnlockEntry_8c228478's actions_0x0c codes: mode clear
 * sets a progress flag, mode 0x200 sets a var_8c1ba2b4 bit. */
void applyUnlockCandidate_8c02b292(void)
{
    UnlockEntry *entry;
    Uint32 actions;

    entry = var_8c22851c + var_selectedUnlockEntry_8c228478;

    for (actions = entry->actions_0x0c; actions != 0; actions >>= 10) {
        if ((actions & 0x3ff) != 0x3ff) {
            if ((actions & 0x200) == 0) {
                setProgressFlag_8c02af78(actions & 0xff);
            } else {
                FUN_8c02b022(actions & 0xff);
            }
        }
    }
}
