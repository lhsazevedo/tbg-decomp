#include <shinobi.h>
#include "011120_asset_queues.h"
#include "serial_debug.h"

/* ==========================
 * Non-initialized Globals
 * ==========================
 */

/* basedir path buffer, filled at runtime */
char var_8c18ad6c[0x20];

extern void *var_8c1bc434;
extern void *var_8c1bc430;
extern void *var_8c1bc43c;
extern void *var_8c1bc438;
extern void *var_8c1bc418;
extern void *var_8c1bc424;
extern void *var_8c1bc3f4;

/* ======================
 * Initialized Globals
 * ======================
 */

/* nj/pvm route model pairs; pvm slot is the shared empty string */
NjPvmPairFilenames init_8c043d64[] = {
    { "3s_2do_x.njd", "" },
    { "3s_4wd_x.njd", "" },
    { "3s_sed_x.njd", "" },
    { "3s_tax_x.njd", "" },
    { "3s_tor_x.njd", "" },
    { "3s_kto_x.njd", "" },
    { "3s_dan_x.njd", "" },
    { "3s_wag_x.njd", "" },
    { "3s_bus_x.njd", "" },
    { "3s_pat_x.njd", "" },
    { "3s_kyu_x.njd", "" },
    { "", "" },
};

/* ==========
 * Functions
 * ==========
 */

void requestSomeFiles_8c013ae8(void)
{
    AsqRequestNj_11492(var_8c18ad6c, "front.njd", &var_8c1bc434, 0);
    AsqRequestPvm_11ac0(var_8c18ad6c, "front.pvm", &var_8c1bc430, 0xf, 0);
    AsqRequestNj_11492(var_8c18ad6c, "syanai.njd", &var_8c1bc43c, 0);
    /* asm loads H'80000000 as attr here; Ghidra shows 0 -- asm wins */
    AsqRequestPvm_11ac0(var_8c18ad6c, "syanai.pvm", &var_8c1bc438, 0x40, 0x80000000);
    AsqRequestPvm_11ac0(var_8c18ad6c, "mark.pvm", &var_8c1bc418, 3, 0);
    AsqRequestPvm_11ac0(var_8c18ad6c, "busstop.pvm", &var_8c1bc424, 1, 0);
    var_8c1bc3f4 = AsqRequestNjPvmPairs_12030(var_8c18ad6c, init_8c043d64, 0);
}
