<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * DebugMenuDemoRecordTask_8c01677e: record counterpart of DemoInputTask_8c016bf4 (tests/016bf4_demo_input).
 * Appends peripheral slot 0's current state to the replay buffer at
 * _var_demoCursor_8c225fa8, advancing it by one 8-byte ReplayInput record.
 *
 * Guard (both must hold, else no-op):
 *   _var_8c1bbc84 > 0                                           (still recording)
 *   cursor < &_var_demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY]  (still inside buffer)
 */
return new class extends TestCase {
    const REPLAY_BUFFER_CAPACITY = 54000;
    const REPLAY_BUFFER_SIZE = self::REPLAY_BUFFER_CAPACITY * 8;

    public function test_appends_one_record(): void
    {
        $this->resolveSymbols();

        $peripheral = $this->addressOf('_var_peripherals_8c1ba35c');
        $record = $this->addressOf('_var_demoBuffer_8c1bc828');

        $this->initUint32($this->addressOf('_var_demoCursor_8c225fa8'), $record);
        $this->initUint32($this->addressOf('_var_8c1bbc84'), 1);

        $this->initUint32($peripheral + 0x08, 0x5);    // on
        $this->initUint16($peripheral + 0x1c, 0x12);   // x1
        $this->initUint16($peripheral + 0x18, 0x34);   // r
        $this->initUint16($peripheral + 0x1a, 0x56);   // l

        $this->call('_DebugMenuDemoRecordTask_8c01677e')->with(0, 0);

        $this->shouldWriteLong($record + 0x00, 1);         // on -> boolean
        $this->shouldWriteByte($record + 0x04, 0x12);      // x1
        $this->shouldWriteByte($record + 0x05, 0x34);      // r
        $this->shouldWriteByte($record + 0x06, 0x56);      // l
        $this->shouldWriteLongTo('_var_demoCursor_8c225fa8', $record + 8);
    }

    public function test_noop_when_count_zero(): void
    {
        $this->resolveSymbols();

        $this->initUint32(
            $this->addressOf('_var_demoCursor_8c225fa8'),
            $this->addressOf('_var_demoBuffer_8c1bc828')
        );
        $this->initUint32($this->addressOf('_var_8c1bbc84'), 0);

        $this->call('_DebugMenuDemoRecordTask_8c01677e')->with(0, 0);
    }

    public function test_noop_when_cursor_at_end(): void
    {
        $this->resolveSymbols();

        $end = $this->addressOf('_var_demoBuffer_8c1bc828') + self::REPLAY_BUFFER_SIZE;

        $this->initUint32($this->addressOf('_var_demoCursor_8c225fa8'), $end);
        $this->initUint32($this->addressOf('_var_8c1bbc84'), 1);

        $this->call('_DebugMenuDemoRecordTask_8c01677e')->with(0, 0);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34);
        $this->setSize('_var_8c1bbc84', 4);
        /*
         * In the real ROM, var_demoCursor_8c225fa8 (address 0x8c225fa8) is placed
         * immediately after var_demoBuffer_8c1bc828's 432000-byte extent (address
         * 0x8c1bc828 + 432000 = 0x8c225fa8) -- the archived asm's end-of-buffer
         * bound check literally reuses &var_demoCursor_8c225fa8 as that coincidence
         * rather than an explicit addend (see 01614c_debug_menu.src's LP_GEN_94139 table).
         * Allocate them adjacently here, in the same order, to reproduce it.
         */
        $this->setSize('_var_demoBuffer_8c1bc828', self::REPLAY_BUFFER_SIZE);
        $this->setSize('_var_demoCursor_8c225fa8', 4);
    }
};
