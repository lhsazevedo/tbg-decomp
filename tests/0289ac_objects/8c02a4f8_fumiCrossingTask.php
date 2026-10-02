<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // State offsets, matching ObjectsInstall_8c02a6ac's test.
    const ST_15 = 0x54; // phase
    const ST_16 = 0x58; // counter (phases 1 and 3)
    const ST_17 = 0x5c; // threshold (phases 1 and 3)
    const ST_18 = 0x60; // counter (phase 2)
    const ST_19 = 0x64; // threshold (phase 2)

    private function initFloat(int $addr, float $value): void
    {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    /** Points var_trafficSignalFrames_8c227e24 at a fresh slots buffer and returns its address. */
    private function slots(int $count = 4): int
    {
        $slots = $this->alloc($count * 4);
        $this->initUint32($this->addressOf('_var_trafficSignalFrames_8c227e24'), $slots);
        return $slots;
    }

    public function test_phase0_no_signal_is_noop(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x3bc), 0x00ffffff);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 0);

        $this->call('_fumiCrossingTask_8c02a4f8')->with($task, $state);
    }

    public function test_phase0_signal_advances_and_clears_slot_zero(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x3bc), 0xff000000);
        $slots = $this->slots();

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 0);

        $this->call('_fumiCrossingTask_8c02a4f8')->with($task, $state);

        $this->shouldWriteLong($state + self::ST_15, 1);
        $this->shouldWriteLong($slots + 0, 0);
    }

    public function test_phase1_below_threshold_advances_counter_and_draws(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 1);
        $this->initFloat($state + self::ST_16, 2.0);
        $this->initFloat($state + self::ST_17, 5.0);

        $this->call('_fumiCrossingTask_8c02a4f8')->with($task, $state);

        $this->shouldWriteFloat($state + self::ST_16, 3.0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawFumiCrossing_8c02a47c'), $state);
    }

    public function test_phase1_reaches_threshold_advances_to_phase2(): void
    {
        $fumi01 = $this->alloc(8);
        $this->initUint32($fumi01 + 4, 9);
        $this->initUint32($this->addressOf('_var_fumiOpenMotion_8c228418'), $fumi01);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 1);
        $this->initFloat($state + self::ST_16, 4.0);
        $this->initFloat($state + self::ST_17, 5.0);

        $this->call('_fumiCrossingTask_8c02a4f8')->with($task, $state);

        $this->shouldWriteFloat($state + self::ST_16, 5.0);
        $this->shouldWriteLong($state + self::ST_15, 2);
        $this->shouldWriteFloat($state + self::ST_16, 0.0);
        $this->shouldWriteFloat($state + self::ST_17, 8.0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawFumiCrossing_8c02a47c'), $state);
    }

    public function test_phase2_below_threshold_advances_counter_and_draws(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 2);
        $this->initFloat($state + self::ST_18, 1.0);
        $this->initFloat($state + self::ST_19, 3.0);

        $this->call('_fumiCrossingTask_8c02a4f8')->with($task, $state);

        $this->shouldWriteFloat($state + self::ST_18, 2.0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawFumiCrossing_8c02a47c'), $state);
    }

    public function test_phase2_reaches_threshold_advances_to_phase3(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 2);
        $this->initFloat($state + self::ST_18, 2.0);
        $this->initFloat($state + self::ST_19, 3.0);

        $this->call('_fumiCrossingTask_8c02a4f8')->with($task, $state);

        $this->shouldWriteFloat($state + self::ST_18, 3.0);
        $this->shouldWriteLong($state + self::ST_15, 3);
        $this->shouldWriteFloat($state + self::ST_19, 0.0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawFumiCrossing_8c02a47c'), $state);
    }

    public function test_phase3_below_threshold_advances_counter_and_draws(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 3);
        $this->initFloat($state + self::ST_16, 2.0);
        $this->initFloat($state + self::ST_17, 8.0);

        $this->call('_fumiCrossingTask_8c02a4f8')->with($task, $state);

        $this->shouldWriteFloat($state + self::ST_16, 3.0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawFumiCrossing_8c02a47c'), $state);
    }

    public function test_phase3_reaches_threshold_freezes_at_last_frame(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 3);
        $this->initFloat($state + self::ST_16, 7.0);
        $this->initFloat($state + self::ST_17, 8.0);

        $this->call('_fumiCrossingTask_8c02a4f8')->with($task, $state);

        $this->shouldWriteFloat($state + self::ST_16, 8.0);
        $this->shouldWriteLong($state + self::ST_15, 4);
        $this->shouldWriteFloat($state + self::ST_16, 8.0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawFumiCrossing_8c02a47c'), $state);
    }

    public function test_phase4_holds_and_just_draws(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 4);

        $this->call('_fumiCrossingTask_8c02a4f8')->with($task, $state);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawFumiCrossing_8c02a47c'), $state);
    }
};
