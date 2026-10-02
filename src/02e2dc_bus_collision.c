/* @unit BusCollision */

#include <shinobi.h>
#include "014a9c_tasks.h"
#include "02081c_geom.h"
#include "026710_traffic.h" /* TrafficEntry */
#include "02e2dc_bus_collision.h"
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "02e400_collision.h"

/* ====================
 * Initialized Globals
 * ====================
 */

/* Local-space bounding boxes, 8 corners each: top face, then bottom face,
 * each going around. Literals keep full float32 precision because the
 * originals aren't exact (3.0500001907348633 is one ULP off 3.05). */
STATIC NJS_POINT3 init_variantBox_8c04c520[8] = {
    {-0.8349999785423279f, 1.5f, 3.0500001907348633f},
    {-0.8349999785423279f, 1.5f, -0.6499999761581421f},
    {0.8349999785423279f, 1.5f, -0.6499999761581421f},
    {0.8349999785423279f, 1.5f, 3.0500001907348633f},
    {-0.8349999785423279f, 0.0f, 3.0500001907348633f},
    {-0.8349999785423279f, 0.0f, -0.6499999761581421f},
    {0.8349999785423279f, 0.0f, -0.6499999761581421f},
    {0.8349999785423279f, 0.0f, 3.0500001907348633f},
};

STATIC NJS_POINT3 init_variantBox_8c04c580[8] = {
    {-0.8650000095367432f, 1.5f, 3.0399999618530273f},
    {-0.8650000095367432f, 1.5f, -0.7599999904632568f},
    {0.8650000095367432f, 1.5f, -0.7599999904632568f},
    {0.8650000095367432f, 1.5f, 3.0399999618530273f},
    {-0.8650000095367432f, 0.0f, 3.0399999618530273f},
    {-0.8650000095367432f, 0.0f, -0.7599999904632568f},
    {0.8650000095367432f, 0.0f, -0.7599999904632568f},
    {0.8650000095367432f, 0.0f, 3.0399999618530273f},
};

STATIC NJS_POINT3 init_variantBox_8c04c5e0[8] = {
    {-0.8999999761581421f, 1.5f, 3.799999952316284f},
    {-0.8999999761581421f, 1.5f, -0.8999999761581421f},
    {0.8999999761581421f, 1.5f, -0.8999999761581421f},
    {0.8999999761581421f, 1.5f, 3.799999952316284f},
    {-0.8999999761581421f, 0.0f, 3.799999952316284f},
    {-0.8999999761581421f, 0.0f, -0.8999999761581421f},
    {0.8999999761581421f, 0.0f, -0.8999999761581421f},
    {0.8999999761581421f, 0.0f, 3.799999952316284f},
};

STATIC NJS_POINT3 init_variantBox_8c04c640[8] = {
    {-0.8500000238418579f, 1.5f, 3.9000000953674316f},
    {-0.8500000238418579f, 1.5f, -0.699999988079071f},
    {0.8500000238418579f, 1.5f, -0.699999988079071f},
    {0.8500000238418579f, 1.5f, 3.9000000953674316f},
    {-0.8500000238418579f, 0.0f, 3.9000000953674316f},
    {-0.8500000238418579f, 0.0f, -0.699999988079071f},
    {0.8500000238418579f, 0.0f, -0.699999988079071f},
    {0.8500000238418579f, 0.0f, 3.9000000953674316f},
};

STATIC NJS_POINT3 init_variantBox_8c04c6a0[8] = {
    {-1.0149999856948853f, 1.5f, 8.899999618530273f},
    {-1.0149999856948853f, 1.5f, -1.5f},
    {1.0149999856948853f, 1.5f, -1.5f},
    {1.0149999856948853f, 1.5f, 8.899999618530273f},
    {-1.0149999856948853f, 0.0f, 8.899999618530273f},
    {-1.0149999856948853f, 0.0f, -1.5f},
    {1.0149999856948853f, 0.0f, -1.5f},
    {1.0149999856948853f, 0.0f, 8.899999618530273f},
};

STATIC NJS_POINT3 init_variantBox_8c04c700[8] = {
    {-0.8999999761581421f, 1.5f, 4.0f},
    {-0.8999999761581421f, 1.5f, -1.0f},
    {0.8999999761581421f, 1.5f, -1.0f},
    {0.8999999761581421f, 1.5f, 4.0f},
    {-0.8999999761581421f, 0.0f, 4.0f},
    {-0.8999999761581421f, 0.0f, -1.0f},
    {0.8999999761581421f, 0.0f, -1.0f},
    {0.8999999761581421f, 0.0f, 4.0f},
};

STATIC NJS_POINT3 init_variantBox_8c04c760[8] = {
    {-1.034999966621399f, 1.5f, 5.300000190734863f},
    {-1.034999966621399f, 1.5f, -2.0f},
    {1.034999966621399f, 1.5f, -2.0f},
    {1.034999966621399f, 1.5f, 5.300000190734863f},
    {-1.034999966621399f, 0.0f, 5.300000190734863f},
    {-1.034999966621399f, 0.0f, -2.0f},
    {1.034999966621399f, 0.0f, -2.0f},
    {1.034999966621399f, 0.0f, 5.300000190734863f},
};

STATIC NJS_POINT3 init_variantBox_8c04c7c0[8] = {
    {-0.7649999856948853f, 1.5f, 3.5f},
    {-0.7649999856948853f, 1.5f, -0.8999999761581421f},
    {0.7649999856948853f, 1.5f, -0.8999999761581421f},
    {0.7649999856948853f, 1.5f, 3.5f},
    {-0.7649999856948853f, 0.0f, 3.5f},
    {-0.7649999856948853f, 0.0f, -0.8999999761581421f},
    {0.7649999856948853f, 0.0f, -0.8999999761581421f},
    {0.7649999856948853f, 0.0f, 3.5f},
};

STATIC NJS_POINT3 init_busBox_8c04c820[8] = {
    {-1.1649999618530273f, 1.5f, 8.0f},
    {-1.1649999618530273f, 1.5f, -2.5999999046325684f},
    {1.1649999618530273f, 1.5f, -2.5999999046325684f},
    {1.1649999618530273f, 1.5f, 8.0f},
    {-1.1649999618530273f, 0.0f, 8.0f},
    {-1.1649999618530273f, 0.0f, -2.5999999046325684f},
    {1.1649999618530273f, 0.0f, -2.5999999046325684f},
    {1.1649999618530273f, 0.0f, 8.0f},
};

STATIC NJS_POINT3 init_variantBox_8c04c880[8] = {
    {-0.8450000286102295f, 1.5f, 3.9600000381469727f},
    {-0.8450000286102295f, 1.5f, -0.7400000095367432f},
    {0.8450000286102295f, 1.5f, -0.7400000095367432f},
    {0.8450000286102295f, 1.5f, 3.9600000381469727f},
    {-0.8450000286102295f, 0.0f, 3.9600000381469727f},
    {-0.8450000286102295f, 0.0f, -0.7400000095367432f},
    {0.8450000286102295f, 0.0f, -0.7400000095367432f},
    {0.8450000286102295f, 0.0f, 3.9600000381469727f},
};

STATIC NJS_POINT3 init_variantBox_8c04c8e0[8] = {
    {-0.8399999737739563f, 1.5f, 3.8999998569488525f},
    {-0.8399999737739563f, 1.5f, -1.0f},
    {0.8399999737739563f, 1.5f, -1.0f},
    {0.8399999737739563f, 1.5f, 3.8999998569488525f},
    {-0.8399999737739563f, 0.0f, 3.8999998569488525f},
    {-0.8399999737739563f, 0.0f, -1.0f},
    {0.8399999737739563f, 0.0f, -1.0f},
    {0.8399999737739563f, 0.0f, 3.8999998569488525f},
};

/* Slot 13 is the player bus. */
NJS_POINT3 *init_variantBoxes_8c04c940[16] = {
    init_variantBox_8c04c520, init_variantBox_8c04c520,
    init_variantBox_8c04c580, init_variantBox_8c04c5e0,
    init_variantBox_8c04c5e0, init_variantBox_8c04c5e0,
    init_variantBox_8c04c640, init_variantBox_8c04c6a0,
    init_variantBox_8c04c6a0, init_variantBox_8c04c700,
    init_variantBox_8c04c760, init_variantBox_8c04c760,
    init_variantBox_8c04c7c0, init_busBox_8c04c820,
    init_variantBox_8c04c880, init_variantBox_8c04c8e0,
};

/* ====================
 * Functions
 * ====================
 */

TrafficEntry *BusCollisionFindHit_8c02e2dc(void)
{
    Task *cursor;
    TrafficEntry *entry;

    njCalcPoints(&var_busState_8c1bb9d0.worldMatrix_0x084, init_busBox_8c04c820,
                 var_collisionSelfBox_8c228978.v, 8);

    var_collisionScanCursor_8c228974 = var_tasks_8c1bac28;

    while (var_collisionScanCursor_8c228974->action != NULL) {
        cursor = var_collisionScanCursor_8c228974;

        if (cursor->action != (TaskAction)-1) {
            entry = (TrafficEntry *)cursor->state;

            if (entry->busDistance_0x490 < 12.0f) {
                njCalcPoints(&entry->worldMatrix_0x84,
                             init_variantBoxes_8c04c940[entry->variantIndex_0x2e0],
                             var_collisionCandidateBox_8c2289d8.v, 8);

                if (GeomQuadOverlap_8c020842(var_collisionSelfBox_8c228978.v,
                                             var_collisionCandidateBox_8c2289d8.v)) {
                    return entry;
                }
            }
        }

        var_collisionScanCursor_8c228974++;
    }

    return NULL;
}

/* Unused. Resumes BusCollisionFindHit_8c02e2dc's scan past its last hit,
 * reusing the bus box it left behind. */
STATIC BusState *findNextHit_8c02e35a(void)
{
    Task *cursor;
    TrafficEntry *entry;

    var_collisionScanCursor_8c228974++;

    while (var_collisionScanCursor_8c228974->action != NULL) {
        cursor = var_collisionScanCursor_8c228974;

        if (cursor->action != (TaskAction)-1) {
            entry = (TrafficEntry *)cursor->state;

            if (entry->busDistance_0x490 < 12.0f) {
                njCalcPoints(&entry->worldMatrix_0x84,
                             init_variantBoxes_8c04c940[entry->variantIndex_0x2e0],
                             var_collisionCandidateBox_8c2289d8.v, 8);

                if (GeomQuadOverlap_8c020842(var_collisionSelfBox_8c228978.v,
                                             var_collisionCandidateBox_8c2289d8.v)) {
                    return (BusState *)entry;
                }
            }
        }

        var_collisionScanCursor_8c228974++;
    }

    return NULL;
}
