#include <shinobi.h>
#include "015ab8_title.h"
#include "014a9c_tasks.h"
#include "011120_asset_queues.h"
#include "014f54_text.h"
#include "serial_debug.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define PACKED_GLYPH_SIZE   0xc0
#define UNPACKED_GLYPH_SIZE 0xc0 * 4
#define GLYPH_TEXTURE_WIDTH 32
#define GLYPH_TEXTURE_SIZE  GLYPH_TEXTURE_WIDTH * GLYPH_TEXTURE_WIDTH
#define GLYPH_WIDTH         24
#define GLYPH_HEIGHT        32
#define GLYPH_PALETTE_SIZE  4
#define GLYPH_COUNT         0x200

#define ARGB1555(a, r, g, b) ( \
    ((a & 0x1) << 15) | ((r & 0x1F) << 10) | ((g & 0x1F) << 5) | (b & 0x1F) \
)

/* =================
 * Type Declarations
 * =================
 */

typedef struct {
    int sprite_no_0x00;
    float x_0x04;
    float y_0x08;
} ResourceGroupSpriteEntry;

typedef struct {
    const char *filename;
    int field_0x04;
    int field_0x08;
} DemoEntry;

typedef struct {
    int x_0x00;
    int y_0x04;
    float priority_0x08;
    int width_0x0c;
    int height_0x10;
    int x2_0x14;
    int y2_0x18;
    Uint16 processed_char_count_0x1c;
    Uint16 processed_tag_count_0x1e;
    Uint16 character_count_0x20;
    Uint16 tag_count_0x22;
    Uint16 palette_0x24[GLYPH_PALETTE_SIZE];
    Uint16 *tokens_0x2c;
    int enable_offset_0x30;
    Float *line_offsets_0x34;
    char *text_0x38;
} TextBox;

/* =======================
 * Non-initialized Globals
 * =======================
 */

extern void *var_busFont_8c1ba1c8;
extern int *var_demoBuf_8c1ba3c4;
extern int var_8c1bb868;
extern int var_inputMapSel_8c1bb8c8;
extern int var_demo_8c1bb8d0;
extern int var_8c1bb8d4;
extern int var_demoIndex_8c1bb8d8;

STATIC NJS_TEXNAME *var_glyphTexnames_8c1bc78c;
STATIC NJS_TEXLIST *var_glyphTexlists_8c1bc790;
STATIC ResourceGroup var_fontResourceGroup_8c1bc794;
STATIC Sint16 *var_8c1bc7a0;
STATIC void *var_glyphBuffer_8c1bc7a4;

MenuState menuState_8c1bc7a8;
void *var_8c1bc824;
ReplayInput demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY];
ReplayInput *demoCursor_8c225fa8;
Uint32 demoPrevOn_8c225fac;

extern int var_demoEntryValue_8c227e14;
extern int var_demoEntryValue_8c22822c;


/* ===================
 * Initialized Globals
 * ===================
 */

STATIC NJS_TEXANIM init_tanim_8c044128 = {
    GLYPH_WIDTH,  /* width */
    GLYPH_HEIGHT, /* height */
    0, 0,         /* center coordinates                 */
    0, 0,         /* upper left UV coordinates  (0-255) */
    184, 248,     /* lower right UV coordinates (0-255) */
    0,            /* number                             */
    0             /* attribute                          */
};

STATIC ResourceGroupSpriteEntry init_contents_8c04413c[2] = {
    { 0, 0, 0 },
    { -1, 0, 0 }
};

STATIC DemoEntry init_demos_8c044154[20] = {
    { "demo2.bin", 0x1E, 0x15 },
    { "demo6.bin", 0x0F, 0x06 },
    { "demo1.bin", 0x0E, 0x09 },
    { "demo0.bin", 0x08, 0x04 },
    { "demo5.bin", 0x0B, 0x04 },

    { "demo2.bin", 0x1E, 0x15 },
    { "demo6.bin", 0x0F, 0x06 },
    { "demo1.bin", 0x0E, 0x09 },
    { "demo0.bin", 0x08, 0x04 },
    { "demo5.bin", 0x0B, 0x04 },

    { "demo2.bin", 0x1E, 0x15 },
    { "demo6.bin", 0x0F, 0x06 },
    { "demo1.bin", 0x0E, 0x09 },
    { "demo0.bin", 0x08, 0x04 },
    { "demo5.bin", 0x0B, 0x04 },

    { "demo2.bin", 0x1E, 0x15 },
    { "demo6.bin", 0x0F, 0x06 },
    { "demo1.bin", 0x0E, 0x09 },
    { "demo0.bin", 0x08, 0x04 },
    { "demo5.bin", 0x0B, 0x04 },
};

/* ====================
 * Forward Declarations
 * ====================
 */

extern void setUknPvmBool_8c014330();

/* =========
 * Functions
 * =========
 */

/**
 * Draws a sprite or series of sprites from a resource group.
 *
 * @note Address: 0x8c014f54
 * 
 * @param res        Pointer to the ResourceGroup containing sprite data
 * @param texture_id ID of the texture to draw (2000 for BUS_FONT.FFF)
 * @param x          X-coordinate for the sprite's position
 * @param y          Y-coordinate for the sprite's position
 * @param priority   Initial draw priority for the sprite
 */
void drawSprite_8c014f54(
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

    // Handle BUS_FONT.FFF
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

    // Initialize NJS_SPRITE
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

/**
 * Draws a sprite with linear interpolation between two points.
 *
 * @note Address: 0x8c014ff6
 * @note This function is not used.
 *
 * @param start_x    The starting X-coordinate for the interpolation
 * @param start_y    The starting Y-coordinate for the interpolation
 * @param priority   The draw priority for the sprite
 * @param end_x      The ending X-coordinate for the interpolation
 * @param end_y      The ending Y-coordinate for the interpolation
 * @param steps_x    The number of steps in the X direction for interpolation
 * @param steps_y    The number of steps in the Y direction for interpolation
 * @param res_group  Pointer to the ResourceGroup containing sprite data
 * @param texture_id ID of the texture to draw
 */
void drawSpriteLerp_8c014ff6(
    float start_x,
    float start_y,
    float priority,
    float end_x,
    float end_y,
    int steps_x,
    int steps_y,
    ResourceGroup *res_group,
    int texture_id
){
    float steps_x_float = steps_x;
    float steps_y_float = steps_y;

    float x_step = (end_x - start_x) / steps_x_float;
    float y_step = (end_y - start_y) / steps_y_float;

    float lerp_x = start_x + steps_x_float * x_step;
    float lerp_y = start_y + steps_y_float * y_step;

    drawSprite_8c014f54(res_group, texture_id, lerp_x, lerp_y, priority);
}

/**
 * Retrieves the index for a given glyph based on the character code.
 */
STATIC Uint16 getGlyphIndex_8c015034(Uint16 character_code)
{
    // Font file offset table
    static const Uint16 font_section_offsets[40] = {
        0x0000, /* Special characters */
        0x005E, /* Special characters */
        0x006C, /* Digits and Roman */
        0x00AA, /* Hiragana */
        0x00FD, /* Katakana */
        0x0153, /* Greek */
        0x0183, /* Cyrillic */
        /* Kanji (32 sets with 94 kanji each)*/
        0x01C5, 0x0223, 0x0281, 0x02DF, 0x033D, 0x039B, 0x03F9, 0x0457,
        0x04B5, 0x0513, 0x0571, 0x05CF, 0x062D, 0x068B, 0x06E9, 0x0747,
        0x07A5, 0x0803, 0x0861, 0x08BF, 0x091D, 0x097B, 0x09D9, 0x0A37,
        0x0A95, 0x0AF3, 0x0B51, 0x0BAF, 0x0C0D, 0x0C6B, 0x0CC9, 0x0D27,
        0x0000
    };

    while (1) {
        Uint8 high_byte = (character_code >> 8) + 0x7f;
        Uint8 low_byte;
        Uint8 offset_index;

        // Check if high_byte is within valid range
        if (high_byte >= 0x6f || (high_byte >= 0x1f && high_byte < 0x3f)) {
            character_code = 0x81A6;  // Default character code
            continue;
        }

        if (high_byte >= 0x1f) high_byte -= 0x40;
        offset_index = (high_byte * 2) + 0x21;
        character_code += 0xc0;
        low_byte = (Uint8) character_code;

        // Check low_byte validity
        if (low_byte >= 0xbd || low_byte == 0x3f) {
            character_code = 0x81A6;  // Default character code
            continue;
        }

        if (low_byte < 0x3f) low_byte++;

        if (low_byte >= 0x5f) {
            low_byte -= 0x3e;
            offset_index++;
        } else {
            low_byte += 0x20;
        }

        switch (offset_index) {
            case 0x23:
                if (low_byte >= 0x61) low_byte -= 0x6;
                if (low_byte >= 0x41) low_byte -= 0x7;
                low_byte -= 0xf;
                break;

            case 0x26:
                if (low_byte >= 0x41) low_byte -= 0x8;
                break;

            case 0x27:
                if ((low_byte) >= 0x51) low_byte -= 0xf;
                break;

            default:
                if (offset_index >= 0x28) {
                    if (offset_index >= 0x30) {
                        if (offset_index >= 0x50) {
                            character_code = 0x81A6;  // Default character code
                            continue;
                        }
                        offset_index -= 0x8;
                    } else {
                        character_code = 0x81A6;  // Default character code
                        continue;
                    }
                }
                break;
        }

        offset_index -= 0x21;
        low_byte -= 0x21;
        return font_section_offsets[offset_index] + low_byte;
    }
}

/**
 * Unpacks and processes a glyph's texture data from the compressed font.
 *
 * This function takes a character code and unpacks its associated font data
 * into a texture buffer. The unpacked data is processed and translated into
 * color values using a provided color array. The final texture twiddled for
 * rendering.
 *
 * @param char_code The character code corresponding to the glyph to unpack.
 * @param palette An array of four 16-bit color values used to translate the
 * unpacked font data.
 * @param font The compressed font data.
 * @param dest The destination buffer for the twiddled texture data.
 */
STATIC unpackGlyph_8c015110(
    Uint16 char_code,
    Uint16 palette[GLYPH_PALETTE_SIZE],
    Uint8 *font,
    Sint16 *dest
) {
    /* Buffer for the unpacked font data */
    Uint8 unpacked[UNPACKED_GLYPH_SIZE] = {0};
    /* Buffer for the mapped texture */
    Sint16 mapped[GLYPH_TEXTURE_SIZE] = {0};

    size_t offset = getGlyphIndex_8c015034(char_code) * PACKED_GLYPH_SIZE;
    size_t i;
    size_t j;

    // Unpack font data
    for (i = 0; i < PACKED_GLYPH_SIZE; i++) {
        Uint8 byte = font[offset + i];

        unpacked[(i * 4)]     = (byte >> 6) & 0x3;
        unpacked[(i * 4) + 1] = (byte >> 4) & 0x3;
        unpacked[(i * 4) + 2] = (byte >> 2) & 0x3;
        unpacked[(i * 4) + 3] = byte & 0x3;
    };

    // Create color mapped texture
    // Note: This loop was refactored, but I would
    // like to preserve the original code.
    for (i = j = 0; i < GLYPH_TEXTURE_SIZE; i++) {
        Uint8 color_index;

        // Texture is square, but glyphs are 24 pixels wide
        if ((i % GLYPH_TEXTURE_WIDTH) >= GLYPH_WIDTH)
            continue;

        color_index = unpacked[j++];
        if (color_index < GLYPH_PALETTE_SIZE) {
            mapped[i] = palette[color_index];
        }
    }

    // Apply twiddle transformation
    njTwiddledTexture(dest, mapped, GLYPH_TEXTURE_WIDTH);
}

void TxtInit_8c01524c()
{
    int i;

    LOG_INFO(("[TXT] Initializing text module\n"));

    var_8c1bc7a0 = syMalloc(GLYPH_COUNT * sizeof(Sint16));
    for (i = 0; i < GLYPH_COUNT; i++) {
        var_8c1bc7a0[i] = (Uint16) -1;
    }

    var_glyphBuffer_8c1bc7a4 = syMalloc(GLYPH_TEXTURE_SIZE * sizeof(Uint16));
    var_glyphTexnames_8c1bc78c = syMalloc(GLYPH_COUNT * sizeof(NJS_TEXNAME));
    var_glyphTexlists_8c1bc790 = syMalloc(GLYPH_COUNT * sizeof(NJS_TEXLIST));
    var_fontResourceGroup_8c1bc794.tanim_0x04 = &init_tanim_8c044128;
    var_fontResourceGroup_8c1bc794.contents_0x08 = &init_contents_8c04413c;
}

void TxtDestroy_8c01529c()
{
    int i;

    LOG_INFO(("[TXT] Destroying text module\n"));

    for (i = 0; i < GLYPH_COUNT; i++) {
        if (var_8c1bc7a0[i] < -19) {
            njReleaseTexture(&var_glyphTexlists_8c1bc790[i]);
        }
    };
    syFree(var_glyphTexlists_8c1bc790);
    syFree(var_glyphTexnames_8c1bc78c);
    syFree(var_glyphBuffer_8c1bc7a4);
    syFree(var_8c1bc7a0);
}

/**
 * Creates a TextBox with the specified parameters.
 *
 * @return A pointer to the created TextBox.
 */
TextBox* TxtCreateTextBox_8c0152fc(
    int x,
    int y,
    float priority,
    int width,
    int height,
    int x2,
    int y2,
    int enable_offset
)
{
    int max_chars;
    int i;
    TextBox *box = syMalloc(sizeof(TextBox));

    LOG_INFO(("[TXT] Creating TextBox instance\n"));
    LOG_DEBUG((
        "      x=%d, y=%d, priority=%f, width=%d, height=%d, "
        "      x2=%d, y2=%d, enable_offset=%d\n",
        x, y, priority, width, height, x2, y2, enable_offset
    ));

    box->x_0x00 = x;
    box->y_0x04 = y;
    box->priority_0x08 = priority;
    box->width_0x0c = width;
    box->height_0x10 = height;
    box->x2_0x14 = x2;
    box->y2_0x18 = y2;
    box->palette_0x24[0] = ARGB1555(0, 0, 0, 0);
    box->palette_0x24[1] = ARGB1555(1, 10, 10, 10);
    box->palette_0x24[2] = ARGB1555(1, 15, 15, 15);
    box->palette_0x24[3] = ARGB1555(1, 17, 17, 17);
    max_chars = 0x28 + (width / GLYPH_WIDTH) * (height / GLYPH_HEIGHT);
    box->tokens_0x2c = syMalloc(max_chars * sizeof(Uint16));
    box->line_offsets_0x34 = syMalloc(height / GLYPH_HEIGHT * sizeof(Float));
    box->enable_offset_0x30 = enable_offset;

    for (i = 0; i < max_chars; i++) {
        box->tokens_0x2c[i] = (Uint16) -1;
    }

    box->text_0x38 = NULL;

    return box;
}

/**
 * Frees the memory allocated for a TextBox.
 *
 * @todo Write a test for this function.
 * @param box The TextBox to be freed.
 */
void TxtDestroyTextBox_8c015410(TextBox *box)
{
    if (box->tokens_0x2c) {
        syFree(box->tokens_0x2c);
    }
    if (box->line_offsets_0x34) {
        syFree(box->line_offsets_0x34);
    }
    syFree(box);
}

int TxtPrepareTextBoxLayout_8c01543a(TextBox *box, char *text)
{
    int i;
    int current_line;
    int character_count;
    int line_count;
    const int characters_per_line = box->width_0x0c / GLYPH_WIDTH;

    // Check if the box already contains characters
    if (*box->tokens_0x2c != (Uint16) -1) {
        int i;
        int available_characters;

        // Release textures for existing characters
        for (i = 0; i < box->character_count_0x20 + box->tag_count_0x22; i++) {
            if (box->tokens_0x2c[i] < 0xffed) {
                njReleaseTexture(
                    &var_glyphTexlists_8c1bc790[box->tokens_0x2c[i]]
                );
                var_8c1bc7a0[box->tokens_0x2c[i]] = -1;
            }
        }

        // Reset character codes
        available_characters =
            0x28 + characters_per_line * (box->height_0x10 / GLYPH_HEIGHT);
        for (i = 0; i < available_characters; i++) {
            box->tokens_0x2c[i] = 0xffff;
        }
    }

    // If the input text is empty, set the box text and return
    if (*text == '\0') {
        box->text_0x38 = text;
        return 0;
    }

    // Count the number of tags in the text
    box->tag_count_0x22 = 0;
    for (i = 0; text[i] != '\0'; i++) {
        if (text[i] == '<') {
            box->tag_count_0x22++;
        }
    }

    // Calculate the number of characters (excluding tags)
    // In Shift JIS, characters can be 1 or 2 bytes.
    // This assumes 2 bytes per character.
    character_count = (strlen(text) - box->tag_count_0x22 * 3) / 2;

    box->text_0x38 = text;
    box->processed_char_count_0x1c = 0;
    box->processed_tag_count_0x1e = 0;
    box->character_count_0x20 = character_count;

    // Initialize line offsets
    line_count = box->height_0x10 / 0x20;
    for (i = 0; i < line_count; i++) {
        box->line_offsets_0x34[i] = 0.0f;
    }

    // Process the text, handling tags and line wrapping
    current_line = 0;
    while (*text) {
        // Handle tags
        if (*text == '<') {
            // Line break tag
            if (*++text == 'E') {
                if (current_line >= line_count)
                    break;
                current_line++;
            }
            text += 2;
            continue;
        }

        // Wrap text if the current line is full
        if (box->line_offsets_0x34[current_line] / 2 >= characters_per_line) {
            if (current_line >= line_count)
                break;
            current_line++;
        };

        box->line_offsets_0x34[current_line] += 1;

        text++;
    }

    // Center-align text on each line
    for (i = 0; i < line_count; i++) {
        box->line_offsets_0x34[i] =
            (characters_per_line - (box->line_offsets_0x34[i] / 2)) / 2;
    }

    return character_count;
}

int TxtDrawTextbox_8c0155e0(TextBox *box, int limit)
{
    int token_idx = 0;
    int token_limit = 0;
    int row = 0;
    int col = 0;

    if (box->text_0x38 == NULL || !*box->text_0x38) {
        return 0;
    }

    if (box->character_count_0x20 >= limit) {
        token_limit = box->processed_tag_count_0x1e + limit;
    } else {
        token_limit = box->character_count_0x20 + box->tag_count_0x22;
    }

    for (token_idx = 0; token_idx < token_limit; token_idx++) {
        // Load glyph if not already loaded
        if (
            box->processed_char_count_0x1c + box->processed_tag_count_0x1e <=
            token_idx
        ) {
            char *currentChar;
            unsigned nextChar; // Move down the scope?

            currentChar = box->text_0x38
                + box->processed_tag_count_0x1e * 3
                + box->processed_char_count_0x1c * 2;

            if (*currentChar == '<') {
                nextChar = currentChar[1];
                switch (nextChar) {
                    case 'E':
                        box->tokens_0x2c[token_idx] = 0xFFFE;
                        break;
                    case 'D':
                        box->tokens_0x2c[token_idx] = 0xFFFD;
                        box->palette_0x24[0] = ARGB1555(0, 0, 0, 0);
                        box->palette_0x24[1] = ARGB1555(1, 10, 10, 10);
                        box->palette_0x24[2] = ARGB1555(1, 15, 15, 15);
                        box->palette_0x24[3] = ARGB1555(1, 17, 17, 17);
                        break;
                    case 'C':
                        box->tokens_0x2c[token_idx] = 0xFFFC;
                        break;
                    case 'R':
                        box->tokens_0x2c[token_idx] = 0xFFFB;
                        box->palette_0x24[0] = ARGB1555(0, 0, 0, 0);
                        box->palette_0x24[1] = ARGB1555(1, 20, 10, 10);
                        box->palette_0x24[2] = ARGB1555(1, 25, 15, 15);
                        box->palette_0x24[3] = ARGB1555(1, 31, 16, 16);
                        break;
                }

                box->processed_tag_count_0x1e++;
            } else {
                int glyphIndex = 0;

                // Load glyph
                while (glyphIndex < GLYPH_COUNT) {
                    if (var_8c1bc7a0[glyphIndex] == -1) {
                        NJS_TEXINFO texInfo;

                        unpackGlyph_8c015110(
                            ((*currentChar & 0xFF) << 8)
                                | (currentChar[1] & 0xFF),
                            box->palette_0x24,
                            var_busFont_8c1ba1c8,
                            var_glyphBuffer_8c1bc7a4
                        );

                        njSetTextureInfo(
                            &texInfo,
                            var_glyphBuffer_8c1bc7a4,
                            NJD_TEXFMT_ARGB_1555 | NJD_TEXFMT_TWIDDLED,
                            GLYPH_TEXTURE_WIDTH,
                            GLYPH_TEXTURE_WIDTH
                        );
                        njSetTextureName(
                            &var_glyphTexnames_8c1bc78c[glyphIndex],
                            &texInfo,
                            glyphIndex,
                            NJD_TEXATTR_TYPE_MEMORY | NJD_TEXATTR_GLOBALINDEX
                        );

                        var_glyphTexlists_8c1bc790[glyphIndex].textures =
                            &var_glyphTexnames_8c1bc78c[glyphIndex];
                        var_glyphTexlists_8c1bc790[glyphIndex].nbTexture = 1;
                        var_8c1bc7a0[glyphIndex] = glyphIndex;
                        box->tokens_0x2c[token_idx] = glyphIndex;

                        njLoadTexture(&var_glyphTexlists_8c1bc790[glyphIndex]);
                        box->processed_char_count_0x1c++;
                        break;
                    }

                    glyphIndex++;
                }

                // Glyph overflow (TODO: improve comment)
                if (glyphIndex >= GLYPH_COUNT) {
                    return -1;
                }
            }
        }

        // Draw glyph
        if (box->tokens_0x2c[token_idx] < 0xffed) {
            var_fontResourceGroup_8c1bc794.tlist_0x00 =
                &var_glyphTexlists_8c1bc790[box->tokens_0x2c[token_idx]];
            // Wrap line
            if ((col + 1) * GLYPH_WIDTH > box->width_0x0c) {
                col = 0;
                row++;
            }

            if ((row + 1) * GLYPH_HEIGHT <= box->height_0x10) {
                if (box->enable_offset_0x30 == -1) {
                    int x = col * GLYPH_WIDTH + box->x_0x00 + box->x2_0x14;
                    int y = row * GLYPH_HEIGHT + box->y_0x04 + box->y2_0x18;

                    x += box->line_offsets_0x34[row] * GLYPH_WIDTH;

                    drawSprite_8c014f54(
                        &var_fontResourceGroup_8c1bc794,
                        2000,
                        x,
                        y,
                        box->priority_0x08
                    );
                } else {
                    int x = col + box->x_0x00 + box->x2_0x14;
                    int y = row + box->y_0x04 + box->y2_0x18;
                    drawSprite_8c014f54(
                        &var_fontResourceGroup_8c1bc794,
                        2000,
                        x,
                        y,
                        box->priority_0x08
                    );
                }

            }

            col++;
        }
        // Line break
        else if (box->tokens_0x2c[token_idx] == 0xfffe) {
            col = 0;
            row += 1;
        }
    }

    return 1;
}

STATIC void FUN_8c01594c(Task *task)
{
    void *local;
    if (!getUknPvmBool_8c01432a()) {
        return;
    }

    var_8c1bb868 = var_demoBuf_8c1ba3c4[1];
    var_inputMapSel_8c1bb8c8 = var_demoBuf_8c1ba3c4[2];
    var_seed_8c157a64 = var_demoBuf_8c1ba3c4[3];
    local = demoBuffer_8c1bc828;
    FUN_8c02f320();
    FUN_readDemo_8c02fa14(&var_demoBuf_8c1ba3c4[4], &local, var_demoBuf_8c1ba3c4[0]);
    syFree(var_demoBuf_8c1ba3c4);
    var_demoBuf_8c1ba3c4 = (int *) -1;
    freeTask_8c014b66(task);
    FUN_8c01328c();
}

void FUN_demo_8c0159ac()
{
    Task *created_task;
    void *created_state;
    pushTask_8c014ae8(
        var_tasks_8c1ba3c8, FUN_8c01594c, &created_task, &created_state, 0
    );
    created_task->field_0x08 = 0;
    // created_task->field_0x0c = NULL;
    var_demo_8c1bb8d0 = 2;
    var_8c1bb8d4 = 1;
    if (++var_demoIndex_8c1bb8d8 >= 20) {
        var_demoIndex_8c1bb8d8 = 0;
    }
    AsqInitQueues_11f36(1,0,0,0);
    AsqResetQueues_11f6c();
    AsqRequestDat_11182(
        "\\SYSTEM",
        init_demos_8c044154[var_demoIndex_8c1bb8d8].filename,
        &var_demoBuf_8c1ba3c4
    );
    var_demoEntryValue_8c227e14 =
        init_demos_8c044154[var_demoIndex_8c1bb8d8].field_0x04;
    var_demoEntryValue_8c22822c =
        init_demos_8c044154[var_demoIndex_8c1bb8d8].field_0x08;
    resetUknPvmBool_8c014322();
    AsqProcessQueues_11fe0(AsqNop_11120, 0, 0, 0, setUknPvmBool_8c014330);
    return;
}