/* @unit Snd */
#include <shinobi.h>
#include <sg_sd.h>
#include <cri_adxt.h>
#include "0100bc_sound.h"
#include "sectionB.h"
#include <cri_adxf.h>
#include <string.h>
#include "serial_debug.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define WKSIZE 184516
// #define	MAX_NFILES (256)
#define	MAX_NFILES (1424)


/* =================
 * Type Declarations
 * =================
 */

typedef struct {
    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int field_0x10;
    int field_0x14;
    float field_0x18;
    float field_0x1c;
    float field_0x20;
} UnknownVolStructA;

typedef struct {
    float field_0x00;
} UnknownVolStructB;

typedef struct {
    char* fname_0x00;
    int nfile_0x04;
} AdxfPartitionInfo;

/* Per-channel volume */
typedef struct {
    int vol0_0x00;
    int vol1_0x04;
} AdxVolumes;

typedef struct {
    /* bit0: ch0 fade-out
     * bit1: ch1 fade-out
     * bit4: ch0 fade-in
     * bit5: ch1 fade-in */
    int fadeFlags_0x00;
    int step0_0x04;
    int step1_0x08;
    int curVol0_0x0c;
    int curVol1_0x10;
} AdxFadeState;


/* =====================
 * External Declarations
 * =====================
 */

/* =======================
 * Non-initialized Globals
 * =======================
 */

ADXT var_adxtHandles_8c0fcd20[2];
SDMIDI var_midiHandles_8c0fcd28[8];
void* var_memblkSource_8c0fcd48;
void* var_memblkSource_8c0fcd4c;
UnknownVolStructA var_uknVol_8c0fcd50;

char var_work_8c0fcd74[WKSIZE * 2];
char var_adxfWork_8c156efc[ADXF_CALC_PTINFO_SIZE(MAX_NFILES)];

AdxFadeState var_adxFade_8c157a34;

/* ===================
 * Initialized Globals
   ===================
 */

int init_8c03bd80 = 0; // Maybe soundIsPlaying or soundIsPlayingForAdx
int init_8c03bd84 = 1; // Maybe soundIsPlayingForMidi
AdxVolumes init_adxVolumes_8c03bd88 = {
    990,
    990
};
int init_vol_8c03bd90 = 127;
AdxfPartitionInfo init_adxfPartitionInfo_8c03bd94[] = {
    { "bgm.afs", 0x50 },
    { "voice.afs", 0x52e },
    { "", 0 },
};

/* === Prototypes === */
void SndMidiResetFxAndPlay_8c010846(int hld_idx, int data_num);
void FUN_8c0109c0();
void FUN_8c010ca6(Bool p1);
int SndProc_8c010cd6(int p1, int p2);

/* Tested */
STATIC void initUknVol_8c0100bc() {
    var_uknVol_8c0fcd50.field_0x18 = (float) init_vol_8c03bd90 / 2600;
    var_uknVol_8c0fcd50.field_0x1c = var_uknVol_8c0fcd50.field_0x18 * 2600 / 3000;
    var_uknVol_8c0fcd50.field_0x14 = init_vol_8c03bd90 * 30 / 100;
    var_uknVol_8c0fcd50.field_0x08 = init_vol_8c03bd90 * 40 / 100;
    var_uknVol_8c0fcd50.field_0x0c = var_uknVol_8c0fcd50.field_0x18 * 3000;
    var_uknVol_8c0fcd50.field_0x20 = (float) init_vol_8c03bd90 / 3900;
}

/* Tested */
STATIC void midiSetVol_8c010128() {
    int r13var_uknVol_8c226468_as_int = var_uknVol_8c226468;

    if ((var_uknVol_8c0fcd50.field_0x00 & 2) == 2) {
        /* A */
        if (r13var_uknVol_8c226468_as_int >= 10.f && r13var_uknVol_8c226468_as_int < 3000.f) {
            /* B */
            sdMidiSetVol(
                var_midiHandles_8c0fcd28[7],
                var_uknVol_8c0fcd50.field_0x08 + (r13var_uknVol_8c226468_as_int - 10.f) * var_uknVol_8c0fcd50.field_0x18 - 127,
                0
            );
        /* 8c010192 */
        } else if (r13var_uknVol_8c226468_as_int >= 3000.f) {
            /* C */
            /* 8c01019a */
            sdMidiSetVol(
                var_midiHandles_8c0fcd28[7],
                var_uknVol_8c0fcd50.field_0x0c - (r13var_uknVol_8c226468_as_int - 3000) * var_uknVol_8c0fcd50.field_0x1c - 127,
                0
            );
        }
    }

    /* LAB_8c0101bc */
    if ((var_uknVol_8c0fcd50.field_0x00 & 4) == 4) {
        /* D */
        sdMidiSetVol(
            var_midiHandles_8c0fcd28[6],
            (r13var_uknVol_8c226468_as_int - 1000.f) * var_uknVol_8c0fcd50.field_0x20 - 127,
            0
        );

        if (r13var_uknVol_8c226468_as_int < 2100) {
            /* E */
            /* 8c0101e2 */
            var_uknVol_8c0fcd50.field_0x00 &= ~4;

            sdMidiSetVol(
                var_midiHandles_8c0fcd28[6],
                -127,
                0
            );
        }
    }
}

/* Matched */
STATIC void midiSetPitch_8c01023c()
{
    int var_uknVol_8c226468_as_int = var_uknVol_8c226468;

    if (var_uknVol_8c226468_as_int > 10.f && var_uknVol_8c226468_as_int < 3000.f) {
        sdMidiSetPitch(
            var_midiHandles_8c0fcd28[6],
            (var_uknVol_8c226468_as_int - 10.f) * 0.05,
            0
        );
        sdMidiSetPitch(
            var_midiHandles_8c0fcd28[7],
            (var_uknVol_8c226468_as_int - 10.f) * 0.05,
            0
        );
    } else if (var_uknVol_8c226468_as_int >= 3000.f && var_uknVol_8c226468_as_int < 4000.f) {
        sdMidiSetPitch(
            var_midiHandles_8c0fcd28[6],
            (var_uknVol_8c226468_as_int - 10.f) * 0.05,
            0
        );
        sdMidiSetPitch(
            var_midiHandles_8c0fcd28[7],
            (var_uknVol_8c226468_as_int - 10.f) * 0.05,
            0
        );
    }
}

/* Tested */
STATIC void FUN_8c0102d8()
{
    int bcb0 = var_8c1bbcb0;
    int var_uknVol_8c226468_as_int = var_uknVol_8c226468;

    if ((var_uknVol_8c0fcd50.field_0x00 & 1) != 1 && bcb0 == 1) {
        /* A */
        /* 8c010312 */
        sdMidiSetPitch(var_midiHandles_8c0fcd28[6], -200, 0);
        sdMidiSetVol(var_midiHandles_8c0fcd28[6], var_uknVol_8c0fcd50.field_0x14 - 127, 0);
        sdMidiPlay(var_midiHandles_8c0fcd28[6], 1, 43, 0);

        // var_uknVol_8c0fcd50.field_0x00 = 0;
        var_uknVol_8c0fcd50.field_0x00 = 0 | 1;
        // }
    } else if ((var_uknVol_8c0fcd50.field_0x00 & 1) == 1 && bcb0 != 1) {
        /* C */
        /* 8c010378 */
        sdMidiSetVol(var_midiHandles_8c0fcd28[6], -127, 2000);
        var_uknVol_8c0fcd50.field_0x00 &= (char) 0xfe;
    }

    // var_uknVol_8c0fcd50_field_0x00_temp = var_uknVol_8c0fcd50.field_0x00 & 2;
    /* 8c010388 */
    if ((var_uknVol_8c0fcd50.field_0x00 & 2) != 2 && var_uknVol_8c226468_as_int >= 400.f) {
        /* D */
        sdMidiSetPitch(var_midiHandles_8c0fcd28[7], 0, 0);
        sdMidiSetVol(var_midiHandles_8c0fcd28[7], -127, 0);
        sdMidiPlay(var_midiHandles_8c0fcd28[7], 1, 44, 0);

        var_uknVol_8c0fcd50.field_0x00 |= 2;
    } else if ((var_uknVol_8c0fcd50.field_0x00 & 2) == 2 && var_uknVol_8c226468_as_int < 400.f) {
        /* F */
        var_uknVol_8c0fcd50.field_0x00 &= (char) 0xfd;
    }

    // var_uknVol_8c0fcd50_field_0x00_temp = var_uknVol_8c0fcd50.field_0x00 & 4;
    /* 8c0103de */
    if ((var_uknVol_8c0fcd50.field_0x00 & 4) != 4 && var_uknVol_8c226468_as_int >= 2100.f) {
        /* G */
        sdMidiSetPitch(var_midiHandles_8c0fcd28[6], 0, 0);
        sdMidiSetVol(var_midiHandles_8c0fcd28[6], -127, 0);
        sdMidiPlay(var_midiHandles_8c0fcd28[6], 1, 45, 0);

        var_uknVol_8c0fcd50.field_0x00 |= 4;
    }
}

/* Matched */
STATIC void createAdxHandles_8c010428()
{
    Sint8 i;
    for (i = 0; i < 2; i++)
    {
        var_adxtHandles_8c0fcd20[i] = ADXT_Create(2, &var_work_8c0fcd74[i * WKSIZE], WKSIZE);
    }
}

/* Matched */
STATIC void createMidiHandles_8c010468()
{
    int i;
    for (i = 0; i < 8; i++)
    {
        sdMidiOpenPort(&var_midiHandles_8c0fcd28[i]);
    }
}

/* Matched */
STATIC void createAdxAndMidiHandles_8c01048e()
{
    createAdxHandles_8c010428();
    createMidiHandles_8c010468();
}

/* Matched */
STATIC Sint32 unused_8c0104bc(Sint32 fsize)
{
    fsize += 2047;
    return (fsize / 2048) * 2048;
}

/* Matched */
STATIC void* unusedReadFile_8c0104d6(char* fname)
{
    void* dat;
    Sint32 fsize, nsct;
    GDFS gdfs = gdFsOpen(fname, 0);

    gdFsGetFileSize(gdfs, &fsize);

    nsct = (fsize+2047)/2048;
    if ( (dat=syMalloc(nsct*2048)) != NULL ) {
        gdFsRead(gdfs, nsct, dat);
    }
    gdFsClose(gdfs);

    return dat;
}

/* Matched */
STATIC void adxErrFunc_8c010532(void *obj, char *msg)
{
    char lmsg[9] = "E8101214";

    if (strncmp(msg, lmsg, strlen(lmsg)) == 0)
    {
        FUN_8c010ca6(0);
        FUN_8c010ca6(1);

        init_8c03bd80 = 0;
        init_8c03bd84 = 0;
    }
}

/* Matched */
STATIC void adxLoad_8c01057a()
{
    int j = 0;
    Sint8 i = 0;

    while (init_adxfPartitionInfo_8c03bd94[i].nfile_0x04 != 0)
    {
        ADXF_LoadPartition(
            i,
            init_adxfPartitionInfo_8c03bd94[i].fname_0x00,
            &var_adxfWork_8c156efc[j],
            init_adxfPartitionInfo_8c03bd94[i].nfile_0x04
        );
        j += ADXF_GetPtinfoSize(i);
        i++;
    }
}

/* Matched */
STATIC void finishSoundInit_8c010614()
{
    SDMEMBLK memblk;

    sdMemBlkCreate(&memblk);
    sdMemBlkSetPrm(memblk, var_memblkSource_8c0fcd4c, 0x119d00, SDD_MEMBLK_SYNC_FUNC, NULL);
    sdMultiUnitDownload(memblk);
    syFree(var_memblkSource_8c0fcd4c);
    sdMemBlkDestroy(memblk);
}

/* Matched */
STATIC void adxInit_8c01064c()
{
    ADXT_Init();
    ADXT_EntryErrFunc(adxErrFunc_8c010532, NULL);
}

/* Matched */
STATIC void soundInit_8c01065e()
{
    SDMEMBLK memblk = NULL;

    sdLibInit(NULL, 0, 0);
    sdMemBlkCreate(&memblk);
    sdMemBlkSetPrm(memblk, var_memblkSource_8c0fcd48, 0, SDD_MEMBLK_SYNC_FUNC, NULL);
    sdDrvInit(memblk);
    sdMemBlkDestroy(memblk);
    syFree(var_memblkSource_8c0fcd48);
    sdMemBlkSetTransferMode(SDE_MEMBLK_TRANSFER_MODE_DMA);
}

/* Matched */
void FUN_8c0106ac()
{
    int s = ADXT_GetStat(var_adxtHandles_8c0fcd20[1]);
    if (s == ADXT_STAT_STOP || s == ADXT_STAT_PLAYEND) {
        init_8c03bd80 &= 0xffffffef;
    }
}

/* Matched */
Bool FUN_8c0106d2(Sint32 param)
{
    if (param >= 7) {
        param += 2;
    }

    if (param >= 0 && param < 10) {
        SndMidiResetFxAndPlay_8c010846(0, param);
        return TRUE;
    } else {
        if (param >= 10 && param <= 70) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, param - 10, 0);
            return TRUE;
        }

        return FALSE;
    }
}

/* Matched */
Bool FUN_8c010720(Sint32 param)
{
    if (param >= 0 && param < 63) {
        SndProc_8c010cd6(1, param + 17);
        return TRUE;
    } else {
        if (param >= 63 && param <= 1388) {
            SndProc_8c010cd6(2, param - 63);
            return TRUE;
        }

        return FALSE;
    }
}

/* Matched */
int FUN_8c0107ac(Sint32 param)
{
    if (param >= 0 && param < 17) {
        SndProc_8c010cd6(0, param);
        return 1;
    }
    return 0;
}

/* Matched */
void SndControlAdxtWithOutVol_8c0107d2(Bool play)
{
    Uint32 i;

    if (play == TRUE) {
        ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], -990);
        ADXT_SetOutVol(var_adxtHandles_8c0fcd20[1], -990);

        for (i = 0; i < 8; i++)
            sdMidiPause(var_midiHandles_8c0fcd28[i]);
    } else if (play == FALSE) {
        ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], -990 + init_adxVolumes_8c03bd88.vol0_0x00);
        ADXT_SetOutVol(var_adxtHandles_8c0fcd20[1], -990 + init_adxVolumes_8c03bd88.vol1_0x04);

        for (i = 0; i < 8; i++)
            sdMidiContinue(var_midiHandles_8c0fcd28[i]);
    }
}

/* Matched */
void SndMidiResetFxAndPlay_8c010846(int hld_idx, int data_num)
{
    Sint8 i;

    sdSndSetFxPrg(0, 0);

    for (i = 0; i < 6; i++)
        sdMidiSetFxLev(var_midiHandles_8c0fcd28[i], 0);

    sdMidiPlay(var_midiHandles_8c0fcd28[hld_idx], 0, data_num, 0);
}

/* Matched
 * Re-check after param type change
 */
Bool SndSetSoundMode_8c0108c0(Sint32 mode)
{
    void *dat;
    int r;

    if (mode == SYD_CFG_STEREO) {
        sdSndSetPanMode(SDE_PAN_MODE_ENABLE);
    } else if (mode == SYD_CFG_MONO) {
        sdSndSetPanMode(SDE_PAN_MODE_DISABLE);
    }

    dat = syMalloc(0x4000);
    r = syCfgInit(dat);
    if (r != SYD_CFG_OK) {
        syFree(dat);
        return FALSE;
    }

    r = syCfgSetSoundMode(mode);
    if (r != SYD_CFG_OK) {
        syFree(dat);
        return FALSE;
    }

    syCfgExit();
    syFree(dat);

    return TRUE;
}

/* Matched */
int SndGetSoundMode_8c010924() {
    void* dat;
    int r;
    Sint32 mode;

    dat = syMalloc(0x4000);
    r = syCfgInit(dat);
    if (r != SYD_CFG_OK) {
        syFree(dat);
        return -1;
    }

    r = syCfgGetSoundMode(&mode);
    if (r != SYD_CFG_OK) {
        syFree(dat);
        return -1;
    }

    syCfgExit();
    syFree(dat);

    return mode;
}

/* Tested */
void SndSetAdxVol_8c010972(int volNo, int handle) {
    // Initialized data
    const int vols_from_8c0332b0[10] = {
        0,
        110,
        220,
        330,
        440,
        550,
        660,
        770,
        880,
        990
    };

    switch (handle)
    {
        case 0: {
            init_adxVolumes_8c03bd88.vol0_0x00 = vols_from_8c0332b0[volNo];
            ADXT_SetOutVol(var_adxtHandles_8c0fcd20[handle], init_adxVolumes_8c03bd88.vol0_0x00 - 990);
            break;
        }
        case 1: {
            init_adxVolumes_8c03bd88.vol1_0x04 = vols_from_8c0332b0[volNo];
            ADXT_SetOutVol(var_adxtHandles_8c0fcd20[handle], init_adxVolumes_8c03bd88.vol1_0x04 - 990);
            break;
        }
    }
}

/* Matched */
void SndSetMidiVolAndInitStruct_8c0109f4(int param1) {
    // Initialized data
    int i;
    int vols_8c0332d8[10] = {
        0,
        25,
        51,
        76,
        102,
        127,
        153,
        178,
        204,
        229
    };

    init_vol_8c03bd90 = vols_8c0332d8[param1];

    for (i = 0; i < 8; i++)
        sdMidiSetVol(var_midiHandles_8c0fcd28[i], init_vol_8c03bd90 - 127, 0);

    initUknVol_8c0100bc();
}

/* Tested */
void SndUpdateAdxVolFade_8c010a40() {
    /* 8c010a56 */
    if ((var_adxFade_8c157a34.fadeFlags_0x00 & 0xf) != 0)
    {
        /* 8c010a5c */
        if ((var_adxFade_8c157a34.fadeFlags_0x00 & 1) == 1)
        {
            /* A */
            var_adxFade_8c157a34.curVol0_0x0c -= var_adxFade_8c157a34.step0_0x04;
            ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], var_adxFade_8c157a34.curVol0_0x0c);
        }

        /* 8c010a70 */
        if ((var_adxFade_8c157a34.fadeFlags_0x00 & 2) == 2)
        {
            /* B */
            var_adxFade_8c157a34.curVol1_0x10 -= var_adxFade_8c157a34.step1_0x08;
            ADXT_SetOutVol(var_adxtHandles_8c0fcd20[1], var_adxFade_8c157a34.curVol1_0x10);
        }

        /* 8c010a86 */
        if (
            (var_adxFade_8c157a34.curVol0_0x0c < -300)
            && (var_adxFade_8c157a34.fadeFlags_0x00 & 1) == 1
        ) 
        {
            /* C */
            ADXT_Stop(var_adxtHandles_8c0fcd20[0]);
            ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], init_adxVolumes_8c03bd88.vol0_0x00 - 990);

            var_adxFade_8c157a34.fadeFlags_0x00 &= ~1;
            init_8c03bd80 &= ~1;
        }

        /* 8c010abe */
        if (
            (var_adxFade_8c157a34.curVol1_0x10 < -300)
            && (var_adxFade_8c157a34.fadeFlags_0x00 & 2) == 2
        )
        {
            /* D */
            ADXT_Stop(var_adxtHandles_8c0fcd20[1]);
            ADXT_SetOutVol(var_adxtHandles_8c0fcd20[1], init_adxVolumes_8c03bd88.vol1_0x04 - 990);
            var_adxFade_8c157a34.fadeFlags_0x00 &= ~2;
            init_8c03bd80 &= ~(1 << 4);
        }
    }
    else
    {
        /* 8c010b38 */
        if ((var_adxFade_8c157a34.fadeFlags_0x00 & 0xf0) != 0) {
            if ((var_adxFade_8c157a34.fadeFlags_0x00 & 0x10) == 0x10) {
                /* E */
                var_adxFade_8c157a34.curVol0_0x0c += var_adxFade_8c157a34.step0_0x04;
                ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], var_adxFade_8c157a34.curVol0_0x0c);
            }

            /* 8c010b4e */
            if ((var_adxFade_8c157a34.fadeFlags_0x00 & 0x20) == 0x20) {
                /* F */
                var_adxFade_8c157a34.curVol1_0x10 += var_adxFade_8c157a34.step1_0x08;
                ADXT_SetOutVol(var_adxtHandles_8c0fcd20[1], var_adxFade_8c157a34.curVol1_0x10);
            }

            /* 8c010b64 */
            if (
                (var_adxFade_8c157a34.curVol0_0x0c > init_adxVolumes_8c03bd88.vol0_0x00)
                && ((var_adxFade_8c157a34.fadeFlags_0x00 & 0x10) == 0x10)
            ) {
                /* G */
                ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], init_adxVolumes_8c03bd88.vol0_0x00);
                var_adxFade_8c157a34.fadeFlags_0x00 &= 0xffffffef;
            }

            /* 8c010b82 */
            if (
                (var_adxFade_8c157a34.curVol1_0x10 > init_adxVolumes_8c03bd88.vol1_0x04)
                && ((var_adxFade_8c157a34.fadeFlags_0x00 & 0x20) == 0x20)
            ) {
                /* H */
                ADXT_SetOutVol(var_adxtHandles_8c0fcd20[1], init_adxVolumes_8c03bd88.vol1_0x04);
                var_adxFade_8c157a34.fadeFlags_0x00 &= 0xffffffdf;
            }
        }
    }
}

/* Tested */
void SndStartAdxFadeOut_8c010bae(int param1) {
    if ((var_adxFade_8c157a34.fadeFlags_0x00 & 0xf0) == 0) {
        /* 8c010bba */
        if (param1 == 0 && (var_adxFade_8c157a34.fadeFlags_0x00 & 0xf) != 1) {
            /* A */
            var_adxFade_8c157a34.fadeFlags_0x00 |= 1;
            var_adxFade_8c157a34.step0_0x04 = (init_adxVolumes_8c03bd88.vol0_0x00 - 300) / 90;
            var_adxFade_8c157a34.curVol0_0x0c = init_adxVolumes_8c03bd88.vol0_0x00 - 990;
        }
        if (param1 == 1 && (var_adxFade_8c157a34.fadeFlags_0x00 & 0xf) != 2) {
            /* B */
            var_adxFade_8c157a34.fadeFlags_0x00 |= 2;
            var_adxFade_8c157a34.step1_0x08 = (init_adxVolumes_8c03bd88.vol1_0x04 - 300) / 90;
            var_adxFade_8c157a34.curVol1_0x10 = init_adxVolumes_8c03bd88.vol1_0x04 - 990;
        }
    }
}

/* Tested */
STATIC void startAdxCh1FadeIn_8c010c2c(Bool param1) {
    if ((var_adxFade_8c157a34.fadeFlags_0x00 & 0xf) == 0
        && param1 == 1
        && (var_adxFade_8c157a34.fadeFlags_0x00 & 0xf0) != 0x20)
    {
        var_adxFade_8c157a34.fadeFlags_0x00 |= 0x20; 
        var_adxFade_8c157a34.step1_0x08 = init_adxVolumes_8c03bd88.vol1_0x04 / 90;
        var_adxFade_8c157a34.curVol1_0x10 = -990;
        init_8c03bd80 &= 0xffffffef;
    }
}

/* Matched */
void FUN_8c010c6e() {
    midiSetVol_8c010128();
    midiSetPitch_8c01023c();
    FUN_8c0102d8();
}

/* Matched */
void FUN_8c010c7c() {
    int i;
    for (i=0; i<2; i++) {
        ADXT_Stop(var_adxtHandles_8c0fcd20[i]);
    }

    init_8c03bd80 = 0;
    return;
}

/* Matched */
void FUN_8c010ca6(Bool p1) {
    if (p1 == 0) {
        init_8c03bd80 &= 0xfffffffe;
    } else if (p1 == 1) {
        init_8c03bd80 &= 0xffffffef;
    }

    ADXT_Stop(var_adxtHandles_8c0fcd20[p1]);
    return;
}

/* Tested */
int SndProc_8c010cd6(int p1, int p2) {
    switch (p1) {
        case 0: {
            ADXT_Stop(var_adxtHandles_8c0fcd20[p1]);
            ADXT_StartAfs(var_adxtHandles_8c0fcd20[p1], 0, p2);
            init_8c03bd80 |= 1;
            return 1;
        }

        case 1: {
            ADXT_Stop(var_adxtHandles_8c0fcd20[p1]);
            ADXT_StartAfs(var_adxtHandles_8c0fcd20[p1], 0, p2);
            init_8c03bd80 |= 0x10;
            return 1;
        }

        case 2: {
            ADXT_Stop(var_adxtHandles_8c0fcd20[1]);
            ADXT_StartAfs(var_adxtHandles_8c0fcd20[1], 1, p2);
            init_8c03bd80 |= 0x10;
            return 1;
        }
    }

    return 0;
}

/* Matched */
void FUN_8c010d8a() {
    ADXT_Stop(var_adxtHandles_8c0fcd20[0]);
    ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], init_adxVolumes_8c03bd88.vol0_0x00 - 990);
    var_adxFade_8c157a34.fadeFlags_0x00 &= 0xfffffffe;
    init_8c03bd80 &= 0xfffffffe;

    ADXT_Stop(var_adxtHandles_8c0fcd20[0]);
    ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], init_adxVolumes_8c03bd88.vol0_0x00 - 990);
    var_adxFade_8c157a34.fadeFlags_0x00 &= 0xfffffffe;
    init_8c03bd80 &= 0xfffffffe;
}

/* Matched */
STATIC void unusedStopMidiAdx_8c010de6() {
    sdMidiStopAll();
    ADXT_Stop(var_adxtHandles_8c0fcd20[0]);
    ADXT_Stop(var_adxtHandles_8c0fcd20[1]);
    init_8c03bd80 = 0;
    init_8c03bd84 = 1;
    memset(&var_adxFade_8c157a34, 0, sizeof(AdxFadeState));
    initUknVol_8c0100bc();
}

/* Matched */
void SndInitSoundMidiAdx_8c010e18(char *dirname) {
    init_8c03bd80 = 0;
    init_8c03bd84 = 1;
    gdFsChangeDir(dirname);
    memset(&var_adxFade_8c157a34, 0, sizeof(AdxFadeState));
    initUknVol_8c0100bc();
    init_8c03bd80 |= 0x11;
    soundInit_8c01065e();
    adxInit_8c01064c();
    createAdxAndMidiHandles_8c01048e();
    adxLoad_8c01057a();
    finishSoundInit_8c010614();
    init_8c03bd80 = 0;
}

