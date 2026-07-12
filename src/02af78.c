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
