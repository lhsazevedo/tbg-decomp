/* @unit VehicleParts */

#include <shinobi.h>

#include "02786c_vehicle_parts.h"
#include "026710_traffic.h" /* TrafficEntry */

/* ====================
 * Functions
 * ====================
 */

void VehiclePartsBind_8c02786c(TrafficEntry *entry, Uint32 typeCode)
{
    NJS_OBJECT *nj;
    NJS_OBJECT *node;

    entry->typeCode_0x000 = typeCode;

    nj = entry->modelLarge_0x0c;

    node = nj->child;
    entry->bodyNode_0x018 = node;
    node->evalflags &= ~NJD_EVAL_UNIT_ANG;

    node = node->sibling;
    entry->frontWheelA_0x01c = node;
    node->evalflags &= ~NJD_EVAL_UNIT_ANG;

    node = node->sibling;
    entry->frontWheelB_0x020 = node;
    node->evalflags &= ~NJD_EVAL_UNIT_ANG;

    node = node->sibling;
    entry->rearWheelA_0x024 = node;
    node->evalflags &= ~NJD_EVAL_UNIT_ANG;

    if (typeCode == 0x14 || typeCode == 0x16 || typeCode == 0x0e || typeCode == 0x10) {
        node = node->sibling;
        entry->rearWheelB_0x028 = node;
        node->evalflags &= ~NJD_EVAL_UNIT_ANG;
    }

    nj = entry->modelLarge_0x0c;
    node = nj->child->child;
    entry->blinkerLights_0x02c[0] = node;

    node = node->sibling;
    entry->blinkerLights_0x02c[1] = node;

    node = node->sibling;
    entry->blinkerLights_0x02c[2] = node;

    node = node->sibling;
    entry->blinkerLights_0x02c[3] = node;

    node = node->sibling;
    entry->blinkerLights_0x02c[4] = node;

    if (typeCode == 0x1a) {
        entry->field_0x058 = node->child;
        entry->field_0x05c = node->child->sibling;
        entry->field_0x060 = node->child->sibling->sibling;
    }

    node = node->sibling;
    if (node != NULL) {
        if (typeCode == 0x1c || typeCode == 0x1e) {
            entry->turnLampA_0x040 = node;
        }

        node = node->sibling;
        if (node != NULL) {
            if (typeCode == 0x1a) {
                entry->turnLampC_0x048 = node;
            }

            if (typeCode == 0x14 || typeCode == 0x16) {
                entry->turnLampB_0x044 = node;
            }

            if (typeCode == 0x1a) {
                node = node->sibling->child;
                entry->field_0x04c = node;

                node = node->sibling;
                entry->field_0x050 = node;

                entry->field_0x054 = node->sibling;
            }
        }
    }
}
