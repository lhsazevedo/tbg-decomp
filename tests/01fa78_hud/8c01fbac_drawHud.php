<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\CallingConventions\RiroCallingConvention;

/*
 * _drawHud_8c01fbac(int arg0): the in-drive HUD render. arg0 is the driver-
 * comment message code updateTask_8c01ff48 stages through the fade-command queue.
 * var_tachoNeedleVerts_8c226478 follows the var_hudState_8c22643c block in
 * section B -- see updateTask_8c01ff48's test for the same adjacency.
 * init_pointsMeterFill_8c045334/374/3b4/414 were local (unexported) labels in the .src
 * object; gated .EXPORTs under .AIFDEF UNIT_TESTING were added so tests can
 * see them, matching init_fadeQuad_8c0455a8's convention in 022464_render.
 * The drawSpeedAndTimers_8c01fe84 tail is exercised in its own test file and mocked here.
 */
return new class extends TestCase {
    private function setup(): array
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3c8);
        // var_tachoNeedleVerts_8c226478 follows var_hudState_8c22643c in section B,
        // so assertions reach both off one base.
        $base = $this->addressOf('_var_hudState_8c22643c');

        $runState = $this->setSize('_var_runState_8c2285c4', 0x9c);

        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_markTexlist_8c1bc418', 4);
        $this->setSize('_var_busStopTexlist_8c1bc424', 4);
        $this->setSize('_var_scratchMatrix_8c1bc46c', 0x40);

        $this->setSize('_SpriteDraw_8c014f54', 4);
        $this->setSize('_njUnitMatrix', 4);
        $this->setSize('_njTranslate', 4);
        $this->setSize('_njRotateZ', 4);
        $this->setSize('_njCalcPoint', 4);
        $this->setSize('_njDrawPolygon', 4);
        $this->setSize('__divls', 4);
        $this->setSize('__modls', 4);

        for ($off = 0; $off < 0x6c; $off += 4) {
            $this->initUint32($base + $off, 0);
        }
        for ($off = 0; $off < 0x3c8; $off += 4) {
            $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + $off, 0);
        }
        for ($off = 0; $off < 0x10; $off += 4) {
            $this->initUint32($this->addressOf('_var_hudMark_8c2264a8') + $off, 0);
        }
        for ($off = 0; $off < 0x9c; $off += 4) {
            $this->initUint32($runState + $off, 0);
        }
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0); // PLAY_MODE_NORMAL
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x0c, 0);

        $mark = 0xcafe1000;
        $busStop = 0xcafe2000;
        $this->initUint32($this->addressOf('_var_markTexlist_8c1bc418'), $mark);
        $this->initUint32($this->addressOf('_var_busStopTexlist_8c1bc424'), $busStop);

        return [$base, $runState];
    }

    protected function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }

    // drawHud_8c01fbac falls through into drawSpeedAndTimers_8c01fe84's code in the original
    // asm (no BSR/JSR anywhere reaches that label -- see drawHud_8c01fbac's own
    // comment), so the .src object can't have that call mocked away: it
    // just keeps executing the real digit/timer draws. The C object made
    // it a real call, which can be mocked. scheduleTime_0x14/runClock_0x18 are assumed 0
    // (this file's default zero-fill) wherever this is used.
    private function assertSpeedTail(int $speed): void
    {
        $busStop = $this->addressOf('_var_busStopTexlist_8c1bc424');

        if ($this->isAsmObject()) {
            $abs = abs($speed);
            $units = $abs % 10;
            $tens = intdiv($abs, 10);
            $this->shouldCall('__modls')->with($abs, 10)->using(new RiroCallingConvention())->andReturn($units);
            $this->shouldCall('_SpriteDraw_8c014f54')->with($busStop, $units, 320.0, 420.0, -1.21);
            $this->shouldCall('__divls')->with($abs, 10)->using(new RiroCallingConvention())->andReturn($tens);
            $this->shouldCall('_SpriteDraw_8c014f54')->with($busStop, $tens, 308.0, 420.0, -1.21);
            $this->shouldCall('_drawTimeDigits_8c01fa80')->with(0, 402.0, 10);
            $this->shouldCall('_drawTimeDigits_8c01fa80')->with(0, 423.0, 20);
            $this->shouldCall('_SpriteDraw_8c014f54')->with($busStop, 0x24, 0.0, 0.0, -1.23);
        } else {
            $this->shouldCall('_drawSpeedAndTimers_8c01fe84')->with($speed);
        }
    }

    // Meter bar + both border quads. Assumes the baseline meter bar inputs
    // (ramped value 0, divisor 100) unless a test overrides them
    // beforehand. In the .src object, barWidthInner is written
    // unconditionally (barWidth - 16.0 = 22.0), then overwritten by the <
    // 38.0 clamp -- two writes to the same address. The C compile keeps
    // the pre-clamp value in a register/local instead, so it only performs
    // the one, already-clamped store -- a legitimate HLL simplification,
    // not a behavior change.
    private function assertMeterAndBorders(): void
    {
        $init045334 = $this->addressOf('_init_pointsMeterFill_8c045334');

        $this->shouldWriteFloat($init045334 + 0x20, 38.0);
        if ($this->isAsmObject()) {
            $this->shouldWriteFloat($init045334 + 0x30, 22.0);
        }
        $this->shouldWriteFloat($init045334 + 0x30, 38.0);

        $this->shouldCall('_njDrawPolygon')->with($init045334, 4, 0);
        $this->shouldCall('_njDrawPolygon')->with($this->addressOf('_init_pointsMeterTrack_8c045374'), 4, 0);
        $this->shouldCall('_njDrawPolygon')->with($this->addressOf('_init_hudPanel_8c0453b4'), 6, 1);
    }

    // The needle matrix/CalcPoint sequence + the speed tail, run
    // unconditionally every frame after the needle-position update.
    private function assertNeedleAndSpeedTail(int $angleDividend, int $angleReturn, int $speed): void
    {
        $mat = $this->addressOf('_var_scratchMatrix_8c1bc46c');
        $init045414 = $this->addressOf('_init_tachoNeedle_8c045414');
        $vtx = $this->addressOf('_var_tachoNeedleVerts_8c226478');

        $this->shouldCall('_njUnitMatrix')->with($mat);
        $this->shouldCall('_njTranslate')->with($mat, 320.0, 436.0, 0.0);
        $this->shouldCall('__divls')->with($angleDividend, 6000)->using(new RiroCallingConvention())->andReturn($angleReturn);
        $this->shouldCall('_njRotateZ')->with($mat, $angleReturn);
        $this->shouldCall('_njCalcPoint')->with($mat, $init045414 + 0, $vtx + 0);
        $this->shouldCall('_njCalcPoint')->with($mat, $init045414 + 12, $vtx + 16);
        $this->shouldCall('_njCalcPoint')->with($mat, $init045414 + 24, $vtx + 32);
        $this->shouldCall('_njDrawPolygon')->with($vtx, 3, 0);

        $this->assertSpeedTail($speed);
    }

    // Full unconditional tail (meter/borders, then needle/speed) for tests
    // that don't need to interleave their own assertions in between.
    private function assertAlwaysOnTail(int $angleDividend, int $angleReturn, int $speed): void
    {
        $this->assertMeterAndBorders();
        $this->assertNeedleAndSpeedTail($angleDividend, $angleReturn, $speed);
    }

    public function test_demo_mode_skips_render(): void
    {
        $this->setup();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 2); // PLAY_MODE_DEMO

        $this->call('_drawHud_8c01fbac')->with(0);

        // No calls, no writes -- pure early return.
    }

    public function test_full_render_normal_path(): void
    {
        [$base, $runState] = $this->setup();

        // Driver-comment popup: both staged icons and arg0's icon are drawn.
        $popup = $this->addressOf('_var_hudMark_8c2264a8');
        $this->initUint32($popup + 0x00, 0x40); // markSpriteId_0x00 (icon id)
        $this->initUint32($popup + 0x04, 0x30); // blinkIconId_0x04 (id, doubles as its own gate)
        $this->initUint32($popup + 0x08, 1);    // displayTimer_0x08 (gate for markSpriteId_0x00)

        // Next-stop icon: mirror level 0, armed, blink window open.
        $this->initUint32($base + 0x14, 5);  // var_hudState_8c22643c.driveMarkIcon_0x14
        $this->initUint32($base + 0x18, 61); // var_hudState_8c22643c.blinkTimer_0x18 (> 60)

        // Driver-points meter: (50.0 * 202.0) / 100 + 38.0 = 139.0, inner = 123.0.
        $this->initUint32($base + 0x1c, unpack('L', pack('f', 50.0))[1]); // var_hudState_8c22643c.pointsMeter_0x1c.displayedValue_0x00
        $this->initUint32($runState + 0x10, 100); // var_runState_8c2285c4.driverPointsMax_0x10 (divisor)

        // Both turn-signal bits set.
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x80, 6); // blinker_0x080

        // Needle: mode 0 (relax toward 0), starts above 0.
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x2e0, 0);
        $this->initUint32($base + 0x2c, unpack('L', pack('f', 300.0))[1]); // var_hudState_8c22643c.engineRpm_0x2c

        // Speed readout: 5.0 * 108000.0 / 1000.0 = 540.0 -> 540.
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x27c, unpack('L', pack('f', 5.0))[1]);

        $mark = $this->addressOf('_var_markTexlist_8c1bc418');
        $busStop = $this->addressOf('_var_busStopTexlist_8c1bc424');

        $this->call('_drawHud_8c01fbac')->with(7);

        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x40, 0.0, 0.0, -1.21);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x30, 0.0, 0.0, -1.2);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 7, 0.0, 0.0, -1.21);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($busStop, 5, 0.0, 0.0, -1.2);

        $init045334 = $this->addressOf('_init_pointsMeterFill_8c045334');
        $this->shouldWriteFloat($init045334 + 0x20, 139.0);
        $this->shouldWriteFloat($init045334 + 0x30, 123.0);

        $this->shouldCall('_njDrawPolygon')->with($init045334, 4, 0);
        $this->shouldCall('_njDrawPolygon')->with($this->addressOf('_init_pointsMeterTrack_8c045374'), 4, 0);
        $this->shouldCall('_njDrawPolygon')->with($this->addressOf('_init_hudPanel_8c0453b4'), 6, 1);

        $this->shouldCall('_SpriteDraw_8c014f54')->with($busStop, 0x25, 0.0, 0.0, -1.21);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($busStop, 0x26, 0.0, 0.0, -1.21);

        $this->shouldWriteFloat($base + 0x2c, 100.0); // engineRpm -= 200

        $mat = $this->addressOf('_var_scratchMatrix_8c1bc46c');
        $init045414 = $this->addressOf('_init_tachoNeedle_8c045414');
        $vtx = $this->addressOf('_var_tachoNeedleVerts_8c226478');
        $this->shouldCall('_njUnitMatrix')->with($mat);
        $this->shouldCall('_njTranslate')->with($mat, 320.0, 436.0, 0.0);
        // angle = (int)100.0 * 32768 / 6000
        $this->shouldCall('__divls')->with(100 * 32768, 6000)->using(new RiroCallingConvention())->andReturn(546);
        $this->shouldCall('_njRotateZ')->with($mat, 546);
        $this->shouldCall('_njCalcPoint')->with($mat, $init045414 + 0, $vtx + 0);
        $this->shouldCall('_njCalcPoint')->with($mat, $init045414 + 12, $vtx + 16);
        $this->shouldCall('_njCalcPoint')->with($mat, $init045414 + 24, $vtx + 32);
        $this->shouldCall('_njDrawPolygon')->with($vtx, 3, 0);

        $this->assertSpeedTail(540);
    }

    public function test_mirror_level_1_uses_fixed_departed_icon(): void
    {
        [$base, $runState] = $this->setup();

        $this->initUint32($runState + 0x20, 1); // var_runState_8c2285c4.stopPhase_0x20
        $this->initUint32($base + 0x14, 5);  // var_hudState_8c22643c.driveMarkIcon_0x14 (armed)
        $this->initUint32($base + 0x18, 61); // var_hudState_8c22643c.blinkTimer_0x18 (> 60)
        $this->initUint32($runState + 0x10, 100);  // var_runState_8c2285c4.driverPointsMax_0x10 (avoid div-by-zero)

        $this->call('_drawHud_8c01fbac')->with(0);

        $busStop = $this->addressOf('_var_busStopTexlist_8c1bc424');
        $this->shouldCall('_SpriteDraw_8c014f54')->with($busStop, 0x1e, 0.0, 0.0, -1.2);

        $this->assertAlwaysOnTail(0, 0, 0);
    }

    public function test_mirror_level_2_uses_fixed_approaching_icon(): void
    {
        [$base, $runState] = $this->setup();

        $this->initUint32($runState + 0x20, 2); // var_runState_8c2285c4.stopPhase_0x20
        $this->initUint32($base + 0x18, 61); // var_hudState_8c22643c.blinkTimer_0x18 (> 60) -- level 2 ignores var_hudState_8c22643c.driveMarkIcon_0x14
        $this->initUint32($runState + 0x10, 100);  // var_runState_8c2285c4.driverPointsMax_0x10

        $this->call('_drawHud_8c01fbac')->with(0);

        $busStop = $this->addressOf('_var_busStopTexlist_8c1bc424');
        $this->shouldCall('_SpriteDraw_8c014f54')->with($busStop, 0x1f, 0.0, 0.0, -1.2);

        $this->assertAlwaysOnTail(0, 0, 0);
    }

    public function test_mirror_level_0_skips_icon_when_not_blinking(): void
    {
        [$base, $runState] = $this->setup();

        $this->initUint32($runState + 0x20, 0); // var_runState_8c2285c4.stopPhase_0x20
        $this->initUint32($base + 0x14, 5);  // var_hudState_8c22643c.driveMarkIcon_0x14 (armed)
        $this->initUint32($base + 0x18, 0);  // var_hudState_8c22643c.blinkTimer_0x18 (blink window closed, bits 1/2 clear)
        $this->initUint32($runState + 0x10, 100);  // var_runState_8c2285c4.driverPointsMax_0x10

        $this->call('_drawHud_8c01fbac')->with(0);

        $this->assertAlwaysOnTail(0, 0, 0);
    }

    public function test_needle_case1_settles_toward_500(): void
    {
        [$base, $runState] = $this->setup();

        $this->initUint32($runState + 0x10, 100); // var_runState_8c2285c4.driverPointsMax_0x10
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x2e0, 1); // needle mode 1
        $this->initUint32($base + 0x2c, unpack('L', pack('f', 100.0))[1]); // var_hudState_8c22643c.engineRpm_0x2c < 500

        $this->call('_drawHud_8c01fbac')->with(0);

        $this->assertMeterAndBorders();

        $this->shouldWriteFloat($base + 0x2c, 300.0); // 100.0 + 200.0, still <= 500

        $this->assertNeedleAndSpeedTail((int)(300.0) * 32768, 0, 0);
    }

    public function test_needle_case2_ramps_toward_target_and_clamps(): void
    {
        [$base, $runState] = $this->setup();

        $this->initUint32($runState + 0x10, 100); // var_runState_8c2285c4.driverPointsMax_0x10
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x2e0, 2); // needle mode 2
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x2e8, unpack('L', pack('f', 650.0))[1]); // targetRpm_0x2e8 > 500
        $this->initUint32($base + 0x2c, unpack('L', pack('f', 600.0))[1]); // above target -> ramp up, overshoots

        $this->call('_drawHud_8c01fbac')->with(0);

        $this->assertMeterAndBorders();

        // target(650) < engineRpm(600) is false, so it ramps up: 600 + 200 =
        // 800 (written), then since 800 overshoots the 650 target, clamps
        // to the target (a second write).
        $this->shouldWriteFloat($base + 0x2c, 800.0);
        $this->shouldWriteFloat($base + 0x2c, 650.0);

        $this->assertNeedleAndSpeedTail((int)(650.0) * 32768, 0, 0);
    }
};
