/* 8c014f54 */
#ifndef _014F54_TEXT_H
#define _014F54_TEXT_H

#include <shinobi.h>
#include "015ab8_title.h"

enum PLAY_MODE {
    PLAY_MODE_NORMAL   = 0,  /* story and free run */
    PLAY_MODE_PRACTICE = 1,
    PLAY_MODE_DEMO     = 2,  /* attract loop */
};

typedef struct {
    Uint32 on;   /* 0x00 */
    Sint8  x1;   /* 0x04 */
    Uint8  r;    /* 0x05 */
    Uint8  l;    /* 0x06 */
    Uint8  pad;  /* 0x07 */
} ReplayInput;

#define REPLAY_BUFFER_CAPACITY 54000
#define GLYPH_PALETTE_SIZE  4

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

extern ReplayInput var_demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY];
extern ReplayInput *var_demoCursor_8c225fa8;
extern Uint32 var_demoPrevOn_8c225fac;
extern void* var_8c1bc824;

void TxtDrawSprite_8c014f54(
    ResourceGroup *resource_group,
    int texture_id,
    float x,
    float y,
    float priority
);
void TxtInit_8c01524c();
void TxtDestroy_8c01529c();
TextBox* TxtCreateTextBox_8c0152fc(
    int x,
    int y,
    float priority,
    int width,
    int height,
    int x2,
    int y2,
    int enable_offset
);
void TxtDestroyTextBox_8c015410(TextBox *box);
int TxtPrepareTextBoxLayout_8c01543a(TextBox *box, char *text);
int TxtDrawTextbox_8c0155e0(TextBox *box, int limit);
void FUN_8c0159ac();

#endif /* _014F54_TEXT_H */
