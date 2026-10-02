/* @unit Objects */
/* 8c0289ac */
#include <shinobi.h>
#include <stdlib.h> /* rand */

#include "0289ac_objects.h"
#include "011120_asset_queues.h" /* AsqRequestNj_8c011492, AsqRequestPvm_8c011ac0, AsqRequestDat_8c011182 */
#include "013ae8_route.h" /* var_commonDirCopy_8c18ad8c, enum ROUTE */
#include "014a9c_tasks.h" /* Task */
#include "015034_text.h" /* enum PLAY_MODE */
#include "0206f0_intersect.h" /* IntersectSegments_8c0206f0 */
#include "020914_ground_query.h" /* GroundQueryFindPolygon_8c020914, GroundQueryResult */
#include "020b6c_ground_probe.h"
#include "021b9c_tile_draw.h"
#include "022464_render.h" /* RenderQueueDraw_8c0223ea, RenderQueueDraw2_8c022420 */
#include "028258_signal.h" /* SignalGetFrame_8c028900, SignalClearPedCrossing_8c02890c */
#include "02d06c_stop_draw.h" /* StopDrawWaitingPassengers_8c02d06c */
#include "02e400_collision.h" /* CollisionQueueReset_8c02e486, CollisionQueueAdd_8c02e48e */
#include "02b464_grading.h"
#include "024b4c_bus_camera.h"
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* One blinker spawn point, terminated by an all-zero entry. */
typedef struct {
    float x_0x00;
    float z_0x04;
    int angleDeg_0x08;
} BlinkerPoint;


/* One node of a pedestrian path, terminated by a node whose flLength_0x00 is 0.
 * A pedestrian flPathPos_0x54 units along the segment sits at
 * flBase + flPathPos * flStep. */
typedef struct {
    float flLength_0x00;
    float flBaseX_0x04;
    float flBaseZ_0x08;
    float flStepX_0x0c;
    float flStepZ_0x10;
    /* Bits 0-11/16-27: crosswalk signal ids, split into nSignalIdB_0x6c/A_0x68.
     * Bits 28-29: mirror-view facing bucket (drawPedestriansMirror_8c028a38). */
    int nFlags_0x14;
} PedPathNode;

/* Task state for a walking pedestrian, driven by pedestrianTask_8c028e00. */
typedef struct {
    NJS_SPRITE sprite_0x00;
    float flBaseX_0x20;
    float flBaseZ_0x24;
    GroundQueryResult aGround_0x28;
    int nKindId_0x38;
    int nKind_0x3c;
    int nReverse_0x40; /* also this pedestrian's sprite-facing quadrant */
    PedPathNode *pPathFirst_0x44;
    PedPathNode *pPathLast_0x48;
    PedPathNode *pPathNode_0x4c;
    float flPathLength_0x50;
    float flPathPos_0x54;
    float flSpeed_0x58;
    int nAnimPhase_0x5c;
    int nNodeFlags_0x60;   /* copy of pPathNode_0x4c's nFlags_0x14 */
    int nState_0x64;       /* 0 walking, 1 checking the signal, 2 waiting, 4 crossing */
    int nSignalIdA_0x68;   /* nNodeFlags_0x60's high 12 bits */
    int nSignalIdB_0x6c;   /* nNodeFlags_0x60's low 12 bits */
} PedestrianState;

/* Capacity of drawPedestrians_8c028b74's per-frame facing cache; matches the
 * original's 0x400-byte allocation. */
#define DIR_CACHE_CAPACITY 0x80

/* Live-pedestrian cap for one group. */
#define PED_GROUP_SLOTS 0x10

typedef struct {
    PedPathNode *key;
    int bucket; /* 0, 0x10000000, 0x20000000 or 0x30000000 */
} DirCacheEntry;

/* One per pedestrian-group slot; var_pedGroups_8c228230's entry type. */
typedef struct {
    int active_0x00;   /* whether list_0x08 has been allocated */
    /* refreshed each frame by pedestriansTask_8c0293f6; drives pedGroupTask_8c029078's
     * teardown decision */
    int wanted_0x04;
    Task *list_0x08;    /* the group's own Task subgroup; NULL-terminated, freed slots -1 */
} PedGroupEntry;

/* One spawn entry from a group's spec list (PedGroupDef.specs_0x08), walked by
 * pedGroupTask_8c029078. */
typedef struct {
    Uint16 nKindId_0x00; /* list terminator: 0xffff */
    Uint8 nKind_0x02;
    Uint8 nReverse_0x03;
    float flX_0x04; /* nKind_0x02 0/1: along-path spawn distance; nKind_0x02 2: world X */
    float flZ_0x08; /* nKind_0x02 0/1: unused; nKind_0x02 2: world Z */
} PedGroupSpawnSpec;

/* var_pedGroupDefs_8c22823c entry, looked up by id when pedestriansTask_8c0293f6
 * spins a group's Task up for the first time. */
typedef struct {
    int id_0x00;
    float radius_0x04;             /* seeds the group's PedGroupTask.radius_0x10 */
    PedGroupSpawnSpec *specs_0x08;
} PedGroupDef;

/* var_pedPaths_8c228238 entry, indexed in lockstep with var_pedGroups_8c228230
 * by pedGroupTask_8c029078. */
typedef struct {
    PedPathNode *pFirst_0x00;
    PedPathNode *pLast_0x04;
    float flLength_0x08;
} PedPathInfo;

/* Task private state for pedGroupTask_8c029078, set up by pedestriansTask_8c0293f6. */
typedef struct {
    TaskAction action;
    void *state;
    int pageListIndex_0x08;
    void *field_0x0c;
    float radius_0x10; /* overlays Task's int field_0x10; no reinterpret cast needed at the read site */
    int field_0x14;
    Task *subTasks_0x18;
    PedGroupSpawnSpec *spec_0x1c;
} PedGroupTask;

/* Task private state for pedestriansTask_8c0293f6. */
typedef struct {
    TaskAction action;
    void *state;
    int lastPreset_0x08;   /* currently-synced var_activePedPreset_8c22822c snapshot */
    int armed_0x0c;      /* pending-preset-change flag, held across the frame it reads 0 */
    int field_0x10;
    int field_0x14;
    void *queuedItem_0x18;
    int field_0x1c;
} PedestriansTask;

/* Trailing 8 bytes of a type-5 row's source struct (see
 * ObjectsStartAssetRequests_8c029ad4), copied verbatim into the dest slot;
 * compiles to a call to the SHC struct-copy helper __quick_evn_mvn. Read by
 * rowMaterialModelTask_8c02a27c as the row's world X/Z position for its distance-fade calc. */
typedef struct {
    float posX_0x00;
    float posZ_0x04;
} ObjectAssetType5Extra;


/* One row's slot in var_assetRequestSlots_8c228288: the handles
 * ObjectsStartAssetRequests_8c029ad4 asks the asset queues to fill in, read back
 * by ObjectsSpawnTasks_8c02a6ac once loaded. Which members a row uses depends on
 * its type. */
typedef struct {
    NJS_TEXLIST *pvm_0x00;
    NJS_CNK_OBJECT *nj_0x04;
    void *dat_0x08;                 /* NJS_OBJECT for type 0, NJS_MOTION for types 0/3, DatBlob for type 1 */
    ObjectAssetType5Extra pos_0x0c; /* type 5 only */
    /* types 2/3/4; matched against var_busState_8c1bb9d0.scenePresetIds_0x3bc's top byte, see
     * rowModelTask_8c02a08a */
    Sint8 sceneGate_0x14;
    /* types 2/3/4; routed into the spawned Task's field_0x0c (2/3) or field_0x08 (4) */
    Sint8 taskFlag_0x15;
    Uint8 fogEnable_0x16;            /* types 2/3/4 */
    Uint8 control3DEnable_0x17;      /* types 2/3/4 */
} AssetRequestSlot;

/* A type-1 row's loaded DAT blob: a header followed in the same allocation by
 * one visibility bitmask per animation step. evalflags bit convention: clear
 * hides a child (0x3e), set shows it (0x36). */
typedef struct {
    int nNodes_0x00;                   /* bounds the child walk; counts from bit 1, bit 0 unused */
    int framesPerStep_0x04;
    int stepCount_0x08;
    int step_0x0c;
    int tick_0x10;
    Uint32 *masks_0x14;                /* == masks_0x20 */
    NJS_TEXLIST *texlist_0x18;
    NJS_CNK_OBJECT *model_0x1c;
    Uint32 masks_0x20[1];              /* stepCount_0x08 entries */
} DatBlob;

/* Shared TaskSpawn_8c014ae8 state for every row-type task ObjectsSpawnTasks_8c02a6ac
 * spawns -- it spawns all of them with sizeof(RowTaskState) (0x7c): -- rowFlyByTask_8c029e94/flyByModelTask_8c029e68,
 * rowDatTask_8c029fcc, rowModelTask_8c02a08a, rowMotionModelTask_8c02a120,
 * rowSimpleModelTask_8c02a1f0, rowMaterialModelTask_8c02a27c and
 * fumiCrossingTask_8c02a4f8. Each row type only touches the subset of fields
 * it needs; bytes below 0x40 are never read or written anywhere in this unit.
 *
 * `dat_0x48` mirrors ObjectsSpawnTasks_8c02a6ac's own row-local `dat` variable: an
 * NJS_MOTION* for types 0/3, a DatBlob* for type 1, or a randomly-chosen train
 * NJS_MOTION* for type 6, hence declared as a bare void*. */
typedef struct RowTaskState {
    char field_0x00[0x40];
    NJS_TEXLIST *texlist_0x40;
    NJS_CNK_OBJECT *model_0x44;
    void *dat_0x48;
    ObjectAssetType5Extra pos_0x4c;    /* type-5 only */
    int phase_0x54;                    /* types 2/3/6 */
    float frame_0x58;
    float frameLimit_0x5c;
    float trainFrame_0x60;             /* type-6 only */
    float trainFrameLimit_0x64;            /* type-6 only: frame threshold, zeroed once the train has passed */
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
/* 12-byte entries {float *first, float *last, float length}, indexed in
 * lockstep with var_pedGroups_8c228230 by pedGroupTask_8c029078. */
STATIC void* var_pedPaths_8c228238;
/* Per-group spawn definition, 12-byte entries {int id, float radius, spec
 * list*}, looked up by id in pedestriansTask_8c0293f6. */
STATIC void* var_pedGroupDefs_8c22823c;
/* int*[] indexed by var_activePedPreset_8c22822c; each list is a -1 terminated array of
 * group ids, consumed by pedestriansTask_8c0293f6. */
STATIC void* var_pedGroupLists_8c228240;
/* Crosswalk table walked by pedestrianTask_8c028e00: entries are pairs of
 * path-node pointers, terminated by the end pointer var_blockedCrosswalksEnd_8c228244. */
STATIC int* var_blockedCrosswalksEnd_8c228244;
STATIC int var_blockedCrosswalks_8c228248[8];
STATIC float var_busAxisA_8c228268[2]; /* segment-intersection scratch, param1 for IntersectSegments_8c0206f0 */
STATIC float var_busAxisB_8c228270[2]; /* segment-intersection scratch, param2 for IntersectSegments_8c0206f0 */
/* nodes[0] = the route's blinker model (var_routeModels_8c1bc3ec[9]); [1..3]
 * are its child/sibling tree, filled by resolveObjectChildren_8c029868. */
STATIC NJS_OBJECT *var_routeBlinkerNodes_8c228278[4];
/* Per-slot destination pointers for a pending object-asset request, one 0x18-byte
 * entry per table row processed by ObjectsStartAssetRequests_8c029ad4 (up to 16
 * rows), also read/freed by ObjectsFreeAssetRequests_8c029cfe and ObjectsSpawnTasks_8c02a6ac. Raw bytes:
 * which fields are used depends on the row's type. */
STATIC Uint8 var_assetRequestSlots_8c228288[16 * 0x18];
/* Table currently in flight for ObjectsStartAssetRequests_8c029ad4: an array of
 * {type, dataPtr} pairs terminated by type == -1. -1 when nothing is queued. */
STATIC int *var_assetRequestTable_8c228408;
/* Fixed asset handles for the type-6 ("FUMI" railway crossing) row, shared by
 * every table that includes one -- there is only ever one railway crossing. */
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
/* nodes[0] = the FUMI lamp model (var_fumiLampModel_8c22841c); [1..16] are its
 * grandchild tree, filled by resolveObjectGrandchildren_8c02a322. */
STATIC NJS_OBJECT *var_fumiLampNodes_8c228434[17];

/* ====================
 * Initialized Globals
 * ====================
 */

/* Mechanically dumped from src/asm/decompiled/0289ac_objects.src sections C/D
 * via scripts/dump_src_data.py, in original file (== address) order; a few
 * symbols below are hand-typed where a semantic type earns its keep, verified
 * byte-identical against the archived asm via scripts/dcdiff.py. */

/* Sprite frames for the pedestrian billboards; indexed off the ped kind. */
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

/* Endpoints of the bus's stop line in bus-local space; njCalcPoint'd into
 * var_busAxisA/B_8c228268 each time the route state is rebuilt. */
STATIC NJS_POINT3 init_busAxisLocalA_8c04650c = { 0.0f, 0.0f, -4.1f };

STATIC NJS_POINT3 init_busAxisLocalB_8c046518 = { 0.0f, 0.0f, 9.5f };

/* Route-specific blinker spawn point tables, keyed by var_route_8c18ad1c. */
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

/* Called by ObjectsStartAssetRequests_8c029ad4 before its own definition. */
void ObjectsFreeAssetRequests_8c029cfe(void);

/* ====================
 * Functions
 * ====================
 */

/* Advances a pedestrian along its path (pPathNode_0x4c, flPathPos_0x54),
 * forward or backward per nReverse_0x40, moving into the next/previous segment
 * and wrapping at the path's ends. Returns whether the segment changed. */
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

/* Mirror-view counterpart of drawPedestrians_8c028b74, registered as its
 * layer-1 draw callback by pedestriansTask_8c0293f6. Takes the facing bucket
 * from the path node's flag bits instead of the camera angle, and flips the
 * facing test. */
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

/* Draws every active pedestrian's sprite for the frame. A type 2 object
 * (checked at 0x3c) just maps its own facing state (0x40, 0-3) to one of
 * four sprite frames. Any other type is bucketed by the angle between the
 * camera and the direction its path segment runs (flStepX_0x0c/flStepZ_0x10
 * of pPathNode_0x4c); every pedestrian on a segment shares that angle, so
 * the bucket is cached per frame keyed by the node pointer.
 * Installed as a DrawFn; the callback arg is unused. */
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
                        continue; /* cache full; skip drawing this object */
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

/* Relocation fixup for a freshly-loaded macHumM0 blob (which becomes
 * var_pedGroupLists_8c228240): a 0-terminated array of self-relative offsets,
 * each turned into an absolute pointer in place. */
void ObjectsRelocatePedGroupLists_8c028dd0(void *handle)
{
    int *entry = handle;
    while (*entry != 0) {
        *entry += (int)handle;
        entry++;
    }
}
/* Relocation fixup for a freshly-loaded PedGroupDef array (013ae8 hands it
 * the course's macHumG0 blob, which becomes var_pedGroupDefs_8c22823c): adds
 * the array's own base address to each entry's specs_0x08, converting it from
 * a self-relative offset to an absolute pointer. Terminated by an entry whose
 * id_0x00 is -1. */
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
        /* Held at the kerb; frame 1 on the signal object means "don't walk". */
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
/* Do-nothing TaskAction for a kind-2 (static sprite) ped-group entry:
 * unlike a walking pedestrian it needs no per-frame update once placed. */
STATIC void pedStaticObjectTask_8c02903e() {}

/* Per-frame driver for one pedestrian-group slot's spawn list, installed via
 * TaskSpawn_8c014ae8 by pedestriansTask_8c0293f6. Each call consumes one
 * PedGroupSpawnSpec entry from the group's spec list (task->spec_0x1c),
 * spawning either a walking pedestrian (pedestrianTask_8c028e00) or a static
 * sprite object (pedStaticObjectTask_8c02903e) into the group's own subtask
 * array (task->subTasks_0x18), then runs that subtask array for the frame.
 * If var_pedGroups_8c228230[task->pageListIndex_0x08] is no longer wanted this
 * frame, tears the whole group down instead. */
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
        TaskKillGroup_8c014ab4(task->subTasks_0x18);
        syFree(task->subTasks_0x18);
        TaskKill_8c014b66((Task *)task);
        return;
    }

    spec = task->spec_0x1c;
    if (spec->nKindId_0x00 != 0xffff) {
        path = &((PedPathInfo *)var_pedPaths_8c228238)[pageListIndex];

        if (spec->flX_0x04 == 0.0f || spec->flX_0x04 <= path->flLength_0x08) {
            /* Faithful to the original: an unrecognised nKind falls into the
             * sprite setup below with subTask/state still uninitialised. Only a
             * failed TaskSpawn skips it. */
            pushFailed = 0;
            if (spec->nKind_0x02 == 0 || spec->nKind_0x02 == 1) {
                if (TaskSpawn_8c014ae8(task->subTasks_0x18, pedestrianTask_8c028e00,
                        &subTask, (void **)&state, sizeof(PedestrianState))) {
                    radius = task->radius_0x10;
                    state->flBaseX_0x20 = ((float)rand() / 32768.0f) * radius * 2.0f - radius;
                    state->flBaseZ_0x24 = ((float)rand() / 32768.0f) * radius * 2.0f - radius;
                    /* Only the seed's low 16 bits: 0.04 to 0.06 units per frame. */
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
                if (TaskSpawn_8c014ae8(task->subTasks_0x18, pedStaticObjectTask_8c02903e,
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
                    TaskKill_8c014b66(subTask);
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

/* Per-frame driver for every pedestrian group: syncs var_pedGroups_8c228230's
 * wanted/active state with the current preset (task->armed_0x0c debounces a
 * single pending-change flag across frames where var_busState_8c1bb9d0.scenePresetIds_0x3bc
 * reads 0), rebuilds this frame's crosswalk intersection scratch, runs every
 * group's Task, then registers the draw callback(s) -- drawPedestriansMirror_8c028a38
 * only outside demo mode. */
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
                /* One slot over PED_GROUP_SLOTS: TaskInitGroup_8c014a9c terminates
                 * the array with a NULL action past the slots it frees. */
                subtasks = syMalloc((PED_GROUP_SLOTS + 1) * sizeof(Task));
                if (subtasks == NULL) {
                    break;
                }
                group->list_0x08 = subtasks;
                TaskInitGroup_8c014a9c(subtasks, PED_GROUP_SLOTS);

                if (!TaskSpawn_8c014ae8(var_tasks_8c1ba808, pedGroupTask_8c029078,
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

/* One-shot pedestrian-group setup for the current run, called from
 * GameEnterDrive_8c01306e. Copies the route's pedestrian tables (loaded by
 * requestCourseAssets_8c014088 into var_currentCourse_8c1bb868's Hum fields) into
 * var_pedPaths_8c228238/var_pedGroupDefs_8c22823c/var_pedGroupLists_8c228240
 * and installs pedestriansTask_8c0293f6 to drive them every frame. No-op if
 * the route defines no pedestrian groups. */
void ObjectsSpawnPedestrianGroups_8c0296d6(void)
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

    TaskSpawn_8c014ae8(var_tasks_8c1ba5e8, pedestriansTask_8c0293f6, (Task **)&task, &state, 0);
    task->lastPreset_0x08 = -1;
    task->armed_0x0c = 1;
}

/* Teardown counterpart to ObjectsSpawnPedestrianGroups_8c0296d6: frees every
 * still-active group's subtask block (see pedestriansTask_8c0293f6, which
 * stashes it in PedGroupEntry.list_0x08) and the group array itself, then
 * resets to the not-yet-loaded
 * sentinel state. No-op if groups were never loaded this run. */
void ObjectsKillPedestrianGroups_8c0297da(void)
{
    PedGroupEntry *groups;
    int i;

    if (var_pedGroupCount_8c228234 < 0) {
        return;
    }

    groups = (PedGroupEntry *)var_pedGroups_8c228230;
    for (i = 0; i < var_pedGroupCount_8c228234; i++) {
        if (groups[i].active_0x00 != 0) {
            TaskKillGroup_8c014ab4(groups[i].list_0x08);
            syFree(groups[i].list_0x08);
        }
    }

    syFree(var_pedGroups_8c228230);
    var_pedGroups_8c228230 = (void *)-1;
    var_pedGroupCount_8c228234 = -1;
}

/* Task private state for routeBlinkerTask_8c029904, set up by
 * ObjectsSpawnBlinkers_8c029920. */
typedef struct {
    TaskAction action;
    void *state;            /* NJS_MATRIX* on the callback */
    int count_0x08;          /* number of NJS_MATRIX entries in state */
    int blinkCounter_0x0c;   /* per-frame counter, round-tripped through Task's void* field_0x0c */
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

/* Draws all `count` blinker matrices built by ObjectsSpawnBlinkers_8c029920,
 * cycling var_routeBlinkerNodes_8c228278's child nodes' eval flags through a 3-phase blink. */
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

/* Per-frame TaskAction for the blinker group: advances the blink counter and
 * queues this frame's draw through the fade command pipeline. */
STATIC void routeBlinkerTask_8c029904(RouteBlinkerTask *task, NJS_MATRIX *state)
{
    task->blinkCounter_0x0c++;
    RenderQueueDraw2_8c022420(RENDER_LAYER_MAIN, drawBlinkers_8c029878, (int)task, (int)state);
}

/* One-shot setup for the current run's route blinkers, called from
 * GameEnterDrive_8c01306e. Resolves var_routeBlinkerNodes_8c228278 from the route model
 * table (mirroring ObjectsSpawnPedestrianGroups_8c0296d6's sibling setup calls)
 * and installs routeBlinkerTask_8c029904 to draw them every frame. No-op if
 * the route's table is empty.
 *
 * Each point's height uses its own ground-query fallback, which differs
 * from snapPointToGround_8c02840c's at the tail. */
void ObjectsSpawnBlinkers_8c029920(void)
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

    TaskSpawn_8c014ae8(var_tasks_8c1ba5e8, &routeBlinkerTask_8c029904, (Task **)&task,
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
/* Frees every asset handle written into var_assetRequestSlots_8c228288 by a prior
 * ObjectsStartAssetRequests_8c029ad4(table), mirroring its per-row type
 * switch. No-op if no table is currently in flight. */
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
/* DrawFn draw callback for the fly-by model spawned by flyByModelTask_8c029e68;
 * state is the spawned task's state array (texlist at 0x40, model at 0x44,
 * motion at 0x48, current frame at 0x58, per rowFlyByTask_8c029e94/flyByModelTask_8c029e68). */
STATIC void drawFlyByModel_8c029e46(int state)
{
    RowTaskState *st = (RowTaskState *)state;

    njSetTexture(st->texlist_0x40);
    njCnkSimpleDrawMotion(st->model_0x44, (NJS_MOTION *)st->dat_0x48, st->frame_0x58);
}
/* TaskAction spawned by rowFlyByTask_8c029e94 once its countdown lapses; animates a
 * randomly-chosen route model's fly-by until its frame count runs out. */
STATIC void flyByModelTask_8c029e68(Task *task, RowTaskState *state)
{
    state->frame_0x58 = state->frame_0x58 + 1.0f;
    if (state->frameLimit_0x5c <= state->frame_0x58) {
        TaskKill_8c014b66(task);
        return;
    }
    RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, drawFlyByModel_8c029e46, (int)state);
}
/* TaskAction for a type-0 row, installed by ObjectsSpawnTasks_8c02a6ac. Counts
 * down task->field_0x08; once it lapses, spawns a fly-by task (flyByModelTask_8c029e68)
 * seeded from a random route model slot and re-arms the countdown. */
STATIC void rowFlyByTask_8c029e94(Task *task, RowTaskState *state)
{
    Task *newTask;
    RowTaskState *newState;
    int idx;

    if (--task->field_0x08 < 0) {
        if (TaskSpawn_8c014ae8(var_tasks_8c1bb448, &flyByModelTask_8c029e68, &newTask,
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
/* DrawFn draw callback for a type-1 row task; state is the spawned
 * task's state (see RowTaskState), with the DatBlob at RowTaskState::dat_0x48. */
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
/* Returns 0 if the model's child chain ran out before nNodes_0x00 children,
 * leaving the step unadvanced. */
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
/* TaskAction for a type-1 row, installed by ObjectsSpawnTasks_8c02a6ac. */
STATIC void rowDatTask_8c029fcc(Task *task, RowTaskState *state)
{
    advanceDatBlob_8c029f54((DatBlob *)state->dat_0x48);
    RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, drawDatModel_8c029f2a, (int)state);
    RenderQueueDraw_8c0223ea(RENDER_LAYER_MIRROR, drawDatModel_8c029f2a, (int)state);
}
/* DrawFn draw callback for a type-2 row task; state is the spawned
 * task's state array. Draws the {texlist, model} pair at offset 0x40/0x44,
 * with fog and njControl3D left toggled off for the draw when the row's
 * flags at 0x78/0x79 are clear. */
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
/* TaskAction for a type-2 row, installed by ObjectsSpawnTasks_8c02a6ac;
 * task->field_0x08 is the target var_busState_8c1bb9d0.scenePresetIds_0x3bc marker, set from
 * row[0x14] there. */
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
/* DrawFn draw callback for a type-3 row task; state is the spawned
 * task's state array, laid out like drawFlyByModel_8c029e46's (texlist 0x40,
 * model 0x44, motion 0x48, current frame 0x58) with drawRowModel_8c02a048's
 * fog/njControl3D toggles (flags at 0x78/0x79). */
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
/* TaskAction for a type-3 row, installed by ObjectsSpawnTasks_8c02a6ac.
 * task->field_0x08 is the target marker, per type-2's rowModelTask_8c02a08a;
 * state->frameLimit_0x5c is set like type-0's flyByModelTask_8c029e68's. */
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
            TaskKill_8c014b66(task);
            return;
        }
        state->frame_0x58 = 0.0f;
    }
}
/* Draws a {texlist, model} pair embedded at offset 0x40 in state, fog
 * left enabled only when fogEnable is non-zero. */
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
/* TaskAction for a type-4 row, installed by ObjectsSpawnTasks_8c02a6ac. */
STATIC void rowSimpleModelTask_8c02a1f0(Task *task, RowTaskState *state)
{
    RenderQueueDraw2_8c022420(RENDER_LAYER_MAIN, drawRowSimpleModel_8c02a1b2, (int)state, task->field_0x08);
}
/* DrawFn draw callback for a type-5 row task; state is the spawned
 * task's state array. Unlike drawRowModel_8c02a048, fades the whole draw via
 * a constant material color (alpha at 0x68, set by rowMaterialModelTask_8c02a27c) instead of
 * toggling fog/lighting per row flags. */
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
/* TaskAction for a type-5 row, installed by ObjectsSpawnTasks_8c02a6ac. Fades
 * the row's material to invisible beyond a distance of 20 from the bus
 * (averaging var_busState_8c1bb9d0's two position pairs against the row's
 * own position, per ObjectAssetType5Extra), ramping from opaque at 10 down
 * to hidden at 20 within that band, gated by var_cameraMode_8c227d9c (some quality/LOD
 * level) being at least 2; then pushes two draw calls (opaque and
 * translucent draw layers) of the row's model via
 * drawRowMaterialModel_8c02a206. task is unused. */
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
/* Resolves 16 grandchildren of nodes[0] into nodes[1..16]: nodes[0]'s two
 * children (child, child->sibling) each contribute 8 nodes -- their own
 * child followed by 7 further siblings. Mirrors resolveObjectChildren_8c029868
 * one level deeper. */
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
/* Sets nodes[1..16]'s eval flags in a 3-phase pattern selected by
 * `selector`, one level deeper than drawBlinkers_8c029878's 3-phase cycle
 * over resolveObjectGrandchildren_8c02a322's output. Any other selector
 * value is a no-op. */
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
/* DrawFn draw callback for a type-6 (FUMI railway crossing) row task,
 * called by fumiCrossingTask_8c02a4f8. Draws the gate, the lamp housing, and --
 * while a passing train is still due (state->trainFrameLimit_0x64 > 0) -- the train
 * model stashed at state->model_0x44/state->dat_0x48 (same slots as
 * drawRowMotionModel_8c02a0d6's). */
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
/* TaskAction for a type-6 (FUMI railway crossing) row, installed by
 * ObjectsSpawnTasks_8c02a6ac. Cycles the crossing through closing, waiting out
 * a passing train, then opening; idles at phase 0 until
 * var_busState_8c1bb9d0.scenePresetIds_0x3bc's top byte (the set-piece trigger) goes nonzero.
 * From phase 1 onward, pushes a draw call of drawFumiCrossing_8c02a47c every
 * frame. task is unused. */
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
/* Installed as a DrawFn on both draw layers (arg0 = layer). Sets up
 * njCnk's simple light for the row draw calls that follow: direction from
 * var_simpleLightDir_8c2264d8 (layer 0) or var_mirrorSimpleLightDir_8c2264e4
 * (layer 1, the mirror side), and a constant intensity/ambient/color already
 * copied from the scene params by 0222dc_fadecmd. */
STATIC void setSimpleLightCallback_8c02a5d0(int arg0)
{
    float *dir = (arg0 == 0) ? var_simpleLightDir_8c2264d8 : var_mirrorSimpleLightDir_8c2264e4;

    njCnkSetSimpleLight(dir[0], dir[1], dir[2]);
    njCnkSetSimpleLightIntensity(var_simpleLightIntensity_8c2264f0[0], var_simpleLightIntensity_8c2264f0[1]);
    njCnkSetSimpleLightColor(var_simpleLightColor_8c2264f8[0], var_simpleLightColor_8c2264f8[1], var_simpleLightColor_8c2264f8[2]);
}
/* TaskAction spawned once after the table is fully processed, installed by
 * ObjectsSpawnTasks_8c02a6ac. Once the run is under way, pushes
 * setSimpleLightCallback_8c02a5d0 as a draw callback on both
 * draw layers, then runs the row tasks just spawned into var_tasks_8c1bb448 to
 * completion. */
STATIC void execRowTaskGroupTask_8c02a60e(void)
{
    if (var_runState_8c2285c4.runPhase_0x00 != 0) {
        RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, setSimpleLightCallback_8c02a5d0, 0);
        RenderQueueDraw_8c0223ea(RENDER_LAYER_MIRROR, setSimpleLightCallback_8c02a5d0, 1);
        TaskRunGroup_8c014b42(var_tasks_8c1bb448);
    }
}
/* For each {type, dataPtr} row in the in-flight table (var_assetRequestTable_8c228408, filled by
 * ObjectsStartAssetRequests_8c029ad4), spawns a per-type task seeded from the
 * row's loaded asset handles in var_assetRequestSlots_8c228288, then spawns one closing task once
 * the whole table has been processed. */
void ObjectsSpawnTasks_8c02a6ac(void)
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
            TaskSpawn_8c014ae8(var_tasks_8c1bb448, &rowFlyByTask_8c029e94, &task, (void **)&state, sizeof(RowTaskState));
            state->dat_0x48 = dat;
            state->frameLimit_0x5c = (float)*(Uint32 *)((Uint8 *)dat + 4) - 1.0f;
            task->field_0x08 = 0;
        } else if (type == 1) {
            TaskSpawn_8c014ae8(var_tasks_8c1bb448, &rowDatTask_8c029fcc, &task, (void **)&state, sizeof(RowTaskState));
            initDatBlob_8c029f42((DatBlob *)dat, pvm, nj);
            state->dat_0x48 = dat;
        } else if (type == 2) {
            TaskSpawn_8c014ae8(var_tasks_8c1bb448, &rowModelTask_8c02a08a, &task, (void **)&state, sizeof(RowTaskState));
            state->phase_0x54 = 0;
            state->texlist_0x40 = pvm;
            state->model_0x44 = nj;
            task->field_0x08 = (int)row->sceneGate_0x14 << 0x18;
            task->field_0x0c = (void *)(int)row->taskFlag_0x15;
            state->fogEnable_0x78 = row->fogEnable_0x16;
            state->control3DEnable_0x79 = row->control3DEnable_0x17;
        } else if (type == 3) {
            TaskSpawn_8c014ae8(var_tasks_8c1bb448, &rowMotionModelTask_8c02a120, &task, (void **)&state, sizeof(RowTaskState));
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
            TaskSpawn_8c014ae8(var_tasks_8c1bb448, &rowSimpleModelTask_8c02a1f0, &task, (void **)&state, sizeof(RowTaskState));
            state->texlist_0x40 = pvm;
            state->model_0x44 = nj;
            task->field_0x08 = (int)row->taskFlag_0x15;
        } else if (type == 5) {
            TaskSpawn_8c014ae8(var_tasks_8c1bb448, &rowMaterialModelTask_8c02a27c, &task, (void **)&state, sizeof(RowTaskState));
            state->texlist_0x40 = pvm;
            state->model_0x44 = nj;
            state->pos_0x4c = row->pos_0x0c;
            state->material_0x68.b = 0.0f;
            state->material_0x68.g = 0.0f;
            state->material_0x68.r = 0.0f;
        } else if (type == 6) {
            void *chosen;

            /* Bug-for-bug: passes state itself (its stale value from the
             * previous row), not &state, as the create_state out-param --
             * matches the archived asm exactly. */
            TaskSpawn_8c014ae8(var_tasks_8c1bb448, &fumiCrossingTask_8c02a4f8, &task, (void **)state, sizeof(RowTaskState));
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

    TaskSpawn_8c014ae8(var_tasks_8c1ba5e8, &execRowTaskGroupTask_8c02a60e, &task, (void **)&state, 0);
}
