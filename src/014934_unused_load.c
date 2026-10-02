/* @unit UnusedLoad */
/* 8c014934 */
#include <shinobi.h>
#include "0129cc_game.h"
#include "013ae8_route.h"
#include "014a9c_tasks.h"
#include "014934_unused_load.h"
#include "011120_asset_queues.h"
#include "1ba1c8_globals.h"

/* ====================
 * Functions
 * ====================
 */

/* See 014934_unused_load.h. */
void UnusedLoadSpawnTask_8c014934()
{
    RouteLoadTask *task;
    void *state;

    njSetBackColor(0xff418dff, 0xff418dff, 0xff418dff);
    var_loadScreenActive_8c157a6c = 1;

    TaskSpawn_8c014ae8(
        var_tasks_8c1ba3c8,
        (void *) &RouteUnusedTask_8c014784,
        (Task **) &task,
        &state,
        0
    );
    task->stage_0x08 = ROUTE_LOAD_STATE_INIT;
    task->frame_0x0c = 0;

    njGarbageTexture(var_tex_8c157af8, 0xc00);

    /* Half the dat/nj queue budget RouteSpawnLoadTask_8c0144fc asks for. */
    AsqInitQueues_8c011f36(0x20, 0x400, 0x400, 0x40);
}
