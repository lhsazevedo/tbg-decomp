<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_RenderQueueDraw_8c0223ea', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_AsqGetRandomInRangeA_8c012178', 4);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_midiHandles_8c0fcd28', 8 * 4);
        $this->setSize('_var_seats_8c228718', 31 * 4);
    }

    private function f(float $v): int
    {
        return unpack('L', pack('f', $v))[1];
    }

    /** Allocates a StopScheduleState. Returns [state, ref_0x00]. */
    private function makeState(int $mode): int
    {
        $state = $this->alloc(0x38);
        $ref = $this->alloc(4);
        $this->initUint8($ref, 5); // stop index, unused by this fn directly
        $this->initUint32($state + 0x00, $ref);
        $this->initUint32($state + 0x04, $mode);
        $this->lastRef = $ref;
        return $state;
    }

    private int $lastRef = 0;

    public function test_case1_countdown_not_elapsed_does_nothing(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_passengersFadedOut_8c22895c'), 1);

        $state = $this->makeState(1);
        $this->initUint32($state + 0x20, 3); // countdown, decrements to 2

        $this->call('_PassengerBoardTask_8c02d21c')->with(0, $state);

        $this->shouldWriteLong($state + 0x20, 2);
        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 1);
    }

    public function test_case1_flag_clear_does_nothing(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_passengersFadedOut_8c22895c'), 0);

        $state = $this->makeState(1);
        $this->initUint32($state + 0x20, 0);

        $this->call('_PassengerBoardTask_8c02d21c')->with(0, $state);

        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 1);
    }

    public function test_case1_countdown_elapsed_positions_and_plays_sound_non_ome(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_passengersFadedOut_8c22895c'), 1);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0); // SHINJUKU

        $anchor = $this->addressOf('_var_boardSpot1_8c228928');
        $this->initUint32($anchor + 0x0, $this->f(1.0));
        $this->initUint32($anchor + 0x4, $this->f(2.0));
        $this->initUint32($anchor + 0x8, $this->f(3.0));

        $midi = $this->addressOf('_var_midiHandles_8c0fcd28');
        $this->initUint32($midi + 2 * 4, 0x1234); // quadrant 2's handle

        $state = $this->makeState(1);
        $this->initUint32($state + 0x20, 0); // decrements to -1 -> elapsed
        $this->initUint32($state + 0x18, $this->f(0.1)); // random offset x
        $this->initUint32($state + 0x1c, $this->f(0.2)); // random offset z
        $this->initUint32($state + 0x2c, 2); // quadrant
        $this->initUint32($state + 0x30, 42); // sound id

        $this->call('_PassengerBoardTask_8c02d21c')->with(0, $state);

        $this->shouldWriteLong($state + 0x20, 0xffffffff);
        $this->shouldWriteFloat($state + 0x08, 1.1);
        $this->shouldWriteFloat($state + 0x0c, 2.0);
        $this->shouldWriteFloat($state + 0x10, 3.2);
        $this->shouldWriteLong($state + 0x14, 0x10);
        $this->shouldWriteLong($state + 0x04, 2);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 42, 0);
        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 1);
    }

    public function test_case2_flag_clear_registers_layer1(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_passengersFadedOut_8c22895c'), 0);

        $state = $this->makeState(2);

        $this->call('_PassengerBoardTask_8c02d21c')->with(0, $state);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(1, $this->addressOf('_drawPassengerSprite_8c02d19c'), $state);
        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 1);
    }

    public function test_case4_claims_empty_slot_below_20(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_passengersFadedOut_8c22895c'), 1);

        $slots = $this->addressOf('_var_seats_8c228718');
        for ($i = 0; $i < 31; $i++) {
            $this->initUint32($slots + $i * 4, $i === 3 ? -1 : 0xbeef);
        }

        // init_seatPositions_8c04c3e4 is defined by this object itself; overwrite the slot
        // this test exercises with known values rather than setSize+init.
        $init = $this->addressOf('_init_seatPositions_8c04c3e4');
        $this->initUint32($init + 3 * 8 + 0, $this->f(0.5));
        $this->initUint32($init + 3 * 8 + 4, $this->f(1.5));

        $state = $this->makeState(4);
        $this->initUint32($state + 0x0c, $this->f(9.0));
        $ref = $this->lastRef; // state->ref_0x00

        $this->call('_PassengerBoardTask_8c02d21c')->with(0, $state);

        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(31)->andReturn(3);
        $this->shouldWriteLong($slots + 3 * 4, $ref);
        $this->shouldWriteFloat($state + 0x08, 0.5);
        $this->shouldWriteFloat($state + 0x10, 1.5);
        $this->shouldWriteLong($state + 0x14, 0x20);
        $this->shouldWriteFloat($state + 0x0c, 9.0 - 0.18000000715255737);
        $this->shouldWriteLong($state + 0x04, 5);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(2, $this->addressOf('_drawPassengerSprite_8c02d19c'), $state);
        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 1);
    }

    public function test_case5_still_fading_registers_layer2(): void
    {
        $this->resolveSymbols();

        $mat = $this->addressOf('_var_passengerFadeColor_8c228960');
        $this->initUint32($mat, $this->f(0.5));

        $state = $this->makeState(5);

        $this->call('_PassengerBoardTask_8c02d21c')->with(0, $state);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(2, $this->addressOf('_drawPassengerSprite_8c02d19c'), $state);
        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 1);
    }

    public function test_case5_fully_faded_resets_to_scripted_stop(): void
    {
        $this->resolveSymbols();

        $mat = $this->addressOf('_var_passengerFadeColor_8c228960');
        $this->initUint32($mat, $this->f(1.0));

        $state = $this->makeState(5);

        $this->call('_PassengerBoardTask_8c02d21c')->with(0, $state);

        $this->shouldWriteLong($state + 0x04, 0);
        $this->shouldWriteLong($state + 0x28, 1);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(2, $this->addressOf('_drawPassengerSprite_8c02d19c'), $state);
        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 1);
    }

    public function test_case0_registers_layer2_and_returns_without_flag(): void
    {
        $this->resolveSymbols();

        $state = $this->makeState(0);

        $this->call('_PassengerBoardTask_8c02d21c')->with(0, $state);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(2, $this->addressOf('_drawPassengerSprite_8c02d19c'), $state);
    }

    public function test_default_does_nothing_but_set_flag(): void
    {
        $this->resolveSymbols();

        $state = $this->makeState(99);

        $this->call('_PassengerBoardTask_8c02d21c')->with(0, $state);

        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 1);
    }
};
