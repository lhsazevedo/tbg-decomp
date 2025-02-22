<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    public function test_no_velocity()
    {
        $this->resolveSymbols();

        $this->initCursor(4.0, 5.0);
        $this->initVelocity(0.0, 0.0);
        $this->initTarget(6.0, 7.0);

        $this->call('_interpolateCursor_8c016d2c');

        $this->shouldWriteCursor(4, 5);

        $this->shouldReturn(1);
    }

    public function test_diagonal()
    {
        $this->resolveSymbols();

        $this->initCursor(4.0, 5.0);
        $this->initVelocity(1.0, 1.0);
        $this->initTarget(6.0, 7.0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(5.0, 6.0);
        $this->shouldReturn(0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(6.0, 7.0);
        $this->shouldReturn(0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(7.0, 8.0);
        // TODO: Assert arguments
        $this->shouldCall('__quick_evn_mvn');
        $this->shouldReturn(1);
    }

    public function test_vertical()
    {
        $this->resolveSymbols();

        $this->initCursor(4.0, 5.0);
        $this->initVelocity(0.0, 1.0);
        $this->initTarget(4.0, 7.0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(4.0, 6.0);
        $this->shouldReturn(0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(4.0, 7.0);
        $this->shouldReturn(0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(4.0, 8.0);
        $this->shouldCall('__quick_evn_mvn');
        $this->shouldReturn(1);
    }

    public function test_horizontal()
    {
        $this->resolveSymbols();

        $this->initCursor(4.0, 5.0);
        $this->initVelocity(1.0, 0.0);
        $this->initTarget(6.0, 5.0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(5.0, 5.0);
        $this->shouldReturn(0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(6.0, 5.0);
        $this->shouldReturn(0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(7.0, 5.0);
        $this->shouldCall('__quick_evn_mvn');
        $this->shouldReturn(1);
    }

    public function test_reverse_diagonal()
    {
        $this->resolveSymbols();

        $this->initCursor(6.0, 7.0);
        $this->initVelocity(-1.0, -1.0);
        $this->initTarget(4.0, 5.0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(5.0, 6.0);
        $this->shouldReturn(0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(4.0, 5.0);
        $this->shouldCall('__quick_evn_mvn');
        $this->shouldReturn(1);
    }

    public function test_reverse_vertical()
    {
        $this->resolveSymbols();

        $this->initCursor(4.0, 7.0);
        $this->initVelocity(0.0, -1.0);
        $this->initTarget(4.0, 5.0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(4.0, 6.0);
        $this->shouldReturn(0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(4.0, 5.0);
        $this->shouldCall('__quick_evn_mvn');
        $this->shouldReturn(1);
    }

    public function test_reverse_horizontal()
    {
        $this->resolveSymbols();

        $this->initCursor(6.0, 5.0);
        $this->initVelocity(-1.0, 0.0);
        $this->initTarget(4.0, 5.0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(5.0, 5.0);
        $this->shouldReturn(0);

        $this->call('_interpolateCursor_8c016d2c');
        $this->shouldWriteCursor(4.0, 5.0);
        $this->shouldCall('__quick_evn_mvn');
        $this->shouldReturn(1);
    }

    private function initCursor(float $x, float $y) {
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x20, fdec($x));
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x24, fdec($y));
    }

    private function initTarget(float $x, float $y) {
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x28, fdec($x));
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x2c, fdec($y));
    }

    private function initVelocity(float $x, float $y) {
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x30, fdec($x));
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x34, fdec($y));
    }

    private function shouldWriteCursor(float $x, float $y) {
        $this->shouldWriteFloat($this->addressOf('_menuState_8c1bc7a8') + 0x20, $x);
        $this->shouldWriteFloat($this->addressOf('_menuState_8c1bc7a8') + 0x24, $y);
    }

    public function resolveSymbols() {
        $this->setSize('_menuState_8c1bc7a8', 0x6c);

        // Functions
        $this->setSize('__quick_evn_mvn', 4);
    }
};
