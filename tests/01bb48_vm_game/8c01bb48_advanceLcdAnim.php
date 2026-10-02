<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// LCD icon animation. anim = {frames, delay, frameIdx}; frames[0] = frame count,
// each frame 0x600 bytes with the LCD bitmap at +4. slot 0..7 maps to a device id.
return new class extends TestCase {
    private function anim(int $frames, int $delay, int $idx): int
    {
        $a = $this->alloc(0xc);
        $this->initUint32($a + 0x0, $frames);
        $this->initUint32($a + 0x4, $delay);
        $this->initUint32($a + 0x8, $idx);
        return $a;
    }

    public function test_slot_maps_to_device()
    {
        // slot 2 -> device 7; not ready -> early return
        $this->resolveSymbols();
        $this->call('_advanceLcdAnim_8c01bb48')->with($this->anim(4, 5, 0), 2);
        $this->shouldCall('_pdVmsLcdIsReady')->with(7)->andReturn(0);
    }

    public function test_slot_passthrough_when_unmapped()
    {
        // slot 8 is outside 0..7, passes through unchanged
        $this->resolveSymbols();
        $this->call('_advanceLcdAnim_8c01bb48')->with($this->anim(4, 5, 0), 8);
        $this->shouldCall('_pdVmsLcdIsReady')->with(8)->andReturn(0);
    }

    public function test_null_clears_lcd()
    {
        $this->resolveSymbols();
        $this->call('_advanceLcdAnim_8c01bb48')->with(0, 0); // slot 0 -> device 1
        $this->shouldCall('_pdVmsLcdIsReady')->with(1)->andReturn(1);
        $this->shouldCall('_memset')
            ->with($this->addressOf('_var_lcdClearBuf_8c2260d4'), 0, 0xc0)
            ->andReturn($this->addressOf('_var_lcdClearBuf_8c2260d4'));
        $this->shouldCall('_pdVmsLcdWrite1')
            ->with(1, $this->addressOf('_var_lcdClearBuf_8c2260d4'))
            ->andReturn(0);
    }

    public function test_disabled_does_nothing()
    {
        // ready, anim != NULL, but var_lcdAnimActive_8c2260a8 == 0 -> no blit
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_lcdAnimActive_8c2260a8'), 0);
        $this->call('_advanceLcdAnim_8c01bb48')->with($this->anim(4, 5, 0), 1); // slot 1 -> device 2
        $this->shouldCall('_pdVmsLcdIsReady')->with(2)->andReturn(1);
    }

    public function test_advances_step_counter()
    {
        // counter < delay -> just increment counter
        $this->resolveSymbols();
        $frames = $this->alloc(0x600 * 4);
        $this->initUint8($frames, 4); // frame count
        $anim = $this->anim($frames, 5, 2);
        $this->initUint32($this->addressOf('_var_lcdAnimActive_8c2260a8'), 1);
        $this->initUint32($this->addressOf('_var_lcdFrameCounter_8c226198'), 1);

        $framePtr = $frames + 2 * 0x600 + 4;
        $this->call('_advanceLcdAnim_8c01bb48')->with($anim, 3); // slot 3 -> device 8
        $this->shouldCall('_pdVmsLcdIsReady')->with(8)->andReturn(1);
        $this->shouldWriteLong($this->addressOf('_var_lcdFramePtr_8c22619c'), $framePtr);
        $this->shouldCall('_pdVmsLcdWrite')->with(8, $framePtr, 3)->andReturn(0);
        $this->shouldWriteLong($this->addressOf('_var_lcdFrameCounter_8c226198'), 2);
    }

    public function test_advances_frame_no_wrap()
    {
        // counter >= delay, frameIdx+1 < count -> reset counter, bump frame
        $this->resolveSymbols();
        $frames = $this->alloc(0x600 * 4);
        $this->initUint8($frames, 4);
        $anim = $this->anim($frames, 5, 1);
        $this->initUint32($this->addressOf('_var_lcdAnimActive_8c2260a8'), 1);
        $this->initUint32($this->addressOf('_var_lcdFrameCounter_8c226198'), 5); // >= delay

        $framePtr = $frames + 1 * 0x600 + 4;
        $this->call('_advanceLcdAnim_8c01bb48')->with($anim, 0); // slot 0 -> device 1
        $this->shouldCall('_pdVmsLcdIsReady')->with(1)->andReturn(1);
        $this->shouldWriteLong($this->addressOf('_var_lcdFramePtr_8c22619c'), $framePtr);
        $this->shouldCall('_pdVmsLcdWrite')->with(1, $framePtr, 3)->andReturn(0);
        $this->shouldWriteLong($this->addressOf('_var_lcdFrameCounter_8c226198'), 0);
        $this->shouldWriteLong($anim + 0x8, 2); // frameIdx 1 -> 2
    }

    public function test_advances_frame_wraps()
    {
        // counter >= delay, frameIdx+1 >= count -> frame wraps to 0
        $this->resolveSymbols();
        $frames = $this->alloc(0x600 * 4);
        $this->initUint8($frames, 4);
        $anim = $this->anim($frames, 5, 3); // last frame
        $this->initUint32($this->addressOf('_var_lcdAnimActive_8c2260a8'), 1);
        $this->initUint32($this->addressOf('_var_lcdFrameCounter_8c226198'), 5);

        $framePtr = $frames + 3 * 0x600 + 4;
        $this->call('_advanceLcdAnim_8c01bb48')->with($anim, 0); // device 1
        $this->shouldCall('_pdVmsLcdIsReady')->with(1)->andReturn(1);
        $this->shouldWriteLong($this->addressOf('_var_lcdFramePtr_8c22619c'), $framePtr);
        $this->shouldCall('_pdVmsLcdWrite')->with(1, $framePtr, 3)->andReturn(0);
        $this->shouldWriteLong($this->addressOf('_var_lcdFrameCounter_8c226198'), 0);
        $this->shouldWriteLong($anim + 0x8, 4); // frameIdx 3 -> 4
        $this->shouldWriteLong($anim + 0x8, 0); // then wrapped to 0
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_pdVmsLcdIsReady', 4);
        $this->setSize('_pdVmsLcdWrite1', 4);
        $this->setSize('_pdVmsLcdWrite', 4);
        $this->setSize('_memset', 4);
    }
};
