<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_RenderQueueDraw_8c0223ea', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_AsqGetRandomInRangeB_8c0121be', 4);
        $this->setSize('_TaskKill_8c014b66', 4);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_midiHandles_8c0fcd28', 8 * 4);
        $this->setSize('_var_seats_8c228718', 31 * 4);
    }

    private function f(float $v): int
    {
        return unpack('L', pack('f', $v))[1];
    }

    private function makeState(int $mode): int
    {
        $state = $this->alloc(0x38);
        $this->initUint32($state + 0x04, $mode);
        return $state;
    }

    public function test_case0_countdown_not_elapsed_does_nothing(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_passengersFadedOut_8c22895c'), 1);

        $state = $this->makeState(0);
        $this->initUint32($state + 0x20, 3);

        $this->call('_PassengerExitTask_8c02d46c')->with(0, $state);

        $this->shouldWriteLong($state + 0x20, 2);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(2, $this->addressOf('_drawPassengerSprite_8c02d19c'), $state);
        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 1);
    }

    public function test_case0_countdown_elapsed_advances_to_case6(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_passengersFadedOut_8c22895c'), 1);

        $state = $this->makeState(0);
        $this->initUint32($state + 0x20, 0);

        $this->call('_PassengerExitTask_8c02d46c')->with(0, $state);

        $this->shouldWriteLong($state + 0x20, 0xffffffff);
        $this->shouldWriteLong($state + 0x04, 6);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(2, $this->addressOf('_drawPassengerSprite_8c02d19c'), $state);
        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 1);
    }

    public function test_case6_flag_clear_only_registers_layer2(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_passengersFadedOut_8c22895c'), 0);

        $mat = $this->addressOf('_var_passengerFadeColor_8c228960');
        $this->initUint32($mat, $this->f(0.5)); // < 1.0, so field_0x28 untouched

        $state = $this->makeState(6);

        $this->call('_PassengerExitTask_8c02d46c')->with(0, $state);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(2, $this->addressOf('_drawPassengerSprite_8c02d19c'), $state);
        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 1);
    }

    public function test_case6_active_positions_and_plays_sound_on_ome(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_passengersFadedOut_8c22895c'), 1);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 2); // ROUTE_OME

        $mat = $this->addressOf('_var_passengerFadeColor_8c228960');
        $this->initUint32($mat, $this->f(1.0)); // >= 1.0 -> field_0x28 = 0

        $slots = $this->addressOf('_var_seats_8c228718');
        $this->initUint32($slots + 4 * 4, 0xbeef);

        $anchor = $this->addressOf('_var_exitSpot1_8c228934');
        $this->initUint32($anchor + 0x0, $this->f(1.0));
        $this->initUint32($anchor + 0x8, $this->f(3.0));

        $midi = $this->addressOf('_var_midiHandles_8c0fcd28');
        $this->initUint32($midi + 1 * 4, 0x5555);

        $state = $this->makeState(6);
        $this->initUint32($state + 0x18, $this->f(0.1));
        $this->initUint32($state + 0x1c, $this->f(0.2));
        $this->initUint32($state + 0x24, 4); // schedule slot to free
        $this->initUint32($state + 0x2c, 1); // quadrant
        $this->initUint32($state + 0x30, 77); // sound id

        $this->call('_PassengerExitTask_8c02d46c')->with(0, $state);

        $this->shouldWriteLong($state + 0x28, 0);
        $this->shouldWriteLong($slots + 4 * 4, 0xffffffff);
        $this->shouldWriteFloat($state + 0x08, 1.1);
        $this->shouldWriteFloat($state + 0x10, 3.2);
        $this->shouldWriteLong($state + 0x14, 0x20);
        $this->shouldWriteLong($state + 0x04, 7);
        $this->shouldCall('_sdMidiPlay')->with(0x5555, 1, 77, 0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(2, $this->addressOf('_drawPassengerSprite_8c02d19c'), $state);
        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 1);
    }

    public function test_case9_flag_set_frees_task(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_passengersFadedOut_8c22895c'), 1);

        $task = $this->alloc(0x20);
        $state = $this->makeState(9);

        $this->call('_PassengerExitTask_8c02d46c')->with($task, $state);

        $this->shouldCall('_TaskKill_8c014b66')->with($task);
    }

    public function test_case9_flag_clear_registers_layer1(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_passengersFadedOut_8c22895c'), 0);

        $task = $this->alloc(0x20);
        $state = $this->makeState(9);

        $this->call('_PassengerExitTask_8c02d46c')->with($task, $state);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(1, $this->addressOf('_drawPassengerSprite_8c02d19c'), $state);
        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 1);
    }

    public function test_default_does_nothing_but_set_flag(): void
    {
        $this->resolveSymbols();

        $state = $this->makeState(42);

        $this->call('_PassengerExitTask_8c02d46c')->with(0, $state);

        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 1);
    }
};
