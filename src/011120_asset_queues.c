/* @unit Asq */

#include <shinobi.h>
#include <string.h>
#include "011120_asset_queues.h"
#include "01bb48_vm_game.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "014a9c_tasks.h"
#include "235ca0_nj_buffers.h"
#include "1ba1c8_globals.h"
#include "stdio.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

/* taskProcessQueues_8c011e80 walks the four queues in this order. */
#define QUEUE_DAT     0
#define QUEUE_NJ      1
#define QUEUE_PVM     2
#define QUEUE_TEXLIST 3

/* Bytes per name in njSetPvmTextureList's filename block. */
#define PVM_NAME_LEN 28

/* Above this many sectors the file gets its own buffer instead of
   var_texbuf_8c277ca0. */
#define BIG_FILE_SECTORS 0x100


/* =================
 * Type Declarations
 * =================
 */

typedef struct {
    char *basedir;
    char *filename;
    void **dest;
    int loaded_0x0c;
} QueuedDat;

typedef struct {
    char *basedir;
    char *filename;
    void **dest_0x08;
    void **dest_0x0c;
    int loaded_0x10;
} QueuedNj;

/* This and the TaskLoadQueued* structs below are Task (014a9c_tasks.h) with
   its generic fields named for one loader: field_0x08 is the phase,
   field_0x0c the open GDFS handle, queuedItem_0x18 the queue cursor.
   TaskSpawn_8c014ae8 only ever hands out a plain Task. */
typedef struct {
    TaskAction action;
    void *state;
    int phase_0x08;
    GDFS gdfs_0x0c;
    int field_0x10;
    int field_0x14;
    QueuedNj* queuedNj_0x18;
    int field_0x1c;
} TaskLoadQueuedNjs;

typedef struct {
    char *basedir;
    char *filename;
    void **texlist_0x08;
    int count_0x0c;
    int attr_0x10;
    int loaded_0x14;
} QueuedPvm;

typedef struct {
    TaskAction action;
    void *state;
    int phase_0x08;
    GDFS gdfs_0x0c;
    int field_0x10;
    int field_0x14;
    QueuedPvm* queuedPvm_0x18;
    int field_0x1c;
} TaskLoadQueuedPvms;

typedef struct {
    char *basedir_0x00;
    NJS_TEXLIST *texlist_0x04;
} QueuedTexlist;

typedef struct {
    int queue_0x00;
    void (*tickCallback_0x04)();
    void (*afterDatCallback_0x08)();
    void (*afterNjCallback_0x0c)();
    void (*afterPvmCallback_0x10)();
    void (*afterTexlistCallback_0x14)();
} TaskProcessQueuesState;

typedef struct {
    TaskAction action;
    void *state;
    int phase_0x08;
    GDFS gdfs_0x0c;
    int field_0x10;
    int field_0x14;
    QueuedDat* queuedDat_0x18;
    int field_0x1c;
} TaskLoadQueuedDats;

/* =======================
 * Non-initialized Globals
 * =======================
 */

int var_queuesAreInitialized_8c157a60;
int var_seed_8c157a64;
STATIC int var_texlistQueueDoneCount_8c157a68;
int var_loadScreenActive_8c157a6c;

/* PDS_PERIPHERAL.support of port 0 masked with BT_CONTROLLER: either
   BT_CONTROLLER or BT_RACING (012324_input.h), or -1 for no usable pad. */
int var_activeCtrlType_8c157a70;
/* TaskSpawn_8c014ae8 needs somewhere to report the task it made;
   InputSpawnTask_8c0128cc gives it this, and nothing ever reads it back. */
Task *var_pushedTask_8c157a74;
int var_resetRequested_8c157a78;
int var_vmBusy_8c157a7c;

STATIC char *var_queueBaseDir_8c157a80;
STATIC Sint8 *var_queueBuffer_8c157a84;
STATIC int var_loadRetryNeeded_8c157a88;

STATIC QueuedDat *var_datQueue_8c157a8c;
STATIC QueuedDat *var_datQueueRear_8c157a90;
STATIC QueuedDat *var_datQueueTail_8c157a94;
STATIC int var_datQueueIsIdle_8c157a98;

STATIC QueuedNj *var_njQueue_8c157a9c;
STATIC QueuedNj *var_njQueueRear_8c157aa0;
STATIC QueuedNj *var_njQueueTail_8c157aa4;
STATIC int var_njQueueIsIdle_8c157aa8;

STATIC QueuedTexlist *var_texlistQueue_8c157aac;
STATIC QueuedTexlist *var_texlistQueueRear_8c157ab0;
STATIC QueuedTexlist *var_texlistQueueTail_8c157ab4;
STATIC int var_texlistQueueIsIdle_8c157ab8;

STATIC QueuedPvm* var_pvmQueue_8c157abc;
STATIC QueuedPvm* var_pvmQueueRear_8c157ac0;
STATIC QueuedPvm* var_pvmQueueTail_8c157ac4;
STATIC int var_pvmQueueIsIdle_8c157ac8;

STATIC int var_seed_8c157acc;
STATIC int var_seed_8c157ad0;

/* ===================
 * Initialized Globals
 * ===================
 */

/* The base dir a queue carries when it holds nothing to load. */
STATIC char *init_dataEmpty_8c03be7c = "DATA EMPTY";

ButtonRemap init_btnRemapManual_8c03be80[7] = {
    /* physical, logical */
    { 0, PDD_DGT_TX },
    { 0, PDD_DGT_TB },
    { 0, PDD_DGT_TY },
    { 0, PDD_DGT_TA },
    { 0, PDD_DGT_KU },
    { 0, PDD_DGT_KD },
    { 0, PDD_DGT_ST },
};

ButtonRemap init_btnRemapAuto_8c03beb8[7] = {
    { 0, PDD_DGT_TX },
    { 0, PDD_DGT_TB },
    { 0, PDD_DGT_TY },
    { 0, PDD_DGT_TA },
    { 0, PDD_DGT_TX },
    { 0, PDD_DGT_TB },
    { 0, PDD_DGT_ST },
};

ButtonRemap init_btnRemapWheelManual_8c03bef0[5] = {
    { 0, PDD_DGT_TX },
    { 0, PDD_DGT_TB },
    { 0, PDD_DGT_TY },
    { 0, PDD_DGT_TA },
    { 0, PDD_DGT_ST },
};

ButtonRemap init_btnRemapWheelAuto_8c03bf18[5] = {
    { 0, PDD_DGT_TX },
    { 0, PDD_DGT_TB },
    { 0, PDD_DGT_TY },
    { 0, PDD_DGT_TA },
    { 0, PDD_DGT_ST },
};


/* =========
 * Functions
 * =========
 */

void AsqNop_8c011120() {
    /* Empty body */
}

STATIC int initDatQueue_8c011124(int n) {
    if (n != 0) {
        if ((var_datQueue_8c157a8c = syMalloc(n * sizeof(QueuedDat))) == NULL) {
            return 0;
        }

        var_datQueueTail_8c157a94 = var_datQueue_8c157a8c + n;
    } else {
        var_datQueue_8c157a8c = var_datQueueTail_8c157a94 = ((void *) -1);
    }

    return 1;
}

STATIC void resetDatQueue_8c01116a() {
    var_datQueueRear_8c157a90 = var_datQueue_8c157a8c;
    var_queueBaseDir_8c157a80 = init_dataEmpty_8c03be7c;
    var_datQueueIsIdle_8c157a98 = 1;
}

int AsqRequestDat_8c011182(char* basedir, char* filename, void* dest) {

    if (*filename == 0) {
        return 0;
    }

    if (var_datQueueRear_8c157a90 >= var_datQueueTail_8c157a94) {
        LOG_WARN(("[ASSET_QUEUES] DAT queue is full. Cannot enqueue \"%s\" (basedir \"%s\")\n", filename, basedir));
        return 0;
    }

    LOG_TRACE(("[ASSET_QUEUES] DAT enqueued: \"%s\" (basedir \"%s\")\n", filename, basedir));

    var_datQueueRear_8c157a90->basedir = basedir;
    var_datQueueRear_8c157a90->filename = filename;
    var_datQueueRear_8c157a90->dest = dest;
    var_datQueueRear_8c157a90->loaded_0x0c = 0;

    var_datQueueRear_8c157a90++;
    return 1;
}

STATIC void taskLoadQueuedDats_8c0111b4(TaskLoadQueuedDats* task, void* state) {
    QueuedDat* item = task->queuedDat_0x18;
    Sint32 size;

    switch (task->phase_0x08) {
        case 0: {
            while (1) {
                if (item >= var_datQueueRear_8c157a90) {
                    break;
                }

                if (item->loaded_0x0c == 0) {
                    if (*item->basedir != 0 &&
                        strcmp(var_queueBaseDir_8c157a80, item->basedir) != 0
                    ) {
                        var_queueBaseDir_8c157a80 = item->basedir;
                        gdFsChangeDir(item->basedir);
                    }

                    task->gdfs_0x0c = gdFsOpen(item->filename, 0);
                    if (task->gdfs_0x0c == NULL) {
                        var_loadRetryNeeded_8c157a88 = 1;
                        task->queuedDat_0x18 = item + 1;
                        task->phase_0x08 = 0;
                        return;
                    }

                    if (!gdFsGetFileSctSize(task->gdfs_0x0c, &size)) {
                        var_loadRetryNeeded_8c157a88 = 1;
                        task->queuedDat_0x18 = item + 1;
                        task->phase_0x08 = 0;
                        return;
                    }

                    *item->dest = syMalloc(size * 2048);

                    if (gdFsRead(task->gdfs_0x0c, size, *item->dest) != GDD_ERR_OK) {
                        var_loadRetryNeeded_8c157a88 = 1;
                        task->queuedDat_0x18 = item + 1;
                        task->phase_0x08 = 0;
                        return;
                    }

                    gdFsClose(task->gdfs_0x0c);
                    item->loaded_0x0c = 1;
                    task->queuedDat_0x18 = item + 1;
                    task->phase_0x08 = 0;
                    return;
                }

                item++;
            }

            if (var_loadRetryNeeded_8c157a88 != 0) {
                task->queuedDat_0x18 = var_datQueue_8c157a8c;
                var_loadRetryNeeded_8c157a88 = 0;
                var_queueBaseDir_8c157a80 = init_dataEmpty_8c03be7c;
                /* return */;
            } else {
                var_datQueueIsIdle_8c157a98 = 1;
                TaskKill_8c014b66((Task*) task);
                /* return; */
            }
            break;
        }

        case 1: {
            switch (gdFsGetStat(task->gdfs_0x0c)) {
                case GDD_STAT_COMPLETE: {
                    gdFsClose(task->gdfs_0x0c);
                    item->loaded_0x0c = 1;
                    task->queuedDat_0x18 = item + 1;
                    task->phase_0x08 = 0;
                    return;
                } 
                case GDD_STAT_READ: {
                    if (gdFsGetTransStat(task->gdfs_0x0c) == GDD_FS_TRANS_READY) {
                        gdFsTrans32(task->gdfs_0x0c, 2048, *item->dest);
                    }
                    break;
                } 
                default: {
                    gdFsClose(task->gdfs_0x0c);
                    syFree(*item->dest);
                    var_loadRetryNeeded_8c157a88 = 1;
                    task->queuedDat_0x18 = item + 1;
                    task->phase_0x08 = 0;
                    break;
                }
            }
            break;
        }
    }
}

STATIC int sortAndLoadDatQueue_8c011310() {
    Task *created_task;
    void *created_state;
    QueuedDat *temp;

    if ((int) var_datQueue_8c157a8c == (int) var_datQueueRear_8c157a90) {
        return 0;
    }

    var_datQueueIsIdle_8c157a98 = 0;

    temp = syMalloc((int) var_datQueueRear_8c157a90 - (int) var_datQueue_8c157a8c);

    while (1) {
        int swapped = 0;
        QueuedDat *a = var_datQueue_8c157a8c;
        QueuedDat *b = var_datQueue_8c157a8c;

        while (++b < var_datQueueRear_8c157a90) {
            if (strcmp(a->filename, b->filename) > 0) {
                *temp = *a;
                *a = *b;
                *b = *temp;
                swapped = 1;
            }

            a++;
        }

        if (!swapped) {
            break;
        }
    }

    syFree(temp);

    if (!TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, &taskLoadQueuedDats_8c0111b4, &created_task, &created_state, 0)) {
        return 0;
    }

    created_task->queuedItem_0x18 = var_datQueue_8c157a8c;
    created_task->field_0x08 = 0;
    var_loadRetryNeeded_8c157a88 = 0;
    var_queueBaseDir_8c157a80 = init_dataEmpty_8c03be7c;

    return 1;
}

STATIC int datQueueIsIdle_8c0113d2() {
    return var_datQueueIsIdle_8c157a98;
}

STATIC void freeDatQueue_8c0113d8() {
    if (var_datQueue_8c157a8c != (QueuedDat*) -1) {
        syFree((void*) var_datQueue_8c157a8c);
    }
}

STATIC int initNjQueue_8c011430(int param) {
    if (param != 0) {
        if ((var_njQueue_8c157a9c = syMalloc(param * sizeof(QueuedNj))) == NULL) {
            return 0;
        }

        var_njQueueTail_8c157aa4 = var_njQueue_8c157a9c + param;
    } else {
        var_njQueue_8c157a9c = var_njQueueTail_8c157aa4 = ((void *) -1);
    }

    return 1;
}

STATIC void resetNjQueue_8c01147a() {
    var_njQueueRear_8c157aa0 = var_njQueue_8c157a9c;
    var_queueBaseDir_8c157a80 = init_dataEmpty_8c03be7c;
    var_njQueueIsIdle_8c157aa8 = 1;
}

int AsqRequestNj_8c011492(char* basedir, char* filename, void* dest, void* dest2) {

    if (*filename == 0) {
        return 0;
    }

    if (var_njQueueRear_8c157aa0 >= var_njQueueTail_8c157aa4) {
        return 0;
    }

    LOG_TRACE(("[ASSET_QUEUES] NJ enqueued: \"%s\" (basedir \"%s\")\n", filename, basedir));

    var_njQueueRear_8c157aa0->basedir = basedir;
    var_njQueueRear_8c157aa0->filename = filename;
    var_njQueueRear_8c157aa0->dest_0x08 = dest;
    var_njQueueRear_8c157aa0->dest_0x0c = dest2;
    var_njQueueRear_8c157aa0->loaded_0x10 = 0;

    var_njQueueRear_8c157aa0++;
    return 1;
}

STATIC void taskLoadQueuedNjs_8c0114cc(TaskLoadQueuedNjs* task, void* state) {
    QueuedNj* qnj = task->queuedNj_0x18;
    Sint32 size;
    Uint32 fpos = 0, rtype;

    switch (task->phase_0x08)
    {
        case 0:
            while (1)
            {
                if (qnj >= var_njQueueRear_8c157aa0) {
                    break;
                }

                if (qnj->loaded_0x10 == 0) {
                    if (
                        *qnj->basedir != 0 &&
                        strcmp(var_queueBaseDir_8c157a80, qnj->basedir) != 0
                    ) {
                        var_queueBaseDir_8c157a80 = qnj->basedir;
                        gdFsChangeDir(qnj->basedir);
                    }

                    task->gdfs_0x0c = gdFsOpen(qnj->filename, 0);

                    if (task->gdfs_0x0c == NULL) {
                        if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                            syFree(var_queueBuffer_8c157a84);
                        }
                        var_loadRetryNeeded_8c157a88 = 1;
                        task->queuedNj_0x18++;
                        task->phase_0x08 = 0;
                        return;
                    }

                    if (!gdFsGetFileSctSize(task->gdfs_0x0c, &size)) {
                        if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                            syFree(var_queueBuffer_8c157a84);
                        }
                        var_loadRetryNeeded_8c157a88 = 1;
                        task->queuedNj_0x18++;
                        task->phase_0x08 = 0;
                        return;
                    }

                    if (size > BIG_FILE_SECTORS) {
                        var_queueBuffer_8c157a84 = syMalloc(size * 2048);
                    } else {
                        var_queueBuffer_8c157a84 = var_texbuf_8c277ca0;
                    }

                    if (gdFsRead(task->gdfs_0x0c, size, var_queueBuffer_8c157a84) != GDD_ERR_OK) {
                        if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                            syFree(var_queueBuffer_8c157a84);
                        }
                        var_loadRetryNeeded_8c157a88 = 1;
                        task->queuedNj_0x18++;
                        task->phase_0x08 = 0;
                        return; 
                    }

                    gdFsClose(task->gdfs_0x0c);
                    qnj->loaded_0x10 = 1;

                    if (qnj->dest_0x08 != 0) {
                        *qnj->dest_0x08 = njReadBinary(var_queueBuffer_8c157a84, &fpos, &rtype);
                    }

                    if (qnj->dest_0x0c != 0) {
                        *qnj->dest_0x0c = njReadBinary(var_queueBuffer_8c157a84, &fpos, &rtype);
                    }

                    if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                        syFree(var_queueBuffer_8c157a84);
                    }
                    task->queuedNj_0x18 = ++qnj;
                    task->phase_0x08 = 0;
                    return;
                }

                qnj++;
            }

            if (var_loadRetryNeeded_8c157a88 != 0) {
                task->queuedNj_0x18 = var_njQueue_8c157a9c;
                var_loadRetryNeeded_8c157a88 = 0;
                var_queueBaseDir_8c157a80 = init_dataEmpty_8c03be7c;
            } else {
                var_njQueueIsIdle_8c157aa8 = 1;
                TaskKill_8c014b66((Task*) task);
            }

            break;

        case 1:
            switch (gdFsGetStat(task->gdfs_0x0c)) {
                case GDD_STAT_COMPLETE: {
                    gdFsClose(task->gdfs_0x0c);
                    qnj->loaded_0x10 = 1;

                    if (qnj->dest_0x08) {
                        *qnj->dest_0x08 = njReadBinary(var_queueBuffer_8c157a84, &fpos, &rtype);
                    }

                    if (qnj->dest_0x0c) {
                        *qnj->dest_0x0c = njReadBinary(var_queueBuffer_8c157a84, &fpos, &rtype);
                    }

                    if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                        syFree(var_queueBuffer_8c157a84);
                    }

                    task->queuedNj_0x18 = ++qnj;
                    task->phase_0x08 = 0;

                    break;
                }
                case GDD_STAT_READ: {
                    if (gdFsGetTransStat(task->gdfs_0x0c) != GDD_FS_TRANS_READY) {
                        return;
                    }

                    gdFsTrans32(task->gdfs_0x0c, 2048, var_queueBuffer_8c157a84);
                    break;
                }
                default: {
                    gdFsClose(task->gdfs_0x0c);

                    /* Freed twice in the original. */
                    if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                        syFree(var_queueBuffer_8c157a84);
                    }

                    if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                        syFree(var_queueBuffer_8c157a84);
                    }

                    var_loadRetryNeeded_8c157a88 = 1;
                    task->queuedNj_0x18 = ++qnj;
                    task->phase_0x08 = 0;
                    break;
                }
            }
            break;
        }
}

STATIC int sortAndLoadNjQueue_8c0116b6() {
    Task *created_task;
    void* created_state;
    QueuedNj *temp;

    if ((int) var_njQueue_8c157a9c == (int) var_njQueueRear_8c157aa0) {
        return 0;
    }

    var_njQueueIsIdle_8c157aa8 = 0;

    temp = syMalloc((int) var_njQueueRear_8c157aa0 - (int) var_njQueue_8c157a9c);

    while (1) {
        int swapped = 0;
        QueuedNj *a = var_njQueue_8c157a9c;
        QueuedNj *b = var_njQueue_8c157a9c;

        while (++b < var_njQueueRear_8c157aa0) {
            if (strcmp(a->filename, b->filename) > 0) {
                *temp = *a;
                *a = *b;
                *b = *temp;
                swapped = 1;
            }

            a++;
        }

        if (!swapped) {
            break;
        }
    }

    syFree(temp);

    if (!TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, &taskLoadQueuedNjs_8c0114cc, &created_task, &created_state, 0)) {
        return 0;
    }

    created_task->queuedItem_0x18 = var_njQueue_8c157a9c;
    created_task->field_0x08 = 0;
    var_loadRetryNeeded_8c157a88 = 0;
    var_queueBaseDir_8c157a80 = init_dataEmpty_8c03be7c;

    return 1;
}

STATIC int njQueueIsIdle_8c01179e() {
    return var_njQueueIsIdle_8c157aa8;
}

STATIC void freeNjQueue_8c0117a4() {
    if (var_njQueue_8c157a9c != (QueuedNj*) -1) {
        syFree(var_njQueue_8c157a9c);
    }
}

STATIC int initTexlistQueue_8c0117b8(int n) {
  if (n != 0) {
    var_texlistQueue_8c157aac = syMalloc(n * sizeof(QueuedTexlist));
    if (var_texlistQueue_8c157aac == NULL) {
      return 0;
    }
    var_texlistQueueTail_8c157ab4 = (void *) ((char*) var_texlistQueue_8c157aac + n * sizeof(QueuedTexlist));
  } else {
    var_texlistQueueTail_8c157ab4 = (void *) -1;
    var_texlistQueue_8c157aac = (void *) -1;
  }
  return 1;
}

STATIC void resetTexlistQueue_8c0117fe() {
    var_texlistQueueRear_8c157ab0 = var_texlistQueue_8c157aac;
    var_queueBaseDir_8c157a80 = init_dataEmpty_8c03be7c;
    var_texlistQueueDoneCount_8c157a68 = 0;
    var_texlistQueueIsIdle_8c157ab8 = 1;
    return;
}

int AsqRequestTexlist_8c01181c(char *basedir, NJS_TEXLIST *texlist) {
    if (var_texlistQueueRear_8c157ab0 >= var_texlistQueueTail_8c157ab4) {
        return 0;
    }

    LOG_TRACE(("[ASSET_QUEUES] Texlist enqueued: %p (basedir \"%s\")\n", texlist, basedir));

    var_texlistQueueRear_8c157ab0->basedir_0x00 = basedir;
    var_texlistQueueRear_8c157ab0->texlist_0x04 = texlist;
    var_texlistQueueRear_8c157ab0++;
    return 1;
}

STATIC void taskLoadQueuedTexlists_8c01183e(Task *task, void *state) {
    QueuedTexlist *item = task->queuedItem_0x18;
    NJS_TEXLIST *texlist;

    while (TRUE) {
        int i;
        /* Assumes the current texlist is already loaded. */
        Bool alreadyLoaded = TRUE;

        texlist = item->texlist_0x04;

        for (i = 0; i < texlist->nbTexture; i++)
        {
            int queueIdx;
            NJS_TEXNAME *currentTexture = &texlist->textures[i];

            for (queueIdx = 0; queueIdx < var_texlistQueueDoneCount_8c157a68; queueIdx++)
            {
                QueuedTexlist *comparedItem = &var_texlistQueue_8c157aac[queueIdx];
                int comparedIndex;
                int comparedTextureCount;
                NJS_TEXNAME *comparedTextures;

                if (!comparedItem->texlist_0x04) {
                    continue;
                }

                comparedTextureCount = comparedItem->texlist_0x04->nbTexture;
                comparedTextures = comparedItem->texlist_0x04->textures;

                /* comparedIndex is 0 when there's nothing to compare against;
                 * the "not found" check below still works either way. */
                for (comparedIndex = 0; comparedIndex < comparedTextureCount; comparedIndex++)
                {
                    if (!strcmp(currentTexture->filename, comparedTextures[comparedIndex].filename)) {
                        currentTexture->texaddr = comparedTextures[comparedIndex].texaddr;
                        break;
                    }
                }

                /* If the current texture is already loaded, advance to the
                 * next one in the current texlist; breaking skips the loop
                 * counter increment. */
                if (comparedIndex != comparedTextureCount) break;

            }

            /* If the current texture wasn't found in any compared texlist,
             * it needs to be loaded. */
            if (queueIdx == var_texlistQueueDoneCount_8c157a68) {
                alreadyLoaded = FALSE;
            }

        }

        if (alreadyLoaded) {
            item->texlist_0x04 = NULL;
        } else {
            if (*item->basedir_0x00 && strcmp(var_queueBaseDir_8c157a80, item->basedir_0x00)) {
                var_queueBaseDir_8c157a80 = item->basedir_0x00;
                gdFsChangeDir(item->basedir_0x00);
            }

            njSetTexture(item->texlist_0x04);

            if (texlist->nbTexture) {
                int i;
                for (i = 0; i < texlist->nbTexture; i++) {
                    if (!texlist->textures[i].texaddr) {
                        njLoadTextureNum(i);
                    }
                }
            }
        }

        item++;
        var_texlistQueueDoneCount_8c157a68++;
        if (item >= var_texlistQueueRear_8c157ab0) {
            var_texlistQueueIsIdle_8c157ab8 = 1;
            TaskKill_8c014b66(task);
            return;
        }

        if (!alreadyLoaded) {
            task->queuedItem_0x18 = item;
            return;
        }
    }
}

STATIC int loadTexlistQueue_8c0119f8() {
    Task *created_task;
    void *created_state;

    if (var_texlistQueue_8c157aac == var_texlistQueueRear_8c157ab0) {
        return 0;
    }

    var_texlistQueueIsIdle_8c157ab8 = 0;
    if (!TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, taskLoadQueuedTexlists_8c01183e, &created_task, &created_state, 0)) {
        return 0;
    }

    created_task->queuedItem_0x18 = var_texlistQueue_8c157aac;
    return 1;
}

STATIC int texlistQueueIsIdle_8c011a42() {
  return var_texlistQueueIsIdle_8c157ab8;
}

STATIC void freeTexlistQueue_8c011a48() {
    if (var_texlistQueue_8c157aac != (void *) -1) {
        syFree(var_texlistQueue_8c157aac);
    }
}

STATIC int initPvmQueue_8c011a5c(int count) {
    if (count) {
        var_pvmQueue_8c157abc = syMalloc(count * sizeof(QueuedPvm));
        if (var_pvmQueue_8c157abc == NULL) {
            return 0;
        }

        var_pvmQueueTail_8c157ac4 = var_pvmQueue_8c157abc + count;
    } else {
        var_pvmQueueTail_8c157ac4 = (void*) -1;
        var_pvmQueue_8c157abc = (void*) -1;
    }

    return 1;
}

int AsqRequestPvm_8c011ac0(char *basedir, char *filename, void *texlist, int count, int attr) {
    if (!*filename || var_pvmQueueRear_8c157ac0 >= var_pvmQueueTail_8c157ac4) {
        return 0;
    }

    LOG_TRACE(("[ASSET_QUEUES] PVM enqueued: %s (basedir %s)\n", filename, basedir));

    var_pvmQueueRear_8c157ac0->basedir = basedir;
    var_pvmQueueRear_8c157ac0->filename = filename;
    var_pvmQueueRear_8c157ac0->texlist_0x08 = texlist;
    var_pvmQueueRear_8c157ac0->count_0x0c = count;
    var_pvmQueueRear_8c157ac0->attr_0x10 = attr;
    var_pvmQueueRear_8c157ac0->loaded_0x14 = 0;

    var_pvmQueueRear_8c157ac0++;

    return 1;
}

STATIC void taskLoadQueuedPvms_8c011b00(TaskLoadQueuedPvms* task, void* state) {
    QueuedPvm *pvm = (QueuedPvm*) task->queuedPvm_0x18;
    Sint32 size;

    switch (task->phase_0x08) {
        case 0: {
            for (; pvm < var_pvmQueueRear_8c157ac0; pvm++)
            {
                if (pvm->loaded_0x14 == 0) {
                    int i;
                    int *temp;
                    char *filename;
                    NJS_TEXLIST *texlist;
                    NJS_TEXNAME *texname;

                    if (*pvm->basedir && strcmp(var_queueBaseDir_8c157a80, pvm->basedir)) {
                        var_queueBaseDir_8c157a80 = pvm->basedir;
                        gdFsChangeDir(pvm->basedir);
                    }

                    task->gdfs_0x0c = gdFsOpen(pvm->filename, 0);
                    if (!task->gdfs_0x0c) {
                        var_loadRetryNeeded_8c157a88 = 1;
                        task->queuedPvm_0x18 = ++pvm;
                        task->phase_0x08 = 0;
                        return;
                    }

                    if (!gdFsGetFileSctSize(task->gdfs_0x0c, &size)) {
                        var_loadRetryNeeded_8c157a88 = 1;
                        task->queuedPvm_0x18 = ++pvm;
                        task->phase_0x08 = 0;
                        return;
                    }

                    if (size > BIG_FILE_SECTORS) {
                        var_queueBuffer_8c157a84 = syMalloc(size * 2048);
                    } else {
                        var_queueBuffer_8c157a84 = var_texbuf_8c277ca0;
                    }

                    if (gdFsRead(task->gdfs_0x0c, size, var_queueBuffer_8c157a84)) {
                        var_loadRetryNeeded_8c157a88 = 1;
                        task->queuedPvm_0x18 = ++pvm;
                        task->phase_0x08 = 0;
                        return;
                    }

                    gdFsClose(task->gdfs_0x0c);
                    pvm->loaded_0x14 = 1;

                    *pvm->texlist_0x08 = texlist = syMalloc(sizeof(NJS_TEXLIST));

                    texname = syMalloc(pvm->count_0x0c * 0xc);
                    for (i = 0; i < pvm->count_0x0c; i++) {
                        texname[i].attr = pvm->attr_0x10;
                    }

                    /* njSetPvmTextureList wants a flat block of 28-byte
                       filename slots, one per texture. */
                    filename = syMalloc(pvm->count_0x0c * PVM_NAME_LEN);

                    njSetPvmTextureList(texlist, texname, filename, pvm->count_0x0c);
                    njLoadTexturePvmMemory((Uint8*) var_queueBuffer_8c157a84, texlist);

                    if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                        syFree(var_queueBuffer_8c157a84);
                    }

                    task->queuedPvm_0x18 = ++pvm;
                    task->phase_0x08 = 0;
                    return;
                }
            }

            if (var_loadRetryNeeded_8c157a88) {
                task->queuedPvm_0x18 = var_pvmQueue_8c157abc;
                var_loadRetryNeeded_8c157a88 = 0;
                var_queueBaseDir_8c157a80 = init_dataEmpty_8c03be7c;
            } else {
                var_pvmQueueIsIdle_8c157ac8 = 1;
                TaskKill_8c014b66((Task*) task);
            }

            break;
        }

        case 1: {
            switch (gdFsGetStat(task->gdfs_0x0c)) {
                case GDD_STAT_COMPLETE: {
                    int i;
                    NJS_TEXLIST *texlist;
                    NJS_TEXNAME *texname;
                    char *filename;

                    gdFsClose(task->gdfs_0x0c);
                    pvm->loaded_0x14 = 1;

                    *pvm->texlist_0x08 = texlist = syMalloc(sizeof(NJS_TEXLIST));

                    texname = syMalloc(pvm->count_0x0c * sizeof(NJS_TEXNAME));
                    for (i = 0; i < pvm->count_0x0c; i++) {
                        texname[i].attr = pvm->attr_0x10;
                    }

                    /* njSetPvmTextureList wants a flat block of 28-byte
                       filename slots, one per texture. */
                    filename = syMalloc(pvm->count_0x0c * PVM_NAME_LEN);

                    njSetPvmTextureList(texlist, texname, filename, pvm->count_0x0c);
                    njLoadTexturePvmMemory((Uint8*) var_queueBuffer_8c157a84, texlist);

                    if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                        syFree(var_queueBuffer_8c157a84);
                    }

                    task->queuedPvm_0x18 = ++pvm;
                    task->phase_0x08 = 0;
                    return;
                }

                case GDD_STAT_READ: {
                    if (gdFsGetTransStat(task->gdfs_0x0c) != GDD_FS_TRANS_READY) {
                        return;
                    }
                    
                    gdFsTrans32(task->gdfs_0x0c, 2048, var_queueBuffer_8c157a84);
                    return;
                }

                default: {
                    gdFsClose(task->gdfs_0x0c);
                    if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                        syFree(var_queueBuffer_8c157a84);
                    }

                    var_loadRetryNeeded_8c157a88 = 1;
                    task->queuedPvm_0x18 = ++pvm;
                    task->phase_0x08 = 0;
                    return;
                }
            }
        }
    }
}

STATIC int sortAndLoadPvmQueue_8c011d24() {
    Task *created_task;
    void* created_state;
    QueuedPvm *temp;

    if ((int) var_pvmQueue_8c157abc == (int) var_pvmQueueRear_8c157ac0) {
        return 0;
    }

    var_pvmQueueIsIdle_8c157ac8 = 0;

    /* One element would do; the original allocates the whole queue's worth. */
    temp = syMalloc((int) var_pvmQueueRear_8c157ac0 - (int) var_pvmQueue_8c157abc);

    if (var_loadScreenActive_8c157a6c != 0) {
        while (1) {
            int swapped = 0;
            QueuedPvm *a = var_pvmQueue_8c157abc;
            QueuedPvm *b = var_pvmQueue_8c157abc;

            while (++b < var_pvmQueueRear_8c157ac0) {
                if (strcmp(a->filename, b->filename) > 0) {
                    *temp = *a;
                    *a = *b;
                    *b = *temp;
                    swapped = 1;
                }

                a++;
            }

            if (!swapped) {
                break;
            }
        }
    }

    syFree(temp);

    if (!TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, &taskLoadQueuedPvms_8c011b00, &created_task, &created_state, 0)) {
        return 0;
    }

    created_task->queuedItem_0x18 = var_pvmQueue_8c157abc;
    created_task->field_0x08 = 0;
    var_loadRetryNeeded_8c157a88 = 0;
    var_queueBaseDir_8c157a80 = init_dataEmpty_8c03be7c;

    return 1;
}

STATIC int pvmQueueIsIdle_8c011e22() {
  return var_pvmQueueIsIdle_8c157ac8;
}

STATIC void freePvmQueue_8c011e28() {
  if (var_pvmQueue_8c157abc != (void *) -1) {
    syFree(var_pvmQueue_8c157abc);
  }
}

void AsqReleaseAndFreeTexlist_8c011e3c(NJS_TEXLIST *texlist) {
    njReleaseTexture(texlist);
    syFree(texlist->textures[0].filename);
    syFree(texlist->textures);
    syFree(texlist);
}

/* Nothing in the image calls this: AsqReleaseAndFreeTexlist_8c011e3c, which
 * releases the textures first, is what everything uses. */
STATIC void asqFreeTexlist_8c011e60(NJS_TEXLIST *texlist) {
    syFree(texlist->textures[0].filename);
    syFree(texlist->textures);
    syFree(texlist);
}

STATIC void taskProcessQueues_8c011e80(Task *task, TaskProcessQueuesState *state) {
    switch (state->queue_0x00) {
        case QUEUE_DAT: {
            if (datQueueIsIdle_8c0113d2()) {
                if (state->afterDatCallback_0x08) {
                    state->afterDatCallback_0x08();
                }

                state->queue_0x00++;
                sortAndLoadNjQueue_8c0116b6();
            }
            break;
        }

        case QUEUE_NJ: {
            if (njQueueIsIdle_8c01179e()) {
                if (state->afterNjCallback_0x0c) {
                    state->afterNjCallback_0x0c();
                }

                state->queue_0x00++;
                sortAndLoadPvmQueue_8c011d24();
            }
            break;
        }

        case QUEUE_PVM: {
            if (pvmQueueIsIdle_8c011e22()) {
                if (state->afterPvmCallback_0x10) {
                    state->afterPvmCallback_0x10();
                }

                state->queue_0x00++;
                loadTexlistQueue_8c0119f8();
            }
            break;
        }

        case QUEUE_TEXLIST: {
            if (texlistQueueIsIdle_8c011a42()) {
                TaskKill_8c014b66(task);
                if (state->afterTexlistCallback_0x14) {
                    state->afterTexlistCallback_0x14();
                }

                return;
            }

            break;
        }
    }

    if (state->tickCallback_0x04) {
        state->tickCallback_0x04();
    }
}

void AsqInitQueues_8c011f36(int datCount,int njCount,int texlistCount,int pvmCount)
{
    LOG_INFO(("[ASSET_QUEUES] Initializing queues: DAT %d, NJ %d, TEXLIST %d, PVM %d\n", datCount, njCount, texlistCount, pvmCount));

    initDatQueue_8c011124(datCount);
    initNjQueue_8c011430(njCount);
    initTexlistQueue_8c0117b8(texlistCount);
    initPvmQueue_8c011a5c(pvmCount);
    VmGameSetLcdSlot_8c01c8fc(2);
    VmGameUpdateLcd_8c01c910();
    var_queuesAreInitialized_8c157a60 = 1;
}

void AsqResetQueues_8c011f6c() {
    LOG_INFO(("[ASSET_QUEUES] Resetting queues\n"));

    resetDatQueue_8c01116a();
    resetNjQueue_8c01147a();
    resetTexlistQueue_8c0117fe();
    var_pvmQueueRear_8c157ac0 = var_pvmQueue_8c157abc;
    var_queueBaseDir_8c157a80 = init_dataEmpty_8c03be7c;
    var_pvmQueueIsIdle_8c157ac8 = 1;
}

void AsqFreeQueues_8c011f7e() {
    LOG_INFO(("[ASSET_QUEUES] Freeing queues\n"));

    freeDatQueue_8c0113d8();
    freeNjQueue_8c0117a4();
    freeTexlistQueue_8c011a48();
    freePvmQueue_8c011e28();
    VmGameSetLcdSlot_8c01c8fc(0);
    var_queuesAreInitialized_8c157a60 = 0;
}

void AsqProcessQueues_8c011fe0(void *tickCallback, void *afterDatCallback, void *afterNjCallback, void *afterPvmCallback, void *afterTexlistCallback) {
    Task* created_task;
    TaskProcessQueuesState* created_state;

    TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, &taskProcessQueues_8c011e80, &created_task, (void**) &created_state, 0x18);
    created_state->queue_0x00 = QUEUE_DAT;
    created_state->afterDatCallback_0x08 = afterDatCallback;
    created_state->afterNjCallback_0x0c = afterNjCallback;
    created_state->afterPvmCallback_0x10 = afterPvmCallback;
    created_state->afterTexlistCallback_0x14 = afterTexlistCallback;
    created_state->tickCallback_0x04 = tickCallback;

    sortAndLoadDatQueue_8c011310();
}

LoadedModel* AsqRequestModels_8c012030(char *basedir, ModelFiles *pairs, int texlistCount) {
    int pairCount = 0;
    LoadedModel *dest;
    int currentPair;

    while (*pairs[pairCount].njFilename || *pairs[pairCount].pvmFilename) {
        pairCount++;
    }

    dest = syMalloc((pairCount + 1) * sizeof(LoadedModel));

    if (pairCount > 0) {
        for (currentPair = 0; currentPair < pairCount; currentPair++) {
            if (!AsqRequestNj_8c011492(basedir, pairs[currentPair].njFilename, 0, &dest[currentPair].njDest)) {
                dest[currentPair].njDest = (void*) -1;
            }

            if (!AsqRequestPvm_8c011ac0(basedir, pairs[currentPair].pvmFilename, &dest[currentPair].texlist, texlistCount, 0)) {
                dest[currentPair].texlist = (void*) -1;
            }
        }
    }

    dest[pairCount].texlist = 0;
    return dest;
}

void AsqFreeModels_8c0120fe(LoadedModel **pairsPtr) {
    int i;
    LoadedModel *pairs = *pairsPtr;

    if (pairs != (void*) -1) {
        for (i = 0; pairs[i].texlist != (void*) 0; i++) {
            if (pairs[i].texlist != (void*) -1) {
                AsqReleaseAndFreeTexlist_8c011e3c(pairs[i].texlist);
            }

            if (pairs[i].njDest != (void*) -1) {
                syFree(pairs[i].njDest);
            }
        }

        syFree(pairs);
        *pairsPtr = (void *) -1;
    }
}

void AsqSetSeedA_8c012160(int seed) {
    var_seed_8c157acc = seed;
}

int AsqGetRandomA_8c012166() {
    var_seed_8c157acc = var_seed_8c157acc * 5 + 13;
    return var_seed_8c157acc;
}

int AsqGetRandomInRangeA_8c012178(unsigned int p1) {
    if (p1) {
        return AsqGetRandomA_8c012166() % p1;
    }

    return 0;
}

void AsqSetSeedB_8c0121a2(int seed) {
    var_seed_8c157ad0 = seed;
}

int AsqGetRandomB_8c0121a8() {
    var_seed_8c157ad0 = (var_seed_8c157ad0 >> 1) * 7 + 0xb;
    return var_seed_8c157ad0;
}

int AsqGetRandomInRangeB_8c0121be(unsigned int p1) {
    if (p1) {
        return AsqGetRandomB_8c0121a8() % p1;
    }

    return 0;
}

void AsqApplyButtonConfig_8c0121e8() {
    int i;

    for (i = 0; i < 7; i++) {
        init_btnRemapManual_8c03be80[i].physical_0x00 = init_btnRemapManual_8c03be80[i].logical_0x04;
    }

    if (var_progress_8c1ba1cc.btnConfigManual_0xcc != 0) {
        if (var_progress_8c1ba1cc.btnConfigManual_0xcc == 1) {
            init_btnRemapManual_8c03be80[0].physical_0x00 = PDD_DGT_TA;
            init_btnRemapManual_8c03be80[3].physical_0x00 = PDD_DGT_TX;
        } else if (var_progress_8c1ba1cc.btnConfigManual_0xcc == 2) {
            init_btnRemapManual_8c03be80[1].physical_0x00 = PDD_DGT_TA;
            init_btnRemapManual_8c03be80[3].physical_0x00 = PDD_DGT_TB;
        }
    }

    for (i = 0; i < 7; i++) {
        init_btnRemapAuto_8c03beb8[i].physical_0x00 = init_btnRemapAuto_8c03beb8[i].logical_0x04;
    }

    init_btnRemapAuto_8c03beb8[4].physical_0x00 = PDD_DGT_KL;
    init_btnRemapAuto_8c03beb8[5].physical_0x00 = PDD_DGT_KR;

    if (var_progress_8c1ba1cc.btnConfigAuto_0xcd != 0) {
        if (var_progress_8c1ba1cc.btnConfigAuto_0xcd == 1) {
            init_btnRemapAuto_8c03beb8[0].physical_0x00 = PDD_DGT_TA;
            init_btnRemapAuto_8c03beb8[3].physical_0x00 = PDD_DGT_TX;
        } else if (var_progress_8c1ba1cc.btnConfigAuto_0xcd == 2) {
            init_btnRemapAuto_8c03beb8[1].physical_0x00 = PDD_DGT_TA;
            init_btnRemapAuto_8c03beb8[3].physical_0x00 = PDD_DGT_TB;
        }
    }

    /* The == 0 arm is subsumed by != 1. The original's branch structure (asm
     * at 8c01229e) has its else arm fall into the case-0 block, which is what
     * this reproduces; the neighbouring 0xcf test keeps a distinct case 2. */
    if (var_progress_8c1ba1cc.btnConfigWheelManual_0xce == 0 || var_progress_8c1ba1cc.btnConfigWheelManual_0xce != 1) {
        init_btnRemapWheelManual_8c03bef0[0].physical_0x00 = PDD_DGT_KD;
        init_btnRemapWheelManual_8c03bef0[1].physical_0x00 = PDD_DGT_KU;
        init_btnRemapWheelManual_8c03bef0[2].physical_0x00 = PDD_DGT_TB;
        init_btnRemapWheelManual_8c03bef0[3].physical_0x00 = PDD_DGT_TA;
    } else {
        init_btnRemapWheelManual_8c03bef0[0].physical_0x00 = PDD_DGT_TB;
        init_btnRemapWheelManual_8c03bef0[1].physical_0x00 = PDD_DGT_TA;
        init_btnRemapWheelManual_8c03bef0[2].physical_0x00 = PDD_DGT_KD;
        init_btnRemapWheelManual_8c03bef0[3].physical_0x00 = PDD_DGT_KU;
    }

    init_btnRemapWheelManual_8c03bef0[4].physical_0x00 = PDD_DGT_ST;

    if (var_progress_8c1ba1cc.btnConfigWheelAuto_0xcf == 0) {
        init_btnRemapWheelAuto_8c03bf18[0].physical_0x00 = PDD_DGT_KD;
        init_btnRemapWheelAuto_8c03bf18[1].physical_0x00 = PDD_DGT_KU;
        init_btnRemapWheelAuto_8c03bf18[2].physical_0x00 = PDD_DGT_TB;
        init_btnRemapWheelAuto_8c03bf18[3].physical_0x00 = PDD_DGT_TA;
    } else {
        if (var_progress_8c1ba1cc.btnConfigWheelAuto_0xcf == 1) {
            init_btnRemapWheelAuto_8c03bf18[0].physical_0x00 = PDD_DGT_TB;
            init_btnRemapWheelAuto_8c03bf18[1].physical_0x00 = PDD_DGT_TA;
            init_btnRemapWheelAuto_8c03bf18[2].physical_0x00 = PDD_DGT_KD;
            init_btnRemapWheelAuto_8c03bf18[3].physical_0x00 = PDD_DGT_KU;
        } else {
            if (var_progress_8c1ba1cc.btnConfigWheelAuto_0xcf != 2) {
                init_btnRemapWheelAuto_8c03bf18[0].physical_0x00 = PDD_DGT_KD;
                init_btnRemapWheelAuto_8c03bf18[1].physical_0x00 = PDD_DGT_KU;
                init_btnRemapWheelAuto_8c03bf18[2].physical_0x00 = PDD_DGT_TB;
                init_btnRemapWheelAuto_8c03bf18[3].physical_0x00 = PDD_DGT_TA;
            } else {
                init_btnRemapWheelAuto_8c03bf18[0].physical_0x00 = PDD_DGT_KL;
                init_btnRemapWheelAuto_8c03bf18[1].physical_0x00 = PDD_DGT_KR;
                init_btnRemapWheelAuto_8c03bf18[2].physical_0x00 = PDD_DGT_KD;
                init_btnRemapWheelAuto_8c03bf18[3].physical_0x00 = PDD_DGT_KU;
            }
        }
    }

    init_btnRemapWheelAuto_8c03bf18[4].physical_0x00 = PDD_DGT_ST;
}
