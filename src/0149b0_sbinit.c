/* 8c0149b0 */
/*
 * Adjusted sbinit from SDK 155j
 */

#include <shinobi.h>
#include <sg_syhw.h>
#include "serial_debug.h"
#include "0149b0_sbinit.h"

#define WORK_TOP 0x8c000000
#define WORK_END 0x8d000000

#define P1AREA   0x80000000

extern Uint8* _BSG_END;

/***** Do you use GD filesystem ?                                     */
#define USE_GDFS 1        /* 0...Do not use.                          */
                          /* 1...Use (default)                        */

/* Do you assign an end of B Section to the top op the heap area ?    */
#define USE_B_END 1       /* 0...Do not assign.                       */
                          /* 1...Assign (default)                     */

/* When you do not assign an end of B section to the top of the heap  */
/* area, please define the size of heap area in HEAP_SIZE macro.      */

/* GD:Maximum file number opening at the same time.                   */
#define FILES 8

/* GD:Current directory buffer                                        */
#define BUFFERS 2048

/* The total volume that can be secured with syMalloc() (about 4MB)   */
/* (This is effective whed does not assign an end of B section to the */
/* top of the heap area.                                              */
#if !USE_B_END
#define HEAP_SIZE 0x00400000
#endif



/* Global work area                                                   */

STATIC Uint8 gMapleRecvBuf[1024 * 24 * 2 + 32];
STATIC Uint8 gMapleSendBuf[1024 * 24 * 2 + 32];

#if USE_GDFS
STATIC Uint8 gdfswork[GDFS_WORK_SIZE(FILES) + 32];
STATIC Uint8 gdfscurdir[GDFS_DIRREC_SIZE(BUFFERS) + 32];
#endif

/* The top address of the heap area */
#if USE_B_END
#define HEAP_AREA ((void*)((((Uint32)_BSG_END | P1AREA) & 0xffffffe0) + 0x20))
#define HEAP_SIZE (WORK_END - (Uint32)HEAP_AREA)
#else
#define HEAP_AREA ((void*)((Uint32)WORK_END - (Uint32)HEAP_SIZE))
#endif


/*
** Initialization function of the application.
** Arguments are compatible with njInitSystem().
*/
void sbInitSystem_8c0149b0(Int mode, Int frame, Int count)
{
    /* Do standard initialization.                                */
    /* By this setting, many applications can get the greatest    */
    /* performance.                                               */
    /* Without the special reason, please do not change a         */
    /* desctiption of this place.                                 */
    /* When change it, please understand specification of each    */
    /* library enough, and revise it in responsibility of library */
    /* user.                                                      */

    /* Disable interrupt. */
    set_imask(15);

    /* Initialize hardware.  */
    syHwInit();

    /* Initialize memory management. */
    syMallocInit(HEAP_AREA, HEAP_SIZE);

    /* Initialize Ninja/Kamui. */
    njInitSystem(mode, frame, count);

    /* Initialize hardware 2.     */
    syHwInit2();

    /* Initialize controller library. */
    pdInitPeripheral(PDD_PLOGIC_ACTIVE, gMapleRecvBuf, gMapleSendBuf);

    /* Enable interrupt. */
    set_imask(0);

#if USE_GDFS
    /* Initialize GD filesystem.                                */
    {
        Uint8* wk;
        Uint8* dir;

        wk  = (Uint8*)(((Uint32)gdfswork & 0xffffffe0) + 0x20);
        dir = (Uint8*)(((Uint32)gdfscurdir & 0xffffffe0) + 0x20);
        gdFsInit(FILES, wk, BUFFERS, dir);
    }
#endif

    /* Other initializations.                                   */
    /* When there are initializations of the other libraries    */
    /* which there are not and the initializations that should  */
    /* be done by user aloft, please describe it here.          */
}


/*
** Finalize application.
*/

void sbExitSystem_8c014a24(void)
{
    /* Finalize controller library.     */
    pdExitPeripheral();

    /* Finalize Ninja/Kamui.       */
    njExitSystem();

    /* Finalize memory management. */
    syMallocFinish();

    /* Finalize hardware.     */
    syHwFinish();

    /* Disable interrupt. */
    set_imask(15);

    syBtExit();
}
