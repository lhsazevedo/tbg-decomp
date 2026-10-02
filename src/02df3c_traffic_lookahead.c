/* @unit TrafficLookahead */

#include <shinobi.h>

#include "014a9c_tasks.h"       /* Task, TaskAction */
#include "026710_traffic.h"     /* TrafficEntry, PathRecord */
#include "02df3c_traffic_lookahead.h"
#include "1ba1c8_globals.h"
#include "02e400_collision.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define SAMPLE_STEP 5.0f
#define CACHE_END 9999.0f       /* terminates lookaheadPoints_0x49c */
#define SEED_RANGE 25.0f        /* TrafficLookaheadInit_8c02df3c fills to here */
#define REFILL_RANGE 20.0f      /* TrafficLookaheadScan_8c02dfca tops back up to here */

#define NEAR_XZ(bx, bz, px, pz, r) \
    (fabsf((bx) - (px)) < (r) && fabsf((bz) - (pz)) < (r))

/* Where the player's bus blocks a sample point: its current position and the
 * newest history sample within 2.5 units, four older samples within 2.0. */
#define BUS_BLOCKS(b, px, pz) (                                               \
    NEAR_XZ((b)->posHistory_0x100[10].x, (b)->posHistory_0x100[10].z,         \
            px, pz, 2.0f) ||                                                  \
    NEAR_XZ((b)->posHistory_0x100[11].x, (b)->posHistory_0x100[11].z,         \
            px, pz, 2.0f) ||                                                  \
    NEAR_XZ((b)->posHistory_0x100[8].x, (b)->posHistory_0x100[8].z,           \
            px, pz, 2.0f) ||                                                  \
    NEAR_XZ((b)->posHistory_0x100[9].x, (b)->posHistory_0x100[9].z,           \
            px, pz, 2.0f) ||                                                  \
    NEAR_XZ((b)->posX_0x0f4, (b)->posZ_0x0fc, px, pz, 2.5f) ||                \
    NEAR_XZ((b)->posHistory_0x100[0].x, (b)->posHistory_0x100[0].z,           \
            px, pz, 2.5f))

/* ====================
 * Functions
 * ====================
 */

/* A -1 path cursor (entry->pathBlocks_0x304 ran out) ends the walk early,
 * same as the refill loop in TrafficLookaheadScan_8c02dfca below. */
void TrafficLookaheadInit_8c02df3c(TrafficEntry *entry)
{
    float *out = (float *)entry->lookaheadPoints_0x49c;
    PathRecord *cursor = entry->lookaheadCursor_0x4f4;
    float dist = entry->lookaheadCursorDist_0x4fc;

    for (;;) {
        if (entry->lookaheadCacheLen_0x4ec >= SEED_RANGE) {
            *out = CACHE_END;
            entry->lookaheadCursor_0x4f4 = cursor;
            entry->lookaheadCursorDist_0x4fc = dist;
            return;
        }

        for (;;) {
            if (cursor == (PathRecord *)-1) {
                entry->lookaheadCacheLen_0x4ec += SAMPLE_STEP;
                *out = CACHE_END;
                entry->lookaheadCursor_0x4f4 = cursor;
                entry->lookaheadCursorDist_0x4fc = dist;
                return;
            }
            if (dist < cursor->length_0x00) {
                break;
            }
            dist -= cursor->length_0x00;
            cursor++;
            if (cursor->length_0x00 == 0.0f) {
                entry->lookaheadCursorBlock_0x4f8++;
                cursor = entry->pathBlocks_0x304[entry->lookaheadCursorBlock_0x4f8];
            }
        }

        out[0] = dist * cursor->dirX_0x0c + cursor->x_0x04;
        out[1] = dist * cursor->dirZ_0x10 + cursor->z_0x08;
        out += 2;
        entry->lookaheadCacheLen_0x4ec += SAMPLE_STEP;
        dist += SAMPLE_STEP;
    }
}

/* The refill first shifts the still-valid tail of the old cache down to slot
 * 0, then walks the path exactly as TrafficLookaheadInit_8c02df3c does.
 *
 * Traffic entries are tested at two points, their front reference
 * (rearPointX_0x10c despite the name) and their position; the bus gets the
 * six-point treatment of BUS_BLOCKS above. */
void *TrafficLookaheadScan_8c02dfca(Task *self, TrafficEntry *entry, float lookahead)
{
    float cacheDist = entry->lookaheadCacheLen_0x4ec;
    float *out = (float *)entry->lookaheadPoints_0x49c;
    float remain;
    float px, pz;

    if (cacheDist < REFILL_RANGE) {
        float *src = (float *)entry->lookaheadPoints_0x49c + 2;
        PathRecord *cursor;
        float dist;

        for (remain = cacheDist; remain > 0.0f; remain -= SAMPLE_STEP) {
            out[0] = src[0];
            out[1] = src[1];
            src += 2;
            out += 2;
        }

        cursor = entry->lookaheadCursor_0x4f4;
        dist = entry->lookaheadCursorDist_0x4fc;

        for (; cacheDist < REFILL_RANGE; cacheDist += SAMPLE_STEP) {
            while (cursor != (PathRecord *)-1 && dist >= cursor->length_0x00) {
                dist -= cursor->length_0x00;
                cursor++;
                if (cursor->length_0x00 == 0.0f) {
                    entry->lookaheadCursorBlock_0x4f8++;
                    cursor = entry->pathBlocks_0x304[entry->lookaheadCursorBlock_0x4f8];
                }
            }
            if (cursor == (PathRecord *)-1) {
                cacheDist += SAMPLE_STEP;
                break;
            }

            out[0] = dist * cursor->dirX_0x0c + cursor->x_0x04;
            out[1] = dist * cursor->dirZ_0x10 + cursor->z_0x08;
            out += 2;
            dist += SAMPLE_STEP;
        }

        if (out == (float *)entry->lookaheadPoints_0x49c) {
            return NULL;
        }
        *out = CACHE_END;
        entry->lookaheadCursor_0x4f4 = cursor;
        entry->lookaheadCursorDist_0x4fc = dist;
        entry->lookaheadCacheLen_0x4ec = cacheDist;
    }

    out = (float *)entry->lookaheadPoints_0x49c;
    for (remain = lookahead + SAMPLE_STEP; remain > 0.0f; remain -= SAMPLE_STEP) {
        px = out[0];
        pz = out[1];
        out += 2;
        if (px == CACHE_END) {
            return NULL;
        }

        if (BUS_BLOCKS(&var_busState_8c1bb9d0, px, pz)) {
            return var_playerBus_8c1bbd9c;
        }

        var_collisionScanCursor_8c228974 = var_tasks_8c1bac28;
        for (; var_collisionScanCursor_8c228974->action != NULL;
             var_collisionScanCursor_8c228974++) {
            TrafficEntry *candidate;

            if (var_collisionScanCursor_8c228974 == self
                || var_collisionScanCursor_8c228974->action == (TaskAction)-1) {
                continue;
            }

            candidate = (TrafficEntry *)var_collisionScanCursor_8c228974->state;
            if (NEAR_XZ(candidate->rearPointX_0x10c, candidate->rearPointZ_0x114,
                        px, pz, 2.5f) ||
                NEAR_XZ(candidate->posX_0xf4, candidate->posZ_0xfc, px, pz, 2.5f)) {
                return candidate;
            }
        }
    }

    /* One point past the lookahead is still tested against the bus, but not
     * against traffic. */
    px = out[0];
    pz = out[1];
    if (px != CACHE_END && BUS_BLOCKS(&var_busState_8c1bb9d0, px, pz)) {
        return var_playerBus_8c1bbd9c;
    }
    return NULL;
}
