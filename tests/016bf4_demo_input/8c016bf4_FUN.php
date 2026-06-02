<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _demoInputTask_8c016bf4: pops one replay record from the queue at _demoCursor_8c225fa8 and
 * folds it into peripheral slot 0 (_var_peripherals_8c1ba35c[0]).
 *
 * Guard (both must hold, else no-op):
 *   _var_8c1bbc84 > 0                          (records remaining)
 *   cursor < &_demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY]  (still inside buffer)
 *
 * Record layout (8 bytes, advanced by 8 each call):
 *   [0..3] on (Uint32)   [4] x1 src (char)   [5] r src (byte)   [6] l src (byte)
 */
return new class extends TestCase {
    const REPLAY_BUFFER_CAPACITY = 54000;
    const REPLAY_BUFFER_SIZE = self::REPLAY_BUFFER_CAPACITY * 8;

    public function test_pops_one_record(): void
    {
        $this->resolveSymbols();

        $buf = $this->addressOf('_var_peripherals_8c1ba35c');
        $record = $this->addressOf('_demoBuffer_8c1bc828');

        $this->initUint32($this->addressOf('_demoCursor_8c225fa8'), $record); // cursor at start
        $this->initUint32($this->addressOf('_var_8c1bbc84'), 1);  // records remaining > 0
        $this->initUint32($this->addressOf('_demoPrevOn_8c225fac'), 0);  // previous "on" (for press calc)

        $this->initUint32($record + 0, 0x5);      // on raw
        $this->initUint8($record + 4, 0x12);      // x1 src
        $this->initUint8($record + 5, 0x34);      // r src
        $this->initUint8($record + 6, 0x56);      // l src

        $this->call('_demoInputTask_8c016bf4');

        // press = on & (oldOn ^ on) = 5 & (0 ^ 5) = 5
        $this->shouldWriteLong($buf + 0x08, 0x5);          // on
        $this->shouldWriteLong($buf + 0x10, 0x5);          // press
        $this->shouldWriteLongTo('_demoPrevOn_8c225fac', 0x5);    // save on as previous
        $this->shouldWriteWord($buf + 0x1c, 0x12);         // x1 = (short)(char)src
        $this->shouldWriteWord($buf + 0x18, 0x34);         // r
        $this->shouldWriteWord($buf + 0x1a, 0x56);         // l
        $this->shouldWriteLongTo('_demoCursor_8c225fa8', $record + 8); // advance cursor
    }

    public function test_noop_when_count_zero(): void
    {
        $this->resolveSymbols();

        $this->initUint32(
            $this->addressOf('_demoCursor_8c225fa8'),
            $this->addressOf('_demoBuffer_8c1bc828')
        );
        $this->initUint32($this->addressOf('_var_8c1bbc84'), 0);  // no records remaining

        $this->call('_demoInputTask_8c016bf4');

        // First guard fails: function returns without touching peripherals,
        // oldOn, or the cursor.
    }

    public function test_noop_when_cursor_at_end(): void
    {
        $this->resolveSymbols();

        $end = $this->addressOf('_demoBuffer_8c1bc828') + self::REPLAY_BUFFER_SIZE;

        $this->initUint32($this->addressOf('_demoCursor_8c225fa8'), $end);  // cursor == end
        $this->initUint32($this->addressOf('_var_8c1bbc84'), 1);     // records remaining > 0

        $this->call('_demoInputTask_8c016bf4');

        // Second guard fails (cursor >= buffer end): no writes.
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_8c1bbc84', 4);
        $this->setSize('_demoPrevOn_8c225fac', 4);
        $this->setSize('_demoCursor_8c225fa8', 4);
        $this->setSize('_demoBuffer_8c1bc828', self::REPLAY_BUFFER_SIZE);
    }
};
