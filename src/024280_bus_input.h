#ifndef _024280_BUS_INPUT_H
#define _024280_BUS_INPUT_H

/* =======================
 * Non-initialized Globals
 * =======================
 */

/* Of the current press, 0..255. */
extern int var_brakePressPeak_8c227d8c;

/* =========
 * Functions
 * =========
 */

/* Pedals, engine state, turn signals and mirrors, and in manual drive mode
 * the steering. */
void BusInputUpdate_8c0246b2(void);

/* Holds same-direction traffic in the mirror view to 20 km/h below the
 * bus. */
void BusInputCapMirrorTraffic_8c024280(void);

#endif // _024280_BUS_INPUT_H
