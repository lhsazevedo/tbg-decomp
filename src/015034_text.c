/* @unit Txt */
/* 8c015034 */
#include <shinobi.h>
#include "013ae8_route.h"
#include "014f54_sprite.h"
#include "015ab8_title.h"
#include "01614c_replay_menu.h"
#include "014a9c_tasks.h"
#include "011120_asset_queues.h"
#include "015034_text.h"
#include "026710_traffic.h"
#include "0289ac_objects.h" /* var_activePedPreset_8c22822c */
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "02f320_replay_codec.h"

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
#define GLYPH_COUNT         0x200

#define ARGB1555(a, r, g, b) ( \
    ((a & 0x1) << 15) | ((r & 0x1F) << 10) | ((g & 0x1F) << 5) | (b & 0x1F) \
)

/* String encoding and layout differ per language.
 *
 * Japanese uses a fixed GLYPH_WIDTH grid. English advances per glyph using
 * the ink metrics measured off BUS_FONT.FFF (init_glyphMetricsEn below): the
 * font draws alphanumerics proportionally within the fixed cell, from 4px
 * ('I') to 20px ('m').
 *
 * GLYPH_ADVANCE_MIN is the narrowest advance any glyph can take; it bounds
 * the token buffer, which must hold the most characters a line can ever fit. */
#define TEXT_TAG_BYTES 3
#ifdef GAME_LANG_EN
#define TEXT_CHAR_BYTES    1
#define TEXT_ASCII_FIRST   0x20
#define TEXT_ASCII_COUNT   0x5f
/* Side bearing added to each glyph's ink width. The one knob worth tuning. */
#define TEXT_GLYPH_GAP     2
#define TEXT_GLYPH_INK_MIN  3 /* narrowest ink in the table below */
#define GLYPH_ADVANCE_MIN  (TEXT_GLYPH_INK_MIN + TEXT_GLYPH_GAP)
/* The panel art is inset from the text area callers pass in. Japanese
 * strings are authored to fit inside it; English wraps short by this much
 * per side, and centring on the full width turns the slack into an even
 * margin. */
#define TEXT_BOX_MARGIN    24
#define TEXT_WRAP_WIDTH(box) ((box)->width_0x0c - 2 * TEXT_BOX_MARGIN)
#define TEXT_GLYPH(c)         (init_glyphMetricsEn[(Uint8)(c) - TEXT_ASCII_FIRST])
#define TEXT_GLYPH_CODE(c)    (TEXT_GLYPH(c).code)
#define TEXT_GLYPH_ADVANCE(c) (TEXT_GLYPH(c).advance)
#define TEXT_GLYPH_BEARING(c) (TEXT_GLYPH(c).bearing)
#else
#define TEXT_CHAR_BYTES   2
#define GLYPH_ADVANCE_MIN GLYPH_WIDTH
#endif

/* ====================
 * Type Declarations
 * ====================
 */

typedef struct {
    const char *filename;
    int trafficPresetId_0x04;
    int pedPresetId_0x08;
} DemoEntry;

#ifdef GAME_LANG_EN
typedef struct {
    Uint16 code;    /* full-width Shift-JIS code point */
    Uint8 advance;  /* pen step: ink width plus the side bearing */
    Uint8 bearing;  /* ink start column within the cell */
} GlyphMetric;
#endif

/* ====================
 * Non-initialized Globals
 * ====================
 */

STATIC NJS_TEXNAME *var_glyphTexnames_8c1bc78c;
STATIC NJS_TEXLIST *var_glyphTexlists_8c1bc790;
STATIC ResourceGroup var_fontResourceGroup_8c1bc794;
/* Per glyph texture slot: 0xffff free, otherwise the slot's own index. */
STATIC Sint16 *var_glyphSlotUsed_8c1bc7a0;
STATIC void *var_glyphBuffer_8c1bc7a4;

/* ====================
 * Initialized Globals
 * ====================
 */

#ifdef GAME_LANG_EN
/* Indexed by ASCII - TEXT_ASCII_FIRST. Measured off
 * tests/015034_text/data/BUS_FONT.FFF with the engine's own glyph lookup and
 * unpack. */
STATIC const GlyphMetric init_glyphMetricsEn[TEXT_ASCII_COUNT] = {
    { 0x8140,  6 + TEXT_GLYPH_GAP,  0 }, /* sp */
    { 0x8149,  6 + TEXT_GLYPH_GAP, 10 }, /* '!' */
    { 0x8168, 10 + TEXT_GLYPH_GAP,  0 }, /* '"' */
    { 0x8194, 17 + TEXT_GLYPH_GAP,  4 }, /* '#' */
    { 0x8190, 16 + TEXT_GLYPH_GAP,  4 }, /* '$' */
    { 0x8193, 21 + TEXT_GLYPH_GAP,  2 }, /* '%' */
    { 0x8195, 15 + TEXT_GLYPH_GAP,  5 }, /* '&' */
    { 0x8166,  4 + TEXT_GLYPH_GAP,  1 }, /* ''' */
    { 0x8169,  5 + TEXT_GLYPH_GAP, 18 }, /* '(' */
    { 0x816A,  6 + TEXT_GLYPH_GAP,  1 }, /* ')' */
    { 0x8196, 16 + TEXT_GLYPH_GAP,  4 }, /* '*' */
    { 0x817B, 18 + TEXT_GLYPH_GAP,  4 }, /* '+' */
    { 0x8143,  5 + TEXT_GLYPH_GAP,  1 }, /* ',' */
    { 0x817C, 18 + TEXT_GLYPH_GAP,  4 }, /* '-' */
    { 0x8144,  4 + TEXT_GLYPH_GAP,  1 }, /* '.' */
    { 0x815E, 23 + TEXT_GLYPH_GAP,  1 }, /* '/' */
    { 0x824F, 18 + TEXT_GLYPH_GAP,  4 }, /* '0' */
    { 0x8250,  6 + TEXT_GLYPH_GAP,  9 }, /* '1' */
    { 0x8251, 13 + TEXT_GLYPH_GAP,  6 }, /* '2' */
    { 0x8252, 14 + TEXT_GLYPH_GAP,  6 }, /* '3' */
    { 0x8253, 18 + TEXT_GLYPH_GAP,  4 }, /* '4' */
    { 0x8254, 15 + TEXT_GLYPH_GAP,  5 }, /* '5' */
    { 0x8255, 15 + TEXT_GLYPH_GAP,  5 }, /* '6' */
    { 0x8256, 15 + TEXT_GLYPH_GAP,  5 }, /* '7' */
    { 0x8257, 14 + TEXT_GLYPH_GAP,  5 }, /* '8' */
    { 0x8258, 15 + TEXT_GLYPH_GAP,  5 }, /* '9' */
    { 0x8146,  4 + TEXT_GLYPH_GAP, 10 }, /* ':' */
    { 0x8147,  4 + TEXT_GLYPH_GAP, 10 }, /* ';' */
    { 0x8183, 13 + TEXT_GLYPH_GAP,  6 }, /* '<' */
    { 0x8181, 18 + TEXT_GLYPH_GAP,  4 }, /* '=' */
    { 0x8184, 12 + TEXT_GLYPH_GAP,  6 }, /* '>' */
    { 0x8148, 15 + TEXT_GLYPH_GAP,  5 }, /* '?' */
    { 0x8197, 20 + TEXT_GLYPH_GAP,  3 }, /* '@' */
    { 0x8260, 19 + TEXT_GLYPH_GAP,  3 }, /* 'A' */
    { 0x8261, 17 + TEXT_GLYPH_GAP,  4 }, /* 'B' */
    { 0x8262, 18 + TEXT_GLYPH_GAP,  4 }, /* 'C' */
    { 0x8263, 16 + TEXT_GLYPH_GAP,  4 }, /* 'D' */
    { 0x8264, 15 + TEXT_GLYPH_GAP,  5 }, /* 'E' */
    { 0x8265, 15 + TEXT_GLYPH_GAP,  5 }, /* 'F' */
    { 0x8266, 17 + TEXT_GLYPH_GAP,  4 }, /* 'G' */
    { 0x8267, 16 + TEXT_GLYPH_GAP,  4 }, /* 'H' */
    { 0x8268,  4 + TEXT_GLYPH_GAP, 10 }, /* 'I' */
    { 0x8269, 13 + TEXT_GLYPH_GAP,  6 }, /* 'J' */
    { 0x826A, 17 + TEXT_GLYPH_GAP,  4 }, /* 'K' */
    { 0x826B, 15 + TEXT_GLYPH_GAP,  5 }, /* 'L' */
    { 0x826C, 19 + TEXT_GLYPH_GAP,  3 }, /* 'M' */
    { 0x826D, 18 + TEXT_GLYPH_GAP,  4 }, /* 'N' */
    { 0x826E, 18 + TEXT_GLYPH_GAP,  3 }, /* 'O' */
    { 0x826F, 16 + TEXT_GLYPH_GAP,  4 }, /* 'P' */
    { 0x8270, 18 + TEXT_GLYPH_GAP,  3 }, /* 'Q' */
    { 0x8271, 17 + TEXT_GLYPH_GAP,  4 }, /* 'R' */
    { 0x8272, 17 + TEXT_GLYPH_GAP,  4 }, /* 'S' */
    { 0x8273, 17 + TEXT_GLYPH_GAP,  4 }, /* 'T' */
    { 0x8274, 17 + TEXT_GLYPH_GAP,  4 }, /* 'U' */
    { 0x8275, 17 + TEXT_GLYPH_GAP,  4 }, /* 'V' */
    { 0x8276, 20 + TEXT_GLYPH_GAP,  2 }, /* 'W' */
    { 0x8277, 16 + TEXT_GLYPH_GAP,  4 }, /* 'X' */
    { 0x8278, 18 + TEXT_GLYPH_GAP,  4 }, /* 'Y' */
    { 0x8279, 15 + TEXT_GLYPH_GAP,  5 }, /* 'Z' */
    { 0x816D,  8 + TEXT_GLYPH_GAP, 16 }, /* '[' */
    { 0x815F, 23 + TEXT_GLYPH_GAP,  1 }, /* '\\' */
    { 0x816E,  8 + TEXT_GLYPH_GAP,  1 }, /* ']' */
    { 0x814F, 12 + TEXT_GLYPH_GAP,  6 }, /* '^' */
    { 0x8151, 24 + TEXT_GLYPH_GAP,  0 }, /* '_' */
    { 0x814D,  8 + TEXT_GLYPH_GAP,  8 }, /* '`' */
    { 0x8281, 13 + TEXT_GLYPH_GAP,  6 }, /* 'a' */
    { 0x8282, 14 + TEXT_GLYPH_GAP,  6 }, /* 'b' */
    { 0x8283, 12 + TEXT_GLYPH_GAP,  6 }, /* 'c' */
    { 0x8284, 14 + TEXT_GLYPH_GAP,  5 }, /* 'd' */
    { 0x8285, 12 + TEXT_GLYPH_GAP,  6 }, /* 'e' */
    { 0x8286, 10 + TEXT_GLYPH_GAP,  8 }, /* 'f' */
    { 0x8287, 13 + TEXT_GLYPH_GAP,  6 }, /* 'g' */
    { 0x8288, 12 + TEXT_GLYPH_GAP,  6 }, /* 'h' */
    { 0x8289,  5 + TEXT_GLYPH_GAP, 10 }, /* 'i' */
    { 0x828A,  7 + TEXT_GLYPH_GAP,  8 }, /* 'j' */
    { 0x828B, 14 + TEXT_GLYPH_GAP,  5 }, /* 'k' */
    { 0x828C,  5 + TEXT_GLYPH_GAP, 10 }, /* 'l' */
    { 0x828D, 20 + TEXT_GLYPH_GAP,  3 }, /* 'm' */
    { 0x828E, 14 + TEXT_GLYPH_GAP,  6 }, /* 'n' */
    { 0x828F, 14 + TEXT_GLYPH_GAP,  6 }, /* 'o' */
    { 0x8290, 14 + TEXT_GLYPH_GAP,  6 }, /* 'p' */
    { 0x8291, 14 + TEXT_GLYPH_GAP,  6 }, /* 'q' */
    { 0x8292, 12 + TEXT_GLYPH_GAP,  7 }, /* 'r' */
    { 0x8293, 13 + TEXT_GLYPH_GAP,  6 }, /* 's' */
    { 0x8294,  9 + TEXT_GLYPH_GAP,  8 }, /* 't' */
    { 0x8295, 14 + TEXT_GLYPH_GAP,  6 }, /* 'u' */
    { 0x8296, 14 + TEXT_GLYPH_GAP,  5 }, /* 'v' */
    { 0x8297, 19 + TEXT_GLYPH_GAP,  3 }, /* 'w' */
    { 0x8298, 13 + TEXT_GLYPH_GAP,  6 }, /* 'x' */
    { 0x8299, 13 + TEXT_GLYPH_GAP,  6 }, /* 'y' */
    { 0x829A, 13 + TEXT_GLYPH_GAP,  6 }, /* 'z' */
    { 0x816F,  8 + TEXT_GLYPH_GAP, 16 }, /* '{' */
    { 0x8162,  3 + TEXT_GLYPH_GAP, 11 }, /* '|' */
    { 0x8170,  8 + TEXT_GLYPH_GAP,  1 }, /* '}' */
    { 0x8160, 21 + TEXT_GLYPH_GAP,  2 }, /* '~' */
};
#endif

STATIC NJS_TEXANIM init_glyphTanim_8c044128 = {
    GLYPH_WIDTH,  /* width */
    GLYPH_HEIGHT, /* height */
    0, 0,         /* center coordinates                 */
    0, 0,         /* upper left UV coordinates  (0-255) */
    184, 248,     /* lower right UV coordinates (0-255) */
    0,            /* number                             */
    0             /* attribute                          */
};

STATIC ResourceGroupSpriteEntry init_glyphSprite_8c04413c[2] = {
    { 0, 0, 0 },
    { -1, 0, 0 }
};

/* Five demos, listed four times over: the attract loop steps one entry per
 * timeout and wraps at 20, so each demo comes round every fifth pass. */
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
 * Functions
 * ====================
 */

STATIC Uint16 getGlyphIndex_8c015034(Uint16 character_code)
{
    static const Uint16 font_section_offsets[40] = {
        0x0000, /* Special characters */
        0x005E, /* Special characters */
        0x006C, /* Digits and Roman */
        0x00AA, /* Hiragana */
        0x00FD, /* Katakana */
        0x0153, /* Greek */
        0x0183, /* Cyrillic */
        /* Kanji (32 sets with 94 kanji each) */
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

        if (high_byte >= 0x6f || (high_byte >= 0x1f && high_byte < 0x3f)) {
            character_code = 0x81A6;
            continue;
        }

        if (high_byte >= 0x1f) high_byte -= 0x40;
        offset_index = (high_byte * 2) + 0x21;
        character_code += 0xc0;
        low_byte = (Uint8) character_code;

        if (low_byte >= 0xbd || low_byte == 0x3f) {
            character_code = 0x81A6;
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
                            character_code = 0x81A6;
                            continue;
                        }
                        offset_index -= 0x8;
                    } else {
                        character_code = 0x81A6;
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

STATIC unpackGlyph_8c015110(
    Uint16 char_code,
    Uint16 palette[GLYPH_PALETTE_SIZE],
    Uint8 *font,
    Sint16 *dest
) {
    /* No `= {0}`: SHC emits it as a section C template the original lacks. */
    Uint8 unpacked[UNPACKED_GLYPH_SIZE];
    Sint16 mapped[GLYPH_TEXTURE_SIZE];

    size_t offset = getGlyphIndex_8c015034(char_code) * PACKED_GLYPH_SIZE;
    size_t i;
    size_t j;

    /* Clearing dest is dead work, but the original does it. */
    for (i = 0; i < GLYPH_TEXTURE_SIZE; i++) {
        dest[i] = 0;
        mapped[i] = 0;
    }

    for (i = 0; i < PACKED_GLYPH_SIZE; i++) {
        Uint8 byte = font[offset + i];

        unpacked[(i * 4)]     = (byte >> 6) & 0x3;
        unpacked[(i * 4) + 1] = (byte >> 4) & 0x3;
        unpacked[(i * 4) + 2] = (byte >> 2) & 0x3;
        unpacked[(i * 4) + 3] = byte & 0x3;
    };

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

    njTwiddledTexture(dest, mapped, GLYPH_TEXTURE_WIDTH);
}

void TxtInit_8c01524c()
{
    int i;

    LOG_INFO(("[TXT] Initializing text module\n"));

    var_glyphSlotUsed_8c1bc7a0 = syMalloc(GLYPH_COUNT * sizeof(Sint16));
    for (i = 0; i < GLYPH_COUNT; i++) {
        var_glyphSlotUsed_8c1bc7a0[i] = (Uint16) -1;
    }

    var_glyphBuffer_8c1bc7a4 = syMalloc(GLYPH_TEXTURE_SIZE * sizeof(Uint16));
    var_glyphTexnames_8c1bc78c = syMalloc(GLYPH_COUNT * sizeof(NJS_TEXNAME));
    var_glyphTexlists_8c1bc790 = syMalloc(GLYPH_COUNT * sizeof(NJS_TEXLIST));
    var_fontResourceGroup_8c1bc794.tanim_0x04 = &init_glyphTanim_8c044128;
    var_fontResourceGroup_8c1bc794.contents_0x08 = &init_glyphSprite_8c04413c;
}

void TxtDestroy_8c01529c()
{
    int i;

    LOG_INFO(("[TXT] Destroying text module\n"));

    for (i = 0; i < GLYPH_COUNT; i++) {
        /* Unsigned, as in the asm (EXTU.W). Signed, every slot looks free,
         * the textures leak, and a later njLoadTexture at the same global
         * index keeps the old glyph. */
        if ((Uint16) var_glyphSlotUsed_8c1bc7a0[i] < 0xffed) {
            njReleaseTexture(&var_glyphTexlists_8c1bc790[i]);
        }
    };
    syFree(var_glyphTexlists_8c1bc790);
    syFree(var_glyphTexnames_8c1bc78c);
    syFree(var_glyphBuffer_8c1bc7a4);
    syFree(var_glyphSlotUsed_8c1bc7a0);
}

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
    max_chars = 0x28 + (width / GLYPH_ADVANCE_MIN) * (height / GLYPH_HEIGHT);
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
 * @todo Write a test for this function.
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
    const int characters_per_line = box->width_0x0c / GLYPH_ADVANCE_MIN;

    // Check if the box already contains characters
    if (*box->tokens_0x2c != (Uint16) -1) {
        int i;
        int available_characters;

        for (i = 0; i < box->character_count_0x20 + box->tag_count_0x22; i++) {
            if (box->tokens_0x2c[i] < 0xffed) {
                njReleaseTexture(
                    &var_glyphTexlists_8c1bc790[box->tokens_0x2c[i]]
                );
                var_glyphSlotUsed_8c1bc7a0[box->tokens_0x2c[i]] = -1;
            }
        }

        available_characters =
            0x28 + characters_per_line * (box->height_0x10 / GLYPH_HEIGHT);
        for (i = 0; i < available_characters; i++) {
            box->tokens_0x2c[i] = 0xffff;
        }
    }

    if (*text == '\0') {
        box->text_0x38 = text;
        return 0;
    }

    box->tag_count_0x22 = 0;
    for (i = 0; text[i] != '\0'; i++) {
        if (text[i] == '<') {
            box->tag_count_0x22++;
        }
    }

    // In Shift JIS, characters can be 1 or 2 bytes.
    // This assumes 2 bytes per character (1 for GAME_LANG_EN's ASCII).
    character_count =
        (strlen(text) - box->tag_count_0x22 * TEXT_TAG_BYTES) / TEXT_CHAR_BYTES;

    box->text_0x38 = text;
    box->processed_char_count_0x1c = 0;
    box->processed_tag_count_0x1e = 0;
    box->character_count_0x20 = character_count;

    line_count = box->height_0x10 / GLYPH_HEIGHT;
    for (i = 0; i < line_count; i++) {
        box->line_offsets_0x34[i] = 0.0f;
    }

    current_line = 0;
    while (*text) {
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

#ifdef GAME_LANG_EN
        // line_offsets_0x34 accumulates pixels, not columns
        if (box->line_offsets_0x34[current_line] + TEXT_GLYPH_ADVANCE(*text)
                > TEXT_WRAP_WIDTH(box)) {
            if (current_line >= line_count)
                break;
            current_line++;
        }

        box->line_offsets_0x34[current_line] += TEXT_GLYPH_ADVANCE(*text);
#else
        if (box->line_offsets_0x34[current_line] / 2 >= characters_per_line) {
            if (current_line >= line_count)
                break;
            current_line++;
        };

        // BUG (kept, see tests/015034_text/8c01543a_TxtPrepareTextBoxLayout.php
        // test_processExceedingLineBreaks): current_line can still equal
        // line_count here, writing one float past line_offsets_0x34. Applies
        // in GAME_LANG_EN too.
        box->line_offsets_0x34[current_line] += 1;
#endif

        text++;
    }

    // Center-align text on each line
    for (i = 0; i < line_count; i++) {
#ifdef GAME_LANG_EN
        box->line_offsets_0x34[i] =
            (box->width_0x0c - box->line_offsets_0x34[i]) / 2;
#else
        box->line_offsets_0x34[i] =
            (characters_per_line - (box->line_offsets_0x34[i] / 2)) / 2;
#endif
    }

    return character_count;
}

int TxtDrawTextBox_8c0155e0(TextBox *box, int limit)
{
    int token_idx = 0;
    int token_limit = 0;
    int row = 0;
    int col = 0;
#ifdef GAME_LANG_EN
    /* col is a pixel pen in English, advanced by walking the source text
     * one character at a time. */
    char *penChar = box->text_0x38;
#endif

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
            unsigned nextChar;

            currentChar = box->text_0x38
                + box->processed_tag_count_0x1e * TEXT_TAG_BYTES
                + box->processed_char_count_0x1c * TEXT_CHAR_BYTES;

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

                while (glyphIndex < GLYPH_COUNT) {
                    if (var_glyphSlotUsed_8c1bc7a0[glyphIndex] == -1) {
                        NJS_TEXINFO texInfo;

                        unpackGlyph_8c015110(
#ifdef GAME_LANG_EN
                            TEXT_GLYPH_CODE(*currentChar),
#else
                            ((*currentChar & 0xFF) << 8)
                                | (currentChar[1] & 0xFF),
#endif
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
                        var_glyphSlotUsed_8c1bc7a0[glyphIndex] = glyphIndex;
                        box->tokens_0x2c[token_idx] = glyphIndex;

                        njLoadTexture(&var_glyphTexlists_8c1bc790[glyphIndex]);
                        box->processed_char_count_0x1c++;
                        break;
                    }

                    glyphIndex++;
                }

                /* No free slot. */
                if (glyphIndex >= GLYPH_COUNT) {
                    return -1;
                }
            }
        }

        if (box->tokens_0x2c[token_idx] < 0xffed) {
            var_fontResourceGroup_8c1bc794.tlist_0x00 =
                &var_glyphTexlists_8c1bc790[box->tokens_0x2c[token_idx]];
#ifdef GAME_LANG_EN
            if (col + TEXT_GLYPH_ADVANCE(*penChar) > TEXT_WRAP_WIDTH(box)) {
                col = 0;
                row++;
            }
#else
            if ((col + 1) * GLYPH_WIDTH > box->width_0x0c) {
                col = 0;
                row++;
            }
#endif

            if ((row + 1) * GLYPH_HEIGHT <= box->height_0x10) {
                if (box->enable_offset_0x30 == -1) {
#ifdef GAME_LANG_EN
                    /* col is a pixel pen; subtract the glyph's bearing so its
                     * ink, not the cell, lands here. */
                    int x = col - TEXT_GLYPH_BEARING(*penChar)
                        + box->x_0x00 + box->x2_0x14;
                    int y = row * GLYPH_HEIGHT + box->y_0x04 + box->y2_0x18;

                    x += box->line_offsets_0x34[row];
#else
                    int x = col * GLYPH_WIDTH + box->x_0x00 + box->x2_0x14;
                    int y = row * GLYPH_HEIGHT + box->y_0x04 + box->y2_0x18;

                    x += box->line_offsets_0x34[row] * GLYPH_WIDTH;
#endif

                    SpriteDraw_8c014f54(
                        &var_fontResourceGroup_8c1bc794,
                        2000,
                        x,
                        y,
                        box->priority_0x08
                    );
                } else {
                    int x = col + box->x_0x00 + box->x2_0x14;
                    int y = row + box->y_0x04 + box->y2_0x18;
                    SpriteDraw_8c014f54(
                        &var_fontResourceGroup_8c1bc794,
                        2000,
                        x,
                        y,
                        box->priority_0x08
                    );
                }

            }

#ifdef GAME_LANG_EN
            col += TEXT_GLYPH_ADVANCE(*penChar);
#else
            col++;
#endif
        }
        // Line break
        else if (box->tokens_0x2c[token_idx] == 0xfffe) {
            col = 0;
            row += 1;
        }

#ifdef GAME_LANG_EN
        /* Tokens are emitted one per source element; walking the text in
         * step with them recovers the character behind each glyph token. */
        penChar += (box->tokens_0x2c[token_idx] < 0xffed)
            ? TEXT_CHAR_BYTES : TEXT_TAG_BYTES;
#endif
    }

    return 1;
}

STATIC void demoLoadTask_8c01594c(Task *task)
{
    void *local;
    if (!RouteGetLatch_8c01432a()) {
        return;
    }

    var_currentCourse_8c1bb868.courseId_0x00 = var_demoBuf_8c1ba3c4[1];
    var_driveMode_8c1bb8c8 = var_demoBuf_8c1ba3c4[2];
    var_seed_8c157a64 = var_demoBuf_8c1ba3c4[3];
    local = var_demoBuffer_8c1bc828;
    ReplayCodecInit_8c02f320();
    ReplayCodecUnpack_8c02fa14(&var_demoBuf_8c1ba3c4[4], &local, var_demoBuf_8c1ba3c4[0]);
    syFree(var_demoBuf_8c1ba3c4);
    var_demoBuf_8c1ba3c4 = (int *) -1;
    TaskKill_8c014b66(task);
    GameStartSelectedCourse_8c01328c();
}

void TxtStartAttractDemo_8c0159ac()
{
    Task *created_task;
    void *created_state;
    TaskSpawn_8c014ae8(
        var_tasks_8c1ba3c8, demoLoadTask_8c01594c, &created_task, &created_state, 0
    );
    created_task->field_0x08 = 0;
    var_playMode_8c1bb8d0 = 2;
    var_isAttractDemo_8c1bb8d4 = 1;
    if (++var_demoIndex_8c1bb8d8 >= 20) {
        var_demoIndex_8c1bb8d8 = 0;
    }
    AsqInitQueues_8c011f36(1,0,0,0);
    AsqResetQueues_8c011f6c();
    AsqRequestDat_8c011182(
        "\\SYSTEM",
        init_demos_8c044154[var_demoIndex_8c1bb8d8].filename,
        &var_demoBuf_8c1ba3c4
    );
    var_activeTrafficPreset_8c227e14 =
        init_demos_8c044154[var_demoIndex_8c1bb8d8].trafficPresetId_0x04;
    var_activePedPreset_8c22822c =
        init_demos_8c044154[var_demoIndex_8c1bb8d8].pedPresetId_0x08;
    RouteClearLatch_8c014322();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteSetLatch_8c014330);
    return;
}