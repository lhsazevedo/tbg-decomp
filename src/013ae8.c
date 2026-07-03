#include <shinobi.h>
#include "011120_asset_queues.h"
#include "serial_debug.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* One entry of the 0x20-slot route-model asset table (var_8c1bbddc). */
typedef struct {
    int requested_0x00;         /* model is in the current request list */
    int needsLoad_0x04;         /* request its files this pass */
    NJS_TEXLIST *texlist_0x08;  /* loaded pvm texlist, -1 when unloaded */
    void *nj_0x0c;              /* loaded njd data */
} RouteModelAsset;

/* ==========================
 * Non-initialized Globals
 * ==========================
 */

/* basedir path buffer, filled at runtime */
char var_basedir_8c18ad6c[0x20];

/* Selects the alternate filename table (init_8c043ecc) when == 2. */
int var_8c18ad20;

/* -1-terminated list of route-model indices to keep loaded */
char *var_8c18adb0;

extern RouteModelAsset var_8c1bbddc[0x20];

/* njd/pvm filename tables, one nj+pvm pointer pair per model. */
extern char *init_8c043dc4[];
extern char *init_8c043ecc[];

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

/* ======================
 * Forward Declarations
 * ======================
 */

/* pvm-ready flag helpers, still asm later in this TU */
void setUknPvmBool_8c014330(void);
void resetUknPvmBool_8c014322(void);

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

/* Reset the handle/id at +8 of each 0x10-byte slot to -1. Walks the array
 * back to front. count == 0 underflows the start index and writes nothing. */
void clearUnknownArray_8c013bbc(void *array, int count)
{
    char *p;

    for (p = (char *) array + (count - 1) * 0x10; p >= (char *) array; p -= 0x10) {
        /* TODO: likely a struct; type it and drop the raw +8 */
        *(Sint32 *)(p + 8) = -1;
    }
}

/* Reconcile loaded route-model assets with a -1-terminated list of model
 * indices: request files for newly-wanted models, free the ones dropped. */
void syncRouteModelAssets_8c013c34(char *models)
{
    RouteModelAsset *slot;
    char **names;
    int i;

    for (slot = var_8c1bbddc; slot < &var_8c1bbddc[0x20]; slot++) {
        slot->requested_0x00 = 0;
        slot->needsLoad_0x04 = 0;
    }

    while ((i = *models) != -1) {
        var_8c1bbddc[i].requested_0x00 = 1;
        models++;
        if (var_8c1bbddc[i].texlist_0x08 == (NJS_TEXLIST *) -1) {
            var_8c1bbddc[i].needsLoad_0x04 = 1;
        }
    }

    for (i = 0; i < 0x20; i++) {
        if (var_8c1bbddc[i].needsLoad_0x04 != 0) {
            if (var_8c18ad20 == 2) {
                AsqRequestNj_11492(var_basedir_8c18ad6c, init_8c043ecc[i * 2], 0, &var_8c1bbddc[i].nj_0x0c);
                names = init_8c043ecc;
            } else {
                AsqRequestNj_11492(var_basedir_8c18ad6c, init_8c043dc4[i * 2], 0, &var_8c1bbddc[i].nj_0x0c);
                names = init_8c043dc4;
            }
            AsqRequestPvm_11ac0(var_basedir_8c18ad6c, names[i * 2 + 1], &var_8c1bbddc[i].texlist_0x08, 0x60, 0);
        } else if (var_8c1bbddc[i].requested_0x00 == 0 && var_8c1bbddc[i].texlist_0x08 != (NJS_TEXLIST *) -1) {
            AsqReleaseAndFreeTexlist_11e3c(var_8c1bbddc[i].texlist_0x08);
            syFree(var_8c1bbddc[i].nj_0x0c);
            var_8c1bbddc[i].texlist_0x08 = (NJS_TEXLIST *) -1;
        }
    }
}

/* afterTexlist callback for an asset-load pass: mark pvm assets ready,
 * then release the request queues. */
void finishAssetLoad_8c013d42(void)
{
    setUknPvmBool_8c014330();
    AsqFreeQueues_11f7e();
}

/* (Re)initialize the asset queues and kick off one load pass for the
 * currently-wanted route models. finishAssetLoad runs once the texlists land. */
void FUN_8c013d78(void)
{
    AsqInitQueues_11f36(0, 0x40, 0, 0x40);
    AsqResetQueues_11f6c();
    resetUknPvmBool_8c014322();
    syncRouteModelAssets_8c013c34(var_8c18adb0);
    AsqProcessQueues_11fe0(AsqNop_11120, 0, 0, 0, finishAssetLoad_8c013d42);
}
