/* 8c016c58 */
#include <shinobi.h>
#include <sg_sd.h>
#include "016c58_prompt.h"

extern SDMIDI var_midiHandles_8c0fcd28[7];
extern PDS_PERIPHERAL var_peripherals_8c1ba35c[2];

int promptHandleMultiple_16c58(int *option, int count)
{
    int newOption = *option;

    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KL) {
        if (--newOption < 0) {
            newOption = count - 1;
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KR) {
        if (++newOption >= count) {
            newOption = 0;
        }
    } else {
        *option = newOption;
        return 0;
    }

    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
    *option = newOption;
    return 1;
}

int promptHandleBinary_16caa(int* option) {
    if (*option == 0) {
        if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KR) {
            *option = 1;
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KL) {
        *option = 0;
        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
    }

    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
        if (*option == 0) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            return 1;
        } else {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
            return 2;
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TB) {
        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
        return 2;
    }

    return 0;
}
