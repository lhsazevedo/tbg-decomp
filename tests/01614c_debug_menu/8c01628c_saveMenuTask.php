<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * saveMenuTask_8c01628c is a TaskAction: (Task *task /r4/, SaveMenuState *state /r5/).
 * task passes through untouched until state 8, which frees it.
 * The state struct: +0x00 state, +0x04 selected vmu, +0x08 port,
 * +0x0c frame counter, +0x10 BupInfo pointer.
 *
 * The whole assembled function loads several external addresses in its prologue,
 * so every external it references must be sized even for these state-0 tests.
 */
return new class extends TestCase {
    public function test_state0_still_counting_just_increments_frame_counter()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 0);   // state
        $this->initUint32($p + 0x04, 3);   // selected vmu (iVar5)
        $this->initUint32($p + 0x0c, 5);   // frame counter

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);   // r4 task unused

        $this->shouldWriteLong($p + 0x0c, 6);   // counter++
        $this->shouldWriteLong($p + 0x04, 3);   // selectedVmu written back unchanged
    }

    public function test_state0_timeout_no_card_goes_to_state7()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 0);
        $this->initUint32($p + 0x04, 3);
        $this->initUint32($p + 0x0c, 10);   // +1 == 11 (>= 0xb): probe for a card

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);   // r4 task unused

        $this->shouldWriteLong($p + 0x0c, 11);
        $this->shouldCall('_VmMenuUpdateVmusStatus_8c019550')
            ->with($this->addressOf('_init_replaySaveNames_8c044294'), 0x1e)
            ->andReturn(0);
        $this->shouldWriteLong($p + 0x00, 7);   // no card -> state 7
        $this->shouldWriteLong($p + 0x04, 3);
    }

    public function test_state0_timeout_card_present_goes_to_state1()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 0);
        $this->initUint32($p + 0x04, 3);
        $this->initUint32($p + 0x0c, 10);

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);   // r4 task unused

        $this->shouldWriteLong($p + 0x0c, 11);
        $this->shouldCall('_VmMenuUpdateVmusStatus_8c019550')
            ->with($this->addressOf('_init_replaySaveNames_8c044294'), 0x1e)
            ->andReturn(1);
        $this->shouldWriteLong($p + 0x00, 1);   // card present -> state 1
        $this->shouldWriteLong($p + 0x04, 0);   // selectedVmu = 0 ...
        $this->shouldWriteLong($p + 0x04, 3);   // ... then overwritten by iVar5
    }

    /* --- state 1: VMU selection screen --- */

    public function test_state1_idle_draws_slots_and_cursor()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 1);        // state 1
        $this->initUint32($p + 0x04, 0);        // cursor on slot 0
        $this->initVmuStatus([0 => 1]);         // slot 0 present
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);   // press = none

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_VmMenuUpdateVmusStatus_8c019550')
            ->with($this->addressOf('_init_replaySaveNames_8c044294'), 0x1e)->andReturn(1);
        $this->shouldCall('_njPrintD')->with(0xf0008, 0, 1);
        $this->shouldCall('_njPrintC')->with(0xf0018, "NO SAVING");
        $this->shouldCall('_njPrintC')->with(0xa0008, "-");
        $this->shouldWriteLong($p + 0x04, 0);   // cursor unchanged
    }

    public function test_state1_confirm_valid_slot_mounts_and_goes_to_state2()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 1);
        $this->initUint32($p + 0x04, 0);
        $this->initVmuStatus([0 => 1]);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x4);   // press = PDD_DGT_TA

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x50, 0);      // Work == NULL -> needs mount

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_VmMenuUpdateVmusStatus_8c019550')
            ->with($this->addressOf('_init_replaySaveNames_8c044294'), 0x1e)->andReturn(1);
        $this->shouldCall('_njPrintD')->with(0xf0008, 0, 1);
        $this->shouldCall('_njPrintC')->with(0xf0018, "NO SAVING");
        $this->shouldWriteLong($p + 0x08, 0);       // port = slot 0
        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldWriteLong($p + 0x10, $bup);    // bupInfo pointer
        $this->shouldCall('_BupMount_8c014c00')->with(0);
        $this->shouldWriteLong($p + 0x00, 2);       // -> state 2
        $this->shouldCall('_njPrintC')->with(0xa0008, "-");
        $this->shouldWriteLong($p + 0x04, 0);       // cursor unchanged (epilogue)
    }

    public function test_state1_confirm_no_saving_slot_goes_to_state4()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 1);
        $this->initUint32($p + 0x04, 8);        // cursor on the "NO SAVING" pseudo-slot
        $this->initVmuStatus([8 => 1]);         // only slot 8 present
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x4);

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_VmMenuUpdateVmusStatus_8c019550')
            ->with($this->addressOf('_init_replaySaveNames_8c044294'), 0x1e)->andReturn(1);
        $this->shouldCall('_njPrintC')->with(0xf0018, "NO SAVING");
        $this->shouldWriteLong($p + 0x00, 4);   // -> state 4
        $this->shouldCall('_njPrintC')->with(0xa0018, "-");
        $this->shouldWriteLong($p + 0x04, 8);   // cursor unchanged (epilogue)
    }

    public function test_state1_down_moves_to_next_present_slot()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 1);
        $this->initUint32($p + 0x04, 0);        // cursor on slot 0
        $this->initVmuStatus([0 => 1, 3 => 1]); // slots 0 and 3 present
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x20);   // press = PDD_DGT_KD

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_VmMenuUpdateVmusStatus_8c019550')
            ->with($this->addressOf('_init_replaySaveNames_8c044294'), 0x1e)->andReturn(1);
        $this->shouldCall('_njPrintD')->with(0xf0008, 0, 1);
        $this->shouldCall('_njPrintD')->with(0xf000e, 3, 1);
        $this->shouldCall('_njPrintC')->with(0xf0018, "NO SAVING");
        $this->shouldCall('_njPrintC')->with(0xa000e, "-");   // cursor now at slot 3
        $this->shouldWriteLong($p + 0x04, 3);
    }

    public function test_state1_up_wraps_to_last_present_slot()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 1);
        $this->initUint32($p + 0x04, 0);        // cursor on slot 0 (nothing below it)
        $this->initVmuStatus([0 => 1, 3 => 1]);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x10);   // press = PDD_DGT_KU

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_VmMenuUpdateVmusStatus_8c019550')
            ->with($this->addressOf('_init_replaySaveNames_8c044294'), 0x1e)->andReturn(1);
        $this->shouldCall('_njPrintD')->with(0xf0008, 0, 1);
        $this->shouldCall('_njPrintD')->with(0xf000e, 3, 1);
        $this->shouldCall('_njPrintC')->with(0xf0018, "NO SAVING");
        $this->shouldCall('_njPrintC')->with(0xa000e, "-");   // wrapped up to slot 3
        $this->shouldWriteLong($p + 0x04, 3);
    }

    public function test_state1_vanished_slot_snaps_to_first_present()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 1);
        $this->initUint32($p + 0x04, 2);        // cursor on slot 2, which is now absent
        $this->initVmuStatus([0 => 1, 5 => 1]);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);   // press = none

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_VmMenuUpdateVmusStatus_8c019550')
            ->with($this->addressOf('_init_replaySaveNames_8c044294'), 0x1e)->andReturn(1);
        $this->shouldCall('_njPrintD')->with(0xf0008, 0, 1);
        $this->shouldCall('_njPrintD')->with(0xf0012, 5, 1);
        $this->shouldCall('_njPrintC')->with(0xf0018, "NO SAVING");
        $this->shouldCall('_njPrintC')->with(0xa0008, "-");   // snapped to slot 0
        $this->shouldWriteLong($p + 0x04, 0);
    }

    /* --- state 2: card readiness / capacity check --- */

    public function test_state2_not_ready_prints_checking()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 2);
        $this->initUint32($p + 0x04, 3);        // selectedVmu, untouched here
        $bup = $this->alloc(0x5c);
        $this->initUint16($bup + 0x00, 0);      // Ready == 0
        $this->initUint32($p + 0x10, $bup);     // bupInfo pointer (read via state)

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_njPrint')->variadic(1)->with(0xa000a, "CHECKING...");
        $this->shouldWriteLong($p + 0x04, 3);   // epilogue, unchanged
    }

    public function test_state2_unformatted_goes_to_state5()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 2);
        $this->initUint32($p + 0x04, 3);
        $bup = $this->alloc(0x5c);
        $this->initUint16($bup + 0x00, 1);      // Ready
        $this->initUint16($bup + 0x02, 0);      // IsFormat == 0
        $this->initUint32($p + 0x10, $bup);

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldWriteLong($p + 0x00, 5);
        $this->shouldWriteLong($p + 0x04, 3);
    }

    public function test_state2_existing_file_goes_to_state3()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 2);
        $this->initUint32($p + 0x04, 3);
        $this->initUint32($p + 0x08, 1);        // port
        $bup = $this->alloc(0x5c);
        $this->initUint16($bup + 0x00, 1);      // Ready
        $this->initUint16($bup + 0x02, 1);      // IsFormat
        $this->initUint16($bup + 0x3a, 0);      // free_user_blocks = 0 (no room)
        $this->initUint32($p + 0x10, $bup);

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_buIsExistFile')->with(1, "BUS_REPLAY")->andReturn(0);   // file exists
        $this->shouldWriteLong($p + 0x00, 3);
        $this->shouldWriteLong($p + 0x04, 3);
    }

    public function test_state2_no_file_but_room_goes_to_state3()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 2);
        $this->initUint32($p + 0x04, 3);
        $this->initUint32($p + 0x08, 1);
        $bup = $this->alloc(0x5c);
        $this->initUint16($bup + 0x00, 1);
        $this->initUint16($bup + 0x02, 1);
        $this->initUint16($bup + 0x3a, 0x1e);   // 30 free blocks (> 0x1d)
        $this->initUint32($p + 0x10, $bup);

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_buIsExistFile')->with(1, "BUS_REPLAY")
            ->andReturn(0xffffff05);            // BUD_ERR_FILE_NOT_FOUND
        $this->shouldWriteLong($p + 0x00, 3);
        $this->shouldWriteLong($p + 0x04, 3);
    }

    public function test_state2_no_file_no_room_goes_to_state6()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 2);
        $this->initUint32($p + 0x04, 3);
        $this->initUint32($p + 0x08, 1);
        $bup = $this->alloc(0x5c);
        $this->initUint16($bup + 0x00, 1);
        $this->initUint16($bup + 0x02, 1);
        $this->initUint16($bup + 0x3a, 0x1d);   // 29 free blocks (not > 0x1d)
        $this->initUint32($p + 0x10, $bup);

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_buIsExistFile')->with(1, "BUS_REPLAY")
            ->andReturn(0xffffff05);            // BUD_ERR_FILE_NOT_FOUND
        $this->shouldWriteLong($p + 0x00, 6);
        $this->shouldWriteLong($p + 0x04, 3);
    }

    /* --- state 3: capacity display / confirm / cancel --- */

    public function test_state3_a_exits_to_state8()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 3);
        $this->initUint32($p + 0x04, 3);        // selectedVmu, untouched
        $this->initUint32($p + 0x08, 1);        // port
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x4);   // PDD_DGT_TA

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldWriteLong($p + 0x00, 8);
        $this->shouldWriteLong($this->addressOf('_var_selectedVm_8c1ba34c'), 1);   // = port
        $this->shouldWriteLong($p + 0x04, 3);   // epilogue, unchanged
    }

    public function test_state3_b_mounted_returns_to_state1_and_unmounts()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 3);
        $this->initUint32($p + 0x04, 3);
        $this->initUint32($p + 0x08, 1);        // port
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x2);   // PDD_DGT_TB
        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x50, 0x1234); // Work != NULL -> unmount
        $this->initUint32($p + 0x10, $bup);

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldWriteLong($p + 0x00, 1);
        $this->shouldWriteLong($p + 0x04, 0);   // selectedVmu = 0 ...
        $this->shouldCall('_BupUnmount_8c014c46')->with(1);
        $this->shouldWriteLong($p + 0x04, 3);   // ... then overwritten by epilogue
    }

    public function test_state3_b_unmounted_returns_to_state1_no_unmount()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 3);
        $this->initUint32($p + 0x04, 3);
        $this->initUint32($p + 0x08, 1);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x2);   // PDD_DGT_TB
        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x50, 0);      // Work == NULL -> no unmount
        $this->initUint32($p + 0x10, $bup);

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldWriteLong($p + 0x00, 1);
        $this->shouldWriteLong($p + 0x04, 0);
        $this->shouldWriteLong($p + 0x04, 3);
    }

    public function test_state3_idle_prints_block_counts()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 3);
        $this->initUint32($p + 0x04, 3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);   // press = none
        $bup = $this->alloc(0x5c);
        $this->initUint16($bup + 0x3a, 0x64);   // free_user_blocks
        $this->initUint16($bup + 0x36, 0xc8);   // total_user_blocks
        $this->initUint32($p + 0x10, $bup);

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_njPrint')->variadic(1)->with(0xa000a, "%04d/%04d BLOCKS", 0x64, 0xc8);
        $this->shouldWriteLong($p + 0x04, 3);   // epilogue, unchanged
    }

    /* --- state 4: "NO SAVING OK?" confirm --- */

    public function test_state4_a_confirms_no_save_and_exits_to_state8()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 4);
        $this->initUint32($p + 0x04, 3);        // selectedVmu, untouched
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x4);   // PDD_DGT_TA

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldWriteLong($p + 0x00, 8);
        $this->shouldWriteLong($this->addressOf('_var_selectedVm_8c1ba34c'), 0xffffffff);   // -1: nothing saved
        $this->shouldCall('_njPrintC')->with(0xa000a, "NO SAVING OK?");
        $this->shouldWriteLong($p + 0x04, 3);   // epilogue, unchanged
    }

    public function test_state4_b_returns_to_state1()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 4);
        $this->initUint32($p + 0x04, 3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x2);   // PDD_DGT_TB

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldWriteLong($p + 0x00, 1);
        $this->shouldWriteLong($p + 0x04, 0);   // selectedVmu = 0 ...
        $this->shouldCall('_njPrintC')->with(0xa000a, "NO SAVING OK?");
        $this->shouldWriteLong($p + 0x04, 3);   // ... then overwritten by epilogue
    }

    public function test_state4_idle_just_prompts()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 4);
        $this->initUint32($p + 0x04, 3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);   // press = none

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_njPrintC')->with(0xa000a, "NO SAVING OK?");
        $this->shouldWriteLong($p + 0x04, 3);   // epilogue, unchanged
    }

    /* --- state 5: unformatted-card prompt --- */

    public function test_state5_confirm_returns_to_state1()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 5);
        $this->initUint32($p + 0x04, 3);        // selectedVmu
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x4);   // PDD_DGT_TA

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldWriteLong($p + 0x00, 1);
        $this->shouldWriteLong($p + 0x04, 0);   // selectedVmu = 0 ...
        $this->shouldCall('_njPrintC')->with(0xa000a, "MEMORY_CARD IS UNFORMAT");
        $this->shouldWriteLong($p + 0x04, 3);   // ... then overwritten by epilogue
    }

    public function test_state5_idle_just_prompts()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 5);
        $this->initUint32($p + 0x04, 3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);   // press = none

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_njPrintC')->with(0xa000a, "MEMORY_CARD IS UNFORMAT");
        $this->shouldWriteLong($p + 0x04, 3);   // epilogue, unchanged
    }

    /* --- state 6: not-enough-free-area prompt --- */

    public function test_state6_confirm_returns_to_state1()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 6);
        $this->initUint32($p + 0x04, 3);        // selectedVmu
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x2);   // PDD_DGT_TB

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldWriteLong($p + 0x00, 1);
        $this->shouldWriteLong($p + 0x04, 0);   // selectedVmu = 0 ...
        $this->shouldCall('_njPrintC')->with(0x2000a, "MEMORY_CARD IS NOT ENOUGH FREE AREA");
        $this->shouldWriteLong($p + 0x04, 3);   // ... then overwritten by epilogue
    }

    public function test_state6_idle_just_prompts()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 6);
        $this->initUint32($p + 0x04, 3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);   // press = none

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_njPrintC')->with(0x2000a, "MEMORY_CARD IS NOT ENOUGH FREE AREA");
        $this->shouldWriteLong($p + 0x04, 3);   // epilogue, unchanged
    }

    /* --- state 7: no VMU connected --- */

    public function test_state7_no_card_prints_message()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 7);
        $this->initUint32($p + 0x04, 3);        // selectedVmu

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_VmMenuUpdateVmusStatus_8c019550')
            ->with($this->addressOf('_init_replaySaveNames_8c044294'), 0x1e)->andReturn(0);
        $this->shouldCall('_njPrintC')->with(0xa000a, "NO_MEMORY_CARD");
        $this->shouldWriteLong($p + 0x04, 3);   // epilogue, unchanged
    }

    public function test_state7_card_found_goes_to_state1()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 7);
        $this->initUint32($p + 0x04, 3);

        $this->call('_saveMenuTask_8c01628c')->with(0, $p);

        $this->shouldCall('_VmMenuUpdateVmusStatus_8c019550')
            ->with($this->addressOf('_init_replaySaveNames_8c044294'), 0x1e)->andReturn(1);
        $this->shouldWriteLong($p + 0x00, 1);
        $this->shouldWriteLong($p + 0x04, 0);   // selectedVmu = 0 ...
        $this->shouldWriteLong($p + 0x04, 3);   // ... then overwritten by epilogue
    }

    /* --- state 8: exit the save flow, frees the task --- */

    public function test_state8_frees_task_and_returns_no_epilogue()
    {
        $this->setupExterns();
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, 8);
        $this->initUint32($p + 0x04, 3);        // selectedVmu; must NOT be written back

        $this->call('_saveMenuTask_8c01628c')->with(0x1234, $p);   // r4 = task

        $this->shouldCall('_TaskFree_8c014b66')->with(0x1234);
        $this->shouldCall('_DebugMenuOpen_8c01673a');
    }

    /* Zero all 9 VMU-status slots, then set the given present ones. External
     * memory isn't zeroed by default, so every slot the loop reads must be set. */
    private function initVmuStatus(array $present): void
    {
        $base = $this->addressOf('_var_vmuStatus_8c226048');
        for ($i = 0; $i < 9; $i++) {
            $this->initUint32($base + $i * 4, $present[$i] ?? 0);
        }
    }

    private function setupExterns(): void
    {
        /* init_replaySaveNames_8c044294 is this unit's own data (defined in both objects); addressOf
         * resolves it directly -- do NOT setSize a defined symbol. */
        $this->setSize('_VmMenuUpdateVmusStatus_8c019550', 4);
        $this->setSize('_var_peripherals_8c1ba35c', 0x40);
        $this->setSize('_var_selectedVm_8c1ba34c', 4);
        $this->setSize('_var_vmuStatus_8c226048', 0x28);   // 9 ints, indexed [0..8]
        $this->setSize('_njPrint', 4);
        $this->setSize('_njPrintC', 4);
        $this->setSize('_njPrintD', 4);
        $this->setSize('_BupMount_8c014c00', 4);
        $this->setSize('_BupUnmount_8c014c46', 4);
        $this->setSize('_BupGetInfo_8c014bba', 4);
        $this->setSize('_TaskFree_8c014b66', 4);
        $this->setSize('_buIsExistFile', 4);
        /* DebugMenuOpen_8c01673a is defined in both objects now -- addressOf resolves it
         * directly, do NOT setSize a defined symbol. */
    }
};
