#include <shinobi.h>
#include "011120_asset_queues.h"
#include "014a9c_tasks.h"
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
    Uint16 ukn_0x00;                 /* 0..3 */
    Uint16 ukn_0x02;                 /* id, 0..0x16c */
    Uint16 ukn_0x04;                 /* always 0 */
    Uint16 ukn_0x06;                 /* id, 0..0xa9 */
    void *ukn_0x08;
    void *ukn_0x0c;                  /* gates dat load; copied to var_8c226534 */
    Sint8 *ukn_0x10;                 /* route-model list (syncRouteModelAssets) */
    void *ukn_0x14;
    Sint8 *ukn_0x18;                 /* second-table list (FUN_8c013df6) */
    void *ukn_0x1c;                  /* FUN_8c029ad4 arg */
    char **ukn_0x20;                 /* dat filenames for the request loop */
    s_8c18ad28 *ukn_0x24;            /* fog params */
    NjPvmPairFilenames *ukn_0x28;    /* nj/pvm pairs */
} UknEntry;

/* Static per-route config record; init_8c043ca4[] holds one per route.
 * entries_0x08 is the UknEntry array indexed by var_8c228708.
 * filenames_0x1c holds the nj/dat asset names loaded by loadRouteModels. */
typedef struct {
    int ukn_0x00;              /* -> var_8c18ad1c (area/season select) */
    int ukn_0x04;              /* -> var_8c18ad20 (day/night select) */
    UknEntry *entries_0x08;
    void *ukn_0x0c;
    void *ukn_0x10;
    int ukn_0x14;
    int ukn_0x18;
    char *filenames_0x1c[19];
} Ukn;

/* Current route id plus the 19 asset handles filled by loadRouteModels
 * (ghidra's var_8c1bb86c..8b4). */
typedef struct {
    int routeId_0x00;
    void *slots_0x04[19];
} s_8c1bb868;

/* Placeholder types for the nested table/record tree hung off the UknEntry
 * pointer slots. Field meanings are provisional (recovered from data shape,
 * not yet from the consuming code); names are byte-layout hints only. */
typedef struct { int count; void *ptr; } IntPtr;           /* {count, rec*} lists, {-1,NULL}-terminated */
typedef struct { char *a, *b; int n0, n1; } Rec_ssii;
typedef struct { char *a, *b; int n; float f0, f1; } Rec_ssiff;
typedef struct { char *a, *b; int n; char *c; } Rec_ssis;
typedef struct { char *a, *b; int n; char *c; Uint8 flags[4]; } Rec_ssisb;

/* Sprite-drawing resource group (see 015ab8_title.h). */
typedef struct {
    NJS_TEXLIST *tlist_0x00;
    NJS_TEXANIM *tanim_0x04;
    void *contents_0x08;
} ResourceGroup;

/* ==========================
 * Non-initialized Globals
 * ==========================
 */

/* basedir path buffer, filled at runtime */
char var_basedir_8c18ad6c[0x20];

/* pvr path buffer, sibling to var_basedir_8c18ad6c */
char var_8c18ad4c[0x20];
char var_8c18ad8c[0x20];

Ukn *var_8c18ad18;

/* area/season and day/night selectors, copied from the route record */
int var_8c18ad1c;

/* fog params for the current entry (from UknEntry.ukn_0x24) */
s_8c18ad28 *var_8c18ad28;

/* path buffer for dat requests, parallel to var_basedir_8c18ad6c */
char var_8c18ad2c[0x20];

/* Selects the alternate filename table (init_8c043ecc) when == 2. */
int var_8c18ad20;

void *var_8c18ad24;

/* pvm-ready flag; async asset processing sets it via setUknPvmBool_8c014330 */
int var_8c18adac;

/* -1-terminated list of route-model indices to keep loaded */
Sint8 *var_8c18adb0;

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

/* shared filename strings living in the next TU (referenced from the
 * filenames_0x1c[19] tables); .IMPORTed in the asm. */
extern char init_8c04ce10[];
extern char init_8c04df38[];
extern char init_8c04e988[];

/* current route id + the asset handles loadRouteModels fills */
extern s_8c1bb868 var_8c1bb868;

/* nj/pvm pairs for the loaded route models */
extern void *var_8c1bc3ec;

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

#include "013ae8.data.inc"

/* ======================
 * Forward Declarations
 * ======================
 */

/* pvm-ready flag helpers, defined at the tail of this TU */
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
void FUN_8c029ad4(void *arg);
void FUN_8c02aa36(void);

/* loading-screen sprite group; njSetTexture/njReleaseTexture use its texlist,
 * drawSprite takes the whole group */
extern ResourceGroup var_8c1bc3f8;

/* set while the route-load screen owns the display */
extern int var_8c157a6c;

extern NJS_TEXMEMLIST var_tex_8c157af8;

void drawSprite_8c014f54(ResourceGroup *res, int textureId, float x, float y, float priority);
void dispatchInputTask_8c012970(void);
void FUN_8c01306e(void);
void FUN_8c02175a(void);
void FUN_8c026da4(void *handle);
void FUN_8c028de8(void *handle);
void FUN_8c028dd0(void *handle);
void FUN_8c02caba(void);
void FUN_8c02b170(void);
void FUN_8c021810(void);
void FUN_8c02190a(void);

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
void syncRouteModelAssets_8c013c34(Sint8 *models)
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
void FUN_8c013df6(Sint8 *models)
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
        var_8c226534 = (int)entry->ukn_0x0c;
        for (i = 0; i < 4; i++) {
            AsqRequestDat_11182(var_8c18ad2c, entry->ukn_0x20[i], &var_8c18adb4[i]);
        }
    }

    if (var_8c1bb900 != 0 && var_demo_8c1bb8d0 == 0) {
        FUN_8c02aa36();
    }
}

/* Select the route config record for the current route id, build the asset
 * base/pvr paths for its area+time of day, and request all of its files. */
void loadRouteModels_8c014088(void)
{
    var_8c18ad18 = init_8c043ca4[var_8c1bb868.routeId_0x00];
    var_8c18ad1c = var_8c18ad18->ukn_0x00;
    var_8c18ad20 = var_8c18ad18->ukn_0x04;
    var_8c18ad24 = var_8c18ad18->ukn_0x10;

    if (var_8c18ad1c == 0) {
        if (var_8c18ad20 == 0 || var_8c18ad20 == 1) {
            strcpy(var_basedir_8c18ad6c, "\\SD_COMMON");
            strcpy(var_8c18ad4c, "\\SD_PVR");
        } else if (var_8c18ad20 == 2) {
            strcpy(var_basedir_8c18ad6c, "\\SN_COMMON");
            strcpy(var_8c18ad4c, "\\SN_PVR");
        }
    } else if (var_8c18ad1c == 1) {
        if (var_8c18ad20 == 0 || var_8c18ad20 == 1) {
            strcpy(var_basedir_8c18ad6c, "\\WD_COMMON");
            strcpy(var_8c18ad4c, "\\WD_PVR");
        } else if (var_8c18ad20 == 2) {
            strcpy(var_basedir_8c18ad6c, "\\WN_COMMON");
            strcpy(var_8c18ad4c, "\\WN_PVR");
        }
    } else if (var_8c18ad1c == 2) {
        if (var_8c18ad20 == 0 || var_8c18ad20 == 1) {
            strcpy(var_basedir_8c18ad6c, "\\OD_COMMON");
            strcpy(var_8c18ad4c, "\\OD_PVR");
        } else if (var_8c18ad20 == 2) {
            strcpy(var_basedir_8c18ad6c, "\\ON_COMMON");
            strcpy(var_8c18ad4c, "\\ON_PVR");
        }
    }

    strcpy(var_8c18ad8c, var_basedir_8c18ad6c);
    strcpy(var_8c18ad2c, var_basedir_8c18ad6c);

    AsqRequestNj_11492(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[0], &var_8c1bb868.slots_0x04[0], 0);
    AsqRequestNj_11492(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[1], &var_8c1bb868.slots_0x04[1], 0);
    var_8c1bb868.slots_0x04[2] = var_8c18ad18->filenames_0x1c[2];
    AsqRequestNj_11492(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[3], &var_8c1bb868.slots_0x04[3], 0);
    AsqRequestNj_11492(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[4], &var_8c1bb868.slots_0x04[4], 0);
    AsqRequestNj_11492(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[5], &var_8c1bb868.slots_0x04[5], 0);
    AsqRequestNj_11492(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[6], &var_8c1bb868.slots_0x04[6], 0);
    AsqRequestNj_11492(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[7], &var_8c1bb868.slots_0x04[7], 0);
    AsqRequestDat_11182(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[8], &var_8c1bb868.slots_0x04[8]);
    AsqRequestNj_11492(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[9], &var_8c1bb868.slots_0x04[9], 0);
    AsqRequestNj_11492(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[10], 0, &var_8c1bb868.slots_0x04[10]);
    AsqRequestDat_11182(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[11], &var_8c1bb868.slots_0x04[11]);
    AsqRequestDat_11182(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[12], &var_8c1bb868.slots_0x04[12]);
    AsqRequestDat_11182(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[13], &var_8c1bb868.slots_0x04[13]);
    AsqRequestNj_11492(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[14], &var_8c1bb868.slots_0x04[14], 0);
    AsqRequestNj_11492(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[15], &var_8c1bb868.slots_0x04[15], 0);
    AsqRequestNj_11492(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[16], &var_8c1bb868.slots_0x04[16], 0);
    AsqRequestNj_11492(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[17], &var_8c1bb868.slots_0x04[17], 0);
    AsqRequestNj_11492(var_basedir_8c18ad6c, var_8c18ad18->filenames_0x1c[18], &var_8c1bb868.slots_0x04[18], 0);

    requestVehicleAssets_8c013ae8();
    var_8c1bc3ec = AsqRequestNjPvmPairs_12030(var_basedir_8c18ad6c, init_8c0440dc, 0x10);
}

void resetUknPvmBool_8c014322(void)
{
    var_8c18adac = 0;
}

int getUknPvmBool_8c01432a(void)
{
    return var_8c18adac;
}

void setUknPvmBool_8c014330(void)
{
    var_8c18adac = 1;
}

/* Route-load screen task. Drives asset loading across several frames, then
 * hands the display over to the input task.
 *   0: reset queues, load the route models, start the load pass.
 *   1: once the pass finishes, run post-load setup and start a second pass.
 *   2: wait for the second pass, then advance.
 *   3: idle one frame.
 *   4: tear down and switch to the input task.
 * States 0-2 keep drawing the animated loading sprite each frame. */
void task_load_8c014338(Task *task, void *state)
{
    int frame;

    switch (task->field_0x08) {
    case 0:
        AsqResetQueues_11f6c();
        njSetTexture(var_8c1bc3f8.tlist_0x00);
        njLoadCacheTexture(var_8c1bc3f8.tlist_0x00);
        loadRouteModels_8c014088();
        resetUknPvmBool_8c014322();
        AsqProcessQueues_11fe0(AsqNop_11120, 0, 0, 0, setUknPvmBool_8c014330);
        task->field_0x08++;
        return;

    case 1:
        if (getUknPvmBool_8c01432a() != 0) {
            FUN_8c02175a();
            FUN_8c026da4(var_8c1bb868.slots_0x04[8]);
            FUN_8c028de8(var_8c1bb868.slots_0x04[11]);
            FUN_8c028dd0(var_8c1bb868.slots_0x04[12]);
            FUN_8c02caba();
            FUN_8c02b170();
            AsqResetQueues_11f6c();
            FUN_8c013f78();
            resetUknPvmBool_8c014322();
            AsqProcessQueues_11fe0(AsqNop_11120, FUN_8c021810, FUN_8c02190a, 0, setUknPvmBool_8c014330);
            task->field_0x08++;
        }
        break;

    case 2:
        if (getUknPvmBool_8c01432a() != 0) {
            task->field_0x08++;
            return;
        }
        break;

    case 3:
        task->field_0x08++;
        return;

    case 4:
        freeTask_8c014b66(task);
        AsqFreeQueues_11f7e();
        var_8c157a6c = 0;
        njReleaseTexture(var_8c1bc3f8.tlist_0x00);
        FUN_8c01306e();
        dispatchInputTask_8c012970();
        return;
    }

    /* Loading animation, drawn for states 0-2 (and any unexpected state). */
    drawSprite_8c014f54(&var_8c1bc3f8, 0, 0.0f, 0.0f, -5.0f);
    frame = (int) task->field_0x0c;
    task->field_0x0c = (void *) (frame + 1);
    drawSprite_8c014f54(&var_8c1bc3f8, (frame >> 2) % 6 + 1, 0.0f, 0.0f, -4.0f);
}

/* Kick off the route-load screen: install task_load and prime the asset queues. */
void FUN_8c0144fc(void)
{
    Task *task;
    void *state;

    njSetBackColor(0xff418dff, 0xff418dff, 0xff418dff);
    var_8c157a6c = 1;

    pushTask_8c014ae8(var_tasks_8c1ba3c8, (void *) task_load_8c014338, &task, &state, 0);
    task->field_0x08 = 0;
    task->field_0x0c = 0;

    njGarbageTexture(&var_tex_8c157af8, 0xc00);

    AsqInitQueues_11f36(0x20, 0x800, 0x800, 0x40);
}

/* Second-stage load task: build the interior, then hand off to the input task.
 * Draws the same loading animation as task_load_8c014338 while it works. */
void FUN_8c014550(Task *task, void *state)
{
    int frame;

    switch (task->field_0x08) {
    case 0:
        FUN_8c02b170();
        AsqResetQueues_11f6c();
        FUN_8c013f78();
        resetUknPvmBool_8c014322();
        AsqProcessQueues_11fe0(AsqNop_11120, FUN_8c021810, FUN_8c02190a, 0, setUknPvmBool_8c014330);
        task->field_0x08++;
        break;
    case 1:
        if (getUknPvmBool_8c01432a() != 0) {
            task->field_0x08++;
            return;
        }
        break;
    case 2:
        task->field_0x08++;
        return;
    case 3:
        freeTask_8c014b66(task);
        AsqFreeQueues_11f7e();
        var_8c157a6c = 0;
        njReleaseTexture(var_8c1bc3f8.tlist_0x00);
        njSetTexture(var_interiorTexlist_8c1bc438);
        njLoadCacheTexture(var_interiorTexlist_8c1bc438);
        FUN_8c01306e();
        dispatchInputTask_8c012970();
        return;
    }

    /* Loading animation, drawn for states 0-1 (and any unexpected state). */
    drawSprite_8c014f54(&var_8c1bc3f8, 0, 0.0f, 0.0f, -5.0f);
    frame = (int) task->field_0x0c;
    task->field_0x0c = (void *) (frame + 1);
    drawSprite_8c014f54(&var_8c1bc3f8, (frame >> 2) % 6 + 1, 0.0f, 0.0f, -4.0f);
}
