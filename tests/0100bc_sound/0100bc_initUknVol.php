<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    public function test_basic()
    {
        $this->setSize('__divls', 0x4);

        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });

        // field_0x18 = (float) init_vol / 2600 -- the cast makes this a hardware
        // float division (FDIV), not an integer one, so no __divls call.
        $this->shouldWriteLong($this->addressOf('_var_uknVol_8c0fcd50') + 0x18, fdec(0.048846155));
        $this->shouldWriteLong($this->addressOf('_var_uknVol_8c0fcd50') + 0x1C, fdec(0.042333334));
        // field_0x14 = init_vol * 30 / 100 (100 isn't a power of two).
        $this->shouldCall('__divls');
        $this->shouldWriteLong($this->addressOf('_var_uknVol_8c0fcd50') + 0x14, 38);
        // field_0x08 = init_vol * 40 / 100.
        $this->shouldCall('__divls');
        $this->shouldWriteLong($this->addressOf('_var_uknVol_8c0fcd50') + 0x08, 50);
        $this->shouldWriteLong($this->addressOf('_var_uknVol_8c0fcd50') + 0x0c, 146);
        // field_0x20 = (float) init_vol / 3900 -- also a hardware float division.
        $this->shouldWriteLong($this->addressOf('_var_uknVol_8c0fcd50') + 0x20, fdec(0.032564103));

        $this->singleCall('_initUknVol_8c0100bc')->run();
    }
};
