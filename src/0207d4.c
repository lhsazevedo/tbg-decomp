/* 8c0207d4 */
#include "0207d4.h"

float FUN_8c0207d4(Struct8c0207d4 *param1, Struct8c0207d4 *param2, Struct8c0207d4 *param3)
{
    float a = param2->field_0x00 - param1->field_0x00;
    float b = param3->field_0x00 - param1->field_0x00;
    float c = param2->field_0x08 - param1->field_0x08;
    float d = param3->field_0x04 - param1->field_0x08;

    return c * d + b * a;
}

float FUN_8c0207fa(Struct8c0207d4 *param1, Struct8c0207d4 *param2, Struct8c0207d4 *param3)
{
    float a = param2->field_0x00 - param1->field_0x00;
    float b = param3->field_0x04 - param1->field_0x08;
    float c = param2->field_0x04 - param1->field_0x08;
    c *= param3->field_0x00 - param1->field_0x00;

    return a * b - c;
}
