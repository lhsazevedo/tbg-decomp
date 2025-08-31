/* 8c011120 */

#include <shinobi.h>
#include <string.h>
#include "011120_asset_queues.h"
#include "serial_debug.h"
#include "014a9c_tasks.h"
#include "stdio.h"

extern char var_progress_8c1ba1cc[];

/* ====================
 * Compiler Definitions
 * ====================
 */


/* =================
 * Type Declarations
 * =================
 */

typedef struct {
    char *basedir;
    char *filename;
    void **dest_0x08;
    void **dest_0x0c;
    int field_0x10;
} QueuedNj;

/* TODO: Same struct as Task, but with QueuedNj. */
typedef struct {
    TaskAction action;
    void *state;
    int field_0x08;
    GDFS gdfs_0x0c;
    int field_0x10;
    int field_0x14;
    /* Perhaps we should use a union or a void* to handle both cases? */
    QueuedNj* queuedNj_0x18;
    int field_0x1c;
} TaskLoadQueuedNjs;

typedef struct {
    char *basedir;
    char *filename;
    void **texlist_0x08;
    int count_0x0c;
    int attr_0x10;
    int field_0x14;
} QueuedPvm;

typedef struct {
    TaskAction action;
    void *state;
    int field_0x08;
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
    void (*func_0x04)();
    void (*afterDatCallback_0x08)();
    void (*afterNjCallback_0x0c)();
    void (*afterPvmCallback_0x10)();
    void (*afterTexlistCallback_0x14)();
} TaskProcessQueuesState;

typedef struct {
    TaskAction action;
    void *state;
    int field_0x08;
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
STATIC int var_texlistQueueCount_8c157a68;
int var_8c157a6c;

/* TODO: Confirm type */
int var_8c157a70;
int var_8c157a74;
int var_resetRequested_8c157a78;
int var_8c157a7c;

STATIC char *var_queueBaseDir_8c157a80;
STATIC Sint8 *var_queueBuffer_8c157a84;
STATIC int var_8c157a88;

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
   ===================
 */

int init_8c03be80[14] = {
    0x0, 0x400,
    0x0,   0x2,
    0x0, 0x200,
    0x0,   0x4,
    0x0,  0x10,
    0x0,  0x20,
    0x0,   0x8,
};

int init_8c03beb8[14] = {
    0x0, 0x400,
    0x0,   0x2,
    0x0, 0x200,
    0x0,   0x4,
    0x0, 0x400,
    0x0,   0x2,
    0x0,   0x8,
};

int init_8c03bef0[10] = {
    0x0, 0x400,
    0x0,   0x2,
    0x0, 0x200,
    0x0,   0x4,
    0x0,   0x8,
};

int init_8c03bf18[10] = {
    0x0, 0x400,
    0x0,   0x2,
    0x0, 0x200,
    0x0,   0x4,
    0x0,   0x8,
};


/* =========
 * Functions
   =========
 */

/* Matched :) */
void AsqNop_11120() {
    /* Empty body */
}

/* Matched :) */
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

/* Matched */
STATIC void resetDatQueue_8c01116a() {
    var_datQueueRear_8c157a90 = var_datQueue_8c157a8c;
    var_queueBaseDir_8c157a80 = "DATA EMPTY";
    var_datQueueIsIdle_8c157a98 = 1;
}

/* Matched */
int AsqRequestDat_11182(char* basedir, char* filename, void* dest) {

    if (*filename == 0) {
        return 0;
    }

    if (var_datQueueRear_8c157a90 >= var_datQueueTail_8c157a94) {
        LOG_WARN(("[ASSET_QUEUES] DAT queue is full. Cannot enqueue \"%s\" (basedir \"%s\")\n", filename, basedir));
        return 0;
    }

    LOG_DEBUG(("[ASSET_QUEUES] DAT enqueued: \"%s\" (basedir \"%s\")\n", filename, basedir));

    var_datQueueRear_8c157a90->basedir = basedir;
    var_datQueueRear_8c157a90->filename = filename;
    var_datQueueRear_8c157a90->dest = dest;
    var_datQueueRear_8c157a90->field_0x0c = 0;

    var_datQueueRear_8c157a90++;
    return 1;
}

/* Almost matching */
STATIC void task_loadQueuedDats_8c0111b4(TaskLoadQueuedDats* task, void* state) {
    QueuedDat* item = task->queuedDat_0x18;
    Sint32 size;

    switch (task->field_0x08) {
        /* 8c0111cc */
        case 0: {
            /* 8c0111da */
            while (1) {
                /* TODO: Test this condition */
                if (item >= var_datQueueRear_8c157a90) {
                    break;
                }

                if (item->field_0x0c == 0) {
                    /* TODO: Test this update */
                    if (*item->basedir != 0 && /* 8c0111ee */
                        strcmp(var_queueBaseDir_8c157a80, item->basedir) != 0 /* 8c0111f6 */
                    ) {
                            var_queueBaseDir_8c157a80 = item->basedir;
                            gdFsChangeDir(item->basedir);
                        // }
                    }

                    /* 8c01120c */
                    task->gdfs_0x0c = gdFsOpen(item->filename, 0);
                    if (task->gdfs_0x0c == NULL) {
                        /* 8c0112f4 (shared) */
                        /* TODO: Write test for this */
                        var_8c157a88 = 1;
                        task->queuedDat_0x18++;
                        task->field_0x08 = 0;
                        return;
                    }

                    /* 8c01121a */
                    if (!gdFsGetFileSctSize(task->gdfs_0x0c, &size)) {
                        /* 8c0112f4 (shared) */
                        /* TODO: Write test for this */
                        var_8c157a88 = 1;
                        task->queuedDat_0x18++;
                        task->field_0x08 = 0;
                        return;
                    }

                    /* 8c011226 */
                    *item->dest = syMalloc(size * 2048);

                    /* 8c011234 */
                    if (gdFsRead(task->gdfs_0x0c, size, *item->dest) != GDD_ERR_OK) {
                        /* 8c0112f4 (shared) */
                        /* TODO: Write test for this */
                        var_8c157a88 = 1;
                        task->queuedDat_0x18++;
                        task->field_0x08 = 0;
                        return;
                    }

                    /* 8c011282 (shared) */
                    gdFsClose(task->gdfs_0x0c);
                    item->field_0x0c = 1;
                    task->queuedDat_0x18 = ++item;
                    task->field_0x08 = 0;
                    return;
                }

                item++;
            }

            /* 8c01124a */
            if (var_8c157a88 != 0) {
                /* 8c011250 */
                task->queuedDat_0x18 = var_datQueue_8c157a8c;
                var_8c157a88 = 0;
                var_queueBaseDir_8c157a80 = "DATA EMPTY";
                /* return */;
            } else {
                /* 8c011262 */
                var_datQueueIsIdle_8c157a98 = 1;
                freeTask_8c014b66((Task*) task);
                /* return; */
            }
            break;
        }

        /* 8c0111d2 */
        case 1: {
            /* 8c011270 */
            switch (gdFsGetStat(task->gdfs_0x0c)) {
                case GDD_STAT_COMPLETE: {
                    /* 8c011282 (shared) */
                    gdFsClose(task->gdfs_0x0c);
                    item->field_0x0c = 1;
                    task->queuedDat_0x18++;
                    task->field_0x08 = 0;
                    return;
                } 
                case GDD_STAT_READ: { /* 8c01127a */
                    /* 8c0112cc */
                    if (gdFsGetTransStat(task->gdfs_0x0c) == GDD_FS_TRANS_READY) {
                        /* 8c0112d6 */
                        gdFsTrans32(task->gdfs_0x0c, 2048, *item->dest);
                    }
                    break;
                } 
                default: {
                    gdFsClose(task->gdfs_0x0c);
                    syFree(item->dest);
                    /* 8c0112f4 (shared) */
                    var_8c157a88 = 1;
                    task->queuedDat_0x18++;
                    task->field_0x08 = 0;
                    break;
                }
            }
            break;
        }
    }
}

/* Tested */
STATIC int sortAndLoadDatQueue_8c011310() {
    int r9;
    Task *created_task;
    void *created_state;
    QueuedDat *temp_r11;

    if ((int) var_datQueue_8c157a8c == (int) var_datQueueRear_8c157a90) {
        return 0;
    }

    /* 8c01132e */
    var_datQueueIsIdle_8c157a98 = 0;

    temp_r11 = syMalloc((int) var_datQueueRear_8c157a90 - (int) var_datQueue_8c157a8c);

    /* 8c011340 */
    while (1) {
        int r9 = 0;
        QueuedDat *a_r13 = var_datQueue_8c157a8c;
        QueuedDat *b_r14 = var_datQueue_8c157a8c;

        /* 8c011376 */
        while (++b_r14 < var_datQueueRear_8c157a90) {
            /* 8c011348 */
            if (strcmp(a_r13->filename, b_r14->filename) > 0) {
                /* 8c011354 */
                *temp_r11 = *a_r13;
                *a_r13 = *b_r14;
                *b_r14 = *temp_r11;
                r9 = 1;
            }

            /* 8c011374 */
            a_r13++;
        }

        if (r9 == 0) { /* 8c011374 */
            break;
        }
    }

    syFree(temp_r11);

    if (!pushTask_8c014ae8(var_tasks_8c1ba3c8, &task_loadQueuedDats_8c0111b4, &created_task, &created_state, 0)) {
        return 0;
    }

    created_task->queuedItem_0x18 = var_datQueue_8c157a8c;
    created_task->field_0x08 = 0;
    var_8c157a88 = 0;
    var_queueBaseDir_8c157a80 = "DATA EMPTY";

    return 1;
}

/* Matched */
STATIC int datQueueIsIdle_8c0113d2() {
    return var_datQueueIsIdle_8c157a98;
}

/* Matched */
STATIC void freeDatQueue_8c0113d8() {
    if (var_datQueue_8c157a8c != (QueuedDat*) -1) {
        syFree((void*) var_datQueue_8c157a8c);
    }
}

/* Matched */
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

/* Matched */
STATIC void resetNjQueue_8c01147a() {
    var_njQueueRear_8c157aa0 = var_njQueue_8c157a9c;
    var_queueBaseDir_8c157a80 = "DATA EMPTY";
    var_njQueueIsIdle_8c157aa8 = 1;
}

/* Matched */
int AsqRequestNj_11492(char* basedir, char* filename, void* dest, void* dest2) {

    if (*filename == 0) {
        return 0;
    }

    if (var_njQueueRear_8c157aa0 >= var_njQueueTail_8c157aa4) {
        return 0;
    }

    LOG_DEBUG(("[ASSET_QUEUES] NJ enqueued: \"%s\" (basedir \"%s\")\n", filename, basedir));

    var_njQueueRear_8c157aa0->basedir = basedir;
    var_njQueueRear_8c157aa0->filename = filename;
    var_njQueueRear_8c157aa0->dest_0x08 = dest;
    var_njQueueRear_8c157aa0->dest_0x0c = dest2;
    var_njQueueRear_8c157aa0->field_0x10 = 0;

    var_njQueueRear_8c157aa0++;
    return 1;
}

/* Tested */
STATIC void task_loadQueuedNjs_8c0114cc(TaskLoadQueuedNjs* task, void* state) {
    QueuedNj* qnj = task->queuedNj_0x18;
    Sint32 size;
    Uint32 fpos = 0, rtype;

    switch (task->field_0x08)
    {
        case 0:
            while (1)
            {
                if (qnj >= var_njQueueRear_8c157aa0) {
                    break;
                }

                if (qnj->field_0x10 == 0) {
                    if (
                        *qnj->basedir != 0 &&
                        strcmp(var_queueBaseDir_8c157a80, qnj->basedir) != 0
                    ) {
                        var_queueBaseDir_8c157a80 = qnj->basedir;
                        gdFsChangeDir(qnj->basedir);
                    }

                    task->gdfs_0x0c = gdFsOpen(qnj->filename, 0);

                    if (task->gdfs_0x0c == NULL) {
                        /* 8c01168c (shared) */
                        if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                            syFree(var_queueBuffer_8c157a84);
                        }
                        var_8c157a88 = 1;
                        task->queuedNj_0x18++;
                        task->field_0x08 = 0;
                        return;
                    }

                    if (!gdFsGetFileSctSize(task->gdfs_0x0c, &size)) {
                        /* 8c01168c (shared) */
                        if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                            syFree(var_queueBuffer_8c157a84);
                        }
                        var_8c157a88 = 1;
                        task->queuedNj_0x18++;
                        task->field_0x08 = 0;
                        return;
                    }

                    if (size > 0x100) {
                        var_queueBuffer_8c157a84 = syMalloc(size * 2048);
                    } else {
                        var_queueBuffer_8c157a84 = var_texbuf_8c277ca0;
                    }

                    if (gdFsRead(task->gdfs_0x0c, size, var_queueBuffer_8c157a84) != GDD_ERR_OK) {
                        /* 8c01168c (shared) */
                        if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                            syFree(var_queueBuffer_8c157a84);
                        }
                        var_8c157a88 = 1;
                        task->queuedNj_0x18++;
                        task->field_0x08 = 0;
                        return; 
                    }

                    gdFsClose(task->gdfs_0x0c);
                    qnj->field_0x10 = 1;

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
                    task->field_0x08 = 0;
                    return;
                }

                qnj++;
            }

            if (var_8c157a88 != 0) {
                task->queuedNj_0x18 = var_njQueue_8c157a9c;
                var_8c157a88 = 0;
                var_queueBaseDir_8c157a80 = "DATA EMPTY";
            } else {
                var_njQueueIsIdle_8c157aa8 = 1;
                freeTask_8c014b66((Task*) task);
            }

            break;

        case 1:
            switch (gdFsGetStat(task->gdfs_0x0c)) {
                /* 8c011614 */
                case GDD_STAT_COMPLETE: {
                    gdFsClose(task->gdfs_0x0c);
                    qnj->field_0x10 = 1;

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
                    task->field_0x08 = 0;

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
                    
                    if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                        syFree(var_queueBuffer_8c157a84);
                    }

                    if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                        syFree(var_queueBuffer_8c157a84);
                    }

                    var_8c157a88 = 1;
                    /* TODO: Test this with other item indexes */
                    task->queuedNj_0x18 = ++qnj;
                    task->field_0x08 = 0;
                    break;
                }
            }
            break;
        }
}

/* Tested */
STATIC int sortAndLoadNjQueue_8c0116b6() {
    Task *created_task;
    void* created_state;
    QueuedNj *temp;

    if ((int) var_njQueue_8c157a9c == (int) var_njQueueRear_8c157aa0) {
        return 0;
    }

    /* 8c01132e */
    var_njQueueIsIdle_8c157aa8 = 0;

    temp = syMalloc((int) var_njQueueRear_8c157aa0 - (int) var_njQueue_8c157a9c);

    /* 8c011340 */
    while (1) {
        int swapped = 0;
        QueuedNj *a = var_njQueue_8c157a9c;
        QueuedNj *b = var_njQueue_8c157a9c;

        /* 8c011376 */
        while (++b < var_njQueueRear_8c157aa0) {
            /* 8c011348 */
            if (strcmp(a->filename, b->filename) > 0) {
                /* 8c011354 */
                *temp = *a;
                *a = *b;
                *b = *temp;
                swapped = 1;
            }

            /* 8c011374 */
            a++;
        }

        if (!swapped) { /* 8c011374 */
            break;
        }
    }

    syFree(temp);

    if (!pushTask_8c014ae8(var_tasks_8c1ba3c8, &task_loadQueuedNjs_8c0114cc, &created_task, &created_state, 0)) {
        return 0;
    }

    created_task->queuedItem_0x18 = var_njQueue_8c157a9c;
    created_task->field_0x08 = 0;
    var_8c157a88 = 0;
    var_queueBaseDir_8c157a80 = "DATA EMPTY";

    return 1;
}

/* Matched */
STATIC int njQueueIsIdle_8c01179e() {
    return var_njQueueIsIdle_8c157aa8;
}

/* Tested */
STATIC void freeNjQueue_8c0117a4() {
    if (var_njQueue_8c157a9c != (QueuedNj*) -1) {
        syFree(var_njQueue_8c157a9c);
    }
}

/* Tested */
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

/* Tested */
STATIC void resetTexlistQueue_8c0117fe() {
    var_texlistQueueRear_8c157ab0 = var_texlistQueue_8c157aac;
    var_queueBaseDir_8c157a80 = "DATA EMPTY";
    var_texlistQueueCount_8c157a68 = 0;
    var_texlistQueueIsIdle_8c157ab8 = 1;
    return;
}

/* Tested */
int AsqRequestTexlist_1181c(char *basedir, NJS_TEXLIST *texlist) {
    if (var_texlistQueueRear_8c157ab0 >= var_texlistQueueTail_8c157ab4) {
        return 0;
    }

    LOG_DEBUG(("[ASSET_QUEUES] Texlist enqueued: %p (basedir \"%s\")\n", texlist, basedir));

    var_texlistQueueRear_8c157ab0->basedir_0x00 = basedir;
    var_texlistQueueRear_8c157ab0->texlist_0x04 = texlist;
    var_texlistQueueRear_8c157ab0++;
    return 1;
}

/* Tested */
STATIC void task_loadQueuedTexlists_8c01183e(Task *task, void *state) {
    QueuedTexlist *item = task->queuedItem_0x18;
    NJS_TEXLIST *texlist;

    while (true) {
        int i;
        /* Assume that the current texlist is already loaded */
        bool alreadyLoaded = true;

        texlist = item->texlist_0x04;

        for (i = 0; i < texlist->nbTexture; i++)
        {
            int queueIdx;
            NJS_TEXNAME *currentTexture = &texlist->textures[i];

            for (queueIdx = 0; queueIdx < var_texlistQueueCount_8c157a68; queueIdx++)
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

                if (comparedTextureCount) {
                    for (comparedIndex = 0; comparedIndex < comparedTextureCount; comparedIndex++)
                    {
                        if (!strcmp(currentTexture->filename, comparedTextures[comparedIndex].filename)) {
                            currentTexture->texaddr = comparedTextures[comparedIndex].texaddr;
                            break;
                        }
                    }
                }

                /* If we the current texture is already loaded,
                   we can sdvance to next one in the current texlist.
                   Note that breaking prevents the loop counter increment. */
                if (comparedIndex != comparedTextureCount) break;

                /* Refactor: */
                /* if (currentTexture->texaddr) break */
            }

            /* If the current texture was not found in any of
               the compared texlists, then it should be loaded. */
            if (queueIdx == var_texlistQueueCount_8c157a68) {
                alreadyLoaded = false;
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
                /* TODO: Test this skip */
                for (i = 0; i < texlist->nbTexture; i++) {
                    if (!texlist->textures[i].texaddr) {
                        njLoadTextureNum(i);
                    }
                }
            }
        }

        item++;
        var_texlistQueueCount_8c157a68++;
        if (item >= var_texlistQueueRear_8c157ab0) {
            var_texlistQueueIsIdle_8c157ab8 = 1;
            freeTask_8c014b66(task);
            return;
        }

        /* TODO: Test this path */
        if (!alreadyLoaded) {
            task->queuedItem_0x18 = item;
            return;
        }
    }
}

/* Tested */
STATIC int loadTexlistQueue_8c0119f8() {
    Task *created_task;
    void *created_state;

    if (var_texlistQueue_8c157aac == var_texlistQueueRear_8c157ab0) {
        return 0;
    }

    var_texlistQueueIsIdle_8c157ab8 = 0;
    if (!pushTask_8c014ae8(var_tasks_8c1ba3c8, task_loadQueuedTexlists_8c01183e, &created_task, &created_state, 0)) {
        return 0;
    }

    created_task->queuedItem_0x18 = var_texlistQueue_8c157aac;
    return 1;
}

/* Tested */
STATIC int texlistQueueIsIdle_8c011a42() {
  return var_texlistQueueIsIdle_8c157ab8;
}

/* Tested */
STATIC void freeTexlistQueue_8c011a48() {
    if (var_texlistQueue_8c157aac != (void *) -1) {
        syFree(var_texlistQueue_8c157aac);
    }
}

/* Tested */
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

/* Tested */
int AsqRequestPvm_11ac0(char *basedir, char *filename, void *texlist, int count, int attr) {
    if (!*filename || var_pvmQueueRear_8c157ac0 >= var_pvmQueueTail_8c157ac4) {
        return 0;
    }

    LOG_DEBUG(("[ASSET_QUEUES] PVM enqueued: %s (basedir %s)\n", filename, basedir));

    var_pvmQueueRear_8c157ac0->basedir = basedir;
    var_pvmQueueRear_8c157ac0->filename = filename;
    var_pvmQueueRear_8c157ac0->texlist_0x08 = texlist;
    var_pvmQueueRear_8c157ac0->count_0x0c = count;
    var_pvmQueueRear_8c157ac0->attr_0x10 = attr;
    var_pvmQueueRear_8c157ac0->field_0x14 = 0;

    var_pvmQueueRear_8c157ac0++;

    return 1;
}

/* Tested */
STATIC void task_loadQueuedPvms_8c011b00(TaskLoadQueuedPvms* task, void* state) {
    QueuedPvm *pvm = (QueuedPvm*) task->queuedPvm_0x18;
    Sint32 size;

    switch (task->field_0x08) {
        case 0: {
            for (; pvm < var_pvmQueueRear_8c157ac0; pvm++)
            {
                if (pvm->field_0x14 == 0) {
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
                        var_8c157a88 = 1;
                        task->queuedPvm_0x18 = ++pvm;
                        task->field_0x08 = 0;
                        return;
                    }

                    if (!gdFsGetFileSctSize(task->gdfs_0x0c, &size)) {
                        var_8c157a88 = 1;
                        task->queuedPvm_0x18 = ++pvm;
                        task->field_0x08 = 0;
                        return;
                    }

                    if (size > 0x100) {
                        /* TODO: Test this path */
                        var_queueBuffer_8c157a84 = syMalloc(size * 2048);
                    } else {
                        var_queueBuffer_8c157a84 = var_texbuf_8c277ca0;
                    }

                    if (gdFsRead(task->gdfs_0x0c, size, var_queueBuffer_8c157a84)) {
                        var_8c157a88 = 1;
                        task->queuedPvm_0x18 = ++pvm;
                        task->field_0x08 = 0;
                        return;
                    }

                    gdFsClose(task->gdfs_0x0c);
                    pvm->field_0x14 = 1;

                    *pvm->texlist_0x08 = texlist = syMalloc(sizeof(NJS_TEXLIST));

                    texname = syMalloc(pvm->count_0x0c * 0xc);
                    for (i = 0; i < pvm->count_0x0c; i++) {
                        texname[i].attr = pvm->attr_0x10;
                    }

                    /* Huh? */
                    filename = syMalloc(pvm->count_0x0c * 0x1c);

                    njSetPvmTextureList(texlist, texname, filename, pvm->count_0x0c);
                    njLoadTexturePvmMemory((Uint8*) var_queueBuffer_8c157a84, texlist);

                    if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                        syFree(var_queueBuffer_8c157a84);
                    }

                    task->queuedPvm_0x18 = ++pvm;
                    task->field_0x08 = 0;
                    return;
                }
            }

            if (var_8c157a88) {
                task->queuedPvm_0x18 = var_pvmQueue_8c157abc;
                var_8c157a88 = 0;
                var_queueBaseDir_8c157a80 = "DATA EMPTY";
            } else {
                var_pvmQueueIsIdle_8c157ac8 = 1;
                freeTask_8c014b66((Task*) task);
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
                    pvm->field_0x14 = 1;

                    *pvm->texlist_0x08 = texlist = syMalloc(sizeof(NJS_TEXLIST));

                    texname = syMalloc(pvm->count_0x0c * sizeof(NJS_TEXNAME));
                    for (i = 0; i < pvm->count_0x0c; i++) {
                        texname[i].attr = pvm->attr_0x10;
                    }

                    /* Huh? */
                    filename = syMalloc(pvm->count_0x0c * 0x1c);

                    njSetPvmTextureList(texlist, texname, filename, pvm->count_0x0c);
                    njLoadTexturePvmMemory((Uint8*) var_queueBuffer_8c157a84, texlist);

                    if (var_queueBuffer_8c157a84 != var_texbuf_8c277ca0) {
                        /* TODO: Test this path */
                        syFree(var_queueBuffer_8c157a84);
                    }

                    task->queuedPvm_0x18 = ++pvm;
                    task->field_0x08 = 0;
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
                        /* TODO: Test this path */
                        syFree(var_queueBuffer_8c157a84);
                    }

                    var_8c157a88 = 1;
                    task->queuedPvm_0x18 = ++pvm;
                    task->field_0x08 = 0;
                    return;
                }
            }
        }
    }
}

/* Tested */
STATIC int sortAndLoadPvmQueue_8c011d24() {
    Task *created_task;
    void* created_state;
    QueuedPvm *temp;

    if ((int) var_pvmQueue_8c157abc == (int) var_pvmQueueRear_8c157ac0) {
        return 0;
    }

    var_pvmQueueIsIdle_8c157ac8 = 0;

    /* Why not allocate a single QueuedPvm? */
    temp = syMalloc((int) var_pvmQueueRear_8c157ac0 - (int) var_pvmQueue_8c157abc);

    /* TODO: Test this skip */
    if (var_8c157a6c != 0) {
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

    if (!pushTask_8c014ae8(var_tasks_8c1ba3c8, &task_loadQueuedPvms_8c011b00, &created_task, &created_state, 0)) {
        return 0;
    }

    created_task->queuedItem_0x18 = var_pvmQueue_8c157abc;
    created_task->field_0x08 = 0;
    var_8c157a88 = 0;
    var_queueBaseDir_8c157a80 = "DATA EMPTY";

    return 1;
}

/* Tested */
STATIC int pvmQueueIsIdle_8c011e22() {
  return var_pvmQueueIsIdle_8c157ac8;
}

/* Tested */
STATIC void freePvmQueue_8c011e28() {
  if (var_pvmQueue_8c157abc != (void *) -1) {
    syFree(var_pvmQueue_8c157abc);
  }
}

/* Tested */
void AsqReleaseAndFreeTexlist_11e3c(NJS_TEXLIST *texlist) {
    njReleaseTexture(texlist);
    syFree(texlist->textures[0].filename);
    syFree(texlist->textures);
    syFree(texlist);
}

/* Tested */
/* Unused */
void AsqFreeTexlist_11e60(NJS_TEXLIST *texlist) {
    syFree(texlist->textures[0].filename);
    syFree(texlist->textures);
    syFree(texlist);
}

/* Tested */
STATIC void task_processQueues_8c011e80(Task *task, TaskProcessQueuesState *state) {
    switch (state->queue_0x00) {
        /* TODO: Use enum */
        case 0: {
            if (datQueueIsIdle_8c0113d2()) {
                if (state->afterDatCallback_0x08) {
                    state->afterDatCallback_0x08();
                }

                state->queue_0x00++;
                sortAndLoadNjQueue_8c0116b6();
            }
            break;
        }

        case 1: {
            if (njQueueIsIdle_8c01179e()) {
                if (state->afterNjCallback_0x0c) {
                    state->afterNjCallback_0x0c();
                }

                state->queue_0x00++;
                sortAndLoadPvmQueue_8c011d24();
            }
            break;
        }

        case 2: {
            if (pvmQueueIsIdle_8c011e22()) {
                if (state->afterPvmCallback_0x10) {
                    state->afterPvmCallback_0x10();
                }

                state->queue_0x00++;
                loadTexlistQueue_8c0119f8();
            }
            break;
        }

        case 3: {
            if (texlistQueueIsIdle_8c011a42()) {
                freeTask_8c014b66(task);
                if (state->afterTexlistCallback_0x14) {
                    state->afterTexlistCallback_0x14();
                }

                return;
            }

            break;
        }
    }

    if (state->func_0x04) {
        state->func_0x04();
    }
}

/* Tested */
void AsqInitQueues_11f36(int datCount,int njCount,int texlistCount,int pvmCount)
{
    LOG_INFO(("[ASSET_QUEUES] Initializing queues: DAT %d, NJ %d, TEXLIST %d, PVM %d\n", datCount, njCount, texlistCount, pvmCount));

    initDatQueue_8c011124(datCount);
    initNjQueue_8c011430(njCount);
    initTexlistQueue_8c0117b8(texlistCount);
    initPvmQueue_8c011a5c(pvmCount);
    vmsLcd_8c01c8fc(2);
    vmsLcd_8c01c910();
    var_queuesAreInitialized_8c157a60 = 1;
}

/* Tested */
void AsqResetQueues_11f6c() {
    LOG_INFO(("[ASSET_QUEUES] Resetting queues\n"));

    resetDatQueue_8c01116a();
    resetNjQueue_8c01147a();
    resetTexlistQueue_8c0117fe();
    var_pvmQueueRear_8c157ac0 = var_pvmQueue_8c157abc;
    var_queueBaseDir_8c157a80 = "DATA EMPTY";
    var_pvmQueueIsIdle_8c157ac8 = 1;
}

/* Tested */
void AsqFreeQueues_11f7e() {
    LOG_INFO(("[ASSET_QUEUES] Freeing queues\n"));

    freeDatQueue_8c0113d8();
    freeNjQueue_8c0117a4();
    freeTexlistQueue_8c011a48();
    freePvmQueue_8c011e28();
    vmsLcd_8c01c8fc(0);
    var_queuesAreInitialized_8c157a60 = 0;
}

/* Tested */
void AsqProcessQueues_11fe0(void *func, void *afterDatCallback, void *afterNjCallback, void *afterPvmCallback, void *afterTexlistCallback) {
    Task* created_task;
    TaskProcessQueuesState* created_state;

    pushTask_8c014ae8(var_tasks_8c1ba3c8, &task_processQueues_8c011e80, &created_task, (void**) &created_state, 0x18);
    created_state->queue_0x00 = 0;
    created_state->afterDatCallback_0x08 = afterDatCallback;
    created_state->afterNjCallback_0x0c = afterNjCallback;
    created_state->afterPvmCallback_0x10 = afterPvmCallback;
    created_state->afterTexlistCallback_0x14 = afterTexlistCallback;
    created_state->func_0x04 = func;

    sortAndLoadDatQueue_8c011310();
}

/* Tested */
NjPvmPair* AsqRequestNjPvmPairs_12030(char *basedir, NjPvmPairFilenames *pairs, int texlistCount) {
    int pairCount = 0;
    NjPvmPair *dest;
    int currentPair;

    while (*pairs[pairCount].njFilename || *pairs[pairCount].pvmFilename) {
        pairCount++;
    }

    dest = syMalloc((pairCount + 1) * sizeof(NjPvmPair));

    if (pairCount > 0) {
        for (currentPair = 0; currentPair < pairCount; currentPair++) {
            if (!AsqRequestNj_11492(basedir, pairs[currentPair].njFilename, 0, &dest[currentPair].njDest)) {
                dest[currentPair].njDest = (void*) -1;
            }

            if (!AsqRequestPvm_11ac0(basedir, pairs[currentPair].pvmFilename, &dest[currentPair].texlist, texlistCount, 0)) {
                dest[currentPair].texlist = (void*) -1;
            }
        }
    }

    dest[pairCount].texlist = 0;
    return dest;
}

/* Tested */
void AsqFreeNjPvmPairs_120fe(NjPvmPair **pairsPtr) {
    int i;
    NjPvmPair *pairs = *pairsPtr;

    if (pairs != (void*) -1) {
        for (i = 0; pairs[i].texlist != (void*) 0; i++) {
            if (pairs[i].texlist != (void*) -1) {
                AsqReleaseAndFreeTexlist_11e3c(pairs[i].texlist);
            }

            if (pairs[i].njDest != (void*) -1) {
                syFree(pairs[i].njDest);
            }
        }

        syFree(pairs);
        *pairsPtr = (void *) -1;
    }
}

/* Tested */
void AsqSetSeedA_12160(int seed) {
    var_seed_8c157acc = seed;
}

/* Tested */
int AsqGetRandomA_12166() {
    var_seed_8c157acc = var_seed_8c157acc * 5 + 13;
    return var_seed_8c157acc;
}

/* Tested */
int AsqGetRandomInRangeA_12178(unsigned int p1) {
    if (p1) {
        return AsqGetRandomA_12166() % p1;
    }

    return 0;
}

/* Tested */
void AsqSetSeedB_121a2(int seed) {
    var_seed_8c157ad0 = seed;
}

/* Tested */
int AsqGetRandomB_121a8() {
    var_seed_8c157ad0 = (var_seed_8c157ad0 >> 1) * 7 + 0xb;
    return var_seed_8c157ad0;
}

/* Tested */
int AsqGetRandomInRangeB_121be(unsigned int p1) {
    if (p1) {
        return AsqGetRandomB_121a8() % p1;
    }

    return 0;
}

/* Tested */
void AsqFUN_121e8() {
    int i;

    for (i = 0; i < 14; i += 2) {
        init_8c03be80[i] = init_8c03be80[i+1];
    }

    if (var_progress_8c1ba1cc[0xcc] != 0) {
        if (var_progress_8c1ba1cc[0xcc] == 1) {
            init_8c03be80[0] = 4;
            init_8c03be80[6] = 0x400; /* init_8c03be98 */
        } else if (var_progress_8c1ba1cc[0xcc] == 2) {
            init_8c03be80[2] = 4;
            init_8c03be80[6] = 2; /* init_8c03be98 */
        }
    }

    for (i = 0; i < 14; i += 2) {
        init_8c03beb8[i] = init_8c03beb8[i + 1];
    }

    init_8c03beb8[8] = 0x40;
    init_8c03beb8[10] = 0x80;

    if (var_progress_8c1ba1cc[0xcd] != 0) {
        if (var_progress_8c1ba1cc[0xcd] == 1) {
            init_8c03beb8[0] = 4;
            init_8c03beb8[6] = 0x400;
        } else if (var_progress_8c1ba1cc[0xcd] == 2) {
            init_8c03beb8[2] = 4;
            init_8c03beb8[6] = 2;
        }
    }

    if (var_progress_8c1ba1cc[0xce] == 0 || var_progress_8c1ba1cc[0xce] != 1) {
        init_8c03bef0[0] = 0x20;
        init_8c03bef0[2] = 0x10;
        init_8c03bef0[4] = 0x02;
        init_8c03bef0[6] = 0x04;
    } else {
        init_8c03bef0[0] = 0x02;
        init_8c03bef0[2] = 0x04;
        init_8c03bef0[4] = 0x20;
        init_8c03bef0[6] = 0x10;
    }

    init_8c03bef0[8] = 8;

    if (var_progress_8c1ba1cc[0xcf] == 0) {
        init_8c03bf18[0] = 0x20;
        init_8c03bf18[2] = 0x10;
        init_8c03bf18[4] = 2;
        init_8c03bf18[6] = 4;
    } else {
        if (var_progress_8c1ba1cc[0xcf] == 1) {
            init_8c03bf18[0] = 2;
            init_8c03bf18[2] = 4;
            init_8c03bf18[4] = 0x20;
            init_8c03bf18[6] = 0x10;
        } else {
            if (var_progress_8c1ba1cc[0xcf] != 2) {
                init_8c03bf18[0] = 0x20;
                init_8c03bf18[2] = 0x10;
                init_8c03bf18[4] = 2;
                init_8c03bf18[6] = 4;
            } else {
                init_8c03bf18[0] = 0x40;
                init_8c03bf18[2] = 0x80;
                init_8c03bf18[4] = 0x20;
                init_8c03bf18[6] = 0x10;
            }
        }
    }

    init_8c03bf18[8] = 8;
}
