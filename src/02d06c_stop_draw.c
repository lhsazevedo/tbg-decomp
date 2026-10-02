/* @unit StopDraw */

#include <shinobi.h>
#include "1ba1c8_globals.h"
#include "021b9c_tile_draw.h"
#include "02d19c_passenger.h"
#include "02c884_stop.h"

/* ====================
 * Functions
 * ====================
 */

void StopDrawWaitingPassengers_8c02d06c(int layer)
{
    int i;
    Sint8 val;
    NJS_TEXLIST *tlist;
    int spriteIndex;

    for (i = 0; i < var_waitingPassengerCount_8c228794; i++) {
        val = *(Sint8 *)var_waitingPassengers_8c228798[i].spot_0x00;
        if (val < 0x41) {
            tlist = var_pedestrianAssets_8c1bbfdc[(int)val].texlist_0x08;
            if (tlist != (NJS_TEXLIST *)-1) {
                var_passengerSprite_8c2288d8.tlist = tlist;
                var_passengerSprite_8c2288d8.p = var_waitingPassengers_8c228798[i].pos_0x04;
                spriteIndex = (layer == 0) ? 0x23 : 0x22;
                njDrawSprite3D(&var_passengerSprite_8c2288d8, spriteIndex, 0x30);
            }
        }
    }
}

void StopDrawLightBegin_8c02d0fc(int arg0)
{
    njCnkSetSimpleLight(var_simpleLightDir_8c2264d8[0], var_simpleLightDir_8c2264d8[1],
                        var_simpleLightDir_8c2264d8[2]);
    njCnkSetSimpleLightIntensity(var_simpleLightIntensity_8c2264f0[0],
                                 var_simpleLightIntensity_8c2264f0[1]);
    njCnkSetSimpleLightColor(var_simpleLightColor_8c2264f8[0], var_simpleLightColor_8c2264f8[1],
                             var_simpleLightColor_8c2264f8[2]);
    njControl3D(0x120);
    njSetConstantAttr(0xffffffff, 0x100000);
    njSetConstantMaterial((NJS_ARGB *)var_passengerFadeColor_8c228960);
}

void StopDrawLightEnd_8c02d146(int arg0)
{
    njControl3D(0x100);
}
