/* @unit DriveMsg */

#include <shinobi.h>
#include "includes.h" /* STATIC */

#include "02b2f0_drive_msg.h"
#include "014f54_sprite.h"
#include "02b464_grading.h"
#include "1ba1c8_globals.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

/* Banner glyphs and 0129cc_game.c's MARK_* sprites share
 * var_markTexlist_8c1bc418: a 16x16 grid of 32x32 cells, an id's low nibble
 * picking the column and the high nibble the row. What 0x78 depicts is
 * unverified. */
#define GLYPH_SIZE        32.0f
#define ATLAS_CELLS       16
#define CELL_UV           0.0625f
#define GLYPH_Z           0.82644623f
#define MARK_RUN_PASSED   0x78
#define MARK_Z_RUN_PASSED -1.16f

#define MSG_SLOTS ((int)(sizeof(var_driveMsgQueue_8c228564) / \
                         sizeof(var_driveMsgQueue_8c228564[0])))

/* Newest slot's top edge; each older one sits a row above. */
#define FIRST_ROW_Y 192.0f

/* ====================
 * Non-initialized Globals
 * ====================
 */

DriveMsgSlot var_driveMsgQueue_8c228564[4];

/* ====================
 * Functions
 * ====================
 */

STATIC void drawMsgGlyphRow_8c02b2f0(Uint32 *ids, int count, float x, float y)
{
    NJS_QUAD_TEXTURE q;
    int i;

    q.y1 = y;
    q.y2 = y + GLYPH_SIZE;

    for (i = 0; i < count; i++) {
        Uint32 id = ids[i];
        float col, row;

        q.x1 = x;
        x += GLYPH_SIZE;
        q.x2 = x;

        col = (float)(id & 0xf);
        q.u1 = col / ATLAS_CELLS;
        q.u2 = q.u1 + CELL_UV;

        /* Masked, not shifted, so the row still carries its factor of 16. */
        row = (float)(id & 0xfffffff0);
        q.v1 = row / (ATLAS_CELLS * ATLAS_CELLS);
        q.v2 = q.v1 + CELL_UV;

        njDrawQuadTexture(&q, GLYPH_Z);
    }
}

void DriveMsgDraw_8c02b388(int unused)
{
    int i;
    float y;

    if (var_runState_8c2285c4.runPassed_0x04 != 0) {
        /* &var_markTexlist_8c1bc418 is the mark ResourceGroup's own address,
         * not a cast of its value -- see 1ba1c8_globals.h. */
        SpriteDraw_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418,
                               MARK_RUN_PASSED, 0.0f, 0.0f, MARK_Z_RUN_PASSED);
        return;
    }

    njSetTexture(var_markTexlist_8c1bc418);
    njQuadTextureStart(1);
    njSetQuadTextureG(0x0a8d, 0xffffffff);

    y = FIRST_ROW_Y;
    for (i = 0; i < MSG_SLOTS; i++) {
        DriveMsgSlot *slot = &var_driveMsgQueue_8c228564[i];
        if (slot->holdFrames != 0) {
            drawMsgGlyphRow_8c02b2f0((Uint32 *)slot->ids, slot->revealed,
                                     slot->x, y);
        }
        y -= GLYPH_SIZE;
    }

    njQuadTextureEnd();
}
