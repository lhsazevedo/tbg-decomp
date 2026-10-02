/* @unit MessageBox */
/* 8c02a9fc */
#include <shinobi.h>

#include "02a9fc_message_box.h"
#include "011120_asset_queues.h"
#include "013ae8_route.h"
#include "014a9c_tasks.h"
#include "015034_text.h"
#include "016d2c_course_menu.h"
#include "0100bc_sound.h"
#include "022464_render.h"
#include "02af78_event.h"
#include "1ba1c8_globals.h"
#include "includes.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* A list ends with layers_0x00 == -1. layers_0x00 is a 0xffff-terminated
 * list of image ids, drawn back to front. */
typedef struct {
    unsigned short *layers_0x00;
    int lineListIndex_0x04;
} EventSlide;

typedef struct {
    int id_0x00;
    void *pvm_0x04;
    void *dat_0x08;
} MessageAssetEntry;

/* A list ends with an empty text_0x00. */
typedef struct {
    char *text_0x00;
    int voiceId_0x04;
} EventLine;

/* The stages fall through in order; phase_0x00 only picks where to enter. */
typedef enum {
    SLIDESHOW_STAGE_SWAP,
    SLIDESHOW_STAGE_WAIT,
    SLIDESHOW_STAGE_DRAW
} SlideshowStage;

typedef struct {
    int phase_0x00;
    int charCount_0x04;
    int revealedChars_0x08;
    int frameCounter_0x0c;
    EventSlide *slide_0x10;
    unsigned short *layers_0x14;
    EventLine *line_0x18;
} SlideshowState;

typedef struct {
    char *map_0x00;
    char *pvm_0x04;
} MessageAssetFiles;

/* ====================
 * Non-initialized Globals
 * ====================
 */

int var_selectedEventEntry_8c228478;
int var_messageBoxActive_8c22847c;
STATIC EventSlide **var_eventSlides_8c228480;
/* One per distinct image id in the selected event's slides. */
STATIC MessageAssetEntry var_messageAssets_8c228484[12];
STATIC int var_messageAssetCount_8c228514;
STATIC EventLine **var_messageTextDat_8c228518; /* -1 when not loaded */

/* ====================
 * Initialized Globals
 * ====================
 */

STATIC MessageAssetFiles init_slideAssetFiles_8c046758[] = {
    {"", ""},
    {"B001.map", "B001.pvm"},
    {"B002.map", "B002.pvm"},
    {"B003.map", "B003.pvm"},
    {"B004.map", "B004.pvm"},
    {"B005.map", "B005.pvm"},
    {"B006.map", "B006.pvm"},
    {"B007.map", "B007.pvm"},
    {"B008.map", "B008.pvm"},
    {"B009.map", "B009.pvm"},
    {"B010.map", "B010.pvm"},
    {"B011.map", "B011.pvm"},
    {"B012.map", "B012.pvm"},
    {"B013.map", "B013.pvm"},
    {"B014.map", "B014.pvm"},
    {"B015.map", "B015.pvm"},
    {"B016.map", "B016.pvm"},
    {"B017.map", "B017.pvm"},
    {"B018.map", "B018.pvm"},
    {"B019.map", "B019.pvm"},
    {"B020.map", "B020.pvm"},
    {"B021.map", "B021.pvm"},
    {"B022.map", "B022.pvm"},
    {"B023.map", "B023.pvm"},
    {"B024.map", "B024.pvm"},
    {"B025.map", "B025.pvm"},
    {"B026.map", "B026.pvm"},
    {"B027.map", "B027.pvm"},
    {"B028.map", "B028.pvm"},
    {"B029.map", "B029.pvm"},
    {"B030.map", "B030.pvm"},
    {"B031.map", "B031.pvm"},
    {"B032.map", "B032.pvm"},
    {"B033.map", "B033.pvm"},
    {"B034.map", "B034.pvm"},
    {"B035.map", "B035.pvm"},
    {"B036.map", "B036.pvm"},
    {"B037.map", "B037.pvm"},
    {"B038.map", "B038.pvm"},
    {"B039.map", "B039.pvm"},
    {"B040.map", "B040.pvm"},
    {"B041.map", "B041.pvm"},
    {"B042.map", "B042.pvm"},
    {"B043.map", "B043.pvm"},
    {"B044.map", "B044.pvm"},
    {"B045.map", "B045.pvm"},
    {"B046.map", "B046.pvm"},
    {"B047.map", "B047.pvm"},
    {"B048.map", "B048.pvm"},
    {"B049.map", "B049.pvm"},
    {"B050.map", "B050.pvm"},
    {"B051.map", "B051.pvm"},
    {"B052.map", "B052.pvm"},
    {"B053.map", "B053.pvm"},
    {"B054.map", "B054.pvm"},
    {"B055.map", "B055.pvm"},
    {"B056.map", "B056.pvm"},
    {"B057.map", "B057.pvm"},
    {"B058.map", "B058.pvm"},
    {"B059.map", "B059.pvm"},
    {"B060.map", "B060.pvm"},
    {"B061.map", "B061.pvm"},
    {"B062.map", "B062.pvm"},
    {"B063.map", "B063.pvm"},
    {"B064.map", "B064.pvm"},
    {"B065.map", "B065.pvm"},
    {"B066.map", "B066.pvm"},
    {"B067.map", "B067.pvm"},
    {"B068.map", "B068.pvm"},
    {"B069.map", "B069.pvm"},
    {"B070.map", "B070.pvm"},
    {"B071.map", "B071.pvm"},
    {"B072.map", "B072.pvm"},
    {"B073.map", "B073.pvm"},
    {"B074.map", "B074.pvm"},
    {"B075.map", "B075.pvm"},
    {"B076.map", "B076.pvm"},
    {"B077.map", "B077.pvm"},
    {"B078.map", "B078.pvm"},
    {"B079.map", "B079.pvm"},
    {"B080.map", "B080.pvm"},
    {"B081.map", "B081.pvm"},
    {"B082.map", "B082.pvm"},
    {"B083.map", "B083.pvm"},
    {"B084.map", "B084.pvm"},
    {"B085.map", "B085.pvm"},
    {"B086.map", "B086.pvm"},
    {"B087.map", "B087.pvm"},
    {"B088.map", "B088.pvm"},
    {"B089.map", "B089.pvm"},
    {"B090.map", "B090.pvm"},
    {"B091.map", "B091.pvm"},
    {"B092.map", "B092.pvm"},
    {"B093.map", "B093.pvm"},
    {"B094.map", "B094.pvm"},
    {"B095.map", "B095.pvm"},
    {"B096.map", "B096.pvm"},
    {"B097.map", "B097.pvm"},
    {"B098.map", "B098.pvm"},
    {"B099.map", "B099.pvm"},
    {"B100.map", "B100.pvm"},
    {"B101.map", "B101.pvm"},
    {"B102.map", "B102.pvm"},
    {"B103.map", "B103.pvm"},
    {"B104.map", "B104.pvm"},
    {"B105.map", "B105.pvm"},
    {"B106.map", "B106.pvm"},
    {"B107.map", "B107.pvm"},
    {"B108.map", "B108.pvm"},
    {"B109.map", "B109.pvm"},
    {"B110.map", "B110.pvm"},
    {"B111.map", "B111.pvm"},
    {"B112.map", "B112.pvm"},
    {"B113.map", "B113.pvm"},
    {"B114.map", "B114.pvm"},
    {"B115.map", "B115.pvm"},
    {"B116.map", "B116.pvm"},
    {"B117.map", "B117.pvm"},
    {"B118.map", "B118.pvm"},
    {"B119.map", "B119.pvm"},
    {"B120.map", "B120.pvm"},
    {"B121.map", "B121.pvm"},
    {"B122.map", "B122.pvm"},
    {"B123.map", "B123.pvm"},
    {"B124.map", "B124.pvm"},
    {"B125.map", "B125.pvm"},
    {"B126.map", "B126.pvm"},
    {"B127.map", "B127.pvm"},
    {"B128.map", "B128.pvm"},
    {"B129.map", "B129.pvm"},
    {"B130.map", "B130.pvm"},
    {"B131.map", "B131.pvm"},
    {"B132.map", "B132.pvm"},
    {"B133.map", "B133.pvm"},
    {"B134.map", "B134.pvm"},
    {"B135.map", "B135.pvm"},
    {"B136.map", "B136.pvm"},
    {"B137.map", "B137.pvm"},
    {"B138.map", "B138.pvm"},
    {"B139.map", "B139.pvm"},
    {"B140.map", "B140.pvm"},
    {"B141.map", "B141.pvm"},
    {"B142.map", "B142.pvm"},
    {"B143.map", "B143.pvm"},
    {"B144.map", "B144.pvm"},
    {"B145.map", "B145.pvm"},
    {"B146.map", "B146.pvm"},
    {"B147.map", "B147.pvm"},
    {"B148.map", "B148.pvm"},
    {"B149.map", "B149.pvm"},
    {"B150.map", "B150.pvm"},
    {"B151.map", "B151.pvm"},
    {"B152.map", "B152.pvm"},
    {"B153.map", "B153.pvm"},
    {"B154.map", "B154.pvm"},
    {"B155.map", "B155.pvm"},
    {"B156.map", "B156.pvm"},
    {"B157.map", "B157.pvm"},
    {"B158.map", "B158.pvm"},
    {"B159.map", "B159.pvm"},
    {"B160.map", "B160.pvm"},
    {"B161.map", "B161.pvm"},
    {"B162.map", "B162.pvm"},
    {"B163.map", "B163.pvm"},
    {"B164.map", "B164.pvm"},
    {"B165.map", "B165.pvm"},
    {"B166.map", "B166.pvm"},
    {"B167.map", "B167.pvm"},
    {"B168.map", "B168.pvm"},
    {"B169.map", "B169.pvm"},
    {"B170.map", "B170.pvm"},
    {"B171.map", "B171.pvm"},
    {"B172.map", "B172.pvm"},
    {"B173.map", "B173.pvm"},
    {"B174.map", "B174.pvm"},
    {"B175.map", "B175.pvm"},
    {"B176.map", "B176.pvm"},
    {"B177.map", "B177.pvm"},
    {"B178.map", "B178.pvm"},
    {"B179.map", "B179.pvm"},
    {"B180.map", "B180.pvm"},
    {"B181.map", "B181.pvm"},
    {"B182.map", "B182.pvm"},
    {"B183.map", "B183.pvm"},
    {"B184.map", "B184.pvm"},
    {"B185.map", "B185.pvm"},
    {"B186.map", "B186.pvm"},
    {"B187.map", "B187.pvm"},
    {"B188.map", "B188.pvm"},
    {"B189.map", "B189.pvm"},
    {"B190.map", "B190.pvm"},
    {"B191.map", "B191.pvm"},
    {"B192.map", "B192.pvm"},
    {"B193.map", "B193.pvm"},
    {"B194.map", "B194.pvm"},
    {"B195.map", "B195.pvm"},
    {"B196.map", "B196.pvm"},
    {"B197.map", "B197.pvm"},
    {"B198.map", "B198.pvm"},
    {"B199.map", "B199.pvm"},
    {"B200.map", "B200.pvm"},
    {"P001.map", "P001.pvm"},
    {"P002.map", "P002.pvm"},
    {"P003.map", "P003.pvm"},
    {"P004.map", "P004.pvm"},
    {"P005.map", "P005.pvm"},
    {"P006.map", "P006.pvm"},
    {"P007.map", "P007.pvm"},
    {"P008.map", "P008.pvm"},
    {"P009.map", "P009.pvm"},
    {"P010.map", "P010.pvm"},
    {"P011.map", "P011.pvm"},
    {"P012.map", "P012.pvm"},
    {"P013.map", "P013.pvm"},
    {"P014.map", "P014.pvm"},
    {"P015.map", "P015.pvm"},
    {"P016.map", "P016.pvm"},
    {"P017.map", "P017.pvm"},
    {"P018.map", "P018.pvm"},
    {"P019.map", "P019.pvm"},
    {"P020.map", "P020.pvm"},
    {"P021.map", "P021.pvm"},
    {"P022.map", "P022.pvm"},
    {"P023.map", "P023.pvm"},
    {"P024.map", "P024.pvm"},
    {"P025.map", "P025.pvm"},
    {"P026.map", "P026.pvm"},
    {"P027.map", "P027.pvm"},
    {"P028.map", "P028.pvm"},
    {"P029.map", "P029.pvm"},
    {"P030.map", "P030.pvm"},
    {"P031.map", "P031.pvm"},
    {"P032.map", "P032.pvm"},
    {"P033.map", "P033.pvm"},
    {"P034.map", "P034.pvm"},
    {"P035.map", "P035.pvm"},
    {"P036.map", "P036.pvm"},
    {"P037.map", "P037.pvm"},
    {"P038.map", "P038.pvm"},
    {"P039.map", "P039.pvm"},
    {"P040.map", "P040.pvm"},
    {"P041.map", "P041.pvm"},
    {"P042.map", "P042.pvm"},
    {"P043.map", "P043.pvm"},
    {"P044.map", "P044.pvm"},
    {"P045.map", "P045.pvm"},
    {"P046.map", "P046.pvm"},
    {"P047.map", "P047.pvm"},
    {"P048.map", "P048.pvm"},
    {"P049.map", "P049.pvm"},
    {"P050.map", "P050.pvm"},
    {"P051.map", "P051.pvm"},
    {"P052.map", "P052.pvm"},
    {"P053.map", "P053.pvm"},
    {"P054.map", "P054.pvm"},
};

/* Unreferenced; same {id..., 0xFFFF} shape as the id lists below. */
STATIC Uint16 init_slideLayers_8c046f50[] = {
    0x0000, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f54[] = {
    0x0001, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f58[] = {
    0x0001, 0x00C9, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f5e[] = {
    0x0002, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f62[] = {
    0x0003, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f66[] = {
    0x0004, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f6a[] = {
    0x0005, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f6e[] = {
    0x0005, 0x00CA, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f74[] = {
    0x0006, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f78[] = {
    0x0007, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f7c[] = {
    0x0007, 0x00CB, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f82[] = {
    0x0007, 0x00CB, 0x00CD, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f8a[] = {
    0x0007, 0x00CC, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f90[] = {
    0x0007, 0x00CC, 0x00CD, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f98[] = {
    0x0007, 0x00CD, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f9e[] = {
    0x0008, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fa2[] = {
    0x0008, 0x00CE, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fa8[] = {
    0x0009, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fac[] = {
    0x000A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fb0[] = {
    0x000A, 0x00CF, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fb6[] = {
    0x000B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fba[] = {
    0x000C, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fbe[] = {
    0x000D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fc2[] = {
    0x000D, 0x00D0, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fc8[] = {
    0x000E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fcc[] = {
    0x000F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fd0[] = {
    0x0010, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fd4[] = {
    0x0011, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fd8[] = {
    0x0012, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fdc[] = {
    0x0013, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fe0[] = {
    0x0013, 0x00D1, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fe6[] = {
    0x0013, 0x00D1, 0x00D3, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fee[] = {
    0x0013, 0x00D2, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046ff4[] = {
    0x0013, 0x00D2, 0x00D3, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046ffc[] = {
    0x0013, 0x00D3, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047002[] = {
    0x0014, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047006[] = {
    0x0015, 0xFFFF, 0x0015, 0x00D4, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047010[] = {
    0x0016, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047014[] = {
    0x0016, 0x00D5, 0xFFFF, 0x0016, 0x00D5, 0x00D7, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047022[] = {
    0x0016, 0x00D6, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047028[] = {
    0x0016, 0x00D6, 0x00D7, 0xFFFF, 0x0016, 0x00D7, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047036[] = {
    0x0017, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04703a[] = {
    0x0018, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04703e[] = {
    0x0018, 0x00D8, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047044[] = {
    0x0019, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047048[] = {
    0x001A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04704c[] = {
    0x001B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047050[] = {
    0x001C, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047054[] = {
    0x001D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047058[] = {
    0x001E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04705c[] = {
    0x001E, 0x00D9, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047062[] = {
    0x001F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047066[] = {
    0x0020, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04706a[] = {
    0x0020, 0x00DA, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047070[] = {
    0x0021, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047074[] = {
    0x0022, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047078[] = {
    0x0022, 0x00DB, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04707e[] = {
    0x0022, 0x00DB, 0x00DD, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047086[] = {
    0x0022, 0x00DC, 0xFFFF, 0x0154, 0x00DC, 0x00DD, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047094[] = {
    0x0022, 0x00DD, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04709a[] = {
    0x0023, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04709e[] = {
    0x0024, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470a2[] = {
    0x0024, 0x00DE, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470a8[] = {
    0x0025, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470ac[] = {
    0x0026, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470b0[] = {
    0x0026, 0x00DF, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470b6[] = {
    0x0027, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470ba[] = {
    0x0028, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470be[] = {
    0x0029, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470c2[] = {
    0x002A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470c6[] = {
    0x002B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470ca[] = {
    0x002B, 0x00E0, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470d0[] = {
    0x002C, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470d4[] = {
    0x002D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470d8[] = {
    0x002E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470dc[] = {
    0x002F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470e0[] = {
    0x0030, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470e4[] = {
    0x0031, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470e8[] = {
    0x0031, 0x00E1, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470ee[] = {
    0x0032, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470f2[] = {
    0x0033, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470f6[] = {
    0x0034, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470fa[] = {
    0x0034, 0x00E2, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047100[] = {
    0x0035, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047104[] = {
    0x0036, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047108[] = {
    0x0037, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04710c[] = {
    0x0038, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047110[] = {
    0x0039, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047114[] = {
    0x003A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047118[] = {
    0x003B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04711c[] = {
    0x003C, 0xFFFF, 0x003C, 0x00E3, 0xFFFF, 0x003C, 0x00E3, 0x00E5,
    0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04712e[] = {
    0x003C, 0x00E4, 0xFFFF, 0x003C, 0x00E4, 0x00E5, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04713c[] = {
    0x003C, 0x00E5, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047142[] = {
    0x003D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047146[] = {
    0x003E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04714a[] = {
    0x003E, 0x00E6, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047150[] = {
    0x003F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047154[] = {
    0x003F, 0x00E7, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04715a[] = {
    0x0040, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04715e[] = {
    0x0040, 0x00E8, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047164[] = {
    0x0041, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047168[] = {
    0x0042, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04716c[] = {
    0x0042, 0x00E9, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047172[] = {
    0x0042, 0x00E9, 0x00EA, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04717a[] = {
    0x0042, 0x00EA, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047180[] = {
    0x0043, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047184[] = {
    0x0044, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047188[] = {
    0x0045, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04718c[] = {
    0x0046, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047190[] = {
    0x0047, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047194[] = {
    0x0048, 0xFFFF, 0x0049, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04719c[] = {
    0x004A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471a0[] = {
    0x004B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471a4[] = {
    0x004C, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471a8[] = {
    0x004D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471ac[] = {
    0x004E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471b0[] = {
    0x004F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471b4[] = {
    0x0050, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471b8[] = {
    0x0050, 0x00EB, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471be[] = {
    0x0051, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471c2[] = {
    0x0052, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471c6[] = {
    0x0052, 0x00EC, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471cc[] = {
    0x0053, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471d0[] = {
    0x0054, 0xFFFF, 0x0054, 0x00ED, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471da[] = {
    0x0055, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471de[] = {
    0x0056, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471e2[] = {
    0x0057, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471e6[] = {
    0x0058, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471ea[] = {
    0x0059, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471ee[] = {
    0x005A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471f2[] = {
    0x005B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471f6[] = {
    0x005C, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471fa[] = {
    0x005D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471fe[] = {
    0x005E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047202[] = {
    0x005F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047206[] = {
    0x0060, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04720a[] = {
    0x0061, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04720e[] = {
    0x0062, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047212[] = {
    0x0063, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047216[] = {
    0x0064, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04721a[] = {
    0x0065, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04721e[] = {
    0x0066, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047222[] = {
    0x0066, 0x00EE, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047228[] = {
    0x0067, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04722c[] = {
    0x0068, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047230[] = {
    0x0069, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047234[] = {
    0x0069, 0x00EF, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04723a[] = {
    0x006A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04723e[] = {
    0x006B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047242[] = {
    0x006B, 0x00F0, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047248[] = {
    0x006B, 0x00F0, 0x00F1, 0xFFFF, 0x006B, 0x00F1, 0xFFFF, 0x006C,
    0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04725a[] = {
    0x006D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04725e[] = {
    0x006C, 0x00F2, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047264[] = {
    0x006E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047268[] = {
    0x006F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04726c[] = {
    0x0070, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047270[] = {
    0x0070, 0x00F3, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047276[] = {
    0x0071, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04727a[] = {
    0x0072, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04727e[] = {
    0x0073, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047282[] = {
    0x0074, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047286[] = {
    0x0074, 0x00F4, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04728c[] = {
    0x0075, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047290[] = {
    0x0076, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047294[] = {
    0x0077, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047298[] = {
    0x0077, 0x00F5, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04729e[] = {
    0x0078, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472a2[] = {
    0x0079, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472a6[] = {
    0x0079, 0x00F6, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472ac[] = {
    0x007A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472b0[] = {
    0x007B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472b4[] = {
    0x007C, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472b8[] = {
    0x007D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472bc[] = {
    0x007D, 0x00F7, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472c2[] = {
    0x007E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472c6[] = {
    0x007F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472ca[] = {
    0x007F, 0x00F8, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472d0[] = {
    0x0080, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472d4[] = {
    0x0081, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472d8[] = {
    0x0082, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472dc[] = {
    0x0083, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472e0[] = {
    0x0084, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472e4[] = {
    0x0085, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472e8[] = {
    0x0085, 0x00F9, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472ee[] = {
    0x0086, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472f2[] = {
    0x0086, 0x00FA, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472f8[] = {
    0x0087, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472fc[] = {
    0x0088, 0xFFFF, 0x0015, 0x00FB, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047306[] = {
    0x0016, 0x00FC, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04730c[] = {
    0x0016, 0x00D5, 0x00FC, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047314[] = {
    0x0016, 0x00D6, 0x00FC, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04731c[] = {
    0x0017, 0x00FD, 0xFFFF, 0x0066, 0x00FE, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047328[] = {
    0x0089, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04732c[] = {
    0x008A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047330[] = {
    0x008B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047334[] = {
    0x008C, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047338[] = {
    0x008D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04733c[] = {
    0x0008, 0x00CD, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047342[] = {
    0x0008, 0x00CD, 0x00CE, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04734a[] = {
    0x008E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04734e[] = {
    0x008F, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000,
};

STATIC EventSlide init_eventSlides_8c04740c[] = {
    { init_slideLayers_8c04715a, 0x00 },
    { init_slideLayers_8c047150, 0x01 },
    { init_slideLayers_8c047164, 0x02 },
    { init_slideLayers_8c04715a, 0x03 },
    { init_slideLayers_8c047150, 0x04 },
    { init_slideLayers_8c04715a, 0x05 },
    { init_slideLayers_8c047150, 0x06 },
    { init_slideLayers_8c04715a, 0x07 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047454[] = {
    { init_slideLayers_8c047150, 0x08 },
    { init_slideLayers_8c047164, 0x09 },
    { init_slideLayers_8c047150, 0x0a },
    { init_slideLayers_8c04715a, 0x0b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04747c[] = {
    { init_slideLayers_8c04715a, 0x0c },
    { init_slideLayers_8c047150, 0x0d },
    { init_slideLayers_8c047164, 0x0e },
    { init_slideLayers_8c04715a, 0x0f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0474a4[] = {
    { init_slideLayers_8c047150, 0x10 },
    { init_slideLayers_8c04715a, 0x11 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0474bc[] = {
    { init_slideLayers_8c047150, 0x12 },
    { init_slideLayers_8c04715a, 0x13 },
    { init_slideLayers_8c047164, 0x14 },
    { init_slideLayers_8c047150, 0x15 },
    { init_slideLayers_8c04715a, 0x16 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0474ec[] = {
    { init_slideLayers_8c047150, 0x17 },
    { init_slideLayers_8c047164, 0x18 },
    { init_slideLayers_8c047150, 0x19 },
    { init_slideLayers_8c04715a, 0x1a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047514[] = {
    { init_slideLayers_8c047154, 0x1b },
    { init_slideLayers_8c047164, 0x1c },
    { init_slideLayers_8c04715e, 0x1d },
    { init_slideLayers_8c047154, 0x1e },
    { init_slideLayers_8c04715e, 0x1f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047544[] = {
    { init_slideLayers_8c04715e, 0x20 },
    { init_slideLayers_8c047154, 0x21 },
    { init_slideLayers_8c04715e, 0x22 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047564[] = {
    { init_slideLayers_8c047154, 0x23 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047574[] = {
    { init_slideLayers_8c047172, 0x26 },
    { init_slideLayers_8c04717a, 0x27 },
    { init_slideLayers_8c047168, 0x28 },
    { init_slideLayers_8c047188, 0x29 },
    { init_slideLayers_8c047168, 0x2a },
    { init_slideLayers_8c04718c, 0x2b },
    { init_slideLayers_8c047188, 0x2c },
    { init_slideLayers_8c04718c, 0x2d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0475bc[] = {
    { init_slideLayers_8c047168, 0x2e },
    { init_slideLayers_8c047188, 0x2f },
    { init_slideLayers_8c04718c, 0x30 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0475dc[] = {
    { init_slideLayers_8c047168, 0x31 },
    { init_slideLayers_8c047188, 0x32 },
    { init_slideLayers_8c047168, 0x33 },
    { init_slideLayers_8c04716c, 0x34 },
    { init_slideLayers_8c04718c, 0x35 },
    { init_slideLayers_8c047168, 0x36 },
    { init_slideLayers_8c04718c, 0x37 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04761c[] = {
    { init_slideLayers_8c047168, 0x38 },
    { init_slideLayers_8c047188, 0x39 },
    { init_slideLayers_8c04718c, 0x3a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04763c[] = {
    { init_slideLayers_8c04717a, 0x3b },
    { init_slideLayers_8c047188, 0x3c },
    { init_slideLayers_8c047168, 0x3d },
    { init_slideLayers_8c04718c, 0x3e },
    { init_slideLayers_8c047168, 0x3f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04766c[] = {
    { init_slideLayers_8c047168, 0x40 },
    { init_slideLayers_8c047188, 0x41 },
    { init_slideLayers_8c04718c, 0x42 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04768c[] = {
    { init_slideLayers_8c047184, 0x43 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04769c[] = {
    { init_slideLayers_8c047184, 0x44 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0476ac[] = {
    { init_slideLayers_8c047180, 0x45 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0476bc[] = {
    { init_slideLayers_8c047180, 0x46 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0476cc[] = {
    { init_slideLayers_8c04715e, 0x47 },
    { init_slideLayers_8c047154, 0x48 },
    { init_slideLayers_8c047164, 0x49 },
    { init_slideLayers_8c047154, 0x4a },
    { init_slideLayers_8c047190, 0x4b },
    { init_slideLayers_8c047190, 0x4c },
    { init_slideLayers_8c047194, 0x4d },
    { init_slideLayers_8c047190, 0x4e },
    { init_slideLayers_8c047190, 0x4f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04771c[] = {
    { init_slideLayers_8c047190, 0x50 },
    { init_slideLayers_8c047194, 0x51 },
    { init_slideLayers_8c047190, 0x52 },
    { init_slideLayers_8c047190, 0x53 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047744[] = {
    { init_slideLayers_8c047190, 0x54 },
    { init_slideLayers_8c047194, 0x55 },
    { init_slideLayers_8c047190, 0x56 },
    { init_slideLayers_8c047190, 0x57 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04776c[] = {
    { init_slideLayers_8c04719c, 0x58 },
    { init_slideLayers_8c0471a4, 0x59 },
    { init_slideLayers_8c0471a0, 0x5a },
    { init_slideLayers_8c04719c, 0x5b },
    { init_slideLayers_8c0471a8, 0x5c },
    { init_slideLayers_8c0471a0, 0x5d },
    { init_slideLayers_8c0471a8, 0x5e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0477ac[] = {
    { init_slideLayers_8c04719c, 0x5f },
    { init_slideLayers_8c0471a4, 0x60 },
    { init_slideLayers_8c0471a8, 0x61 },
    { init_slideLayers_8c04719c, 0x62 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0477d4[] = {
    { init_slideLayers_8c04719c, 0x63 },
    { init_slideLayers_8c0471a4, 0x64 },
    { init_slideLayers_8c0471a0, 0x65 },
    { init_slideLayers_8c0471a8, 0x66 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0477fc[] = {
    { init_slideLayers_8c04719c, 0x67 },
    { init_slideLayers_8c0471a4, 0x68 },
    { init_slideLayers_8c0471a0, 0x69 },
    { init_slideLayers_8c0471a8, 0x6a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047824[] = {
    { init_slideLayers_8c0471a0, 0x6b },
    { init_slideLayers_8c04719c, 0x6c },
    { init_slideLayers_8c0471a4, 0x6d },
    { init_slideLayers_8c04719c, 0x6e },
    { init_slideLayers_8c0471a0, 0x6f },
    { init_slideLayers_8c0471a8, 0x70 },
    { init_slideLayers_8c04719c, 0x71 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047864[] = {
    { init_slideLayers_8c0471a0, 0x72 },
    { init_slideLayers_8c0471a4, 0x73 },
    { init_slideLayers_8c04719c, 0x74 },
    { init_slideLayers_8c0471a8, 0x75 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04788c[] = {
    { init_slideLayers_8c0471a0, 0x76 },
    { init_slideLayers_8c0471a4, 0x77 },
    { init_slideLayers_8c04719c, 0x78 },
    { init_slideLayers_8c0471a8, 0x79 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0478b4[] = {
    { init_slideLayers_8c0471a0, 0x7a },
    { init_slideLayers_8c0471a4, 0x7b },
    { init_slideLayers_8c04719c, 0x7c },
    { init_slideLayers_8c0471a8, 0x7d },
    { init_slideLayers_8c04719c, 0x7e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0478e4[] = {
    { init_slideLayers_8c0471ac, 0x7f },
    { init_slideLayers_8c04719c, 0x80 },
    { init_slideLayers_8c0471a0, 0x81 },
    { init_slideLayers_8c0471a4, 0x82 },
    { init_slideLayers_8c04719c, 0x83 },
    { init_slideLayers_8c0471a8, 0x84 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04791c[] = {
    { init_slideLayers_8c0471a0, 0x85 },
    { init_slideLayers_8c0471a4, 0x86 },
    { init_slideLayers_8c04719c, 0x87 },
    { init_slideLayers_8c0471a8, 0x88 },
    { init_slideLayers_8c04719c, 0x89 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04794c[] = {
    { init_slideLayers_8c04719c, 0x8a },
    { init_slideLayers_8c0471a4, 0x8b },
    { init_slideLayers_8c0471a8, 0x8c },
    { init_slideLayers_8c0471a0, 0x8d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047974[] = {
    { init_slideLayers_8c046fd4, 0x8e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047984[] = {
    { init_slideLayers_8c046fd4, 0x8f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047994[] = {
    { init_slideLayers_8c046fe6, 0x90 },
    { init_slideLayers_8c046ffc, 0x91 },
    { init_slideLayers_8c047006, 0x92 },
    { init_slideLayers_8c046fdc, 0x93 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0479bc[] = {
    { init_slideLayers_8c046fdc, 0x94 },
    { init_slideLayers_8c046fee, 0x95 },
    { init_slideLayers_8c047006, 0x96 },
    { init_slideLayers_8c046fe0, 0x97 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0479e4[] = {
    { init_slideLayers_8c046fe0, 0x98 },
    { init_slideLayers_8c047002, 0x99 },
    { init_slideLayers_8c047006, 0x9a },
    { init_slideLayers_8c046fee, 0x9b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047a0c[] = {
    { init_slideLayers_8c046fdc, 0x9c },
    { init_slideLayers_8c046fee, 0x9d },
    { init_slideLayers_8c047006, 0x9e },
    { init_slideLayers_8c047002, 0x9f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047a34[] = {
    { init_slideLayers_8c046fdc, 0xa0 },
    { init_slideLayers_8c047002, 0xa1 },
    { init_slideLayers_8c047006, 0xa2 },
    { init_slideLayers_8c046fdc, 0xa3 },
    { init_slideLayers_8c047002, 0xa4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047a64[] = {
    { init_slideLayers_8c046fdc, 0xa5 },
    { init_slideLayers_8c047006, 0xa6 },
    { init_slideLayers_8c046fdc, 0xa7 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047a84[] = {
    { init_slideLayers_8c046fe6, 0xa8 },
    { init_slideLayers_8c047002, 0xa9 },
    { init_slideLayers_8c047006, 0xaa },
    { init_slideLayers_8c046fe0, 0xab },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047aac[] = {
    { init_slideLayers_8c046ffc, 0xac },
    { init_slideLayers_8c047006, 0xad },
    { init_slideLayers_8c047002, 0xae },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047acc[] = {
    { init_slideLayers_8c046fe0, 0xaf },
    { init_slideLayers_8c047002, 0xb0 },
    { init_slideLayers_8c047006, 0xb1 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047aec[] = {
    { init_slideLayers_8c046fee, 0xb2 },
    { init_slideLayers_8c047002, 0xb3 },
    { init_slideLayers_8c047006, 0xb4 },
    { init_slideLayers_8c046fdc, 0xb5 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047b14[] = {
    { init_slideLayers_8c046fe6, 0xb6 },
    { init_slideLayers_8c046fd4, 0xb7 },
    { init_slideLayers_8c046fe0, 0xb8 },
    { init_slideLayers_8c047002, 0xb9 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047b3c[] = {
    { init_slideLayers_8c046fe6, 0xba },
    { init_slideLayers_8c046fd4, 0xbb },
    { init_slideLayers_8c047002, 0xbc },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047b5c[] = {
    { init_slideLayers_8c047328, 0xbd },
    { init_slideLayers_8c047334, 0xbe },
    { init_slideLayers_8c047330, 0xbf },
    { init_slideLayers_8c047328, 0xc0 },
    { init_slideLayers_8c04732c, 0xc1 },
    { init_slideLayers_8c047328, 0xc2 },
    { init_slideLayers_8c047334, 0xc3 },
    { init_slideLayers_8c047330, 0xc4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047ba4[] = {
    { init_slideLayers_8c047330, 0xc5 },
    { init_slideLayers_8c04732c, 0xc6 },
    { init_slideLayers_8c047334, 0xc7 },
    { init_slideLayers_8c047328, 0xc8 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047bcc[] = {
    { init_slideLayers_8c047328, 0xc9 },
    { init_slideLayers_8c047330, 0xca },
    { init_slideLayers_8c047328, 0xcb },
    { init_slideLayers_8c047330, 0xcc },
    { init_slideLayers_8c047328, 0xcd },
    { init_slideLayers_8c047334, 0xce },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047c04[] = {
    { init_slideLayers_8c047334, 0xcf },
    { init_slideLayers_8c047328, 0xd0 },
    { init_slideLayers_8c047334, 0xd1 },
    { init_slideLayers_8c047328, 0xd2 },
    { init_slideLayers_8c04732c, 0xd3 },
    { init_slideLayers_8c047328, 0xd4 },
    { init_slideLayers_8c04732c, 0xd5 },
    { init_slideLayers_8c047328, 0xd6 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047c4c[] = {
    { init_slideLayers_8c047330, 0xd7 },
    { init_slideLayers_8c047334, 0xd8 },
    { init_slideLayers_8c047328, 0xd9 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047c6c[] = {
    { init_slideLayers_8c047338, 0xda },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047c7c[] = {
    { init_slideLayers_8c047338, 0xdb },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047c8c[] = {
    { init_slideLayers_8c047338, 0xdc },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047c9c[] = {
    { init_slideLayers_8c047338, 0xdd },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047cac[] = {
    { init_slideLayers_8c047338, 0xde },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047cbc[] = {
    { init_slideLayers_8c047338, 0xdf },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047ccc[] = {
    { init_slideLayers_8c047338, 0xe0 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047cdc[] = {
    { init_slideLayers_8c047338, 0xe1 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047cec[] = {
    { init_slideLayers_8c047078, 0xe2 },
    { init_slideLayers_8c04709a, 0xe3 },
    { init_slideLayers_8c04709e, 0xe4 },
    { init_slideLayers_8c047074, 0xe5 },
    { init_slideLayers_8c047086, 0xe6 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047d1c[] = {
    { init_slideLayers_8c047074, 0xe7 },
    { init_slideLayers_8c04709e, 0xe8 },
    { init_slideLayers_8c04709a, 0xe9 },
    { init_slideLayers_8c04709e, 0xea },
    { init_slideLayers_8c047086, 0xeb },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047d4c[] = {
    { init_slideLayers_8c047078, 0xec },
    { init_slideLayers_8c04709a, 0xed },
    { init_slideLayers_8c04709e, 0xee },
    { init_slideLayers_8c047074, 0xef },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047d74[] = {
    { init_slideLayers_8c047074, 0xf0 },
    { init_slideLayers_8c04709e, 0xf1 },
    { init_slideLayers_8c04709a, 0xf2 },
    { init_slideLayers_8c0470a2, 0xf3 },
    { init_slideLayers_8c047086, 0xf4 },
    { init_slideLayers_8c047094, 0xf5 },
    { init_slideLayers_8c047074, 0xf6 },
    { init_slideLayers_8c04709a, 0xf7 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047dbc[] = {
    { init_slideLayers_8c047074, 0xf8 },
    { init_slideLayers_8c04709a, 0xf9 },
    { init_slideLayers_8c04709e, 0xfa },
    { init_slideLayers_8c047086, 0xfb },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047de4[] = {
    { init_slideLayers_8c0471ee, 0xfc },
    { init_slideLayers_8c0471f2, 0xfd },
    { init_slideLayers_8c0471b0, 0xfe },
    { init_slideLayers_8c0471b4, 0xff },
    { init_slideLayers_8c0471b8, 0x100 },
    { init_slideLayers_8c0471b4, 0x101 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047e1c[] = {
    { init_slideLayers_8c0471f6, 0x102 },
    { init_slideLayers_8c0471ee, 0x103 },
    { init_slideLayers_8c0471f6, 0x104 },
    { init_slideLayers_8c0471f2, 0x105 },
    { init_slideLayers_8c0471b0, 0x106 },
    { init_slideLayers_8c0471b4, 0x107 },
    { init_slideLayers_8c0471b0, 0x108 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047e5c[] = {
    { init_slideLayers_8c0471be, 0x109 },
    { init_slideLayers_8c0471c2, 0x10a },
    { init_slideLayers_8c0471c6, 0x10b },
    { init_slideLayers_8c0471be, 0x10c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047e84[] = {
    { init_slideLayers_8c0471be, 0x10d },
    { init_slideLayers_8c0471c2, 0x10e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047e9c[] = {
    { init_slideLayers_8c0471cc, 0x10f },
    { init_slideLayers_8c0471d0, 0x110 },
    { init_slideLayers_8c0471cc, 0x111 },
    { init_slideLayers_8c0471d0, 0x112 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047ec4[] = {
    { init_slideLayers_8c0471cc, 0x113 },
    { init_slideLayers_8c0471d0, 0x114 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047edc[] = {
    { init_slideLayers_8c0471ee, 0x115 },
    { init_slideLayers_8c0471f6, 0x116 },
    { init_slideLayers_8c0471b0, 0x117 },
    { init_slideLayers_8c0471b4, 0x118 },
    { init_slideLayers_8c0471b8, 0x119 },
    { init_slideLayers_8c0471b0, 0x11a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047f14[] = {
    { init_slideLayers_8c0471ee, 0x11b },
    { init_slideLayers_8c0471f2, 0x11c },
    { init_slideLayers_8c0471f6, 0x11d },
    { init_slideLayers_8c0471ee, 0x11e },
    { init_slideLayers_8c0471b0, 0x11f },
    { init_slideLayers_8c0471b4, 0x120 },
    { init_slideLayers_8c0471b8, 0x121 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047f54[] = {
    { init_slideLayers_8c0471be, 0x122 },
    { init_slideLayers_8c0471c6, 0x123 },
    { init_slideLayers_8c0471c2, 0x124 },
    { init_slideLayers_8c0471be, 0x125 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047f7c[] = {
    { init_slideLayers_8c0471be, 0x126 },
    { init_slideLayers_8c0471c2, 0x127 },
    { init_slideLayers_8c0471be, 0x128 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047f9c[] = {
    { init_slideLayers_8c0471da, 0x129 },
    { init_slideLayers_8c0471de, 0x12a },
    { init_slideLayers_8c0471e6, 0x12b },
    { init_slideLayers_8c0471e2, 0x12c },
    { init_slideLayers_8c0471ea, 0x12d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047fcc[] = {
    { init_slideLayers_8c0471fa, 0x12e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047fdc[] = {
    { init_slideLayers_8c0471fe, 0x12f },
    { init_slideLayers_8c047202, 0x130 },
    { init_slideLayers_8c047206, 0x131 },
    { init_slideLayers_8c0471fe, 0x132 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048004[] = {
    { init_slideLayers_8c0471fe, 0x133 },
    { init_slideLayers_8c047202, 0x134 },
    { init_slideLayers_8c0471fe, 0x135 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048024[] = {
    { init_slideLayers_8c0471fe, 0x136 },
    { init_slideLayers_8c047202, 0x137 },
    { init_slideLayers_8c047206, 0x138 },
    { init_slideLayers_8c0471fe, 0x139 },
    { init_slideLayers_8c047202, 0x13a },
    { init_slideLayers_8c047206, 0x13b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04805c[] = {
    { init_slideLayers_8c0471fe, 0x13c },
    { init_slideLayers_8c047206, 0x13d },
    { init_slideLayers_8c047202, 0x13e },
    { init_slideLayers_8c0471fe, 0x13f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048084[] = {
    { init_slideLayers_8c0471fe, 0x140 },
    { init_slideLayers_8c047202, 0x141 },
    { init_slideLayers_8c047206, 0x142 },
    { init_slideLayers_8c0471fe, 0x143 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0480ac[] = {
    { init_slideLayers_8c0471fe, 0x144 },
    { init_slideLayers_8c047206, 0x145 },
    { init_slideLayers_8c047202, 0x146 },
    { init_slideLayers_8c0471fe, 0x147 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0480d4[] = {
    { init_slideLayers_8c047216, 0x148 },
    { init_slideLayers_8c047118, 0x149 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0480ec[] = {
    { init_slideLayers_8c047118, 0x14a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0480fc[] = {
    { init_slideLayers_8c04720a, 0x14b },
    { init_slideLayers_8c047212, 0x14c },
    { init_slideLayers_8c04720a, 0x14d },
    { init_slideLayers_8c04720e, 0x14e },
    { init_slideLayers_8c04720a, 0x14f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04812c[] = {
    { init_slideLayers_8c04711c, 0x150 },
    { init_slideLayers_8c047142, 0x151 },
    { init_slideLayers_8c047146, 0x152 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04814c[] = {
    { init_slideLayers_8c047118, 0x153 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04815c[] = {
    { init_slideLayers_8c04711c, 0x154 },
    { init_slideLayers_8c047142, 0x155 },
    { init_slideLayers_8c047146, 0x156 },
    { init_slideLayers_8c04712e, 0x157 },
    { init_slideLayers_8c047146, 0x158 },
    { init_slideLayers_8c047142, 0x159 },
    { init_slideLayers_8c04711c, 0x15a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04819c[] = {
    { init_slideLayers_8c04711c, 0x15b },
    { init_slideLayers_8c047142, 0x15c },
    { init_slideLayers_8c047146, 0x15d },
    { init_slideLayers_8c04712e, 0x15e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0481c4[] = {
    { init_slideLayers_8c04711c, 0x15f },
    { init_slideLayers_8c04713c, 0x160 },
    { init_slideLayers_8c04711c, 0x161 },
    { init_slideLayers_8c047142, 0x162 },
    { init_slideLayers_8c047146, 0x163 },
    { init_slideLayers_8c04712e, 0x164 },
    { init_slideLayers_8c04714a, 0x165 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048204[] = {
    { init_slideLayers_8c047180, 0x166 },
    { init_slideLayers_8c04715e, 0x167 },
    { init_slideLayers_8c047154, 0x168 },
    { init_slideLayers_8c047164, 0x169 },
    { init_slideLayers_8c047154, 0x16a },
    { init_slideLayers_8c047190, 0x16b },
    { init_slideLayers_8c047190, 0x16c },
    { init_slideLayers_8c047194, 0x16d },
    { init_slideLayers_8c047190, 0x16e },
    { init_slideLayers_8c047190, 0x16f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04825c[] = {
    { init_slideLayers_8c047180, 0x170 },
    { init_slideLayers_8c047190, 0x171 },
    { init_slideLayers_8c047194, 0x172 },
    { init_slideLayers_8c047190, 0x173 },
    { init_slideLayers_8c047190, 0x174 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04828c[] = {
    { init_slideLayers_8c047180, 0x175 },
    { init_slideLayers_8c047190, 0x176 },
    { init_slideLayers_8c047194, 0x177 },
    { init_slideLayers_8c047190, 0x178 },
    { init_slideLayers_8c047190, 0x179 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0482bc[] = {
    { init_slideLayers_8c046fe0, 0x17a },
    { init_slideLayers_8c047002, 0x17b },
    { init_slideLayers_8c047006, 0x17c },
    { init_slideLayers_8c046fd4, 0x17d },
    { init_slideLayers_8c046fe6, 0x17e },
    { init_slideLayers_8c046fd4, 0x17f },
    { init_slideLayers_8c046fe0, 0x180 },
    { init_slideLayers_8c047002, 0x181 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048304[] = {
    { init_slideLayers_8c046fee, 0x182 },
    { init_slideLayers_8c047002, 0x183 },
    { init_slideLayers_8c047006, 0x184 },
    { init_slideLayers_8c046fdc, 0x185 },
    { init_slideLayers_8c046fd4, 0x186 },
    { init_slideLayers_8c046fe6, 0x187 },
    { init_slideLayers_8c046fd4, 0x188 },
    { init_slideLayers_8c047002, 0x189 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04834c[] = {
    { init_slideLayers_8c047216, 0x18a },
    { init_slideLayers_8c047118, 0x18b },
    { init_slideLayers_8c04720a, 0x18c },
    { init_slideLayers_8c047212, 0x18d },
    { init_slideLayers_8c04720a, 0x18e },
    { init_slideLayers_8c04720e, 0x18f },
    { init_slideLayers_8c04720a, 0x190 },
    { init_slideLayers_8c04711c, 0x191 },
    { init_slideLayers_8c047142, 0x192 },
    { init_slideLayers_8c047146, 0x193 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0483a4[] = {
    { init_slideLayers_8c047118, 0x194 },
    { init_slideLayers_8c04720a, 0x195 },
    { init_slideLayers_8c047212, 0x196 },
    { init_slideLayers_8c04720a, 0x197 },
    { init_slideLayers_8c04720e, 0x198 },
    { init_slideLayers_8c04720a, 0x199 },
    { init_slideLayers_8c04711c, 0x19a },
    { init_slideLayers_8c047142, 0x19b },
    { init_slideLayers_8c047146, 0x19c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0483f4[] = {
    { init_slideLayers_8c047118, 0x19d },
    { init_slideLayers_8c04711c, 0x19e },
    { init_slideLayers_8c047142, 0x19f },
    { init_slideLayers_8c047146, 0x1a0 },
    { init_slideLayers_8c04712e, 0x1a1 },
    { init_slideLayers_8c047146, 0x1a2 },
    { init_slideLayers_8c047142, 0x1a3 },
    { init_slideLayers_8c04711c, 0x1a4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide *init_wanganEvents_8c04843c[] = {
    init_eventSlides_8c04740c, init_eventSlides_8c047454, init_eventSlides_8c04747c, init_eventSlides_8c0474a4,
    init_eventSlides_8c0474bc, init_eventSlides_8c0474ec, init_eventSlides_8c047514, init_eventSlides_8c047544,
    init_eventSlides_8c047564, init_eventSlides_8c047574, init_eventSlides_8c0475bc, init_eventSlides_8c0475dc,
    init_eventSlides_8c04761c, init_eventSlides_8c04763c, init_eventSlides_8c04766c, init_eventSlides_8c04768c,
    init_eventSlides_8c04769c, init_eventSlides_8c0476ac, init_eventSlides_8c0476bc, init_eventSlides_8c0476cc,
    init_eventSlides_8c04771c, init_eventSlides_8c047744, init_eventSlides_8c04776c, init_eventSlides_8c0477ac,
    init_eventSlides_8c0477d4, init_eventSlides_8c0477fc, init_eventSlides_8c047824, init_eventSlides_8c047864,
    init_eventSlides_8c04788c, init_eventSlides_8c0478b4, init_eventSlides_8c0478e4, init_eventSlides_8c04791c,
    init_eventSlides_8c04794c, init_eventSlides_8c047974, init_eventSlides_8c047984, init_eventSlides_8c047994,
    init_eventSlides_8c0479bc, init_eventSlides_8c0479e4, init_eventSlides_8c047a0c, init_eventSlides_8c047a34,
    init_eventSlides_8c047a64, init_eventSlides_8c047a84, init_eventSlides_8c047aac, init_eventSlides_8c047acc,
    init_eventSlides_8c047aec, init_eventSlides_8c047b14, init_eventSlides_8c047b3c, init_eventSlides_8c047b5c,
    init_eventSlides_8c047ba4, init_eventSlides_8c047bcc, init_eventSlides_8c047c04, init_eventSlides_8c047c4c,
    init_eventSlides_8c047c6c, init_eventSlides_8c047c7c, init_eventSlides_8c047c8c, init_eventSlides_8c047c9c,
    init_eventSlides_8c047cac, init_eventSlides_8c047cbc, init_eventSlides_8c047ccc, init_eventSlides_8c047cdc,
    init_eventSlides_8c047cec, init_eventSlides_8c047d1c, init_eventSlides_8c047d4c, init_eventSlides_8c047d74,
    init_eventSlides_8c047dbc, init_eventSlides_8c047de4, init_eventSlides_8c047e1c, init_eventSlides_8c047e5c,
    init_eventSlides_8c047e84, init_eventSlides_8c047e9c, init_eventSlides_8c047ec4, init_eventSlides_8c047edc,
    init_eventSlides_8c047f14, init_eventSlides_8c047f54, init_eventSlides_8c047f7c, init_eventSlides_8c047f9c,
    init_eventSlides_8c047fcc, init_eventSlides_8c047fdc, init_eventSlides_8c048004, init_eventSlides_8c048024,
    init_eventSlides_8c04805c, init_eventSlides_8c048084, init_eventSlides_8c0480ac, init_eventSlides_8c0480d4,
    init_eventSlides_8c0480ec, init_eventSlides_8c0480fc, init_eventSlides_8c04812c, init_eventSlides_8c04814c,
    init_eventSlides_8c04815c, init_eventSlides_8c04819c, init_eventSlides_8c0481c4, init_eventSlides_8c048204,
    init_eventSlides_8c04825c, init_eventSlides_8c04828c, init_eventSlides_8c0482bc, init_eventSlides_8c048304,
    init_eventSlides_8c04834c, init_eventSlides_8c0483a4, init_eventSlides_8c0483f4, NULL,
};

STATIC EventSlide init_eventSlides_8c0485cc[] = {
    { init_slideLayers_8c046f54, 0x00 },
    { init_slideLayers_8c046f58, 0x01 },
    { init_slideLayers_8c046f5e, 0x02 },
    { init_slideLayers_8c046f54, 0x03 },
    { init_slideLayers_8c046f58, 0x04 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0485fc[] = {
    { init_slideLayers_8c046f5e, 0x05 },
    { init_slideLayers_8c046f54, 0x06 },
    { init_slideLayers_8c046f62, 0x07 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04861c[] = {
    { init_slideLayers_8c046f54, 0x08 },
    { init_slideLayers_8c046f5e, 0x09 },
    { init_slideLayers_8c046f62, 0x0a },
    { init_slideLayers_8c046f5e, 0x0b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048644[] = {
    { init_slideLayers_8c046f54, 0x0d },
    { init_slideLayers_8c046f5e, 0x0e },
    { init_slideLayers_8c046f62, 0x0f },
    { init_slideLayers_8c046f58, 0x10 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04866c[] = {
    { init_slideLayers_8c046f54, 0x11 },
    { init_slideLayers_8c046f5e, 0x12 },
    { init_slideLayers_8c046f62, 0x13 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04868c[] = {
    { init_slideLayers_8c046f54, 0x16 },
    { init_slideLayers_8c046f5e, 0x17 },
    { init_slideLayers_8c046f62, 0x18 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0486ac[] = {
    { init_slideLayers_8c046f5e, 0x19 },
    { init_slideLayers_8c046f54, 0x1a },
    { init_slideLayers_8c046f62, 0x1b },
    { init_slideLayers_8c046f5e, 0x1c },
    { init_slideLayers_8c046f58, 0x1d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0486dc[] = {
    { init_slideLayers_8c046f54, 0x1e },
    { init_slideLayers_8c046f62, 0x1f },
    { init_slideLayers_8c046f5e, 0x20 },
    { init_slideLayers_8c046f58, 0x21 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048704[] = {
    { init_slideLayers_8c046f66, 0x22 },
    { init_slideLayers_8c046f6e, 0x23 },
    { init_slideLayers_8c046f66, 0x24 },
    { init_slideLayers_8c046f6a, 0x25 },
    { init_slideLayers_8c046f66, 0x26 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048734[] = {
    { init_slideLayers_8c046f66, 0x27 },
    { init_slideLayers_8c046f6a, 0x28 },
    { init_slideLayers_8c046f66, 0x29 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048754[] = {
    { init_slideLayers_8c046f66, 0x2a },
    { init_slideLayers_8c046f6e, 0x2b },
    { init_slideLayers_8c046f74, 0x2c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048774[] = {
    { init_slideLayers_8c046f66, 0x2d },
    { init_slideLayers_8c046f6a, 0x2e },
    { init_slideLayers_8c046f66, 0x2f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048794[] = {
    { init_slideLayers_8c046f66, 0x30 },
    { init_slideLayers_8c046f6e, 0x31 },
    { init_slideLayers_8c046f74, 0x32 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0487b4[] = {
    { init_slideLayers_8c046f66, 0x33 },
    { init_slideLayers_8c046f6e, 0x34 },
    { init_slideLayers_8c046f74, 0x35 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0487d4[] = {
    { init_slideLayers_8c046f7c, 0x36 },
    { init_slideLayers_8c046f82, 0x37 },
    { init_slideLayers_8c046f78, 0x38 },
    { init_slideLayers_8c046fb6, 0x39 },
    { init_slideLayers_8c046f7c, 0x3a },
    { init_slideLayers_8c046f98, 0x3b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04880c[] = {
    { init_slideLayers_8c046f7c, 0x3c },
    { init_slideLayers_8c046fb6, 0x3d },
    { init_slideLayers_8c046f78, 0x3e },
    { init_slideLayers_8c046f98, 0x3f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048834[] = {
    { init_slideLayers_8c046fba, 0x40 },
    { init_slideLayers_8c046f78, 0x41 },
    { init_slideLayers_8c046fa8, 0x42 },
    { init_slideLayers_8c046f82, 0x43 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04885c[] = {
    { init_slideLayers_8c046f82, 0x44 },
    { init_slideLayers_8c046fa8, 0x45 },
    { init_slideLayers_8c046fb6, 0x46 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04887c[] = {
    { init_slideLayers_8c046f9e, 0x47 },
    { init_slideLayers_8c046fac, 0x48 },
    { init_slideLayers_8c04733c, 0x49 },
    { init_slideLayers_8c046fb0, 0x4a },
    { init_slideLayers_8c04733c, 0x4b },
    { init_slideLayers_8c046fb6, 0x4c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0488b4[] = {
    { init_slideLayers_8c046fa2, 0x4d },
    { init_slideLayers_8c046fb0, 0x4e },
    { init_slideLayers_8c046fb6, 0x4f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0488d4[] = {
    { init_slideLayers_8c046f9e, 0x50 },
    { init_slideLayers_8c046fb6, 0x51 },
    { init_slideLayers_8c046fb0, 0x52 },
    { init_slideLayers_8c047342, 0x53 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0488fc[] = {
    { init_slideLayers_8c046f9e, 0x54 },
    { init_slideLayers_8c046fac, 0x55 },
    { init_slideLayers_8c04733c, 0x56 },
    { init_slideLayers_8c046fb0, 0x57 },
    { init_slideLayers_8c047342, 0x58 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04892c[] = {
    { init_slideLayers_8c046fb0, 0x59 },
    { init_slideLayers_8c046fb6, 0x5a },
    { init_slideLayers_8c046f9e, 0x5b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04894c[] = {
    { init_slideLayers_8c046fd0, 0x5c },
    { init_slideLayers_8c046fbe, 0x5d },
    { init_slideLayers_8c046fc8, 0x5e },
    { init_slideLayers_8c046fc2, 0x5f },
    { init_slideLayers_8c046fc8, 0x60 },
    { init_slideLayers_8c046fc2, 0x61 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048984[] = {
    { init_slideLayers_8c046fd0, 0x62 },
    { init_slideLayers_8c046fbe, 0x63 },
    { init_slideLayers_8c046fc2, 0x64 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0489a4[] = {
    { init_slideLayers_8c046fd0, 0x65 },
    { init_slideLayers_8c046fbe, 0x66 },
    { init_slideLayers_8c046fc2, 0x67 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0489c4[] = {
    { init_slideLayers_8c046fd0, 0x68 },
    { init_slideLayers_8c046fbe, 0x69 },
    { init_slideLayers_8c046fc2, 0x6a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0489e4[] = {
    { init_slideLayers_8c046fc8, 0x6b },
    { init_slideLayers_8c046fc2, 0x6c },
    { init_slideLayers_8c046f8a, 0x6d },
    { init_slideLayers_8c046f90, 0x6e },
    { init_slideLayers_8c046fa8, 0x6f },
    { init_slideLayers_8c046f98, 0x70 },
    { init_slideLayers_8c046fa8, 0x71 },
    { init_slideLayers_8c046f98, 0x72 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048a2c[] = {
    { init_slideLayers_8c046fc8, 0x73 },
    { init_slideLayers_8c046fc2, 0x74 },
    { init_slideLayers_8c046f8a, 0x75 },
    { init_slideLayers_8c046f90, 0x76 },
    { init_slideLayers_8c046f78, 0x77 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048a5c[] = {
    { init_slideLayers_8c046fcc, 0x78 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048a6c[] = {
    { init_slideLayers_8c046fcc, 0x79 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048a7c[] = {
    { init_slideLayers_8c046f9e, 0x7a },
    { init_slideLayers_8c046fa2, 0x7b },
    { init_slideLayers_8c046fb6, 0x7c },
    { init_slideLayers_8c046fb0, 0x7d },
    { init_slideLayers_8c047342, 0x7e },
    { init_slideLayers_8c046fb0, 0x7f },
    { init_slideLayers_8c046fb6, 0x80 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048abc[] = {
    { init_slideLayers_8c046fa2, 0x81 },
    { init_slideLayers_8c046fb6, 0x82 },
    { init_slideLayers_8c046fb0, 0x83 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048adc[] = {
    { init_slideLayers_8c046fd4, 0x84 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048aec[] = {
    { init_slideLayers_8c046fd4, 0x85 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048afc[] = {
    { init_slideLayers_8c046fe6, 0x86 },
    { init_slideLayers_8c046fdc, 0x87 },
    { init_slideLayers_8c047006, 0x88 },
    { init_slideLayers_8c046ff4, 0x89 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048b24[] = {
    { init_slideLayers_8c046fe6, 0x8a },
    { init_slideLayers_8c047002, 0x8b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048b3c[] = {
    { init_slideLayers_8c046fd8, 0x8c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048b4c[] = {
    { init_slideLayers_8c046fd8, 0x8d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048b5c[] = {
    { init_slideLayers_8c046fe6, 0x8e },
    { init_slideLayers_8c046fdc, 0x8f },
    { init_slideLayers_8c047006, 0x90 },
    { init_slideLayers_8c046fee, 0x91 },
    { init_slideLayers_8c047006, 0x92 },
    { init_slideLayers_8c047002, 0x93 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048b94[] = {
    { init_slideLayers_8c046fe6, 0x94 },
    { init_slideLayers_8c047002, 0x95 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048bac[] = {
    { init_slideLayers_8c046fdc, 0x96 },
    { init_slideLayers_8c046fe0, 0x97 },
    { init_slideLayers_8c046fe6, 0x98 },
    { init_slideLayers_8c047002, 0x99 },
    { init_slideLayers_8c046fdc, 0x9a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048bdc[] = {
    { init_slideLayers_8c046fdc, 0x9b },
    { init_slideLayers_8c047002, 0x9c },
    { init_slideLayers_8c047006, 0x9d },
    { init_slideLayers_8c046ffc, 0x9e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048c04[] = {
    { init_slideLayers_8c047002, 0x9f },
    { init_slideLayers_8c046fdc, 0xa0 },
    { init_slideLayers_8c046fee, 0xa1 },
    { init_slideLayers_8c047006, 0xa2 },
    { init_slideLayers_8c046fdc, 0xa3 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048c34[] = {
    { init_slideLayers_8c046fdc, 0xa4 },
    { init_slideLayers_8c046fee, 0xa5 },
    { init_slideLayers_8c047006, 0xa6 },
    { init_slideLayers_8c046fdc, 0xa7 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048c5c[] = {
    { init_slideLayers_8c046ffc, 0xa8 },
    { init_slideLayers_8c046fdc, 0xa9 },
    { init_slideLayers_8c047002, 0xaa },
    { init_slideLayers_8c046fdc, 0xab },
    { init_slideLayers_8c046fee, 0xac },
    { init_slideLayers_8c047006, 0xad },
    { init_slideLayers_8c046fdc, 0xae },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048c9c[] = {
    { init_slideLayers_8c047006, 0xaf },
    { init_slideLayers_8c047002, 0xb0 },
    { init_slideLayers_8c046fdc, 0xb1 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048cbc[] = {
    { init_slideLayers_8c046fe0, 0xb2 },
    { init_slideLayers_8c047002, 0xb3 },
    { init_slideLayers_8c047006, 0xb4 },
    { init_slideLayers_8c046fdc, 0xb5 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048ce4[] = {
    { init_slideLayers_8c046fe0, 0xb6 },
    { init_slideLayers_8c046fdc, 0xb7 },
    { init_slideLayers_8c047006, 0xb8 },
    { init_slideLayers_8c046fe0, 0xb9 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048d0c[] = {
    { init_slideLayers_8c047010, 0xba },
    { init_slideLayers_8c047306, 0xbb },
    { init_slideLayers_8c047022, 0xbc },
    { init_slideLayers_8c047306, 0xbd },
    { init_slideLayers_8c04703e, 0xbe },
    { init_slideLayers_8c04703a, 0xbf },
    { init_slideLayers_8c04730c, 0xc0 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048d4c[] = {
    { init_slideLayers_8c047022, 0xc1 },
    { init_slideLayers_8c047036, 0xc2 },
    { init_slideLayers_8c04703a, 0xc3 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048d6c[] = {
    { init_slideLayers_8c047010, 0xc4 },
    { init_slideLayers_8c047036, 0xc5 },
    { init_slideLayers_8c047014, 0xc6 },
    { init_slideLayers_8c04703a, 0xc7 },
    { init_slideLayers_8c04730c, 0xc8 },
    { init_slideLayers_8c04703e, 0xc9 },
    { init_slideLayers_8c047036, 0xca },
    { init_slideLayers_8c047306, 0xcb },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048db4[] = {
    { init_slideLayers_8c047022, 0xcc },
    { init_slideLayers_8c047306, 0xcd },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048dcc[] = {
    { init_slideLayers_8c047010, 0xce },
    { init_slideLayers_8c047306, 0xcf },
    { init_slideLayers_8c04703a, 0xd0 },
    { init_slideLayers_8c04703e, 0xd1 },
    { init_slideLayers_8c047306, 0xd2 },
    { init_slideLayers_8c04703e, 0xd3 },
    { init_slideLayers_8c04731c, 0xd4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048e0c[] = {
    { init_slideLayers_8c047022, 0xd5 },
    { init_slideLayers_8c047314, 0xd6 },
    { init_slideLayers_8c04703e, 0xd7 },
    { init_slideLayers_8c047314, 0xd8 },
    { init_slideLayers_8c047010, 0xd9 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048e3c[] = {
    { init_slideLayers_8c047010, 0xda },
    { init_slideLayers_8c04703a, 0xdb },
    { init_slideLayers_8c047306, 0xdc },
    { init_slideLayers_8c047010, 0xdd },
    { init_slideLayers_8c04703e, 0xde },
    { init_slideLayers_8c047028, 0xdf },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048e74[] = {
    { init_slideLayers_8c047022, 0xe0 },
    { init_slideLayers_8c047036, 0xe1 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048e8c[] = {
    { init_slideLayers_8c047044, 0xe2 },
    { init_slideLayers_8c047050, 0xe3 },
    { init_slideLayers_8c047044, 0xe4 },
    { init_slideLayers_8c04704c, 0xe5 },
    { init_slideLayers_8c047048, 0xe6 },
    { init_slideLayers_8c04704c, 0xe7 },
    { init_slideLayers_8c047050, 0xe8 },
    { init_slideLayers_8c047044, 0xe9 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048ed4[] = {
    { init_slideLayers_8c047044, 0xea },
    { init_slideLayers_8c047048, 0xeb },
    { init_slideLayers_8c047050, 0xec },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048ef4[] = {
    { init_slideLayers_8c047044, 0xed },
    { init_slideLayers_8c04704c, 0xee },
    { init_slideLayers_8c047048, 0xef },
    { init_slideLayers_8c047044, 0xf0 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048f1c[] = {
    { init_slideLayers_8c047044, 0xf1 },
    { init_slideLayers_8c047048, 0xf2 },
    { init_slideLayers_8c047050, 0xf3 },
    { init_slideLayers_8c047044, 0xf4 },
    { init_slideLayers_8c04704c, 0xf5 },
    { init_slideLayers_8c047050, 0xf6 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048f54[] = {
    { init_slideLayers_8c04704c, 0xf7 },
    { init_slideLayers_8c047048, 0xf8 },
    { init_slideLayers_8c04704c, 0xf9 },
    { init_slideLayers_8c047044, 0xfa },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048f7c[] = {
    { init_slideLayers_8c047050, 0xfb },
    { init_slideLayers_8c047044, 0xfc },
    { init_slideLayers_8c04704c, 0xfd },
    { init_slideLayers_8c047044, 0xfe },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048fa4[] = {
    { init_slideLayers_8c047054, 0xff },
    { init_slideLayers_8c047062, 0x100 },
    { init_slideLayers_8c047054, 0x101 },
    { init_slideLayers_8c047062, 0x102 },
    { init_slideLayers_8c047054, 0x103 },
    { init_slideLayers_8c04705c, 0x104 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048fdc[] = {
    { init_slideLayers_8c04705c, 0x105 },
    { init_slideLayers_8c047062, 0x106 },
    { init_slideLayers_8c047054, 0x107 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048ffc[] = {
    { init_slideLayers_8c047066, 0x108 },
    { init_slideLayers_8c047070, 0x109 },
    { init_slideLayers_8c047066, 0x10a },
    { init_slideLayers_8c047070, 0x10b },
    { init_slideLayers_8c047066, 0x10c },
    { init_slideLayers_8c04706a, 0x10d },
    { init_slideLayers_8c047074, 0x10e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04903c[] = {
    { init_slideLayers_8c047066, 0x10f },
    { init_slideLayers_8c047070, 0x110 },
    { init_slideLayers_8c047066, 0x111 },
    { init_slideLayers_8c047070, 0x112 },
    { init_slideLayers_8c047066, 0x113 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04906c[] = {
    { init_slideLayers_8c04705c, 0x114 },
    { init_slideLayers_8c047054, 0x115 },
    { init_slideLayers_8c047058, 0x116 },
    { init_slideLayers_8c047062, 0x117 },
    { init_slideLayers_8c047054, 0x118 },
    { init_slideLayers_8c047058, 0x119 },
    { init_slideLayers_8c047054, 0x11a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0490ac[] = {
    { init_slideLayers_8c047054, 0x11b },
    { init_slideLayers_8c047062, 0x11c },
    { init_slideLayers_8c04705c, 0x11d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0490cc[] = {
    { init_slideLayers_8c04707e, 0x11e },
    { init_slideLayers_8c047074, 0x11f },
    { init_slideLayers_8c047086, 0x120 },
    { init_slideLayers_8c04709e, 0x121 },
    { init_slideLayers_8c04709a, 0x122 },
    { init_slideLayers_8c047074, 0x123 },
    { init_slideLayers_8c04709a, 0x124 },
    { init_slideLayers_8c0470a2, 0x125 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049114[] = {
    { init_slideLayers_8c047074, 0x126 },
    { init_slideLayers_8c04709a, 0x127 },
    { init_slideLayers_8c047094, 0x128 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049134[] = {
    { init_slideLayers_8c04707e, 0x129 },
    { init_slideLayers_8c047086, 0x12a },
    { init_slideLayers_8c04709e, 0x12b },
    { init_slideLayers_8c047074, 0x12c },
    { init_slideLayers_8c04709a, 0x12d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049164[] = {
    { init_slideLayers_8c047074, 0x12e },
    { init_slideLayers_8c04709e, 0x12f },
    { init_slideLayers_8c047086, 0x130 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049184[] = {
    { init_slideLayers_8c04707e, 0x131 },
    { init_slideLayers_8c047078, 0x132 },
    { init_slideLayers_8c04709a, 0x133 },
    { init_slideLayers_8c04709e, 0x134 },
    { init_slideLayers_8c047078, 0x135 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0491b4[] = {
    { init_slideLayers_8c047078, 0x136 },
    { init_slideLayers_8c047074, 0x137 },
    { init_slideLayers_8c04709e, 0x138 },
    { init_slideLayers_8c04709a, 0x139 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0491dc[] = {
    { init_slideLayers_8c047078, 0x13a },
    { init_slideLayers_8c0470a2, 0x13b },
    { init_slideLayers_8c047086, 0x13c },
    { init_slideLayers_8c047074, 0x13d },
    { init_slideLayers_8c04709a, 0x13e },
    { init_slideLayers_8c04709e, 0x13f },
    { init_slideLayers_8c047086, 0x140 },
    { init_slideLayers_8c0470a2, 0x141 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049224[] = {
    { init_slideLayers_8c047074, 0x142 },
    { init_slideLayers_8c0470a2, 0x143 },
    { init_slideLayers_8c047074, 0x144 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049244[] = {
    { init_slideLayers_8c0470ba, 0x145 },
    { init_slideLayers_8c0470c2, 0x146 },
    { init_slideLayers_8c0470ba, 0x147 },
    { init_slideLayers_8c0470be, 0x148 },
    { init_slideLayers_8c0470c2, 0x149 },
    { init_slideLayers_8c0470a8, 0x14a },
    { init_slideLayers_8c0470b6, 0x14b },
    { init_slideLayers_8c0470ac, 0x14c },
    { init_slideLayers_8c0470a8, 0x14d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049294[] = {
    { init_slideLayers_8c0470c6, 0x14e },
    { init_slideLayers_8c0470d4, 0x14f },
    { init_slideLayers_8c0470d8, 0x150 },
    { init_slideLayers_8c0470c6, 0x151 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0492bc[] = {
    { init_slideLayers_8c0470c6, 0x152 },
    { init_slideLayers_8c0470d8, 0x153 },
    { init_slideLayers_8c0470c6, 0x154 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0492dc[] = {
    { init_slideLayers_8c0470d0, 0x155 },
    { init_slideLayers_8c0470c6, 0x156 },
    { init_slideLayers_8c0470b0, 0x157 },
    { init_slideLayers_8c0470ca, 0x158 },
    { init_slideLayers_8c0470a8, 0x159 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04930c[] = {
    { init_slideLayers_8c0470dc, 0x15a },
    { init_slideLayers_8c0470e0, 0x15b },
    { init_slideLayers_8c0470e4, 0x15c },
    { init_slideLayers_8c0470dc, 0x15d },
    { init_slideLayers_8c0470e8, 0x15e },
    { init_slideLayers_8c0470ee, 0x15f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049344[] = {
    { init_slideLayers_8c0470f2, 0x160 },
    { init_slideLayers_8c047100, 0x161 },
    { init_slideLayers_8c0470f6, 0x162 },
    { init_slideLayers_8c0470f2, 0x163 },
    { init_slideLayers_8c0470fa, 0x164 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049374[] = {
    { init_slideLayers_8c0470f2, 0x165 },
    { init_slideLayers_8c0470fa, 0x166 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04938c[] = {
    { init_slideLayers_8c0470f2, 0x167 },
    { init_slideLayers_8c047100, 0x168 },
    { init_slideLayers_8c0470f6, 0x169 },
    { init_slideLayers_8c0470f2, 0x16a },
    { init_slideLayers_8c0470f6, 0x16b },
    { init_slideLayers_8c047100, 0x16c },
    { init_slideLayers_8c0470f2, 0x16d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0493cc[] = {
    { init_slideLayers_8c0470f2, 0x16e },
    { init_slideLayers_8c0470fa, 0x16f },
    { init_slideLayers_8c047100, 0x170 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0493ec[] = {
    { init_slideLayers_8c047104, 0x171 },
    { init_slideLayers_8c047108, 0x172 },
    { init_slideLayers_8c047104, 0x173 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04940c[] = {
    { init_slideLayers_8c047104, 0x174 },
    { init_slideLayers_8c047108, 0x175 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049424[] = {
    { init_slideLayers_8c04711c, 0x176 },
    { init_slideLayers_8c047110, 0x177 },
    { init_slideLayers_8c04710c, 0x178 },
    { init_slideLayers_8c047110, 0x179 },
    { init_slideLayers_8c04710c, 0x17a },
    { init_slideLayers_8c047114, 0x17b },
    { init_slideLayers_8c047110, 0x17c },
    { init_slideLayers_8c04710c, 0x17d },
    { init_slideLayers_8c047110, 0x17e },
    { init_slideLayers_8c047108, 0x17f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04947c[] = {
    { init_slideLayers_8c047110, 0x180 },
    { init_slideLayers_8c047108, 0x181 },
    { init_slideLayers_8c047110, 0x182 },
    { init_slideLayers_8c04710c, 0x183 },
    { init_slideLayers_8c047110, 0x184 },
    { init_slideLayers_8c047108, 0x185 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0494b4[] = {
    { init_slideLayers_8c047118, 0x186 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0494c4[] = {
    { init_slideLayers_8c047118, 0x187 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0494d4[] = {
    { init_slideLayers_8c04711c, 0x188 },
    { init_slideLayers_8c04712e, 0x189 },
    { init_slideLayers_8c047146, 0x18a },
    { init_slideLayers_8c047142, 0x18b },
    { init_slideLayers_8c04711c, 0x18c },
    { init_slideLayers_8c047146, 0x18d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04950c[] = {
    { init_slideLayers_8c04711c, 0x18e },
    { init_slideLayers_8c047146, 0x18f },
    { init_slideLayers_8c047142, 0x190 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04952c[] = {
    { init_slideLayers_8c04711c, 0x191 },
    { init_slideLayers_8c047142, 0x192 },
    { init_slideLayers_8c047146, 0x193 },
    { init_slideLayers_8c04712e, 0x194 },
    { init_slideLayers_8c047146, 0x195 },
    { init_slideLayers_8c047142, 0x196 },
    { init_slideLayers_8c04711c, 0x197 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04956c[] = {
    { init_slideLayers_8c04711c, 0x198 },
    { init_slideLayers_8c047142, 0x199 },
    { init_slideLayers_8c047146, 0x19a },
    { init_slideLayers_8c04711c, 0x19b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049594[] = {
    { init_slideLayers_8c04711c, 0x19c },
    { init_slideLayers_8c04714a, 0x19d },
    { init_slideLayers_8c047142, 0x19e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0495b4[] = {
    { init_slideLayers_8c046fd0, 0x19f },
    { init_slideLayers_8c046fbe, 0x1a0 },
    { init_slideLayers_8c046fc2, 0x1a1 },
    { init_slideLayers_8c046fc8, 0x1a2 },
    { init_slideLayers_8c046fc2, 0x1a3 },
    { init_slideLayers_8c046f8a, 0x1a4 },
    { init_slideLayers_8c046f90, 0x1a5 },
    { init_slideLayers_8c046fa8, 0x1a6 },
    { init_slideLayers_8c046f98, 0x1a7 },
    { init_slideLayers_8c046fa8, 0x1a8 },
    { init_slideLayers_8c046f98, 0x1a9 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049614[] = {
    { init_slideLayers_8c046fd0, 0x1aa },
    { init_slideLayers_8c046fbe, 0x1ab },
    { init_slideLayers_8c046fc2, 0x1ac },
    { init_slideLayers_8c046fc8, 0x1ad },
    { init_slideLayers_8c046fc2, 0x1ae },
    { init_slideLayers_8c046f8a, 0x1af },
    { init_slideLayers_8c046f90, 0x1b0 },
    { init_slideLayers_8c046fa8, 0x1b1 },
    { init_slideLayers_8c046f98, 0x1b2 },
    { init_slideLayers_8c046fa8, 0x1b3 },
    { init_slideLayers_8c046f98, 0x1b4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049674[] = {
    { init_slideLayers_8c046fd0, 0x1b5 },
    { init_slideLayers_8c046fbe, 0x1b6 },
    { init_slideLayers_8c046fc2, 0x1b7 },
    { init_slideLayers_8c046fc8, 0x1b8 },
    { init_slideLayers_8c046fc2, 0x1b9 },
    { init_slideLayers_8c046f8a, 0x1ba },
    { init_slideLayers_8c046f90, 0x1bb },
    { init_slideLayers_8c046f78, 0x1bc },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0496bc[] = {
    { init_slideLayers_8c046fcc, 0x1bd },
    { init_slideLayers_8c046f9e, 0x1be },
    { init_slideLayers_8c046fa2, 0x1bf },
    { init_slideLayers_8c046fb6, 0x1c0 },
    { init_slideLayers_8c046fb0, 0x1c1 },
    { init_slideLayers_8c047342, 0x1c2 },
    { init_slideLayers_8c046fb0, 0x1c3 },
    { init_slideLayers_8c046fb6, 0x1c4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049704[] = {
    { init_slideLayers_8c046fcc, 0x1c5 },
    { init_slideLayers_8c046f9e, 0x1c6 },
    { init_slideLayers_8c046fa2, 0x1c7 },
    { init_slideLayers_8c046fb6, 0x1c8 },
    { init_slideLayers_8c046fb0, 0x1c9 },
    { init_slideLayers_8c047342, 0x1ca },
    { init_slideLayers_8c046fb0, 0x1cb },
    { init_slideLayers_8c046fb6, 0x1cc },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04974c[] = {
    { init_slideLayers_8c046fcc, 0x1cd },
    { init_slideLayers_8c046fa2, 0x1ce },
    { init_slideLayers_8c046fb6, 0x1cf },
    { init_slideLayers_8c046fb0, 0x1d0 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049774[] = {
    { init_slideLayers_8c046fd4, 0x1d1 },
    { init_slideLayers_8c046fe6, 0x1d2 },
    { init_slideLayers_8c046fdc, 0x1d3 },
    { init_slideLayers_8c047006, 0x1d4 },
    { init_slideLayers_8c046ff4, 0x1d5 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0497a4[] = {
    { init_slideLayers_8c046fd4, 0x1d6 },
    { init_slideLayers_8c046fe6, 0x1d7 },
    { init_slideLayers_8c047002, 0x1d8 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0497c4[] = {
    { init_slideLayers_8c046fd8, 0x1d9 },
    { init_slideLayers_8c046fe6, 0x1da },
    { init_slideLayers_8c046fdc, 0x1db },
    { init_slideLayers_8c047006, 0x1dc },
    { init_slideLayers_8c046fee, 0x1dd },
    { init_slideLayers_8c047006, 0x1de },
    { init_slideLayers_8c047002, 0x1df },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049804[] = {
    { init_slideLayers_8c046fd8, 0x1e0 },
    { init_slideLayers_8c046fe6, 0x1e1 },
    { init_slideLayers_8c047002, 0x1e2 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049824[] = {
    { init_slideLayers_8c047066, 0x1e3 },
    { init_slideLayers_8c047070, 0x1e4 },
    { init_slideLayers_8c047066, 0x1e5 },
    { init_slideLayers_8c047070, 0x1e6 },
    { init_slideLayers_8c047066, 0x1e7 },
    { init_slideLayers_8c04706a, 0x1e8 },
    { init_slideLayers_8c047074, 0x1e9 },
    { init_slideLayers_8c047054, 0x1ea },
    { init_slideLayers_8c047062, 0x1eb },
    { init_slideLayers_8c047054, 0x1ec },
    { init_slideLayers_8c047062, 0x1ed },
    { init_slideLayers_8c047054, 0x1ee },
    { init_slideLayers_8c04705c, 0x1ef },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049894[] = {
    { init_slideLayers_8c047066, 0x1f0 },
    { init_slideLayers_8c047070, 0x1f1 },
    { init_slideLayers_8c047066, 0x1f2 },
    { init_slideLayers_8c047070, 0x1f3 },
    { init_slideLayers_8c047066, 0x1f4 },
    { init_slideLayers_8c04705c, 0x1f5 },
    { init_slideLayers_8c047058, 0x1f6 },
    { init_slideLayers_8c047054, 0x1f7 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0498dc[] = {
    { init_slideLayers_8c0470c6, 0x1f8 },
    { init_slideLayers_8c0470d4, 0x1f9 },
    { init_slideLayers_8c0470d0, 0x1fa },
    { init_slideLayers_8c0470d8, 0x1fb },
    { init_slideLayers_8c0470c6, 0x1fc },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04990c[] = {
    { init_slideLayers_8c0470c6, 0x1fd },
    { init_slideLayers_8c0470d4, 0x1fe },
    { init_slideLayers_8c0470d8, 0x1ff },
    { init_slideLayers_8c0470c6, 0x200 },
    { init_slideLayers_8c0470d0, 0x201 },
    { init_slideLayers_8c0470c6, 0x202 },
    { init_slideLayers_8c0470b0, 0x203 },
    { init_slideLayers_8c0470ca, 0x204 },
    { init_slideLayers_8c0470a8, 0x205 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04995c[] = {
    { init_slideLayers_8c0470c6, 0x206 },
    { init_slideLayers_8c0470d4, 0x207 },
    { init_slideLayers_8c0470d0, 0x208 },
    { init_slideLayers_8c0470d8, 0x209 },
    { init_slideLayers_8c0470c6, 0x20a },
    { init_slideLayers_8c0470d0, 0x20b },
    { init_slideLayers_8c0470c6, 0x20c },
    { init_slideLayers_8c0470b0, 0x20d },
    { init_slideLayers_8c0470ca, 0x20e },
    { init_slideLayers_8c0470a8, 0x20f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0499b4[] = {
    { init_slideLayers_8c047104, 0x210 },
    { init_slideLayers_8c047108, 0x211 },
    { init_slideLayers_8c047104, 0x212 },
    { init_slideLayers_8c04711c, 0x213 },
    { init_slideLayers_8c047110, 0x214 },
    { init_slideLayers_8c04710c, 0x215 },
    { init_slideLayers_8c047110, 0x216 },
    { init_slideLayers_8c04710c, 0x217 },
    { init_slideLayers_8c047114, 0x218 },
    { init_slideLayers_8c047110, 0x219 },
    { init_slideLayers_8c04710c, 0x21a },
    { init_slideLayers_8c047110, 0x21b },
    { init_slideLayers_8c047108, 0x21c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049a24[] = {
    { init_slideLayers_8c047104, 0x21d },
    { init_slideLayers_8c047108, 0x21e },
    { init_slideLayers_8c047110, 0x21f },
    { init_slideLayers_8c047108, 0x220 },
    { init_slideLayers_8c047110, 0x221 },
    { init_slideLayers_8c04710c, 0x222 },
    { init_slideLayers_8c047110, 0x223 },
    { init_slideLayers_8c047108, 0x224 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide *init_shinjukuEvents_8c049a6c[] = {
    init_eventSlides_8c0485cc, init_eventSlides_8c0485fc, init_eventSlides_8c04861c, init_eventSlides_8c048644,
    init_eventSlides_8c04866c, init_eventSlides_8c04868c, init_eventSlides_8c0486ac, init_eventSlides_8c0486dc,
    init_eventSlides_8c048704, init_eventSlides_8c048734, init_eventSlides_8c048754, init_eventSlides_8c048774,
    init_eventSlides_8c048794, init_eventSlides_8c0487b4, init_eventSlides_8c0487d4, init_eventSlides_8c04880c,
    init_eventSlides_8c048834, init_eventSlides_8c04885c, init_eventSlides_8c04887c, init_eventSlides_8c0488b4,
    init_eventSlides_8c0488d4, init_eventSlides_8c0488fc, init_eventSlides_8c04892c, init_eventSlides_8c04894c,
    init_eventSlides_8c048984, init_eventSlides_8c0489a4, init_eventSlides_8c0489c4, init_eventSlides_8c0489e4,
    init_eventSlides_8c048a2c, init_eventSlides_8c048a5c, init_eventSlides_8c048a6c, init_eventSlides_8c048a7c,
    init_eventSlides_8c048abc, init_eventSlides_8c048adc, init_eventSlides_8c048aec, init_eventSlides_8c048afc,
    init_eventSlides_8c048b24, init_eventSlides_8c048b3c, init_eventSlides_8c048b4c, init_eventSlides_8c048b5c,
    init_eventSlides_8c048b94, init_eventSlides_8c048bac, init_eventSlides_8c048bdc, init_eventSlides_8c048c04,
    init_eventSlides_8c048c34, init_eventSlides_8c048c5c, init_eventSlides_8c048c9c, init_eventSlides_8c048cbc,
    init_eventSlides_8c048ce4, init_eventSlides_8c048d0c, init_eventSlides_8c048d4c, init_eventSlides_8c048d6c,
    init_eventSlides_8c048db4, init_eventSlides_8c048dcc, init_eventSlides_8c048e0c, init_eventSlides_8c048e3c,
    init_eventSlides_8c048e74, init_eventSlides_8c048e8c, init_eventSlides_8c048ed4, init_eventSlides_8c048ef4,
    init_eventSlides_8c048f1c, init_eventSlides_8c048f54, init_eventSlides_8c048f7c, init_eventSlides_8c048fa4,
    init_eventSlides_8c048fdc, init_eventSlides_8c048ffc, init_eventSlides_8c04903c, init_eventSlides_8c04906c,
    init_eventSlides_8c0490ac, init_eventSlides_8c0490cc, init_eventSlides_8c049114, init_eventSlides_8c049134,
    init_eventSlides_8c049164, init_eventSlides_8c049184, init_eventSlides_8c0491b4, init_eventSlides_8c0491dc,
    init_eventSlides_8c049224, init_eventSlides_8c049244, init_eventSlides_8c049294, init_eventSlides_8c0492bc,
    init_eventSlides_8c0492dc, init_eventSlides_8c04930c, init_eventSlides_8c049344, init_eventSlides_8c049374,
    init_eventSlides_8c04938c, init_eventSlides_8c0493cc, init_eventSlides_8c0493ec, init_eventSlides_8c04940c,
    init_eventSlides_8c049424, init_eventSlides_8c04947c, init_eventSlides_8c0494b4, init_eventSlides_8c0494c4,
    init_eventSlides_8c0494d4, init_eventSlides_8c04950c, init_eventSlides_8c04952c, init_eventSlides_8c04956c,
    init_eventSlides_8c049594, init_eventSlides_8c0495b4, init_eventSlides_8c049614, init_eventSlides_8c049674,
    init_eventSlides_8c0496bc, init_eventSlides_8c049704, init_eventSlides_8c04974c, init_eventSlides_8c049774,
    init_eventSlides_8c0497a4, init_eventSlides_8c0497c4, init_eventSlides_8c049804, init_eventSlides_8c049824,
    init_eventSlides_8c049894, init_eventSlides_8c0498dc, init_eventSlides_8c04990c, init_eventSlides_8c04995c,
    init_eventSlides_8c0499b4, init_eventSlides_8c049a24, NULL,
};

STATIC EventSlide init_eventSlides_8c049c38[] = {
    { init_slideLayers_8c04721a, 0x00 },
    { init_slideLayers_8c04722c, 0x01 },
    { init_slideLayers_8c04721e, 0x02 },
    { init_slideLayers_8c047228, 0x03 },
    { init_slideLayers_8c04721a, 0x04 },
    { init_slideLayers_8c04722c, 0x05 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049c70[] = {
    { init_slideLayers_8c04722c, 0x06 },
    { init_slideLayers_8c04721e, 0x07 },
    { init_slideLayers_8c04721a, 0x08 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049c90[] = {
    { init_slideLayers_8c04734a, 0x09 },
    { init_slideLayers_8c04721a, 0x0a },
    { init_slideLayers_8c04721e, 0x0b },
    { init_slideLayers_8c04721a, 0x0c },
    { init_slideLayers_8c04722c, 0x0d },
    { init_slideLayers_8c047228, 0x0e },
    { init_slideLayers_8c04722c, 0x0f },
    { init_slideLayers_8c047222, 0x10 },
    { init_slideLayers_8c04721a, 0x11 },
    { init_slideLayers_8c04722c, 0x12 },
    { init_slideLayers_8c047222, 0x13 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049cf0[] = {
    { init_slideLayers_8c04721a, 0x14 },
    { init_slideLayers_8c04722c, 0x15 },
    { init_slideLayers_8c04721e, 0x16 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049d10[] = {
    { init_slideLayers_8c04721a, 0x17 },
    { init_slideLayers_8c047228, 0x18 },
    { init_slideLayers_8c04722c, 0x19 },
    { init_slideLayers_8c04721e, 0x1a },
    { init_slideLayers_8c04721a, 0x1b },
    { init_slideLayers_8c04722c, 0x1c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049d48[] = {
    { init_slideLayers_8c04722c, 0x1d },
    { init_slideLayers_8c04721e, 0x1e },
    { init_slideLayers_8c04721a, 0x1f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049d68[] = {
    { init_slideLayers_8c047270, 0x20 },
    { init_slideLayers_8c04727a, 0x21 },
    { init_slideLayers_8c047270, 0x22 },
    { init_slideLayers_8c04727a, 0x23 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049d90[] = {
    { init_slideLayers_8c04721a, 0x24 },
    { init_slideLayers_8c047228, 0x25 },
    { init_slideLayers_8c04721e, 0x26 },
    { init_slideLayers_8c04721a, 0x27 },
    { init_slideLayers_8c04722c, 0x28 },
    { init_slideLayers_8c047228, 0x29 },
    { init_slideLayers_8c047222, 0x2a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049dd0[] = {
    { init_slideLayers_8c047230, 0x2b },
    { init_slideLayers_8c047234, 0x2c },
    { init_slideLayers_8c04723a, 0x2d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049df0[] = {
    { init_slideLayers_8c047230, 0x2e },
    { init_slideLayers_8c04723a, 0x2f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049e08[] = {
    { init_slideLayers_8c047230, 0x30 },
    { init_slideLayers_8c047234, 0x31 },
    { init_slideLayers_8c04723a, 0x32 },
    { init_slideLayers_8c047234, 0x33 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049e30[] = {
    { init_slideLayers_8c047230, 0x34 },
    { init_slideLayers_8c047234, 0x35 },
    { init_slideLayers_8c04723a, 0x36 },
    { init_slideLayers_8c047230, 0x37 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049e58[] = {
    { init_slideLayers_8c04723a, 0x38 },
    { init_slideLayers_8c047234, 0x39 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049e70[] = {
    { init_slideLayers_8c04723e, 0x3a },
    { init_slideLayers_8c047264, 0x3b },
    { init_slideLayers_8c04723e, 0x3c },
    { init_slideLayers_8c04725a, 0x3d },
    { init_slideLayers_8c047268, 0x3e },
    { init_slideLayers_8c04725a, 0x3f },
    { init_slideLayers_8c04723e, 0x40 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049eb0[] = {
    { init_slideLayers_8c04723e, 0x41 },
    { init_slideLayers_8c047264, 0x42 },
    { init_slideLayers_8c04725a, 0x43 },
    { init_slideLayers_8c047268, 0x44 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049ed8[] = {
    { init_slideLayers_8c04723e, 0x45 },
    { init_slideLayers_8c04725a, 0x46 },
    { init_slideLayers_8c047268, 0x47 },
    { init_slideLayers_8c04725e, 0x48 },
    { init_slideLayers_8c04723e, 0x49 },
    { init_slideLayers_8c047264, 0x4a },
    { init_slideLayers_8c047242, 0x4b },
    { init_slideLayers_8c047248, 0x4c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049f20[] = {
    { init_slideLayers_8c04726c, 0x4d },
    { init_slideLayers_8c04727a, 0x4e },
    { init_slideLayers_8c047276, 0x4f },
    { init_slideLayers_8c04726c, 0x50 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049f48[] = {
    { init_slideLayers_8c04726c, 0x51 },
    { init_slideLayers_8c04727a, 0x52 },
    { init_slideLayers_8c04726c, 0x53 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049f68[] = {
    { init_slideLayers_8c04726c, 0x54 },
    { init_slideLayers_8c04727a, 0x55 },
    { init_slideLayers_8c047276, 0x56 },
    { init_slideLayers_8c047270, 0x57 },
    { init_slideLayers_8c04727a, 0x58 },
    { init_slideLayers_8c04726c, 0x59 },
    { init_slideLayers_8c04727a, 0x5a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049fa8[] = {
    { init_slideLayers_8c04726c, 0x5b },
    { init_slideLayers_8c04727a, 0x5c },
    { init_slideLayers_8c047276, 0x5d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049fc8[] = {
    { init_slideLayers_8c04726c, 0x5e },
    { init_slideLayers_8c047276, 0x5f },
    { init_slideLayers_8c04727a, 0x60 },
    { init_slideLayers_8c04726c, 0x61 },
    { init_slideLayers_8c04727a, 0x62 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049ff8[] = {
    { init_slideLayers_8c04726c, 0x63 },
    { init_slideLayers_8c047276, 0x64 },
    { init_slideLayers_8c04727a, 0x65 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a018[] = {
    { init_slideLayers_8c046fd4, 0x66 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a028[] = {
    { init_slideLayers_8c046fd4, 0x67 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a038[] = {
    { init_slideLayers_8c046fe6, 0x68 },
    { init_slideLayers_8c046fdc, 0x69 },
    { init_slideLayers_8c047006, 0x6a },
    { init_slideLayers_8c047002, 0x6b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a060[] = {
    { init_slideLayers_8c046fdc, 0x6c },
    { init_slideLayers_8c047006, 0x6d },
    { init_slideLayers_8c047002, 0x6e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a080[] = {
    { init_slideLayers_8c047002, 0x6f },
    { init_slideLayers_8c047006, 0x70 },
    { init_slideLayers_8c046fee, 0x71 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a0a0[] = {
    { init_slideLayers_8c047006, 0x72 },
    { init_slideLayers_8c046fdc, 0x73 },
    { init_slideLayers_8c047006, 0x74 },
    { init_slideLayers_8c046ffc, 0x75 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a0c8[] = {
    { init_slideLayers_8c046ffc, 0x76 },
    { init_slideLayers_8c046fdc, 0x77 },
    { init_slideLayers_8c047002, 0x78 },
    { init_slideLayers_8c047006, 0x79 },
    { init_slideLayers_8c046fee, 0x7a },
    { init_slideLayers_8c047006, 0x7b },
    { init_slideLayers_8c046fdc, 0x7c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a108[] = {
    { init_slideLayers_8c04727e, 0x7d },
    { init_slideLayers_8c047282, 0x7e },
    { init_slideLayers_8c04727e, 0x7f },
    { init_slideLayers_8c047290, 0x80 },
    { init_slideLayers_8c04728c, 0x81 },
    { init_slideLayers_8c04727e, 0x82 },
    { init_slideLayers_8c04728c, 0x83 },
    { init_slideLayers_8c047298, 0x84 },
    { init_slideLayers_8c047286, 0x85 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a158[] = {
    { init_slideLayers_8c04727e, 0x86 },
    { init_slideLayers_8c047282, 0x87 },
    { init_slideLayers_8c047298, 0x88 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a178[] = {
    { init_slideLayers_8c04727e, 0x89 },
    { init_slideLayers_8c04728c, 0x8a },
    { init_slideLayers_8c047290, 0x8b },
    { init_slideLayers_8c047298, 0x8c },
    { init_slideLayers_8c04727e, 0x8d },
    { init_slideLayers_8c047298, 0x8e },
    { init_slideLayers_8c047286, 0x8f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a1b8[] = {
    { init_slideLayers_8c04727e, 0x90 },
    { init_slideLayers_8c047282, 0x91 },
    { init_slideLayers_8c047298, 0x92 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a1d8[] = {
    { init_slideLayers_8c047286, 0x93 },
    { init_slideLayers_8c04728c, 0x94 },
    { init_slideLayers_8c04727e, 0x95 },
    { init_slideLayers_8c047294, 0x96 },
    { init_slideLayers_8c04727e, 0x97 },
    { init_slideLayers_8c047294, 0x98 },
    { init_slideLayers_8c047282, 0x99 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a218[] = {
    { init_slideLayers_8c04727e, 0x9a },
    { init_slideLayers_8c047294, 0x9b },
    { init_slideLayers_8c04728c, 0x9c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a238[] = {
    { init_slideLayers_8c04727e, 0x9d },
    { init_slideLayers_8c047290, 0x9e },
    { init_slideLayers_8c047282, 0x9f },
    { init_slideLayers_8c047294, 0xa0 },
    { init_slideLayers_8c04728c, 0xa1 },
    { init_slideLayers_8c04727e, 0xa2 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a270[] = {
    { init_slideLayers_8c04727e, 0xa3 },
    { init_slideLayers_8c047290, 0xa4 },
    { init_slideLayers_8c047294, 0xa5 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a290[] = {
    { init_slideLayers_8c04729e, 0xa6 },
    { init_slideLayers_8c0472a2, 0xa7 },
    { init_slideLayers_8c0472ac, 0xa8 },
    { init_slideLayers_8c04729e, 0xa9 },
    { init_slideLayers_8c0472a6, 0xaa },
    { init_slideLayers_8c04729e, 0xab },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a2c8[] = {
    { init_slideLayers_8c04729e, 0xac },
    { init_slideLayers_8c0472ac, 0xad },
    { init_slideLayers_8c0472a2, 0xae },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a2e8[] = {
    { init_slideLayers_8c04729e, 0xaf },
    { init_slideLayers_8c0472a2, 0xb0 },
    { init_slideLayers_8c0472ac, 0xb1 },
    { init_slideLayers_8c04729e, 0xb2 },
    { init_slideLayers_8c0472a6, 0xb3 },
    { init_slideLayers_8c0472ac, 0xb4 },
    { init_slideLayers_8c04729e, 0xb5 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a328[] = {
    { init_slideLayers_8c04729e, 0xb6 },
    { init_slideLayers_8c0472a2, 0xb7 },
    { init_slideLayers_8c0472ac, 0xb8 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a348[] = {
    { init_slideLayers_8c04729e, 0xb9 },
    { init_slideLayers_8c0472ac, 0xba },
    { init_slideLayers_8c0472a6, 0xbb },
    { init_slideLayers_8c04729e, 0xbc },
    { init_slideLayers_8c0472ac, 0xbd },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a378[] = {
    { init_slideLayers_8c0472a6, 0xbe },
    { init_slideLayers_8c0472ac, 0xbf },
    { init_slideLayers_8c04729e, 0xc0 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a398[] = {
    { init_slideLayers_8c0472b0, 0xc1 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a3a8[] = {
    { init_slideLayers_8c0472b0, 0xc2 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a3b8[] = {
    { init_slideLayers_8c0472b0, 0xc3 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a3c8[] = {
    { init_slideLayers_8c0472b0, 0xc4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a3d8[] = {
    { init_slideLayers_8c0472b0, 0xc5 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a3e8[] = {
    { init_slideLayers_8c0472b0, 0xc6 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a3f8[] = {
    { init_slideLayers_8c0472b4, 0xc7 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a408[] = {
    { init_slideLayers_8c0472b4, 0xc8 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a418[] = {
    { init_slideLayers_8c0472b4, 0xc9 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a428[] = {
    { init_slideLayers_8c0472b4, 0xca },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a438[] = {
    { init_slideLayers_8c0472b8, 0xcb },
    { init_slideLayers_8c0472c2, 0xcc },
    { init_slideLayers_8c0472c6, 0xcd },
    { init_slideLayers_8c0472b8, 0xce },
    { init_slideLayers_8c0472ca, 0xcf },
    { init_slideLayers_8c0472bc, 0xd0 },
    { init_slideLayers_8c0472ca, 0xd1 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a478[] = {
    { init_slideLayers_8c0472b8, 0xd2 },
    { init_slideLayers_8c0472c6, 0xd3 },
    { init_slideLayers_8c0472c2, 0xd4 },
    { init_slideLayers_8c0472c6, 0xd5 },
    { init_slideLayers_8c0472b8, 0xd6 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a4a8[] = {
    { init_slideLayers_8c0472d0, 0xd7 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a4b8[] = {
    { init_slideLayers_8c0472d0, 0xd8 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a4c8[] = {
    { init_slideLayers_8c0472b8, 0xd9 },
    { init_slideLayers_8c0472c2, 0xda },
    { init_slideLayers_8c0472c6, 0xdb },
    { init_slideLayers_8c0472b8, 0xdc },
    { init_slideLayers_8c0472c6, 0xdd },
    { init_slideLayers_8c0472b8, 0xde },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a500[] = {
    { init_slideLayers_8c0472b8, 0xdf },
    { init_slideLayers_8c0472c6, 0xe0 },
    { init_slideLayers_8c0472b8, 0xe1 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a520[] = {
    { init_slideLayers_8c0472d0, 0xe2 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a530[] = {
    { init_slideLayers_8c0472d0, 0xe3 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a540[] = {
    { init_slideLayers_8c047074, 0xe4 },
    { init_slideLayers_8c04709e, 0xe5 },
    { init_slideLayers_8c047074, 0xe6 },
    { init_slideLayers_8c047086, 0xe7 },
    { init_slideLayers_8c0470a2, 0xe8 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a570[] = {
    { init_slideLayers_8c04707e, 0xe9 },
    { init_slideLayers_8c047078, 0xea },
    { init_slideLayers_8c04709a, 0xeb },
    { init_slideLayers_8c047074, 0xec },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a598[] = {
    { init_slideLayers_8c047074, 0xed },
    { init_slideLayers_8c04709a, 0xee },
    { init_slideLayers_8c047074, 0xef },
    { init_slideLayers_8c04709a, 0xf0 },
    { init_slideLayers_8c0470a2, 0xf1 },
    { init_slideLayers_8c04709a, 0xf2 },
    { init_slideLayers_8c04709e, 0xf3 },
    { init_slideLayers_8c047086, 0xf4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a5e0[] = {
    { init_slideLayers_8c047086, 0xf5 },
    { init_slideLayers_8c04734e, 0xf6 },
    { init_slideLayers_8c047086, 0xf7 },
    { init_slideLayers_8c04709a, 0xf8 },
    { init_slideLayers_8c04709e, 0xf9 },
    { init_slideLayers_8c047074, 0xfa },
    { init_slideLayers_8c047086, 0xfb },
    { init_slideLayers_8c04709e, 0xfc },
    { init_slideLayers_8c047074, 0xfd },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a630[] = {
    { init_slideLayers_8c047094, 0xfe },
    { init_slideLayers_8c047074, 0xff },
    { init_slideLayers_8c04709e, 0x100 },
    { init_slideLayers_8c04709a, 0x101 },
    { init_slideLayers_8c047074, 0x102 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a660[] = {
    { init_slideLayers_8c0472d4, 0x103 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a670[] = {
    { init_slideLayers_8c0472d4, 0x104 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a680[] = {
    { init_slideLayers_8c0472d4, 0x105 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a690[] = {
    { init_slideLayers_8c0472d4, 0x106 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a6a0[] = {
    { init_slideLayers_8c0472d4, 0x107 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a6b0[] = {
    { init_slideLayers_8c0472d4, 0x108 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a6c0[] = {
    { init_slideLayers_8c0472d4, 0x109 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a6d0[] = {
    { init_slideLayers_8c0472d4, 0x10a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a6e0[] = {
    { init_slideLayers_8c0472d4, 0x10b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a6f0[] = {
    { init_slideLayers_8c0472d4, 0x10c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a700[] = {
    { init_slideLayers_8c0472d4, 0x10d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a710[] = {
    { init_slideLayers_8c0472d4, 0x10e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a720[] = {
    { init_slideLayers_8c0472d8, 0x10f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a730[] = {
    { init_slideLayers_8c0472d8, 0x110 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a740[] = {
    { init_slideLayers_8c0472dc, 0x111 },
    { init_slideLayers_8c0472e4, 0x112 },
    { init_slideLayers_8c0472e0, 0x113 },
    { init_slideLayers_8c0472e8, 0x114 },
    { init_slideLayers_8c0472dc, 0x115 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a770[] = {
    { init_slideLayers_8c0472ee, 0x116 },
    { init_slideLayers_8c0472fc, 0x117 },
    { init_slideLayers_8c0472ee, 0x118 },
    { init_slideLayers_8c0472f2, 0x119 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a798[] = {
    { init_slideLayers_8c0472ee, 0x11a },
    { init_slideLayers_8c0472f8, 0x11b },
    { init_slideLayers_8c0472fc, 0x11c },
    { init_slideLayers_8c0472f2, 0x11d },
    { init_slideLayers_8c0472f8, 0x11e },
    { init_slideLayers_8c0472ee, 0x11f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a7d0[] = {
    { init_slideLayers_8c04711c, 0x120 },
    { init_slideLayers_8c04711c, 0x121 },
    { init_slideLayers_8c047146, 0x122 },
    { init_slideLayers_8c04711c, 0x123 },
    { init_slideLayers_8c047142, 0x124 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a800[] = {
    { init_slideLayers_8c047118, 0x125 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a810[] = {
    { init_slideLayers_8c047118, 0x126 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a820[] = {
    { init_slideLayers_8c04711c, 0x127 },
    { init_slideLayers_8c04711c, 0x128 },
    { init_slideLayers_8c047142, 0x129 },
    { init_slideLayers_8c047146, 0x12a },
    { init_slideLayers_8c04711c, 0x12b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a850[] = {
    { init_slideLayers_8c04721a, 0x12c },
    { init_slideLayers_8c047228, 0x12d },
    { init_slideLayers_8c04722c, 0x12e },
    { init_slideLayers_8c04721e, 0x12f },
    { init_slideLayers_8c04721a, 0x130 },
    { init_slideLayers_8c04722c, 0x131 },
    { init_slideLayers_8c047270, 0x132 },
    { init_slideLayers_8c04727a, 0x133 },
    { init_slideLayers_8c047270, 0x134 },
    { init_slideLayers_8c04727a, 0x135 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a8a8[] = {
    { init_slideLayers_8c04722c, 0x136 },
    { init_slideLayers_8c04721e, 0x137 },
    { init_slideLayers_8c04721a, 0x138 },
    { init_slideLayers_8c047270, 0x139 },
    { init_slideLayers_8c04727a, 0x13a },
    { init_slideLayers_8c047270, 0x13b },
    { init_slideLayers_8c04727a, 0x13c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a8e8[] = {
    { init_slideLayers_8c0472b4, 0x13d },
    { init_slideLayers_8c0472b8, 0x13e },
    { init_slideLayers_8c0472c2, 0x13f },
    { init_slideLayers_8c0472c6, 0x140 },
    { init_slideLayers_8c0472b8, 0x141 },
    { init_slideLayers_8c0472ca, 0x142 },
    { init_slideLayers_8c0472bc, 0x143 },
    { init_slideLayers_8c0472ca, 0x144 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a930[] = {
    { init_slideLayers_8c0472b4, 0x145 },
    { init_slideLayers_8c0472b8, 0x146 },
    { init_slideLayers_8c0472c6, 0x147 },
    { init_slideLayers_8c0472c2, 0x148 },
    { init_slideLayers_8c0472c6, 0x149 },
    { init_slideLayers_8c0472b8, 0x14a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a968[] = {
    { init_slideLayers_8c0472ee, 0x14b },
    { init_slideLayers_8c0472f8, 0x14c },
    { init_slideLayers_8c0472fc, 0x14d },
    { init_slideLayers_8c0472f2, 0x14e },
    { init_slideLayers_8c0472f8, 0x14f },
    { init_slideLayers_8c0472ee, 0x150 },
    { init_slideLayers_8c04711c, 0x151 },
    { init_slideLayers_8c04711c, 0x152 },
    { init_slideLayers_8c047146, 0x153 },
    { init_slideLayers_8c04711c, 0x154 },
    { init_slideLayers_8c047142, 0x155 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide *init_omeEvents_8c04a9c8[] = {
    init_eventSlides_8c049c38, init_eventSlides_8c049c70, init_eventSlides_8c049c90, init_eventSlides_8c049cf0,
    init_eventSlides_8c049d10, init_eventSlides_8c049d48, init_eventSlides_8c049d68, init_eventSlides_8c049d90,
    init_eventSlides_8c049dd0, init_eventSlides_8c049df0, init_eventSlides_8c049e08, init_eventSlides_8c049e30,
    init_eventSlides_8c049e58, init_eventSlides_8c049e70, init_eventSlides_8c049eb0, init_eventSlides_8c049ed8,
    init_eventSlides_8c049f20, init_eventSlides_8c049f48, init_eventSlides_8c049f68, init_eventSlides_8c049fa8,
    init_eventSlides_8c049fc8, init_eventSlides_8c049ff8, init_eventSlides_8c04a018, init_eventSlides_8c04a028,
    init_eventSlides_8c04a038, init_eventSlides_8c04a060, init_eventSlides_8c04a080, init_eventSlides_8c04a0a0,
    init_eventSlides_8c04a0c8, init_eventSlides_8c04a108, init_eventSlides_8c04a158, init_eventSlides_8c04a178,
    init_eventSlides_8c04a1b8, init_eventSlides_8c04a1d8, init_eventSlides_8c04a218, init_eventSlides_8c04a238,
    init_eventSlides_8c04a270, init_eventSlides_8c04a290, init_eventSlides_8c04a2c8, init_eventSlides_8c04a2e8,
    init_eventSlides_8c04a328, init_eventSlides_8c04a348, init_eventSlides_8c04a378, init_eventSlides_8c04a398,
    init_eventSlides_8c04a3a8, init_eventSlides_8c04a3b8, init_eventSlides_8c04a3c8, init_eventSlides_8c04a3d8,
    init_eventSlides_8c04a3e8, init_eventSlides_8c04a3f8, init_eventSlides_8c04a408, init_eventSlides_8c04a418,
    init_eventSlides_8c04a428, init_eventSlides_8c04a438, init_eventSlides_8c04a478, init_eventSlides_8c04a4a8,
    init_eventSlides_8c04a4b8, init_eventSlides_8c04a4c8, init_eventSlides_8c04a500, init_eventSlides_8c04a520,
    init_eventSlides_8c04a530, init_eventSlides_8c04a540, init_eventSlides_8c04a570, init_eventSlides_8c04a598,
    init_eventSlides_8c04a5e0, init_eventSlides_8c04a630, init_eventSlides_8c04a660, init_eventSlides_8c04a670,
    init_eventSlides_8c04a680, init_eventSlides_8c04a690, init_eventSlides_8c04a6a0, init_eventSlides_8c04a6b0,
    init_eventSlides_8c04a6c0, init_eventSlides_8c04a6d0, init_eventSlides_8c04a6e0, init_eventSlides_8c04a6f0,
    init_eventSlides_8c04a700, init_eventSlides_8c04a710, init_eventSlides_8c04a720, init_eventSlides_8c04a730,
    init_eventSlides_8c04a740, init_eventSlides_8c04a770, init_eventSlides_8c04a798, init_eventSlides_8c04a7d0,
    init_eventSlides_8c04a800, init_eventSlides_8c04a810, init_eventSlides_8c04a820, init_eventSlides_8c04a850,
    init_eventSlides_8c04a8a8, init_eventSlides_8c04a8e8, init_eventSlides_8c04a930, init_eventSlides_8c04a968,
    NULL,
};

/* Full-screen 20x15 grid of 32x32 cells, shared by every slide layer. */
STATIC NJS_SCROLL init_slideScroll_8c04ab3c = {
    /* celps      */ 32,
    /* mapw, maph */ 20, 15,
    /* sw, sh     */ 0, 0,
    /* list, map  */ NULL, NULL,
    /* px, py     */ 0.0f, 0.0f,
    /* bx, by     */ 0.0f, 0.0f,
    /* pr         */ 0.0f,
    /* sflag      */ 0,
    /* sx, sy     */ 1.0f, 1.0f,
    /* spx, spy   */ 0.0f, 0.0f,
    /* mflag      */ 0,
    /* cx, cy     */ 0.0f, 0.0f,
    /* m          */ { 0.0f, 0.0f, 0.0f, 0.0f },
    /* colmode    */ 0x0210000A,
    /* clip       */ { { 0, 0 }, { 0, 0 } },
    /* attr       */ 0,
    /* sclc       */ ARGB(0xff, 0xff, 0xff, 0xff),
};


/* ====================
 * Forward Declarations
 * ====================
 */

void MessageBoxOpenTextbox_8c02ae3e(int x, int y, float priority, int width, int height, int x2, int y2, int enable_offset);

void MessageBoxFreeAssets_8c02adee(void);
int MessageBoxSetText_8c02aefc(char *string);
int MessageBoxDrawText_8c02af1c(int limit);

void MessageBoxFreeTextboxes_8c02af32(void);

/* ====================
 * Functions
 * ====================
 */

/* The text dat is a 0-terminated array of line-list offsets, each list
 * {text offset, voice id} pairs ending at an empty string. Offsets are
 * relative to the file start. */
STATIC void relocateMessageText_8c02a9fc(void *handle)
{
    int *group;
    int *entry;
    int offset;

    for (group = (int *)handle; *group != 0; group++) {
        entry = (int *)(*group + (int)handle);
        *group = (int)entry;
        for (;;) {
            offset = *entry;
            *entry = offset + (int)handle;
            if (*(char *)(offset + (int)handle) == '\0') {
                break;
            }
            entry += 2;
        }
    }
}

void MessageBoxClearAssets_8c02aa28(void)
{
    var_messageAssetCount_8c228514 = 0;
    var_messageTextDat_8c228518 = (EventLine **) -1;
}
/* An unknown route skips the dat request but still walks the stale
 * var_eventSlides_8c228480, as the original does. */
void MessageBoxRequestAssets_8c02aa36(void)
{
    EventSlide *slide;
    unsigned short *ids;
    unsigned short id;
    int i;

    if (var_route_8c18ad1c == ROUTE_SHINJUKU) {
        var_eventSlides_8c228480 = init_shinjukuEvents_8c049a6c;
        AsqRequestDat_8c011182(var_commonDir_8c18ad6c, "s_text.dat", &var_messageTextDat_8c228518);
    } else if (var_route_8c18ad1c == ROUTE_WANGAN) {
        var_eventSlides_8c228480 = init_wanganEvents_8c04843c;
        AsqRequestDat_8c011182(var_commonDir_8c18ad6c, "w_text.dat", &var_messageTextDat_8c228518);
    } else if (var_route_8c18ad1c == ROUTE_OME) {
        var_eventSlides_8c228480 = init_omeEvents_8c04a9c8;
        AsqRequestDat_8c011182(var_commonDir_8c18ad6c, "o_text.dat", &var_messageTextDat_8c228518);
    }

    var_messageAssetCount_8c228514 = 0;

    for (slide = var_eventSlides_8c228480[var_selectedEventEntry_8c228478]; (int)slide->layers_0x00 != -1; slide++) {
        for (ids = slide->layers_0x00; *ids != 0xffff; ids++) {
            id = *ids;

            for (i = 0; i < var_messageAssetCount_8c228514 && var_messageAssets_8c228484[i].id_0x00 != (int)id; i++) {
            }

            if (i != var_messageAssetCount_8c228514) {
                continue;
            }

            var_messageAssets_8c228484[i].id_0x00 = id;
            AsqRequestPvm_8c011ac0(var_commonDir_8c18ad6c, init_slideAssetFiles_8c046758[id].pvm_0x04,
                                   &var_messageAssets_8c228484[i].pvm_0x04, 0xde, 0);
            AsqRequestDat_8c011182(var_commonDir_8c18ad6c, init_slideAssetFiles_8c046758[id].map_0x00,
                                   &var_messageAssets_8c228484[i].dat_0x08);
            var_messageAssetCount_8c228514++;
        }
    }
}
/* Reveals each line one char per 3 frames (2 per frame while A is held),
 * then waits for A to advance. */
STATIC void slideshowTask_8c02ab7a(Task *task, SlideshowState *state)
{
    SlideshowStage stage;
    int i;
    unsigned short *ids;
    unsigned short id;
    char *nextString;

    stage = SLIDESHOW_STAGE_DRAW;

    switch (state->phase_0x00) {
    case 0:
        state->layers_0x14 = state->slide_0x10->layers_0x00;
        state->line_0x18 = var_messageTextDat_8c228518[state->slide_0x10->lineListIndex_0x04];
        stage = SLIDESHOW_STAGE_SWAP;
        break;

    case 1:
        stage = SLIDESHOW_STAGE_SWAP;
        break;

    case 2:
        stage = SLIDESHOW_STAGE_WAIT;
        break;

    case 3:
        if ((var_peripheral_8c1ba358->on & PDD_DGT_TA) == 0) {
            state->phase_0x00 = 2;
        } else {
            state->revealedChars_0x08 += 2;
            if (state->revealedChars_0x08 >= state->charCount_0x04) {
                state->phase_0x00 = 4;
            }
        }
        break;

    case 4:
        if ((var_peripheral_8c1ba358->press & PDD_DGT_TA) != 0) {
            SndStopAdx_8c010ca6(1);
            state->line_0x18++;
            nextString = state->line_0x18->text_0x00;
            if (*nextString == '\0') {
                state->slide_0x10++;
                if ((int)state->slide_0x10->layers_0x00 == -1) {
                    var_fadeRequest_8c226564 = FADE_REQUEST_OUT;
                    state->phase_0x00 = 5;
                } else {
                    state->phase_0x00 = 0;
                }
            } else {
                state->phase_0x00 = 1;
            }
        }
        break;

    case 5:
        if (!var_isFading_8c226568) {
            MessageBoxFreeAssets_8c02adee();
            TaskUninstall_8c014b66(task);
            EventApplyFlags_8c02b292();
            RouteStartModelLoadPass_8c013d78();
            var_fadeRequest_8c226564 = FADE_REQUEST_IN;
            var_sceneVisible_8c226560 = 1;
            var_messageBoxActive_8c22847c = 0;
            return;
        }
        break;
    }

    if (stage == SLIDESHOW_STAGE_SWAP) {
        state->charCount_0x04 = MessageBoxSetText_8c02aefc(state->line_0x18->text_0x00);
        SndPlayAdx_8c010cd6(2, state->line_0x18->voiceId_0x04);
        state->revealedChars_0x08 = 1;
        state->frameCounter_0x0c = 0;
        state->phase_0x00 = 2;
        stage = SLIDESHOW_STAGE_WAIT;
    }

    if (stage == SLIDESHOW_STAGE_WAIT) {
        if ((var_peripheral_8c1ba358->press & PDD_DGT_TA) != 0) {
            state->frameCounter_0x0c = 99;
            state->phase_0x00 = 3;
            SndStopAdx_8c010ca6(1);
        }
        state->frameCounter_0x0c++;
        if (state->frameCounter_0x0c >= 3) {
            state->revealedChars_0x08++;
            if (state->revealedChars_0x08 < state->charCount_0x04) {
                state->frameCounter_0x0c = 0;
            } else {
                state->phase_0x00 = 4;
            }
        }
    }

    init_slideScroll_8c04ab3c.pr = -3.0f;
    ids = state->layers_0x14;
    for (; *ids != 0xffff; ids++) {
        id = *ids;
        for (i = 0; i < var_messageAssetCount_8c228514; i++) {
            if (var_messageAssets_8c228484[i].id_0x00 == id) {
                init_slideScroll_8c04ab3c.list = (NJS_TEXLIST *)var_messageAssets_8c228484[i].pvm_0x04;
                init_slideScroll_8c04ab3c.map = (Uint32 *)var_messageAssets_8c228484[i].dat_0x08;
                break;
            }
        }
        njDrawScroll(&init_slideScroll_8c04ab3c);
        init_slideScroll_8c04ab3c.pr += 0.1f;
    }
    MessageBoxDrawText_8c02af1c(state->revealedChars_0x08);
    SndPollVoiceEnd_8c0106ac();
}
void MessageBoxStart_8c02ad8c(void)
{
    Task *task;
    SlideshowState *state;

    relocateMessageText_8c02a9fc(var_messageTextDat_8c228518);
    TaskInstall_8c014ae8(var_tasks_8c1ba3c8, &slideshowTask_8c02ab7a, &task, (void **)&state, 0x1c);
    state->slide_0x10 = var_eventSlides_8c228480[var_selectedEventEntry_8c228478];
    MessageBoxOpenTextbox_8c02ae3e(0x20, 0x180, -2.0, 0x240, 0x40, 0, 0, -1);
    state->phase_0x00 = 0;
    var_messageBoxActive_8c22847c = 1;
    var_eventCount_8c1bb8e8++;
}
void MessageBoxFreeAssets_8c02adee(void)
{
    int i;

    for (i = 0; i < var_messageAssetCount_8c228514; i++) {
        syFree(var_messageAssets_8c228484[i].dat_0x08);
        AsqReleaseAndFreeTexlist_8c011e3c(var_messageAssets_8c228484[i].pvm_0x04);
    }
    var_messageAssetCount_8c228514 = 0;

    if (var_messageTextDat_8c228518 != (EventLine **) -1) {
        syFree(var_messageTextDat_8c228518);
        var_messageTextDat_8c228518 = (EventLine **) -1;
    }

    MessageBoxFreeTextboxes_8c02af32();
}
/* Creates a double-buffered pair of identical boxes. The original returns
 * &var_menuTextboxCharLimit_8c225fb8, which no caller uses. */
void MessageBoxOpenTextbox_8c02ae3e(int x, int y, float priority, int width, int height, int x2, int y2, int enable_offset)
{
    if (var_messageTextBoxA_8c1bc404 != (void *) -1) {
        MessageBoxFreeTextboxes_8c02af32();
    }

    TxtInit_8c01524c();
    var_messageTextBoxA_8c1bc404 = TxtCreateTextBox_8c0152fc(x, y, priority, width, height, x2, y2, enable_offset);
    var_messageTextBoxB_8c1bc408 = TxtCreateTextBox_8c0152fc(x, y, priority, width, height, x2, y2, enable_offset);
    var_messageTextBoxIndex_8c1bc40c = 1;
    var_menuTextboxCharLimit_8c225fb8 = 0;
}
/* Lays the text out in the back buffer and flips to it. Returns the char
 * count. */
int MessageBoxSetText_8c02aefc(char *string)
{
    var_messageTextBoxIndex_8c1bc40c ^= 1;
    return TxtPrepareTextBoxLayout_8c01543a((TextBox *)(&var_messageTextBoxA_8c1bc404)[var_messageTextBoxIndex_8c1bc40c], string);
}
/* Draws the first `limit` chars. */
int MessageBoxDrawText_8c02af1c(int limit)
{
    return TxtDrawTextBox_8c0155e0((TextBox *)(&var_messageTextBoxA_8c1bc404)[var_messageTextBoxIndex_8c1bc40c], limit);
}
void MessageBoxFreeTextboxes_8c02af32(void)
{
    if (var_messageTextBoxA_8c1bc404 != (void *) -1) {
        TxtDestroyTextBox_8c015410((TextBox *)var_messageTextBoxA_8c1bc404);
        TxtDestroyTextBox_8c015410((TextBox *)var_messageTextBoxB_8c1bc408);
        TxtDestroy_8c01529c();
        var_messageTextBoxA_8c1bc404 = (void *) -1;
    }
}
