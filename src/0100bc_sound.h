/* 8c0100bc */
#ifndef _SOUND_H
#define _SOUND_H

#include <shinobi.h>
#include <sg_sd.h>

/* =========================
 * External Declarations
 * =========================
 */

/* Nonzero while an ADX stream is playing (bit0 music, bit4 voice); menus poll
 * it before tearing their screen down. GameTask_8c012f44 and
 * pauseDemoEndTask_8c012d5a also store 1 here with init_soundOk_8c03bd84 at 0,
 * which is how GameMain_8c01392e is told to quit. */
extern int init_adxPlaying_8c03bd80;
/* Cleared when the ADX layer gives up on the disc. */
extern int init_soundOk_8c03bd84;
/* manatee.drv and bus.mlt, loaded into these by GameMain_8c01392e and freed by
 * SndInit_8c010e18 once downloaded to the AICA. */
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
/* Clears the voice bit once handle 1's stream has run out. */
void SndPollVoiceEnd_8c0106ac();
Bool SndPlaySfx_8c0106d2(Sint32 id);
Bool SndPlayVoice_8c010720(Sint32 id);
int SndPlayBgm_8c0107ac(Sint32 id);
/* Mutes the streams and pauses the MIDI ports, for the pause menu. */
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
