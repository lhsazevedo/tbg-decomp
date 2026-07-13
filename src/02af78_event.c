#include <shinobi.h>

#include "02af78_event.h"
#include "014f54_text_pre_data.h"
#include "0fcd20_sectionB.h"
#include "serial_debug.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

/* EventEntry packs two bitfields into plain 32-bit ints instead of C
 * bitfields (the original code doesn't use them here), so the layout is
 * spelled out below instead of being visible in the struct itself.
 *
 * dayMask_0x04: up to 6 slots of 5 bits, LSB first. A UNLOCK_DAYSn() call
 * with n < 6 simply omits the trailing slots (they read as 0, which is
 * also what tells scanEventCandidates_8c02b03c's scan loop to stop):
 *
 *     bits 29-25   bits 24-20   bits 19-15   bits 14-10   bits 9-5    bits 4-0
 *    +------------+------------+------------+------------+------------+------------+
 *    |   day[5]   |   day[4]   |   day[3]   |   day[2]   |   day[1]   |   day[0]   |
 *    +------------+------------+------------+------------+------------+------------+
 *
 * conditions_0x08 / actions_0x0c: always exactly 3 slots of 10 bits
 * ({mode:2,flag:8} each), LSB first, built by UNLOCK_CODES3(slot0, slot1, slot2):
 *
 *     bits 31-30    bits 29-20         bits 19-10         bits 9-0
 *    +------------+-------------------+-------------------+-------------------+
 *    |  (unused)  |    code slot 2    |    code slot 1    |    code slot 0    |
 *    +------------+-------------------+-------------------+-------------------+
 *
 *   each 10-bit slot:
 *
 *       bit 9  bit 8  |  bit 7 ......................... bit 0
 *      +-------------+-----------------------------------+
 *      |    mode     |               flag                |
 *      +-------------+-----------------------------------+
 *
 *   mode in conditions_0x08:                mode in actions_0x0c:
 *     00 UNLOCK_IF_UNSET     (0x000)          x0 UNLOCK_SET_PROGRESS (0x100)
 *     01 UNLOCK_IF_SET       (0x100)          x1 UNLOCK_SET_RUN      (0x300)
 *     10 UNLOCK_IF_RUN_UNSET (0x200)            (only bit 9, UNLOCK_ACTION_MODE,
 *     11 UNLOCK_IF_RUN_SET   (0x300)             is actually read for actions)
 *
 *   slot == 0x3ff (all 10 bits set, UNLOCK_NONE) is a no-op/padding slot.
 *
 * Example: UNLOCK_CODES1(UNLOCK_IF_UNSET(0x32))
 *   -> slot0 = 0x032, slot1 = slot2 = 0x3ff
 *   -> packed = 0x032 | (0x3ff << 10) | (0x3ff << 20) = 0x3FFFFC32
 */
#define UNLOCK_CODE_BITS  10
#define UNLOCK_CODE_MASK  0x3ff
#define UNLOCK_CODE_EMPTY 0x3ff
#define UNLOCK_CODE_FLAG  0x0ff
#define UNLOCK_CODE_MODE  0x300

#define UNLOCK_MODE_FORBID_PROGRESS   0x000 /* fail if progress flag set */
#define UNLOCK_MODE_REQUIRE_PROGRESS  0x100 /* fail if progress flag clear */
#define UNLOCK_MODE_FORBID_EPHEMERAL  0x200 /* fail if var_runEventFlags_8c1ba2b4 bit set */
#define UNLOCK_MODE_REQUIRE_EPHEMERAL 0x300 /* fail if var_runEventFlags_8c1ba2b4 bit clear */

#define UNLOCK_ACTION_MODE 0x200

#define UNLOCK_DAY_BITS 5
#define UNLOCK_DAY_MASK 0x1f

/* EventEntry array terminator (see timeOfDay_0x00 in 02af78_event.h). */
#define EVENT_ENTRY_END 0xffff

/* conditions_0x08 code constructors (checked by scanEventCandidates_8c02b03c
 * / pickSegmentEvent_8c02b170; see UNLOCK_MODE_* above). */
#define UNLOCK_IF_UNSET(flag)     (UNLOCK_MODE_FORBID_PROGRESS   | (flag))
#define UNLOCK_IF_SET(flag)       (UNLOCK_MODE_REQUIRE_PROGRESS  | (flag))
#define UNLOCK_IF_RUN_UNSET(flag) (UNLOCK_MODE_FORBID_EPHEMERAL  | (flag))
#define UNLOCK_IF_RUN_SET(flag)   (UNLOCK_MODE_REQUIRE_EPHEMERAL | (flag))

/* actions_0x0c code constructors (applied by applyEventFlags_8c02b292; only
 * bit UNLOCK_ACTION_MODE is actually read, but the byte layout still uses
 * the full conditions_0x08 mode encoding -- see UNLOCK_ACTION_MODE above). */
#define UNLOCK_SET_PROGRESS(flag) (UNLOCK_MODE_REQUIRE_PROGRESS  | (flag))
#define UNLOCK_SET_RUN(flag)      (UNLOCK_MODE_REQUIRE_EPHEMERAL | (flag))

#define UNLOCK_NONE UNLOCK_CODE_EMPTY

/* conditions_0x08 / actions_0x0c always pack exactly 3 slots; trailing
 * omitted slots default to UNLOCK_NONE (see the bit-layout diagram above). */
#define UNLOCK_CODES1(a)          ((a) | (UNLOCK_NONE << UNLOCK_CODE_BITS) | (UNLOCK_NONE << (UNLOCK_CODE_BITS * 2)))
#define UNLOCK_CODES2(a, b)       ((a) | ((b) << UNLOCK_CODE_BITS) | (UNLOCK_NONE << (UNLOCK_CODE_BITS * 2)))
#define UNLOCK_CODES3(a, b, c)    ((a) | ((b) << UNLOCK_CODE_BITS) | ((c) << (UNLOCK_CODE_BITS * 2)))

/* dayMask_0x04 packs 1..6 day values; unused trailing slots are simply
 * omitted (the 0-terminator falls out naturally once the value is fully
 * shifted out -- see scanEventCandidates_8c02b03c). */
#define UNLOCK_DAYS1(a)               (a)
#define UNLOCK_DAYS2(a, b)            (UNLOCK_DAYS1(a) | ((b) << UNLOCK_DAY_BITS))
#define UNLOCK_DAYS3(a, b, c)         (UNLOCK_DAYS2(a, b) | ((c) << (UNLOCK_DAY_BITS * 2)))
#define UNLOCK_DAYS4(a, b, c, d)      (UNLOCK_DAYS3(a, b, c) | ((d) << (UNLOCK_DAY_BITS * 3)))
#define UNLOCK_DAYS5(a, b, c, d, e)   (UNLOCK_DAYS4(a, b, c, d) | ((e) << (UNLOCK_DAY_BITS * 4)))
#define UNLOCK_DAYS6(a, b, c, d, e, f) (UNLOCK_DAYS5(a, b, c, d, e) | ((f) << (UNLOCK_DAY_BITS * 5)))

/* ====================
 * Initialized Globals
 * ====================
 */

STATIC EventEntry init_wanganEvents_8c04abb0[] = {
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0009,
        /* dayMask_0x04    */ UNLOCK_DAYS3(3, 4, 5),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x32)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x32)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0009,
        /* dayMask_0x04    */ UNLOCK_DAYS2(4, 5),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x32)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0009,
        /* dayMask_0x04    */ UNLOCK_DAYS3(10, 11, 12),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x33)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x33)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0009,
        /* dayMask_0x04    */ UNLOCK_DAYS2(11, 12),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x33)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0009,
        /* dayMask_0x04    */ UNLOCK_DAYS3(17, 18, 19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x34)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x34)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0009,
        /* dayMask_0x04    */ UNLOCK_DAYS2(18, 19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x34)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0009,
        /* dayMask_0x04    */ UNLOCK_DAYS3(24, 25, 26),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_UNSET(0x35), UNLOCK_IF_UNSET(0x3b)),
        /* actions_0x0c    */ UNLOCK_CODES2(UNLOCK_SET_PROGRESS(0x35), UNLOCK_SET_RUN(0x03)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0009,
        /* dayMask_0x04    */ UNLOCK_DAYS2(25, 26),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_SET(0x35), UNLOCK_IF_UNSET(0x3b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_RUN(0x03)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0009,
        /* dayMask_0x04    */ UNLOCK_DAYS2(29, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_NONE),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_RUN(0x03)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS2(4, 5),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x36)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x36)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS1(5),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x36)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS2(11, 12),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x37)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x37)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS1(12),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x37)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS2(18, 19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x38)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x38)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS1(19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x38)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS4(7, 14, 21, 28),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x39)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x39)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS3(14, 21, 28),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x39)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS5(15, 23, 26, 29, 30),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_UNSET(0x3a), UNLOCK_IF_RUN_UNSET(0x03)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x3a)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS4(23, 26, 29, 30),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_SET(0x3a), UNLOCK_IF_RUN_UNSET(0x03)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS6(1, 2, 3, 6, 7, 8),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x3c)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x3c)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS5(2, 3, 6, 7, 8),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x3c)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS5(2, 3, 6, 7, 8),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x3c)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS5(2, 3, 6, 7, 8),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x3c)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS6(10, 13, 14, 16, 17, 20),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x3d)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x3d)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS5(13, 14, 16, 17, 20),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x3d)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS5(13, 14, 16, 17, 20),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x3d)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS5(13, 14, 16, 17, 20),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x3d)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS6(22, 24, 27, 28, 29, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x3e)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x3e)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS5(24, 27, 28, 29, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x3e)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS5(24, 27, 28, 29, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x3e)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS4(1, 2, 8, 9),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x3f)),
        /* actions_0x0c    */ UNLOCK_CODES2(UNLOCK_SET_PROGRESS(0x3f), UNLOCK_SET_RUN(0x01)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS3(2, 8, 9),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x3f)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_RUN(0x01)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(1, 2),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x40)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x40)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(2),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x40)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS2(1, 2),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x41)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x41)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS1(2),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x41)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(8, 9),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x42)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x42)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(9),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x42)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(15, 16),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x43)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x43)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(16),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x43)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0009,
        /* dayMask_0x04    */ UNLOCK_DAYS2(12, 15),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x46)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x46)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0009,
        /* dayMask_0x04    */ UNLOCK_DAYS1(15),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x46)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0009,
        /* dayMask_0x04    */ UNLOCK_DAYS1(19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x47)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x47)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0009,
        /* dayMask_0x04    */ UNLOCK_DAYS2(23, 26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x48)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x48)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0009,
        /* dayMask_0x04    */ UNLOCK_DAYS1(26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x48)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS2(9, 10),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x49)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x49)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS1(10),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x49)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS2(11, 16),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x4a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x4a)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS1(16),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x4a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS2(17, 18),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x4b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x4b)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS1(18),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x4b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS3(24, 25, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x4c)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x4c)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS2(25, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x4c)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0006,
        /* dayMask_0x04    */ UNLOCK_DAYS2(8, 9),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x4d)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x4d)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0006,
        /* dayMask_0x04    */ UNLOCK_DAYS1(9),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x4d)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0006,
        /* dayMask_0x04    */ UNLOCK_DAYS2(15, 16),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x4e)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x4e)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0006,
        /* dayMask_0x04    */ UNLOCK_DAYS1(16),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x4e)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0006,
        /* dayMask_0x04    */ UNLOCK_DAYS1(22),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x4f)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x4f)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0008,
        /* dayMask_0x04    */ UNLOCK_DAYS3(10, 14, 17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x50)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x50)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0008,
        /* dayMask_0x04    */ UNLOCK_DAYS2(14, 17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x50)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0008,
        /* dayMask_0x04    */ UNLOCK_DAYS3(11, 15, 18),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x51)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x51)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0008,
        /* dayMask_0x04    */ UNLOCK_DAYS2(15, 18),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x51)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0008,
        /* dayMask_0x04    */ UNLOCK_DAYS2(12, 19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x52)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x52)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0008,
        /* dayMask_0x04    */ UNLOCK_DAYS1(19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x52)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0008,
        /* dayMask_0x04    */ UNLOCK_DAYS2(22, 24),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x53)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x53)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0008,
        /* dayMask_0x04    */ UNLOCK_DAYS1(24),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x53)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0008,
        /* dayMask_0x04    */ UNLOCK_DAYS2(23, 25),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x54)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x54)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0008,
        /* dayMask_0x04    */ UNLOCK_DAYS1(25),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x54)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0008,
        /* dayMask_0x04    */ UNLOCK_DAYS1(26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x55)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x55)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0008,
        /* dayMask_0x04    */ UNLOCK_DAYS1(20),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x56)),
        /* actions_0x0c    */ UNLOCK_CODES2(UNLOCK_SET_PROGRESS(0x56), UNLOCK_SET_RUN(0x04)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS3(13, 20, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x57)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x57)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS2(20, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x57)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS2(10, 17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x58)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x58)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS1(17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x58)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS2(24, 27),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x5a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x5a)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS1(27),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x5a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS2(13, 20),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_UNSET(0x5b), UNLOCK_IF_RUN_UNSET(0x04)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x5b)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS1(20),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_SET(0x5b), UNLOCK_IF_RUN_UNSET(0x04)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(20),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x60)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x60)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(27),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x61)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x61)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS3(26, 29, 30),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_UNSET(0x3b), UNLOCK_IF_RUN_SET(0x03)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x3b)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS2(29, 30),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_SET(0x3b), UNLOCK_IF_RUN_SET(0x03)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS2(29, 30),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_SET(0x3b), UNLOCK_IF_RUN_SET(0x03)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS2(15, 16),
        /* conditions_0x08 */ UNLOCK_CODES3(UNLOCK_IF_UNSET(0x44), UNLOCK_IF_UNSET(0x45), UNLOCK_IF_RUN_SET(0x01)),
        /* actions_0x0c    */ UNLOCK_CODES2(UNLOCK_SET_PROGRESS(0x44), UNLOCK_SET_PROGRESS(0x45)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS1(16),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x44)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS1(20),
        /* conditions_0x08 */ UNLOCK_CODES3(UNLOCK_IF_UNSET(0x5b), UNLOCK_IF_UNSET(0x5c), UNLOCK_IF_RUN_SET(0x04)),
        /* actions_0x0c    */ UNLOCK_CODES2(UNLOCK_SET_PROGRESS(0x5b), UNLOCK_SET_PROGRESS(0x5c)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS1(20),
        /* conditions_0x08 */ UNLOCK_CODES3(UNLOCK_IF_SET(0x5b), UNLOCK_IF_UNSET(0x5c), UNLOCK_IF_RUN_SET(0x04)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x5c)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS1(27),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x5e)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x5e)),
    },
    {
        /* timeOfDay_0x00  */ EVENT_ENTRY_END,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS1(27),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x5e)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x5e)),
    },
};

STATIC EventEntry init_shinjukuEvents_8c04b1f0[] = {
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS2(6, 8),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x00)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x00)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS1(8),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x00)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS2(13, 15),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x01)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x01)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS1(15),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x01)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS2(20, 22),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x02)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x02)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS1(22),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x02)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS2(27, 29),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x03)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x03)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0002,
        /* dayMask_0x04    */ UNLOCK_DAYS1(29),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x03)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(6, 10),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x04)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x04)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS1(10),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x04)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(13, 17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x05)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x05)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS1(17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x05)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS3(20, 24, 27),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x06)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x06)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(24, 27),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x06)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS3(7, 9, 11),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x07)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x07)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS2(9, 11),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x07)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS3(14, 16, 18),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x08)),
        /* actions_0x0c    */ UNLOCK_CODES2(UNLOCK_SET_PROGRESS(0x08), UNLOCK_SET_RUN(0x00)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS2(16, 18),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x08)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_RUN(0x00)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS5(20, 21, 22, 24, 25),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x09)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x09)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS4(21, 22, 24, 25),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x09)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS4(21, 22, 24, 25),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x09)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS3(27, 28, 29),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x0a)),
        /* actions_0x0c    */ UNLOCK_CODES2(UNLOCK_SET_PROGRESS(0x0a), UNLOCK_SET_RUN(0x00)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS2(28, 29),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x0a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_RUN(0x00)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS3(7, 9, 11),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x0b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x0b)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS2(9, 11),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x0b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS3(14, 16, 18),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_UNSET(0x0c), UNLOCK_IF_RUN_UNSET(0x00)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x0c)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS2(16, 18),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_SET(0x0c), UNLOCK_IF_RUN_UNSET(0x00)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS3(27, 28, 29),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_UNSET(0x0e), UNLOCK_IF_RUN_UNSET(0x00)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x0e)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS2(28, 29),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_SET(0x0e), UNLOCK_IF_RUN_UNSET(0x00)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(5, 6),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x12)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x12)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(6),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x12)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(11, 12),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x13)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x13)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(12),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x13)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(18, 19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x14)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x14)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x14)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(25, 26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x15)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x15)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x15)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS3(11, 14, 16),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x16)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x16)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(14, 16),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x16)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(18, 21),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x17)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x17)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS1(21),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x17)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(23, 25),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x18)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x18)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS1(25),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x18)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(28, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x19)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x19)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS1(30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x19)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS4(13, 14, 16, 17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x1a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x1a)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS3(14, 16, 17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x1a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS3(14, 16, 17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x1a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS5(22, 27, 28, 29, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x1b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x1b)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS4(27, 28, 29, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x1b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS4(27, 28, 29, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x1b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS2(25, 26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x1d)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x1d)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS1(26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x1d)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(11, 12),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x1e)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x1e)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(12),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x1e)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(25, 26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x1f)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x1f)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x1f)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(18, 19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x20)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x20)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x20)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x000e,
        /* dayMask_0x04    */ UNLOCK_DAYS2(18, 19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x21)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x21)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x000e,
        /* dayMask_0x04    */ UNLOCK_DAYS1(19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x21)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS3(20, 21, 24),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x2c)),
        /* actions_0x0c    */ UNLOCK_CODES2(UNLOCK_SET_PROGRESS(0x2c), UNLOCK_SET_RUN(0x02)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS2(21, 24),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x22)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS1(30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x24)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x24)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x000f,
        /* dayMask_0x04    */ UNLOCK_DAYS3(15, 18, 19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x25)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x25)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x000f,
        /* dayMask_0x04    */ UNLOCK_DAYS2(18, 19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x25)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x000f,
        /* dayMask_0x04    */ UNLOCK_DAYS2(25, 26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x26)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x26)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x000f,
        /* dayMask_0x04    */ UNLOCK_DAYS1(26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x26)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS2(22, 28),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x28)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x28)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS1(28),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x28)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(13, 14),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x29)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x29)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(14),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x29)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(23),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x2a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x2a)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(29, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x2b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x2b)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x2b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS3(14, 16, 18),
        /* conditions_0x08 */ UNLOCK_CODES3(UNLOCK_IF_UNSET(0x0c), UNLOCK_IF_UNSET(0x0d), UNLOCK_IF_RUN_SET(0x00)),
        /* actions_0x0c    */ UNLOCK_CODES2(UNLOCK_SET_PROGRESS(0x0c), UNLOCK_SET_PROGRESS(0x0d)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS2(16, 18),
        /* conditions_0x08 */ UNLOCK_CODES3(UNLOCK_IF_SET(0x0c), UNLOCK_IF_UNSET(0x0d), UNLOCK_IF_RUN_SET(0x00)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x0d)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS2(16, 18),
        /* conditions_0x08 */ UNLOCK_CODES3(UNLOCK_IF_SET(0x0c), UNLOCK_IF_SET(0x0d), UNLOCK_IF_RUN_SET(0x00)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS3(27, 28, 29),
        /* conditions_0x08 */ UNLOCK_CODES3(UNLOCK_IF_UNSET(0x0e), UNLOCK_IF_UNSET(0x0f), UNLOCK_IF_RUN_SET(0x00)),
        /* actions_0x0c    */ UNLOCK_CODES2(UNLOCK_SET_PROGRESS(0x0e), UNLOCK_SET_PROGRESS(0x0f)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS2(28, 29),
        /* conditions_0x08 */ UNLOCK_CODES3(UNLOCK_IF_SET(0x0e), UNLOCK_IF_UNSET(0x0f), UNLOCK_IF_RUN_SET(0x00)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x0f)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000a,
        /* dayMask_0x04    */ UNLOCK_DAYS2(28, 29),
        /* conditions_0x08 */ UNLOCK_CODES3(UNLOCK_IF_SET(0x0e), UNLOCK_IF_SET(0x0f), UNLOCK_IF_RUN_SET(0x00)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS2(25, 26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x10)),
        /* actions_0x0c    */ UNLOCK_CODES2(UNLOCK_SET_PROGRESS(0x10), UNLOCK_SET_RUN(0x01)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS1(26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x10)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_RUN(0x01)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0014,
        /* dayMask_0x04    */ UNLOCK_DAYS2(25, 26),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_UNSET(0x11), UNLOCK_IF_RUN_SET(0x01)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x11)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0014,
        /* dayMask_0x04    */ UNLOCK_DAYS1(26),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_SET(0x11), UNLOCK_IF_RUN_SET(0x01)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS4(11, 12, 18, 19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x1c)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x1c)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS3(12, 18, 19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x1c)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ 0,
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS3(20, 21, 24),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_UNSET(0x22), UNLOCK_IF_RUN_SET(0x02)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x22)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0007,
        /* dayMask_0x04    */ UNLOCK_DAYS2(21, 24),
        /* conditions_0x08 */ UNLOCK_CODES3(UNLOCK_IF_SET(0x22), UNLOCK_IF_UNSET(0x23), UNLOCK_IF_RUN_SET(0x02)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x23)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS2(29, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x27)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x27)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS1(30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x27)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ EVENT_ENTRY_END,
        /* segmentId_0x02  */ 0x000b,
        /* dayMask_0x04    */ UNLOCK_DAYS1(30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x27)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
};

STATIC EventEntry init_omeEvents_8c04b920[] = {
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0004,
        /* dayMask_0x04    */ UNLOCK_DAYS3(3, 7, 10),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x64)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x64)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0004,
        /* dayMask_0x04    */ UNLOCK_DAYS2(7, 10),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x64)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0004,
        /* dayMask_0x04    */ UNLOCK_DAYS2(14, 17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x65)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x65)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0004,
        /* dayMask_0x04    */ UNLOCK_DAYS1(17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x65)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0004,
        /* dayMask_0x04    */ UNLOCK_DAYS2(21, 24),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_UNSET(0x66), UNLOCK_IF_RUN_UNSET(0x05)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x66)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0004,
        /* dayMask_0x04    */ UNLOCK_DAYS1(24),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_SET(0x66), UNLOCK_IF_RUN_UNSET(0x05)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0004,
        /* dayMask_0x04    */ UNLOCK_DAYS1(28),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x68)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x68)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS2(1, 8),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x69)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x69)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS1(8),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x69)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS1(15),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x6a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x6a)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS2(22, 29),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x6b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x6b)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x000d,
        /* dayMask_0x04    */ UNLOCK_DAYS1(29),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x6b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS3(4, 11, 18),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x6c)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x6c)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(11, 18),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x6c)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(25),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x6d)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x6d)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS3(2, 6, 9),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x6e)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x6e)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(6, 9),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x6e)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS3(13, 16, 20),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x6f)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x6f)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(16, 20),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x6f)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS3(24, 27, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x70)),
        /* actions_0x0c    */ UNLOCK_CODES2(UNLOCK_SET_RUN(0x05), UNLOCK_SET_PROGRESS(0x70)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(27, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x70)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_RUN(0x05)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(3, 10),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x71)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x71)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(10),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x71)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS2(17, 24),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x72)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x72)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS1(24),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x72)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS2(3, 4),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x73)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x73)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS1(4),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x73)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS1(10),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x74)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x74)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS3(3, 6, 8),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x75)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x75)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS2(6, 8),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x75)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS3(10, 13, 17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x76)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x76)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS2(13, 17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x76)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS3(20, 22, 24),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x77)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x77)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS2(22, 24),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x77)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS2(27, 29),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x78)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x78)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0005,
        /* dayMask_0x04    */ UNLOCK_DAYS1(29),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x78)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS3(6, 9, 13),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x79)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x79)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(9, 13),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x79)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS3(16, 20, 23),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x7a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x7a)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(20, 23),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x7a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(27, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x7b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x7b)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS1(30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x7b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS5(3, 4, 5, 10, 12),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x7c)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x7c)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS4(4, 5, 10, 12),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x7c)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS3(17, 18, 19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x7d)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x7d)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(18, 19),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x7d)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS3(24, 25, 26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x7e)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x7e)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(25, 26),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x7e)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS5(4, 7, 11, 14, 25),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x7f)),
        /* actions_0x0c    */ UNLOCK_CODES2(UNLOCK_SET_PROGRESS(0x7f), UNLOCK_SET_RUN(0x06)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS4(7, 11, 14, 25),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x7f)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_RUN(0x06)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS3(7, 11, 14),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x82)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x82)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(11, 14),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x82)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x000c,
        /* dayMask_0x04    */ UNLOCK_DAYS3(7, 11, 14),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_UNSET(0x83), UNLOCK_IF_RUN_SET(0x06)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x83)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x000c,
        /* dayMask_0x04    */ UNLOCK_DAYS2(11, 14),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_SET(0x83), UNLOCK_IF_RUN_SET(0x06)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(18, 21),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x84)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x84)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(21),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x84)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(3, 10),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x85)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x85)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS1(10),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x85)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS1(17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x86)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x86)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(24, 28),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x87)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x87)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS1(28),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x87)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS5(6, 7, 8, 9, 10),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x88)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x88)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS4(7, 8, 9, 10),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x88)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS4(7, 8, 9, 10),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x88)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS4(13, 14, 16, 17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x89)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x89)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS3(14, 16, 17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x89)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS3(14, 16, 17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x89)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(20, 22),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x8a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x8a)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(22),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x8a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(22),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x8a)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS3(27, 29, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x8b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x8b)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(29, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x8b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS2(29, 30),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x8b)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(10, 17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x8c)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x8c)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS1(17),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x8c)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS1(24),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x8d)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x8d)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0004,
        /* dayMask_0x04    */ UNLOCK_DAYS1(21),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x8e)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x8e)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(31),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0xc8)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0xc8)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS4(18, 21, 23, 24),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x91)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x91)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS3(21, 23, 24),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_SET(0x91)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_NONE),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0000,
        /* dayMask_0x04    */ UNLOCK_DAYS1(28),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x92)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x92)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0004,
        /* dayMask_0x04    */ UNLOCK_DAYS1(24),
        /* conditions_0x08 */ UNLOCK_CODES3(UNLOCK_IF_UNSET(0x66), UNLOCK_IF_UNSET(0x67), UNLOCK_IF_RUN_SET(0x05)),
        /* actions_0x0c    */ UNLOCK_CODES2(UNLOCK_SET_PROGRESS(0x66), UNLOCK_SET_PROGRESS(0x67)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_DAY,
        /* segmentId_0x02  */ 0x0004,
        /* dayMask_0x04    */ UNLOCK_DAYS1(24),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_SET(0x66), UNLOCK_IF_RUN_SET(0x05)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x67)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS2(18, 21),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_UNSET(0x80), UNLOCK_IF_RUN_SET(0x06)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x80)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_EVENING,
        /* segmentId_0x02  */ 0x0003,
        /* dayMask_0x04    */ UNLOCK_DAYS1(21),
        /* conditions_0x08 */ UNLOCK_CODES2(UNLOCK_IF_SET(0x80), UNLOCK_IF_RUN_SET(0x06)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x81)),
    },
    {
        /* timeOfDay_0x00  */ TIME_OF_DAY_NIGHT,
        /* segmentId_0x02  */ 0x0004,
        /* dayMask_0x04    */ UNLOCK_DAYS1(28),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x8f)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x8f)),
    },
    {
        /* timeOfDay_0x00  */ EVENT_ENTRY_END,
        /* segmentId_0x02  */ 0x0004,
        /* dayMask_0x04    */ UNLOCK_DAYS1(28),
        /* conditions_0x08 */ UNLOCK_CODES1(UNLOCK_IF_UNSET(0x8f)),
        /* actions_0x0c    */ UNLOCK_CODES1(UNLOCK_SET_PROGRESS(0x8f)),
    },
};

/* ====================
 * Functions
 * ====================
 */

/* Marks progress flag `index` in both paired bitsets of the progress struct. */
void setProgressFlag_8c02af78(int index)
{
    int word = index >> 5;
    int mask = 1 << (index & 0x1f);

    var_progress_8c1ba1cc.field_0x04[word] |= mask;
    var_progress_8c1ba1cc.field_0x18[word] |= mask;
}

/* Tests progress flag `index` in the field_0x04 bitset (see setProgressFlag_8c02af78). */
int hasProgressFlag_8c02afbe(int index)
{
    int word = index >> 5;
    int mask = 1 << (index & 0x1f);

    return var_progress_8c1ba1cc.field_0x04[word] & mask;
}

/* Tests progress flag `index` in the field_0x18 bitset (see setProgressFlag_8c02af78). */
int hasProgressFlagAlt_8c02aff0(int index)
{
    int word = index >> 5;
    int mask = 1 << (index & 0x1f);

    return var_progress_8c1ba1cc.field_0x18[word] & mask;
}

/* Sets bit `index` of var_runEventFlags_8c1ba2b4, the same-run "already
 * fired" bitset checked by conditions_0x08 modes 2/3 (cleared each
 * scanEventCandidates_8c02b03c call). */
void setRunEventFlag_8c02b022(int index)
{
    var_runEventFlags_8c1ba2b4 |= 1 << (index & 0x1f);
}

/* Tests bit `index` of var_runEventFlags_8c1ba2b4 (see setRunEventFlag_8c02b022). */
int hasRunEventFlag_8c02b030(int index)
{
    return var_runEventFlags_8c1ba2b4 & (1 << (index & 0x1f));
}

/* Scans the active route's EventEntry table for entries matching the
 * current time of day whose prerequisites are met, and collects their
 * indices into var_eventCandidates_8c228520/var_eventCandidateCount_8c228560 as event candidates. Skipped
 * entirely (other than resetting the candidate count) during practice
 * mode. */
void scanEventCandidates_8c02b03c(void)
{
    EventEntry *entry;
    int index;
    Uint32 dayMask;
    Uint32 conditions;

    var_eventCandidateCount_8c228560 = 0;

    if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE) {
        LOG_DEBUG(("[EVENT] skipped scanning candidates (practice mode)\n"));
        return;
    }

    var_runEventFlags_8c1ba2b4 = 0;

    switch (var_route_8c18ad1c) {
        case ROUTE_SHINJUKU: {
            var_routeEvents_8c22851c = init_shinjukuEvents_8c04b1f0;
            break;
        }
        case ROUTE_WANGAN: {
            var_routeEvents_8c22851c = init_wanganEvents_8c04abb0;
            break;
        }
        case ROUTE_OME: {
            var_routeEvents_8c22851c = init_omeEvents_8c04b920;
            break;
        }
    }

    for (index = 0; var_routeEvents_8c22851c[index].timeOfDay_0x00 != EVENT_ENTRY_END; index++) {
        entry = &var_routeEvents_8c22851c[index];

        if (entry->timeOfDay_0x00 != var_timeOfDay_8c18ad20) {
            continue;
        }

        for (dayMask = entry->dayMask_0x04;
             dayMask != 0 && var_progress_8c1ba1cc.days_0x00 != (int)(dayMask & UNLOCK_DAY_MASK);
             dayMask >>= UNLOCK_DAY_BITS) {
        }

        if (dayMask == 0) {
            continue;
        }

        for (conditions = entry->conditions_0x08; conditions != 0; conditions >>= UNLOCK_CODE_BITS) {
            int code = conditions & UNLOCK_CODE_MASK;

            if (code == UNLOCK_CODE_EMPTY) {
                continue;
            }

            if ((code & UNLOCK_CODE_MODE) == UNLOCK_MODE_FORBID_PROGRESS) {
                if (hasProgressFlag_8c02afbe(code & UNLOCK_CODE_FLAG) != 0) {
                    break;
                }
                continue;
            }

            if ((code & UNLOCK_CODE_MODE) == UNLOCK_MODE_REQUIRE_PROGRESS &&
                hasProgressFlag_8c02afbe(code & UNLOCK_CODE_FLAG) == 0) {
                break;
            }
        }

        if (conditions == 0) {
            var_eventCandidates_8c228520[var_eventCandidateCount_8c228560] = index;
            var_eventCandidateCount_8c228560++;
        }
    }

    LOG_DEBUG(("[EVENT] found %d candidate(s) for timeOfDay=%d\n",
               var_eventCandidateCount_8c228560, var_timeOfDay_8c18ad20));
}

/* Narrows scanEventCandidates_8c02b03c's candidates to the ones whose
 * segmentId_0x02 matches the current segment and whose conditions_0x08
 * var_runEventFlags_8c1ba2b4-bit checks (mode 2/3) also pass, then randomly picks one into
 * var_selectedEventEntry_8c228478 and arms var_cutsceneActive_8c1bb900.
 * Skipped (cutsceneActive cleared) outside normal play, during the course
 * menu (var_gameMode_8c1bb8fc), or while var_8c2285dc > var_8c2285d8. */
void pickSegmentEvent_8c02b170(void)
{
    EventEntry *entry;
    int i;
    int count;
    int tableIndex;
    Uint32 conditions;
    int candidates[5];

    if (!(var_playMode_8c1bb8d0 == PLAY_MODE_NORMAL && var_gameMode_8c1bb8fc == 0 &&
          var_8c2285dc <= var_8c2285d8)) {
        var_cutsceneActive_8c1bb900 = 0;
        LOG_DEBUG(("[EVENT] skipped picking for segment (guard not open)\n"));
        return;
    }

    count = 0;

    for (i = 0; i < var_eventCandidateCount_8c228560; i++) {
        tableIndex = var_eventCandidates_8c228520[i];
        entry = var_routeEvents_8c22851c + tableIndex;

        if (var_currentSegment_8c228708 != entry->segmentId_0x02) {
            continue;
        }

        for (conditions = entry->conditions_0x08; conditions != 0; conditions >>= UNLOCK_CODE_BITS) {
            int code = conditions & UNLOCK_CODE_MASK;

            if (code == UNLOCK_CODE_EMPTY) {
                continue;
            }

            if ((code & UNLOCK_CODE_MODE) == UNLOCK_MODE_FORBID_EPHEMERAL) {
                if (hasRunEventFlag_8c02b030(code & UNLOCK_CODE_FLAG) != 0) {
                    break;
                }
                continue;
            }

            if ((code & UNLOCK_CODE_MODE) == UNLOCK_MODE_REQUIRE_EPHEMERAL &&
                hasRunEventFlag_8c02b030(code & UNLOCK_CODE_FLAG) == 0) {
                break;
            }
        }

        if (conditions == 0) {
            candidates[count] = tableIndex;
            count++;
        }
    }

    if (count == 0) {
        var_cutsceneActive_8c1bb900 = 0;
        LOG_DEBUG(("[EVENT] no eligible candidates for segment=%d\n",
                   var_currentSegment_8c228708));
        return;
    }

    var_cutsceneActive_8c1bb900 = 1;
    var_selectedEventEntry_8c228478 = candidates[AsqGetRandomInRangeB_8c0121be(count)];
    LOG_DEBUG(("[EVENT] selected entry %d from %d candidate(s)\n",
               var_selectedEventEntry_8c228478, count));
}

/* Applies var_selectedEventEntry_8c228478's actions_0x0c codes: mode clear
 * sets a progress flag, mode 0x200 sets a var_runEventFlags_8c1ba2b4 bit. */
void applyEventFlags_8c02b292(void)
{
    EventEntry *entry;
    Uint32 actions;

    entry = var_routeEvents_8c22851c + var_selectedEventEntry_8c228478;

    LOG_DEBUG(("[EVENT] applying entry %d\n",
               var_selectedEventEntry_8c228478));

    for (actions = entry->actions_0x0c; actions != 0; actions >>= UNLOCK_CODE_BITS) {
        int code = actions & UNLOCK_CODE_MASK;

        if (code == UNLOCK_CODE_EMPTY) {
            continue;
        }

        if ((code & UNLOCK_ACTION_MODE) == 0) {
            setProgressFlag_8c02af78(code & UNLOCK_CODE_FLAG);
            continue;
        }

        setRunEventFlag_8c02b022(code & UNLOCK_CODE_FLAG);
    }
}
