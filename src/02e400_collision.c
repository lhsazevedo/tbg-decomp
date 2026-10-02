/* @unit Collision */

#include <shinobi.h>
#include "014a9c_tasks.h"
#include "026710_traffic.h" /* TrafficEntry */
#include "02e2dc_bus_collision.h"
#include "02e400_collision.h"
#include "1ba1c8_globals.h"

/* =======================
 * Non-initialized Globals
 * =======================
 */

Task *var_collisionScanCursor_8c228974;

NJS_BOX var_collisionSelfBox_8c228978;

NJS_BOX var_collisionCandidateBox_8c2289d8;

/* Pedestrians on crossings. */
void *var_collisionQueue_8c228a38[64];

Sint32 var_collisionQueueCount_8c228b38;

/* ====================
 * Functions
 * ====================
 */

/* Original bug, preserved: self's box is built from the table slot's
 * address, i.e. the pointer table's own bytes read as floats. */
TrafficEntry *CollisionFindTaskHit_8c02e400(Task *self, TrafficEntry *entry)
{
    TrafficEntry *candidateEntry;

    njCalcPoints(&entry->worldMatrix_0x84,
                 (NJS_POINT3 *)&init_variantBoxes_8c04c940[entry->variantIndex_0x2e0],
                 var_collisionSelfBox_8c228978.v, 8);

    var_collisionScanCursor_8c228974 = var_tasks_8c1bac28;
    for (;;) {
        if (var_collisionScanCursor_8c228974->action == NULL) {
            return NULL;
        }

        if (var_collisionScanCursor_8c228974 != self
            && var_collisionScanCursor_8c228974->action != (TaskAction)-1) {
            candidateEntry = (TrafficEntry *)var_collisionScanCursor_8c228974->state;
            njCalcPoints(&candidateEntry->worldMatrix_0x84,
                         init_variantBoxes_8c04c940[candidateEntry->variantIndex_0x2e0],
                         var_collisionCandidateBox_8c2289d8.v, 8);
            if (njCollisionCheckBB(&var_collisionSelfBox_8c228978,
                                   &var_collisionCandidateBox_8c2289d8)) {
                return candidateEntry;
            }
        }

        var_collisionScanCursor_8c228974++;
    }
}

void CollisionQueueReset_8c02e486(void)
{
    var_collisionQueueCount_8c228b38 = 0;
}

void CollisionQueueAdd_8c02e48e(void *obj)
{
    if (var_collisionQueueCount_8c228b38 < 64) {
        var_collisionQueue_8c228a38[var_collisionQueueCount_8c228b38++] = obj;
    }
}

void *CollisionQueueTest_8c02e4ac(void)
{
    Sint32 i;

    for (i = 0; i < var_collisionQueueCount_8c228b38; i++) {
        if (njCollisionCheckBS(&var_collisionSelfBox_8c228978,
                               var_collisionQueue_8c228a38[i])) {
            return var_collisionQueue_8c228a38[i];
        }
    }

    return NULL;
}
