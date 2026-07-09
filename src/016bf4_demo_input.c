/* 8c016bf4 */
#include <shinobi.h>
#include "014f54_text.h"
#include "016bf4_demo_input.h"

extern PDS_PERIPHERAL var_peripherals_8c1ba35c[2];
extern int var_8c1bbc84;

void demoInputTask_8c016bf4()
{
    Uint32 on;

    if ((var_8c1bbc84 > 0) && (var_demoCursor_8c225fa8 < &var_demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY])) {
        on = var_demoCursor_8c225fa8->on;
        var_peripherals_8c1ba35c[0].on = on;
        var_peripherals_8c1ba35c[0].press = on & (var_demoPrevOn_8c225fac ^ on);
        var_demoPrevOn_8c225fac = var_peripherals_8c1ba35c[0].on;
        var_peripherals_8c1ba35c[0].x1 = var_demoCursor_8c225fa8->x1;
        var_peripherals_8c1ba35c[0].r = var_demoCursor_8c225fa8->r;
        var_peripherals_8c1ba35c[0].l = var_demoCursor_8c225fa8->l;
        var_demoCursor_8c225fa8++;
    }
}
