/* @unit Signal */
/* 8c028258 */
#include <shinobi.h>

#include "028258_signal.h"
#include "014a9c_tasks.h" /* Task */
#include "020914_ground_query.h" /* GroundQueryFindPolygon_8c020914, GroundQueryResult */
#include "020b6c_ground_probe.h"
#include "022464_render.h" /* RenderQueueDraw_8c0223ea, RenderQueueDraw2_8c022420 */
#include "026710_traffic.h" /* TrafficMarkSignalIdsInUse_8c026dcc */
#include "027958_bus_draw.h" /* BusDrawSignal_8c0281ac */
#include "02b464_grading.h" /* var_runState_8c2285c4 */
#include "024b4c_bus_camera.h"
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */

/* ====================
 * Non-initialized Globals
 * ====================
 */

Task *var_trafficSignalTasks_8c227e20;
int *var_trafficSignalFrames_8c227e24;
STATIC TrafficSignal **var_trafficSignalStates_8c227e28; /* per-id TrafficSignal* */
/* Per crossing, re-marked every frame: a pedestrian is on it, which traffic
 * reads; a vehicle is on it, which holds pedestrians back. */
STATIC int var_pedCrossingFlags_8c227e2c[128];
STATIC int var_crossingOccupiedFlags_8c22802c[128];

/* ====================
 * Functions
 * ====================
 */

STATIC void headTask_8c028258(Task *task, TrafficSignal *obj)
{
    float dx, dz;
    float dist;

    if (++obj->counter_0x04 >= obj->durations_0x08[obj->frame_0x0c]) {
        obj->counter_0x04 = 0;
        if (++obj->frame_0x0c > 2) {
            obj->frame_0x0c = 0;
        }
        var_trafficSignalFrames_8c227e24[obj->id_0x00] = obj->frame_0x0c;
    }

    obj->drawB_0xcc = 0;
    obj->drawA_0xc8 = 0;

    if (obj->posA_0x1c.x != 0.0f) {
        dx = var_busState_8c1bb9d0.posX_0x2fc - obj->posA_0x1c.x;
        dz = var_busState_8c1bb9d0.posZ_0x304 - obj->posA_0x1c.z;
        dist = njSqrt(dx * dx + dz * dz);
        if (dist < 200.0f) {
            RenderQueueDraw2_8c022420(RENDER_LAYER_MAIN, BusDrawSignal_8c0281ac, (int)obj, (int)&obj->mtxA_0x34);
            obj->drawA_0xc8 = 1;
        }
    }

    if (obj->posB_0x28.x != 0.0f) {
        dx = var_busState_8c1bb9d0.posX_0x2fc - obj->posB_0x28.x;
        dz = var_busState_8c1bb9d0.posZ_0x304 - obj->posB_0x28.z;
        dist = njSqrt(dx * dx + dz * dz);
        if (dist < 200.0f) {
            RenderQueueDraw2_8c022420(RENDER_LAYER_MAIN, BusDrawSignal_8c0281ac, (int)obj, (int)&obj->mtxB_0x74);
            obj->drawB_0xcc = 1;
        }
    }
}

STATIC void attachmentTask_8c02833c(Task *task, TrafficSignal *obj)
{
    TrafficSignal *linked = obj->attachedTo_0xd0;

    if (linked->frame_0x0c != 0) {
        obj->frame_0x0c = 1;
        obj->drawA_0xc8 = 0;
    } else if (linked->counter_0x04 <= obj->counter_0x04) {
        obj->frame_0x0c = 1;
        obj->drawA_0xc8 = 1;
    } else {
        obj->frame_0x0c = 0;
        obj->drawA_0xc8 = 0;
    }

    var_trafficSignalFrames_8c227e24[obj->id_0x00] = obj->frame_0x0c;

    if (linked->drawA_0xc8 != 0) {
        RenderQueueDraw2_8c022420(RENDER_LAYER_MAIN, BusDrawSignalAttachment_8c028206, (int)obj, (int)&linked->mtxA_0x34);
    }
    if (linked->drawB_0xcc != 0) {
        RenderQueueDraw2_8c022420(RENDER_LAYER_MAIN, BusDrawSignalAttachment_8c028206, (int)obj, (int)&linked->mtxB_0x74);
    }
}

STATIC void setLightCallback_8c0283d4(int arg0)
{
    njCnkSetSimpleLight(var_busSimpleLightDir_8c227db8[0], var_busSimpleLightDir_8c227db8[1], var_busSimpleLightDir_8c227db8[2]);
}

STATIC void execGroupTask_8c0283e8(Task *task, void *state)
{
    if (var_runState_8c2285c4.runPhase_0x00 != 0) {
        RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, setLightCallback_8c0283d4, 0);
        TaskRunGroup_8c014b42(var_trafficSignalTasks_8c227e20);
    }
}

STATIC void snapPointToGround_8c02840c(void)
{
    GroundQueryResult result;

    var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariHum_0x28;
    GroundQueryFindPolygon_8c020914(var_groundQueryPoint_8c1bc460.x, var_groundQueryPoint_8c1bc460.y, var_groundQueryPoint_8c1bc460.z, &result);
    if (result.count_0x0c == 0) {
        var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariBus_0x04;
        GroundQueryFindPolygon_8c020914(var_groundQueryPoint_8c1bc460.x, var_groundQueryPoint_8c1bc460.y, var_groundQueryPoint_8c1bc460.z, &result);
    }
    GroundProbeInterpolateHeight_8c020f7e(&result, (float *)&var_groundQueryPoint_8c1bc460);
}

/* Heads are spawned first so each attachment can find its head among the
 * installed tasks. */
void SignalInit_8c02845a(void)
{
    TrafficSignalDef *def;
    int maxId;
    int allocSize;
    int count;
    int i;
    Task *task;
    TrafficSignal *state;
    Task *slot;
    TrafficSignal *linked;
    NJS_OBJECT *frame;

    maxId = 0;
    for (def = var_currentCourse_8c1bb868.macSignal_0x38; def->type_0x00 != 0; def++) {
        if (maxId < def->id_0x04) {
            maxId = def->id_0x04;
        }
    }
    allocSize = (maxId + 1) * 4;
    var_trafficSignalFrames_8c227e24 = syMalloc(allocSize);
    var_trafficSignalStates_8c227e28 = syMalloc(allocSize);
    TrafficMarkSignalIdsInUse_8c026dcc(maxId);

    for (def = var_currentCourse_8c1bb868.macSignal_0x38; def->type_0x00 != 0; def++) {
        if (def->type_0x00 != 1) {
            var_trafficSignalFrames_8c227e24[def->linkedId_0x08] = 1;
        }
    }

    count = 0;
    for (i = 0; i < maxId; i++) {
        if (var_trafficSignalFrames_8c227e24[i] != 0) {
            count++;
        }
    }

    if (count == 0) {
        return;
    }

    var_trafficSignalTasks_8c227e20 = syMalloc((count + 1) * 0x20);
    TaskInitGroup_8c014a9c(var_trafficSignalTasks_8c227e20, count);

    /* Carried across both loops: if the link search below finds no match, the
     * type 2/3/4 branch still writes through whatever it last pointed at. */
    linked = (TrafficSignal *)&var_groundQueryPoint_8c1bc460;

    for (def = var_currentCourse_8c1bb868.macSignal_0x38; def->type_0x00 != 0; def++) {
        if (var_trafficSignalFrames_8c227e24[def->id_0x04] == 0 || def->type_0x00 != 1) {
            continue;
        }

        TaskInstall_8c014ae8(var_trafficSignalTasks_8c227e20, &headTask_8c028258,
                          &task, (void **)&state, 0xd4);
        state->id_0x00 = def->id_0x04;
        state->counter_0x04 = def->linkedId_0x08;
        state->durations_0x08 = def->durations_0x2c;
        state->posA_0x1c = def->posA_0x10;
        state->posB_0x28 = def->posB_0x20;
        state->tlist_0xb4 = (NJS_TEXLIST *)((int *)var_routeModels_8c1bc3ec)[0];
        state->model_0xb8 = (NJS_OBJECT *)((int *)var_routeModels_8c1bc3ec)[1];
        frame = state->model_0xb8->child;
        state->lamps_0x10[0] = frame;
        state->lamps_0x10[1] = frame->sibling;
        state->lamps_0x10[2] = frame->sibling->sibling;
        if (def->posA_0x10.x != 0.0f) {
            var_groundQueryPoint_8c1bc460 = def->posA_0x10;
            snapPointToGround_8c02840c();
            njUnitMatrix(&state->mtxA_0x34);
            njTranslate(&state->mtxA_0x34,
                        var_groundQueryPoint_8c1bc460.x, var_groundQueryPoint_8c1bc460.y, var_groundQueryPoint_8c1bc460.z);
            njRotateY(&state->mtxA_0x34,
                      (int)((float)def->rotADeg_0x0c * 65536.0f / 360.0f));
        }
        if (def->posB_0x20.x != 0.0f) {
            var_groundQueryPoint_8c1bc460 = def->posB_0x20;
            snapPointToGround_8c02840c();
            njUnitMatrix(&state->mtxB_0x74);
            njTranslate(&state->mtxB_0x74,
                        var_groundQueryPoint_8c1bc460.x, var_groundQueryPoint_8c1bc460.y, var_groundQueryPoint_8c1bc460.z);
            njRotateY(&state->mtxB_0x74,
                      (int)((float)def->rotBDeg_0x1c * 65536.0f / 360.0f));
        }
        state->frame_0x0c = 0;
        while (state->durations_0x08[state->frame_0x0c] <= state->counter_0x04) {
            state->counter_0x04 -= state->durations_0x08[state->frame_0x0c];
            state->frame_0x0c++;
        }
        state->attached_0xbc[2] = NULL;
        state->attached_0xbc[1] = NULL;
        state->attached_0xbc[0] = NULL;
        var_trafficSignalFrames_8c227e24[state->id_0x00] = state->frame_0x0c;
        var_trafficSignalStates_8c227e28[state->id_0x00] = state;
    }

    for (def = var_currentCourse_8c1bb868.macSignal_0x38; def->type_0x00 != 0; def++) {
        if (var_trafficSignalFrames_8c227e24[def->id_0x04] == 0 || def->type_0x00 == 1) {
            continue;
        }

        TaskInstall_8c014ae8(var_trafficSignalTasks_8c227e20, &attachmentTask_8c02833c,
                          &task, (void **)&state, 0xd4);
        for (slot = var_trafficSignalTasks_8c227e20; slot->action != NULL; slot++) {
            if (slot->action == (TaskAction)-1 || slot == task) {
                continue;
            }
            linked = (TrafficSignal *)slot->state;
            if (linked->id_0x00 == def->linkedId_0x08) {
                state->attachedTo_0xd0 = linked;
                break;
            }
        }
        state->id_0x00 = def->id_0x04;
        /* An attachment's first duration is the head counter it waits for. */
        state->counter_0x04 = def->durations_0x2c[0];
        if (def->type_0x00 == 2) {
            state->tlist_0xb4 = (NJS_TEXLIST *)((int *)var_routeModels_8c1bc3ec)[2];
            state->model_0xb8 = (NJS_OBJECT *)((int *)var_routeModels_8c1bc3ec)[3];
            linked->attached_0xbc[0] = state;
        } else if (def->type_0x00 == 3) {
            state->tlist_0xb4 = (NJS_TEXLIST *)((int *)var_routeModels_8c1bc3ec)[4];
            state->model_0xb8 = (NJS_OBJECT *)((int *)var_routeModels_8c1bc3ec)[5];
            linked->attached_0xbc[1] = state;
        } else if (def->type_0x00 == 4) {
            state->tlist_0xb4 = (NJS_TEXLIST *)((int *)var_routeModels_8c1bc3ec)[6];
            state->model_0xb8 = (NJS_OBJECT *)((int *)var_routeModels_8c1bc3ec)[7];
            linked->attached_0xbc[2] = state;
        }
        state->lamps_0x10[0] = state->model_0xb8->child;
        if (linked->frame_0x0c == 0 && state->counter_0x04 < linked->counter_0x04) {
            state->frame_0x0c = 0;
        } else {
            state->frame_0x0c = 1;
        }
        var_trafficSignalFrames_8c227e24[state->id_0x00] = state->frame_0x0c;
    }
    TaskInstall_8c014ae8(var_tasks_8c1ba5e8, &execGroupTask_8c0283e8, &task, (void **)&state, 0);
}

TrafficSignal *SignalGet_8c0288b2(int index)
{
    return var_trafficSignalStates_8c227e28[index];
}

void SignalFree_8c0288be(void)
{
    if (var_trafficSignalTasks_8c227e20 != (void *)-1) {
        TaskUninstallGroup_8c014ab4(var_trafficSignalTasks_8c227e20);
        syFree(var_trafficSignalTasks_8c227e20);
        var_trafficSignalTasks_8c227e20 = (void *)-1;
    }
    if (var_trafficSignalFrames_8c227e24 != (int *)-1) {
        syFree(var_trafficSignalFrames_8c227e24);
        syFree(var_trafficSignalStates_8c227e28);
        var_trafficSignalFrames_8c227e24 = (int *)-1;
    }
}
int SignalGetFrame_8c028900(int index)
{
    return var_trafficSignalFrames_8c227e24[index];
}
void SignalClearPedCrossing_8c02890c(void)
{
    int i;

    for (i = 0; i < 64; i++) {
        var_pedCrossingFlags_8c227e2c[i * 2] = 0;
        var_pedCrossingFlags_8c227e2c[i * 2 + 1] = 0;
    }
}

void SignalClearCrossingOccupied_8c028958(void)
{
    int i;

    for (i = 0; i < 64; i++) {
        var_crossingOccupiedFlags_8c22802c[i * 2] = 0;
        var_crossingOccupiedFlags_8c22802c[i * 2 + 1] = 0;
    }
}
void SignalMarkPedCrossing_8c02897a(int index)
{
    var_pedCrossingFlags_8c227e2c[index] = 1;
}

void SignalMarkCrossingOccupied_8c028984(int index)
{
    var_crossingOccupiedFlags_8c22802c[index] = 1;
}

int SignalIsCrossingOccupied_8c02898e(int index)
{
    return var_crossingOccupiedFlags_8c22802c[index];
}

int SignalIsPedCrossing_8c028998(int index)
{
    return var_pedCrossingFlags_8c227e2c[index];
}
