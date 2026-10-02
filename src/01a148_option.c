/* @unit Option */
#include <shinobi.h>
#include <njdef.h>
#include <sg_sd.h>
#include "0100bc_sound.h"
#include "011120_asset_queues.h"
#include "012324_input.h"
#include "014f54_sprite.h"
#include "015ab8_title.h"
#include "018644_file_select.h"
#include "019e98_main_menu.h"
#include "01a148_option.h"
#include "022464_render.h"
#include "014a9c_tasks.h"
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define CHANGE_STATE(x) var_menuState_8c1bc7a8.state_0x18 = (x)

/* Each screen's own fade-out phase; only 0 and 1 are shared (see OPTION_STATE). */
#define TOP_MENU_FADE_OUT   2
#define KEY_CONFIG_FADE_OUT 6
#define AUDIO_FADE_OUT      9

/* ====================
 * Type Declarations
 * ====================
 */

/* state_0x18. Only 0 and 1 mean the same on all four screens; from 2 up each
 * numbers its own phases, so EDIT/FADE_OUT are the SETTING screen's. */
enum OPTION_STATE {
    OPTION_STATE_FADE_IN  = 0,
    OPTION_STATE_NAVIGATE = 1,
    OPTION_STATE_EDIT     = 2,
    OPTION_STATE_FADE_OUT = 3,
};

/* ====================
 * Non-initialized Globals
 * ====================
 */

Sint8 var_soundMode_8c226070;
/* The SETTING rows, starting at var_progress_8c1ba1cc.difficulty_0xc4. */
STATIC char *var_settingValues_8c226074;
/* Sound-test fields, one int per decimal digit, ones at index 0. Must stay
 * contiguous: enterAudio_8c01afd8 clears across them. */
STATIC int var_musicTestDigits_8c226078[2];
STATIC int var_sfxTestDigits_8c226080[2];
STATIC int var_voiceTestDigits_8c226088[4];

/* ====================
 * Initialized Globals
 * ====================
 */

/* SETTING per-row option counts: DIFFICULTY, DRIVE MODE, DEFAULT VIEW, VIBRATION,
 * SCREEN ROLL. Each toggle wraps within its count. */
STATIC char init_settingOptionCounts_8c044de0[5] = { 3, 2, 4, 2, 2 };

/* KEY CONFIGURE sensitivity bar; the left x and the y are set per draw. */
STATIC NJS_POLYGON_VTX init_sensitivityBarQuad_8c044de8[4] = {
    {   0.0f, 0.0f, 0.8264462351799011f, ARGB(0xc4, 0xf4, 0xf4, 0x00) },
    {   0.0f, 0.0f, 0.8264462351799011f, ARGB(0xc4, 0xf4, 0xf4, 0x00) },
    { 576.0f, 0.0f, 0.8264462351799011f, ARGB(0xc4, 0xf4, 0x00, 0x00) },
    { 576.0f, 0.0f, 0.8264462351799011f, ARGB(0xc4, 0xf4, 0x00, 0x00) },
};

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void enterSetting_8c01a3c0(Task *task);
STATIC void enterKeyConfig_8c01a89c(Task *task);
STATIC void enterAudio_8c01afd8(Task *task);

/* ====================
 * Initialized Globals (continued: needs the forward decls above)
 * ====================
 */

/* Indexed by top-menu row: SETTING, KEY CONFIGURE, AUDIO, RETURN. */
STATIC TaskAction init_topMenuActions_8c044e28[4] = {
    enterSetting_8c01a3c0,
    enterKeyConfig_8c01a89c,
    enterAudio_8c01afd8,
    MainMenuEnter_8c01a09a,
};

/* ====================
 * Functions
 * ====================
 */

/* Rows 0-4 are the toggles, row 5 DEFAULT, row 6 RETURN. */
STATIC void settingTask_8c01a148(Task *task)
{
    MenuState *m = &var_menuState_8c1bc7a8;
    unsigned int press = var_peripherals_8c1ba35c[0].press;
    int i;

    switch (m->state_0x18) {
        case OPTION_STATE_FADE_IN: {
            if (var_isFading_8c226568 == 0) {
                CHANGE_STATE(OPTION_STATE_NAVIGATE);
            }
            break;
        }

        case OPTION_STATE_NAVIGATE: {
            if (press & PDD_DGT_TA) {
                if (m->selected_0x38 < 5) {
                    CHANGE_STATE(OPTION_STATE_EDIT);
                } else if (m->selected_0x38 == 5) {
                    FileSelectResetSettingDefaults_8c018862();
                } else {
                    CHANGE_STATE(OPTION_STATE_FADE_OUT);
                    RenderStartFadeOut_8c022b60(10);
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            } else if (press & PDD_DGT_TB) {
                CHANGE_STATE(OPTION_STATE_FADE_OUT);
                RenderStartFadeOut_8c022b60(10);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
            } else if (press & PDD_DGT_KU) {
                m->selected_0x38 -= 1;
                if (m->selected_0x38 < 0) {
                    m->selected_0x38 = 6;
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            } else if (press & PDD_DGT_KD) {
                m->selected_0x38 += 1;
                if (m->selected_0x38 > 6) {
                    m->selected_0x38 = 0;
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            }
            break;
        }

        case OPTION_STATE_EDIT: {
            if (press & PDD_DGT_TA) {
                CHANGE_STATE(OPTION_STATE_NAVIGATE);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            } else if (press & PDD_DGT_KL) {
                var_settingValues_8c226074[m->selected_0x38] -= 1;
                if (var_settingValues_8c226074[m->selected_0x38] < 0) {
                    var_settingValues_8c226074[m->selected_0x38] = init_settingOptionCounts_8c044de0[m->selected_0x38] - 1;
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            } else if (press & PDD_DGT_KR) {
                var_settingValues_8c226074[m->selected_0x38] += 1;
                if (var_settingValues_8c226074[m->selected_0x38] >= init_settingOptionCounts_8c044de0[m->selected_0x38]) {
                    var_settingValues_8c226074[m->selected_0x38] = 0;
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            }
            break;
        }

        case OPTION_STATE_FADE_OUT: {
            if (var_isFading_8c226568 == 0) {
                OptionEnter_8c01b122(task, 0);
                return;
            }
            break;
        }
    }

    for (i = 0; i < 5; i++) {
        int idx = (i == m->selected_0x38) ? i + 0x23 : i + 0x1e;
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, idx, 0.0f, 0.0f, -5.0f);
        /* Value marker; blinks while its row is being edited. */
        if (m->state_0x18 != OPTION_STATE_EDIT || i != m->selected_0x38 ||
            (m->timer_0x68++ & 1)) {
            SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x29,
                                   337.0f + 64.0f * var_settingValues_8c226074[i],
                                   77.0f + 56.0f * i, -4.0f);
        }
    }

    if (m->selected_0x38 < 5) {
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x61, 0.0f, 0.0f, -4.0f);
    } else {
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c,
                               (m->selected_0x38 == 5) ? 0x62 : 0x63,
                               0.0f, 0.0f, -4.0f);
    }
    SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x28, 0.0f, 0.0f, -5.0f);
    SpriteDraw_8c014f54(&m->resourceGroupA_0x00, 0, 0.0f, 0.0f, -7.0f);
}

STATIC void enterSetting_8c01a3c0(Task *task)
{
    TaskReinstall_8c014b3e(task, settingTask_8c01a148);
    var_menuState_8c1bc7a8.state_0x18 = OPTION_STATE_FADE_IN;
    var_menuState_8c1bc7a8.selected_0x38 = 0;
    RenderStartFadeIn_8c022a9c(10);
}

STATIC void cycleValue_8c01a3da(char *value, char count)
{
    char v = *value;

    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KL) {
        v -= 1;
        if (v < 0) {
            v = count - 1;
        }
        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KR) {
        v += 1;
        if (v >= count) {
            v = 0;
        }
        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
    }

    *value = v;
}

/* value is 0-0x80. Returns the bar's left edge. */
STATIC float drawSensitivityBar_8c01a42a(float y, unsigned char value)
{
    float x = (float)value * 320.0f / 256.0f + 256.0f;

    /* The left-edge x is stored twice, mirroring the original's redundant stores. */
    init_sensitivityBarQuad_8c044de8[1].x = x;
    init_sensitivityBarQuad_8c044de8[0].x = x;
    init_sensitivityBarQuad_8c044de8[2].y = y;
    init_sensitivityBarQuad_8c044de8[0].y = y;
    init_sensitivityBarQuad_8c044de8[1].x = x;
    init_sensitivityBarQuad_8c044de8[0].x = x;
    init_sensitivityBarQuad_8c044de8[3].y = y + 20.0f;
    init_sensitivityBarQuad_8c044de8[1].y = y + 20.0f;
    njDrawPolygon(init_sensitivityBarQuad_8c044de8, 4, 1);
    return x;
}

/* Returns 1 once the edit is left: A, B, or an unsupported controller. */
STATIC int keyConfigEditExit_8c01a4b4(void)
{
    if (var_activeCtrlType_8c157a70 == BT_CONTROLLER || var_activeCtrlType_8c157a70 == BT_RACING) {
        if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
            var_menuState_8c1bc7a8.state_0x18 = OPTION_STATE_NAVIGATE;
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            return 1;
        }
        if (!(var_peripherals_8c1ba35c[0].press & PDD_DGT_TB)) {
            return 0;
        }
    }
    var_menuState_8c1bc7a8.state_0x18 = OPTION_STATE_NAVIGATE;
    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
    return 1;
}

/* Rows 0-2 are edited in state row + 2, row 3 is DEFAULT, row 4 RETURN:
 *   2 = button assignment, per controller type and drive mode
 *   3 = ACCEL sensitivity
 *   4 = BRAKE sensitivity */
STATIC void keyConfigTask_8c01a50c(Task *task)
{
    MenuState *m = &var_menuState_8c1bc7a8;
    unsigned int press = var_peripherals_8c1ba35c[0].press;
    int i;

    switch (m->state_0x18) {
        case OPTION_STATE_FADE_IN: {
            if (var_isFading_8c226568 == 0) {
                CHANGE_STATE(OPTION_STATE_NAVIGATE);
            }
            break;
        }

        case OPTION_STATE_NAVIGATE: {
            if (press & PDD_DGT_TA) {
                if (m->selected_0x38 < 3) {
                    CHANGE_STATE(m->selected_0x38 + 2);
                } else if (m->selected_0x38 == 3) {
                    FileSelectResetKeyConfigDefaults_8c0188bc();
                } else {
                    CHANGE_STATE(KEY_CONFIG_FADE_OUT);
                    RenderStartFadeOut_8c022b60(10);
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            } else if (press & PDD_DGT_TB) {
                CHANGE_STATE(KEY_CONFIG_FADE_OUT);
                RenderStartFadeOut_8c022b60(10);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
            } else if (press & PDD_DGT_KU) {
                m->selected_0x38 -= 1;
                if (m->selected_0x38 < 0) {
                    m->selected_0x38 = 4;
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            } else if (press & PDD_DGT_KD) {
                m->selected_0x38 += 1;
                if (m->selected_0x38 > 4) {
                    m->selected_0x38 = 0;
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            }
            break;
        }

        case 2: {
            if (!keyConfigEditExit_8c01a4b4()) {
                if (var_progress_8c1ba1cc.driveMode_0xc5 == 0) {
                    if (var_activeCtrlType_8c157a70 == BT_CONTROLLER) {
                        cycleValue_8c01a3da(&var_progress_8c1ba1cc.btnConfigManual_0xcc, 3);
                    } else if (var_activeCtrlType_8c157a70 == BT_RACING) {
                        cycleValue_8c01a3da(&var_progress_8c1ba1cc.btnConfigWheelManual_0xce, 2);
                    }
                } else {
                    if (var_activeCtrlType_8c157a70 == BT_CONTROLLER) {
                        cycleValue_8c01a3da(&var_progress_8c1ba1cc.btnConfigAuto_0xcd, 3);
                    } else if (var_activeCtrlType_8c157a70 == BT_RACING) {
                        cycleValue_8c01a3da(&var_progress_8c1ba1cc.btnConfigWheelAuto_0xcf, 3);
                    }
                }
            }
            break;
        }

        case 3: {
            if (!keyConfigEditExit_8c01a4b4()) {
                var_progress_8c1ba1cc.accelSensitivity_0xd0 = var_peripherals_8c1ba35c[0].r;
                if (var_progress_8c1ba1cc.accelSensitivity_0xd0 > 0x80) {
                    var_progress_8c1ba1cc.accelSensitivity_0xd0 = 0x80;
                }
            }
            break;
        }

        case 4: {
            if (!keyConfigEditExit_8c01a4b4()) {
                var_progress_8c1ba1cc.brakeSensitivity_0xd1 = var_peripherals_8c1ba35c[0].l;
                if (var_progress_8c1ba1cc.brakeSensitivity_0xd1 > 0x80) {
                    var_progress_8c1ba1cc.brakeSensitivity_0xd1 = 0x80;
                }
            }
            break;
        }

        case KEY_CONFIG_FADE_OUT: {
            if (var_isFading_8c226568 == 0) {
                OptionEnter_8c01b122(task, 1);
                return;
            }
            break;
        }
    }

    for (i = 0; i < 3; i++) {
        int idx = (i == m->selected_0x38) ? i + 0x2e : i + 0x2a;
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, idx, 0.0f, 0.0f, -5.0f);
    }

    if (m->state_0x18 == 2) {
        int idx = -1;

        if (var_progress_8c1ba1cc.driveMode_0xc5 == 0) {
            if (var_activeCtrlType_8c157a70 == BT_CONTROLLER) {
                idx = var_progress_8c1ba1cc.btnConfigManual_0xcc + 0x35;
            } else if (var_activeCtrlType_8c157a70 == BT_RACING) {
                idx = var_progress_8c1ba1cc.btnConfigWheelManual_0xce + 0x3b;
            }
        } else {
            if (var_activeCtrlType_8c157a70 == BT_CONTROLLER) {
                idx = var_progress_8c1ba1cc.btnConfigAuto_0xcd + 0x38;
            } else if (var_activeCtrlType_8c157a70 == BT_RACING) {
                idx = var_progress_8c1ba1cc.btnConfigWheelAuto_0xcf + 0x3d;
            }
        }

        if (idx != -1) {
            SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, idx, 0.0f, 0.0f, -5.0f);
        }
    } else {
        float x;

        x = drawSensitivityBar_8c01a42a(144.0f, var_progress_8c1ba1cc.accelSensitivity_0xd0);
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, (m->state_0x18 == 3) ? 0x44 : 0x43,
                               x - 10.0f, 124.0f, -5.0f);
        x = drawSensitivityBar_8c01a42a(224.0f, var_progress_8c1ba1cc.brakeSensitivity_0xd1);
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, (m->state_0x18 == 4) ? 0x44 : 0x43,
                               x - 10.0f, 203.0f, -5.0f);
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x40, 0.0f, 0.0f, -5.0f);
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x41, 0.0f, 0.0f, -5.0f);
    }

    if (m->selected_0x38 < 3) {
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x61, 0.0f, 0.0f, -5.0f);
    } else {
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, (m->selected_0x38 == 3) ? 0x62 : 0x63,
                               0.0f, 0.0f, -5.0f);
    }
    SpriteDraw_8c014f54(&m->resourceGroupA_0x00, 0, 0.0f, 0.0f, -7.0f);
}

STATIC void enterKeyConfig_8c01a89c(Task *task)
{
    TaskReinstall_8c014b3e(task, keyConfigTask_8c01a50c);
    var_menuState_8c1bc7a8.state_0x18 = OPTION_STATE_FADE_IN;
    var_menuState_8c1bc7a8.selected_0x38 = 0;
    RenderStartFadeIn_8c022a9c(10);
}

STATIC void audioEditValue_8c01a8b6(char *value, char count)
{
    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
        var_menuState_8c1bc7a8.state_0x18 = OPTION_STATE_NAVIGATE;
        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TB) {
        var_menuState_8c1bc7a8.state_0x18 = OPTION_STATE_NAVIGATE;
        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
    }
    cycleValue_8c01a3da(value, count);
}

STATIC int soundTestFieldRead_8c01a904(int *digits, int count)
{
    int value = 0;
    int i;

    for (i = count - 1; i >= 0; i--) {
        value = value * 10 + digits[i];
    }
    return value;
}

/* L/R pick the digit, U/D step it with carry, wrapping within [0, max]. B
 * leaves the field and stops its playback. */
STATIC void soundTestFieldAdjust_8c01a926(int *digits, int count, int max)
{
    unsigned int press = var_peripherals_8c1ba35c[0].press;
    int i;

    if (press & PDD_DGT_KL) {
        if (var_menuState_8c1bc7a8.cursorCol_0x3c < count - 1) {
            var_menuState_8c1bc7a8.cursorCol_0x3c += 1;
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
        }
    } else if (press & PDD_DGT_KR) {
        if (var_menuState_8c1bc7a8.cursorCol_0x3c > 0) {
            var_menuState_8c1bc7a8.cursorCol_0x3c -= 1;
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
        }
    }

    if (press & PDD_DGT_KU) {
        for (i = var_menuState_8c1bc7a8.cursorCol_0x3c; i < count; i++) {
            int v = digits[i] + 1;
            digits[i] = v;
            if (v <= 9) {
                break;
            }
            digits[i] = 0;
        }
        if (soundTestFieldRead_8c01a904(digits, count) > max) {
            for (i = 0; i < count; i++) {
                digits[i] = 0;
            }
        }
    } else if (press & PDD_DGT_KD) {
        for (i = var_menuState_8c1bc7a8.cursorCol_0x3c; i < count; i++) {
            int v = digits[i] - 1;
            digits[i] = v;
            if (v >= 0) {
                break;
            }
            digits[i] = 9;
        }
        if (i >= count) {
            for (i = 0; i < count; i++) {
                digits[i] = max % 10;
                max = max / 10;
            }
        }
    }

    if (!(press & PDD_DGT_TB)) {
        return;
    }
    if (var_menuState_8c1bc7a8.state_0x18 == 6) {
        SndStopAdx_8c010ca6(0);
    } else if (var_menuState_8c1bc7a8.state_0x18 == 7) {
        sdMidiStop(var_midiHandles_8c0fcd28[0]);
    } else if (var_menuState_8c1bc7a8.state_0x18 == 8) {
        SndStopAdx_8c010ca6(1);
    }
    var_menuState_8c1bc7a8.state_0x18 = OPTION_STATE_NAVIGATE;
    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
}

/* Right to left from x; sprite 0x54 is '0'. */
STATIC void soundTestFieldDraw_8c01aaaa(float x, float y, int *digits, int count)
{
    int i;

    for (i = 0; i < count; i++) {
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                               digits[i] + 0x54, x, y, -4.0f);
        x -= 26.0f;
    }
}

/* Rows 0-6 are edited in state row + 2, row 7 is DEFAULT, row 8 RETURN.
 * cursorCol_0x3c is the sound-test digit being edited. */
STATIC void audioTask_8c01ab08(Task *task)
{
    MenuState *m = &var_menuState_8c1bc7a8;
    unsigned int press = var_peripherals_8c1ba35c[0].press;
    int i;

    switch (m->state_0x18) {
        case OPTION_STATE_FADE_IN: {
            if (var_isFading_8c226568 == 0) {
                CHANGE_STATE(OPTION_STATE_NAVIGATE);
            }
            break;
        }

        case OPTION_STATE_NAVIGATE: {
            if (press & PDD_DGT_TA) {
                if (m->selected_0x38 < 7) {
                    CHANGE_STATE(m->selected_0x38 + 2);
                    m->cursorCol_0x3c = 0;
                } else if (m->selected_0x38 == 7) {
                    FileSelectResetSoundDefaults_8c0188dc();
                } else {
                    CHANGE_STATE(AUDIO_FADE_OUT);
                    SndSetSoundMode_8c0108c0(var_soundMode_8c226070);
                    RenderStartFadeOut_8c022b60(10);
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            } else if (press & PDD_DGT_TB) {
                CHANGE_STATE(AUDIO_FADE_OUT);
                SndSetSoundMode_8c0108c0(var_soundMode_8c226070);
                RenderStartFadeOut_8c022b60(10);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
            } else if (press & PDD_DGT_KU) {
                m->selected_0x38 -= 1;
                if (m->selected_0x38 < 0) {
                    m->selected_0x38 = 8;
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            } else if (press & PDD_DGT_KD) {
                m->selected_0x38 += 1;
                if (m->selected_0x38 > 8) {
                    m->selected_0x38 = 0;
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            }
            break;
        }

        case 2: {   /* SOUND mode */
            audioEditValue_8c01a8b6(&var_soundMode_8c226070, 2);
            break;
        }

        case 3: {   /* MUSIC volume */
            audioEditValue_8c01a8b6(&var_progress_8c1ba1cc.musicVolume_0xd4, 10);
            if (press & PDD_DGT_TA) {
                SndSetAdxVol_8c010972(var_progress_8c1ba1cc.musicVolume_0xd4, 0);
            }
            break;
        }

        case 4: {   /* SFX volume */
            audioEditValue_8c01a8b6(&var_progress_8c1ba1cc.sfxVolume_0xd5, 10);
            if (press & PDD_DGT_TA) {
                SndSetMidiVol_8c0109f4(var_progress_8c1ba1cc.sfxVolume_0xd5);
            }
            break;
        }

        case 5: {   /* VOICE volume */
            audioEditValue_8c01a8b6(&var_progress_8c1ba1cc.voiceVolume_0xd6, 10);
            if (press & PDD_DGT_TA) {
                SndSetAdxVol_8c010972(var_progress_8c1ba1cc.voiceVolume_0xd6, 1);
            }
            break;
        }

        case 6: {   /* MUSIC test */
            if (press & PDD_DGT_TA) {
                SndPlayBgm_8c0107ac(soundTestFieldRead_8c01a904(var_musicTestDigits_8c226078, 2));
            } else {
                soundTestFieldAdjust_8c01a926(var_musicTestDigits_8c226078, 2, 0x10);
            }
            SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x70,
                                   436.0f - (float)m->cursorCol_0x3c * 26.0f, 220.0f, -3.0f);
            break;
        }

        case 7: {   /* SFX test */
            if (press & PDD_DGT_TA) {
                SndPlaySfx_8c0106d2(soundTestFieldRead_8c01a904(var_sfxTestDigits_8c226080, 2));
            } else {
                soundTestFieldAdjust_8c01a926(var_sfxTestDigits_8c226080, 2, 0x44);
            }
            SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x70,
                                   436.0f - (float)m->cursorCol_0x3c * 26.0f, 256.0f, -3.0f);
            break;
        }

        case 8: {   /* VOICE test */
            if (press & PDD_DGT_TA) {
                SndPlayVoice_8c010720(soundTestFieldRead_8c01a904(var_voiceTestDigits_8c226088, 4));
            } else {
                soundTestFieldAdjust_8c01a926(var_voiceTestDigits_8c226088, 4, 0x56c);
            }
            SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x70,
                                   436.0f - (float)m->cursorCol_0x3c * 26.0f, 292.0f, -3.0f);
            break;
        }

        case AUDIO_FADE_OUT: {
            if (var_isFading_8c226568 == 0) {
                OptionEnter_8c01b122(task, 2);
                return;
            }
            break;
        }
    }

    for (i = 0; i < 7; i++) {
        int idx = (i == m->selected_0x38) ? i + 0x4c : i + 0x45;
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, idx, 0.0f, 0.0f, -5.0f);
    }

    /* SOUND mode and volume markers; each blinks while being edited. */
    if (m->state_0x18 != 2 || m->selected_0x38 != 0 || (m->timer_0x68++ & 1)) {
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x29,
                               (float)var_soundMode_8c226070 * 64.0f + 401.0f, 69.0f, -4.0f);
    }
    if (m->state_0x18 != 3 || (m->timer_0x68++ & 1)) {
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x5e,
                               (float)var_progress_8c1ba1cc.musicVolume_0xd4 * 20.0f + 400.0f, 110.0f, -4.0f);
    }
    if (m->state_0x18 != 4 || (m->timer_0x68++ & 1)) {
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x5e,
                               (float)var_progress_8c1ba1cc.sfxVolume_0xd5 * 20.0f + 400.0f, 146.0f, -4.0f);
    }
    if (m->state_0x18 != 5 || (m->timer_0x68++ & 1)) {
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x5e,
                               (float)var_progress_8c1ba1cc.voiceVolume_0xd6 * 20.0f + 400.0f, 182.0f, -4.0f);
    }

    soundTestFieldDraw_8c01aaaa(441.0f, 225.0f, var_musicTestDigits_8c226078, 2);
    soundTestFieldDraw_8c01aaaa(441.0f, 261.0f, var_sfxTestDigits_8c226080, 2);
    soundTestFieldDraw_8c01aaaa(441.0f, 298.0f, var_voiceTestDigits_8c226088, 4);

    if (m->selected_0x38 < 7) {
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x61, 0.0f, 0.0f, -4.0f);
    } else {
        SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, (m->selected_0x38 == 7) ? 0x62 : 0x63,
                               0.0f, 0.0f, -4.0f);
    }
    SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x53, 0.0f, 0.0f, -5.0f);
    SpriteDraw_8c014f54(&m->resourceGroupA_0x00, 0, 0.0f, 0.0f, -7.0f);
}

STATIC void enterAudio_8c01afd8(Task *task)
{
    TaskReinstall_8c014b3e(task, audioTask_8c01ab08);
    var_menuState_8c1bc7a8.state_0x18 = OPTION_STATE_FADE_IN;
    var_menuState_8c1bc7a8.selected_0x38 = 0;
    var_musicTestDigits_8c226078[1] = 0;
    var_musicTestDigits_8c226078[0] = 0;
    var_sfxTestDigits_8c226080[2] = 0;   /* original's redundant store to var_voiceTestDigits_8c226088[0] */
    var_sfxTestDigits_8c226080[1] = 0;
    var_sfxTestDigits_8c226080[0] = 0;
    var_voiceTestDigits_8c226088[3] = 0;
    var_voiceTestDigits_8c226088[2] = 0;
    var_voiceTestDigits_8c226088[1] = 0;
    var_voiceTestDigits_8c226088[0] = 0;
    RenderStartFadeIn_8c022a9c(10);
}

STATIC void topMenuTask_8c01b00a(Task *task)
{
    MenuState *m = &var_menuState_8c1bc7a8;
    unsigned int press = var_peripherals_8c1ba35c[0].press;

    switch (m->state_0x18) {
        case OPTION_STATE_FADE_IN: {
            if (var_isFading_8c226568 == 0) {
                CHANGE_STATE(OPTION_STATE_NAVIGATE);
            }
            break;
        }

        case OPTION_STATE_NAVIGATE: {
            if (press & PDD_DGT_TA) {
                CHANGE_STATE(TOP_MENU_FADE_OUT);
                m->returnAction_0x70 = (int)init_topMenuActions_8c044e28[m->selected_0x38];
                m->returnActionArg_0x74 = 2;
                RenderStartFadeOut_8c022b60(10);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            } else if (press & PDD_DGT_TB) {
                CHANGE_STATE(TOP_MENU_FADE_OUT);
                m->returnAction_0x70 = (int)MainMenuEnter_8c01a09a;
                m->returnActionArg_0x74 = 2;
                RenderStartFadeOut_8c022b60(10);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
            } else if (press & PDD_DGT_KU) {
                m->selected_0x38 -= 1;
                if (m->selected_0x38 < 0) {
                    m->selected_0x38 = 3;
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            } else if (press & PDD_DGT_KD) {
                m->selected_0x38 += 1;
                if (m->selected_0x38 > 3) {
                    m->selected_0x38 = 0;
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            }
            break;
        }

        case TOP_MENU_FADE_OUT: {
            if (var_isFading_8c226568 == 0) {
                ((TaskAction)m->returnAction_0x70)(task, (void *)m->returnActionArg_0x74);
                return;
            }
            break;
        }
    }

    SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, m->selected_0x38 + 0x1a, 0.0f, 0.0f, -4.0f);
    SpriteDraw_8c014f54(&m->resourceGroupB_0x0c, 0x19, 0.0f, 0.0f, -5.0f);
    SpriteDraw_8c014f54(&m->resourceGroupA_0x00, 0, 0.0f, 0.0f, -7.0f);
}

void OptionEnter_8c01b122(Task *task, int row)
{
    TaskReinstall_8c014b3e(task, topMenuTask_8c01b00a);
    var_menuState_8c1bc7a8.state_0x18 = OPTION_STATE_FADE_IN;
    var_menuState_8c1bc7a8.selected_0x38 = row;
    var_settingValues_8c226074 = &var_progress_8c1ba1cc.difficulty_0xc4;
    RenderStartFadeIn_8c022a9c(10);
}
