#include <shinobi.h>
// sizeof _var_8c225fa8 = 8

typedef struct Struct8c225fa8 Struct8c225fa8;

struct Struct8c225fa8
{
    Struct8c225fa8* field_0x00;
    Sint8 field_0x04;
    Sint8 field_0x05;
    Sint8 field_0x06;
    Sint8 field_0x07;
};


extern Struct8c225fa8* var_8c225fa8;
extern Struct8c225fa8* var_8c225fa8_2;
extern int var_8c1bbc84;
//extern int var_8c1ba364;
extern int var_8c225fac;
extern PDS_PERIPHERAL var_peripherals_8c1ba35c[2];

void task_8c016bf4()
{
    Struct8c225fa8* temp;

    if (var_8c1bbc84 >= 1) {
        temp = &var_8c225fa8->field_0x00;

        if (temp < &var_8c225fa8_2) {
            // var_8c1ba364 = *_var_8c225fa8;
            var_peripherals_8c1ba35c[0].on = (int) &temp;

            // var_8c1ba36c = var_8c1ba364 & (_var_8c225fac ^ var_8c1ba364);
            var_peripherals_8c1ba35c[0].press = var_peripherals_8c1ba35c[0].on & (var_8c225fac ^ var_peripherals_8c1ba35c[0].on);

            // var_8c1ba378 = (short)*(char *)(_var_8c225fa8 + 1);
            var_peripherals_8c1ba35c[0].x2 = var_8c225fa8->field_0x04;

            // var_8c1ba374 = (ushort)*(byte *)((int)_var_8c225fa8 + 5);
            var_peripherals_8c1ba35c[0].x1 = var_8c225fa8->field_0x05;

            // var_8c1ba376 = (ushort)*(byte *)((int)_var_8c225fa8 + 6);
            var_peripherals_8c1ba35c[0].y1 = var_8c225fa8->field_0x06;

            // _var_8c225fa8 = _var_8c225fa8 + 2;
            var_8c225fa8++;

            // _var_8c225fac = var_8c1ba364;
            var_8c225fac = var_peripherals_8c1ba35c[0].on;
        }
    }
}
