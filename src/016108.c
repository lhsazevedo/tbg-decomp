/* 8c016108 */
#include <shinobi.h>
#include "015ab8_title.h"

extern void* var_8c225fb0;
extern ResourceGroup* var_resourceGroup_8c2263a8;
extern CourseMenuFreeResourceGroup_8c0185c4(ResourceGroup* dds);

/* Matched */
void freeResourceGroups_8c016108()
{
    CourseMenuFreeResourceGroup_8c0185c4(&menuState_8c1bc7a8.resourceGroupA_0x00);
    CourseMenuFreeResourceGroup_8c0185c4(&menuState_8c1bc7a8.resourceGroupB_0x0c);
    CourseMenuFreeResourceGroup_8c0185c4(&var_resourceGroup_8c2263a8);

    free_8c1bc404_8c02af32();
    var_8c225fb0 = (void *) -1;
}
