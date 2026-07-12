#include <shinobi.h>

#include "014f54_text_pre_data.h"

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
