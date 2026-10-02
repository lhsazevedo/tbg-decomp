/* @unit Intersect */

#include <shinobi.h>

#include "0206f0_intersect.h"

/* ====================
 * Functions
 * ====================
 */

int IntersectSegments_8c0206f0(float *a0, float *a1, float *b0, float *b1, float *out)
{
    float ma, ba, mb, bb;
    float denom;
    float x, y;

    if (a1[0] - a0[0] == 0.0f) {
        /* segment a is vertical */
        if (b1[0] - b0[0] == 0.0f) {
            return 0; /* both vertical */
        }
        x = a0[0];
        mb = (b1[1] - b0[1]) / (b1[0] - b0[0]);
        bb = b0[1] - mb * b0[0];
        y = x * mb + bb;
    } else {
        ma = (a1[1] - a0[1]) / (a1[0] - a0[0]);
        ba = a0[1] - ma * a0[0];
        if (b1[0] - b0[0] == 0.0f) {
            /* segment b is vertical */
            x = b0[0];
            y = x * ma + ba;
        } else {
            mb = (b1[1] - b0[1]) / (b1[0] - b0[0]);
            denom = mb - ma;
            if (denom == 0.0f) {
                return 0; /* parallel, neither segment vertical */
            }
            bb = b0[1] - mb * b0[0];
            x = (ba - bb) / denom;

            /* Original bug: stale out[0], not x. */
            y = out[0] * ma + ba;
        }
    }

    if (a1[0] < a0[0]) {
        if (a0[0] < x) return 0;
        if (x < a1[0]) return 0;
    } else {
        if (x < a0[0]) return 0;
        if (a1[0] < x) return 0;
    }

    if (b1[0] < b0[0]) {
        if (b0[0] < x) return 0;
        if (x < b1[0]) return 0;
    } else {
        if (x < b0[0]) return 0;
        if (b1[0] < x) return 0;
    }

    out[0] = x;
    out[1] = y;
    return 1;
}
