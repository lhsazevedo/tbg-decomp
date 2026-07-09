/* 8c1ba1c8: undecompiled BSS block (section B) referenced from many units */
#ifndef _014F54_TEXT_PRE_DATA_H
#define _014F54_TEXT_PRE_DATA_H

#include <shinobi.h>
#include "011120_asset_queues.h"
#include "013ae8_route_load.h"
#include "014a9c_tasks.h"
#include "014b8c_backup.h"
#include "014f54_text.h"

extern signed char var_8c1ba290;
extern Uint32 var_8c1ba291;
extern Uint32 var_8c1ba292;
extern int var_8c1ba2b8[5]; // Maybe progress backup
extern int var_8c1ba2cc[5]; // Maybe progress backup
extern void* var_8c1ba2e0;
extern void* var_8c1ba344;
extern void* var_8c1ba348;
extern void* var_8c1bb86c;
extern int var_8c1bb8b8; // Maybe courseMenuHasResult or courseMenuHasDialog
extern int var_8c1bb8bc;
extern int var_8c1bb8c4;
extern int var_8c1bb8cc;
extern int var_8c1bb8d4;
extern int var_8c1bb8dc;
extern int var_8c1bb8e0; // course was unlocked
extern int var_8c1bb8e4;
extern int var_8c1bb8e8;
extern int var_8c1bb8ec;
extern int var_8c1bb8f0;
extern int var_8c1bb8f4;
extern float var_8c1bbc4c;
extern int var_8c1bbc84;
extern Uint32 var_8c1bbcb0;
extern int var_8c1bbcc4;
extern void* var_8c1bc404;
extern void* var_8c1bc410;
extern void* var_8c1bc414;
extern void* var_8c1bc440;
extern void* var_8c1bc444;
extern float var_8c1bc450;
extern void* var_8c1bc454;
extern int var_award_8c1bb8f8;
extern void* var_busFont_8c1ba1c8;
extern char var_busState_8c1bb9d0[]; // real type is 012f44.c's local BusState struct
extern void* var_busstopDat_8c1bc42c;
extern void* var_busstopPartsDat_8c1bc428;
extern NJS_TEXLIST *var_busStopTexlist_8c1bc424;
extern CurrentCourse var_currentCourse_8c1bb868;
extern int var_cutsceneActive_8c1bb900;
extern int* var_demoBuf_8c1ba3c4;
extern int var_demoIndex_8c1bb8d8;
extern int var_exp_8c1ba25c;
extern void *var_frontNj_8c1bc434;
extern NJS_TEXLIST *var_frontTexlist_8c1bc430;
extern int var_gameMode_8c1bb8fc;
extern BACKUPINFO var_gBupInfo_8c1bc4ac[8];
extern int var_inputMapSel_8c1bb8c8;
extern void *var_interiorNj_8c1bc43c;
extern NJS_TEXLIST *var_interiorTexlist_8c1bc438;
extern NJS_MOTION* var_loadedFooNjm_8c1bc448;
extern ResourceGroup var_loadingResourceGroup_8c1bc3f8;
extern void* var_markDat_8c1bc420;
extern void* var_markPartsDat_8c1bc41c;
extern NJS_TEXLIST *var_markTexlist_8c1bc418;
extern ModelSlot var_pedestrianAssets_8c1bbfdc[0x41];
extern PDS_PERIPHERAL *var_peripheral_8c1ba358;
extern PDS_PERIPHERAL var_peripherals_8c1ba35c[2];
extern enum PLAY_MODE var_playMode_8c1bb8d0;
extern char var_progress_8c1ba1cc[]; // real size exceeds PlayerProgress (011120_asset_queues.c indexes past it)
extern void* var_routeModels_8c1bc3ec;
extern ModelSlot var_routeModelSlots_8c1bbddc[0x20];
extern LoadedModel *var_segmentModels_8c1bc3f0;
extern int var_selectedVm_8c1ba34c;
extern int var_shouldShowFreeRunIntro_8c1bb8c0;
extern Task var_tasks_8c1ba3c8[];
extern Task var_tasks_8c1ba5e8[];
extern Task var_tasks_8c1ba808[];
extern Task var_tasks_8c1bac28[];
extern Task var_tasks_8c1bb448[];
extern LoadedModel *var_trafficModels_8c1bc3f4;
extern Uint32 var_vibport_8c1ba354;

#endif // _014F54_TEXT_PRE_DATA_H
