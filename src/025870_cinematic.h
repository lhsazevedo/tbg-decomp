/* 8c025870 */
#ifndef _025870_CINEMATIC_H
#define _025870_CINEMATIC_H

/* Indexed by courseId_0x00 - 0x26. */
extern char init_demoFirstShot_8c0460b0[];

/* The passenger boarding view, down the aisle. */
void CinematicBoardingCamera_8c025870(void);

/* Also refreshes the camera-to-bus move delta that 027958_bus_draw's
 * forward cone tests against. */
void CinematicUpdateCamera_8c025906(void);

/* The attract-mode camera tour: shots cued by marks painted along the
 * route, not one per bus stop. */
void CinematicStartTour_8c025af4(void);

#endif // _025870_CINEMATIC_H
