/* 8c02a9fc */
#ifndef _02A9FC_MESSAGE_BOX_H
#define _02A9FC_MESSAGE_BOX_H

#include <shinobi.h>

/* table index (into the EventEntry array pointed to by var_routeEvents_8c22851c)
 * chosen by EventPickForSegment_8c02b170, consumed by
 * EventApplyFlags_8c02b292 */
extern int var_selectedEventEntry_8c228478;
extern int var_messageBoxActive_8c22847c;

void MessageBoxClearAssets_8c02aa28(void);
void MessageBoxRequestAssets_8c02aa36(void);
void MessageBoxStart_8c02ad8c(void);
void MessageBoxFreeAssets_8c02adee(void);
void MessageBoxOpenTextbox_8c02ae3e(int x, int y, float priority, int width, int height, int x2, int y2, int enable_offset);
int MessageBoxSetText_8c02aefc(char *text);
int MessageBoxDrawText_8c02af1c(int limit);
void MessageBoxFreeTextboxes_8c02af32(void);

#endif // _02A9FC_MESSAGE_BOX_H
