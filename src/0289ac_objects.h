/* 8c0289ac */
#ifndef _0289AC_OBJECTS_H
#define _0289AC_OBJECTS_H

#include <shinobi.h>

extern int var_activePedPreset_8c22822c;
extern void* var_pedGroups_8c228230;
extern int var_pedGroupCount_8c228234; /* -1 when not loaded */

void ObjectsSpawnPedestrianGroups_8c0296d6(void);
void ObjectsKillPedestrianGroups_8c0297da(void);
void ObjectsSpawnBlinkers_8c029920(void);
void ObjectsClearAssetRequestTable_8c029acc(void);
void ObjectsFreeAssetRequests_8c029cfe(void);
void ObjectsRelocatePedGroupLists_8c028dd0(void *handle);
void ObjectsRelocatePedGroupDefs_8c028de8(void *handle);
void ObjectsStartAssetRequests_8c029ad4(int *table);
void ObjectsSpawnTasks_8c02a6ac(void);

extern NJS_TEXANIM init_pedestrianTexAnims_8c04623c[];

#endif // _0289AC_OBJECTS_H
