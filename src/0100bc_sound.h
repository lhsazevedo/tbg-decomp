/* 8c0100bc */
#ifndef _SOUND_H
#define _SOUND_H

#include <shinobi.h>
#include <sg_sd.h>

/* =========================
 * External Declarations
 * =========================
 */

extern int init_8c03bd80;
extern int init_8c03bd84;
extern void* var_memblkSource_8c0fcd48;
extern void* var_memblkSource_8c0fcd4c;
extern SDMIDI var_midiHandles_8c0fcd28[8];

/* =========
 * Functions
 * =========
 */

void midiResetFxAndPlay_8c010846(int hld_idx, int data_num);
void FUN_8c010ca6(Bool p1);
int snd_8c010cd6(int p1, int p2);
void FUN_8c0106ac();
Bool FUN_8c0106d2(Sint32 param);
Bool FUN_8c010720(Sint32 param);
int FUN_8c0107ac(Sint32 param);
void controlAdxtWithOutVol_8c0107d2(Bool play);
Bool setSoundMode_8c0108c0(Sint32 mode);
int getSoundMode_8c010924();
void setAdxVol_8c010972(int volNo, int handle);
void setMidiVolAndInitStruct_8c0109f4(int param1);
void updateAdxVolFade_8c010a40();
void startAdxFadeOut_8c010bae(int param1);
void FUN_8c010c6e();
void FUN_8c010c7c();
void FUN_8c010d8a();
void initSoundMidiAdx_8c010e18(char *dirname);

#endif // _SOUND_H
