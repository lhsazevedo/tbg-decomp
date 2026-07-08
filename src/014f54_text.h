/* 8c014f54 */
#ifndef _014F54_TEXT_H
#define _014F54_TEXT_H

#include <shinobi.h>

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

extern enum PLAY_MODE var_playMode_8c1bb8d0;
extern ReplayInput demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY];
extern ReplayInput *demoCursor_8c225fa8;
extern Uint32 demoPrevOn_8c225fac;

#endif /* _014F54_TEXT_H */
