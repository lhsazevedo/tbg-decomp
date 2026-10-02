/* 8c0100bc */
#ifndef _SOUND_H
#define _SOUND_H

#include <shinobi.h>
#include <sg_sd.h>

/* =========================
 * External Declarations
 * =========================
 */

/* Nonzero while a stream plays (bit0 music, bit4 voice). Stored as 1 with
 * init_soundOk_8c03bd84 at 0, it tells GameMain_8c01392e to quit. */
extern int init_adxPlaying_8c03bd80;
/* Cleared when the ADX layer gives up on the disc. */
extern int init_soundOk_8c03bd84;
/* manatee.drv and bus.mlt, to be loaded before SndInit_8c010e18, which frees
 * them. */
extern void* var_sndDrvData_8c0fcd48;
extern void* var_sndBankData_8c0fcd4c;
extern SDMIDI var_midiHandles_8c0fcd28[8];

/* =========
 * Functions
 * =========
 */

void SndMidiResetFxAndPlay_8c010846(int hld_idx, int data_num);
void SndStopAdx_8c010ca6(Bool handle);
/* source: 0 music (bgm.afs, handle 0), 1 announcements (bgm.afs, handle 1),
 * 2 dialogue (voice.afs, handle 1). */
int SndPlayAdx_8c010cd6(int source, int fileNo);
void SndPollVoiceEnd_8c0106ac();
Bool SndPlaySfx_8c0106d2(Sint32 id);
Bool SndPlayVoice_8c010720(Sint32 id);
int SndPlayBgm_8c0107ac(Sint32 id);
void SndSetPaused_8c0107d2(Bool paused);
Bool SndSetSoundMode_8c0108c0(Sint32 mode);
int SndGetSoundMode_8c010924();
/* volNo is the 0-9 setting; handle 0 is MUSIC, 1 is VOICE. */
void SndSetAdxVol_8c010972(int volNo, int handle);
/* param1 is the 0-9 SFX setting. */
void SndSetMidiVol_8c0109f4(int volNo);
void SndUpdateAdxVolFade_8c010a40();
void SndStartAdxFadeOut_8c010bae(int handle);
void SndUpdateEngine_8c010c6e();
void SndStopAllAdx_8c010c7c();
void SndStopBgm_8c010d8a();
void SndInit_8c010e18(char *dirname);

#endif // _SOUND_H
