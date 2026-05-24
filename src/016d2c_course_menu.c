#include <shinobi.h>
#include <sg_sd.h>
#include <njdef.h>
#include <sg_xpt.h>
#include "015ab8_title.h"
#include "014a9c_tasks.h"
#include "011120_asset_queues.h"
#include "019e98_main_menu.h"
#include "016d2c_course_menu.h"
#include "serial_debug.h"

// TODO:
// - Review comments

/* ====================
 * Compiler Definitions
 * ====================
 */

#ifdef SERIAL_DEBUG
char *DEBUG_courseMenuStateNames[] = {
    "INIT",
    "FADE_IN",
    "DIALOG",
    "IDLE",
    "ANIMATING",
    "COURSE_SELECTED",
    "FADE_OUT",
    "FADE_OUT_TO_MAIN_MENU"
};

char *DEBUG_courseConfirmStateNames[] = {
    "INIT",
    "FADE_IN",
    "PROMPT",
    "FADE_OUT",
    "ROUTE_INFO_FADE_IN",
    "ROUTE_INFO_DISPLAY",
    "START_LOADING",
    "FADE_OUT_TO_COURSE_MENU"
};
#endif

#ifdef SERIAL_DEBUG
#define CHANGE_STATE(x) menuState_8c1bc7a8.state_0x18 = x; LOG_DEBUG(("[COURSE_MENU] State changed: %s\n", DEBUG_courseMenuStateNames[x]))
#define CHANGE_CONFIRM_STATE(x) menuState_8c1bc7a8.state_0x18 = x; LOG_DEBUG(("[COURSE_MENU] Confirm state changed: %s\n", DEBUG_courseConfirmStateNames[x]))
#else
#define CHANGE_STATE(x) menuState_8c1bc7a8.state_0x18 = x
#define CHANGE_CONFIRM_STATE(x) menuState_8c1bc7a8.state_0x18 = x
#endif

/* =================
 * Type Declarations
 * =================
 */

typedef struct {
    int enabled_0x00;
    int unlocked_0x04;
    float x_0x08;
    float y_0x0c;
    int spriteNo_0x10;
    void (*onSelect_0x14)(Task *task);
    int courseId_0x18;
} CourseMenuButton;

typedef struct {
    char *text_0x00;
    int instructorSpriteNo_0x04;
} MenuDialog;

typedef struct {
    Uint8 unlocked_0x00;
    Uint8 new_0x01;
    Uint8 field_0x02;
    Uint8 storySpriteNo_0x03;
    Uint8 freeRunSpriteNo_0x04;
    Uint8 field_0x05[3]; // Padding?
} CourseProgress;

typedef struct {
    int days_0x00;
    char field_0x04[0x28];
    int letters_0x2c[6];
    CourseProgress courses_0x44[9];
    int field_0x8c;
    int field_0x90;
} PlayerProgress;

typedef struct {
    int state_0x00;
    MenuDialog *dialog_0x04;
    int field_0x08;
    int field_0x0c;
    int field_0x10;
    int field_0x14;
    int *field_0x18;
} DialogSequenceTaskState;

typedef struct {
    TaskAction action;
    void *state;
    int field_0x08;
    void* field_0x0c;
    int field_0x10;
    int field_0x14;
    int *field_0x18;
    int field_0x1c;
} DialogSequenceTask;

enum {
    // --- Story / Training ---
    SEQ_STORY_INTRO           = 0,
    // SEQ_SUCCESS_PERFECT       = 1,
    // SEQ_SUCCESS_HIGH          = 2,
    // SEQ_SUCCESS_NORMAL        = 3,
    // SEQ_FAILURE_FINAL         = 4,
    // SEQ_FREE_RUN_INTRO        = 5,
    SEQ_STORY_CHOOSE_COURSE   = 6,
    SEQ_GOOD_PRACTICE         = 7,
    SEQ_SUCCESS               = 8,

    // --- Awards / Unlocks ---
    SEQ_AWARD_BADGE_GOLD      = 9,
    SEQ_AWARD_BADGE_SILVER    = 10,
    SEQ_AWARD_BADGE_BRONZE    = 11,
    SEQ_FAILURE_RETRY         = 12,
    SEQ_COURSE_UNLOCKED       = 13,
    SEQ_PASSENGER_LETTER      = 14,
    SEQ_COURSE_LOCKED         = 15,
    // SEQ_FORCE_PRACTICE        = 16,
    // SEQ_FINAL_DAY             = 17,

    // --- Lesson Mode ---
    // SEQ_LESSON_INTRO          = 18,
    // SEQ_LESSON_COMPLETE       = 19,
    // SEQ_LESSON_NEXT           = 20,
    // SEQ_LESSON_RETRY          = 21,
    // SEQ_LESSON_TIPS           = 22,
    // SEQ_LESSON_WARNING        = 23,
    // SEQ_LESSON_CHOOSE         = 24,
    // SEQ_SCORE_RECORD          = 25,
    // SEQ_LESSON_FINAL_DAY      = 26,
    // SEQ_LESSON_PERFECT        = 27,
    // SEQ_LESSON_GOOD           = 28,
    // SEQ_LESSON_PASS           = 29,
    // SEQ_LESSON_FAIL_MINOR     = 30,
    // SEQ_LESSON_FAIL_MAJOR     = 31,

    // --- Driving Mistakes / Penalties ---
    // SEQ_COLLISION_CAR_MINOR   = 32,
    // SEQ_COLLISION_CAR_MEDIUM  = 33,
    // SEQ_COLLISION_CAR_SEVERE  = 34,
    // SEQ_COLLISION_CAR_FATAL   = 35,
    // SEQ_COLLISION_WALL_MINOR  = 36,
    // SEQ_COLLISION_WALL_MEDIUM = 37,
    // SEQ_COLLISION_WALL_SEVERE = 38,
    // SEQ_NEAR_MISS_PEDESTRIAN  = 39,
    // SEQ_OFF_COURSE_MINOR      = 40,
    // SEQ_OFF_COURSE_MEDIUM     = 41,
    // SEQ_OFF_COURSE_MAJOR      = 42,
    // SEQ_SPEEDING_MINOR        = 43,
    // SEQ_SPEEDING_MAJOR        = 44,
    // SEQ_WRONG_LANE            = 45,
    // SEQ_LANE_STRADDLE         = 46,
    // SEQ_NO_SIGNAL             = 47,
    // SEQ_NO_SIGNAL_TURN        = 48,
    // SEQ_UKN_49                 = 49,
    // SEQ_SIGNAL_VIOLATION      = 50,
    // SEQ_BAD_STOP_LINE         = 51,
    // SEQ_ILLEGAL_LANE_CHANGE   = 52,
    // SEQ_BLOCK_INTERSECTION    = 53,
    // SEQ_WRONG_WAY             = 54,
    // SEQ_RAPID_ACCEL           = 55,
    // SEQ_HARD_BRAKE            = 56,
    // SEQ_SWERVING              = 57,
    // SEQ_MISSED_STOP           = 58,
    // SEQ_BAD_STOP_POSITION_1   = 59,
    // SEQ_BAD_STOP_POSITION_2   = 60,
    // SEQ_TIME_MANAGEMENT       = 61,
    // SEQ_ANNOUNCEMENT          = 62,
    // SEQ_DOOR_OPERATION        = 63,

    // --- Free Run Mode ---
    SEQ_FREE_RUN_INTRO_2       = 64,
    SEQ_FREE_RUN_CHOOSE_COURSE = 65,
};

enum {
    COURSE_MENU_STATE_INIT = 0,
    COURSE_MENU_STATE_FADE_IN = 1,
    COURSE_MENU_STATE_DIALOG = 2,
    COURSE_MENU_STATE_IDLE = 3,
    COURSE_MENU_STATE_ANIMATING = 4,
    COURSE_MENU_STATE_COURSE_SELECTED = 5,
    COURSE_MENU_STATE_FADE_OUT = 6,
    COURSE_MENU_STATE_FADE_OUT_TO_MAIN_MENU = 7
};

enum {
    COURSE_CONFIRM_STATE_INIT = 0,
    COURSE_CONFIRM_STATE_FADE_IN = 1,
    COURSE_CONFIRM_STATE_PROMPT = 2,
    COURSE_CONFIRM_STATE_FADE_OUT = 3,
    COURSE_CONFIRM_STATE_ROUTE_INFO_FADE_IN = 4,
    COURSE_CONFIRM_STATE_ROUTE_INFO_DISPLAY = 5,
    COURSE_CONFIRM_STATE_START_LOADING = 6,
    COURSE_CONFIRM_STATE_FADE_OUT_TO_COURSE_MENU = 7
};

/* =======================
 * Non-initialized Globals
 * =======================
 */

extern void snd_8c010cd6(int p1, int p2);
extern void setUknPvmBool_8c014330();
extern int CourseMenuRequestSysResgrp_8c018568(ResourceGroup* dds, ResourceGroupInfo* rg);
extern void CourseMenuConfirmInit_8c0184cc(Task *task);
extern void FUN_8c01f114(Task *task);
extern void FUN_8c01ba64(Task *task);
extern void FUN_8c01d1c4(Task *task);
extern void FUN_8c01d6e2(Task *task);
extern void* const_8c03628c;
extern SDMIDI var_midiHandles_8c0fcd28[7];
extern PlayerProgress var_progress_8c1ba1cc;
extern int var_exp_8c1ba25c;
extern int var_dialogSequenceIsActive_8c225fb4;
extern int var_menuTextboxCharLimit_8c225fb8;
extern PDS_PERIPHERAL var_peripherals_8c1ba35c[2];
extern ResourceGroupInfo init_mainMenuResourceGroup_8c044264;
extern int var_8c1bb8e0; // course was unlocked
extern int var_8c1bb8e4;
extern int var_8c1bb8e8;
extern int var_8c1bb8ec;
extern int var_8c1bb8f0;
extern int var_8c1bb8f4;
extern int var_8c1ba2b8[5]; // Maybe progress backup
extern int var_8c1ba2cc[5]; // Maybe progress backup
extern void pushLoadingTask_8c013310(int p1);
extern MenuDialog *init_dialogSequences_8c044c08[];
extern int var_game_mode_8c1bb8fc;
extern int var_dialogQueue_8c225fbc[4]; // TODO: Confirm length
extern Sint8 var_coursesToUnlock_8c225fd4[];
extern int var_demo_8c1bb8d0;
extern void resetUknPvmBool_8c014322();
extern NJS_TEXMEMLIST var_tex_8c157af8[];
extern int var_8c1bb8b8; // Maybe courseMenuHasResult or courseMenuHasDialog
extern int var_8c1bb8bc;
extern int var_8c1bb8dc;
extern int var_award_8c1bb8f8;
extern Bool isFading_8c226568;
extern int init_8c03bd80;
extern void *var_currentSysResGroupInfo_8c225fb0;
extern int var_shouldShowFreeRunIntro_8c1bb8c0;
extern void pushInputTask_8c0128cc(void);
extern void task_8c012f44(Task *task, void *state);
extern FUN_8c02ae3e(int p1, int p2, float fp1, int p3, int p4, int p5, int p6, int p7);
extern int promptHandleBinary_16caa(int *promptState);

/* ===================
 * Initialized Globals
 * ===================
 */

 /*  0  1    2  3  4
  *  5  6    7  8  9
  * 10 11   12 13 14  */
STATIC CourseMenuButton init_courseMenuButtons_8c04442c[15] = {
    {   /* [0] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 45.0f, 109.0f,
        /* spriteNo */ 0,
        /* onSelect */ FUN_8c01f114,
        /* courseId */ 0,
    },
    {   /* [1] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 110.0f, 109.0f,
        /* spriteNo */ 0,
        /* onSelect */ FUN_8c01ba64,
        /* courseId */ 0,
    },
    {   /* [2] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 190.0f, 109.0f,
        /* spriteNo */ 11,
        /* onSelect */ CourseMenuConfirmInit_8c0184cc,
        /* courseId */ 0,
    },
    {   /* [3] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 284.0f, 109.0f,
        /* spriteNo */ 12,
        /* onSelect */ CourseMenuConfirmInit_8c0184cc,
        /* courseId */ 3,
    },
    {   /* [4] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 377.0f, 109.0f,
        /* spriteNo */ 13,
        /* onSelect */ CourseMenuConfirmInit_8c0184cc,
        /* courseId */ 6,
    },
    {   /* [5] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 45.0f, 182.0f,
        /* spriteNo */ 0,
        /* onSelect */ FUN_8c01d1c4,
        /* courseId */ 0,
    },
    {   /* [6] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 109.0f, 182.0f,
        /* spriteNo */ 0,
        /* onSelect */ FUN_8c01d6e2,
        /* courseId */ 0,
    },
    {   /* [7] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 190.0f, 182.0f,
        /* spriteNo */ 14,
        /* onSelect */ CourseMenuConfirmInit_8c0184cc,
        /* courseId */ 9,
    },
    {   /* [8] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 285.0f, 182.0f,
        /* spriteNo */ 15,
        /* onSelect */ CourseMenuConfirmInit_8c0184cc,
        /* courseId */ 12,
    },
    {   /* [9] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 378.0f, 182.0f,
        /* spriteNo */ 16,
        /* onSelect */ CourseMenuConfirmInit_8c0184cc,
        /* courseId */ 15,
    },
    {   /* [10] */
        /* enabled  */ 0,
        /* unlocked */ 0,
        /* x, y     */ 44.0f, 256.0f,
        /* spriteNo */ 0,
        /* onSelect */ NULL,
        /* courseId */ 0,
    },
    {   /* [11] */
        /* enabled  */ 0,
        /* unlocked */ 0,
        /* x, y     */ 108.0f, 256.0f,
        /* spriteNo */ 0,
        /* onSelect */ NULL,
        /* courseId */ 0,
    },
    {   /* [12] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 191.0f, 256.0f,
        /* spriteNo */ 17,
        /* onSelect */ CourseMenuConfirmInit_8c0184cc,
        /* courseId */ 18,
    },
    {   /* [13] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 285.0f, 256.0f,
        /* spriteNo */ 18,
        /* onSelect */ CourseMenuConfirmInit_8c0184cc,
        /* courseId */ 21,
    },
    {   /* [14] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 378.0f, 256.0f,
        /* spriteNo */ 19,
        /* onSelect */ CourseMenuConfirmInit_8c0184cc,
        /* courseId */ 24,
    },
};

STATIC MenuDialog init_seqStoryIntro_8c0445d0[] = {
    { "やぁ、よく来たね<E>私がここの所長だ、以後よろしく", 1 },
    { "今日から一ヶ月間、キミには<E>ここで実地研修を受けてもらう", 0 },
    { "この研修に合格すると<E>キミは晴れて都営バスの運転手だ", 0 },
    { "最後の研修だが<E>気を抜かずに頑張ってくれよ", 0 },
    { "まずは比較的簡単な〈梅７６〉と<E>〈虹０１〉の２コースで研修しよう", 0 },
    { "視界も広く、交通量も多くない時間帯を<E>選んでいるので、研修にはちょうどいいだろう", 0 },
    { "慣れてきたら、少しづつ難しいコースも<E>走れるようになるからな", 0 },
    { "いきなりコースを走るのが不安なら<E>練習することもできるぞ", 0 },
    { "練習したい場合は画面左上の<E>ＰＲＡＣＴＩＣＥを選ぶといい", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqSuccessPerfect_8c044620[] = {
    { "キミの成績には、正直、びっくりしたよ<E>実に素晴らしい！", 0 },
    { "どのコースをとっても<E>まさにパーフェクト！", 0 },
    { "おめでとう<E>実地研修は満点で合格だ！", 2 },
    { "キミならどこに配属されても大丈夫<E>研修を担当した私も鼻が高いよ", 2 },
    { "都営バスの未来を担う一員として<E>キミには期待しているぞ", 3 },
    { "", 0 },
};

STATIC MenuDialog init_seqSuccessHigh_8c044650[] = {
    { "昨日で無事、実地研修が終了した<E>本当によく頑張ったな", 0 },
    { "キミの成績を見せてもらったが<E>申し分のない成績だったよ", 0 },
    { "全コースとも高得点をあげている<E>これは並大抵の事ではない", 0 },
    { "おめでとう<E>実地研修はもちろん合格だ！", 2 },
    { "今日からキミは都営バスの立派な運転手だ<E>これからもよろしく頼むぞ！", 3 },
    { "", 0 },
};

STATIC MenuDialog init_seqSuccessNormal_8c044680[] = {
    { "昨日で無事、実地研修が終了した<E>本当によく頑張ったな", 0 },
    { "キミの成績を見せてもらったが<E>まぁ、十分に合格点をあげられるだろう", 0 },
    { "各コース、致命的なミスはなく<E>成績も悪くない", 0 },
    { "これからも注意を怠ることなく<E>この調子でますます努力・精進してくれ", 0 },
    { "おめでとう<E>実地研修は合格だ！", 2 },
    { "明日からも気を抜くことなく、<E>研修の延長線だと思って、業務に取り組んでくれ", 3 },
    { "", 0 },
};

STATIC MenuDialog init_seqFailureFinal_8c0446b8[] = {
    { "１ヶ月間本当によく頑張ったな", 0 },
    { "…と言いたい所なんだが<E>合格点には少し及ばなかったようだ", 1 },
    { "運転手は、乗客の命を預かっているんだ<E>軽率な行動やミスは許されない", 1 },
    { "キミはもう少し研修を続けた方が<E>良さそうだな、頑張ってくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqFreeRunIntro_8c0446e0[] = {
    { "ここでは、好きなコースを<E>何度でも走る事ができるぞ", 0 },
    { "但しストーリーモードで走ったコース限る", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqStoryChooseCourse_8c0446f8[] = {
    { "さぁ、今日の研修を選んでくれ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqGoodPractice_8c044708[] = {
    { "昨日は充実した練習が出来たようだね", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqSuccess_8c044718[] = {
    { "昨日はなんとかうまく走れたようだね<E>その調子で頑張ってくれよ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqAwardBadgeGold_8c044728[] = {
    { "昨日は素晴らしい成績だったな", 0 },
    { "キミには優秀ドライバーの証し<E>ゴールドバッジを与えよう", 0 },
    { "この調子で頑張ってくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqAwardBadgeSilver_8c044748[] = {
    { "キミには優秀ドライバーの証し<E>シルバーバッジを与えよう", 1 },
    { "もっと高得点が出れば、さらに上の<E>ゴールドバッジもあるからな", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqAwardBadgeBronze_8c044760[] = {
    { "キミには優秀ドライバーの証し<E>ブロンズバッジを与えよう", 1 },
    { "もっと高得点が出れば、さらに上の<E>シルバーバッジやゴールドバッジもあるぞ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqFailureRetry_8c044778[] = {
    { "昨日は残念な結果になってしまったな<E>まぁ、誰にだって失敗はある", 0 },
    { "特に慣れないうちは仕方ないさ<E>気分を改めて今日も頑張ってくれよ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqCourseUnlocked_8c044790[] = {
    { "それから…経験も積んだようなので<E>そろそろ違うコースで研修してみよう", 0 },
    { "今までより、少し難しくなっていると思うが<E>キミなら大丈夫だ", 0 },
    { "勇気を持ってチャレンジしてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqPassengerLetter_8c0447b0[] = {
    { "そうそう、今日はお客さんから手紙が届いたよ", 0 },
    { "掲示板に張っておいたから、<E>見るときはＡＬＢＵＭを選んでくれ", 0 },
    { "お客さんってありがたいもんだね<E>運転手やってて本当に良かったって実感するよ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqCourseLocked_8c0447d0[] = {
    { "残念だがそのコースはまだ走れないよ<E>もっと経験を積んでからだな", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqForcePractice_8c0447e0[] = {
    { "はやる気持ちも分からんでもないが、<E>今日は練習してもらうよ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqFinalDay_8c0447f0[] = {
    { "長かった研修も今日が最後だな", 0 },
    { "さぁ、最後の研修を選んでくれ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonIntro_8c044808[] = {
    { "今日は初日なので、全てのＬＥＳＳＯＮを<E>順番にやってもらうよ", 0 },
    { "合格ラインは７０点。決して難しい事では<E>ないので、落ち着いて慎重に進めていこう", 0 },
    { "", 0 },
};

// Unused?
STATIC MenuDialog init_8c044810[] = {
    { "合格ラインは７０点。決して難しい事では<E>ないので、落ち着いて慎重に進めていこう", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonComplete_8c044820[] = {
    { "おめでとう<E>これで全てのＬＥＳＳＯＮは終了だ", 0 },
    { "明日からは実際の街中にでて<E>実地研修を行うとしよう", 0 },
    { "もっとも、運行に何か不安があったら<E>ここに来て練習すればいい", 0 },
    { "次からは好きなＬＥＳＳＯＮが選べるから<E>有効に活用してくれ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonNext_8c044848[] = {
    { "よーし、次のＬＥＳＳＯＮに進もう", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonRetry_8c044858[] = {
    { "誰だって最初はうまくできないものさ<E>気を落とさないで、もう一度やってみよう", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonTips_8c044868[] = {
    { "ここでは運行中の諸注意を<E>ポイント毎に練習できるぞ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonWarning_8c044878[] = {
    { "一度ＬＥＳＳＯＮを選ぶと、今日はコースに<E>出られなくなるので、気を付けるように", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonChoose_8c044888[] = {
    { "練習したいＬＥＳＳＯＮを選んでくれ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqScoreRecord_8c044898[] = {
    { "そうそう、最高得点が更新されたようだ<E>この調子で頑張ってくれよ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonFinalDay_8c0448a8[] = {
    { "今日は研修の最終日だぞ<E>もう練習しているより、実戦あるのみ", 0 },
    { "まぁ、どうしてもと言うのなら止めはしないが", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonPerfect_8c0448c0[] = {
    { "素晴らしい！<E>パーフェクトで合格だ", 0 },
    { "この調子で他のＬＥＳＳＯＮはもちろん<E>コースの方も頑張ってくれよ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonGood_8c0448d8[] = {
    { "しかし、ミスはその１カ所だけのようだな", 0 },
    { "他はパーフェクトだっただけに<E>悔やまれるが、十分に合格点だ！", 0 },
    { "今度走る時はパーフェクトを<E>目指してみてはどうかな？", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonPass_8c0448f8[] = {
    { "まぁ、他にもミスはあったが<E>致命的なものではないようだな", 0 },
    { "少し不安は残るが、一応は合格点だ<E>次はもっと高得点を目指してくれよ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonFailMinor_8c044910[] = {
    { "ミスはその１カ所だけのようだが<E>状況によっては大事故になる危険なミスだぞ", 1 },
    { "残念だが今回は不合格だな<E>気を取り直して、次は頑張ってくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonFailMajor_8c044928[] = {
    { "…その他にもミスが目立つようだ<E>とても合格点はあげられないな", 1 },
    { "乗客を乗せているということは、<E>命を預かっているのと同じことなんだ", 1 },
    { "我々の不注意が、多くの人々を危険にさらして<E>しまう事を、よく肝に銘じて練習に励んでくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqCollisionCarMinor_8c044948[] = {
    { "他の車に接触してしまったようだな…", 1 },
    { "車間距離を十分に取り、<E>早めにブレーキをかけるようにしてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqCollisionCarMedium_8c044960[] = {
    { "スピードは出ていなかったようだが<E>停車車両に接触してしまったな…", 1 },
    { "一日も早く、車両感覚を身につけてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqCollisionCarSevere_8c044978[] = {
    { "停車車両に接触してしまったようだな…<E>しかもスピードが大分出ていたな", 1 },
    { "一日も早く、車両感覚を身につけてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqCollisionCarFatal_8c044990[] = {
    { "停車車両に接触してしまったようだな…<E>しかも凄いスピードで…", 1 },
    { "まず速度はむやみに出さないこと！<E>そして車両感覚をしっかり身につけてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqCollisionWallMinor_8c0449a8[] = {
    { "スピードは出ていなかったようだが<E>壁に接触してしまったな…", 1 },
    { "一日も早く、車両感覚を身につけてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqCollisionWallMedium_8c0449c0[] = {
    { "壁に接触してしまったようだな…<E>しかもスピードが大分出ていたな", 1 },
    { "一日も早く、車両感覚を身につけてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqCollisionWallSevere_8c0449d8[] = {
    { "壁に接触してしまったようだな…<E>しかも凄いスピードで…", 1 },
    { "まず速度はむやみに出さないこと！<E>そして車両感覚をしっかり身につけてくれ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqNearMissPedestrian_8c0449f0[] = {
    { "ふー、危機一髪だったな", 1 },
    { "おいおい、通行人の安全は<E>何に替えても守ってくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqOffCourseMinor_8c044a08[] = {
    { "コースを外れてしまったようだな…<E>コースは常に正確に走ってくれないと困るぞ", 1 },
    { "コースから外れかけても、慌てずに速度を落とし<E>正しいコースに戻ればいい", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqOffCourseMedium_8c044a20[] = {
    { "コースを外れてしまったようだな…<E>コースは常に正確に走ってくれないと困るぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqOffCourseMajor_8c044a30[] = {
    { "コースを外れてしまったようだな…<E>コースは常に正確に走ってくれないと困るぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqSpeedingMinor_8c044a40[] = {
    { "少しスピードオーバーしたみたいだな…<E>制限速度には常に注意を払ってくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqSpeedingMajor_8c044a50[] = {
    { "大分スピードオーバーしたようだな…<E>制限速度を守るのは安全運転の基本だぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqWrongLane_8c044a60[] = {
    { "対向車線を越えるとは言語道断！<E>大事故の元なので、以後、絶対にないように", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqLaneStraddle_8c044a70[] = {
    { "車体が大きいので大変なのは分かるが<E>車線をまたがって走ってはいかんぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqNoSignal_8c044a80[] = {
    { "ウインカーを忘れていたぞ<E>車線変更や交差点での右左折", 1 },
    { "そして停留所に停車する時は<E>必ずウインカーを出すようにな", 1 },
    { "大切なのは、自分の行動を<E>周りの車にちゃんと知らせることだ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqNoSignalTurn_8c044aa0[] = {
    { "右左折する時のウインカーを忘れていたぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqUkn49_8c044ab0[] = {
    { "", 0 },
};

STATIC MenuDialog init_seqSignalViolation_8c044ab8[] = {
    { "おいおい、大事故になる所だったぞ", 1 },
    { "信号はしっかりと確認してくれよ<E>交通法規の基本中の基本だぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqBadStopLine_8c044ad0[] = {
    { "停止時にちょっと失敗したな", 1 },
    { "停止線をきちんと確認して、<E>正しい位置に停止するようにしてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqIllegalLaneChange_8c044ae8[] = {
    { "してはいけない所で車線変更をしたようだな…", 1 },
    { "交差点の手前などでは禁止されているので<E>気を付けてくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqBlockIntersection_8c044b00[] = {
    { "交差点での立ち往生は<E>絶対にしてはいけないぞ", 1 },
    { "事故や渋滞を引き起こす危険があるからな…<E>交差点は素早く通り抜けることだ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqWrongWay_8c044b18[] = {
    { "コースを正しく走る<E>これが運転の基本だ", 1 },
    { "落ち着いて走れば逆走などするわけがない<E>大事故になるので絶対にやめてくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqRapidAccel_8c044b30[] = {
    { "基本的には、お客さんあってのバスだからね<E>急発進などしないように気を付けよう", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqHardBrake_8c044b40[] = {
    { "急ブレーキがあったようだな<E>今度はお客さんの身になって運転してみるといい", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqSwerving_8c044b50[] = {
    { "荒っぽい運転をすると<E>立っているお客さんは大変だぞ", 1 },
    { "もっと安全運転を心がけて<E>急ハンドルなんてしないようにな", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqMissedStop_8c044b68[] = {
    { "バスが停留所を通り過ぎてどうするんだ", 1 },
    { "きちんと停留所情報を確認して<E>必要な停留所では停車してくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqBadStopPosition1_8c044b80[] = {
    { "停留所の停車位置が良くなかったようだな", 1 },
    { "今度からは、お客さんが乗り降り<E>しやすいように、正確に停車してくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqBadStopPosition2_8c044b98[] = {
    { "停留所の停車位置が良くなかったようだな", 1 },
    { "今度からは、お客さんが乗り降り<E>しやすいように、正確に停車してくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqTimeManagement_8c044bb0[] = {
    { "運転技術はもちろんのこと<E>時間配分にも気を配ってくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqAnnouncement_8c044bc0[] = {
    { "バスの運転手がお客さんにしなければならない<E>こととして、車内アナウンスは重要だぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqDoorOperation_8c044bd0[] = {
    { "ドアの開閉の確認は基本事項だぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqFreeRunIntro2_8c044be0[] = {
    { "ここでは、好きなコースを<E>何度でも走る事ができるぞ", 0 },
    { "但しストーリーモードで<E>走ったコースに限るがな", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqFreeRunChooseCourse_8c044bf8[] = {
    { "さぁ、コースを選んでくれ", 0 },
    { "", 0 },
};

MenuDialog *init_dialogSequences_8c044c08[] = {
    init_seqStoryIntro_8c0445d0,
    init_seqSuccessPerfect_8c044620,
    init_seqSuccessHigh_8c044650,
    init_seqSuccessNormal_8c044680,
    init_seqFailureFinal_8c0446b8,
    init_seqFreeRunIntro_8c0446e0,
    init_seqStoryChooseCourse_8c0446f8,
    init_seqGoodPractice_8c044708,
    init_seqSuccess_8c044718,
    init_seqAwardBadgeGold_8c044728,
    init_seqAwardBadgeSilver_8c044748,
    init_seqAwardBadgeBronze_8c044760,
    init_seqFailureRetry_8c044778,
    init_seqCourseUnlocked_8c044790,
    init_seqPassengerLetter_8c0447b0,
    init_seqCourseLocked_8c0447d0,
    init_seqForcePractice_8c0447e0,
    init_seqFinalDay_8c0447f0,
    init_seqLessonIntro_8c044808,
    init_seqLessonComplete_8c044820,
    init_seqLessonNext_8c044848,
    init_seqLessonRetry_8c044858,
    init_seqLessonTips_8c044868,
    init_seqLessonWarning_8c044878,
    init_seqLessonChoose_8c044888,
    init_seqScoreRecord_8c044898,
    init_seqLessonFinalDay_8c0448a8,
    init_seqLessonPerfect_8c0448c0,
    init_seqLessonGood_8c0448d8,
    init_seqLessonPass_8c0448f8,
    init_seqLessonFailMinor_8c044910,
    init_seqLessonFailMajor_8c044928,
    init_seqCollisionCarMinor_8c044948,
    init_seqCollisionCarMedium_8c044960,
    init_seqCollisionCarSevere_8c044978,
    init_seqCollisionCarFatal_8c044990,
    init_seqCollisionWallMinor_8c0449a8,
    init_seqCollisionWallMedium_8c0449c0,
    init_seqCollisionWallSevere_8c0449d8,
    init_seqNearMissPedestrian_8c0449f0,
    init_seqOffCourseMinor_8c044a08,
    init_seqOffCourseMedium_8c044a20,
    init_seqOffCourseMajor_8c044a30,
    init_seqSpeedingMinor_8c044a40,
    init_seqSpeedingMajor_8c044a50,
    init_seqWrongLane_8c044a60,
    init_seqLaneStraddle_8c044a70,
    init_seqNoSignal_8c044a80,
    init_seqNoSignalTurn_8c044aa0,
    init_seqUkn49_8c044ab0,
    init_seqSignalViolation_8c044ab8,
    init_seqBadStopLine_8c044ad0,
    init_seqIllegalLaneChange_8c044ae8,
    init_seqBlockIntersection_8c044b00,
    init_seqWrongWay_8c044b18,
    init_seqRapidAccel_8c044b30,
    init_seqHardBrake_8c044b40,
    init_seqSwerving_8c044b50,
    init_seqMissedStop_8c044b68,
    init_seqBadStopPosition1_8c044b80,
    init_seqBadStopPosition2_8c044b98,
    init_seqTimeManagement_8c044bb0,
    init_seqAnnouncement_8c044bc0,
    init_seqDoorOperation_8c044bd0,
    init_seqFreeRunIntro2_8c044be0,
    init_seqFreeRunChooseCourse_8c044bf8
};

// 30 days -> course variant index (0-2)
Uint8 init_courseVariants_8c044d10[30] = {
    0, 0, 1, 2, 2, 0, 0,
    1, 1, 0, 2, 2, 0, 1,
    2, 1, 0, 2, 2, 0, 1,
    1, 2, 0, 2, 2, 0, 1,
    1, 0
};

// 3 courses -> 3 shifts -> hh, mm
Uint8 init_routeInfoTime_8c044d2e[3 * 3 * 2] = {
    // hh, mm
    12, 28,
    16, 52,
    20, 36,

    02, 05,
    16, 44,
    20, 48,

    12, 46,
    17, 04,
    19, 22
};

STATIC ResourceGroupInfo init_courseResourceGroup_8c044d40 = {
    "corse_parts.dat",
    "course.dat",
    "corse.pvm",
    4
};

/* ====================
 * Forward Declarations
 * ====================
 */

extern void CourseMenuFreeResourceGroup_8c0185c4(ResourceGroup *res_group);
extern void CourseMenuFreeRunMenuTask_8c017ada(Task * task, void *state);
extern void CourseMenuRequestCommonResources_8c01852c(void);

/* =========
 * Functions
 * =========
 */

/**
 * Returns 1 if the cursor has reached its target position, 0 otherwise.
 */
int CourseMenuInterpolateCursor_8c016d2c()
{
    menuState_8c1bc7a8.pos.cursor.cursor_0x20.x += menuState_8c1bc7a8.cursorVelocity_0x30.x;
    menuState_8c1bc7a8.pos.cursor.cursor_0x20.y += menuState_8c1bc7a8.cursorVelocity_0x30.y;

    if (menuState_8c1bc7a8.cursorVelocity_0x30.x) {
        if (
            (menuState_8c1bc7a8.cursorVelocity_0x30.x >= 0)
            || (menuState_8c1bc7a8.pos.cursor.cursor_0x20.x > menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x)
        ) {
            // if (!(menuState_8c1bc7a8.cursorVelocity_0x30.x > 0)) {
            if (menuState_8c1bc7a8.cursorVelocity_0x30.x <= 0) {
                return 0;
            }

            // if (!(menuState_8c1bc7a8.pos.cursor.cursor_0x20.x > menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x)) {
            if (menuState_8c1bc7a8.pos.cursor.cursor_0x20.x <= menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x) {
                return 0;
            }

        }
        menuState_8c1bc7a8.pos.cursor.cursor_0x20 = menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28;
    } else if (menuState_8c1bc7a8.cursorVelocity_0x30.y) {
        if (
            (menuState_8c1bc7a8.cursorVelocity_0x30.y >= 0)
            || (menuState_8c1bc7a8.pos.cursor.cursor_0x20.y > menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.y)
        ) {
            // if (!(menuState_8c1bc7a8.cursorVelocity_0x30.y > 0)) {
            if (menuState_8c1bc7a8.cursorVelocity_0x30.y <= 0) {
                return 0;
            }

            if (!(menuState_8c1bc7a8.pos.cursor.cursor_0x20.y > menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.y)) {
                return 0;
            }

        }
        menuState_8c1bc7a8.pos.cursor.cursor_0x20 = menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28;
    }

    return 1;
}

STATIC int cursorOffTarget_8c016dc6()
{
    int selected;
    float y;
    float x;

    selected = menuState_8c1bc7a8.field_0x3c + menuState_8c1bc7a8.field_0x40 * 5;
    x = init_courseMenuButtons_8c04442c[selected].x_0x08;
    y = init_courseMenuButtons_8c04442c[selected].y_0x0c;
    if (
        (menuState_8c1bc7a8.pos.cursor.cursor_0x20.x == x)
        && (menuState_8c1bc7a8.pos.cursor.cursor_0x20.y == y)
    ) {
        return 0;
    }
    menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x = x;
    menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.y = y;
    menuState_8c1bc7a8.cursorVelocity_0x30.x = (x - menuState_8c1bc7a8.pos.cursor.cursor_0x20.x) / 6.0;
    menuState_8c1bc7a8.cursorVelocity_0x30.y = (y - menuState_8c1bc7a8.pos.cursor.cursor_0x20.y) / 6.0;
    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
    swapMessageBoxFor_8c02aefc("");
    return 1;
}

STATIC void drawInteger_8c016e6c(int value, float x, float y)
{
    do {
        drawSprite_8c014f54(
            &menuState_8c1bc7a8.resourceGroupA_0x00,
            15 + value % 10,
            x,
            y,
            -4.0
        );
        x -= 10.0;
    } while (value /= 10);
}

STATIC unsigned int getWeekDayIndex_8c016ed2()
{
    unsigned int r = var_progress_8c1ba1cc.days_0x00 + 1;
    return r % 7;
}

void CourseMenuDrawDateAndExp_8c016ee6()
{
    float x;
    int days, sprite_id;

    // Draw date
    days = var_progress_8c1ba1cc.days_0x00;
    if (days < 10) {
        x = 84.0;
    } else {
        x = 95.0;
    }
    drawInteger_8c016e6c(days, x, 82.0);

    // Hmm...
    if (days == 15) {
        sprite_id = 13;
    } else if (days == 23) {
        sprite_id = 14;
    } else {
        sprite_id = 6 + getWeekDayIndex_8c016ed2();
    }

    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupA_0x00,
        sprite_id,
        112.0,
        82.0,
        -4.0
    );

    drawInteger_8c016e6c(var_progress_8c1ba1cc.field_0x90, 534.0, 82.0);
}

STATIC void dialogSequenceTask_8c016f98(DialogSequenceTask *task, DialogSequenceTaskState *state)
{
    switch(state->state_0x00) {
        case 0: {
            int r;

            if (!*(state->dialog_0x04->text_0x00)) {
                var_dialogSequenceIsActive_8c225fb4 = 0;
                freeTask_8c014b66((void *) task);
                return;
            }

            if (task->field_0x18 && *task->field_0x18) {
                snd_8c010cd6(2, *task->field_0x18);
                task->field_0x18++;
            }

            state->field_0x08 = swapMessageBoxFor_8c02aefc(state->dialog_0x04->text_0x00);
            menuState_8c1bc7a8.instructorSprite_0x60 = state->dialog_0x04->instructorSpriteNo_0x04;
            state->field_0x0c = 1;
            state->field_0x10 = 0;
            state->state_0x00 = 1;
            break;
        }

        case 1: {
            if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
                state->field_0x10 = 99;
                state->state_0x00 = 2;
                FUN_8c010ca6(1);
            }

            if (++state->field_0x10 < 3) {
                break;
            }

            if (++state->field_0x0c < state->field_0x08) {
                state->field_0x10 = 0;
            } else {
                state->state_0x00 = 3;
            }

            break;
        }

        case 2: {
            if (!(var_peripherals_8c1ba35c[0].on & PDD_DGT_TA)) {
                state->state_0x00 = 1;
                break;
            }

            if ((state->field_0x0c += 2) < state->field_0x08) {
                break;
            }

            state->state_0x00 = 3;
            break;
        }

        case 3: {
            if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
                state->dialog_0x04++;
                state->state_0x00 = 0;
            }

            state->field_0x14 += 0x1111;
            drawSprite_8c014f54(
                &menuState_8c1bc7a8.resourceGroupA_0x00,
                44,
                32.0,
                -16.0 + 8 * njCos(state->field_0x14),
                -3.0
            );

            break;
        }
    }

    var_menuTextboxCharLimit_8c225fb8 = state->field_0x0c;
}

void CourseMenuPushDialogTask_8c0170c6(int dialog_index, int *p2)
{
    DialogSequenceTask *task;
    DialogSequenceTaskState *state;

    pushTask_8c014ae8(
        var_tasks_8c1ba3c8,
        &dialogSequenceTask_8c016f98,
        &task,
        &state,
        0x18
    );

    task->field_0x18 = p2;
    state->state_0x00 = 0;
    state->dialog_0x04 = init_dialogSequences_8c044c08[dialog_index];
    var_dialogSequenceIsActive_8c225fb4 = 1;
}

STATIC void swapDialogMessageBox_8c017108(int sequence)
{
    var_menuTextboxCharLimit_8c225fb8 = swapMessageBoxFor_8c02aefc(
        init_dialogSequences_8c044c08[sequence]->text_0x00
    );
}

STATIC void handleCourseMenuInput_8c017126()
{
    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
        if (
            init_courseMenuButtons_8c04442c[
                menuState_8c1bc7a8.field_0x3c
                + menuState_8c1bc7a8.field_0x40 * 5
            ]
            .unlocked_0x04 == 0
        ) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
            swapDialogMessageBox_8c017108(SEQ_COURSE_LOCKED);
        } else {
            FUN_8c010bae(0);
            FUN_8c010bae(1);
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            CHANGE_STATE(COURSE_MENU_STATE_COURSE_SELECTED);
            menuState_8c1bc7a8.logo_timer_0x68 = 0;
        }
    }

    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KU) {
        do {
            if (--menuState_8c1bc7a8.field_0x40 < 0) {
                menuState_8c1bc7a8.field_0x40 = 2;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c
            ].enabled_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            CHANGE_STATE(COURSE_MENU_STATE_ANIMATING);
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KD) {
        do {
            if (++menuState_8c1bc7a8.field_0x40 > 2) {
                menuState_8c1bc7a8.field_0x40 = 0;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c
            ].enabled_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            CHANGE_STATE(COURSE_MENU_STATE_ANIMATING);
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KL) {
        do {
            if (--menuState_8c1bc7a8.field_0x3c < 0) {
                menuState_8c1bc7a8.field_0x3c = 4;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c
            ].enabled_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            CHANGE_STATE(COURSE_MENU_STATE_ANIMATING);
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KR) {
        do {
            if (++menuState_8c1bc7a8.field_0x3c > 4) {
                menuState_8c1bc7a8.field_0x3c = 0;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c
            ].enabled_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            CHANGE_STATE(COURSE_MENU_STATE_ANIMATING);
        }
    }
}

int CourseMenuBuildCourseUnlockList_8c0172dc()
{
    int i = 0;
    int j = 0;
    for (; i < 9; i++) {
        if (var_progress_8c1ba1cc.courses_0x44[i].unlocked_0x00)
            continue;

        switch (i) {
            case 0:
                continue;

            case 1:
                if (
                    var_progress_8c1ba1cc.days_0x00 < 8 ||
                    var_exp_8c1ba25c < 4000
                )
                    continue;
                break;

            case 2:
                if (
                    var_progress_8c1ba1cc.days_0x00 < 9 ||
                    var_exp_8c1ba25c < 5500
                )
                    continue;
                break;

            case 3:
                if (
                    var_progress_8c1ba1cc.days_0x00 < 5 ||
                    var_exp_8c1ba25c < 2000
                )
                    continue;
                break;

            case 4:
                if (
                    var_progress_8c1ba1cc.days_0x00 < 11 ||
                    var_exp_8c1ba25c < 8000
                )
                    continue;
                break;

            case 5:
                if (
                    var_progress_8c1ba1cc.days_0x00 < 13 ||
                    var_exp_8c1ba25c < 12000
                )
                    continue;
                break;

            case 6:
                continue;

            case 7:
                if (
                    var_progress_8c1ba1cc.days_0x00 < 3 ||
                    var_exp_8c1ba25c < 500
                )
                    continue;
                break;

            case 8:
                if (
                    var_progress_8c1ba1cc.days_0x00 < 6 ||
                    var_exp_8c1ba25c < 3000
                )
                    continue;
                break;
        }

        var_coursesToUnlock_8c225fd4[j] = i;
        j++;
    }

    var_coursesToUnlock_8c225fd4[j] = -1;
    return j;
}

void CourseMenuApplyUnlocks_8c0173e6(void)
{
    int i;
    for (i = 0; var_coursesToUnlock_8c225fd4[i] != -1; i++) {
        int j = var_coursesToUnlock_8c225fd4[i];
        var_progress_8c1ba1cc.courses_0x44[j].unlocked_0x00 = 1;
        var_progress_8c1ba1cc.courses_0x44[j].new_0x01 = 1;
    }
}

STATIC void buildCourseMenuDialogFlow_8c017420(void)
{
    int cur = 0;

    // Default choose course
    if (var_8c1bb8b8 == 0) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_STORY_CHOOSE_COURSE;
        var_dialogQueue_8c225fbc[cur]   = -1;
        return;
    }

    // On the first day, show the intro briefing
    if (var_progress_8c1ba1cc.days_0x00 == 1) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_STORY_INTRO;
        var_dialogQueue_8c225fbc[cur++] = SEQ_STORY_CHOOSE_COURSE;
        var_dialogQueue_8c225fbc[cur]   = -1;
        return;
    }

    // Special Success
    if (var_8c1bb8bc != 0) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_GOOD_PRACTICE;
        var_dialogQueue_8c225fbc[cur++] = SEQ_STORY_CHOOSE_COURSE;
        var_dialogQueue_8c225fbc[cur]   = -1;
        return;
    }

    // Result
    if (var_8c1bb8dc == 0) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_FAILURE_RETRY;
    } else {
        int award_seq = SEQ_SUCCESS;
        if      (var_award_8c1bb8f8 == 1) award_seq = SEQ_AWARD_BADGE_BRONZE;
        else if (var_award_8c1bb8f8 == 2) award_seq = SEQ_AWARD_BADGE_SILVER;
        else if (var_award_8c1bb8f8 == 3) award_seq = SEQ_AWARD_BADGE_GOLD;
        var_dialogQueue_8c225fbc[cur++] = award_seq;
    }

    // Course unlocked
    if (CourseMenuBuildCourseUnlockList_8c0172dc() != 0) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_COURSE_UNLOCKED;
    }

    // Passenger letter received
    if (((var_progress_8c1ba1cc.days_0x00 + 1) % 7) == 0) {
        int r = AsqGetRandomInRangeB_121be(6);
        if (var_progress_8c1ba1cc.letters_0x2c[r] == 0) {
            var_progress_8c1ba1cc.letters_0x2c[r] = 1;
            var_dialogQueue_8c225fbc[cur++] = SEQ_PASSENGER_LETTER;
        }
    }

    var_dialogQueue_8c225fbc[cur++] = SEQ_STORY_CHOOSE_COURSE;

    var_dialogQueue_8c225fbc[cur] = -1;
}


/* This is the original code for the function above. (with some gotos to be removed)
void buildCourseMenuDialogFlow_8c017420()
{
    bool bVar1_done;
    int iVar2;
    int iVar3_index;
    int iVar4_curDialog;

    iVar4_curDialog = 0;
    bVar1_done = false;
    iVar3_index = 0;
    do
    {
        if ((4 < iVar3_index) || (bVar1_done))
        {
            var_dialogQueue_8c225fbc[iVar4_curDialog] = -1;
            return;
        }
        if (iVar3_index == 0)
        {
            // This path always marks and done,
            // except if the loop is explicitly continued
            iVar2 = iVar4_curDialog;
            if (var_8c1bb8b8 != 0)
            {
                if (var_progress_8c1ba1cc.field_0x00 == 1)
                {
                    iVar2 = iVar4_curDialog + 1;
                    // SEQ_STORY_INTRO
                    var_dialogQueue_8c225fbc[iVar4_curDialog] = 0;
                }
                else
                {

                    if (var_8c1bb8bc == 0)
                    {
                        // This is the only path that can lead to a loop continue
                        if (var_8c1bb8dc == 0)
                        {
                            // SEQ_FAILURE_RETRY
                            var_dialogQueue_8c225fbc[iVar4_curDialog] = 12;
                            iVar4_curDialog = iVar4_curDialog + 1;
                        }
                        else
                        {
                            if (var_award_8c1bb8f8 == 0)
                            {
                                // SEQ_SUCCESS
                                iVar2 = 8;
                            }
                            else if (var_award_8c1bb8f8 == 1)
                            {
                                // SEQ_AWARD_BADGE_BRONZE
                                iVar2 = 11;
                            }
                            else if (var_award_8c1bb8f8 == 2)
                            {
                                // SEQ_AWARD_BADGE_SILVER
                                iVar2 = 10;
                            }
                            else
                            {
                                if (var_award_8c1bb8f8 != 3)
                                    goto LAB_8c01754c;
                                // SEQ_AWARD_BADGE_GOLD
                                iVar2 = 9;
                            }
                            var_dialogQueue_8c225fbc[iVar4_curDialog] = iVar2;
                            iVar4_curDialog = iVar4_curDialog + 1;
                        }
                        goto LAB_8c01754c;
                    }
                    // SEQ_SUCCESS_2
                    var_dialogQueue_8c225fbc[iVar4_curDialog] = 7;
                    iVar2 = iVar4_curDialog + 1;
                }
            }
            // SEQ_STORY_CHOOSE_COURSE
            var_dialogQueue_8c225fbc[iVar2] = 6;
            bVar1_done = true;
            iVar4_curDialog = iVar2 + 1;
        }
        else if (iVar3_index != 1)
        {
            if (iVar3_index == 2)
            {
                iVar2 = CourseMenuBuildCourseUnlockList_8c0172dc();
                if (iVar2 != 0)
                {
                    // SEQ_COURSE_UNLOCKED
                    var_dialogQueue_8c225fbc[iVar4_curDialog] = 13;
                    iVar4_curDialog = iVar4_curDialog + 1;
                }
            }
            else if (iVar3_index == 3)
            {
                iVar2 = (var_progress_8c1ba1cc.field_0x00 + 1) % 7;
                if (iVar2 == 0)
                {
                    iVar2 = AsqGetRandomInRangeB_121be(6);
                    if (var_progress_8c1ba1cc.field_0x2c[iVar2] == 0)
                    {
                        var_progress_8c1ba1cc.field_0x2c[iVar2] = 1;
                        // SEQ_PASSENGER_LETTER
                        var_dialogQueue_8c225fbc[iVar4_curDialog] = 14;
                        iVar4_curDialog = iVar4_curDialog + 1;
                    }
                }
            }
            else if (iVar3_index == 4)
            {
                // SEQ_STORY_CHOOSE_COURSE
                var_dialogQueue_8c225fbc[iVar4_curDialog] = 6;
                iVar4_curDialog = iVar4_curDialog + 1;
            }
        }
    LAB_8c01754c:
        iVar3_index = iVar3_index + 1;
    } while (true);
} */

// This function has been refactored.
STATIC void drawCourseButtons_8c017590()
{
    int i;

    if (menuState_8c1bc7a8.field_0x48) {
        drawSprite_8c014f54(
            &menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x18,
            menuState_8c1bc7a8.pos.cursor.cursor_0x20.x,
            menuState_8c1bc7a8.pos.cursor.cursor_0x20.y,
            -3.0
        );
    }

    // TODO: Extract length constant
    for (i = 0; i < 15; i++) {
        CourseMenuButton *btn = &init_courseMenuButtons_8c04442c[i];

        if (btn->unlocked_0x04 == 0 || btn->spriteNo_0x10 == 0)
            continue;

        drawSprite_8c014f54(
            &menuState_8c1bc7a8.resourceGroupB_0x0c,
            btn->spriteNo_0x10,
            0.0,
            0.0,
            -4.0
        );
    }

    // TODO: Extract length constant
    for (i = 0; i < 9; i++) {
        char spriteNo = var_game_mode_8c1bb8fc == 0
            ? var_progress_8c1ba1cc.courses_0x44[i].storySpriteNo_0x03
            : var_progress_8c1ba1cc.courses_0x44[i].freeRunSpriteNo_0x04;

        if (!spriteNo)
            continue;

        drawSprite_8c014f54(
            &menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x18 - spriteNo,
            240.0 + (i % 3) * 93.0,
            106.0 + (i / 3) * 74.0,
            -3.5
        );
    }
}

/* This is the original code for the function above.
void drawCourseButtons_8c017590()
{
    int iVar1;
    int iVar2;
    CourseMenuButton *pRVar3;
    CourseMenuButton *pRVar4;
    int iVar5;
    float x;
    float fVar6;
    float y;
    float fVar7;
    float priority;

    if (menuState_8c1bc7a8.field_0x48 != 0) {
        drawSprite_8c014f54(
            &menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x18,
            menuState_8c1bc7a8.pos.cursor.cursor_0x20.x,
            menuState_8c1bc7a8.pos.cursor.cursor_0x20.y,
            -3.0
        );
    }
    pRVar3 = init_courseMenuButtons_8c04442c;
    fVar7 = -4.0;
    fVar6 = 0.0;
    do {
        iVar2 = 0;
        pRVar4 = pRVar3;
        do {
            if ((pRVar4->field_0x04 != 0) && (pRVar4->field_0x10 != 0)) {
                drawSprite_8c014f54(
                    &menuState_8c1bc7a8.resourceGroupB_0x0c,
                    pRVar4->field_0x10,
                    fVar6,
                    fVar6,
                    fVar7
                );
            }
            iVar2 = iVar2 + 1;
            pRVar4 = pRVar4 + 1;
        } while (iVar2 < 5);
        pRVar3 = pRVar3 + 5;
    } while (pRVar3 < &init_courseMenuButtons_8c04442c[15]); // TODO: Extract length constant
    fVar7 = 240.0;
    fVar6 = 93.0;
    priority = -3.5;
    iVar2 = 0;
    do {
        iVar1 = iVar2 * 3;
        iVar5 = 0;
        y = (float)iVar2 * 74.0 + 106.0;
        do {
            x = fVar6 * (float)iVar5 + fVar7;
            if (var_game_mode_8c1bb8fc == 0) { // TODO: Extract constant
                if (var_progress_8c1ba1cc.courses_0x44[iVar1 + iVar5].field_0x03 != 0) {
                    drawSprite_8c014f54(
                        &menuState_8c1bc7a8.resourceGroupB_0x0c,
                        0x18 - var_progress_8c1ba1cc.courses_0x44[iVar1 + iVar5].field_0x03,
                        x,
                        y,
                        priority
                    );
                }
            }
            else if (var_progress_8c1ba1cc.courses_0x44[iVar1 + iVar5].field_0x04[0] != 0) {
                drawSprite_8c014f54(
                    &menuState_8c1bc7a8.resourceGroupB_0x0c,
                    0x18 - var_progress_8c1ba1cc.courses_0x44[iVar1 + iVar5].field_0x04[0],
                    x,
                    y,
                    priority
                );
            }
            iVar5++;
        } while (iVar5 < 3);
        iVar2++;
    } while (iVar2 < 3);
    return;
}
*/

void CourseMenuStoryMenuTask_8c017718(Task * task, void *state)
{
    switch (menuState_8c1bc7a8.state_0x18) {
        case COURSE_MENU_STATE_INIT: {
            if (getUknPvmBool_8c01432a())
                return;

            AsqFreeQueues_11f7e();
            CHANGE_STATE(COURSE_MENU_STATE_FADE_IN);
            FUN_8c010d8a();
            snd_8c010cd6(0, 15);
            push_fadein_8c022a9c(10);
            return;
        }

        case COURSE_MENU_STATE_FADE_IN: {
            if (isFading_8c226568 == 0) {
                CourseMenuPushDialogTask_8c0170c6(var_dialogQueue_8c225fbc[0], 0);
                CHANGE_STATE(COURSE_MENU_STATE_DIALOG);
            }
            break;
        }

        case COURSE_MENU_STATE_DIALOG: {
            // Dialog still running
            if (var_dialogSequenceIsActive_8c225fb4) break;

            if (var_dialogQueue_8c225fbc[task->field_0x08] == SEQ_COURSE_UNLOCKED) {
                int row;
                CourseMenuApplyUnlocks_8c0173e6();
                for (row = 0; row < 3; row++) {
                    int col;
                    for (col = 0; col < 3; col++) {
                        // We offset by 2 because the first two entries
                        // of each row are not courses buttons.
                        init_courseMenuButtons_8c04442c[2 + row * 5 + col].unlocked_0x04 =
                            var_progress_8c1ba1cc.courses_0x44[row * 3 + col].unlocked_0x00;
                    }
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[5], 1, 0x16, 0);
            }

            // TODO: Rename to dialogSequenceIndex
            task->field_0x08++;

            // If we finished the last dialog sequence
            if (var_dialogQueue_8c225fbc[task->field_0x08] == -1) {
                CHANGE_STATE(COURSE_MENU_STATE_IDLE);
                swapMessageBoxFor_8c02aefc("");
            }
            // Otherwise, start the next dialog sequence
            else {
                CourseMenuPushDialogTask_8c0170c6(var_dialogQueue_8c225fbc[task->field_0x08], 0);
                if (var_dialogQueue_8c225fbc[task->field_0x08] == SEQ_COURSE_UNLOCKED) {
                    midiResetFxAndPlay_8c010846(0, 0);
                }
            }
            break;
        }

        case COURSE_MENU_STATE_IDLE: {
            handleCourseMenuInput_8c017126();
            break;
        }

        case COURSE_MENU_STATE_ANIMATING: {
            if (!CourseMenuInterpolateCursor_8c016d2c())
                break;

            CHANGE_STATE(COURSE_MENU_STATE_IDLE);
            break;
        }

        case COURSE_MENU_STATE_COURSE_SELECTED: {
            if (++menuState_8c1bc7a8.logo_timer_0x68 > 10) {
                CHANGE_STATE(COURSE_MENU_STATE_FADE_OUT);
                push_fadeout_8c022b60(10);
            }
            menuState_8c1bc7a8.field_0x48 = menuState_8c1bc7a8.logo_timer_0x68 & 1;
            break;
        }

        case COURSE_MENU_STATE_FADE_OUT: {
            int buttonIndex;

            if (isFading_8c226568) {
                menuState_8c1bc7a8.field_0x48 = ++menuState_8c1bc7a8.logo_timer_0x68 & 1;
                break;
            }

            if (init_8c03bd80)
                return;

            if (menuState_8c1bc7a8.field_0x3c != 1 || menuState_8c1bc7a8.field_0x40 != 0) {
                CourseMenuFreeResourceGroup_8c0185c4(&menuState_8c1bc7a8.resourceGroupB_0x0c);
                var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
            }

            menuState_8c1bc7a8.selected_0x38 = 0;
            buttonIndex = menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c;
            menuState_8c1bc7a8.field_0x50 =
                init_courseMenuButtons_8c04442c[buttonIndex].courseId_0x18;

            var_8c1bb8dc = 1;
            var_8c1bb8b8 = 0;
            var_8c1bb8bc = 1;

            init_courseMenuButtons_8c04442c[buttonIndex].onSelect_0x14(task);
            return;
        }

        case COURSE_MENU_STATE_FADE_OUT_TO_MAIN_MENU: {
            if (isFading_8c226568)
                break;

            if (init_8c03bd80)
                return;

            var_8c1bb8b8 = 0;
            MainMenuSwitchFromTask_8c01a09a(task);
            return;
        }
    }

    CourseMenuDrawDateAndExp_8c016ee6();
    drawCourseButtons_8c017590();
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupB_0x0c, 10, 0.0, 0.0, -5.0
    );
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupA_0x00, 0x2b, 0.0, 0.0, -4.0
    );
    if (menuTextboxText_8c02af1c(var_menuTextboxCharLimit_8c225fb8) ) {
        drawSprite_8c014f54(
            &menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0, 0.0, -5.0
        );
    }

    // Draw instructor
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupB_0x0c,
        menuState_8c1bc7a8.instructorSprite_0x60,
        0.0,
        0.0,
        -6.0
    );

    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -7.0
    );
    AsqGetRandomA_12166();
}

void CourseMenuFreeRunMenuTask_8c017ada(Task * task, void *state)
{
    switch (menuState_8c1bc7a8.state_0x18) {
        case COURSE_MENU_STATE_INIT: {
            if (getUknPvmBool_8c01432a())
                return;

            AsqFreeQueues_11f7e();
            CHANGE_STATE(COURSE_MENU_STATE_FADE_IN);
            FUN_8c010d8a();
            snd_8c010cd6(0, 15);
            push_fadein_8c022a9c(10);
            return;
        }

        case COURSE_MENU_STATE_FADE_IN: {
            if (isFading_8c226568 == 0) {
                CourseMenuPushDialogTask_8c0170c6(var_dialogQueue_8c225fbc[0], 0);
                CHANGE_STATE(COURSE_MENU_STATE_DIALOG);
            }
            break;
        }

        case COURSE_MENU_STATE_DIALOG: {
            // Dialog still running
            if (var_dialogSequenceIsActive_8c225fb4) break;

            if (var_dialogQueue_8c225fbc[task->field_0x08] == SEQ_COURSE_UNLOCKED) {
                int row;
                CourseMenuApplyUnlocks_8c0173e6();
                for (row = 0; row < 3; row++) {
                    int col;
                    for (col = 0; col < 3; col++) {
                        // We offset by 2 because the first two entries
                        // of each row are not courses buttons.
                        init_courseMenuButtons_8c04442c[2 + row * 5 + col].unlocked_0x04 =
                            var_progress_8c1ba1cc.courses_0x44[row * 3 + col].unlocked_0x00;
                    }
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[5], 1, 0x16, 0);
            }

            // TODO: Rename to dialogSequenceIndex
            task->field_0x08++;

            // If we finished the last dialog sequence
            if (var_dialogQueue_8c225fbc[task->field_0x08] == -1) {
                CHANGE_STATE(COURSE_MENU_STATE_IDLE);
                swapMessageBoxFor_8c02aefc("");
            }
            // Otherwise, start the next dialog sequence
            else {
                CourseMenuPushDialogTask_8c0170c6(var_dialogQueue_8c225fbc[task->field_0x08], 0);
                if (var_dialogQueue_8c225fbc[task->field_0x08] == SEQ_COURSE_UNLOCKED) {
                    midiResetFxAndPlay_8c010846(0, 0);
                }
            }
            break;
        }

        case COURSE_MENU_STATE_IDLE: {
            handleCourseMenuInput_8c017126();
            break;
        }

        case COURSE_MENU_STATE_ANIMATING: {
            if (!CourseMenuInterpolateCursor_8c016d2c())
                break;

            CHANGE_STATE(COURSE_MENU_STATE_IDLE);
            break;
        }

        case COURSE_MENU_STATE_COURSE_SELECTED: {
            if (++menuState_8c1bc7a8.logo_timer_0x68 > 10) {
                CHANGE_STATE(COURSE_MENU_STATE_FADE_OUT);
                push_fadeout_8c022b60(10);
            }
            menuState_8c1bc7a8.field_0x48 = menuState_8c1bc7a8.logo_timer_0x68 & 1;
            break;
        }

        case COURSE_MENU_STATE_FADE_OUT: {
            int buttonIndex;

            if (isFading_8c226568) {
                menuState_8c1bc7a8.field_0x48 = ++menuState_8c1bc7a8.logo_timer_0x68 & 1;
                break;
            }

            if (init_8c03bd80)
                return;

            if (menuState_8c1bc7a8.field_0x3c != 1 || menuState_8c1bc7a8.field_0x40 != 0) {
                CourseMenuFreeResourceGroup_8c0185c4(&menuState_8c1bc7a8.resourceGroupB_0x0c);
                var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
            }

            menuState_8c1bc7a8.selected_0x38 = 0;
            buttonIndex = menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c;
            menuState_8c1bc7a8.field_0x50 =
                init_courseMenuButtons_8c04442c[buttonIndex].courseId_0x18;

            // var_8c1bb8dc = 1;
            // var_8c1bb8b8 = 0;
            // var_8c1bb8bc = 1;

            init_courseMenuButtons_8c04442c[buttonIndex].onSelect_0x14(task);
            return;
        }

        case COURSE_MENU_STATE_FADE_OUT_TO_MAIN_MENU: {
            if (isFading_8c226568)
                break;

            if (init_8c03bd80)
                return;

            // var_8c1bb8b8 = 0;
            MainMenuSwitchFromTask_8c01a09a(task);
            return;
        }
    }

    // CourseMenuDrawDateAndExp_8c016ee6();
    drawCourseButtons_8c017590();
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupB_0x0c, 9, 0.0, 0.0, -5.0
    );
    // drawSprite_8c014f54(
    //     &menuState_8c1bc7a8.resourceGroupA_0x00, 0x2b, 0.0, 0.0, -4.0
    // );
    if (menuTextboxText_8c02af1c(var_menuTextboxCharLimit_8c225fb8) ) {
        drawSprite_8c014f54(
            &menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0, 0.0, -5.0
        );
    }

    // Draw instructor
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupB_0x0c,
        menuState_8c1bc7a8.instructorSprite_0x60,
        0.0,
        0.0,
        -6.0
    );

    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -7.0
    );
    AsqGetRandomA_12166();
}


// This function has been refactored.
STATIC void buildFreeRunMenuDialogFlow_8c017a20(void)
{
    int idx = 0;

    if (var_shouldShowFreeRunIntro_8c1bb8c0) {
        var_dialogQueue_8c225fbc[idx++] = SEQ_FREE_RUN_INTRO_2;
    }

    var_dialogQueue_8c225fbc[idx++] = SEQ_FREE_RUN_CHOOSE_COURSE;
    var_dialogQueue_8c225fbc[idx]   = -1;

    var_shouldShowFreeRunIntro_8c1bb8c0 = 0;
}

// This is the original code for the function above.
/*
void buildFreeRunMenuDialogFlow_8c017a20(void)
{
    int queueIndex = 0;
    int step = 0;

    for (step = 0; step < 2; step++) {
        if (step == 0) {
            if (var_shouldShowFreeRunIntro_8c1bb8c0 != 0) {
                var_dialogQueue_8c225fbc[queueIndex] = SEQ_FREE_RUN_INTRO_2;
                queueIndex++;
            }
        }
        else if (step == 1) {
            var_dialogQueue_8c225fbc[queueIndex] = SEQ_FREE_RUN_CHOOSE_COURSE;
            queueIndex++;
        }
    }

    var_dialogQueue_8c225fbc[queueIndex] = -1;
    var_shouldShowFreeRunIntro_8c1bb8c0 = 0;
}
*/

STATIC void FUN_8c017d54(void)
{
    int enabled;
    int row;
    int game_mode = var_game_mode_8c1bb8fc;

    // Enable cursor
    menuState_8c1bc7a8.field_0x48 = 1;

    // Update cursor target/velocity if off-target
    cursorOffTarget_8c016dc6();

    // Snap current cursor position to its target
    menuState_8c1bc7a8.pos.cursor.cursor_0x20 = menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28;

    // Event and Album buttons: enabled in Story Mode, disabled in Free Run
    enabled = game_mode == 0 ? 1 : 0;
    init_courseMenuButtons_8c04442c[5].enabled_0x00 = enabled;
    init_courseMenuButtons_8c04442c[6].enabled_0x00 = enabled;

    // Refresh the 3x3 grid of course buttons from PlayerProgress
    for (row = 0; row < 3; row++) {
        int col;
        for (col = 0; col < 3; col++) {
            int courseIdx = row * 3 + col;
            int buttonIdx = 2 + row * 5 + col; // offset by 2 each row
            init_courseMenuButtons_8c04442c[buttonIdx].unlocked_0x04 =
                game_mode == 0
                    ? var_progress_8c1ba1cc.courses_0x44[courseIdx].unlocked_0x00
                    : var_progress_8c1ba1cc.courses_0x44[courseIdx].new_0x01;
        }
    }
}

void CourseMenuSwitchFromTask_8c017e18(Task *task)
{
    LOG_INFO(("[COURSE_MENU] Initializing course menu (mode=%d)\n", var_game_mode_8c1bb8fc));

    if (var_game_mode_8c1bb8fc == 0) {
        setTaskAction_8c014b3e(task, CourseMenuStoryMenuTask_8c017718);
        buildCourseMenuDialogFlow_8c017420();
    } else {
        setTaskAction_8c014b3e(task, CourseMenuFreeRunMenuTask_8c017ada);
        buildFreeRunMenuDialogFlow_8c017a20();
    }

    // Get instructor sprite from the first dialog entry
    menuState_8c1bc7a8.instructorSprite_0x60 =
        init_dialogSequences_8c044c08[
            var_dialogQueue_8c225fbc[0]
        ]->instructorSpriteNo_0x04;
    task->field_0x08 = 0;
    var_menuTextboxCharLimit_8c225fb8 = 0;
    var_demo_8c1bb8d0 = 0;
    FUN_8c017d54();
    njGarbageTexture(&var_tex_8c157af8, 0xc00);
    AsqInitQueues_11f36(8, 0, 0, 8);
    AsqResetQueues_11f6c();

    if (!CourseMenuRequestSysResgrp_8c018568(
        &menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_mainMenuResourceGroup_8c044264
    )) {
        AsqFreeQueues_11f7e();
        CHANGE_STATE(COURSE_MENU_STATE_FADE_IN);
        push_fadein_8c022a9c(10);
        snd_8c010cd6(0, 15);
        return;
    }

    setUknPvmBool_8c014330();
    AsqProcessQueues_11fe0(AsqNop_11120, 0, 0, 0, resetUknPvmBool_8c014322);
    CHANGE_STATE(COURSE_MENU_STATE_INIT);
}

void CourseMenuFUN_8c017ef2(void)
{
    Task *createdTask;
    void *createdState;

    LOG_INFO(("[COURSE_MENU] Setting up story course menu\n"));

    pushInputTask_8c0128cc();

    pushTask_8c014ae8(
        var_tasks_8c1ba3c8,
        &task_8c012f44,
        &createdTask,
        &createdState,
        0
    );

    pushTask_8c014ae8(
        var_tasks_8c1ba3c8,
        &CourseMenuStoryMenuTask_8c017718,
        &createdTask,
        &createdState,
        0
    );

    buildCourseMenuDialogFlow_8c017420();

    menuState_8c1bc7a8.instructorSprite_0x60 =
        init_dialogSequences_8c044c08[
            var_dialogQueue_8c225fbc[0]
        ]->instructorSpriteNo_0x04;

    createdTask->field_0x08 = 0;

    var_menuTextboxCharLimit_8c225fb8 = 0;

    njGarbageTexture(&var_tex_8c157af8, 0xc00);
    FUN_8c02ae3e(0x20, 0x180, -2.0, 0x240, 0x40, 0, 0, -1);
    swapMessageBoxFor_8c02aefc("");
    var_demo_8c1bb8d0 = 0;

    FUN_8c017d54();
    AsqInitQueues_11f36(8, 0, 0, 8);
    AsqResetQueues_11f6c();

    CourseMenuRequestSysResgrp_8c018568(
        &menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_mainMenuResourceGroup_8c044264
    );
    CourseMenuRequestCommonResources_8c01852c();
    setUknPvmBool_8c014330();
    AsqProcessQueues_11fe0(AsqNop_11120, 0, 0, 0, resetUknPvmBool_8c014322);

    CHANGE_STATE(COURSE_MENU_STATE_INIT);
}

STATIC void drawFixedInteger_8c01803e(float x, float y, int value, int digits)
{
    float tracking = 19.0;
    do {
        do {
            drawSprite_8c014f54(
                &menuState_8c1bc7a8.resourceGroupB_0x0c,
                12 + value % 10,
                x,
                y,
                -4.0
            );
            x -= tracking;
            digits--;
        } while (value /= 10);
    } while (digits > 0);
}

STATIC void drawRouteInfo_8c018118(void)
{
    int index = menuState_8c1bc7a8.field_0x40 * 6 + (menuState_8c1bc7a8.field_0x3c - 2) * 2;

    // Draw day
    drawFixedInteger_8c01803e(219.0, 108.0, var_progress_8c1ba1cc.days_0x00, 0);

    // Draw weekday sprite
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupB_0x0c,
        getWeekDayIndex_8c016ed2() + 0x16,
        281.0,
        110.0,
        -4.0
    );

    // Draw hour and minute
    drawFixedInteger_8c01803e(421.0, 108.0, init_routeInfoTime_8c044d2e[index], 2);
    drawFixedInteger_8c01803e(471.0, 108.0, init_routeInfoTime_8c044d2e[index + 1], 2);

    // Draw route info
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupB_0x0c,
        menuState_8c1bc7a8.field_0x40 + 9,
        0.0,
        0.0,
        -7.0
    );
}

STATIC void CourseConfirmMenuTask_8c0181b6(Task * task, void *state)
{
    switch (menuState_8c1bc7a8.state_0x18) {
        case COURSE_CONFIRM_STATE_INIT: {
            if (getUknPvmBool_8c01432a())
                return;

            AsqFreeQueues_11f7e();
            CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_FADE_IN);
            push_fadein_8c022a9c(10);
            snd_8c010cd6(0, 15);
            return;
        }

        case COURSE_CONFIRM_STATE_FADE_IN: {
            if (isFading_8c226568 == 0) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_PROMPT);
            }
            break;
        }

        case COURSE_CONFIRM_STATE_PROMPT: {
            int r = promptHandleBinary_16caa(&menuState_8c1bc7a8.selected_0x38);
            if (r == 1) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_FADE_OUT);
                push_fadeout_8c022b60(10);
            } else if (r == 2) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_FADE_OUT_TO_COURSE_MENU);
                FUN_8c010bae(0);
                FUN_8c010bae(1);
                push_fadeout_8c022b60(10);
            }
            break;
        }

        case COURSE_CONFIRM_STATE_FADE_OUT: {
            if (isFading_8c226568 == 0) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_ROUTE_INFO_FADE_IN);
                push_fadein_8c022a9c(0x14);
            }
            break;
        }

        case COURSE_CONFIRM_STATE_ROUTE_INFO_FADE_IN: {
            if (isFading_8c226568 == 0) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_ROUTE_INFO_DISPLAY);
                menuState_8c1bc7a8.logo_timer_0x68 = 0;
            }
            // State 4 uses drawRouteInfo instead of epilogue rendering
            drawRouteInfo_8c018118();
            return;
        }

        case COURSE_CONFIRM_STATE_ROUTE_INFO_DISPLAY: {
            menuState_8c1bc7a8.logo_timer_0x68++;
            if (menuState_8c1bc7a8.logo_timer_0x68 > 30) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_START_LOADING);
                FUN_8c010bae(0);
                FUN_8c010bae(1);
                push_fadeout_8c022b60(20);
            }
            // State 5 uses drawRouteInfo instead of epilogue rendering
            drawRouteInfo_8c018118();
            return;
        }

        case COURSE_CONFIRM_STATE_START_LOADING: {
            if (isFading_8c226568 == 0) {
                int i = 0;
                int courseIndex = menuState_8c1bc7a8.field_0x50 / 3;

                if (init_8c03bd80 != 0) {
                    // init is busy, just return early
                    return;
                }
                // Step 1: Initialize game systems
                FUN_8c016182();

                // Step 2: Get course index and check if unlocked
                if (var_progress_8c1ba1cc.courses_0x44[courseIndex].field_0x02 == 0) {
                    // Course not unlocked, mark it
                    var_8c1bb8e0 = 1;
                    var_progress_8c1ba1cc.courses_0x44[courseIndex].field_0x02 = 1;
                } else {
                    var_8c1bb8e0 = 0;
                }

                // Step 3: Initialize various game state variables
                var_8c1bb8e8 = 0;
                var_8c1bb8e4 = 0;
                var_8c1bb8f0 = 0;
                var_8c1bb8ec = 0x1d;
                var_8c1bb8f4 = 0;

                // Step 4: Copy progress data to two arrays (5 uint32 values each)
                for (i = 0; i < 5; i++) {
                    var_8c1ba2b8[i] = ((int*)(&var_progress_8c1ba1cc.field_0x04))[i];
                    var_8c1ba2cc[i] = ((int*)(&var_progress_8c1ba1cc.field_0x04))[i + 5];
                }

                // Step 5: Update field_0x50 by adding day-based lookup value
                menuState_8c1bc7a8.field_0x50 += 
                    init_courseVariants_8c044d10[var_progress_8c1ba1cc.days_0x00 - 1];

                // Step 6: Initialize game and push loading task
                pushLoadingTask_8c013310(menuState_8c1bc7a8.field_0x50);
                return;
            }
            // State 6 uses drawRouteInfo instead of epilogue rendering
            drawRouteInfo_8c018118();
            return;
        }

        case COURSE_CONFIRM_STATE_FADE_OUT_TO_COURSE_MENU: {
            if (isFading_8c226568 == 0) {
                if (init_8c03bd80 != 0) {
                    // init is busy, just return early
                    return;
                }

                CourseMenuFreeResourceGroup_8c0185c4(&menuState_8c1bc7a8.resourceGroupB_0x0c);
                var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
                CourseMenuSwitchFromTask_8c017e18(task);
                return;
            }
            break; // State 7 uses normal epilogue rendering
        }
    }

    // Epilogue rendering that runs every frame for this task
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupB_0x0c,
        menuState_8c1bc7a8.field_0x50 / 3,
        0.0,
        0.0,
        -4.0
    );

    // 2) Draw confirm/cancel prompt (sprite id = field_0x38 + 2)
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupA_0x00,
        menuState_8c1bc7a8.selected_0x38 + 2,
        376.0,
        378.0,
        -4.0
    );

    // 3) Foreground overlay
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupA_0x00,
        0,
        0.0,
        0.0,
        -7.0
    );
}

void CourseMenuConfirmInit_8c0184cc(Task *task)
{
    LOG_INFO(("[COURSE_MENU] Initializing course confirmation menu\n"));

    njGarbageTexture(&var_tex_8c157af8, 0xc00);
    setTaskAction_8c014b3e(task, CourseConfirmMenuTask_8c0181b6);
    CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_INIT);
    menuState_8c1bc7a8.selected_0x38 = 0;
    AsqInitQueues_11f36(8, 0, 0, 8);
    AsqResetQueues_11f6c();
    CourseMenuRequestSysResgrp_8c018568(
        &menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_courseResourceGroup_8c044d40
    );
    setUknPvmBool_8c014330();
    AsqProcessQueues_11fe0(AsqNop_11120, 0, 0, 0, resetUknPvmBool_8c014322);
    CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_INIT);
    return;
}

void CourseMenuRequestCommonResources_8c01852c(void)
{
    AsqRequestDat_11182(
        "\\SYSTEM",
        "common_parts.dat",
        &menuState_8c1bc7a8.resourceGroupA_0x00.tanim_0x04
    );
    AsqRequestDat_11182(
        "\\SYSTEM",
        "common.dat",
        &menuState_8c1bc7a8.resourceGroupA_0x00.contents_0x08
    );
    AsqRequestPvm_11ac0("\\SYSTEM", "common.pvm", &menuState_8c1bc7a8, 1, 0);
    return;
}

int CourseMenuRequestSysResgrp_8c018568(ResourceGroup *res_group, ResourceGroupInfo *res_group_info)
{
    if (var_currentSysResGroupInfo_8c225fb0 == res_group_info) {
        return 0;
    }

    var_currentSysResGroupInfo_8c225fb0 = res_group_info;

    if (res_group->tlist_0x00 != (void *) -1) {
        CourseMenuFreeResourceGroup_8c0185c4(res_group);
    }

    AsqRequestDat_11182(
        "\\SYSTEM", res_group_info->parts, &res_group->tanim_0x04
    );
    AsqRequestDat_11182(
        "\\SYSTEM", res_group_info->dat, &res_group->contents_0x08
    );
    AsqRequestPvm_11ac0(
        "\\SYSTEM",
        res_group_info->pvm, 
        res_group,
        res_group_info->tex_count,
        0
    );

    return 1;
}

void CourseMenuFreeResourceGroup_8c0185c4(ResourceGroup *res_group)
{
    if (res_group->tlist_0x00 == (void *) -1) {
        return;
    }
    AsqReleaseAndFreeTexlist_11e3c(res_group->tlist_0x00);
    syFree(res_group->contents_0x08);
    syFree(res_group->tanim_0x04);
    res_group->tlist_0x00 = (void *) -1;
}
