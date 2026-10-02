<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// Retargets the VM SELECT cursor to slot's cell (lerp over 6 frames) and, when
// subState_0x1c is set, pops a warning box keyed on the slot's VMU status.
return new class extends TestCase {
    private const SLOT = 2;

    private function seedGeometry(): void
    {
        $m = $this->addressOf('_var_menuState_8c1bc7a8');
        // current cursor position
        $this->initUint32($m + 0x20, fdec(10.0));
        $this->initUint32($m + 0x24, fdec(4.0));
        // target table entry for this slot
        $table = $this->addressOf('_init_slotCursorTargets_8c044e50') + self::SLOT * 8;
        $this->initUint32($table + 0x0, fdec(100.0));
        $this->initUint32($table + 0x4, fdec(40.0));
    }

    private function expectLerp(): void
    {
        $m = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteFloat($m + 0x28, 100.0);          // cursorTarget.x
        $this->shouldWriteFloat($m + 0x2c, 40.0);           // cursorTarget.y
        $this->shouldWriteFloat($m + 0x30, (100.0 - 10.0) / 6.0); // velocity.x
        $this->shouldWriteFloat($m + 0x34, (40.0 - 4.0) / 6.0);   // velocity.y
    }

    public function test_lerp_only_when_flag_clear()
    {
        $this->resolveSymbols();
        $this->seedGeometry();
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x1c, 0);

        $this->call('_selectSlot_8c01bf2a')->with(self::SLOT);
        $this->expectLerp();
    }

    public function test_warns_incompatible()
    {
        $this->runWarning(1, "交通標識クイズがありません");
    }

    public function test_warns_incompatible_status2()
    {
        $this->runWarning(2, "交通標識クイズがありません");
    }

    public function test_warns_incompatible_status4()
    {
        $this->runWarning(4, "交通標識クイズがありません");
    }

    public function test_warns_full_status5()
    {
        $this->runWarning(5, "交通標識クイズがあります");
    }

    public function test_warns_full_status6()
    {
        $this->runWarning(6, "交通標識クイズがあります");
    }

    public function test_warns_default()
    {
        $this->runWarning(3, "");
    }

    private function runWarning(int $status, string $text): void
    {
        $this->resolveSymbols();
        $this->seedGeometry();
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x1c, 1);
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048') + self::SLOT * 4, $status);

        $this->call('_selectSlot_8c01bf2a')->with(self::SLOT);
        $this->expectLerp();
        $this->shouldCall('_MessageBoxSetText_8c02aefc')
            ->with($text)
            ->andReturn(0);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_MessageBoxSetText_8c02aefc', 4);
    }
};
