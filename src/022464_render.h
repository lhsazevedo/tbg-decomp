/* 8c022464 */
#ifndef _022464_RENDER_H
#define _022464_RENDER_H

#include <njdef.h>
#include "014a9c_tasks.h"

/* =================
 * Type Declarations
 * =================
 */

/* IN reveals from black, OUT goes to black; HELD is the frame at black
 * before OUT finishes. */
typedef enum {
    FADE_PHASE_IDLE = 0,
    FADE_PHASE_IN   = 1,
    FADE_PHASE_OUT  = 2,
    FADE_PHASE_HELD = 3
} FadePhase;

/* Consumed once the phase is FADE_PHASE_IDLE. */
typedef enum {
    FADE_REQUEST_NONE = 0,
    FADE_REQUEST_IN   = 1,
    FADE_REQUEST_OUT  = 2
} FadeRequest;

typedef enum {
    MIRROR_NONE  = 0,
    MIRROR_LEFT  = 1,
    MIRROR_RIGHT = 2,
    /* Not a wing mirror: the view alongside the front door a run starts in. */
    MIRROR_DOOR  = 3
} MirrorSelect;

/* Overlays Task, up to the fields this task uses. */
typedef struct {
    TaskAction action;
    void *state;
    Uint32 frames_0x08;
} FadeInTask;

/* HOLD keeps solid black for three frames before the task ends. */
typedef enum {
    FADE_OUT_PHASE_RAMP = 0,
    FADE_OUT_PHASE_HOLD = 1
} FadeOutPhase;

typedef struct {
    TaskAction action;
    void *state;
    Uint32 frames_0x08;
    FadeOutPhase phase_0x0c;
} FadeOutTask;

/* Draw-command queues; RenderDrawFrame_8c022560 draws each with its own camera. */
typedef enum {
    RENDER_LAYER_MAIN   = 0,
    RENDER_LAYER_MIRROR = 1,
    RENDER_LAYER_CABIN  = 2,
    RENDER_LAYER_COUNT
} RenderLayer;

typedef void (*DrawFn)(int);
typedef void (*DrawFn2)(int, int);

/* Any other value is a no-op entry. */
typedef enum {
    /* Nothing in the game queues 0-4, only the callback entries 5-6. */
    DRAW_CMD_DRAW_OBJECT            = 0,
    DRAW_CMD_CNK_DRAW_OBJECT        = 1,
    DRAW_CMD_CNK_EASY_DRAW_OBJECT   = 2,
    DRAW_CMD_CNK_SIMPLE_DRAW_OBJECT = 3,
    DRAW_CMD_CNK_MOD_DRAW_OBJECT    = 4,
    DRAW_CMD_CALL                   = 5,
    DRAW_CMD_CALL2                  = 6
} DrawCommandType;

typedef struct {
    DrawCommandType type;
    union {
        /* 0-4; type 4 skips njSetTexture */
        struct {
            NJS_MATRIX *matrix;
            NJS_TEXLIST *texlist;
            union {
                NJS_OBJECT *object;        /* type 0 */
                NJS_CNK_OBJECT *cnkObject; /* types 1-4 */
            } obj;
        } draw;
        /* 5 */
        struct {
            DrawFn fn;
            int arg0;
        } call;
        /* 6 */
        struct {
            DrawFn2 fn;
            int arg0;
            int arg1;
        } call2;
    } u;
} DrawCommand;

/* =======================
 * Non-initialized Globals
 * =======================
 */

extern NJS_CAMERA* var_drawCamera_8c226558; // camera for the layer being drawn
extern int var_viewLayout_8c22655c; // 0: main view + wing mirror; 1, 2: tall mirror inset, 1 adding the cabin view
extern int var_sceneVisible_8c226560; // cleared once a fade-out reaches black
extern FadeRequest var_fadeRequest_8c226564;
extern Bool var_isFading_8c226568;
extern void (*var_fadeCompleteCallback_8c22656c)(void); // -1 when unset

/* =========
 * Functions
 * =========
 */

void RenderResetQueues_8c02239c(void);
void RenderQueueDraw_8c0223ea(RenderLayer layer, DrawFn fn, int arg0);
void RenderQueueDraw2_8c022420(RenderLayer layer, DrawFn2 fn, int arg0, int arg1);
void RenderDrawFrame_8c022560(void);
void RenderStartRunFade_8c0228a2(void);
void RenderDrawFrameMainOnly_8c022910(void);
void RenderStartFadeIn_8c022a9c(int frames);
void RenderStartFadeOut_8c022b60(int frames);

#endif // _022464_RENDER_H
