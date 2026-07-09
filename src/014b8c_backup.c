/* 8c014b8c */
/*
 * Adjusted backup sample from SDK 155j
 */

#include <shinobi.h>
#include "014b8c_backup.h"

/*
 * Mamimum volume to use.
 */
#define MAX_CAPS BUD_CAPACITY_1MB

/*
 * Number of memory card to use.
 */
#define MAX_DRIVES 8
#define USE_DRIVES BUD_USE_DRIVE_ALL

/*
 * Structure to store the information of memory card.
 * (See backup.h)
 */
extern BACKUPINFO var_gBupInfo_8c1bc4ac[8];



/*
 * Prototypes of static functions.
 */
static Sint32 BupComplete_8c014e70(Sint32 drive, Sint32 op, Sint32 stat, Uint32 param);
static Sint32 BupProgress_8c014f04(Sint32 drive, Sint32 op, Sint32 count, Sint32 max);
static void BupInitCallback_8c014e5e(void);
void ClearInfo_8c014c8a(Sint32 drive);



void bupInit_8c014b8c(void)
{
    memset(var_gBupInfo_8c1bc4ac, 0, sizeof(var_gBupInfo_8c1bc4ac));
    buInit(MAX_CAPS, USE_DRIVES, NULL, BupInitCallback_8c014e5e);
}

void BupExit(void)
{
    do {} while (buExit() != BUD_ERR_OK);
}

const BACKUPINFO* BupGetInfo_8c014bba(Sint32 drive)
{
    return (const BACKUPINFO*)&var_gBupInfo_8c1bc4ac[drive];
}

Sint32 BupLoad_8c014bc6(Sint32 drive, const char* fname, void* buf)
{
    return buLoadFile(drive, fname, buf, 0);
}

enum {
    Sunday = 0,
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday
};

static SYS_RTC_DATE gBupTime_8c04411c = {
    1998, 12, 31,   /* year, month, day     */
    23, 59, 59,     /* hour, minute, second */
    Thursday, 0     /* day of week, age of moon */
};

Sint32 BupSave_8c014bcc(Sint32 drive, const char* fname, void* buf, Sint32 nblock)
{
    syRtcGetDate(&gBupTime_8c04411c);
    return buSaveFile(drive, fname, buf, nblock, &gBupTime_8c04411c,
                            BUD_FLAG_VERIFY | BUD_FLAG_COPY(0));
}

Sint32 BupDelete_8c014bfa(Sint32 drive, const char* fname)
{
    return buDeleteFile(drive, fname);
}

void BupMount_8c014c00(Sint32 drive)
{
    BACKUPINFO* info;

    info = &var_gBupInfo_8c1bc4ac[drive];

    if (info->Work) return;

    info->Work = syMalloc(info->WorkSize);

    buMountDisk(drive, info->Work, info->WorkSize);
}

void BupUnmount_8c014c46(Sint32 drive)
{
    BACKUPINFO* info;
    Sint32 err;

    info = &var_gBupInfo_8c1bc4ac[drive];

    if (info->Work == NULL) return;

    if (buStat(drive) == BUD_STAT_READY) {
        buUnmount(drive);
        syFree(info->Work);
        ClearInfo_8c014c8a(drive);
    }
}

void ClearInfo_8c014c8a(Sint32 drive)
{
    BACKUPINFO* info;

    info = &var_gBupInfo_8c1bc4ac[drive];
    info->ProgressCount = 0;
    info->ProgressMax = 0;
    info->Operation = 0;
    info->Ready = FALSE;
    info->IsFormat = FALSE;
    info->Work = NULL;
    memset(&info->DiskInfo, 0, sizeof(BUS_DISKINFO));
}

const char* BupGetErrorString_8c014cfc(Sint32 err)
{
    switch (err) {
        case BUD_ERR_OK:             return "OK\0";
        case BUD_ERR_BUSY:           return "BUSY\0";
        case BUD_ERR_INVALID_PARAM:  return "INVALID PARAMETER\0";
        case BUD_ERR_ILLEGAL_DISK:   return "ILLEGAL DISK\0";
        case BUD_ERR_UNKNOWN_DISK:   return "UNKNOWN DISK\0";
        case BUD_ERR_NO_DISK:        return "NO DISK\0";
        case BUD_ERR_UNFORMAT:       return "UNFORMAT\0";
        case BUD_ERR_DISK_FULL:      return "DISK FULL\0";
        case BUD_ERR_FILE_NOT_FOUND: return "FILE NOT FOUND\0";
        case BUD_ERR_FILE_EXIST:     return "FILE EXIST\0";
        case BUD_ERR_CANNOT_OPEN:    return "CANNOT OPEN\0";
        case BUD_ERR_CANNOT_CREATE:  return "CANNOT CREATE\0";
        case BUD_ERR_EXECFILE_EXIST: return "EXECUTABLE FILE EXIST\0";
        case BUD_ERR_ACCESS_DENIED:  return "ACCESS DENIED\0";
        case BUD_ERR_VERIFY:         return "VERIFY ERROR\0";
        default:						    return "GENERIC ERROR\0";
    }
}

const char* BupGetOperationString_8c014e0c(Sint32 op)
{
    switch (op) {
        case BUD_OP_CONNECT:         return "CONNECTED\0";
        case BUD_OP_MOUNT:           return "MOUNTED\0";
        case BUD_OP_UNMOUNT:         return "UNMOUNTED\0";
        case BUD_OP_FORMATDISK:      return "FORMATDISK\0";
        case BUD_OP_DELETEFILE:      return "DELETEFILE\0";
        case BUD_OP_LOADFILE:        return "LOADFILE\0";
        case BUD_OP_SAVEFILE:        return "SAVEFILE\0";
        default:                     return "UNKNOWN OPERATION\0";
    }
}


/*
 * Callback functions.
 */

static void BupInitCallback_8c014e5e(void)
{
    Sint32 i;

    buSetCompleteCallback(BupComplete_8c014e70);
    buSetProgressCallback(BupProgress_8c014f04);
}

static Sint32 BupComplete_8c014e70(Sint32 drive, Sint32 op, Sint32 stat, Uint32 param)
{
    BACKUPINFO* info;
    Sint32 ret;


    info = &var_gBupInfo_8c1bc4ac[drive];

    switch (op) {
        case BUD_OP_CONNECT:
            info->Connect = TRUE;
            info->WorkSize = BUM_WORK_SIZE(stat, 1);
            info->Capacity = stat;
            break;
        case BUD_OP_MOUNT:
            if (stat == BUD_ERR_OK) {
                info->Ready = TRUE;
                ret = buGetDiskInfo(drive, &info->DiskInfo);
                if (ret == BUD_ERR_OK) {
                    info->IsFormat = TRUE;
                }
                info->LastError = BUD_ERR_OK;
            }
            break;
        case BUD_OP_UNMOUNT:
            if (info->Work) syFree(info->Work);
            ClearInfo_8c014c8a(drive);
            info->Connect = FALSE;
            break;
        default:
            info->LastError = stat;

            buGetDiskInfo(drive, &info->DiskInfo);
    }

    info->Operation = 0;

    return BUD_CBRET_OK;
}

static Sint32 BupProgress_8c014f04(Sint32 drive, Sint32 op, Sint32 count, Sint32 max)
{
    BACKUPINFO* info;

    info = &var_gBupInfo_8c1bc4ac[drive];

    info->ProgressCount = count;
    info->ProgressMax = max;
    info->Operation = op;

    return BUD_CBRET_OK;
}
