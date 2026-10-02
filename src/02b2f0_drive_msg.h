/* Drive messages: the HUD banners that call out a penalty or award. */
#ifndef _02B2F0_DRIVE_MSG_H
#define _02B2F0_DRIVE_MSG_H

/* One banner: a {count, id...} row of init_penaltyMsgGlyphs_8c04c35c.
 * [0] is the newest, older ones shift back as each new one arrives. Typed
 * out one glyph per two frames, then held 60 frames. */
typedef struct {
    int count;
    int *ids;
    float x;          /* row's left edge, centered: (640 - 32 * count) / 2 */
    int revealed;     /* glyphs typed out so far; revealCounter >> 1 */
    int revealCounter;
    int holdFrames;   /* counts down once fully revealed; 0 = slot free */
} DriveMsgSlot;
extern DriveMsgSlot var_driveMsgQueue_8c228564[4];

/* Once the run has passed, draws only the run-passed mark. */
void DriveMsgDraw_8c02b388(int unused);

#endif // _02B2F0_DRIVE_MSG_H
