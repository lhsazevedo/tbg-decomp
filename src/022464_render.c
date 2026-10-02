/* @unit Render */
#include <shinobi.h>
#include "includes.h" /* STATIC */
#include <njdef.h>
#include "014f54_sprite.h"
#include "014a9c_tasks.h"
#include "015034_text.h"
#include "1ba1c8_globals.h"
#include "01e27c_practice_menu.h"
#include "0129cc_game.h"
#include "022464_render.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define FADE_NO_CALLBACK ((void (*)(void))-1)

/* ====================
 * Non-initialized Globals
 * ====================
 */

NJS_CAMERA* var_drawCamera_8c226558;
int var_viewLayout_8c22655c;
int var_sceneVisible_8c226560;
FadeRequest var_fadeRequest_8c226564;
Bool var_isFading_8c226568;
void (*var_fadeCompleteCallback_8c22656c)(void);
STATIC int var_drawCommandCount_8c226570[RENDER_LAYER_COUNT];
STATIC DrawCommand var_drawCommands_8c22657c[RENDER_LAYER_COUNT][128];
STATIC FadePhase var_fadePhase_8c227d7c;
STATIC Uint32 var_fadeProgress_8c227d80; // overlay alpha: bits 24-31 for fade-ins, bits 16-23 for fade-outs

/* ====================
 * Initialized Globals
 * ====================
 */

STATIC NJS_TEXTURE_VTX init_mirrorQuadLeft_8c045438[4] = {
    {   /* [0] */
        /* x, y, z */ 32.0f, 32.0f, 0.8403361f,
        /* u, v */ 0.625f, 0.0f,
        /* col */ ARGB(0xff, 0xff, 0xff, 0xff),
    },
    {   /* [1] */
        /* x, y, z */ 32.0f, 288.0f, 0.8403361f,
        /* u, v */ 0.625f, 0.49804688f,
        /* col */ ARGB(0xff, 0xff, 0xff, 0xff),
    },
    {   /* [2] */
        /* x, y, z */ 192.0f, 32.0f, 0.8403361f,
        /* u, v */ 0.00390625f, 0.0f,
        /* col */ ARGB(0xff, 0xff, 0xff, 0xff),
    },
    {   /* [3] */
        /* x, y, z */ 192.0f, 288.0f, 0.8403361f,
        /* u, v */ 0.00390625f, 0.49804688f,
        /* col */ ARGB(0xff, 0xff, 0xff, 0xff),
    },
};
STATIC NJS_TEXTURE_VTX init_mirrorQuadRight_8c045498[4] = {
    {   /* [0] */
        /* x, y, z */ 448.0f, 32.0f, 0.8403361f,
        /* u, v */ 0.625f, 0.0f,
        /* col */ ARGB(0xff, 0xff, 0xff, 0xff),
    },
    {   /* [1] */
        /* x, y, z */ 448.0f, 288.0f, 0.8403361f,
        /* u, v */ 0.625f, 0.49804688f,
        /* col */ ARGB(0xff, 0xff, 0xff, 0xff),
    },
    {   /* [2] */
        /* x, y, z */ 608.0f, 32.0f, 0.8403361f,
        /* u, v */ 0.00390625f, 0.0f,
        /* col */ ARGB(0xff, 0xff, 0xff, 0xff),
    },
    {   /* [3] */
        /* x, y, z */ 608.0f, 288.0f, 0.8403361f,
        /* u, v */ 0.00390625f, 0.49804688f,
        /* col */ ARGB(0xff, 0xff, 0xff, 0xff),
    },
};
STATIC NJS_TEXTURE_VTX init_mirrorQuadTall_8c0454f8[4] = {
    {   /* [0] */
        /* x, y, z */ 32.0f, 32.0f, 0.8403361f,
        /* u, v */ 0.625f, 0.0f,
        /* col */ ARGB(0xff, 0xff, 0xff, 0xff),
    },
    {   /* [1] */
        /* x, y, z */ 32.0f, 384.0f, 0.8403361f,
        /* u, v */ 0.625f, 0.6855469f,
        /* col */ ARGB(0xff, 0xff, 0xff, 0xff),
    },
    {   /* [2] */
        /* x, y, z */ 192.0f, 32.0f, 0.8403361f,
        /* u, v */ 0.00390625f, 0.0f,
        /* col */ ARGB(0xff, 0xff, 0xff, 0xff),
    },
    {   /* [3] */
        /* x, y, z */ 192.0f, 384.0f, 0.8403361f,
        /* u, v */ 0.00390625f, 0.6855469f,
        /* col */ ARGB(0xff, 0xff, 0xff, 0xff),
    },
};
STATIC NJS_POINT2 init_clipMirrorLeft_8c045558[2] = {
    /* [0] */ { 1.0f, 1.0f },
    /* [1] */ { 5.0f, 8.0f },
};
STATIC NJS_POINT2 init_clipMirrorRight_8c045568[2] = {
    /* [0] */ { 14.0f, 1.0f },
    /* [1] */ { 18.0f, 8.0f },
};
STATIC NJS_POINT2 init_clipMirrorView_8c045578[2] = {
    /* [0] */ { 0.0f, 0.0f },
    /* [1] */ { 4.0f, 7.0f },
};
STATIC NJS_POINT2 init_clipMirrorViewTall_8c045588[2] = {
    /* [0] */ { 0.0f, 0.0f },
    /* [1] */ { 4.0f, 10.0f },
};
STATIC NJS_POINT2 init_clipCabin_8c045598[2] = {
    /* [0] */ { 6.0f, 2.0f },
    /* [1] */ { 18.0f, 9.0f },
};
STATIC NJS_POLYGON_VTX init_fadeQuad_8c0455a8[4] = {
    /* [0] */ { 0.0f, 0.0f, 1.0f, ARGB(0, 0, 0, 0) },
    /* [1] */ { 0.0f, 480.0f, 1.0f, ARGB(0, 0, 0, 0) },
    /* [2] */ { 640.0f, 0.0f, 1.0f, ARGB(0, 0, 0, 0) },
    /* [3] */ { 640.0f, 480.0f, 1.0f, ARGB(0, 0, 0, 0) },
};
STATIC NJS_SCREEN init_screenFull_8c0455e8 = {
    /* dist */ 500.0f,
    /* w, h */ 640.0f, 480.0f,
    /* cx, cy */ 320.0f, 240.0f,
};
STATIC NJS_SCREEN init_screenMirror_8c0455fc = {
    /* dist */ 500.0f,
    /* w, h */ 160.0f, 256.0f,
    /* cx, cy */ 80.0f, 128.0f,
};
STATIC NJS_SCREEN init_screenMirrorTall_8c045610 = {
    /* dist */ 500.0f,
    /* w, h */ 160.0f, 352.0f,
    /* cx, cy */ 80.0f, 176.0f,
};
STATIC NJS_SCREEN init_screenCabin_8c045624 = {
    /* dist */ 500.0f,
    /* w, h */ 640.0f, 480.0f,
    /* cx, cy */ 400.0f, 192.0f,
};

/* ====================
 * Functions
 * ====================
 */

void RenderResetQueues_8c02239c(void)
{
    int i;

    for (i = 0; i < RENDER_LAYER_COUNT; i++) {
        var_drawCommandCount_8c226570[i] = 0;
    }
}

void RenderQueueDraw_8c0223ea(RenderLayer layer, DrawFn fn, int arg0)
{
    DrawCommand *cmd;

    if (var_drawCommandCount_8c226570[layer] < 0x80) {
        cmd = var_drawCommands_8c22657c[layer]
            + var_drawCommandCount_8c226570[layer];
        cmd->type = DRAW_CMD_CALL;
        cmd->u.call.fn = fn;
        cmd->u.call.arg0 = arg0;
        var_drawCommandCount_8c226570[layer]++;
    }
}

void RenderQueueDraw2_8c022420(RenderLayer layer, DrawFn2 fn, int arg0, int arg1)
{
    DrawCommand *cmd;

    if (var_drawCommandCount_8c226570[layer] < 0x80) {
        cmd = var_drawCommands_8c22657c[layer]
            + var_drawCommandCount_8c226570[layer];
        cmd->type = DRAW_CMD_CALL2;
        cmd->u.call2.fn = fn;
        cmd->u.call2.arg0 = arg0;
        cmd->u.call2.arg1 = arg1;
        var_drawCommandCount_8c226570[layer]++;
    }
}

STATIC void drawLayer_8c022464(RenderLayer layer)
{
  int count;
  int i;
  DrawCommandType type;
  DrawCommand *cmd;

  count = var_drawCommandCount_8c226570[layer];
  cmd = var_drawCommands_8c22657c[layer];
  for (i = 0; i < count; i++, cmd++) {
    njSetCamera(var_drawCamera_8c226558);
    type = cmd->type;
    switch (type) {
      case DRAW_CMD_DRAW_OBJECT:
      case DRAW_CMD_CNK_DRAW_OBJECT:
      case DRAW_CMD_CNK_EASY_DRAW_OBJECT:
      case DRAW_CMD_CNK_SIMPLE_DRAW_OBJECT:
      case DRAW_CMD_CNK_MOD_DRAW_OBJECT:
        njMultiMatrix(0, cmd->u.draw.matrix);
        if (type != DRAW_CMD_CNK_MOD_DRAW_OBJECT) {
          njSetTexture(cmd->u.draw.texlist);
        }
        switch (type) {
          case DRAW_CMD_DRAW_OBJECT:
            njDrawObject(cmd->u.draw.obj.object);
            break;
          case DRAW_CMD_CNK_DRAW_OBJECT:
            njCnkDrawObject(cmd->u.draw.obj.cnkObject);
            break;
          case DRAW_CMD_CNK_EASY_DRAW_OBJECT:
            njCnkEasyDrawObject(cmd->u.draw.obj.cnkObject);
            break;
          case DRAW_CMD_CNK_SIMPLE_DRAW_OBJECT:
            njCnkSimpleDrawObject(cmd->u.draw.obj.cnkObject);
            break;
          case DRAW_CMD_CNK_MOD_DRAW_OBJECT:
            njCnkModDrawObject(cmd->u.draw.obj.cnkObject);
            break;
          default:
            break;
        }
        break;
      case DRAW_CMD_CALL:
        cmd->u.call.fn(cmd->u.call.arg0);
        break;
      case DRAW_CMD_CALL2:
        cmd->u.call2.fn(cmd->u.call2.arg0, cmd->u.call2.arg1);
        break;
      default:
        break;
    }
  }
}

void RenderDrawFrame_8c022560(void)
{
  if (var_sceneVisible_8c226560 != 0) {
    switch (var_viewLayout_8c22655c) {
    case 0:
      njControl3D(NJD_CONTROL_3D_MODEL_CLIP);
      switch (var_busState_8c1bb9d0.mirror_0x268) {
        case MIRROR_NONE:
          njUserClipping(NJD_CLIP_DISABLE, init_clipMirrorView_8c045578);
          break;
        case MIRROR_LEFT:
        case MIRROR_RIGHT:
        default:
          njUserClipping(NJD_CLIP_INSIDE, init_clipMirrorView_8c045578);
          njSetScreen(&init_screenMirror_8c0455fc);
          var_drawCamera_8c226558 = &var_mirrorCamera_8c1bb944;
          drawLayer_8c022464(RENDER_LAYER_MIRROR);
          njSetTexture(&init_renderTexlist_8c03bf44);
          njRenderTextureNumG(999);
          if (var_busState_8c1bb9d0.mirror_0x268 == MIRROR_LEFT) {
            njUserClipping(NJD_CLIP_INSIDE, init_clipMirrorLeft_8c045558);
            njDrawTexture(init_mirrorQuadLeft_8c045438, 4, 999, 0);
            njUserClipping(NJD_CLIP_DISABLE, init_clipMirrorLeft_8c045558);
            SpriteDraw_8c014f54((ResourceGroup *)&var_busStopTexlist_8c1bc424, 0x27, 0.0f, 0.0f, -1.17f);
            njUserClipping(NJD_CLIP_OUTSIDE, init_clipMirrorLeft_8c045558);
          }
          else if (var_busState_8c1bb9d0.mirror_0x268 == MIRROR_RIGHT) {
            njUserClipping(NJD_CLIP_INSIDE, init_clipMirrorRight_8c045568);
            njDrawTexture(init_mirrorQuadRight_8c045498, 4, 999, 0);
            njUserClipping(NJD_CLIP_DISABLE, init_clipMirrorRight_8c045568);
            SpriteDraw_8c014f54((ResourceGroup *)&var_busStopTexlist_8c1bc424, 0x29, 0.0f, 0.0f, -1.17f);
            njUserClipping(NJD_CLIP_OUTSIDE, init_clipMirrorRight_8c045568);
          }
          break;
      }
      njSetScreen(&init_screenFull_8c0455e8);
      var_drawCamera_8c226558 = &var_camera_8c1bb904;
      drawLayer_8c022464(RENDER_LAYER_MAIN);
      break;
    case 1:
      njControl3D(NJD_CONTROL_3D_MODEL_CLIP);
      njUserClipping(NJD_CLIP_INSIDE, init_clipMirrorViewTall_8c045588);
      njSetScreen(&init_screenMirrorTall_8c045610);
      var_drawCamera_8c226558 = &var_mirrorCamera_8c1bb944;
      drawLayer_8c022464(RENDER_LAYER_MIRROR);
      njSetTexture(&init_renderTexlist_8c03bf44);
      njRenderTextureNumG(999);
      njUserClipping(NJD_CLIP_INSIDE, init_clipCabin_8c045598);
      njSetScreen(&init_screenCabin_8c045624);
      var_drawCamera_8c226558 = &var_cabinCamera_8c1bb984;
      drawLayer_8c022464(RENDER_LAYER_CABIN);
      njUserClipping(NJD_CLIP_OUTSIDE, init_clipCabin_8c045598);
      njSetScreen(&init_screenFull_8c0455e8);
      njDrawTexture(init_mirrorQuadTall_8c0454f8, 4, 999, 0);
      var_drawCamera_8c226558 = &var_camera_8c1bb904;
      drawLayer_8c022464(RENDER_LAYER_MAIN);
      njUserClipping(NJD_CLIP_DISABLE, init_clipCabin_8c045598);
      SpriteDraw_8c014f54((ResourceGroup *)&var_busStopTexlist_8c1bc424, 0x28, 0.0f, 0.0f, -1.17f);
      break;
    case 2:
      njControl3D(NJD_CONTROL_3D_MODEL_CLIP);
      njUserClipping(NJD_CLIP_INSIDE, init_clipMirrorViewTall_8c045588);
      njSetScreen(&init_screenMirrorTall_8c045610);
      var_drawCamera_8c226558 = &var_mirrorCamera_8c1bb944;
      drawLayer_8c022464(RENDER_LAYER_MIRROR);
      njSetTexture(&init_renderTexlist_8c03bf44);
      njRenderTextureNumG(999);
      njUserClipping(NJD_CLIP_DISABLE, init_clipMirrorViewTall_8c045588);
      njSetScreen(&init_screenFull_8c0455e8);
      njDrawTexture(init_mirrorQuadTall_8c0454f8, 4, 999, 0);
      SpriteDraw_8c014f54((ResourceGroup *)&var_busStopTexlist_8c1bc424, 0x2a, 0.0f, 0.0f, -1.17f);
      var_drawCamera_8c226558 = &var_camera_8c1bb904;
      drawLayer_8c022464(RENDER_LAYER_MAIN);
      break;
    }
  }
  njUserClipping(NJD_CLIP_DISABLE, init_clipCabin_8c045598);

  if (var_fadePhase_8c227d7c == FADE_PHASE_IDLE) {
    if (var_fadeRequest_8c226564 == FADE_REQUEST_IN) {
      var_fadePhase_8c227d7c = FADE_PHASE_IN;
      var_fadeProgress_8c227d80 = 0xff000000;
      var_isFading_8c226568 = 1;
    }
    else if (var_fadeRequest_8c226564 == FADE_REQUEST_OUT) {
      var_fadePhase_8c227d7c = FADE_PHASE_OUT;
      var_fadeProgress_8c227d80 = 0;
      var_isFading_8c226568 = 1;
    }
    else {
      return;
    }
  }

  switch (var_fadePhase_8c227d7c) {
    case FADE_PHASE_IN: {
      Uint32 progress = var_fadeProgress_8c227d80 - 0x4400000;
      var_fadeProgress_8c227d80 = progress;
      if (progress > 0x1000000) {
        Uint32 color = progress & 0xff000000;
        init_fadeQuad_8c0455a8[0].col = color;
        init_fadeQuad_8c0455a8[1].col = color;
        init_fadeQuad_8c0455a8[2].col = color;
        init_fadeQuad_8c0455a8[3].col = color;
        njDrawPolygon(init_fadeQuad_8c0455a8, 4, 1);
        return;
      }
      break;
    }
    case FADE_PHASE_OUT: {
      Uint32 color;
      var_fadeProgress_8c227d80 = var_fadeProgress_8c227d80 + 0x44000;
      if (var_fadeProgress_8c227d80 > 0xffffff) {
        var_fadePhase_8c227d7c = FADE_PHASE_HELD;
        var_sceneVisible_8c226560 = 0;
        var_fadeProgress_8c227d80 = 0xff0000;
      }
      color = (var_fadeProgress_8c227d80 << 8) & 0xff000000;
      init_fadeQuad_8c0455a8[0].col = color;
      init_fadeQuad_8c0455a8[1].col = color;
      init_fadeQuad_8c0455a8[2].col = color;
      init_fadeQuad_8c0455a8[3].col = color;
      njDrawPolygon(init_fadeQuad_8c0455a8, 4, 1);
      return;
    }
    case FADE_PHASE_HELD:
      break;
    default:
      return;
  }

  /* Fade-in done, or the held frame at black: finish. */
  var_fadePhase_8c227d7c = FADE_PHASE_IDLE;
  var_fadeRequest_8c226564 = FADE_REQUEST_NONE;
  var_isFading_8c226568 = 0;
  if (var_fadeCompleteCallback_8c22656c != FADE_NO_CALLBACK) {
    (*var_fadeCompleteCallback_8c22656c)();
    var_fadeCompleteCallback_8c22656c = FADE_NO_CALLBACK;
  }
}

void RenderStartRunFade_8c0228a2(void)
{
  if ((var_playMode_8c1bb8d0 == PLAY_MODE_NORMAL) || ((var_practiceRules_8c226410 & 8) == 8)) {
    var_viewLayout_8c22655c = 1;
  }
  else {
    var_viewLayout_8c22655c = 0;
  }
  var_fadePhase_8c227d7c = FADE_PHASE_IDLE;
  var_sceneVisible_8c226560 = 1;
  var_fadeRequest_8c226564 = FADE_REQUEST_IN;
  var_fadeCompleteCallback_8c22656c = FADE_NO_CALLBACK;
  var_isFading_8c226568 = 1;
}

/* Unlike RenderDrawFrame_8c022560, clearing var_isFading_8c226568 cuts a fade
 * short. */
void RenderDrawFrameMainOnly_8c022910(void)
{
  if (var_sceneVisible_8c226560 != 0) {
    njControl3D(NJD_CONTROL_3D_MODEL_CLIP);
    njUserClipping(NJD_CLIP_DISABLE, init_clipMirrorView_8c045578);
    njSetScreen(&init_screenFull_8c0455e8);
    var_drawCamera_8c226558 = &var_camera_8c1bb904;
    drawLayer_8c022464(RENDER_LAYER_MAIN);
  }
  if (var_fadePhase_8c227d7c == FADE_PHASE_IDLE) {
    if (var_fadeRequest_8c226564 == FADE_REQUEST_IN) {
      var_fadePhase_8c227d7c = FADE_PHASE_IN;
      var_fadeProgress_8c227d80 = 0xff000000;
      var_isFading_8c226568 = 1;
    }
    else if (var_fadeRequest_8c226564 == FADE_REQUEST_OUT) {
      var_fadePhase_8c227d7c = FADE_PHASE_OUT;
      var_fadeProgress_8c227d80 = 0;
      var_isFading_8c226568 = 1;
    }
    else {
      return;
    }
  }

  switch (var_fadePhase_8c227d7c) {
    case FADE_PHASE_IN: {
      Uint32 progress = var_fadeProgress_8c227d80 - 0x4400000;
      var_fadeProgress_8c227d80 = progress;
      if (progress > 0x1000000 && var_isFading_8c226568 != 0) {
        Uint32 color = progress & 0xff000000;
        init_fadeQuad_8c0455a8[0].col = color;
        init_fadeQuad_8c0455a8[1].col = color;
        init_fadeQuad_8c0455a8[2].col = color;
        init_fadeQuad_8c0455a8[3].col = color;
        njDrawPolygon(init_fadeQuad_8c0455a8, 4, 1);
        return;
      }
      break;
    }
    case FADE_PHASE_OUT: {
      Uint32 color;
      var_fadeProgress_8c227d80 = var_fadeProgress_8c227d80 + 0x44000;
      if (var_fadeProgress_8c227d80 > 0xffffff || var_isFading_8c226568 == 0) {
        var_fadePhase_8c227d7c = FADE_PHASE_HELD;
        var_sceneVisible_8c226560 = 0;
        var_fadeProgress_8c227d80 = 0xff0000;
      }
      color = (var_fadeProgress_8c227d80 << 8) & 0xff000000;
      init_fadeQuad_8c0455a8[0].col = color;
      init_fadeQuad_8c0455a8[1].col = color;
      init_fadeQuad_8c0455a8[2].col = color;
      init_fadeQuad_8c0455a8[3].col = color;
      njDrawPolygon(init_fadeQuad_8c0455a8, 4, 1);
      return;
    }
    case FADE_PHASE_HELD:
      break;
    default:
      return;
  }

  /* Fade-in done or cut short, or the held frame at black: finish. */
  var_fadePhase_8c227d7c = FADE_PHASE_IDLE;
  var_fadeRequest_8c226564 = FADE_REQUEST_NONE;
  var_isFading_8c226568 = 0;
  if (var_fadeCompleteCallback_8c22656c != FADE_NO_CALLBACK) {
    (*var_fadeCompleteCallback_8c22656c)();
    var_fadeCompleteCallback_8c22656c = FADE_NO_CALLBACK;
  }
}

STATIC void fadeInTask_8c022a54(FadeInTask *task, void *state)
{
  Uint32 level;
  (void)state;

  level = var_fadeProgress_8c227d80 - 0xff000000 / task->frames_0x08;
  var_fadeProgress_8c227d80 = level;
  if (level > 0x1000000) {
    Uint32 color = level & 0xff000000;
    init_fadeQuad_8c0455a8[0].col = color;
    init_fadeQuad_8c0455a8[1].col = color;
    init_fadeQuad_8c0455a8[2].col = color;
    init_fadeQuad_8c0455a8[3].col = color;
    njDrawPolygon(init_fadeQuad_8c0455a8, 4, 1);
    return;
  }
  var_isFading_8c226568 = 0;
  TaskKill_8c014b66((Task *)task);
}

void RenderStartFadeIn_8c022a9c(int frames)
{
  FadeInTask *task;
  void *state;

  TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, fadeInTask_8c022a54, (Task **)&task, &state, 0);
  task->frames_0x08 = frames;
  var_fadeProgress_8c227d80 = 0xff000000;
  var_isFading_8c226568 = 1;
}

STATIC void fadeOutTask_8c022ad0(FadeOutTask *task, void *state)
{
  FadeOutPhase phase;
  Uint32 color;
  (void)state;

  phase = task->phase_0x0c;
  switch (phase) {
    case FADE_OUT_PHASE_RAMP:
      var_fadeProgress_8c227d80 = var_fadeProgress_8c227d80 + 0xff0000 / task->frames_0x08;
      if (var_fadeProgress_8c227d80 > 0xffffff) {
        var_fadeProgress_8c227d80 = 0xff0000;
        njSetBackColor(0, 0, 0);
        task->frames_0x08 = 0; /* reused as the hold counter */
        task->phase_0x0c = FADE_OUT_PHASE_HOLD;
      }
      break;
    case FADE_OUT_PHASE_HOLD: {
      Uint32 holdCount = task->frames_0x08;
      task->frames_0x08 = holdCount + 1;
      if (holdCount > 1) {
        var_isFading_8c226568 = 0;
        TaskKill_8c014b66((Task *)task);
        njSetBackColor(0, 0, 0);
      }
      var_fadeProgress_8c227d80 = 0xff0000;
      break;
    }
  }
  color = (var_fadeProgress_8c227d80 & 0xff0000) << 8;
  init_fadeQuad_8c0455a8[0].col = color;
  init_fadeQuad_8c0455a8[1].col = color;
  init_fadeQuad_8c0455a8[2].col = color;
  init_fadeQuad_8c0455a8[3].col = color;
  njDrawPolygon(init_fadeQuad_8c0455a8, 4, 1);
}

void RenderStartFadeOut_8c022b60(int frames)
{
  FadeOutTask *task;
  void *state;

  TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, fadeOutTask_8c022ad0, (Task **)&task, &state, 0);
  task->frames_0x08 = frames;
  task->phase_0x0c = FADE_OUT_PHASE_RAMP;
  var_fadeProgress_8c227d80 = 0;
  var_isFading_8c226568 = 1;
}
