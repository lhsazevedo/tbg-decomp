/* 8c013ae8 */

#include <shinobi.h>

/* =================
 * Type Declarations
 * =================
 */

enum ROUTE {
    ROUTE_SHINJUKU = 0,
    ROUTE_WANGAN   = 1,
    ROUTE_OME      = 2,
};

enum TIME_OF_DAY {
    TIME_OF_DAY_DAY     = 0,
    TIME_OF_DAY_EVENING = 1,
    TIME_OF_DAY_NIGHT   = 2,
};

typedef struct {
    int field_0x00;
    int field_0x04; // holds a float bit-pattern (e.g. -300.0f), not an int
    Uint8 field_0x08;
    Uint8 field_0x09;
    Uint8 field_0x0a;
    Uint8 field_0x0b;
    float fogN_0x0c;
    float fogF_0x10;
} FogParams;

/* =====================
 * External Declarations
 * =====================
 */

extern enum ROUTE var_route_8c18ad1c;
extern enum TIME_OF_DAY var_timeOfDay_8c18ad20;
extern FogParams *var_fogParams_8c18ad28;

/* =========
 * Functions
 * =========
 */

void pushRouteLoadTask_8c0144fc(void);
void pushInteriorLoadTask_8c01468e(void);
