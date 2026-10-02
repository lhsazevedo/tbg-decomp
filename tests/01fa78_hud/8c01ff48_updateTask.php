<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function setup(): int
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3c8);
        // var_tachoNeedleVerts_8c226478 follows var_hudState_8c22643c in section B,
        // so assertions reach both off one base.
        $base = $this->addressOf('_var_hudState_8c22643c');

        $this->setSize('_var_runState_8c2285c4', 0x9c);

        // Zero the whole scratch region and bus state by default; each test
        // overrides only the bits it cares about.
        for ($off = 0; $off < 0x6c; $off += 4) {
            $this->initUint32($base + $off, 0);
        }
        for ($off = 0; $off < 0x3c8; $off += 4) {
            $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + $off, 0);
        }
        for ($off = 0; $off < 0x10; $off += 4) {
            $this->initUint32($this->addressOf('_var_hudMark_8c2264a8') + $off, 0);
        }
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x0c, 0);

        return $base;
    }

    public function test_idle_frame_pushes_meter_render_with_zero_message(): void
    {
        $this->setup();

        $this->call('_updateTask_8c01ff48');

        $this->shouldWriteSymbolOffset('_var_hudMark_8c2264a8', 0x04, 0); // out-of-window: no comment digit
        $this->shouldWriteSymbolOffset('_var_hudState_8c22643c', 0x18, 1); // var_hudState_8c22643c.blinkTimer_0x18 += 1
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_drawHud_8c01fbac'), 0);
    }

    public function test_blinker_bit_stages_message_and_arms_flag(): void
    {
        $base = $this->setup();
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x3b0, 3); // blinker bits 0-2 = 3

        $this->call('_updateTask_8c01ff48');

        $this->shouldWriteSymbolOffset('_var_hudState_8c22643c', 0x14, 3 + 0x1f); // var_hudState_8c22643c.driveMarkIcon_0x14
        $this->shouldWriteSymbolOffset('_var_hudState_8c22643c', 0x10, 1); // flag
        $this->shouldWriteSymbolOffset('_var_hudState_8c22643c', 0x18, 0); // var_hudState_8c22643c.blinkTimer_0x18 reset
        $this->shouldWriteSymbolOffset('_var_hudMark_8c2264a8', 0x04, 0); // out-of-window: no comment digit
        $this->shouldWriteSymbolOffset('_var_hudState_8c22643c', 0x18, 1); // var_hudState_8c22643c.blinkTimer_0x18 += 1 (later, ends at 1)
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_drawHud_8c01fbac'), 0);
    }
};
