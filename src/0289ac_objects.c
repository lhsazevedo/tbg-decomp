/* @unit Objects */
/* 8c0289ac */
#include <shinobi.h>
#include <stdlib.h> /* rand */

#include "0289ac_objects.h"
#include "011120_asset_queues.h"
#include "013ae8_route.h"
#include "014a9c_tasks.h"
#include "015034_text.h"
#include "0206f0_intersect.h"
#include "020914_ground_query.h"
#include "020b6c_ground_probe.h"
#include "021b9c_tile_draw.h"
#include "022464_render.h"
#include "028258_signal.h"
#include "02d06c_stop_draw.h"
#include "02e400_collision.h"
#include "02b464_grading.h"
#include "024b4c_bus_camera.h"
#include "1ba1c8_globals.h"
#include "includes.h"
#include "serial_debug.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* Tables end with an all-zero entry. */
typedef struct {
    float x_0x00;
    float z_0x04;
    int angleDeg_0x08;
} BlinkerPoint;

/* A path ends with a node whose flLength_0x00 is 0. A pedestrian flPathPos_0x54
 * along a node sits at flBase + flPathPos * flStep. */
typedef struct {
    float flLength_0x00;
    float flBaseX_0x04;
    float flBaseZ_0x08;
    float flStepX_0x0c;
    float flStepZ_0x10;
    /* Bits 0-11: crossing id, bits 16-27: signal id, bits 28-29: mirror-view
     * facing bucket. */
    int nFlags_0x14;
} PedPathNode;

typedef struct {
    NJS_SPRITE sprite_0x00;
    float flBaseX_0x20;
    float flBaseZ_0x24;
    GroundQueryResult aGround_0x28;
    int nKindId_0x38;
    int nKind_0x3c;
    int nReverse_0x40; /* kind 2: sprite facing, 0-3 */
    PedPathNode *pPathFirst_0x44;
    PedPathNode *pPathLast_0x48;
    PedPathNode *pPathNode_0x4c;
    float flPathLength_0x50;
    float flPathPos_0x54;
    float flSpeed_0x58;
    int nAnimPhase_0x5c;
    int nNodeFlags_0x60;
    int nState_0x64;       /* 0 walking, 1 waiting out a green, 2 waiting for green, 4 crossing */
    int nSignalIdA_0x68;
    int nSignalIdB_0x6c;
} PedestrianState;

#define DIR_CACHE_CAPACITY 0x80

#define PED_GROUP_SLOTS 0x10

typedef struct {
    PedPathNode *key;
    int bucket; /* 0, 0x10000000, 0x20000000 or 0x30000000 */
} DirCacheEntry;

typedef struct {
    int active_0x00;
    int wanted_0x04;   /* in the active preset; a group no longer wanted tears itself down */
    Task *list_0x08;
} PedGroupEntry;

typedef struct {
    Uint16 nKindId_0x00; /* list terminator: 0xffff */
    Uint8 nKind_0x02;
    Uint8 nReverse_0x03;
    float flX_0x04; /* kinds 0/1: distance along the path, 0 for random; kind 2: world X */
    float flZ_0x08; /* kind 2 only */
} PedGroupSpawnSpec;

/* Terminated by id_0x00 == -1. */
typedef struct {
    int id_0x00;
    float radius_0x04;
    PedGroupSpawnSpec *specs_0x08;
} PedGroupDef;

typedef struct {
    PedPathNode *pFirst_0x00;
    PedPathNode *pLast_0x04;
    float flLength_0x08;
} PedPathInfo;

typedef struct {
    TaskAction action;
    void *state;
    int pageListIndex_0x08;
    void *field_0x0c;
    float radius_0x10; /* spawn scatter around the path */
    int field_0x14;
    Task *subTasks_0x18;
    PedGroupSpawnSpec *spec_0x1c;
} PedGroupTask;

typedef struct {
    TaskAction action;
    void *state;
    int lastPreset_0x08;
    int armed_0x0c;      /* a preset applies only after the field has read 0 */
    int field_0x10;
    int field_0x14;
    void *queuedItem_0x18;
    int field_0x1c;
} PedestriansTask;

/* A struct, not two floats: the copy compiles to SHC's __quick_evn_mvn. */
typedef struct {
    float posX_0x00;
    float posZ_0x04;
} ObjectAssetType5Extra;

/* Which members a row uses depends on its type. */
typedef struct {
    NJS_TEXLIST *pvm_0x00;
    NJS_CNK_OBJECT *nj_0x04;
    void *dat_0x08;                 /* NJS_MOTION for types 0/3, DatBlob for type 1 */
    ObjectAssetType5Extra pos_0x0c; /* type 5 */
    Sint8 sceneGate_0x14;           /* types 2/3: scenePresetIds_0x3bc top byte to wait for */
    Sint8 taskFlag_0x15;            /* type 3: loop the motion; type 4: fog enable */
    Uint8 fogEnable_0x16;            /* types 2/3 */
    Uint8 control3DEnable_0x17;      /* types 2/3 */
} AssetRequestSlot;

/* A type-1 row's DAT file: a header, then one child-visibility bitmask per
 * animation step. */
typedef struct {
    int nNodes_0x00;                   /* mask bit n is child n; bit 0 unused */
    int framesPerStep_0x04;
    int stepCount_0x08;
    int step_0x0c;
    int tick_0x10;
    Uint32 *masks_0x14;
    NJS_TEXLIST *texlist_0x18;
    NJS_CNK_OBJECT *model_0x1c;
    Uint32 masks_0x20[1];              /* stepCount_0x08 entries */
} DatBlob;

/* State of every row task, whatever its type; each type uses a subset. */
typedef struct RowTaskState {
    char field_0x00[0x40];
    NJS_TEXLIST *texlist_0x40;
    NJS_CNK_OBJECT *model_0x44;
    void *dat_0x48;                    /* as AssetRequestSlot.dat_0x08; type 6: the train motion */
    ObjectAssetType5Extra pos_0x4c;    /* type-5 only */
    int phase_0x54;                    /* types 2/3/6 */
    float frame_0x58;
    float frameLimit_0x5c;
    float trainFrame_0x60;             /* type-6 only */
    float trainFrameLimit_0x64;        /* type-6 only; zeroed once the train has passed */
    NJS_ARGB material_0x68;            /* type-5 only */
    Uint8 fogEnable_0x78;              /* types 2/3 */
    Uint8 control3DEnable_0x79;        /* types 2/3 */
    Uint8 field_0x7a[2];
} RowTaskState;

/* ====================
 * Non-initialized Globals
 * ====================
 */

int var_activePedPreset_8c22822c;
void* var_pedGroups_8c228230;
int var_pedGroupCount_8c228234;
STATIC void* var_pedPaths_8c228238;     /* PedPathInfo[], one per group */
STATIC void* var_pedGroupDefs_8c22823c; /* PedGroupDef[] */
/* Indexed by var_activePedPreset_8c22822c: -1 terminated lists of group ids. */
STATIC void* var_pedGroupLists_8c228240;
/* Node pairs bounding the crosswalk segments the bus straddles this frame. */
STATIC int* var_blockedCrosswalksEnd_8c228244;
STATIC int var_blockedCrosswalks_8c228248[8];
STATIC float var_busAxisA_8c228268[2]; /* world X/Z */
STATIC float var_busAxisB_8c228270[2]; /* world X/Z */
/* The blinker model and its three lamp children. */
STATIC NJS_OBJECT *var_routeBlinkerNodes_8c228278[4];
STATIC Uint8 var_assetRequestSlots_8c228288[16 * 0x18]; /* AssetRequestSlot[16] */
/* {type, data} pairs terminated by type -1; -1 when no table is loaded. */
STATIC int *var_assetRequestTable_8c228408;
/* Type-6 (railway crossing) assets: fixed files, so at most one crossing. */
STATIC void *var_fumiGateModel_8c22840c;
STATIC void *var_fumiTexlist_8c228410;
STATIC void *var_fumiCloseMotion_8c228414;
STATIC void *var_fumiOpenMotion_8c228418;
STATIC void *var_fumiLampModel_8c22841c;
STATIC void *var_fumiLampTexlist_8c228420;
STATIC void *var_fumiTrainModel_8c228424;
STATIC void *var_fumiTrainTexlist_8c228428;
STATIC void *var_fumiTrainMotionA_8c22842c;
STATIC void *var_fumiTrainMotionB_8c228430;
/* The lamp model and its 16 grandchildren. */
STATIC NJS_OBJECT *var_fumiLampNodes_8c228434[17];

/* ====================
 * Initialized Globals
 * ====================
 */

/* Pedestrian billboard frames: 8 walk frames per facing bucket, then 4 for
 * kind-2 statics. */
NJS_TEXANIM init_pedestrianTexAnims_8c04623c[] = {
    {   /* [0] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 128, 0,
        /* u2, v2   */ 191, 63,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [1] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 192, 0,
        /* u2, v2   */ 255, 63,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [2] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 64,
        /* u2, v2   */ 63, 127,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [3] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 192, 0,
        /* u2, v2   */ 255, 63,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [4] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 128, 0,
        /* u2, v2   */ 191, 63,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [5] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 64, 0,
        /* u2, v2   */ 127, 63,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [6] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 0,
        /* u2, v2   */ 63, 63,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [7] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 64, 0,
        /* u2, v2   */ 127, 63,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [8] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 192, 64,
        /* u2, v2   */ 255, 127,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [9] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 128,
        /* u2, v2   */ 63, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [10] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 64, 128,
        /* u2, v2   */ 127, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [11] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 128,
        /* u2, v2   */ 63, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [12] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 192, 64,
        /* u2, v2   */ 255, 127,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [13] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 128, 64,
        /* u2, v2   */ 191, 127,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [14] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 64, 64,
        /* u2, v2   */ 127, 127,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [15] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 128, 64,
        /* u2, v2   */ 191, 127,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [16] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 192,
        /* u2, v2   */ 63, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [17] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 64, 192,
        /* u2, v2   */ 127, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [18] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 128, 192,
        /* u2, v2   */ 191, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [19] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 64, 192,
        /* u2, v2   */ 127, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [20] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 192,
        /* u2, v2   */ 63, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [21] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 192, 128,
        /* u2, v2   */ 255, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [22] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 128, 128,
        /* u2, v2   */ 191, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [23] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 192, 128,
        /* u2, v2   */ 255, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [24] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 63, 192,
        /* u2, v2   */ 0, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [25] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 127, 192,
        /* u2, v2   */ 64, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [26] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 191, 192,
        /* u2, v2   */ 128, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [27] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 127, 192,
        /* u2, v2   */ 64, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [28] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 63, 192,
        /* u2, v2   */ 0, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [29] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 255, 128,
        /* u2, v2   */ 192, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [30] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 191, 128,
        /* u2, v2   */ 128, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [31] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 255, 128,
        /* u2, v2   */ 192, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [32] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 0,
        /* u2, v2   */ 127, 127,
        /* texid    */ 1,
        /* attr     */ 0,
    },
    {   /* [33] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 128, 0,
        /* u2, v2   */ 255, 127,
        /* texid    */ 1,
        /* attr     */ 0,
    },
    {   /* [34] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 128,
        /* u2, v2   */ 127, 255,
        /* texid    */ 1,
        /* attr     */ 0,
    },
    {   /* [35] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 127, 128,
        /* u2, v2   */ 0, 255,
        /* texid    */ 1,
        /* attr     */ 0,
    },
};

/* Bus-local ends of the bus's long axis. */
STATIC NJS_POINT3 init_busAxisLocalA_8c04650c = { 0.0f, 0.0f, -4.1f };

STATIC NJS_POINT3 init_busAxisLocalB_8c046518 = { 0.0f, 0.0f, 9.5f };

STATIC BlinkerPoint init_shinjukuBlinkerPoints_8c046524[] = {
    {   /* [0] */
        /* x_0x00, z_0x04 */ 4301.0f, 4493.0f,
        /* angleDeg_0x08  */ 68,
    },
    {   /* [1] */
        /* x_0x00, z_0x04 */ 3898.0f, 4387.0f,
        /* angleDeg_0x08  */ 6,
    },
    {   /* [2] */
        /* x_0x00, z_0x04 */ 3867.0f, 3877.0f,
        /* angleDeg_0x08  */ -18,
    },
    {   /* [3] */
        /* x_0x00, z_0x04 */ 3979.0f, 3709.0f,
        /* angleDeg_0x08  */ 70,
    },
    {   /* [4] */
        /* x_0x00, z_0x04 */ 3022.0f, 3051.0f,
        /* angleDeg_0x08  */ 18,
    },
    {   /* [5] */
        /* x_0x00, z_0x04 */ 2905.0f, 2484.0f,
        /* angleDeg_0x08  */ 109,
    },
    {   /* [6] */
        /* x_0x00, z_0x04 */ 2788.0f, 2552.0f,
        /* angleDeg_0x08  */ 20,
    },
    {   /* [7] */
        /* x_0x00, z_0x04 */ 2311.0f, 1604.0f,
        /* angleDeg_0x08  */ 30,
    },
    {   /* [8] */
        /* x_0x00, z_0x04 */ 2426.0f, 772.0f,
        /* angleDeg_0x08  */ 89,
    },
    {   /* [9] */
        /* x_0x00, z_0x04 */ 2406.0f, 850.0f,
        /* angleDeg_0x08  */ -3,
    },
    {   /* [10] */
        /* x_0x00, z_0x04 */ 1999.0f, 848.0f,
        /* angleDeg_0x08  */ 20,
    },
    {   /* [11] */
        /* x_0x00, z_0x04 */ 640.0f, 154.0f,
        /* angleDeg_0x08  */ 90,
    },
    {   /* [12] */
        /* x_0x00, z_0x04 */ 500.0f, 154.0f,
        /* angleDeg_0x08  */ 180,
    },
    {   /* [13] */
        /* x_0x00, z_0x04 */ 537.0f, 349.0f,
        /* angleDeg_0x08  */ 90,
    },
    {   /* [14] */
        /* x_0x00, z_0x04 */ 0.0f, 0.0f,
        /* angleDeg_0x08  */ 0,
    },
};

STATIC BlinkerPoint init_wanganBlinkerPoints_8c0465d8[] = {
    {   /* [0] */
        /* x_0x00, z_0x04 */ 3948.0f, 2871.0f,
        /* angleDeg_0x08  */ -90,
    },
    {   /* [1] */
        /* x_0x00, z_0x04 */ 4024.0f, 2825.0f,
        /* angleDeg_0x08  */ -90,
    },
    {   /* [2] */
        /* x_0x00, z_0x04 */ 4171.0f, 3055.0f,
        /* angleDeg_0x08  */ 122,
    },
    {   /* [3] */
        /* x_0x00, z_0x04 */ 2938.0f, 3860.0f,
        /* angleDeg_0x08  */ 135,
    },
    {   /* [4] */
        /* x_0x00, z_0x04 */ 2823.0f, 3169.0f,
        /* angleDeg_0x08  */ 10,
    },
    {   /* [5] */
        /* x_0x00, z_0x04 */ 2630.0f, 4066.0f,
        /* angleDeg_0x08  */ -160,
    },
    {   /* [6] */
        /* x_0x00, z_0x04 */ 2894.0f, 4580.0f,
        /* angleDeg_0x08  */ 124,
    },
    {   /* [7] */
        /* x_0x00, z_0x04 */ 2195.0f, 4344.0f,
        /* angleDeg_0x08  */ 14,
    },
    {   /* [8] */
        /* x_0x00, z_0x04 */ 2603.0f, 2848.0f,
        /* angleDeg_0x08  */ 15,
    },
    {   /* [9] */
        /* x_0x00, z_0x04 */ 748.0f, 1974.0f,
        /* angleDeg_0x08  */ -23,
    },
    {   /* [10] */
        /* x_0x00, z_0x04 */ 741.0f, 1824.0f,
        /* angleDeg_0x08  */ -14,
    },
    {   /* [11] */
        /* x_0x00, z_0x04 */ 764.0f, 1601.0f,
        /* angleDeg_0x08  */ 90,
    },
    {   /* [12] */
        /* x_0x00, z_0x04 */ 604.0f, 1610.0f,
        /* angleDeg_0x08  */ 25,
    },
    {   /* [13] */
        /* x_0x00, z_0x04 */ 728.0f, 1302.0f,
        /* angleDeg_0x08  */ -112,
    },
    {   /* [14] */
        /* x_0x00, z_0x04 */ 796.0f, 1335.0f,
        /* angleDeg_0x08  */ -20,
    },
    {   /* [15] */
        /* x_0x00, z_0x04 */ 1174.0f, 486.0f,
        /* angleDeg_0x08  */ 70,
    },
    {   /* [16] */
        /* x_0x00, z_0x04 */ 0.0f, 0.0f,
        /* angleDeg_0x08  */ 0,
    },
};

STATIC BlinkerPoint init_omeBlinkerPoints_8c0466a4[] = {
    {   /* [0] */
        /* x_0x00, z_0x04 */ 6992.0f, 6229.0f,
        /* angleDeg_0x08  */ -123,
    },
    {   /* [1] */
        /* x_0x00, z_0x04 */ 7179.0f, 6394.0f,
        /* angleDeg_0x08  */ -48,
    },
    {   /* [2] */
        /* x_0x00, z_0x04 */ 7307.0f, 6233.0f,
        /* angleDeg_0x08  */ 7,
    },
    {   /* [3] */
        /* x_0x00, z_0x04 */ 7282.0f, 6159.0f,
        /* angleDeg_0x08  */ 104,
    },
    {   /* [4] */
        /* x_0x00, z_0x04 */ 7189.0f, 6189.0f,
        /* angleDeg_0x08  */ 63,
    },
    {   /* [5] */
        /* x_0x00, z_0x04 */ 6871.0f, 6076.0f,
        /* angleDeg_0x08  */ 5,
    },
    {   /* [6] */
        /* x_0x00, z_0x04 */ 6781.0f, 5526.0f,
        /* angleDeg_0x08  */ 45,
    },
    {   /* [7] */
        /* x_0x00, z_0x04 */ 6492.0f, 5214.0f,
        /* angleDeg_0x08  */ 38,
    },
    {   /* [8] */
        /* x_0x00, z_0x04 */ 5486.0f, 3643.0f,
        /* angleDeg_0x08  */ 55,
    },
    {   /* [9] */
        /* x_0x00, z_0x04 */ 4581.0f, 3102.0f,
        /* angleDeg_0x08  */ 7,
    },
    {   /* [10] */
        /* x_0x00, z_0x04 */ 4915.0f, 2539.0f,
        /* angleDeg_0x08  */ 17,
    },
    {   /* [11] */
        /* x_0x00, z_0x04 */ 794.0f, 1025.0f,
        /* angleDeg_0x08  */ 29,
    },
    {   /* [12] */
        /* x_0x00, z_0x04 */ 746.0f, 979.0f,
        /* angleDeg_0x08  */ 160,
    },
    {   /* [13] */
        /* x_0x00, z_0x04 */ 5902.0f, 4057.0f,
        /* angleDeg_0x08  */ 50,
    },
    {   /* [14] */
        /* x_0x00, z_0x04 */ 0.0f, 0.0f,
        /* angleDeg_0x08  */ 0,
    },
};


/* ====================
 * Forward Declarations
 * ====================
 */

void ObjectsFreeAssetRequests_8c029cfe(void);

/* ====================
 * Functions
 * ====================
 */

/* Wraps at the path's ends. Returns whether the node changed. */
STATIC Bool advancePedPathPos_8c0289ac(PedestrianState *ped)
{
    PedPathNode *node;
    PedPathNode *startNode;
    float pathPos;

    node = ped->pPathNode_0x4c;
    startNode = node;
    pathPos = ped->flPathPos_0x54;

    if (ped->nReverse_0x40 == 0) {
        while (node->flLength_0x00 <= pathPos) {
            pathPos -= node->flLength_0x00;
            node++;
            if (node->flLength_0x00 == 0.0f) {
                node = ped->pPathFirst_0x44;
            }
        }
    } else {
        for (; pathPos <= 0.0f; pathPos += node->flLength_0x00) {
            if (node == ped->pPathFirst_0x44) {
                node = ped->pPathLast_0x48;
            } else {
                node--;
            }
        }
    }

    if (startNode != node) {
        ped->pPathNode_0x4c = node;
        ped->nNodeFlags_0x60 = node->nFlags_0x14;
    }
    ped->flPathPos_0x54 = pathPos;
    ped->sprite_0x00.p.x = pathPos * node->flStepX_0x0c + node->flBaseX_0x04 + ped->flBaseX_0x20;
    ped->sprite_0x00.p.z = pathPos * node->flStepZ_0x10 + node->flBaseZ_0x08 + ped->flBaseZ_0x24;
    return startNode != node;
}

/* Takes the facing bucket from the path node's flags instead of the bus's
 * direction of travel. */
STATIC void drawPedestriansMirror_8c028a38(int arg0)
{
    PedGroupEntry *groups = (PedGroupEntry *)var_pedGroups_8c228230;
    int pageListIndex;
    Task *slot;
    PedestrianState *ped;
    int facing;
    int bucket;
    int spriteIndex;

    for (pageListIndex = 0; pageListIndex < var_pedGroupCount_8c228234; pageListIndex++) {
        if (groups[pageListIndex].active_0x00 == 0) {
            continue;
        }

        for (slot = groups[pageListIndex].list_0x08; slot->action != NULL; slot++) {
            if (slot->action == (TaskAction)-1) {
                continue;
            }

            ped = (PedestrianState *)slot->state;
            facing = ped->nReverse_0x40;

            if (ped->nKind_0x3c == 2) {
                switch (facing) {
                    case 0: spriteIndex = 0x20; break;
                    case 1: spriteIndex = 0x21; break;
                    case 2: spriteIndex = 0x22; break;
                    case 3: spriteIndex = 0x23; break;
                }
            } else {
                bucket = ped->nNodeFlags_0x60 & 0x30000000;
                if (facing == 1) {
                    switch (bucket) {
                        case 0: spriteIndex = 0; break;
                        case 0x10000000: spriteIndex = 8; break;
                        case 0x20000000: spriteIndex = 0x10; break;
                        case 0x30000000: spriteIndex = 0x18; break;
                    }
                } else {
                    switch (bucket) {
                        case 0: spriteIndex = 8; break;
                        case 0x10000000: spriteIndex = 0; break;
                        case 0x20000000: spriteIndex = 0x18; break;
                        case 0x30000000: spriteIndex = 0x10; break;
                    }
                }
                spriteIndex += (ped->nAnimPhase_0x5c >> 2) & 7;
            }

            njDrawSprite3D(&ped->sprite_0x00, spriteIndex, 0x30);
        }
    }
}

/* A walker's facing bucket is the angle between the bus's direction of travel
 * and its path node's, cached per node for the frame. */
STATIC void drawPedestrians_8c028b74(int arg0)
{
    PedGroupEntry *groups = (PedGroupEntry *)var_pedGroups_8c228230;
    DirCacheEntry *cache = (DirCacheEntry *)syMalloc(DIR_CACHE_CAPACITY * sizeof(DirCacheEntry));
    DirCacheEntry *cacheEnd = cache;
    DirCacheEntry *found;
    Angle selfAngle;
    Angle targetAngle;
    int diff;
    int pageListIndex;
    Task *slot;
    PedestrianState *ped;
    int facing;
    PedPathNode *key;
    int bucket;
    int spriteIndex;

    selfAngle = njArcTan2(-var_busState_8c1bb9d0.moveDeltaX_0x308, -var_busState_8c1bb9d0.moveDeltaZ_0x310);

    for (pageListIndex = 0; pageListIndex < var_pedGroupCount_8c228234; pageListIndex++) {
        if (groups[pageListIndex].active_0x00 == 0) {
            continue;
        }

        for (slot = groups[pageListIndex].list_0x08; slot->action != NULL; slot++) {
            if (slot->action == (TaskAction)-1) {
                continue;
            }

            ped = (PedestrianState *)slot->state;
            facing = ped->nReverse_0x40;

            if (ped->nKind_0x3c == 2) {
                switch (facing) {
                    case 0: spriteIndex = 0x20; break;
                    case 1: spriteIndex = 0x21; break;
                    case 2: spriteIndex = 0x22; break;
                    case 3: spriteIndex = 0x23; break;
                }
            } else {
                key = ped->pPathNode_0x4c;

                for (found = cache; found < cacheEnd && found->key != key; found++) {
                }

                if (cacheEnd <= found) {
                    if (cache + DIR_CACHE_CAPACITY <= cacheEnd) {
                        continue; /* cache full: not drawn */
                    }
                    found->key = key;
                    targetAngle = njArcTan2(-key->flStepX_0x0c, -key->flStepZ_0x10);
                    diff = selfAngle - targetAngle;
                    if ((diff < -0x2000 || 0x1fff < diff) &&
                        diff < 0xe000 && -0xe001 < diff) {
                        if ((diff < 0x2000 || 0x5fff < diff) &&
                            (diff < -0xe000 || -0xa001 < diff)) {
                            if ((diff < -0x6000 || -0x2001 < diff) &&
                                (diff < 0xa000 || 0xdfff < diff)) {
                                found->bucket = 0x10000000;
                            } else {
                                found->bucket = 0x30000000;
                            }
                        } else {
                            found->bucket = 0x20000000;
                        }
                    } else {
                        found->bucket = 0;
                    }
                    cacheEnd++;
                }

                bucket = found->bucket;
                if (facing == 0) {
                    switch (bucket) {
                        case 0: spriteIndex = 0; break;
                        case 0x10000000: spriteIndex = 8; break;
                        case 0x20000000: spriteIndex = 0x10; break;
                        case 0x30000000: spriteIndex = 0x18; break;
                    }
                } else {
                    switch (bucket) {
                        case 0: spriteIndex = 8; break;
                        case 0x10000000: spriteIndex = 0; break;
                        case 0x20000000: spriteIndex = 0x18; break;
                        case 0x30000000: spriteIndex = 0x10; break;
                    }
                }
                spriteIndex += (ped->nAnimPhase_0x5c >> 2) & 7;
            }

            njDrawSprite3D(&ped->sprite_0x00, spriteIndex, 0x30);
        }
    }

    syFree(cache);
}

/* macHumM0: 0-terminated offsets, made absolute in place. */
void ObjectsRelocatePedGroupLists_8c028dd0(void *handle)
{
    int *entry = handle;
    while (*entry != 0) {
        *entry += (int)handle;
        entry++;
    }
}
/* macHumG0: makes each PedGroupDef's specs_0x08 offset absolute. */
void ObjectsRelocatePedGroupDefs_8c028de8(void *handle)
{
    int *entry = handle;
    while (entry[0] != -1) {
        entry[2] += (int)handle;
        entry += 3;
    }
}
STATIC void pedestrianTask_8c028e00(Task *task, PedestrianState *ped)
{
    int state;
    Bool shouldMove;
    Bool crossingStarted;
    int *entry;
    float pos[2];

    state = ped->nState_0x64;
    shouldMove = TRUE;
    crossingStarted = FALSE;

    if (state == 0 && (ped->nNodeFlags_0x60 & 0xfff) == 0) {
        /* Nothing ahead on this node: keep walking. */
    } else if (state == 0 || state == 1) {
        if (state == 0) {
            ped->nState_0x64 = 1;
            ped->nSignalIdA_0x68 = (int)(short)((ped->nNodeFlags_0x60 >> 16) & 0xfff);
            ped->nSignalIdB_0x6c = ped->nNodeFlags_0x60 & 0xfff;
            ped->nAnimPhase_0x5c = 0;
        }
        /* Frame 1 is green: a pedestrian arriving mid-green waits for the next one. */
        if (SignalGetFrame_8c028900(ped->nSignalIdA_0x68) != 1) {
            ped->nState_0x64 = 2;
        }
        shouldMove = FALSE;
    } else if (state == 4 ||
               (state == 2 && SignalGetFrame_8c028900(ped->nSignalIdA_0x68) == 1 &&
                SignalIsCrossingOccupied_8c02898e(ped->nSignalIdB_0x6c) == 0)) {
        if (state == 2) {
            ped->nState_0x64 = 4;
        }
        if ((ped->nNodeFlags_0x60 & 0xfff) == 0) {
            ped->nState_0x64 = 0;
        } else {
            for (entry = var_blockedCrosswalks_8c228248; entry < var_blockedCrosswalksEnd_8c228244; entry += 2) {
                if (ped->pPathNode_0x4c == (PedPathNode *)*entry) {
                    pos[0] = ped->sprite_0x00.p.x;
                    pos[1] = ped->sprite_0x00.p.z;
                    if (ped->nReverse_0x40 == 0) {
                        if (IntersectSegments_8c0206f0(var_busAxisA_8c228268, var_busAxisB_8c228270, pos,
                                (float *)(entry[1] + 4), &var_crossingIntersectPoint_8c1bc458) != 0) {
                            shouldMove = FALSE;
                        }
                    } else {
                        if (IntersectSegments_8c0206f0(var_busAxisA_8c228268, var_busAxisB_8c228270, pos,
                                (float *)(*entry + 4), &var_crossingIntersectPoint_8c1bc458) != 0) {
                            shouldMove = FALSE;
                        }
                    }
                    break;
                }
            }
            SignalMarkPedCrossing_8c02897a(ped->nSignalIdB_0x6c);
            CollisionQueueAdd_8c02e48e(ped);
            crossingStarted = TRUE;
        }
    } else if (state == 2) {
        shouldMove = FALSE;
    }

    if (shouldMove) {
        state = ped->nState_0x64;
        if (state == 0) {
            if (ped->nReverse_0x40 == 0) {
                ped->flPathPos_0x54 = ped->flPathPos_0x54 + ped->flSpeed_0x58;
            } else {
                ped->flPathPos_0x54 = ped->flPathPos_0x54 - ped->flSpeed_0x58;
            }
        } else if (state == 4) {
            if (SignalGetFrame_8c028900(ped->nSignalIdA_0x68) == 1) {
                if (ped->nReverse_0x40 == 0) {
                    ped->flPathPos_0x54 = ped->flPathPos_0x54 + ped->flSpeed_0x58 * 2.0f;
                } else {
                    ped->flPathPos_0x54 = ped->flPathPos_0x54 - ped->flSpeed_0x58 * 2.0f;
                }
            } else {
                if (ped->nReverse_0x40 == 0) {
                    ped->flPathPos_0x54 = ped->flPathPos_0x54 + ped->flSpeed_0x58 * 3.0f;
                } else {
                    ped->flPathPos_0x54 = ped->flPathPos_0x54 - ped->flSpeed_0x58 * 3.0f;
                }
                ped->nAnimPhase_0x5c = ped->nAnimPhase_0x5c + 1;
            }
        }
        ped->nAnimPhase_0x5c = ped->nAnimPhase_0x5c + 1;

        if ((advancePedPathPos_8c0289ac(ped) || (ped->nKind_0x3c == 1 && !crossingStarted)) &&
            (GroundProbeTrackPolygon_8c020b6c(ped->sprite_0x00.p.x - ped->flBaseX_0x20, ped->sprite_0x00.p.y,
                          ped->sprite_0x00.p.z - ped->flBaseZ_0x24, &ped->aGround_0x28),
             ped->aGround_0x28.count_0x0c != 0)) {
            GroundProbeInterpolateHeight_8c020f7e(&ped->aGround_0x28, (float *)ped);
        }
    }
}
STATIC void pedStaticObjectTask_8c02903e() {}

/* Spawns one pedestrian per frame from the group's spec list. */
STATIC void pedGroupTask_8c029078(PedGroupTask *task)
{
    PedGroupEntry *group;
    PedGroupSpawnSpec *spec;
    PedPathInfo *path;
    Task *subTask;
    PedestrianState *state;
    float radius;
    float targetDist;
    PedPathNode *node;
    int pageListIndex;
    int pushFailed;

    pageListIndex = task->pageListIndex_0x08;
    group = &((PedGroupEntry *)var_pedGroups_8c228230)[pageListIndex];

    if (group->wanted_0x04 == 0) {
        group->active_0x00 = 0;
        TaskUninstallGroup_8c014ab4(task->subTasks_0x18);
        syFree(task->subTasks_0x18);
        TaskUninstall_8c014b66((Task *)task);
        return;
    }

    spec = task->spec_0x1c;
    if (spec->nKindId_0x00 != 0xffff) {
        path = &((PedPathInfo *)var_pedPaths_8c228238)[pageListIndex];

        if (spec->flX_0x04 == 0.0f || spec->flX_0x04 <= path->flLength_0x08) {
            /* Faithful to the original: an unrecognised nKind falls into the
             * sprite setup below with subTask/state still uninitialised. Only a
             * failed TaskInstall skips it. */
            pushFailed = 0;
            if (spec->nKind_0x02 == 0 || spec->nKind_0x02 == 1) {
                if (TaskInstall_8c014ae8(task->subTasks_0x18, pedestrianTask_8c028e00,
                        &subTask, (void **)&state, sizeof(PedestrianState))) {
                    radius = task->radius_0x10;
                    state->flBaseX_0x20 = ((float)rand() / 32768.0f) * radius * 2.0f - radius;
                    state->flBaseZ_0x24 = ((float)rand() / 32768.0f) * radius * 2.0f - radius;
                    /* 0.04 to 0.06 units per frame */
                    state->flSpeed_0x58 =
                            ((float)(Uint16)AsqGetRandomA_8c012166() / 65536.0f) / 50.0f + 0.04f;
                    state->nAnimPhase_0x5c = AsqGetRandomA_8c012166();
                    state->nState_0x64 = 0;
                    state->pPathFirst_0x44 = path->pFirst_0x00;
                    state->pPathLast_0x48 = path->pLast_0x04;
                    state->flPathLength_0x50 = path->flLength_0x08;

                    if (spec->flX_0x04 == 0.0f) {
                        targetDist = ((float)rand() / 32768.0f) * state->flPathLength_0x50;
                    } else if (spec->nReverse_0x03 == 0) {
                        targetDist = spec->flX_0x04;
                    } else {
                        targetDist = state->flPathLength_0x50 - spec->flX_0x04;
                    }

                    node = state->pPathFirst_0x44;
                    while (node->flLength_0x00 <= targetDist) {
                        targetDist -= node->flLength_0x00;
                        node++;
                        if (node->flLength_0x00 == 0.0f) {
                            node = state->pPathFirst_0x44;
                        }
                    }

                    state->nNodeFlags_0x60 = node->nFlags_0x14;
                    while ((state->nNodeFlags_0x60 & 0xfff) != 0) {
                        if (spec->nReverse_0x03 == 0) {
                            if (node == state->pPathFirst_0x44) {
                                node = state->pPathLast_0x48;
                            } else {
                                node--;
                            }
                            targetDist = node->flLength_0x00;
                        } else {
                            targetDist = 0.0f;
                            if (node == state->pPathLast_0x48) {
                                node = state->pPathFirst_0x44;
                            } else {
                                node++;
                            }
                        }
                        state->nNodeFlags_0x60 = node->nFlags_0x14;
                    }

                    state->flPathPos_0x54 = targetDist;
                    state->pPathNode_0x4c = node;
                    state->nKindId_0x38 = spec->nKindId_0x00;
                    state->nKind_0x3c = spec->nKind_0x02;
                    state->nReverse_0x40 = spec->nReverse_0x03;

                    advancePedPathPos_8c0289ac(state);
                    GroundQueryFindPolygon_8c020914(state->sprite_0x00.p.x, state->sprite_0x00.p.y, state->sprite_0x00.p.z,
                            &state->aGround_0x28);
                    GroundProbeInterpolateHeight_8c020f7e(&state->aGround_0x28, (float *)state);
                } else {
                    pushFailed = 1;
                }
            } else if (spec->nKind_0x02 == 2) {
                if (TaskInstall_8c014ae8(task->subTasks_0x18, pedStaticObjectTask_8c02903e,
                        &subTask, (void **)&state, sizeof(PedestrianState))) {
                    state->flBaseX_0x20 = 0.0f;
                    state->flBaseZ_0x24 = 0.0f;
                    state->sprite_0x00.p.x = spec->flX_0x04;
                    state->sprite_0x00.p.z = spec->flZ_0x08;

                    GroundQueryFindPolygon_8c020914(state->sprite_0x00.p.x, 0.0f, state->sprite_0x00.p.z, &state->aGround_0x28);
                    GroundProbeInterpolateHeight_8c020f7e(&state->aGround_0x28, (float *)state);

                    state->nKindId_0x38 = spec->nKindId_0x00;
                    state->nKind_0x3c = spec->nKind_0x02;
                    state->nReverse_0x40 = spec->nReverse_0x03;
                } else {
                    pushFailed = 1;
                }
            }

            if (!pushFailed) {
                state->sprite_0x00.sx = 0.015f;
                state->sprite_0x00.sy = 0.015f;
                state->sprite_0x00.ang = 0;

                if (var_pedestrianAssets_8c1bbfdc[state->nKindId_0x38].texlist_0x08 == (NJS_TEXLIST *)-1) {
                    TaskUninstall_8c014b66(subTask);
                } else {
                    state->sprite_0x00.tlist = var_pedestrianAssets_8c1bbfdc[state->nKindId_0x38].texlist_0x08;
                    state->sprite_0x00.tanim = init_pedestrianTexAnims_8c04623c;
                }
            }
        }

        task->spec_0x1c = spec + 1;
    }

    TaskRunGroup_8c014b42(task->subTasks_0x18);
}

STATIC void pedestriansTask_8c0293f6(PedestriansTask *task)
{
    PedGroupEntry *groups;
    PedGroupEntry *group;
    PedGroupDef *groupDef;
    PedPathInfo *paths;
    PedGroupTask *subTask;
    void *state;
    Task *subtasks;
    int *list;
    PedPathNode *node;
    int *cursor;
    int presetField;
    int pageListIndex;
    int i;
    Bool isDemo;
    int layer;
    DrawFn fn;

    if (var_runState_8c2285c4.runPhase_0x00 == 0) {
        return;
    }

    var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariHum_0x28;

    presetField = var_busState_8c1bb9d0.scenePresetIds_0x3bc & 0xff0000;
    if (task->armed_0x0c == 0) {
        if (presetField == 0) {
            task->armed_0x0c = 1;
        }
    } else if (presetField != 0) {
        var_activePedPreset_8c22822c = (int)(short)(presetField >> 16);
        task->armed_0x0c = 0;
    }

    groups = (PedGroupEntry *)var_pedGroups_8c228230;

    if (task->lastPreset_0x08 != var_activePedPreset_8c22822c) {
        task->lastPreset_0x08 = var_activePedPreset_8c22822c;

        for (i = 0; i < var_pedGroupCount_8c228234; i++) {
            groups[i].wanted_0x04 = 0;
        }

        list = ((int **)var_pedGroupLists_8c228240)[var_activePedPreset_8c22822c];
        for (; *list != -1; list++) {
            pageListIndex = *list;
            group = &groups[pageListIndex];

            if (group->active_0x00 == 0) {
                /* +1 for TaskInitGroup_8c014a9c's terminator */
                subtasks = syMalloc((PED_GROUP_SLOTS + 1) * sizeof(Task));
                if (subtasks == NULL) {
                    break;
                }
                group->list_0x08 = subtasks;
                TaskInitGroup_8c014a9c(subtasks, PED_GROUP_SLOTS);

                if (!TaskInstall_8c014ae8(var_tasks_8c1ba808, pedGroupTask_8c029078,
                        (Task **)&subTask, &state, 0)) {
                    syFree(subtasks);
                    break;
                }

                for (groupDef = (PedGroupDef *)var_pedGroupDefs_8c22823c;
                        groupDef->id_0x00 != pageListIndex; groupDef++) {
                }

                subTask->subTasks_0x18 = subtasks;
                subTask->spec_0x1c = groupDef->specs_0x08;
                subTask->pageListIndex_0x08 = pageListIndex;
                subTask->radius_0x10 = groupDef->radius_0x04;
                group->active_0x00 = 1;
            }

            group->wanted_0x04 = 1;
        }
    }

    CollisionQueueReset_8c02e486();
    SignalClearPedCrossing_8c02890c();

    njCalcPoint(&var_busState_8c1bb9d0.worldMatrix_0x084, &init_busAxisLocalA_8c04650c, &var_groundQueryPoint_8c1bc460);
    var_busAxisA_8c228268[0] = var_groundQueryPoint_8c1bc460.x;
    var_busAxisA_8c228268[1] = var_groundQueryPoint_8c1bc460.z;

    njCalcPoint(&var_busState_8c1bb9d0.worldMatrix_0x084, &init_busAxisLocalB_8c046518, &var_groundQueryPoint_8c1bc460);
    var_busAxisB_8c228270[0] = var_groundQueryPoint_8c1bc460.x;
    var_busAxisB_8c228270[1] = var_groundQueryPoint_8c1bc460.z;

    var_blockedCrosswalksEnd_8c228244 = var_blockedCrosswalks_8c228248;
    paths = (PedPathInfo *)var_pedPaths_8c228238;

    for (pageListIndex = 0; pageListIndex < var_pedGroupCount_8c228234; pageListIndex++) {
        if (groups[pageListIndex].active_0x00 != 0) {
            for (node = paths[pageListIndex].pFirst_0x00; node->flLength_0x00 != 0.0f; node++) {
                if ((node->nFlags_0x14 & 0xfff) != 0 &&
                        IntersectSegments_8c0206f0(var_busAxisA_8c228268, var_busAxisB_8c228270,
                                &node->flBaseX_0x04, &node[1].flBaseX_0x04,
                                &var_crossingIntersectPoint_8c1bc458)) {
                    cursor = var_blockedCrosswalksEnd_8c228244;
                    var_blockedCrosswalksEnd_8c228244 = cursor + 1;
                    *cursor = (int)node;

                    cursor = var_blockedCrosswalksEnd_8c228244;
                    var_blockedCrosswalksEnd_8c228244 = cursor + 1;
                    *cursor = (int)(node + 1);
                }
            }
        }
    }

    TaskRunGroup_8c014b42(var_tasks_8c1ba808);

    isDemo = var_playMode_8c1bb8d0 == PLAY_MODE_DEMO;
    if (isDemo) {
        fn = drawPedestrians_8c028b74;
    } else {
        RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, drawPedestrians_8c028b74, 0);
        fn = drawPedestriansMirror_8c028a38;
    }
    layer = !isDemo;
    RenderQueueDraw_8c0223ea(layer, fn, layer);

    if (var_runState_8c2285c4.stopPhase_0x20 == 2) {
        RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, StopDrawWaitingPassengers_8c02d06c, 0);
        RenderQueueDraw_8c0223ea(RENDER_LAYER_MIRROR, StopDrawWaitingPassengers_8c02d06c, 1);
    }
}

void ObjectsInstallPedestrianGroups_8c0296d6(void)
{
    PedGroupEntry *groups;
    int **lists;
    int i, j;
    PedestriansTask *task;
    void *state;

    var_pedPaths_8c228238 = var_currentCourse_8c1bb868.lineHum_0x2c;
    var_pedGroupLists_8c228240 = var_currentCourse_8c1bb868.macHumM0_0x34;
    var_pedGroupDefs_8c22823c = var_currentCourse_8c1bb868.macHumG0_0x30;

    lists = (int **)var_pedGroupLists_8c228240;
    var_pedGroupCount_8c228234 = -1;
    for (i = 0; lists[i] != NULL; i++) {
        for (j = 0; lists[i][j] != -1; j++) {
            if (var_pedGroupCount_8c228234 < lists[i][j]) {
                var_pedGroupCount_8c228234 = lists[i][j];
            }
        }
    }

    if (var_pedGroupCount_8c228234 < 0) {
        CollisionQueueReset_8c02e486();
        return;
    }

    var_pedGroupCount_8c228234++;
    groups = (PedGroupEntry *)syMalloc(var_pedGroupCount_8c228234 * sizeof(PedGroupEntry));
    var_pedGroups_8c228230 = groups;
    for (i = 0; i < var_pedGroupCount_8c228234; i++) {
        groups[i].active_0x00 = 0;
    }

    TaskInstall_8c014ae8(var_tasks_8c1ba5e8, pedestriansTask_8c0293f6, (Task **)&task, &state, 0);
    task->lastPreset_0x08 = -1;
    task->armed_0x0c = 1;
}

void ObjectsUninstallPedestrianGroups_8c0297da(void)
{
    PedGroupEntry *groups;
    int i;

    if (var_pedGroupCount_8c228234 < 0) {
        return;
    }

    groups = (PedGroupEntry *)var_pedGroups_8c228230;
    for (i = 0; i < var_pedGroupCount_8c228234; i++) {
        if (groups[i].active_0x00 != 0) {
            TaskUninstallGroup_8c014ab4(groups[i].list_0x08);
            syFree(groups[i].list_0x08);
        }
    }

    syFree(var_pedGroups_8c228230);
    var_pedGroups_8c228230 = (void *)-1;
    var_pedGroupCount_8c228234 = -1;
}

typedef struct {
    TaskAction action;
    void *state;            /* NJS_MATRIX[count_0x08] */
    int count_0x08;
    int blinkCounter_0x0c;
    int field_0x10;
    int field_0x14;
    void *queuedItem_0x18;
    int field_0x1c;
} RouteBlinkerTask;

STATIC void resolveObjectChildren_8c029868(NJS_OBJECT **nodes)
{
    NJS_OBJECT *child = nodes[0]->child;
    nodes[1] = child;
    child = child->sibling;
    nodes[2] = child;
    nodes[3] = child->sibling;
}

/* The three lamps light in turn, 4 frames each. */
STATIC void drawBlinkers_8c029878(int taskArg, int matricesArg)
{
    RouteBlinkerTask *task = (RouteBlinkerTask *)taskArg;
    NJS_MATRIX *matrix = (NJS_MATRIX *)matricesArg;
    int count = task->count_0x08;
    int phase = (task->blinkCounter_0x0c >> 2) % 3;
    int i;

    switch (phase) {
    case 0:
        var_routeBlinkerNodes_8c228278[1]->evalflags = 0x3f;
        var_routeBlinkerNodes_8c228278[2]->evalflags = 0x3f;
        var_routeBlinkerNodes_8c228278[3]->evalflags = 0x37;
        break;
    case 1:
        var_routeBlinkerNodes_8c228278[1]->evalflags = 0x3f;
        var_routeBlinkerNodes_8c228278[2]->evalflags = 0x37;
        var_routeBlinkerNodes_8c228278[3]->evalflags = 0x3f;
        break;
    case 2:
        var_routeBlinkerNodes_8c228278[1]->evalflags = 0x37;
        var_routeBlinkerNodes_8c228278[2]->evalflags = 0x3f;
        var_routeBlinkerNodes_8c228278[3]->evalflags = 0x3f;
        break;
    default:
        break;
    }

    for (i = 0; i < count; i++) {
        njSetCamera(var_drawCamera_8c226558);
        njMultiMatrix(0, matrix);
        njCnkSimpleDrawObject((NJS_CNK_OBJECT *)var_routeBlinkerNodes_8c228278[0]);
        matrix++;
    }
}

STATIC void routeBlinkerTask_8c029904(RouteBlinkerTask *task, NJS_MATRIX *state)
{
    task->blinkCounter_0x0c++;
    RenderQueueDraw2_8c022420(RENDER_LAYER_MAIN, drawBlinkers_8c029878, (int)task, (int)state);
}

/* Original quirk: a hit on the pedestrian ground grid leaves the point's height
 * unset (stale from the previous point); only the bus-grid fallback
 * interpolates one. */
void ObjectsInstallBlinkers_8c029920(void)
{
    const BlinkerPoint *p, *points;
    int count;
    RouteBlinkerTask *task;
    NJS_MATRIX *matrices;

    var_routeBlinkerNodes_8c228278[0] = (NJS_OBJECT *)((int *)var_routeModels_8c1bc3ec)[9];
    resolveObjectChildren_8c029868(var_routeBlinkerNodes_8c228278);

    switch (var_route_8c18ad1c) {
    case ROUTE_WANGAN:
        points = init_wanganBlinkerPoints_8c0465d8;
        break;
    case ROUTE_SHINJUKU:
        points = init_shinjukuBlinkerPoints_8c046524;
        break;
    case ROUTE_OME:
        points = init_omeBlinkerPoints_8c0466a4;
        break;
    }

    count = 0;
    for (p = points; p->x_0x00 != 0.0f; p++) {
        count++;
    }

    if (count == 0) {
        return;
    }

    TaskInstall_8c014ae8(var_tasks_8c1ba5e8, &routeBlinkerTask_8c029904, (Task **)&task,
                       (void **)&matrices, count * sizeof(NJS_MATRIX));
    task->count_0x08 = count;
    task->blinkCounter_0x0c = 0;

    for (p = points; p->x_0x00 != 0.0f; p++) {
        GroundQueryResult result;

        var_groundQueryPoint_8c1bc460.x = p->x_0x00;
        var_groundQueryPoint_8c1bc460.z = p->z_0x04;

        var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariHum_0x28;
        GroundQueryFindPolygon_8c020914(var_groundQueryPoint_8c1bc460.x, var_groundQueryPoint_8c1bc460.y, var_groundQueryPoint_8c1bc460.z, &result);
        if (result.count_0x0c == 0) {
            var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariBus_0x04;
            GroundQueryFindPolygon_8c020914(var_groundQueryPoint_8c1bc460.x, var_groundQueryPoint_8c1bc460.y, var_groundQueryPoint_8c1bc460.z, &result);
            if (result.count_0x0c != 0) {
                GroundProbeInterpolateHeight_8c020f7e(&result, (float *)&var_groundQueryPoint_8c1bc460);
            } else {
                var_groundQueryPoint_8c1bc460.y = var_busState_8c1bb9d0.posY_0x0f8;
            }
        }

        njUnitMatrix(matrices);
        njTranslate(matrices, var_groundQueryPoint_8c1bc460.x, var_groundQueryPoint_8c1bc460.y, var_groundQueryPoint_8c1bc460.z);
        njRotateY(matrices, (int)((float)p->angleDeg_0x08 * 65536.0f / 360.0f));
        matrices++;
    }
}
void ObjectsClearAssetRequestTable_8c029acc(void)
{
    var_assetRequestTable_8c228408 = (int *)-1;
}

void ObjectsStartAssetRequests_8c029ad4(int *table)
{
    AssetRequestSlot *slots;
    AssetRequestSlot *slot;
    int rowType;
    char *name;
    int rowIndex;
    int *data;

    if (table == NULL) {
        ObjectsFreeAssetRequests_8c029cfe();
        return;
    }
    if (var_assetRequestTable_8c228408 == table) {
        return;
    }

    ObjectsFreeAssetRequests_8c029cfe();
    slots = (AssetRequestSlot *)var_assetRequestSlots_8c228288;
    rowIndex = 0;
    var_assetRequestTable_8c228408 = table;
    for (; *table != -1; table = table + 2) {
        rowType = *table;
        slot = &slots[rowIndex];
        if (rowType == 0) {
            name = *(char **)(table[1] + 0xc);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, name, &slot->dat_0x08, 0);
        }
        else if (rowType == 1) {
            data = (int *)table[1];
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, (void *)data[0], &slot->nj_0x04, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, (void *)data[1], &slot->pvm_0x00,
                data[2], 0);
            AsqRequestDat_8c011182(var_commonDirCopy_8c18ad8c, (void *)data[3], &slot->dat_0x08);
        }
        else if (rowType == 2) {
            data = (int *)table[1];
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, (void *)data[0], &slot->nj_0x04, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, (void *)data[1], &slot->pvm_0x00,
                data[2], 0);
            slot->sceneGate_0x14 = ((Sint8 *)data)[0xc];
            slot->taskFlag_0x15 = ((Sint8 *)data)[0xd];
            slot->fogEnable_0x16 = ((Uint8 *)data)[0xe];
            slot->control3DEnable_0x17 = ((Uint8 *)data)[0xf];
        }
        else if (rowType == 3) {
            data = (int *)table[1];
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, (void *)data[0], &slot->nj_0x04, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, (void *)data[1], &slot->pvm_0x00,
                data[2], 0);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, (void *)data[3], &slot->dat_0x08, 0);
            slot->sceneGate_0x14 = ((Sint8 *)data)[0x10];
            slot->taskFlag_0x15 = ((Sint8 *)data)[0x11];
            slot->fogEnable_0x16 = ((Uint8 *)data)[0x12];
            slot->control3DEnable_0x17 = ((Uint8 *)data)[0x13];
        }
        else if (rowType == 4) {
            data = (int *)table[1];
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, (void *)data[0], &slot->nj_0x04, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, (void *)data[1], &slot->pvm_0x00,
                data[2], 0);
            slot->taskFlag_0x15 = (Sint8)data[3];
        }
        else if (rowType == 5) {
            data = (int *)table[1];
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, (void *)data[0], &slot->nj_0x04, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, (void *)data[1], &slot->pvm_0x00,
                data[2], 0);
            slot->pos_0x0c = *(ObjectAssetType5Extra *)((Uint8 *)data + 0xc);
        }
        else if (rowType == 6) {
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, "O_FUMI_00.njd", &var_fumiGateModel_8c22840c, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, "O_FUMI.pvm", &var_fumiTexlist_8c228410, 2, 0);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, "O_FUMI_00.njm", &var_fumiCloseMotion_8c228414, 0);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, "O_FUMI_01.njm", &var_fumiOpenMotion_8c228418, 0);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, "O_FUMI_LAMP.njd", &var_fumiLampModel_8c22841c, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, "O_FUMI_LAMP.pvm", &var_fumiLampTexlist_8c228420, 0x10, 0);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, "od_chu00.njd", &var_fumiTrainModel_8c228424, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, "od_chu00.pvm", &var_fumiTrainTexlist_8c228428, 0x10, 0);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, "o01_tra0.njm", &var_fumiTrainMotionA_8c22842c, 0);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, "o01_tra1.njm", &var_fumiTrainMotionB_8c228430, 0);
        }
        rowIndex = rowIndex + 1;
    }
}
void ObjectsFreeAssetRequests_8c029cfe(void)
{
    AssetRequestSlot *slots;
    AssetRequestSlot *slot;
    int *entry;
    int rowIndex;
    int type;

    if (var_assetRequestTable_8c228408 == (int *)-1) {
        return;
    }

    slots = (AssetRequestSlot *)var_assetRequestSlots_8c228288;
    rowIndex = 0;
    for (entry = var_assetRequestTable_8c228408; *entry != -1; entry += 2) {
        type = *entry;
        slot = &slots[rowIndex];

        if (type == 0) {
            syFree(slot->dat_0x08);
        } else if (type == 1 || type == 3) {
            syFree(slot->nj_0x04);
            AsqReleaseAndFreeTexlist_8c011e3c(slot->pvm_0x00);
            syFree(slot->dat_0x08);
        } else if (type == 2 || type == 4 || type == 5) {
            syFree(slot->nj_0x04);
            AsqReleaseAndFreeTexlist_8c011e3c(slot->pvm_0x00);
        } else if (type == 6) {
            syFree(var_fumiGateModel_8c22840c);
            AsqReleaseAndFreeTexlist_8c011e3c(var_fumiTexlist_8c228410);
            syFree(var_fumiCloseMotion_8c228414);
            syFree(var_fumiOpenMotion_8c228418);
            syFree(var_fumiLampModel_8c22841c);
            AsqReleaseAndFreeTexlist_8c011e3c(var_fumiLampTexlist_8c228420);
            syFree(var_fumiTrainModel_8c228424);
            AsqReleaseAndFreeTexlist_8c011e3c(var_fumiTrainTexlist_8c228428);
            syFree(var_fumiTrainMotionA_8c22842c);
            syFree(var_fumiTrainMotionB_8c228430);
        }

        rowIndex = rowIndex + 1;
    }

    var_assetRequestTable_8c228408 = (int *)-1;
}
STATIC void drawFlyByModel_8c029e46(int state)
{
    RowTaskState *st = (RowTaskState *)state;

    njSetTexture(st->texlist_0x40);
    njCnkSimpleDrawMotion(st->model_0x44, (NJS_MOTION *)st->dat_0x48, st->frame_0x58);
}
STATIC void flyByModelTask_8c029e68(Task *task, RowTaskState *state)
{
    state->frame_0x58 = state->frame_0x58 + 1.0f;
    if (state->frameLimit_0x5c <= state->frame_0x58) {
        TaskUninstall_8c014b66(task);
        return;
    }
    RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, drawFlyByModel_8c029e46, (int)state);
}
/* Every 16-63 frames, runs the row's motion once on a random route model. */
STATIC void rowFlyByTask_8c029e94(Task *task, RowTaskState *state)
{
    Task *newTask;
    RowTaskState *newState;
    int idx;

    if (--task->field_0x08 < 0) {
        if (TaskInstall_8c014ae8(var_tasks_8c1bb448, &flyByModelTask_8c029e68, &newTask,
                (void **)&newState, sizeof(RowTaskState))) {
            do {
                idx = AsqGetRandomInRangeA_8c012178(0xd) * 2 + 1;
            } while (var_routeModelSlots_8c1bbddc[idx].texlist_0x08 == (NJS_TEXLIST *)-1);
            newState->texlist_0x40 = var_routeModelSlots_8c1bbddc[idx].texlist_0x08;
            newState->model_0x44 = var_routeModelSlots_8c1bbddc[idx].nj_0x0c;
            newState->dat_0x48 = state->dat_0x48;
            newState->frame_0x58 = 0.0f;
            newState->frameLimit_0x5c = state->frameLimit_0x5c;
        }
        task->field_0x08 = AsqGetRandomInRangeA_8c012178(0x30) + 0x10;
    }
}
STATIC void drawDatModel_8c029f2a(int state)
{
    DatBlob *dat = (DatBlob *)((RowTaskState *)state)->dat_0x48;
    njSetTexture(dat->texlist_0x18);
    njCnkSimpleDrawObject(dat->model_0x1c);
}
STATIC void initDatBlob_8c029f42(DatBlob *dat, NJS_TEXLIST *texlist, NJS_CNK_OBJECT *model)
{
    dat->step_0x0c = 0;
    dat->tick_0x10 = 0;
    dat->texlist_0x18 = texlist;
    dat->model_0x1c = model;
    dat->masks_0x14 = dat->masks_0x20;
}
/* Returns 0, step unadvanced, if the model has fewer than nNodes_0x00
 * children. */
STATIC int advanceDatBlob_8c029f54(DatBlob *dat)
{
    int stepIndex;
    int tick;

    if (dat->tick_0x10 == 0) {
        NJS_CNK_OBJECT *node;
        int nodeIndex;

        node = dat->model_0x1c->child;
        for (nodeIndex = 1; nodeIndex < dat->nNodes_0x00; nodeIndex++) {
            if ((dat->masks_0x14[dat->step_0x0c] & (1 << nodeIndex)) == 0) {
                node->evalflags = 0x3e;
            }
            else {
                node->evalflags = 0x36;
            }
            if (node->sibling == NULL) {
                if (nodeIndex != dat->nNodes_0x00 - 1) {
                    return 0;
                }
            }
            else {
                node = node->sibling;
            }
        }
        stepIndex = dat->step_0x0c;
        dat->step_0x0c = stepIndex + 1;
        if (dat->stepCount_0x08 <= stepIndex + 1) {
            dat->step_0x0c = 0;
        }
    }
    tick = dat->tick_0x10;
    dat->tick_0x10 = tick + 1;
    if (dat->framesPerStep_0x04 <= tick + 1) {
        dat->tick_0x10 = 0;
    }
    return 1;
}
STATIC void rowDatTask_8c029fcc(Task *task, RowTaskState *state)
{
    advanceDatBlob_8c029f54((DatBlob *)state->dat_0x48);
    RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, drawDatModel_8c029f2a, (int)state);
    RenderQueueDraw_8c0223ea(RENDER_LAYER_MIRROR, drawDatModel_8c029f2a, (int)state);
}
STATIC void drawRowModel_8c02a048(int state)
{
    RowTaskState *st = (RowTaskState *)state;

    if (st->control3DEnable_0x79 == 0) {
        njControl3D(0);
    }
    if (st->fogEnable_0x78 == 0) {
        njFogDisable();
    }
    njSetTexture(st->texlist_0x40);
    njCnkSimpleDrawObject(st->model_0x44);
    njFogEnable();
    njControl3D(0x100);
}
/* Appears once scenePresetIds_0x3bc's top byte matches task->field_0x08. */
STATIC void rowModelTask_8c02a08a(Task *task, RowTaskState *state)
{
    if (state->phase_0x54 == 0) {
        if (task->field_0x08 == (int)(var_busState_8c1bb9d0.scenePresetIds_0x3bc & 0xff000000)) {
            state->phase_0x54 = state->phase_0x54 + 1;
        }
    } else if (state->phase_0x54 == 1) {
        RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, drawRowModel_8c02a048, (int)state);
        RenderQueueDraw_8c0223ea(RENDER_LAYER_MIRROR, drawRowModel_8c02a048, (int)state);
    }
}
STATIC void drawRowMotionModel_8c02a0d6(int state)
{
    RowTaskState *st = (RowTaskState *)state;

    if (st->control3DEnable_0x79 == 0) {
        njControl3D(0);
    }
    if (st->fogEnable_0x78 == 0) {
        njFogDisable();
    }
    njSetTexture(st->texlist_0x40);
    njCnkSimpleDrawMotion(st->model_0x44, (NJS_MOTION *)st->dat_0x48, st->frame_0x58);
    njFogEnable();
    njControl3D(0x100);
}
/* Triggered like rowModelTask_8c02a08a; task->field_0x0c loops the motion. */
STATIC void rowMotionModelTask_8c02a120(Task *task, RowTaskState *state)
{
    if (state->phase_0x54 == 0) {
        if (task->field_0x08 == (int)(var_busState_8c1bb9d0.scenePresetIds_0x3bc & 0xff000000)) {
            state->phase_0x54 = state->phase_0x54 + 1;
        }
    } else if (state->phase_0x54 == 1) {
        state->frame_0x58 = state->frame_0x58 + 1.0f;
        if (state->frame_0x58 < state->frameLimit_0x5c) {
            RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, drawRowMotionModel_8c02a0d6, (int)state);
            return;
        }
        if (task->field_0x0c == 0) {
            TaskUninstall_8c014b66(task);
            return;
        }
        state->frame_0x58 = 0.0f;
    }
}
STATIC void drawRowSimpleModel_8c02a1b2(int state, int fogEnable)
{
    RowTaskState *st = (RowTaskState *)state;

    njControl3D(0);
    if (fogEnable == 0) {
        njFogDisable();
    }
    njSetTexture(st->texlist_0x40);
    njCnkSimpleDrawObject(st->model_0x44);
    njFogEnable();
    njControl3D(0x100);
}
STATIC void rowSimpleModelTask_8c02a1f0(Task *task, RowTaskState *state)
{
    RenderQueueDraw2_8c022420(RENDER_LAYER_MAIN, drawRowSimpleModel_8c02a1b2, (int)state, task->field_0x08);
}
STATIC void drawRowMaterialModel_8c02a206(int state)
{
    RowTaskState *st = (RowTaskState *)state;

    njControl3D(0x920);
    njSetConstantAttr(0xffffffff, 0x800);
    njSetConstantMaterial(&st->material_0x68);
    njSetTexture(st->texlist_0x40);
    njCnkSimpleDrawObject(st->model_0x44);
    njControl3D(0x100);
}
/* Material alpha runs from -0.5 within 10 of the bus to 0 at 20 and beyond. */
STATIC void rowMaterialModelTask_8c02a27c(void *task, RowTaskState *state)
{
    ObjectAssetType5Extra *pos = &state->pos_0x4c;

    if (var_cameraMode_8c227d9c < 2) {
        state->material_0x68.a = 0.0f;
    } else {
        float dx = (var_busState_8c1bb9d0.posX_0x0f4 + var_busState_8c1bb9d0.posX_0x2fc) / 2.0f
                   - pos->posX_0x00;
        float dz = (var_busState_8c1bb9d0.posZ_0x0fc + var_busState_8c1bb9d0.posZ_0x304) / 2.0f
                   - pos->posZ_0x04;
        float dist = njSqrt(dx * dx + dz * dz);

        if (dist > 20.0f) {
            state->material_0x68.a = 0.0f;
        } else {
            dist = dist - 10.0f;
            if (dist < 0.0f) {
                dist = 0.0f;
            }
            state->material_0x68.a = dist * 0.05f - 0.5f;
        }
    }
    RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, drawRowMaterialModel_8c02a206, (int)state);
    RenderQueueDraw_8c0223ea(RENDER_LAYER_MIRROR, drawRowMaterialModel_8c02a206, (int)state);
}
/* nodes[1..8] and [9..16]: the children of nodes[0]'s first two children. */
STATIC void resolveObjectGrandchildren_8c02a322(NJS_OBJECT **nodes)
{
    NJS_OBJECT *node;
    int i;

    node = nodes[0]->child->child;
    nodes[1] = node;
    for (i = 2; i <= 8; i++) {
        node = node->sibling;
        nodes[i] = node;
    }

    node = nodes[0]->child->sibling->child;
    nodes[9] = node;
    for (i = 10; i <= 16; i++) {
        node = node->sibling;
        nodes[i] = node;
    }
}
/* Never called in the original. */
STATIC void setGrandchildEvalFlags_8c02a370(NJS_OBJECT **nodes, char selector)
{
    switch (selector) {
    case 0:
        nodes[1]->evalflags = 0x37;
        nodes[2]->evalflags = 0x37;
        nodes[3]->evalflags = 0x3f;
        nodes[4]->evalflags = 0x3f;
        nodes[5]->evalflags = 0x3f;
        nodes[6]->evalflags = 0x3f;
        nodes[7]->evalflags = 0x3f;
        nodes[8]->evalflags = 0x3f;
        nodes[9]->evalflags = 0x37;
        nodes[10]->evalflags = 0x37;
        nodes[11]->evalflags = 0x3f;
        nodes[12]->evalflags = 0x3f;
        nodes[13]->evalflags = 0x3f;
        nodes[14]->evalflags = 0x3f;
        nodes[15]->evalflags = 0x3f;
        break;
    case 1:
        nodes[1]->evalflags = 0x3f;
        nodes[2]->evalflags = 0x3f;
        nodes[3]->evalflags = 0x37;
        nodes[4]->evalflags = 0x37;
        nodes[5]->evalflags = 0x3f;
        nodes[6]->evalflags = 0x3f;
        nodes[7]->evalflags = 0x3f;
        nodes[8]->evalflags = 0x37;
        nodes[9]->evalflags = 0x3f;
        nodes[10]->evalflags = 0x3f;
        nodes[11]->evalflags = 0x37;
        nodes[12]->evalflags = 0x37;
        nodes[13]->evalflags = 0x3f;
        nodes[14]->evalflags = 0x3f;
        nodes[15]->evalflags = 0x3f;
        nodes[16]->evalflags = 0x37;
        return;
    case 2:
        nodes[1]->evalflags = 0x3f;
        nodes[2]->evalflags = 0x3f;
        nodes[3]->evalflags = 0x3f;
        nodes[4]->evalflags = 0x3f;
        nodes[5]->evalflags = 0x37;
        nodes[6]->evalflags = 0x37;
        nodes[7]->evalflags = 0x37;
        nodes[8]->evalflags = 0x3f;
        nodes[9]->evalflags = 0x3f;
        nodes[10]->evalflags = 0x3f;
        nodes[11]->evalflags = 0x3f;
        nodes[12]->evalflags = 0x3f;
        nodes[13]->evalflags = 0x37;
        nodes[14]->evalflags = 0x37;
        nodes[15]->evalflags = 0x37;
        break;
    default:
        return;
    }
    nodes[16]->evalflags = 0x3f;
}
STATIC void drawFumiCrossing_8c02a47c(int state)
{
    RowTaskState *st = (RowTaskState *)state;
    int phase = st->phase_0x54;

    njSetTexture((NJS_TEXLIST *)var_fumiTexlist_8c228410);
    if (phase == 0 || phase == 1) {
        njCnkSimpleDrawMotion((NJS_CNK_OBJECT *)var_fumiGateModel_8c22840c, (NJS_MOTION *)var_fumiCloseMotion_8c228414,
                              st->frame_0x58);
    } else if (phase == 2 || phase == 3 || phase == 4) {
        njCnkSimpleDrawMotion((NJS_CNK_OBJECT *)var_fumiGateModel_8c22840c, (NJS_MOTION *)var_fumiOpenMotion_8c228418,
                              st->frame_0x58);
    }

    njSetTexture((NJS_TEXLIST *)var_fumiLampTexlist_8c228420);
    njCnkSimpleDrawObject((NJS_CNK_OBJECT *)var_fumiLampModel_8c22841c);

    if (st->trainFrameLimit_0x64 > 0.0f) {
        njSetTexture((NJS_TEXLIST *)var_fumiTrainTexlist_8c228428);
        njCnkSimpleDrawMotion(st->model_0x44, (NJS_MOTION *)st->dat_0x48, st->trainFrame_0x60);
    }
}
/* Phases: 0 idle until scenePresetIds_0x3bc's top byte is set, 1 gate closing,
 * 2 train passing, 3 gate opening, 4 done. */
STATIC void fumiCrossingTask_8c02a4f8(void *task, RowTaskState *state)
{
    int phase = state->phase_0x54;

    if (phase != 0) {
        if (phase == 1) {
            state->frame_0x58 = state->frame_0x58 + 1.0f;
            if (state->frameLimit_0x5c <= state->frame_0x58) {
                state->phase_0x54 = state->phase_0x54 + 1;
                state->frame_0x58 = 0.0f;
                state->frameLimit_0x5c = (float)*(Uint32 *)((Uint8 *)var_fumiOpenMotion_8c228418 + 4) - 1.0f;
            }
        } else if (phase == 2) {
            state->trainFrame_0x60 = state->trainFrame_0x60 + 1.0f;
            if (state->trainFrameLimit_0x64 <= state->trainFrame_0x60) {
                state->phase_0x54 = state->phase_0x54 + 1;
                state->trainFrameLimit_0x64 = 0.0f;
            }
        } else if (phase == 3) {
            state->frame_0x58 = state->frame_0x58 + 1.0f;
            if (state->frameLimit_0x5c <= state->frame_0x58) {
                state->phase_0x54 = state->phase_0x54 + 1;
                state->frame_0x58 = state->frameLimit_0x5c;
            }
        }
        RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, drawFumiCrossing_8c02a47c, (int)state);
        return;
    }

    if ((var_busState_8c1bb9d0.scenePresetIds_0x3bc & 0xff000000) != 0) {
        state->phase_0x54 = state->phase_0x54 + 1;
        var_trafficSignalFrames_8c227e24[0] = 0;
    }
}
/* arg0 is the draw layer. */
STATIC void setSimpleLightCallback_8c02a5d0(int arg0)
{
    float *dir = (arg0 == 0) ? var_simpleLightDir_8c2264d8 : var_mirrorSimpleLightDir_8c2264e4;

    njCnkSetSimpleLight(dir[0], dir[1], dir[2]);
    njCnkSetSimpleLightIntensity(var_simpleLightIntensity_8c2264f0[0], var_simpleLightIntensity_8c2264f0[1]);
    njCnkSetSimpleLightColor(var_simpleLightColor_8c2264f8[0], var_simpleLightColor_8c2264f8[1], var_simpleLightColor_8c2264f8[2]);
}
STATIC void execRowTaskGroupTask_8c02a60e(void)
{
    if (var_runState_8c2285c4.runPhase_0x00 != 0) {
        RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, setSimpleLightCallback_8c02a5d0, 0);
        RenderQueueDraw_8c0223ea(RENDER_LAYER_MIRROR, setSimpleLightCallback_8c02a5d0, 1);
        TaskRunGroup_8c014b42(var_tasks_8c1bb448);
    }
}
void ObjectsInstall_8c02a6ac(void)
{
    AssetRequestSlot *slots;
    int *entry;
    int rowIndex;
    Task *task;
    RowTaskState *state;

    if (var_assetRequestTable_8c228408 == (int *)-1) {
        return;
    }

    slots = (AssetRequestSlot *)var_assetRequestSlots_8c228288;

    rowIndex = 0;
    for (entry = var_assetRequestTable_8c228408; *entry != -1; entry += 2) {
        int type = entry[0];
        AssetRequestSlot *row = &slots[rowIndex];
        NJS_TEXLIST *pvm = row->pvm_0x00;
        NJS_CNK_OBJECT *nj = row->nj_0x04;
        void *dat = row->dat_0x08;

        if (type == 0) {
            TaskInstall_8c014ae8(var_tasks_8c1bb448, &rowFlyByTask_8c029e94, &task, (void **)&state, sizeof(RowTaskState));
            state->dat_0x48 = dat;
            state->frameLimit_0x5c = (float)*(Uint32 *)((Uint8 *)dat + 4) - 1.0f;
            task->field_0x08 = 0;
        } else if (type == 1) {
            TaskInstall_8c014ae8(var_tasks_8c1bb448, &rowDatTask_8c029fcc, &task, (void **)&state, sizeof(RowTaskState));
            initDatBlob_8c029f42((DatBlob *)dat, pvm, nj);
            state->dat_0x48 = dat;
        } else if (type == 2) {
            TaskInstall_8c014ae8(var_tasks_8c1bb448, &rowModelTask_8c02a08a, &task, (void **)&state, sizeof(RowTaskState));
            state->phase_0x54 = 0;
            state->texlist_0x40 = pvm;
            state->model_0x44 = nj;
            task->field_0x08 = (int)row->sceneGate_0x14 << 0x18;
            task->field_0x0c = (void *)(int)row->taskFlag_0x15;
            state->fogEnable_0x78 = row->fogEnable_0x16;
            state->control3DEnable_0x79 = row->control3DEnable_0x17;
        } else if (type == 3) {
            TaskInstall_8c014ae8(var_tasks_8c1bb448, &rowMotionModelTask_8c02a120, &task, (void **)&state, sizeof(RowTaskState));
            state->phase_0x54 = 0;
            state->texlist_0x40 = pvm;
            state->model_0x44 = nj;
            state->dat_0x48 = dat;
            state->frame_0x58 = 0.0f;
            state->frameLimit_0x5c = (float)*(Uint32 *)((Uint8 *)dat + 4) - 1.0f;
            task->field_0x08 = (int)row->sceneGate_0x14 << 0x18;
            task->field_0x0c = (void *)(int)row->taskFlag_0x15;
            state->fogEnable_0x78 = row->fogEnable_0x16;
            state->control3DEnable_0x79 = row->control3DEnable_0x17;
        } else if (type == 4) {
            TaskInstall_8c014ae8(var_tasks_8c1bb448, &rowSimpleModelTask_8c02a1f0, &task, (void **)&state, sizeof(RowTaskState));
            state->texlist_0x40 = pvm;
            state->model_0x44 = nj;
            task->field_0x08 = (int)row->taskFlag_0x15;
        } else if (type == 5) {
            TaskInstall_8c014ae8(var_tasks_8c1bb448, &rowMaterialModelTask_8c02a27c, &task, (void **)&state, sizeof(RowTaskState));
            state->texlist_0x40 = pvm;
            state->model_0x44 = nj;
            state->pos_0x4c = row->pos_0x0c;
            state->material_0x68.b = 0.0f;
            state->material_0x68.g = 0.0f;
            state->material_0x68.r = 0.0f;
        } else if (type == 6) {
            void *chosen;

            /* Original bug: passes state (stale from the previous row), not
             * &state, as the out-param. */
            TaskInstall_8c014ae8(var_tasks_8c1bb448, &fumiCrossingTask_8c02a4f8, &task, (void **)state, sizeof(RowTaskState));
            state->phase_0x54 = 0;
            state->frame_0x58 = 0.0f;
            state->frameLimit_0x5c = (float)*(Uint32 *)((Uint8 *)var_fumiCloseMotion_8c228414 + 4) - 1.0f;
            state->trainFrame_0x60 = 0.0f;
            chosen = (AsqGetRandomInRangeA_8c012178(2) == 0) ? var_fumiTrainMotionB_8c228430 : var_fumiTrainMotionA_8c22842c;
            state->dat_0x48 = chosen;
            state->trainFrameLimit_0x64 = (float)*(Uint32 *)((Uint8 *)chosen + 4) - 1.0f;
            var_fumiLampNodes_8c228434[0] = (NJS_OBJECT *)var_fumiLampModel_8c22841c;
            resolveObjectGrandchildren_8c02a322(var_fumiLampNodes_8c228434);
        }

        rowIndex++;
    }

    TaskInstall_8c014ae8(var_tasks_8c1ba5e8, &execRowTaskGroupTask_8c02a60e, &task, (void **)&state, 0);
}
