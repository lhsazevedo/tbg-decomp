/* 8c014934 */
#include <shinobi.h>
#include "012f44.h"
#include "013ae8_route_load.h"
#include "014a9c_tasks.h"
#include "014934.h"
#include "011120_asset_queues.h"

extern Task var_tasks_8c1ba3c8[16];

void FUN_8c014934()
{
    Task* task;
    void* state;

    njSetBackColor(0xff418dff, 0xff418dff, 0xff418dff);
    var_loadScreenActive_8c157a6c = 1;

    pushTask_8c014ae8(var_tasks_8c1ba3c8, (void *) &unknownRouteLoadTask_8c014784, &task, &state, 0);
    task->field_0x08 = 0;
    task->field_0x0c = 0;

    njGarbageTexture(var_tex_8c157af8, 0xc00);

    AsqInitQueues_8c011f36(0x20,0x400,0x400,0x40);
}
