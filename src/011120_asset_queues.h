/* 8c011120 - Asset Queues */
#ifndef _011120_ASSET_QUEUES_H
#define _011120_ASSET_QUEUES_H

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
} ModelFiles;

// SHC doesn't like "Model"
typedef struct {
    NJS_TEXLIST *texlist;
    void *njDest;
} LoadedModel;

typedef struct {
    unsigned int physical_0x00;
    unsigned int logical_0x04;
} ButtonRemap;

/* =======================
 * Non-initialized Globals
 * =======================
 */

extern int var_queuesAreInitialized_8c157a60;
extern int var_seed_8c157a64;
extern int var_loadScreenActive_8c157a6c;

/* PDS_PERIPHERAL.support of the pad on port 0, masked to BT_CONTROLLER, or -1
 * when nothing usable is plugged in (see 012324_input.h). */
extern int var_activeCtrlType_8c157a70;
extern Task *var_pushedTask_8c157a74;
extern int var_resetRequested_8c157a78;
extern int var_vmBusy_8c157a7c;

extern ButtonRemap init_btnRemapManual_8c03be80[7];
extern ButtonRemap init_btnRemapAuto_8c03beb8[7];
extern ButtonRemap init_btnRemapWheelManual_8c03bef0[5];
extern ButtonRemap init_btnRemapWheelAuto_8c03bf18[5];

/* =========
 * Functions
 * =========
 */

void AsqNop_8c011120();

int AsqRequestDat_8c011182(char* basedir, char* filename, void* dest);

int AsqRequestNj_8c011492(char* basedir, char* filename, void* dest, void* dest2);

int AsqRequestTexlist_8c01181c(char *basedir, NJS_TEXLIST *texlist);

int AsqRequestPvm_8c011ac0(char *basedir, char *filename, void *texlist, int count, int attr);

void AsqReleaseAndFreeTexlist_8c011e3c(NJS_TEXLIST *texlist);


void AsqInitQueues_8c011f36(int datCount,int njCount,int texlistCount,int pvmCount);

void AsqResetQueues_8c011f6c();

void AsqFreeQueues_8c011f7e();

void AsqProcessQueues_8c011fe0(void *tickCallback, void *afterDatCallback, void *afterNjCallback, void *afterPvmCallback, void *afterTexlistCallback);

LoadedModel* AsqRequestModels_8c012030(char *basedir, ModelFiles *pairs, int texlistCount);

void AsqFreeModels_8c0120fe(LoadedModel **pairsPtr);

void AsqSetSeedA_8c012160(int p1);

int AsqGetRandomA_8c012166();

int AsqGetRandomInRangeA_8c012178(unsigned int p1);

void AsqSetSeedB_8c0121a2(int p1);

int AsqGetRandomB_8c0121a8();

int AsqGetRandomInRangeB_8c0121be(unsigned int p1);

void AsqApplyButtonConfig_8c0121e8();

#endif // _011120_ASSET_QUEUES_H
