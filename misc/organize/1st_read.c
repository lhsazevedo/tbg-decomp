/* Tokyo Bus Guide IP.BIN
 *
 * reg[REG] = access to register REG
 *
 */

#include "sg_gd.h";
#include "1st_read.h";

boolean ukn_pvm_bool_at_8c18adac;
Task var_tasks_8c1ba3c8[16 + 1];
Task var_tasks_8c1ba5e8[16 + 1];
// TODO: ?
QueuedDat queued_dats[10];
QueuedDat *ukn_dat_end_8c157a90;
int *ff_ptr_8c157a98 = 0;
char *cur_dir_8c157a80;
boolean *_8c0112a8 = FALSE;
void queued_thing_at_8c157ac0
void allocated_8c157abc;
char *DATA_EMPTY_at_8c03334c = "DATA EMPTY.";
int _8c157ac8;
int init_8c03bd80;
int _8c157a60;
int _8c1ba354;
int _8c03bfa8;
int _init_8c03bd84;
void *_8c0139f4;
char _8c013650;
struct _8c315be8 {};
typedef struct _8c315be8 _8c315be8;
boolean _8c0571e2_update_b = FALSE;
// TODO: Check initial value
boolean _8c315598_c = FALSE;
// Task tasks_8c1ba808[?];
// 0x8c935900
// TODO: Discover loaded flag - Appears to be setted at 8c0139c6 ?
QueuedDat *queued_dat_base_8c935900;

ResourceGroup title_resource_group_8c044254 = {
    .parts = "title_parts.dat",
    .dat = "title.dat",
    .pvm = "title.pvm",
    .text_count = 1,
}


// ac010000: entry
//  - Write to register ff00001c
//  - Jump to 8c04f6c0
void mainfunc_8c010080() {
    //   8c010088 0b 43
    // Note: takes some time...
    njUserInit_8c0134ec();

    // 0x8c01008c: Main game loop
    while (1) {
		if (njUserMain_8c01392e() < 0) break;
		njWaitVSync_8c0571e2();
	}

    // Game exits next.

    //   8c0100a0
    return njUserExit();
}

// ...

_dat_8c0111b4(UknDatStruct *ukn_dat_struct) {
    // r13 = ukn_dat_struct
    // r14 = ukn_dat_struct->queued_dat_0x18

    QueuedDat *queued_dat = ukn_dat_struct->queued_dat_0x18;

    if (ukn_dat_struct->_08_int == 0) {
        for (; queued_dat < ukn_dat_end_8c157a90; queued_dat += sizeof(QueuedDat)) {
            if (queued_dat->complete == 0) {
                if (*queued_dat->basepath != '\0') {
                    if (!strcmp("DATA EMPTY.", queued_dat->basepath)) {
                        cur_dir_8c157a80 = queued_dat->basepath;

                        // gdFsChangeDir?
                        gdFsChangeDir_8c0532ac(queued_dat->basepath);
                    }
                }

                // gdFsOpen?
                GDFS *gdfs = gdFsOpen_8c053ba6(queued_dat->filename, 0);
                ukn_dat_struct->gdfs = gdfs;
                if (!gdfs) {
                    gdFsClose_8c0532c4(ukn_dat_struct->gdfs_0x0c);
                    free_8c0545a4(*queued_dat->dest);

                    _8c0112a8 = TRUE;
                    break;
                }

                int size;
                gdFsGetFileSize_8c053316(gdfs, &size);
                if (!size) {
                    _8c0112a8 = TRUE;
                    break;
                }

                size = gdFsCalcSctSize(size);

                void* dest = malloc_8c0544d6(size);
                queued_dat->dest = dest;

                int ret = gdFsRead_8c0533ee(gdfs, size, queued_dat->dest);
                if (!ret) {
                    _8c0112a8 = TRUE;
                    break;
                }

                gdFsClose_8c0532c4(gdfs);

                queued_dat->complete = TRUE;

                return;
            }
        }

        if (var_8c157a88 == 0) {
            *ff_ptr_8c157a98 = 1;

            _8c014b66(ukn_dat_struct);
        } else {
            // 0x8c011250
            ukn_dat_struct->queued_dat = _8c157a8c;

            var_8c157a88 = 0;

            // 0x8c011260
            // TODO
            cur_dir_8c157a80 = "DATA EMPTY."
        }
    } else if (ukn_dat_struct->_08_int == 1) {
        int stat = gdFsGetStat_8c053874(ukn_dat_struct->gdfs_0x0c);

        if (stat == GDD_STAT_COMPLETE) {
            // 0x8c011282...
            gdFsClose_8c0532c4(ukn_dat_struct->gdfs_0x0c);
            queued_dat->complete = TRUE;
        } else if (stat == GDD_STAT_READ) {
            // 0x8c0112cc...
            int ret = _8c0537c8(ukn_dat_struct->gdfs_0x0c);

            if (ret != 0) {
                _8c05371c(ukn_dat_struct->gdfs_0x0c, 2048, queued_dat->dest);
            }

            return;
        } else {
            gdFsClose_8c0532c4(ukn_dat_struct->gdfs_0x0c);

            free_8c0545a4(*queued_dat->dest);
        }

        // 0x8c0112f6...
        queued_dat += sizeof(QueuedDat);
        ukn_dat_struct->queued_dat_0x18 = queued_dat;
    }

    return;
}

sortQueuedDats_8c011310() {
    // The comments below are relative to the marks_parts.dat call

    // 0x8c157a8c = QueuedDat *queued_dat_base_8c935900
    // 0x8c935900 = char* queued_dat_base_8c935900->basepath
    // 0x8c033380 = char '\'

    // 0x8c011322: r12 = 0x8c157a8c = 0x8c935900 = 0x8c033380 = "\SYSTEM"
    // 0x8c011324: r8  = 0x8c157a90 = 0x8c9359a0 = 0xffffffff
    // 0x8c011326: r2  = 0x8c935900 = 0x8c033380 = "\SYSTEM"
    // 0x8c011328: r3  = 0x8c9359a0 = 0xffffffff
    // 0x8c01132a: r2 != r3 (0x8c935900 != 0x8c9359a0)

    //                                TODO
    //                               vvvvvvv
    if (queued_dat_base_8c935900->basepath != reg[r3]) {

        // malloc?
        //                                              TODO
        //                                             vvvvvvv
        void *temp_queued_dats_maybe = malloc_8c0544d6(reg[r4] - queued_dat_base_8c935900->basepath);
        // temp_queued_dats_maybe = allocated memory (0xa0 bytes?)

        boolean swapped;

        do {
            swapped = FALSE;

            QueuedDat* next_queued_dat_base = queued_dat_base_8c935900 + sizeof(QueuedDat);

            while (queued_dat_base_8c935900 < reg[r2]) {
                if (strcmp(queued_dat_base_8c935900->filename, next_queued_dat_base->filename)) {
                    memcpy(queued_dat_base_8c935900, temp_queued_dats_maybe, 0x10);
                    memcpy(next_queued_dat_base, queued_dat_base_8c935900, 0x10);
                    memcpy(temp_queued_dats_maybe, next_queued_dat_base, 0x10);

                    swapped = TRUE;
                }

                queued_dat_base_8c935900 += sizeof(QueuedDat);
            }
        } while (swapped);

        free(temp_queued_dats_maybe);

        //   8c01139c 08 20         tst       r0,r0
        //   8c01139e 02 8f         bf/s      LAB_8c0113a6
        //   8c0113a0 04 7f         _add      #0x4,r15
        
        // 1st param is a func**
        // 2nd param is a func* (QueuedDat)
        // 3rd param is a unknown * 
        // 4rd param is a unknown * --> 0x8C2260D4 on this call
        //     appears to be destination
        if (pushTask_8c014ae8(_8c1ba3c8, _8c0111b4, ptr1, ptr2, 0)) {
            // TODO:
            //   8c0113a6 c2 62         mov.l     @r12=>DAT_8c157a8c,r2
            //   8c0113a8 f2 63         mov.l     @r15=>local_28,r3
            //   8c0113aa 1f d1         mov.l     PTR_PTR_s_DATA_EMPTY_8c011428,r1           = 8c03be7c
            //   8c0113ac 26 13         mov.l     r2,@(0x18,r3)
            //   8c0113ae f2 63         mov.l     @r15=>local_28,r3
            //   8c0113b0 1e d0         mov.l     PTR_DAT_8c01142c,r0                        = 8c157a80
            //   8c0113b2 a2 13         mov.l     r10,@(0x8,r3)
            //   8c0113b4 1b d3         mov.l     PTR_DAT_8c011424,r3                        = 8c157a88
            //   8c0113b6 a2 23         mov.l     r10,@r3=>DAT_8c157a88
            //   8c0113b8 12 62         mov.l     @r1=>PTR_s_DATA_EMPTY_8c03be7c,r2          = 8c03334c
            //   8c0113ba 22 20         mov.l     r2,@r0=>DAT_8c157a80
            //   8c0113bc 01 e0         mov       #0x1,r0

            return 1;
        }
    }

    return 0;
}

void _8c011f36() {

}

void _8c011f6c() {

    // 
    _8c01116a();

    _8c01147a();

    _8c0117fe();

    queued_thing_at_8c157ac0 = allocated_8c157abc;

    allocated_8c157abc = DATA_EMPTY_at_8c03334c;
    cur_dir_8c157a80 = DATA_EMPTY_at_8c03334c;
    _8c157ac8 = 1;

    return _8c157ac8;
}

// ...

void task_8c013388(Task *task, void *allocated) {
    if (task->field_0x08 == 0) {
        // 0x8c013440

        if (!isPvmReady_8c01432a()) {
            return;
        }

        // 0x8c01344a

        task->field_0x08++;

        float val = _8c1bc448->field_0x04;

        // float comparison in r3?
        if (val < 0) {
            val += 4.3;
        }

        val -= 1.0;

        // Allocation stuff?
        _8c011f6c();

        // 0x8c013472
        
        // Sound stuff...

        _8c011fe0();
    } else if (task->field_0x08 = 1) {
        if (isPvmReady_8c01432a()) {
            return;
        }

        _8c011f7e();

        _8c014b66(task);

        _8c010e18("\\SOUND");

        _8c2260a8 = 0;

        return _8c015fd6(0);
    } else {
        return;
    }
}

void njUserInit_8c0134ec() {
    _8c01356c(0x100000);

    if (_8c0550b0() == 0) {
        _8c013574(0x31, 0, 2);
    } else {
        _8c0149b0(0x38, 0, 2);

        _8c079318(1.0, 0.91);
    }

    // 0x8c01358c
    void *_8c2f8ca0 = (void *) 0x8c2f8ca0;
    _8c073248(_8c2f8ca0, 0x10, 0);

    // 0x8c013596
    void *_8c255ca0 = (void *) 0x8c255ca0;
    _8c05b7b0(_8c255ca0, 0x0800);

    // 0x8c013596
    // njInitVertexBuffer?
    _8c056ea2(800000, 320000, 320000, 320000, 20000);

    // 0x8c0135b2
    void *_8c277ca0 = (void *) 0x8C277CA0;
    _8c059dcc(_8c277ca0, 0x80800);

    // 0x8c0135bc
    void *_8c157af8 = (void *) 0x8C157AF8;
    _8c059de2(_8c157af8, 0x0c00);

    void *_8c235ca0 = (void *) 0x8c235ca0;
    _8c057f18(_8c235ca8C014F82
    void *_8c2f84a0 = (void *) 0x8c2f84a0;
    _8c05b62e(_8c2f84a0);

    _8c05483c();

    // 0x8c0135de
    a = _8c010924();

    char param1;
    if (a < 0) {
        param1 = 0
    } else {
        param1 = *a;
    }

    // 0x8c013656

    _8c0108c0(param1);

    _8c010fbe();

    _8c014b8c();

    _8c059abc(512, _8c277ca0, 2817, 256);

    _8c059aca(_8c18acf8, 512, 999, 0x40800000);

    _8c059b98(256);

    _8c059e3a(_8c03bf44);

    fill_with_ffffs_8c014a9c(?, 16);

    fill_with_ffffs_8c014a9c(?, 16);

    fill_with_ffffs_8c014a9c(?, 32);

    fill_with_ffffs_8c014a9c(?, 64);

    fill_with_ffffs_8c014a9c(?, 32);

    // ...

    // 0x8c013834
    pushTask_8c014ae8(var_tasks_8c1ba3c8, task_8c013388, local_sega, local_val0x78, 0);

    // 0x8c013850
    // Allocations and preps?
    _8c011f36();

    // 0x8c013856
    // Allocations and preps?
    _8c011f6c();

    // ...
}

int njUserMain_8c01392e() {
    if (init_8c03bd80 == 0) {
        // 0x8c013956
        if (!_8c157a60 == 0) {
            // 0x8c01395e
            // Just before logo
            if (_8c03bfa8 != 0) {
                // 0x8c013976

                // gdFs?
                _8c053c02();

                int ret = gdFsGetStat_8c053874(?);

                // GDD_STAT_BUSY
                if (ret == 4) {
                    _8c03bfa8 = 0;
                }
            } else {
                // 0x8c013966
                int ret = gdFsGetStat_8c053874();

                // GDD_STAT_IDLE
                if (ret == 0) {
                    _8c03bfa8 = 1;
                }
            }
        }

        // 0x8c01398a
        // gdFs?
        int ret = _8c053c5c();

        if (ret == 6) {
            if (_8c1ba354 != -1) {
                _8c056450(_8c1ba354);
            }

            return -1;
        }

        // 0x8c013994
        int ret2 = _8c053c5c();

        if (ret2 == 6 || ret2 = 0) {
            _8c053c74(ret2);
        }

        // 0x8c0139aa
        if (_8c013aa4 != 0) {
            return -1;
        }
    } else {
        // 0x8c01393e
        if (_init_8c03bd84 == 0) {
            if (_8c1ba354 != -1) {
                _8c056450(_8c1ba354);
            }

            return -1;
        }
    }

    // 0x8c0139c2

    // Param _8c0139f4 is 0x8c1ba3c8 on first call,
    // and already populated with tree items.
    execTasks_8c014b42(_8c0139f4);

    return 0;
}

// ...

void resetPvmReady_8c014322() {
    *ukn_pvm_bool_at_8c18adac = FALSE;
}

boolean isPvmReady_8c01432a() {
    return *ukn_pvm_bool_at_8c18adac;
}

void setPvmReady_8c014330() {
    *ukn_pvm_bool_at_8c18adac = TRUE;
}

void pushTask_8c014ae8(Task task[], void (*cb_r5)(void), void *param_r6, void *param_r7, void alloc_size_st1) {
    while (task->callback_0x00 != -1) {
        if (task->callback_0x00 == 0) {
            return 0;
        }
        task += 0x20; // sizeof(Task)
    }

    if (alloc_size_st1 != 0) {
        void *ret = malloc(alloc_size_st1);
        task->allocated_0x04 = ret;
        if (ret == 0) {
            return 0;
        }   
    } else {
        task->allocated_0x04 = 0;
    }

    *task->callback_0x00 = cb_r5;

    // TODO: ???
    *param_r6 = task;
    *param_r7 = task->allocated_0x04;

    return 1;
}

void _8c014b3e();

void execTasks_8c014b42(Task *task) {
    while (*task) {
        if (*task->callback_0x00 == -1) {
            continue;
        }

        task->callback_0x00(task, task->allocated_0x04);

        // size = 0x20 (32 bytes)
        task += sizeof(Task);
    }
}

// ...

void draw_dat_8c014f54(UknDatStruct2 *ukndatstruct_r4, int texture_id, float x, float y, float float_fr7) {
    char *nextptr;
    if (texture_id == 2000) {
        // 0x8c014f76
        nextptr = ukndatstruct_r4->contents;
    } else {
        // 0x8c014f78
        int offset = ukndatstruct_r4->contents + texture_id * 4;
        nextptr = ukndatstruct_r4->contents + offset * 4;
    }

    // 0x8c014f68
    void *local_24_0x18 = ukndatstruct_r4->field_0x00;
    void *local_28_0x1c = ukndatstruct_r4->field_0x04;
    // 0x8c014f9a
    int local_20_0x14 = 0;
    // 0x8c014f9c
    float local_12_0x0c = 1.0;
    // 0x8c014fa0
    float local_16_0x10 = 1.0;
    
    // 0x8c014fa2: float constant used later
    // float a_fr14 = 0.0001;


    int i = 0;
    DatFileUknStruct1 *uknstruct3 = nextptr;
    float frpass = y;

    while (TRUE) {
        DatFileUknStruct1 *uknstruct3 = (DatFileUknStruct1*) nextptr + i * 12;

        if (uknstruct3->field_0x00 == -1) {
            break;
        }

        // fr12 = fr4
        // fr13 = fr5

        // Struct
        float float_local_0 = uknstruct3->float_1 + x;
        float float_local_4 = uknstruct3->float_2 + x;

        _8c074c08(uknstruct3->field_0x00, &float_local_0, 0x20);

        frpass += 0.0001;
        uknstruct3 += sizeof(DatFileUknStruct1);
        i++;
    }
}

// ...

undefined task_8c015ab8() {

}

void _8c015fd6(int param_r4) {
    _8c0128cc(0);

    pushTask_8c014ae8(var_tasks_8c1ba3c8, task_8c012f44, st_plus_4, st_plus_8, 0);

    pushTask_8c014ae8(var_tasks_8c1ba3c8, task_8c015ab8, st_plus_8, st_plus_c, 0);

    _8c1bc7a8->field_0x18 = 0;
    _8c1bc7a8->field_0x64 = 0;

    // 0x8c016024 ...

    _8c059f94(?);

    // ...

    _8c011f36();

    _8c011f6c();

    // ...

    // 0x8c01606e
    request_sys_resgrp_8c018568(_8c1bc7a8->field_0x0c, title_resource_group_8c044254);

    // Common dats and PVMs
    _8c01852c();

    // Pushes another task
    _8c011fe0();
}

void _8c04f6c0() {
    //   8c04f6c0 24 d2         mov.l     LAB_8c04f754,r2
    //   8c04f6c2 22 4f         sts.l     PR,@-r15
    //   8c04f6c4 22 63         mov.l     @r2=>DAT_f649f449,r3
    // r3 = "---\n", but used as an address?
    //   8c04f6c6 30 61         mov.b     @r3,r1
    // r1 = 0

    //   0x8c04f6c8 to 0x8c04f6d4
    // Write "SEGA" (or "AGES" ?) from 0x8c00c000 to 0x8c00f400
    // TODO

    //   8c04f6d6
    // No effect observed (without patching)
    _8c04f6f8();

    //   8c04f6da
    // No effect observed (without patching)
    _8c04f6e8();

    //   8c04f6e0
    //  Game runs here?
    mainfunc_8c010080();

    // f6e4
    _8c04f6f0();

    // f6f4
    _8c051e1a();


    //   8c04f6de 21 d2         mov.l     PTR_DAT_8c04f764,r2                        = 8c09067a
    //   8c04f6e0 52 1f         mov.l     r5,@(0x8,r15)
    //   8c04f6e2 62 85         mov.w     @(0x4,r6)=>DAT_f80bf82f,r0
    //   8c04f6e4 14 7c         add       #0x14,r12
    //   8c04f6e6 c3 63         mov       r12,r3
}

void njWaitVSync_8c0571e2() { 
    //   8c0571f6
    //   in r12
    _8c315be8 a = _8c0807b2();

    // 0x8C057202
    // If skipped: No effect observed...
    // If return here: No game update, song continues.
    if (_8c0571e2_update_b) {

        // TODO: Find declaration
        int local_r14 = 0;

        // 0x8c057222
        // 166000 is at 0x8c057318
        // TODO: No effect observed changing this number...
        while (_8c315598_c || local_r14 < 166000) {
            _8c0807b2();
            ... = _8c0807bc(...);
            local_r14 = _8c0807f6(...);
        }

        //   8c05722a
        if (local_r14 < 166000) {
            _8c0571cc();
        }
    }

    // 0x8c057234
    // If skipped
    //   - at SEGA: Freeze, able to return
    //   - at Title screen: Screen freeze, audio continues, able to press enter. Crashes soon after.
    //   - at demo: Instant crash
    _8c096be0();

    // 0x8c3153dc = 0
    // 0x8c3153e0 = 0
    // 0x8c3153e4 = 0
    // _8c315404++

    // 0x8c057254
    // If skipped: Rushes licenses and runs demo without graphics
    _8c0966a0(_8c315404);

    // 0x8c057254
    // If skipped: No input
    _8c05413a();

    // 0x8c05725e: No effect observed
    _8c0807b2();

    // 0x8c057262
    // Some writes...
}

_8c315be8 _8c0807b2() {}

// ...

int _8c0807f6(int r4_param1) {
    int r4_copy = (r4_param1 & 0x7f) * 100;

    int r4_copy2 = (r4_param1 >> 25) * 100;

    return (r4_copy >> 25) + r4_copy2;
}

// ...

// Pretty long
_8c096be0 () {
    //   8c096be0 e6 2f         mov.l     r14,@-r15=>local_4
    //   8c096be2 d6 2f         mov.l     r13,@-r15=>local_8
    //   8c096be4 c6 2f         mov.l     r12,@-r15=>local_c
    //   8c096be6 b6 2f         mov.l     r11,@-r15=>local_10
    //   8c096be8 a6 2f         mov.l     r10,@-r15=>local_14
    //   8c096bea 96 2f         mov.l     r9,@-r15=>local_18
    //   8c096bec 86 2f         mov.l     r8,@-r15=>local_1c
    //   8c096bee 22 4f         sts.l     PR,@-r15=>local_20
    // Save regs

    //   8c096bf0 a8 7f         add       #-0x58,r15
    //   8c096bf2 83 d3         mov.l     PTR_DAT_8c096e00,r3                        = 8c0e9c04
    //   8c096bf4 f3 62         mov       r15,r2
    //   8c096bf6 10 72         add       #0x10,r2
    //   8c096bf8 32 61         mov.l     @r3=>DAT_8c0e9c04,r1                       = 80000000h
    //   8c096bfa 31 50         mov.l     @(0x4,r3)=>DAT_8c0e9c08,r0
    //   8c096bfc 12 22         mov.l     r1,@r2=>local_68
    //   8c096bfe 01 12         mov.l     r0,@(0x4,r2)=>local_64
    //   8c096c00 33 50         mov.l     @(0xc,r3)=>DAT_8c0e9c10,r0
    //   8c096c02 32 51         mov.l     @(0x8,r3)=>DAT_8c0e9c0c,r1                 = 00800400h
    //   8c096c04 12 12         mov.l     r1,@(0x8,r2)=>local_60
    //   8c096c06 03 12         mov.l     r0,@(0xc,r2)=>local_5c
    //   8c096c08 35 50         mov.l     @(0x14,r3)=>DAT_8c0e9c18,r0                = FFFFFFFFh
    //   8c096c0a 34 51         mov.l     @(0x10,r3)=>DAT_8c0e9c14,r1                = FFFFFFFFh
    //   8c096c0c 14 12         mov.l     r1,@(0x10,r2)=>local_58
    //   8c096c0e 05 12         mov.l     r0,@(0x14,r2)=>local_54
    //   8c096c10 37 50         mov.l     @(0x1c,r3)=>DAT_8c0e9c20,r0                = FFFFFFFFh
    //   8c096c12 36 51         mov.l     @(0x18,r3)=>DAT_8c0e9c1c,r1                = FFFFFFFFh
    //   8c096c14 16 12         mov.l     r1,@(0x18,r2)=>local_50
    //   8c096c16 07 12         mov.l     r0,@(0x1c,r2)=>local_4c
    //   8c096c18 7a d2         mov.l     DAT_8c096e04,r2                            = 8C34E7D8h
    //   8c096c1a 22 60         mov.l     @r2=>DAT_8c34e7d8,r0
    //   8c096c1c 40 c8         tst       #0x40,r0
    //   8c096c1e 0d 89         bt        LAB_8c096c3c



    // ...
}
