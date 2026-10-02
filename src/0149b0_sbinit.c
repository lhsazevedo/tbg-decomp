/* @unit Sb */
/* 8c0149b0 */
/*
 * Adjusted sbinit from SDK 155j
 */

#include <shinobi.h>
#include <sg_syhw.h>
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "0149b0_sbinit.h"

#define WORK_TOP 0x8c000000
#define WORK_END 0x8d000000

#define P1AREA   0x80000000

extern Uint8* _BSG_END;

#define USE_GDFS 1

/* Heap starts at the end of section B; else HEAP_SIZE below. */
#define USE_B_END 1

/* GD: max files open at once. */
#define FILES 8

/* GD: current directory buffers. */
#define BUFFERS 2048

#if !USE_B_END
#define HEAP_SIZE 0x00400000
#endif


STATIC Uint8 gMapleRecvBuf[1024 * 24 * 2 + 32];
STATIC Uint8 gMapleSendBuf[1024 * 24 * 2 + 32];

#if USE_GDFS
STATIC Uint8 gdfswork[GDFS_WORK_SIZE(FILES) + 32];
STATIC Uint8 gdfscurdir[GDFS_DIRREC_SIZE(BUFFERS) + 32];
#endif

#if USE_B_END
#define HEAP_AREA ((void*)((((Uint32)_BSG_END | P1AREA) & 0xffffffe0) + 0x20))
#define HEAP_SIZE (WORK_END - (Uint32)HEAP_AREA)
#else
#define HEAP_AREA ((void*)((Uint32)WORK_END - (Uint32)HEAP_SIZE))
#endif


/* Arguments are njInitSystem()'s. */
void SbInitSystem_8c0149b0(Int mode, Int frame, Int count)
{
    set_imask(15);
    syHwInit();
    syMallocInit(HEAP_AREA, HEAP_SIZE);
    njInitSystem(mode, frame, count);
    syHwInit2();
    pdInitPeripheral(PDD_PLOGIC_ACTIVE, gMapleRecvBuf, gMapleSendBuf);
    set_imask(0);

#if USE_GDFS
    {
        Uint8* wk;
        Uint8* dir;

        wk  = (Uint8*)(((Uint32)gdfswork & 0xffffffe0) + 0x20);
        dir = (Uint8*)(((Uint32)gdfscurdir & 0xffffffe0) + 0x20);
        gdFsInit(FILES, wk, BUFFERS, dir);
    }
#endif
}

void SbExitSystem_8c014a24(void)
{
    pdExitPeripheral();
    njExitSystem();
    syMallocFinish();
    syHwFinish();
    set_imask(15);
    syBtExit();
}
