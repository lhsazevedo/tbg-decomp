/*
 *  Adjusted backup sample from SDK 155j
 */

#ifndef _BACKUP_H_
#define _BACKUP_H_

#include "sg_bup.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
	Uint16 Ready;
	Uint16 IsFormat;
	Uint32 LastError;
	Uint32 ProgressCount;
	Uint32 ProgressMax;
	Uint32 Operation;
	BUS_DISKINFO DiskInfo;
	Uint32 Connect;
	void* Work;
	Uint32 WorkSize;
	Uint32 Capacity;
} BACKUPINFO;


void BupInit_8c014b8c(void);
void BupExit(void);
const BACKUPINFO* BupGetInfo_8c014bba(Sint32 drive);
Sint32 BupLoad_8c014bc6(Sint32 drive, const char* fname, void* buf);
Sint32 BupSave_8c014bcc(Sint32 drive, const char* fname, void* buf, Sint32 nblock);
void BupMount_8c014c00(Sint32 drive);
void BupUnmount_8c014c46(Sint32 drive);
void BupClearInfo_8c014c8a(Sint32 drive);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _BACKUP_H_ */
