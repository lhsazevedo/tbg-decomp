/* 8c022464 */
#ifndef _022464_RENDER_H
#define _022464_RENDER_H

#include <njdef.h>
#include "014a9c_tasks.h"

/* =================
 * Type Declarations
 * =================
 */

/* var_fadePhase_8c227d7c: the fade state machine's current phase. */
typedef enum {
    FADE_PHASE_IDLE = 0,
    FADE_PHASE_IN   = 1,
    FADE_PHASE_OUT  = 2,
    FADE_PHASE_HELD = 3
} FadePhase;

/* var_fadeRequest_8c226564: a transition requested of RenderDrawFrame_8c022560 /
 * RenderDrawFrameMainOnly_8c022910, consumed once phase is FADE_PHASE_IDLE. */
typedef enum {
    FADE_REQUEST_NONE = 0,
    FADE_REQUEST_IN   = 1,
    FADE_REQUEST_OUT  = 2
} FadeRequest;

/* BusState.mirror_0x268: which mirror view is up. */
typedef enum {
    MIRROR_NONE  = 0,
    MIRROR_LEFT  = 1,
    MIRROR_RIGHT = 2,
    /* Not a wing mirror: the door-side view a run starts in
     * (busInitPlaceBus_8c023310) -- BusCameraUpdateMirror_8c025604 puts
     * the camera close alongside the front door instead of back down the
     * flank. RenderDrawFrame_8c022560 draws it like the wing mirrors but with no
     * label sprite. */
    MIRROR_DOOR  = 3
} MirrorSelect;

/* Task private state for fadeInTask_8c022a54/RenderStartFadeIn_8c022a9c. Its fields
 * match Task (see 014a9c_tasks.h) up through the one this task uses -- the
 * real slot is a full Task (var_tasks_8c1ba3c8 is an array of those), and
 * the rest is unused by this callback. */
typedef struct {
    TaskAction action;
    void *state;
    Uint32 frames_0x08;
} FadeInTask;

/* FadeOutTask.phase_0x0c: RAMP while ramping var_fadeProgress_8c227d80 up to
 * full black, HOLD while holding solid black for the two ticks before the
 * task frees itself. */
typedef enum {
    FADE_OUT_PHASE_RAMP = 0,
    FADE_OUT_PHASE_HOLD = 1
} FadeOutPhase;

/* Task private state for fadeOutTask_8c022ad0/RenderStartFadeOut_8c022b60. Same
 * layout as FadeInTask above, but field_0x0c is reused as phase_0x0c
 * instead of a pointer. */
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

/* Opaque per-entry callbacks for DrawCommand types 5/6 (see below),
 * queued by RenderQueueDraw_8c0223ea/RenderQueueDraw2_8c022420. They pass whatever
 * function pointer their caller supplies, so there is no fixed SDK
 * signature to name these after. */
typedef void (*DrawFn)(int);
typedef void (*DrawFn2)(int, int);

/* DrawCommand.type: which arm of its union holds the remaining 12 bytes.
 * Anything outside these 7 values is a no-op entry. */
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

/* One 16-byte entry in var_drawCommands_8c22657c's per-layer draw-command
 * queue. */
typedef struct {
    DrawCommandType type;
    union {
        /* 0-4: an nj*DrawObject variant, type 4 skipping njSetTexture */
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

extern NJS_CAMERA* var_drawCamera_8c226558; // camera for the layer being drawn; RenderDrawFrame_8c022560 picks main/mirror/cabin
extern int var_viewLayout_8c22655c; // bus-stop-arrival overlay layout (0-2) drawn by RenderDrawFrame_8c022560; despite the SDK Bool this held before, values above 1 are reachable (switch in RenderDrawFrame_8c022560 handles 0-2)
extern int var_sceneVisible_8c226560; // gates RenderDrawFrame_8c022560's bus-stop-arrival draw; cleared once its fade-out finishes
extern FadeRequest var_fadeRequest_8c226564; // requested fade transition, consumed by RenderDrawFrame_8c022560
extern Bool var_isFading_8c226568;
extern void (*var_fadeCompleteCallback_8c22656c)(void); // fade-complete callback; sentinel -1 (0xffffffff) means unset

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
