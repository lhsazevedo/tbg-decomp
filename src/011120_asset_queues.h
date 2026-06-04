/* 8c011120 */

#include <shinobi.h>
#include <string.h>
#include "serial_debug.h"
#include "014a9c_tasks.h"
#include "stdio.h"


/* =================
 * Type Declarations
 * =================
 */

typedef struct {
    char *njFilename;
    char *pvmFilename;
} NjPvmPairFilenames;

typedef struct {
    NJS_TEXLIST *texlist;
    void *njDest;
} NjPvmPair;

typedef struct {
    unsigned int physical_0x00;
    unsigned int logical_0x04;
} ButtonRemap;

/* =====================
 * External Declarations
   =====================
 */

extern Task var_tasks_8c1ba3c8[];
/* TODO: DRY */
extern Sint8 var_texbuf_8c277ca0[];


/* =======================
 * Non-initialized Globals
 * =======================
 */

extern int var_queuesAreInitialized_8c157a60;
extern int var_seed_8c157a64;
extern int var_8c157a6c;

/* TODO: Confirm type */
extern int var_activeCtrlType_8c157a70;
extern int var_8c157a74;
extern int var_resetRequested_8c157a78;
extern int var_8c157a7c;

extern ButtonRemap init_btnRemap_8c03be80[7];
extern ButtonRemap init_btnRemapAlt_8c03beb8[7];
extern ButtonRemap init_btnRemapWheel_8c03bef0[5];
extern ButtonRemap init_btnRemapWheelAlt_8c03bf18[5];

/* =========
 * Functions
   =========
 */

void AsqNop_11120();

int AsqRequestDat_11182(char* basedir, char* filename, void* dest);

int AsqRequestNj_11492(char* basedir, char* filename, void* dest, void* dest2);

int AsqRequestTexlist_1181c(char *basedir, NJS_TEXLIST *texlist);

int AsqRequestPvm_11ac0(char *basedir, char *filename, void *texlist, int count, int attr);

void AsqReleaseAndFreeTexlist_11e3c(NJS_TEXLIST *texlist);

void AsqFreeTexlist_11e60(NJS_TEXLIST *texlist);

void AsqInitQueues_11f36(int datCount,int njCount,int texlistCount,int pvmCount);

void AsqResetQueues_11f6c();

void AsqFreeQueues_11f7e();

void AsqProcessQueues_11fe0(void *func, void *afterDatCallback, void *afterNjCallback, void *afterPvmCallback, void *afterTexlistCallback);

NjPvmPair* AsqRequestNjPvmPairs_12030(char *basedir, NjPvmPairFilenames *pairs, int texlistCount);

void AsqFreeNjPvmPairs_120fe(NjPvmPair **pairsPtr);

void AsqSetSeedA_12160(int p1);

int AsqGetRandomA_12166();

int AsqGetRandomInRangeA_12178(unsigned int p1);

void AsqSetSeedB_121a2(int p1);

int AsqGetRandomB_121a8();

int AsqGetRandomInRangeB_121be(unsigned int p1);

void AsqApplyButtonConfig_121e8();
