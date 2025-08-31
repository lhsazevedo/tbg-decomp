<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!defined('PDD_DGT_KR')) {
    define('PDD_DGT_KR', (1 <<  7));
    define('PDD_DGT_KL', (1 <<  6));
    define('PDD_DGT_KD', (1 <<  5));
    define('PDD_DGT_KU', (1 <<  4));
    define('PDD_DGT_TA', (1 <<  2));
};

return new class extends TestCase {
    public function test_case0_waitForTimer()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 3); // timer

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 4);


        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case0_advancesToCase1()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 4); // timer

        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048') + 0x00, 0);
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048') + 0x04, 4);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 5);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3)
            ->andReturn(1);

        $this->shouldWriteMenuState(1);

        $this->shouldCall('_initCursorLerp_19788')->with(1);

        // TODO: Move implementation to Simulator
        // TODO: Handle calling conventions for expectations in Simulator
        $menuState = $this->addressOf('_menuState_8c1bc7a8');
        $mvn = function () use ($menuState) {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            if (!$src->equals($menuState + 0x28)) {
                throw new \Exception('Unexpected move source ' . $this->registers[2]->readable());
            }

            if (!$dst->equals($menuState + 0x20)) {
                throw new \Exception('Unexpected move dest ' . $this->registers[1]->readable());
            }

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8($dst->value + $i, $this->readUInt8($src->value + $i));
            }
        };

        $this->shouldCall('__quick_evn_mvn')->do($mvn);
        $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with(0xcafe0004);
        $this->shouldCall('_FUN_8c010d8a');
        $this->shouldCall('_snd_8c010cd6');

        $this->shouldCall('_push_fadein_8c022a9c')->with(10);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case0_advancesToCase6()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $this->resolveSymbols();

        $state = $this->alloc(8 * 4);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 4); // timer
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048') + 0x00, 0);
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048') + 0x04, 4);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 5);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3)
            ->andReturn(0);

        // Advance state
        $this->shouldWriteMenuState(6);
        $this->shouldWriteLong($state + 0x08, 0);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x3c, 0);

        $this->shouldCall('_push_fadein_8c022a9c')->with(10);

        $this->singleCall($this->entryName())->with($state, 0)->run();
    }

    public function test_case1_waitsForFadeIn()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $this->resolveSymbols();

        $state = $this->alloc(8 * 4);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 1); // state
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldWriteSelectedSlot(0);

        $this->singleCall($this->entryName())->with($state, 0)->run();
    }

    public function test_case1_advancesAfterFade()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $this->resolveSymbols();

        $state = $this->alloc(8 * 4);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 1); // state
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $this->shouldWriteMenuState(2);
        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldWriteSelectedSlot(0);

        $this->singleCall($this->entryName())->with($state, 0)->run();
    }

    public function test_case2_startsInAnAvailableSlot()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2); // state
        // Init slots
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048') + 0 * 4, 0); // Unavailable
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048') + 1 * 4, 0); // Unavailable
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048') + 2 * 4, 4); // Available

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3);

        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 3, 0);
        $this->shouldCall('_initCursorLerp_19788')->with(2);
        $this->shouldWriteMenuState(3);
        $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with(0xcafe0004);

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0x20);
        $this->shouldWriteSelectedSlot(2);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case2_moveRightOnUpperRow()
    {
        $this->runSelectTest(
            slots: [0, 4, 0, 4],
            initialSlot: 1,
            expectedSlot: 3,
            press: PDD_DGT_KR
        );
    }

    public function test_case2_loopRightOnUpperRow()
    {
        $this->runSelectTest(
            slots: [0, 4, 0, 4],
            initialSlot: 3,
            expectedSlot: 1,
            press: PDD_DGT_KR
        );
    }

    public function test_case2_doNotMoveRightWhenSingleSlotOnUpperRow()
    {
        $this->runSelectTest(
            slots: [0, 4, 0, 0],
            initialSlot: 1,
            expectedSlot: 1,
            press: PDD_DGT_KR
        );
    }

    public function test_case2_moveLeftOnUpperRow()
    {
        $this->runSelectTest(
            slots: [0, 4, 0, 4],
            initialSlot: 3,
            expectedSlot: 1,
            press: PDD_DGT_KL
        );
    }

    public function test_case2_loopLeftOnUpperRow()
    {
        $this->runSelectTest(
            slots: [0, 4, 0, 4],
            initialSlot: 1,
            expectedSlot: 3,
            press: PDD_DGT_KL
        );
    }

    public function test_case2_doNotMoveLeftWhenSingleSlotOnUpperRow()
    {
        $this->runSelectTest(
            slots: [0, 4, 0, 0],
            initialSlot: 1,
            expectedSlot: 1,
            press: PDD_DGT_KL
        );
    }

    public function test_case2_moveDownOnUpperRow()
    {
        $this->runSelectTest(
            slots: [
                0, 4, 0, 4,
                0, 4, 0, 4, 3
            ],
            initialSlot: 1,
            expectedSlot: 5,
            press: PDD_DGT_KD
        );
    }

    public function test_case2_preservesColumnWhenMovingDownOnUpperRow()
    {
        $this->runSelectTest(
            slots: [
                0, 4, 0, 4,
                0, 4, 0, 4, 3
            ],
            initialSlot: 3,
            expectedSlot: 7,
            press: PDD_DGT_KD
        );
    }

    public function test_case2_moveRightOnLowerRow()
    {
        $this->runSelectTest(
            slots: [
                0, 4, 0, 4,
                0, 4, 0, 4, 3
            ],
            initialSlot: 5,
            expectedSlot: 7,
            press: PDD_DGT_KR
        );
    }

    public function test_case2_loopRightOnLowerRow()
    {
        $this->runSelectTest(
            slots: [0, 4, 0, 4],
            initialSlot: 3,
            expectedSlot: 1,
            press: PDD_DGT_KR
        );
    }

    public function test_case2_doNotMoveRightWhenSingleSlotOnLowerRow()
    {
        $this->runSelectTest(
            slots: [
                0, 4, 0, 0,
                0, 0, 0, 0, 3,
            ],
            initialSlot: 8,
            expectedSlot: 8,
            press: PDD_DGT_KR
        );
    }

    public function test_case2_moveLeftOnLowerRow()
    {
        $this->runSelectTest(
            slots: [
                0, 4, 0, 4,
                0, 4, 0, 4, 3
            ],
            initialSlot: 7,
            expectedSlot: 5,
            press: PDD_DGT_KL
        );
    }

    public function test_case2_loopLeftOnLowerRow()
    {
        $this->runSelectTest(
            slots: [
                0, 4, 0, 4,
                0, 4, 0, 4, 3
            ],
            initialSlot: 5,
            expectedSlot: 8,
            press: PDD_DGT_KL
        );
    }

    public function test_case2_doNotMoveLeftWhenSingleSlotOnLowerRow()
    {
        $this->runSelectTest(
            slots: [
                0, 4, 0, 0,
                0, 0, 0, 0, 3,
            ],
            initialSlot: 8,
            expectedSlot: 8,
            press: PDD_DGT_KL
        );
    }

    public function test_case2_moveUpOnLowerRow()
    {
        $this->runSelectTest(
            slots: [
                0, 4, 0, 0,
                0, 4, 0, 0, 3,
            ],
            initialSlot: 5,
            expectedSlot: 1,
            press: PDD_DGT_KU
        );
    }

    public function test_case2_preservesColumnWhenMovingUpOnLowerRow()
    {
        $this->runSelectTest(
            slots: [
                0, 4, 0, 4,
                0, 4, 0, 4, 3,
            ],
            initialSlot: 7,
            expectedSlot: 3,
            press: PDD_DGT_KU
        );
    }

    public function test_case2_findsLastUpperSlotWhenWhenMovingUpOnLowerRow()
    {
        $this->runSelectTest(
            slots: [
                0, 4, 0, 0,
                0, 4, 0, 4, 3,
            ],
            initialSlot: 7,
            expectedSlot: 1,
            press: PDD_DGT_KU
        );
    }

    public function test_case2_showConfirmMessageWhenSelectingAvailableSlot()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, PDD_DGT_TA);
        // Init slots
        $this->initUint32Array($this->addressOf('_var_vmuStatus_8c226048'), [
            0, 4, 0, 0,
            0, 0, 0, 0, 3,
        ]);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3);

        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 0, 0);
        $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with("よろしいですか？");
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x6c, 1);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(1)->andReturn(0x5a5a5a5a);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x78, 0x5a5a5a5a);
        $this->shouldWriteMenuState(4);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x3c, 0);

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0x20);
        $this->shouldWriteSelectedSlot(1);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case2_showConfirmMessageWhenSelectingExistingSave()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, PDD_DGT_TA);
        // Init slots
        $this->initUint32Array($this->addressOf('_var_vmuStatus_8c226048'), [
            0, 5, 0, 0,
            0, 0, 0, 0, 3,
        ]);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3);

        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 0, 0);
        $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with("よろしいですか？");
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x6c, 1);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(1)->andReturn(0x5a5a5a5a);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x78, 0x5a5a5a5a);
        $this->shouldWriteMenuState(4);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x3c, 0);

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0x20);
        $this->shouldWriteSelectedSlot(1);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case2_showConfirmMessageWhenSelectingExistingSaveB()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, PDD_DGT_TA);
        // Init slots
        $this->initUint32Array($this->addressOf('_var_vmuStatus_8c226048'), [
            0, 6, 0, 0,
            0, 0, 0, 0, 3,
        ]);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3);

        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 0, 0);
        $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with("よろしいですか？");
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x6c, 1);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(1)->andReturn(0x5a5a5a5a);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x78, 0x5a5a5a5a);
        $this->shouldWriteMenuState(4);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x3c, 0);

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0x20);
        $this->shouldWriteSelectedSlot(1);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case2_showConfirmMessageWhenProceedingWithoutSaving()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 8); // slot
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, PDD_DGT_TA);
        // Init slots
        $this->initUint32Array($this->addressOf('_var_vmuStatus_8c226048'), [
            0, 4, 0, 0,
            0, 0, 0, 0, 3,
        ]);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3);

        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 0, 0);
        $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with("ファイルを設定しないとセーブできません<E>このままゲームを開始してもよろしいですか？");
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x3c, 0);

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0x20);
        $this->shouldWriteSelectedSlot(8);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case2_playFailSoundOnInvalidOption()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, PDD_DGT_TA);
        // Init slots
        $this->initUint32Array($this->addressOf('_var_vmuStatus_8c226048'), [
            0, 2, 0, 0,
            0, 0, 0, 0, 3,
        ]);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3);

        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 2, 0);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x3c, 0);

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0x20);
        $this->shouldWriteSelectedSlot(1);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case2_MoveUpFromNinithSlot()
    {
        $this->runSelectTest(
            slots: [
                4, 0, 4, 0,
                0, 4, 0, 0, 3,
            ],
            initialSlot: 8,
            expectedSlot: 2,
            press: PDD_DGT_KU
        );
    }

    public function test_case3_waitsForInterpolation()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 3); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot

        $this->shouldCall('_interpolateCursor_8c016d2c')->andReturn(0);

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0x20);
        $this->shouldWriteSelectedSlot(1);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case3_advancesWhenInterpolated()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 3); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        // Init slots
        $this->initUint32Array($this->addressOf('_var_vmuStatus_8c226048'), [
            0, 4, 0, 0,
            0, 0, 0, 0, 3,
        ]);

        $this->shouldCall('_interpolateCursor_8c016d2c')->andReturn(1);

        $this->shouldWriteMenuState(2);
        $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with(0xcafe0004);

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0x20);
        $this->shouldWriteSelectedSlot(1);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case4_waitsForUserInput()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 4); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        // Init slots
        $this->initUint32Array($this->addressOf('_var_vmuStatus_8c226048'), [
            0, 4, 0, 0,
            0, 0, 0, 0, 3,
        ]);

        $this->shouldCall('_promptHandleBinary_16caa')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x3c)
            ->andReturn(0);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x00,
            2,
            228.0,
            304.0,
            -5.0,
        );

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0xff);
        $this->shouldWriteSelectedSlot(1);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case4_advancesOnOk()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 4); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3); // slot
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x6c, 3); // slot
        // Init slots
        $this->initUint32Array($this->addressOf('_var_vmuStatus_8c226048'), [
            0, 4, 0, 0,
            0, 0, 0, 0, 3,
        ]);

        $this->shouldCall('_promptHandleBinary_16caa')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x3c)
            ->do(function ($params) {
                $this->memory->writeUInt32($params[0], U32::of(1));
            })
            ->andReturn(1);

        $this->shouldWriteLongTo('_var_selectedVm_8c1ba34c', 3);
        $this->shouldWriteMenuState(8);
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x00,
            3,
            228.0,
            304.0,
            -5.0,
        );

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0xff);
        $this->shouldWriteSelectedSlot(3);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case4_goesBackOnCancel()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 4); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3); // slot
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x6c, 3); // slot
        // Init slots
        $this->initUint32Array($this->addressOf('_var_vmuStatus_8c226048'), [
            0, 4, 0, 4,
            0, 0, 0, 0, 3,
        ]);

        $this->shouldCall('_promptHandleBinary_16caa')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x3c)
            ->do(function ($params) {
                $this->memory->writeUInt32($params[0], U32::of(2));
            })
            ->andReturn(2);

        $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with(0xcafe0004);
        $this->shouldWriteMenuState(2);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x00,
            4,
            228.0,
            304.0,
            -5.0,
        );

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0xff);
        $this->shouldWriteSelectedSlot(3);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case5_waitsForUserInput()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 8); // slot
        // Init slots
        $this->initUint32Array($this->addressOf('_var_vmuStatus_8c226048'), [
            0, 4, 0, 0,
            0, 0, 0, 0, 3,
        ]);

        $this->shouldCall('_promptHandleBinary_16caa')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x3c)
            ->andReturn(0);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x00,
            2,
            228.0,
            304.0,
            -5.0,
        );

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0xff);
        $this->shouldWriteSelectedSlot(8);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case5_advancesOnOk()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 8); // slot
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x6c, 8); // slot
        // Init slots
        $this->initUint32Array($this->addressOf('_var_vmuStatus_8c226048'), [
            0, 4, 0, 0,
            0, 0, 0, 0, 3,
        ]);

        $this->shouldCall('_promptHandleBinary_16caa')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x3c)
            ->do(function ($params) {
                $this->memory->writeUInt32($params[0], U32::of(1));
            })
            ->andReturn(1);

        $this->shouldWriteLongTo('_var_selectedVm_8c1ba34c', -1);
        $this->shouldCall('_FUN_8c01895e');
        $this->shouldWriteMenuState(9);
        $this->shouldCall('_FUN_8c010bae')->with(0);
        $this->shouldCall('_FUN_8c010bae')->with(1);
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x00,
            3,
            228.0,
            304.0,
            -5.0,
        );

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0xff);
        $this->shouldWriteSelectedSlot(8);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case5_goesBackOnCancel()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 8); // slot
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x6c, 8); // slot
        // Init slots
        $this->initUint32Array($this->addressOf('_var_vmuStatus_8c226048'), [
            0, 4, 0, 4,
            0, 0, 0, 0, 3,
        ]);

        $this->shouldCall('_promptHandleBinary_16caa')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x3c)
            ->do(function ($params) {
                $this->memory->writeUInt32($params[0], U32::of(2));
            })
            ->andReturn(2);

        $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with(0xcafe0003);
        $this->shouldWriteMenuState(2);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x00,
            4,
            228.0,
            304.0,
            -5.0,
        );

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0xff);
        $this->shouldWriteSelectedSlot(8);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case6_waitsForFadeIn()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 6); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_DrawVmWarning_19852');
        $this->shouldWriteSelectedSlot(1);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case6_advancesAfterFade()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 6); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $this->shouldWriteMenuState(7);
        $this->shouldCall('_DrawVmWarning_19852');
        $this->shouldWriteSelectedSlot(1);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case7_subcase0_waitsForPlayerInput()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 7); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $task = $this->alloc(0x0c);
        $this->initUint32($task + 0x08, 0); // substate

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3)
            ->andReturn(0);

        $this->shouldCall('_promptHandleBinary_16caa')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x3c)
            ->andReturn(0);

        $this->shouldCall('_DrawVmWarning_19852');

        $this->singleCall($this->entryName())->with($task, 0)->run();
    }

    public function test_case7_subcase0_advancesOnOk()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 7); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $task = $this->alloc(0x0c);
        $this->initUint32($task + 0x08, 0); // substate

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3)
            ->andReturn(0);

        $this->shouldCall('_promptHandleBinary_16caa')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x3c)
            ->andReturn(1);

        $this->shouldWriteLongTo('_var_selectedVm_8c1ba34c', -1);
        $this->shouldCall('_FUN_8c01895e');
        $this->shouldWrite($task + 0x08, 2); // substate
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);

        $this->shouldCall('_DrawVmWarning_19852');

        $this->singleCall($this->entryName())->with($task, 0)->run();
    }

    public function test_case7_subcase0_goesBackOnCancel()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 7); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $task = $this->alloc(0x0c);
        $this->initUint32($task + 0x08, 0); // substate
        // Init slots
        $this->initUint32Array($this->addressOf('_var_vmuStatus_8c226048'), [
            0, 0, 0, 0,
            0, 0, 4, 0, 3,
        ]);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3)
            ->andReturn(0);

        $this->shouldCall('_promptHandleBinary_16caa')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x3c)
            ->andReturn(2);

        $this->shouldCall('_initCursorLerp_19788')->with(6);
        $menuState = $this->addressOf('_menuState_8c1bc7a8');
        $mvn = function () use ($menuState) {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            if (!$src->equals($menuState + 0x28)) {
                throw new \Exception('Unexpected move source ' . $this->registers[2]->readable());
            }

            if (!$dst->equals($menuState + 0x20)) {
                throw new \Exception('Unexpected move dest ' . $this->registers[1]->readable());
            }

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8($dst->value + $i, $this->readUInt8($src->value + $i));
            }
        };
        $this->shouldCall('__quick_evn_mvn')->do($mvn);

        $this->shouldCall('_swapMessageBoxFor_8c02aefc', 0xcafe0006);
        $this->shouldWrite($task + 0x08, 3); // substate
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);

        $this->shouldCall('_DrawVmWarning_19852');

        $this->singleCall($this->entryName())->with($task, 0)->run();
    }

    public function test_case7_subcase0_else()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 7); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $task = $this->alloc(0x0c);
        $this->initUint32($task + 0x08, 0); // substate

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3)
            ->andReturn(1);
        $this->shouldWriteLong($task + 0x08, 1); // substate
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);

        $this->shouldCall('_DrawVmWarning_19852');

        $this->singleCall($this->entryName())->with($task, 0)->run();
    }

    public function test_case7_subcase1_waitsForFade()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 7); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $task = $this->alloc(0x0c);
        $this->initUint32($task + 0x08, 1); // substate
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_DrawVmWarning_19852');

        $this->singleCall($this->entryName())->with($task, 0)->run();
    }

    public function test_case7_subcase1_advancesAfterFade()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 7); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $task = $this->alloc(0x0c);
        $this->initUint32($task + 0x08, 1); // substate
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 10);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0);

        $this->singleCall($this->entryName())->with($task, 0)->run();
    }

    public function test_case7_subcase2_waitsForFade()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 7); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $task = $this->alloc(0x0c);
        $this->initUint32($task + 0x08, 2); // substate
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_DrawVmWarning_19852');

        $this->singleCall($this->entryName())->with($task, 0)->run();
    }

    public function test_case7_subcase2_advancedAfterFade()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 7); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $task = $this->alloc(0x0c);
        $this->initUint32($task + 0x08, 2); // substate
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $this->shouldCall('_MainMenuSwitchFromTask_8c01a09a')->with($task);

        $this->singleCall($this->entryName())->with($task, 0)->run();
    }

    public function test_case7_subcase3_waitsForFade()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 7); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $task = $this->alloc(0x0c);
        $this->initUint32($task + 0x08, 3); // substate
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_DrawVmWarning_19852');

        $this->singleCall($this->entryName())->with($task, 0)->run();
    }

    public function test_case7_subcase3()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 7); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1); // slot
        $task = $this->alloc(0x0c);
        $this->initUint32($task + 0x08, 3); // substate
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 10);
        //$this->shouldCall('_MainMenuSwitchFromTask_8c01a09a');

        $this->singleCall($this->entryName())->with($task, 0)->run();
    }

    public function test_case8_waitsForFade()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 8); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3); // slot
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x3c, 0);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            2,
            228.0,
            304.0,
            -5.0
        );

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case8_advancesAfterFade()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 8); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3); // slot
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x3c, 0);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $this->shouldCall('_FUN_8c019334')->with(0xcafecafe);

        $this->singleCall($this->entryName())->with(0xcafecafe, 0)->run();
    }

    public function test_case9_waitsForFade()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 9); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3); // slot
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case9_waitsFor8c03bd80()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 9); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3); // slot
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 1);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case9_advances()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 9); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3); // slot
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0);

        $this->shouldCall('_MainMenuSwitchFromTask_8c01a09a')->with(0xcafecafe);

        $this->singleCall($this->entryName())->with(0xcafecafe, 0)->run();
    }

    public function test_case10_waitsForFade()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 10); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3); // slot
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case10_advancesAfterFade()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 10); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3); // slot
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);
        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    public function test_case_default()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 15); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3); // slot
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_menuState_8c1bc7a8', 0x6c);
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_init_saveNames_8c044d50', 4 * 11);
        $this->setSize('_isFading_8c226568', 4);
        $this->setSize('_var_vmuStatus_8c226048', 0x24);
        $this->setSize('_var_midiHandles_8c0fcd28', 7 * 4);

        // Basic inits
        $this->initUint32Array($this->addressOf('_var_midiHandles_8c0fcd28'), [
            0xbeef0000,
            0xbeef0001,
            0xbeef0002,
            0xbeef0003,
            0xbeef0004,
            0xbeef0005,
            0xbeef0006,
        ]);

        $this->initUint32Array($this->addressOf('_init_vmuStatusMessages_8c044dc4'), [
            0,
            0xcafe0001,
            0xcafe0002,
            0xcafe0003,
            0xcafe0004,
            0xcafe0005,
            0xcafe0006,
        ]);

        // Functions
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_menuTextboxText_8c02af1c', 4);
        $this->setSize('_swapMessageBoxFor_8c02aefc', 4);
        $this->setSize('_BupGetInfo_8c014bba', 4);
        $this->setSize('_drawSprite_8c014f54', 4);
        $this->setSize('_push_fadeout_8c022b60', 4);
        $this->setSize('_MainMenuSwitchFromTask_8c01a09a', 4);
    }

    private function initUint32Array(int $address, array $values): void
    {
        foreach ($values as $i => $value) {
            $this->initUint32($address + $i * 4, $value);
        }
    }

    private function shouldWriteSelectedSlot(int $slot): void
    {
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x38, $slot);
    }

    private function shouldWriteMenuState(int $state): void
    {
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, $state);
    }

    private function runSelectTest(array $slots, int $initialSlot, $expectedSlot, $press): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2); // state
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, $initialSlot);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, $press);
        // Init slots
        $this->initUint32Array($this->addressOf('_var_vmuStatus_8c226048'), $slots);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3);

        if ($expectedSlot !== $initialSlot) {
            $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 3, 0);
            $this->shouldCall('_initCursorLerp_19788')->with($expectedSlot);
            $this->shouldWriteMenuState(3);

            $messages = [
                0,
                0xcafe0001,
                0xcafe0002,
                0xcafe0003,
                0xcafe0004,
                0xcafe0005,
                0xcafe0006,
            ];
            $message = $messages[$slots[$expectedSlot]];
            $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with($message);
        }

        $this->shouldCall('_drawVmMenu_197c0');
        $this->shouldCall('_menuTextboxText_8c02af1c')->with(0x20);
        $this->shouldWriteSelectedSlot($expectedSlot);

        $this->singleCall($this->entryName())->with(0, 0)->run();
    }

    private function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }

    private function entryName(): string
    {
        return $this->isAsmObject()
            ? '_VmMenuTask_198a0'
            : '_VmMenuTask_198a0';
    }
};
