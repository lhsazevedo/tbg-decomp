/* @unit TrafficPathScan */

#include <shinobi.h>
#include "014a9c_tasks.h"
#include "026710_traffic.h"
#include "02f0c8_traffic_path_scan.h"
#include "1ba1c8_globals.h"

/* =======================
 * Non-initialized Globals
 * =======================
 */

/** The route's signal ids grouped by intersection */
Sint32 *var_signalGroups_8c228b40;

/** The group whose intersection a vehicle occupies */
Sint32 *var_occupiedGroup_8c228b44;

/* Sample-point scratch buffer for up to 10 (x, z) pairs. */
float var_samples_8c228b48[20];

/* Task-slot pointer to exclude from scan. Setter still unclear. */
void *var_excludedTask_8c228b98;

float *var_sampleCursor_8c228b9c;
float *var_sampleEnd_8c228ba0;

/* ===================
 * Initialized Globals
 * ===================
 */

Sint32 init_signalGroupsWangan_8c04c980[91] = {
    1, 2, -1,
    3, 4, -1,
    5, 6, -1,
    7, 60, 8, 61, -1,
    9, 10, -1,
    11, 12, 62, -1,
    13, 63, 14, -1,
    15, 16, -1,
    17, 18, -1,
    19, 20, -1,
    21, 22, -1,
    23, -1,
    24, 25, -1,
    26, 27, -1,
    28, 29, 64, -1,
    30, 31, -1,
    32, 33, -1,
    34, 65, 35, -1,
    36, 37, -1,
    38, 39, -1,
    40, 41, -1,
    42, 67, 43, 68, -1,
    44, 45, -1,
    46, 47, -1,
    48, 49, -1,
    50, 51, -1,
    52, 53, -1,
    54, 55, -1,
};

Sint32 init_signalGroupsShinjuku_8c04caec[147] = {
    1, 2, -1,
    3, -1,
    4, 5, 6, 101, -1,
    7, 8, 102, 103, -1,
    9, 10, -1,
    11, 12, 104, 105, -1,
    13, -1,
    14, 15, 106, -1,
    17, 18, 107, 108, -1,
    20, 21, 22, -1,
    23, 24, -1,
    25, 26, 27, -1,
    28, -1,
    29, -1,
    30, 31, 109, -1,
    32, 33, -1,
    34, 35, 82, 83, -1,
    36, 37, -1,
    38, 39, 84, 85, -1,
    40, 41, -1,
    42, 43, 44, -1,
    45, -1,
    46, 47, -1,
    48, 49, 86, 87, -1,
    50, -1,
    51, 52, 53, 88, 89, -1,
    54, 55, 56, -1,
    57, -1,
    58, 59, -1,
    60, 61, -1,
    62, 63, 90, 91, -1,
    64, 65, 92, 93, -1,
    66, 67, 95, -1,
    68, 69, 96, -1,
    70, 71, 97, -1,
    72, 73, 98, -1,
    74, 75, 76, 94, 99, -1,
    77, 78, 100, -1,
    79, -1,
    80, 81, -1,
};

Sint32 init_signalGroupsOme_8c04cd38[54] = {
    1, 37, -1,
    2, 3, -1,
    4, 5, -1,
    6, 7, 8, -1,
    9, 10, -1,
    11, 12, -1,
    13, 14, -1,
    15, 16, -1,
    17, 18, -1,
    19, 20, -1,
    21, 22, -1,
    23, 24, -1,
    25, -1,
    26, 36, 27, -1,
    28, 29, -1,
    30, 31, 35, -1,
    32, 33, 34, -1,
};

/* ====================
 * Functions
 * ====================
 */

void *TrafficPathScanFirst_8c02f0c8(
    Task *self,
    TrafficEntry *entry,
    PathRecord *firstRecord,
    Sint32 blockIndex,
    float startProgress,
    float window
)
{
    PathRecord *seg = firstRecord;
    float remaining = startProgress;
    float traveled;
    float *cursor;
    float *p;
    float x, z;
    Task *t;
    TrafficEntry *candidateEntry;

    while (seg->length_0x00 <= remaining) {
        remaining -= seg->length_0x00;
        seg++;
        if (seg->length_0x00 == 0.0f) {
            blockIndex++;
            seg = entry->pathBlocks_0x304[blockIndex];
        }
    }

    cursor = var_samples_8c228b48;
    for (traveled = startProgress; traveled < startProgress + window; traveled += 5.0f) {
        while (seg != (PathRecord *)-1 && seg->length_0x00 <= remaining) {
            remaining -= seg->length_0x00;
            seg++;
            if (seg->length_0x00 == 0.0f) {
                blockIndex++;
                seg = entry->pathBlocks_0x304[blockIndex];
            }
        }
        if (seg == (PathRecord *)-1) {
            break;
        }

        cursor[0] = remaining * seg->dirX_0x0c + seg->x_0x04;
        cursor[1] = remaining * seg->dirZ_0x10 + seg->z_0x08;
        cursor += 2;
        remaining += 5.0f;
    }

    if (cursor != var_samples_8c228b48) {
        var_sampleEnd_8c228ba0 = cursor;
        for (p = var_samples_8c228b48; p < cursor; p += 2) {
            x = p[0];
            z = p[1];

            if ((fabsf(var_busState_8c1bb9d0.posX_0x0f4 - x) < 2.5f &&
                 fabsf(var_busState_8c1bb9d0.posZ_0x0fc - z) < 2.5f) ||
                (fabsf(var_busState_8c1bb9d0.posHistory_0x100[0].x - x) < 2.5f &&
                 fabsf(var_busState_8c1bb9d0.posHistory_0x100[0].z - z) < 2.5f)) {
                var_sampleCursor_8c228b9c = p + 2;
                return var_playerBus_8c1bbd9c;
            }

            for (t = var_tasks_8c1bac28; t->action != NULL; t++) {
                if (t->action != (TaskAction)-1 && t != self) {
                    candidateEntry = t->state;
                    if (fabsf(candidateEntry->posX_0xf4 - x) < 2.5f &&
                        fabsf(candidateEntry->posZ_0xfc - z) < 2.5f) {
                        var_sampleCursor_8c228b9c = p + 2;
                        return candidateEntry;
                    }
                }
            }
        }
    }

    return NULL;
}

void *TrafficPathScanNext_8c02f212(void)
{
    float *p;
    Task *t;
    TrafficEntry *candidateEntry;
    int advance;

    p = var_sampleCursor_8c228b9c;
    for (;;) {
        if (p >= var_sampleEnd_8c228ba0) {
            return NULL;
        }

        for (t = var_tasks_8c1bac28; t->action != NULL; t++) {
            advance = 0;

            // Note: var_excludedTask_8c228b98 is never written
            if (t->action != (TaskAction) -1 && (void *) t != var_excludedTask_8c228b98) {
                candidateEntry = t->state;
                advance = 1;
                if (fabsf(candidateEntry->posX_0xf4 - p[0]) < 2.5f) {
                    advance = 2;
                    if (fabsf(candidateEntry->posZ_0xfc - p[1]) < 2.5f) {
                        var_sampleCursor_8c228b9c = p + 2;
                        return candidateEntry;
                    }
                }
            }

            p += advance;
        }
    }
}

Sint32 TrafficPathScanJunctionOccupied_8c02f28a(Sint32 signalId)
{
    Sint32 *group;
    Sint32 *scan;
    Task *t;

    group = var_occupiedGroup_8c228b44;
    if (var_occupiedGroup_8c228b44 == (Sint32 *)-1) {
        for (t = var_tasks_8c1bac28; ; t++) {
            if (t->action == NULL) {
                return 0;
            }
            if (t->action != (TaskAction)-1 &&
                ((TrafficEntry *)t->state)->atGroundJunction_0x50c != 0) {
                break;
            }
        }

        scan = var_signalGroups_8c228b40;
        group = var_signalGroups_8c228b40;
        while (((TrafficEntry *)t->state)->signalWaitId_0x450 != (Uint32)*scan) {
            if (*scan == -1) {
                scan++;
                group = scan;
            } else {
                scan++;
            }
        }
        var_occupiedGroup_8c228b44 = group;
    }

    while (*group != -1) {
        if (signalId == *group) {
            return 1;
        }
        group++;
    }
    return 0;
}
