#include <shinobi.h>
#include "011120_asset_queues.h"
#include "serial_debug.h"

/* ==========================
 * Non-initialized Globals
 * ==========================
 */

/* basedir path buffer, filled at runtime */
char var_basedir_8c18ad6c[0x20];

extern void *var_frontNj_8c1bc434;
extern NJS_TEXLIST *var_frontTexlist_8c1bc430;
extern void *var_interiorNj_8c1bc43c;
extern NJS_TEXLIST *var_interiorTexlist_8c1bc438;
extern NJS_TEXLIST *var_markTexlist_8c1bc418;
extern NJS_TEXLIST *var_busStopTexlist_8c1bc424;
extern void *var_routeModelPairs_8c1bc3f4;

/* ======================
 * Initialized Globals
 * ======================
 */

/* nj/pvm route model pairs; pvm slot is the shared empty string */
NjPvmPairFilenames init_routeModelFilenames_8c043d64[] = {
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

void requestVehicleAssets_8c013ae8(void)
{
    AsqRequestNj_11492(var_basedir_8c18ad6c, "front.njd", &var_frontNj_8c1bc434, 0);
    AsqRequestPvm_11ac0(var_basedir_8c18ad6c, "front.pvm", &var_frontTexlist_8c1bc430, 0xf, 0);
    AsqRequestNj_11492(var_basedir_8c18ad6c, "syanai.njd", &var_interiorNj_8c1bc43c, 0);
    /* asm loads H'80000000 as attr here; Ghidra shows 0 -- asm wins */
    AsqRequestPvm_11ac0(var_basedir_8c18ad6c, "syanai.pvm", &var_interiorTexlist_8c1bc438, 0x40, 0x80000000);
    AsqRequestPvm_11ac0(var_basedir_8c18ad6c, "mark.pvm", &var_markTexlist_8c1bc418, 3, 0);
    AsqRequestPvm_11ac0(var_basedir_8c18ad6c, "busstop.pvm", &var_busStopTexlist_8c1bc424, 1, 0);
    var_routeModelPairs_8c1bc3f4 = AsqRequestNjPvmPairs_12030(var_basedir_8c18ad6c, init_routeModelFilenames_8c043d64, 0);
}

void freeVehicleAssets_8c013b5a(void)
{
    Uint32 i;

    if (var_interiorTexlist_8c1bc438 != (NJS_TEXLIST *) -1) {
        njSetTexture(var_interiorTexlist_8c1bc438);
        for (i = 0; i < var_interiorTexlist_8c1bc438->nbTexture; i++) {
            njReleaseCacheTextureNum(i);
        }
        AsqReleaseAndFreeTexlist_11e3c(var_interiorTexlist_8c1bc438);
        syFree(var_interiorNj_8c1bc43c);
        AsqReleaseAndFreeTexlist_11e3c(var_frontTexlist_8c1bc430);
        syFree(var_frontNj_8c1bc434);
        AsqReleaseAndFreeTexlist_11e3c(var_markTexlist_8c1bc418);
        AsqReleaseAndFreeTexlist_11e3c(var_busStopTexlist_8c1bc424);
        var_interiorTexlist_8c1bc438 = (NJS_TEXLIST *) -1;
    }
}
