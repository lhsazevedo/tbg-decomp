/* 8c015034 */
#ifndef _015034_TEXT_H
#define _015034_TEXT_H

#include <shinobi.h>

enum PLAY_MODE {
    PLAY_MODE_NORMAL   = 0,  /* story and free run */
    PLAY_MODE_PRACTICE = 1,
    PLAY_MODE_DEMO     = 2,  /* attract loop */
};

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
int TxtDrawTextBox_8c0155e0(TextBox *box, int limit);
void TxtStartAttractDemo_8c0159ac();

#endif /* _015034_TEXT_H */
