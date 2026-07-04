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
    NJS_TEXLIST *texlist_0x08;  /* loaded texture set (pvm), -1 when unloaded */
    void *nj_0x0c;              /* loaded model (njd) */
} RouteModelAsset;

/* Fog params (shared with 012f44). */
typedef struct {
    int field_0x00;
    int field_0x04;
    Uint8 field_0x08;
    Uint8 field_0x09;
    Uint8 field_0x0a;
    Uint8 field_0x0b;
    float fogN_0x0c;
    float fogF_0x10;
} s_8c18ad28;

/* Entry in var_8c18ad18's array; most fields still unidentified. */
typedef struct {
    char _ukn_0x00[0x0c];
    int ukn_0x0c;                    /* dat count for the request loop */
    char *ukn_0x10;                  /* route-model list (syncRouteModelAssets) */
    char _ukn_0x14[0x04];
    char *ukn_0x18;                  /* second-table list (FUN_8c013df6) */
    int ukn_0x1c;                    /* FUN_8c029ad4 arg */
    char **ukn_0x20;                 /* dat filenames for the request loop */
    s_8c18ad28 *ukn_0x24;            /* fog params */
    NjPvmPairFilenames *ukn_0x28;    /* nj/pvm pairs */
} UknEntry;

/* Pointed to by var_8c18ad18; +8 holds the UknEntry array. */
typedef struct {
    char _ukn_0x00[0x08];
    UknEntry *entries_0x08;
} Ukn;

/* ==========================
 * Non-initialized Globals
 * ==========================
 */

/* basedir path buffer, filled at runtime */
char var_basedir_8c18ad6c[0x20];

Ukn *var_8c18ad18;

/* fog params for the current entry (from UknEntry.ukn_0x24) */
s_8c18ad28 *var_8c18ad28;

/* path buffer for dat requests, parallel to var_basedir_8c18ad6c */
char var_8c18ad2c[0x20];

/* Selects the alternate filename table (init_8c043ecc) when == 2. */
int var_8c18ad20;

/* -1-terminated list of route-model indices to keep loaded */
char *var_8c18adb0;

/* dat handles filled by the request loop (4 slots) */
void *var_8c18adb4[4];

extern RouteModelAsset var_8c1bbddc[0x20];

/* Second, 0x41-slot asset table (pvm only); FUN_8c013df6 reconciles it. */
extern RouteModelAsset var_8c1bbfdc[0x41];

/* njd/pvm filename tables, one nj+pvm pointer pair per model. */
extern char *init_8c043dc4[];
extern char *init_8c043ecc[];

/* one pvm filename per var_8c1bbfdc slot */
extern char *init_8c043fd8[];

extern void *var_frontNj_8c1bc434;
extern NJS_TEXLIST *var_frontTexlist_8c1bc430;
extern void *var_interiorNj_8c1bc43c;
extern NJS_TEXLIST *var_interiorTexlist_8c1bc438;
extern NJS_TEXLIST *var_markTexlist_8c1bc418;
extern NJS_TEXLIST *var_busStopTexlist_8c1bc424;
extern NjPvmPair *var_routeModelPairs_8c1bc3f4;

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

/* demo-mode fixed model list: model 0x1a, then terminator */
char init_8c043fd4[] = { 0x1a, -1, 0, 0 };

/* ======================
 * Forward Declarations
 * ======================
 */

/* pvm-ready flag helpers, still asm later in this TU */
void setUknPvmBool_8c014330(void);
void resetUknPvmBool_8c014322(void);

/* selected entry index into var_8c18ad18's array */
extern int var_8c228708;
extern NjPvmPair *var_8c1bc3f0;

/* fog-derived values published alongside var_8c18ad28 */
extern int var_8c226504;
extern int var_8c226508;
extern int var_8c227dd0;

/* active nj/pvm pair id, or -1 when the entry has none */
extern int var_8c226534;

/* set outside demo playback; var_demo is the demo-active flag */
extern int var_8c1bb900;
extern int var_demo_8c1bb8d0;

void FUN_8c021a24(void);
void FUN_8c029ad4(int arg);
void FUN_8c02aa36(void);

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

/* Release every loaded route-model asset: free its texlist and njd, then
 * mark the slot unloaded. */
void FUN_8c013dae(void)
{
    RouteModelAsset *slot;

    for (slot = var_8c1bbddc; slot < &var_8c1bbddc[0x20]; slot++) {
        if (slot->texlist_0x08 != (NJS_TEXLIST *) -1) {
            AsqReleaseAndFreeTexlist_11e3c(slot->texlist_0x08);
            syFree(slot->nj_0x0c);
            slot->texlist_0x08 = (NJS_TEXLIST *) -1;
        }
    }
}

/* Reconcile the second (0x41-slot) asset table with a -1-terminated index
 * list: request pvms for newly-wanted slots, release the ones dropped.
 * Unlike syncRouteModelAssets there is no njd, so no syFree. */
void FUN_8c013df6(char *models)
{
    RouteModelAsset *slot;
    int i;

    for (slot = var_8c1bbfdc; slot < &var_8c1bbfdc[0x41]; slot++) {
        slot->requested_0x00 = 0;
        slot->needsLoad_0x04 = 0;
    }

    while ((i = *models) != -1) {
        var_8c1bbfdc[i].requested_0x00 = 1;
        models++;
        if (var_8c1bbfdc[i].texlist_0x08 == (NJS_TEXLIST *) -1) {
            var_8c1bbfdc[i].needsLoad_0x04 = 1;
        }
    }

    for (i = 0; i < 0x41; i++) {
        if (var_8c1bbfdc[i].needsLoad_0x04 != 0) {
            AsqRequestPvm_11ac0(var_basedir_8c18ad6c, init_8c043fd8[i], &var_8c1bbfdc[i].texlist_0x08, 2, 0);
        } else if (var_8c1bbfdc[i].requested_0x00 == 0 && var_8c1bbfdc[i].texlist_0x08 != (NJS_TEXLIST *) -1) {
            AsqReleaseAndFreeTexlist_11e3c(var_8c1bbfdc[i].texlist_0x08);
            var_8c1bbfdc[i].texlist_0x08 = (NJS_TEXLIST *) -1;
        }
    }
}

/* Release every loaded slot of the second asset table (texlist only). */
void FUN_8c013ee4(void)
{
    RouteModelAsset *slot;

    for (slot = var_8c1bbfdc; slot < &var_8c1bbfdc[0x41]; slot++) {
        if (slot->texlist_0x08 != (NJS_TEXLIST *) -1) {
            AsqReleaseAndFreeTexlist_11e3c(slot->texlist_0x08);
            slot->texlist_0x08 = (NJS_TEXLIST *) -1;
        }
    }
}

/* For the currently-selected entry, free its nj/pvm pairs and, when flagged,
 * hand off to FUN_8c021a24. */
void FUN_8c013f22(void)
{
    UknEntry *entry;

    entry = &var_8c18ad18->entries_0x08[var_8c228708];
    if (entry->ukn_0x28 != 0) {
        AsqFreeNjPvmPairs_120fe(&var_8c1bc3f0);
    }
    if (entry->ukn_0x0c != 0) {
        FUN_8c021a24();
    }
}

/* Bring the currently-selected entry's assets in line: publish its fog params,
 * request its nj/pvm pairs, reconcile both route-model tables, and request its
 * dat files. Demo playback substitutes a fixed model list and skips the tail. */
void FUN_8c013f78(void)
{
    UknEntry *entry;
    int i;

    entry = &var_8c18ad18->entries_0x08[var_8c228708];

    if (entry->ukn_0x24 != 0) {
        var_8c18ad28 = entry->ukn_0x24;
        var_8c226504 = var_8c18ad28->field_0x00 - 1;
        var_8c226508 = var_8c226504 / 2;
        var_8c227dd0 = var_8c18ad28->field_0x04;
    }

    if (entry->ukn_0x28 != 0) {
        var_8c1bc3f0 = AsqRequestNjPvmPairs_12030(var_basedir_8c18ad6c, entry->ukn_0x28, 0x10);
    }

    if (var_8c1bb900 == 0 || var_demo_8c1bb8d0 != 0) {
        if (entry->ukn_0x10 != 0) {
            var_8c18adb0 = entry->ukn_0x10;
            syncRouteModelAssets_8c013c34(entry->ukn_0x10);
        }
    } else {
        if (entry->ukn_0x10 != 0) {
            var_8c18adb0 = entry->ukn_0x10;
        }
        syncRouteModelAssets_8c013c34(init_8c043fd4);
    }

    if (entry->ukn_0x18 != 0) {
        FUN_8c013df6(entry->ukn_0x18);
    }

    FUN_8c029ad4(entry->ukn_0x1c);

    if (entry->ukn_0x0c == 0) {
        var_8c226534 = -1;
    } else {
        var_8c226534 = entry->ukn_0x0c;
        for (i = 0; i < 4; i++) {
            AsqRequestDat_11182(var_8c18ad2c, entry->ukn_0x20[i], &var_8c18adb4[i]);
        }
    }

    if (var_8c1bb900 != 0 && var_demo_8c1bb8d0 == 0) {
        FUN_8c02aa36();
    }
}
