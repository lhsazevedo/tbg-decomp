/* @unit Sprite */
/* 8c014f54 */
#include <shinobi.h>
#include "014f54_sprite.h"
#include "includes.h" /* STATIC */

/* ====================
 * Functions
 * ====================
 */

void SpriteDraw_8c014f54(
    ResourceGroup *resource_group,
    int texture_id,
    float x,
    float y,
    float priority
) {
    ResourceGroupSpriteEntry *sprite_entry;
    int *dat_section_base;
    int i;
    NJS_SPRITE sprite;

    /* 2000 means the font group, whose contents_0x08 is the sprite entry
     * itself rather than a table of offsets to one. */
    if (texture_id == 2000) {
        sprite_entry =
            (ResourceGroupSpriteEntry *) resource_group->contents_0x08;
    } else {
        int *offset_table = resource_group->contents_0x08;
        int texture_offset = offset_table[texture_id];
        sprite_entry =
            (ResourceGroupSpriteEntry *)
            &((int *) resource_group->contents_0x08)[texture_offset];
    }

    sprite.tlist = resource_group->tlist_0x00;
    sprite.tanim = resource_group->tanim_0x04;
    sprite.ang = 0;
    sprite.sx = 1.0f;
    sprite.sy = 1.0f;

    for (i = 0; sprite_entry[i].sprite_no_0x00 != -1; i++) {
        sprite.p.x = x + sprite_entry[i].x_0x04;
        sprite.p.y = y + sprite_entry[i].y_0x08;

        njDrawSprite2D(
            &sprite, sprite_entry[i].sprite_no_0x00, priority, NJD_SPRITE_ALPHA
        );

        priority += .0001f;
    }
}

/* Unreferenced. Draws at step/total of the way from start to end. */
STATIC void drawSpriteLerp_8c014ff6(
    float start_x,
    float start_y,
    float priority,
    float end_x,
    float end_y,
    int total,
    int step,
    ResourceGroup *res_group,
    int texture_id
){
    float total_f = total;
    float step_f = step;

    float x = start_x + step_f * ((end_x - start_x) / total_f);
    float y = start_y + step_f * ((end_y - start_y) / total_f);

    SpriteDraw_8c014f54(res_group, texture_id, x, y, priority);
}
