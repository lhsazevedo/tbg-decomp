/* 8c010080 */
/*---------------------------------------------------------------------------
  Tokyo Bus Guide
---------------------------------------------------------------------------*/

#include <shinobi.h>

int main(void)
{
    GameInit_8c0134ec();

    while (1) {
        if (GameMain_8c01392e() < NJD_USER_CONTINUE) break;
        njWaitVSync();
    }

    GameExit_8c0139d4();
}
