/* @unit Vib */
/* 8c010e90 */
#include <shinobi.h>
#include "010e90_vibration.h"
#include "includes.h" /* NM_STATIC */
#include "serial_debug.h"

/* =================
 * Type Declarations
 * =================
 */

/* power is a Sint8 in PDS_VIBPARAM: 0xff/0xfe/0xf9 are -1/-2/-7. */
typedef struct {
    int frames_0x00;
    Uint8 flag_0x04;
    Uint8 power_0x05;
    Uint8 freq_0x06;
} VibStep;

typedef struct {
    int pattern_0x00;
    int frame_0x04;
    int step_0x08;
    int playing_0x0c;
} VibState;

/* =======================
 * Non-initialized Globals
 * =======================
 */

VibState var_vibState_8c157a48;

/* ===================
 * Initialized Globals
 * ===================
 */

const PDS_PERIPHERAL const_peripheralZero_8c033318 = {0};

/* A zero frames_0x00 terminates a pattern. */
VibStep init_vibEngineStart_8c03bdac[] = {
    {0x14, 0x01, 0xFF, 0x0F},
    {0x64, 0x01, 0xFE, 0x1E},
    {0x00, 0x00, 0x00, 0x00}
};

VibStep init_vibChimeLong_8c03bdc4[] = {
    {0x1E, 0x01, 0x03, 0x1E},
    {0x14, 0x01, 0x00, 0x00},
    {0x1E, 0x01, 0x03, 0x1E},
    {0x00, 0x00, 0x00, 0x00}
};

VibStep init_vibChimeShort_8c03bde4[] = {
    {0x14, 0x01, 0x03, 0x1E},
    {0x00, 0x00, 0x00, 0x00}
};

VibStep init_vibHardBrake_8c03bdf4[] = {
    {0x0C, 0x01, 0xF9, 0x0F},
    {0x00, 0x00, 0x00, 0x00}
};

VibStep init_vibBumpLight_8c03be04[] = {
    {0x0A, 0x01, 0x03, 0x0F},
    {0x0A, 0x01, 0x02, 0x14},
    {0x00, 0x00, 0x00, 0x00}
};

VibStep init_vibBumpMedium_8c03be1c[] = {
    {0x0F, 0x01, 0x04, 0x1E},
    {0x0F, 0x01, 0x03, 0x14},
    {0x00, 0x00, 0x00, 0x00}
};

VibStep init_vibBumpHeavy_8c03be34[] = {
    {0x19, 0x01, 0x07, 0x28},
    {0x0A, 0x01, 0x05, 0x1E},
    {0x00, 0x00, 0x00, 0x00}
};

VibStep init_vibIdle_8c03be4c[] = {
    {0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00}
};

/* Index is also priority. 7 is idle. */
VibStep* init_vibPatterns_8c03be5c[] = {
    init_vibEngineStart_8c03bdac,
    init_vibChimeLong_8c03bdc4,
    init_vibChimeShort_8c03bde4,
    init_vibHardBrake_8c03bdf4,
    init_vibBumpLight_8c03be04,
    init_vibBumpMedium_8c03be1c,
    init_vibBumpHeavy_8c03be34,
    init_vibIdle_8c03be4c
};

/* =========
 * Functions
 * =========
 */

NM_STATIC void stepPattern_8c010e90(int port) {
    PDS_VIBPARAM param;
    VibStep* pattern;

    pattern = init_vibPatterns_8c03be5c[var_vibState_8c157a48.pattern_0x00];

    if (!var_vibState_8c157a48.step_0x08) {
        param.unit = 1;

        param.flag = pattern[var_vibState_8c157a48.step_0x08].flag_0x04;
        param.power = pattern[var_vibState_8c157a48.step_0x08].power_0x05;
        param.freq = pattern[var_vibState_8c157a48.step_0x08].freq_0x06;

        param.inc = 0;

        pdVibMxStart(port, &param);

        var_vibState_8c157a48.step_0x08++;
        var_vibState_8c157a48.playing_0x0c = 1;
    } else if (pattern[var_vibState_8c157a48.step_0x08 - 1].frames_0x00 < var_vibState_8c157a48.frame_0x04) {
        pdVibMxStop(port);

        var_vibState_8c157a48.frame_0x04 = 0;

        if (pattern[var_vibState_8c157a48.step_0x08].frames_0x00 == 0) {
            VibClear_8c010fbe();
        } else {
            param.unit = 1;

            param.flag = pattern[var_vibState_8c157a48.step_0x08].flag_0x04;
            param.power = pattern[var_vibState_8c157a48.step_0x08].power_0x05;
            param.freq = pattern[var_vibState_8c157a48.step_0x08].freq_0x06;

            param.inc = 0;

            /* The pack answers PDD_VIBERR_BUSY while the previous command is
             * still in flight. */
            while (1) {
                int r = pdVibMxStart(port, &param);
                if (r == PDD_VIBERR_OK)
                    break;
            }

            var_vibState_8c157a48.step_0x08++;
        }
    }

    if (var_vibState_8c157a48.playing_0x0c == 1) {
        var_vibState_8c157a48.frame_0x04++;
    }
}

/* A higher-priority pattern takes over without resetting step/frame, so it
 * starts partway through. Possibly an original-game quirk. */
void VibStart_8c010f7a(int pattern) {
    if (pattern < 8) {
        if (var_vibState_8c157a48.playing_0x0c == 1) {
            if (pattern > var_vibState_8c157a48.pattern_0x00) {
                var_vibState_8c157a48.pattern_0x00 = pattern;
            }
        } else if (var_vibState_8c157a48.playing_0x0c == 0) {
            VibClear_8c010fbe();
            var_vibState_8c157a48.pattern_0x00 = pattern;
        }
    }
}

void VibUpdate_8c010fae(int port) {
    if (var_vibState_8c157a48.pattern_0x00 != 7) {
        stepPattern_8c010e90(port);
    }
}

void VibClear_8c010fbe() {
    memset(&var_vibState_8c157a48, 0, sizeof(VibState));
    var_vibState_8c157a48.pattern_0x00 = 7;
}
