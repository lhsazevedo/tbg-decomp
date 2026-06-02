/* 8c016bf4 */
#include <shinobi.h>
#include "014f54_text.h"

extern PDS_PERIPHERAL var_peripherals_8c1ba35c[2];
extern int var_8c1bbc84;

void demoInputTask_8c016bf4()
{
    Uint32 on;

    if ((var_8c1bbc84 > 0) && (demoCursor_8c225fa8 < &demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY])) {
        on = demoCursor_8c225fa8->on;
        var_peripherals_8c1ba35c[0].on = on;
        var_peripherals_8c1ba35c[0].press = on & (demoPrevOn_8c225fac ^ on);
        demoPrevOn_8c225fac = var_peripherals_8c1ba35c[0].on;
        var_peripherals_8c1ba35c[0].x1 = demoCursor_8c225fa8->x1;
        var_peripherals_8c1ba35c[0].r = demoCursor_8c225fa8->r;
        var_peripherals_8c1ba35c[0].l = demoCursor_8c225fa8->l;
        demoCursor_8c225fa8++;
    }
}
