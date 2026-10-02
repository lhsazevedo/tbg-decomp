/* @unit Snd */
#include <shinobi.h>
#include <sg_sd.h>
#include <cri_adxt.h>
#include "0100bc_sound.h"
#include "1ba1c8_globals.h"
#include "01fa78_hud.h" /* var_hudState_8c22643c */
#include <cri_adxf.h>
#include <string.h>
#include "includes.h" /* STATIC */
#include "serial_debug.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define WKSIZE 184516
#define	MAX_NFILES (1424)

/* init_adxPlaying_8c03bd80 bits, one per ADXT handle. */
#define SND_ADX_BGM   (1 << 0)
#define SND_ADX_VOICE (1 << 4)


/* =================
 * Type Declarations
 * =================
 */

typedef struct {
    /* Which engine MIDI layers are up: bit0 idle (port 6, seq 43), bit1
     * running (port 7, seq 44), bit2 high-rev (port 6, seq 45) -- bits 0
     * and 2 share port 6, so the later sdMidiPlay displaces the earlier. */
    int activeFlags_0x00;
    int field_0x04;  /* never read or written anywhere in the image */
    int midRpmVolBase_0x08;
    int highRpmVolBase_0x0c;
    int field_0x10;  /* same */
    int idleVol_0x14;
    float midRpmSlope_0x18;
    float highRpmSlope_0x1c;
    float highRevSlope_0x20;
} EngineSoundState;

typedef struct {
    char* fname_0x00;
    int nfile_0x04;
} AdxfPartitionInfo;

/* Per-handle level, 0-990. */
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


/* =======================
 * Non-initialized Globals
 * =======================
 */

/* [0] carries music, [1] the spoken announcements and dialogue. */
ADXT var_adxtHandles_8c0fcd20[2];
SDMIDI var_midiHandles_8c0fcd28[8];
/* manatee.drv and bus.mlt; freed once downloaded to the AICA. */
void* var_sndDrvData_8c0fcd48;
void* var_sndBankData_8c0fcd4c;
EngineSoundState var_engineSound_8c0fcd50;

char var_adxtWork_8c0fcd74[WKSIZE * 2];
char var_adxfWork_8c156efc[ADXF_CALC_PTINFO_SIZE(MAX_NFILES)];

AdxFadeState var_adxFade_8c157a34;

/* ===================
 * Initialized Globals
 * ===================
 */

/* SND_ADX_* bitset of the streams playing. Stored as 1 with
 * init_soundOk_8c03bd84 at 0, it tells GameMain_8c01392e to quit. */
int init_adxPlaying_8c03bd80 = 0;
/* Cleared when the ADX layer gives up on the disc. */
int init_soundOk_8c03bd84 = 1;
AdxVolumes init_adxVolumes_8c03bd88 = {
    990,
    990
};
int init_midiVol_8c03bd90 = 127;
/* ADXF partition table, terminated by a zero nfile_0x04. bgm.afs holds 80
 * streams -- 0-16 music, 17-79 announcements -- and voice.afs the 1326
 * dialogue lines. */
AdxfPartitionInfo init_adxfPartitionInfo_8c03bd94[] = {
    { "bgm.afs", 0x50 },
    { "voice.afs", 0x52e },
    { "", 0 },
};

/* ====================
 * Forward Declarations
 * ====================
 */

void SndMidiResetFxAndPlay_8c010846(int hld_idx, int data_num);
void SndStopAdx_8c010ca6(Bool handle);
int SndPlayAdx_8c010cd6(int source, int fileNo);

/* =========
 * Functions
 * =========
 */

STATIC void initEngineVol_8c0100bc() {
    var_engineSound_8c0fcd50.midRpmSlope_0x18 = (float) init_midiVol_8c03bd90 / 2600;
    var_engineSound_8c0fcd50.highRpmSlope_0x1c = var_engineSound_8c0fcd50.midRpmSlope_0x18 * 2600 / 3000;
    var_engineSound_8c0fcd50.idleVol_0x14 = init_midiVol_8c03bd90 * 30 / 100;
    var_engineSound_8c0fcd50.midRpmVolBase_0x08 = init_midiVol_8c03bd90 * 40 / 100;
    var_engineSound_8c0fcd50.highRpmVolBase_0x0c = var_engineSound_8c0fcd50.midRpmSlope_0x18 * 3000;
    var_engineSound_8c0fcd50.highRevSlope_0x20 = (float) init_midiVol_8c03bd90 / 3900;
}

/* sdMidiSetVol takes a Sint8, hence the -127 on every level below. */
STATIC void updateEngineVol_8c010128() {
    int rpm = var_hudState_8c22643c.engineRpm_0x2c;

    if ((var_engineSound_8c0fcd50.activeFlags_0x00 & 2) == 2) {
        if (rpm >= 10.f && rpm < 3000.f) {
            sdMidiSetVol(
                var_midiHandles_8c0fcd28[7],
                var_engineSound_8c0fcd50.midRpmVolBase_0x08 + (rpm - 10.f) * var_engineSound_8c0fcd50.midRpmSlope_0x18 - 127,
                0
            );
        } else if (rpm >= 3000.f) {
            sdMidiSetVol(
                var_midiHandles_8c0fcd28[7],
                var_engineSound_8c0fcd50.highRpmVolBase_0x0c - (rpm - 3000) * var_engineSound_8c0fcd50.highRpmSlope_0x1c - 127,
                0
            );
        }
    }
    if ((var_engineSound_8c0fcd50.activeFlags_0x00 & 4) == 4) {
        sdMidiSetVol(
            var_midiHandles_8c0fcd28[6],
            (rpm - 1000.f) * var_engineSound_8c0fcd50.highRevSlope_0x20 - 127,
            0
        );

        if (rpm < 2100) {
            var_engineSound_8c0fcd50.activeFlags_0x00 &= ~4;

            sdMidiSetVol(
                var_midiHandles_8c0fcd28[6],
                -127,
                0
            );
        }
    }
}

/* The two branches are identical in the original, so this is just
 * 10 < rpm < 4000. */
STATIC void updateEnginePitch_8c01023c()
{
    int rpm = var_hudState_8c22643c.engineRpm_0x2c;

    if (rpm > 10.f && rpm < 3000.f) {
        sdMidiSetPitch(
            var_midiHandles_8c0fcd28[6],
            (rpm - 10.f) * 0.05,
            0
        );
        sdMidiSetPitch(
            var_midiHandles_8c0fcd28[7],
            (rpm - 10.f) * 0.05,
            0
        );
    } else if (rpm >= 3000.f && rpm < 4000.f) {
        sdMidiSetPitch(
            var_midiHandles_8c0fcd28[6],
            (rpm - 10.f) * 0.05,
            0
        );
        sdMidiSetPitch(
            var_midiHandles_8c0fcd28[7],
            (rpm - 10.f) * 0.05,
            0
        );
    }
}

STATIC void updateEngineVoices_8c0102d8()
{
    int engineState = var_busState_8c1bb9d0.engineState_0x2e0;
    int rpm = var_hudState_8c22643c.engineRpm_0x2c;

    if ((var_engineSound_8c0fcd50.activeFlags_0x00 & 1) != 1 && engineState == 1) {
        sdMidiSetPitch(var_midiHandles_8c0fcd28[6], -200, 0);
        sdMidiSetVol(var_midiHandles_8c0fcd28[6], var_engineSound_8c0fcd50.idleVol_0x14 - 127, 0);
        sdMidiPlay(var_midiHandles_8c0fcd28[6], 1, 43, 0);

        /* An assignment, not |=: the other two layers are marked down without
         * being stopped. */
        var_engineSound_8c0fcd50.activeFlags_0x00 = 1;
    } else if ((var_engineSound_8c0fcd50.activeFlags_0x00 & 1) == 1 && engineState != 1) {
        sdMidiSetVol(var_midiHandles_8c0fcd28[6], -127, 2000);
        var_engineSound_8c0fcd50.activeFlags_0x00 &= ~1;
    }

    if ((var_engineSound_8c0fcd50.activeFlags_0x00 & 2) != 2 && rpm >= 400.f) {
        sdMidiSetPitch(var_midiHandles_8c0fcd28[7], 0, 0);
        sdMidiSetVol(var_midiHandles_8c0fcd28[7], -127, 0);
        sdMidiPlay(var_midiHandles_8c0fcd28[7], 1, 44, 0);

        var_engineSound_8c0fcd50.activeFlags_0x00 |= 2;
    } else if ((var_engineSound_8c0fcd50.activeFlags_0x00 & 2) == 2 && rpm < 400.f) {
        var_engineSound_8c0fcd50.activeFlags_0x00 &= ~2;
    }

    if ((var_engineSound_8c0fcd50.activeFlags_0x00 & 4) != 4 && rpm >= 2100.f) {
        sdMidiSetPitch(var_midiHandles_8c0fcd28[6], 0, 0);
        sdMidiSetVol(var_midiHandles_8c0fcd28[6], -127, 0);
        sdMidiPlay(var_midiHandles_8c0fcd28[6], 1, 45, 0);

        var_engineSound_8c0fcd50.activeFlags_0x00 |= 4;
    }
}

STATIC void createAdxHandles_8c010428()
{
    Sint8 i;
    for (i = 0; i < 2; i++)
    {
        var_adxtHandles_8c0fcd20[i] = ADXT_Create(2, &var_adxtWork_8c0fcd74[i * WKSIZE], WKSIZE);
    }
}

STATIC void createMidiHandles_8c010468()
{
    int i;
    for (i = 0; i < 8; i++)
    {
        sdMidiOpenPort(&var_midiHandles_8c0fcd28[i]);
    }
}

STATIC void createAdxAndMidiHandles_8c01048e()
{
    createAdxHandles_8c010428();
    createMidiHandles_8c010468();
}

STATIC Sint32 unusedSectorAlign_8c0104bc(Sint32 fsize)
{
    fsize += 2047;
    return (fsize / 2048) * 2048;
}

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

/* E8101214 is the only ADXT error handled. */
STATIC void adxErrFunc_8c010532(void *obj, char *msg)
{
    char lmsg[9] = "E8101214";

    if (strncmp(msg, lmsg, strlen(lmsg)) == 0)
    {
        SndStopAdx_8c010ca6(0);
        SndStopAdx_8c010ca6(1);

        init_adxPlaying_8c03bd80 = 0;
        init_soundOk_8c03bd84 = 0;
    }
}

STATIC void loadAdxPartitions_8c01057a()
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

/* bus.mlt: the tone bank the MIDI ports play from. */
STATIC void downloadSoundBank_8c010614()
{
    SDMEMBLK memblk;

    sdMemBlkCreate(&memblk);
    sdMemBlkSetPrm(memblk, var_sndBankData_8c0fcd4c, 0x119d00, SDD_MEMBLK_SYNC_FUNC, NULL);
    sdMultiUnitDownload(memblk);
    syFree(var_sndBankData_8c0fcd4c);
    sdMemBlkDestroy(memblk);
}

STATIC void adxInit_8c01064c()
{
    ADXT_Init();
    ADXT_EntryErrFunc(adxErrFunc_8c010532, NULL);
}

STATIC void initSoundDriver_8c01065e()
{
    SDMEMBLK memblk = NULL;

    sdLibInit(NULL, 0, 0);
    sdMemBlkCreate(&memblk);
    sdMemBlkSetPrm(memblk, var_sndDrvData_8c0fcd48, 0, SDD_MEMBLK_SYNC_FUNC, NULL);
    sdDrvInit(memblk);
    sdMemBlkDestroy(memblk);
    syFree(var_sndDrvData_8c0fcd48);
    sdMemBlkSetTransferMode(SDE_MEMBLK_TRANSFER_MODE_DMA);
}

/* Nothing signals the end of an ADX stream, so it has to be polled. */
void SndPollVoiceEnd_8c0106ac()
{
    int s = ADXT_GetStat(var_adxtHandles_8c0fcd20[1]);
    if (s == ADXT_STAT_STOP || s == ADXT_STAT_PLAYEND) {
        init_adxPlaying_8c03bd80 &= ~SND_ADX_VOICE;
    }
}

/* The option screen's SFX test, over MIDI port 0. Ids 0-7 are bank 0
 * sequences 0-6 and 9 (7 and 8 are skipped), ids 8-68 bank 1 sequences 0-60. */
Bool SndPlaySfx_8c0106d2(Sint32 id)
{
    if (id >= 7) {
        id += 2;
    }

    if (id >= 0 && id < 10) {
        SndMidiResetFxAndPlay_8c010846(0, id);
        return TRUE;
    } else {
        if (id >= 10 && id <= 70) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, id - 10, 0);
            return TRUE;
        }

        return FALSE;
    }
}

/* The option screen's VOICE test: ids 0-62 are bgm.afs 17-79, 63-1388 are
 * voice.afs 0-1325. */
Bool SndPlayVoice_8c010720(Sint32 id)
{
    if (id >= 0 && id < 63) {
        SndPlayAdx_8c010cd6(1, id + 17);
        return TRUE;
    } else {
        if (id >= 63 && id <= 1388) {
            SndPlayAdx_8c010cd6(2, id - 63);
            return TRUE;
        }

        return FALSE;
    }
}

/* The option screen's MUSIC test: bgm.afs 0-16. */
int SndPlayBgm_8c0107ac(Sint32 id)
{
    if (id >= 0 && id < 17) {
        SndPlayAdx_8c010cd6(0, id);
        return 1;
    }
    return 0;
}

/* The MIDI ports really pause; the streams only drop to minimum volume and
 * keep running. */
void SndSetPaused_8c0107d2(Bool paused)
{
    Uint32 i;

    if (paused == TRUE) {
        ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], -990);
        ADXT_SetOutVol(var_adxtHandles_8c0fcd20[1], -990);

        for (i = 0; i < 8; i++)
            sdMidiPause(var_midiHandles_8c0fcd28[i]);
    } else if (paused == FALSE) {
        ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], -990 + init_adxVolumes_8c03bd88.vol0_0x00);
        ADXT_SetOutVol(var_adxtHandles_8c0fcd20[1], -990 + init_adxVolumes_8c03bd88.vol1_0x04);

        for (i = 0; i < 8; i++)
            sdMidiContinue(var_midiHandles_8c0fcd28[i]);
    }
}

/* Bank 0, dry: the effect send is zeroed on ports 0-5 first. */
void SndMidiResetFxAndPlay_8c010846(int hld_idx, int data_num)
{
    Sint8 i;

    sdSndSetFxPrg(0, 0);

    for (i = 0; i < 6; i++)
        sdMidiSetFxLev(var_midiHandles_8c0fcd28[i], 0);

    sdMidiPlay(var_midiHandles_8c0fcd28[hld_idx], 0, data_num, 0);
}

/* Also saves the mode to the console's system config. */
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

/* ADXT_SetOutVol takes an attenuation: level - 990. */
void SndSetAdxVol_8c010972(int volNo, int handle) {
    const int adxVols[10] = {
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
            init_adxVolumes_8c03bd88.vol0_0x00 = adxVols[volNo];
            ADXT_SetOutVol(var_adxtHandles_8c0fcd20[handle], init_adxVolumes_8c03bd88.vol0_0x00 - 990);
            break;
        }
        case 1: {
            init_adxVolumes_8c03bd88.vol1_0x04 = adxVols[volNo];
            ADXT_SetOutVol(var_adxtHandles_8c0fcd20[handle], init_adxVolumes_8c03bd88.vol1_0x04 - 990);
            break;
        }
    }
}

void SndSetMidiVol_8c0109f4(int volNo) {
    int i;
    int midiVols[10] = {
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

    init_midiVol_8c03bd90 = midiVols[volNo];

    for (i = 0; i < 8; i++)
        sdMidiSetVol(var_midiHandles_8c0fcd28[i], init_midiVol_8c03bd90 - 127, 0);

    initEngineVol_8c0100bc();
}

/* Per frame. The fade-in half is dead: only unusedStartAdxCh1FadeIn_8c010c2c
 * raises a 0xf0 bit. */
void SndUpdateAdxVolFade_8c010a40() {
    if ((var_adxFade_8c157a34.fadeFlags_0x00 & 0xf) != 0)
    {
        if ((var_adxFade_8c157a34.fadeFlags_0x00 & 1) == 1)
        {
            var_adxFade_8c157a34.curVol0_0x0c -= var_adxFade_8c157a34.step0_0x04;
            ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], var_adxFade_8c157a34.curVol0_0x0c);
        }

        if ((var_adxFade_8c157a34.fadeFlags_0x00 & 2) == 2)
        {
            var_adxFade_8c157a34.curVol1_0x10 -= var_adxFade_8c157a34.step1_0x08;
            ADXT_SetOutVol(var_adxtHandles_8c0fcd20[1], var_adxFade_8c157a34.curVol1_0x10);
        }

        if (
            (var_adxFade_8c157a34.curVol0_0x0c < -300)
            && (var_adxFade_8c157a34.fadeFlags_0x00 & 1) == 1
        )
        {
            ADXT_Stop(var_adxtHandles_8c0fcd20[0]);
            ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], init_adxVolumes_8c03bd88.vol0_0x00 - 990);

            var_adxFade_8c157a34.fadeFlags_0x00 &= ~1;
            init_adxPlaying_8c03bd80 &= ~SND_ADX_BGM;
        }

        if (
            (var_adxFade_8c157a34.curVol1_0x10 < -300)
            && (var_adxFade_8c157a34.fadeFlags_0x00 & 2) == 2
        )
        {
            ADXT_Stop(var_adxtHandles_8c0fcd20[1]);
            ADXT_SetOutVol(var_adxtHandles_8c0fcd20[1], init_adxVolumes_8c03bd88.vol1_0x04 - 990);
            var_adxFade_8c157a34.fadeFlags_0x00 &= ~2;
            init_adxPlaying_8c03bd80 &= ~SND_ADX_VOICE;
        }
    }
    else
    {
        if ((var_adxFade_8c157a34.fadeFlags_0x00 & 0xf0) != 0) {
            if ((var_adxFade_8c157a34.fadeFlags_0x00 & 0x10) == 0x10) {
                var_adxFade_8c157a34.curVol0_0x0c += var_adxFade_8c157a34.step0_0x04;
                ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], var_adxFade_8c157a34.curVol0_0x0c);
            }

            if ((var_adxFade_8c157a34.fadeFlags_0x00 & 0x20) == 0x20) {
                var_adxFade_8c157a34.curVol1_0x10 += var_adxFade_8c157a34.step1_0x08;
                ADXT_SetOutVol(var_adxtHandles_8c0fcd20[1], var_adxFade_8c157a34.curVol1_0x10);
            }

            if (
                (var_adxFade_8c157a34.curVol0_0x0c > init_adxVolumes_8c03bd88.vol0_0x00)
                && ((var_adxFade_8c157a34.fadeFlags_0x00 & 0x10) == 0x10)
            ) {
                ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], init_adxVolumes_8c03bd88.vol0_0x00);
                var_adxFade_8c157a34.fadeFlags_0x00 &= ~0x10;
            }

            if (
                (var_adxFade_8c157a34.curVol1_0x10 > init_adxVolumes_8c03bd88.vol1_0x04)
                && ((var_adxFade_8c157a34.fadeFlags_0x00 & 0x20) == 0x20)
            ) {
                ADXT_SetOutVol(var_adxtHandles_8c0fcd20[1], init_adxVolumes_8c03bd88.vol1_0x04);
                var_adxFade_8c157a34.fadeFlags_0x00 &= ~0x20;
            }
        }
    }
}

/* Ignored while a fade-in is up. The step would take the level to 300 in 90
 * frames, but the fade ends once the attenuation passes -300: ~43 frames at
 * full volume, one frame at setting 6 or below. The guards compare the whole
 * low nibble, so a stream already fading restarts if the other one is too. */
void SndStartAdxFadeOut_8c010bae(int handle) {
    if ((var_adxFade_8c157a34.fadeFlags_0x00 & 0xf0) == 0) {
        if (handle == 0 && (var_adxFade_8c157a34.fadeFlags_0x00 & 0xf) != 1) {
            var_adxFade_8c157a34.fadeFlags_0x00 |= 1;
            var_adxFade_8c157a34.step0_0x04 = (init_adxVolumes_8c03bd88.vol0_0x00 - 300) / 90;
            var_adxFade_8c157a34.curVol0_0x0c = init_adxVolumes_8c03bd88.vol0_0x00 - 990;
        }
        if (handle == 1 && (var_adxFade_8c157a34.fadeFlags_0x00 & 0xf) != 2) {
            var_adxFade_8c157a34.fadeFlags_0x00 |= 2;
            var_adxFade_8c157a34.step1_0x08 = (init_adxVolumes_8c03bd88.vol1_0x04 - 300) / 90;
            var_adxFade_8c157a34.curVol1_0x10 = init_adxVolumes_8c03bd88.vol1_0x04 - 990;
        }
    }
}

STATIC void unusedStartAdxCh1FadeIn_8c010c2c(Bool handle) {
    if ((var_adxFade_8c157a34.fadeFlags_0x00 & 0xf) == 0
        && handle == 1
        && (var_adxFade_8c157a34.fadeFlags_0x00 & 0xf0) != 0x20)
    {
        var_adxFade_8c157a34.fadeFlags_0x00 |= 0x20;
        var_adxFade_8c157a34.step1_0x08 = init_adxVolumes_8c03bd88.vol1_0x04 / 90;
        var_adxFade_8c157a34.curVol1_0x10 = -990;
        init_adxPlaying_8c03bd80 &= ~SND_ADX_VOICE;
    }
}

void SndUpdateEngine_8c010c6e() {
    updateEngineVol_8c010128();
    updateEnginePitch_8c01023c();
    updateEngineVoices_8c0102d8();
}

void SndStopAllAdx_8c010c7c() {
    int i;
    for (i=0; i<2; i++) {
        ADXT_Stop(var_adxtHandles_8c0fcd20[i]);
    }

    init_adxPlaying_8c03bd80 = 0;
    return;
}

void SndStopAdx_8c010ca6(Bool handle) {
    if (handle == 0) {
        init_adxPlaying_8c03bd80 &= ~SND_ADX_BGM;
    } else if (handle == 1) {
        init_adxPlaying_8c03bd80 &= ~SND_ADX_VOICE;
    }

    ADXT_Stop(var_adxtHandles_8c0fcd20[handle]);
    return;
}

int SndPlayAdx_8c010cd6(int source, int fileNo) {
    switch (source) {
        case 0: {
            ADXT_Stop(var_adxtHandles_8c0fcd20[source]);
            ADXT_StartAfs(var_adxtHandles_8c0fcd20[source], 0, fileNo);
            init_adxPlaying_8c03bd80 |= SND_ADX_BGM;
            return 1;
        }

        case 1: {
            ADXT_Stop(var_adxtHandles_8c0fcd20[source]);
            ADXT_StartAfs(var_adxtHandles_8c0fcd20[source], 0, fileNo);
            init_adxPlaying_8c03bd80 |= SND_ADX_VOICE;
            return 1;
        }

        case 2: {
            ADXT_Stop(var_adxtHandles_8c0fcd20[1]);
            ADXT_StartAfs(var_adxtHandles_8c0fcd20[1], 1, fileNo);
            init_adxPlaying_8c03bd80 |= SND_ADX_VOICE;
            return 1;
        }
    }

    return 0;
}

/* The body is written out twice in the original; the second pass is a no-op. */
void SndStopBgm_8c010d8a() {
    ADXT_Stop(var_adxtHandles_8c0fcd20[0]);
    ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], init_adxVolumes_8c03bd88.vol0_0x00 - 990);
    var_adxFade_8c157a34.fadeFlags_0x00 &= ~1;
    init_adxPlaying_8c03bd80 &= ~SND_ADX_BGM;

    ADXT_Stop(var_adxtHandles_8c0fcd20[0]);
    ADXT_SetOutVol(var_adxtHandles_8c0fcd20[0], init_adxVolumes_8c03bd88.vol0_0x00 - 990);
    var_adxFade_8c157a34.fadeFlags_0x00 &= ~1;
    init_adxPlaying_8c03bd80 &= ~SND_ADX_BGM;
}

STATIC void unusedStopMidiAdx_8c010de6() {
    sdMidiStopAll();
    ADXT_Stop(var_adxtHandles_8c0fcd20[0]);
    ADXT_Stop(var_adxtHandles_8c0fcd20[1]);
    init_adxPlaying_8c03bd80 = 0;
    init_soundOk_8c03bd84 = 1;
    memset(&var_adxFade_8c157a34, 0, sizeof(AdxFadeState));
    initEngineVol_8c0100bc();
}

/* dirname holds the .afs partitions. The busy bits set partway through are
 * cleared again at the end; the loads are synchronous. */
void SndInit_8c010e18(char *dirname) {
    init_adxPlaying_8c03bd80 = 0;
    init_soundOk_8c03bd84 = 1;
    gdFsChangeDir(dirname);
    memset(&var_adxFade_8c157a34, 0, sizeof(AdxFadeState));
    initEngineVol_8c0100bc();
    init_adxPlaying_8c03bd80 |= SND_ADX_BGM | SND_ADX_VOICE;
    initSoundDriver_8c01065e();
    adxInit_8c01064c();
    createAdxAndMidiHandles_8c01048e();
    loadAdxPartitions_8c01057a();
    downloadSoundBank_8c010614();
    init_adxPlaying_8c03bd80 = 0;
}
