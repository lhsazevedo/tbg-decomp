#ifndef _0206F0_INTERSECT_H
#define _0206F0_INTERSECT_H

/* Points are {x, y}. Returns 1 and writes the hit to out, else 0. Original
 * bug, kept: when neither segment is vertical, y is taken from the stale
 * out[0] rather than the solved x. */
int IntersectSegments_8c0206f0(float *a0, float *a1, float *b0, float *b1, float *out);

#endif // _0206F0_INTERSECT_H
