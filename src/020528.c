/* 8c020528 */
#include "014a9c_tasks.h"
#include "020214.h"
#include "014f54_text_pre_data.h"
#include "0fcd20_sectionB.h"
#include "020528.h"

struct Struct8c2264b8 {
    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int field_0x10;
    int field_0x14;
    int field_0x18;
}
typedef Struct8c2264b8;

#define var_8c2264b8_typed (*(Struct8c2264b8*)var_8c2264b8)

void FUN_8c020528()
{
    Task* created_task;
    void* created_state;

    if (var_playMode_8c1bb8d0 != 2) {
        pushTask_8c014ae8(var_tasks_8c1ba5e8, &FUN_8c020214, &created_task, &created_state, 0);
        var_8c2264b8_typed.field_0x00 = 0;
        var_8c2264b8_typed.field_0x04 = AsqGetRandomInRangeB_8c0121be(300) + 0x96;
        var_8c2264b8_typed.field_0x08 = 3;
        var_8c2264b8_typed.field_0x0c = 1;
        var_8c2264b8_typed.field_0x14 = 0;
        var_8c2264b8_typed.field_0x18 = 0;
    }
}
