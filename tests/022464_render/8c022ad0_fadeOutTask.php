<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\CallingConventions\RiroCallingConvention;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('__divlu', 4);
        $this->setSize('_njSetBackColor', 4);
        $this->setSize('_njDrawPolygon', 4);
        $this->setSize('_TaskUninstall_8c014b66', 4);
    }

    // Each vertex's .col field is written via displacement addressing from
    // init_fadeQuad_8c0455a8 (offsets 0xc/0x1c/0x2c/0x3c);
    // no separate symbol exists for them in either object.
    private function colAddresses(int $init8c0455a8): array {
        return [$init8c0455a8 + 12, $init8c0455a8 + 28, $init8c0455a8 + 44, $init8c0455a8 + 60];
    }

    public function test_ramps_up_while_below_floor(): void {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 60); // frame count
        $this->initUint32($task + 0x0c, 0);  // phase: ramping

        $this->initUint32($this->addressOf('_var_fadeProgress_8c227d80'), 0);

        $init8c0455a8 = $this->addressOf('_init_fadeQuad_8c0455a8');
        [$init8c0455b4, $init8c0455c4, $init8c0455d4, $init8c0455e4] = $this->colAddresses($init8c0455a8);

        $this->call('_fadeOutTask_8c022ad0')->with($task, 0);

        // 0xff0000 / 60 = 0x44000
        $this->shouldCall('__divlu')->with(0xff0000, 60)->using(new RiroCallingConvention())->andReturn(0x44000);
        $this->shouldWriteLong($this->addressOf('_var_fadeProgress_8c227d80'), 0x44000);
        $uVar1 = (0x44000 & 0xff0000) << 8;
        $this->shouldWriteLong($init8c0455b4, $uVar1);
        $this->shouldWriteLong($init8c0455c4, $uVar1);
        $this->shouldWriteLong($init8c0455d4, $uVar1);
        $this->shouldWriteLong($init8c0455e4, $uVar1);
        $this->shouldCall('_njDrawPolygon')->with($init8c0455a8, 4, 1);
    }

    public function test_crosses_floor_and_enters_hold(): void {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 60); // frame count
        $this->initUint32($task + 0x0c, 0);  // phase: ramping

        // One step (0x44000) away from crossing 0xffffff.
        $this->initUint32($this->addressOf('_var_fadeProgress_8c227d80'), 0xfbc000);

        $init8c0455a8 = $this->addressOf('_init_fadeQuad_8c0455a8');
        [$init8c0455b4, $init8c0455c4, $init8c0455d4, $init8c0455e4] = $this->colAddresses($init8c0455a8);

        $this->call('_fadeOutTask_8c022ad0')->with($task, 0);

        $this->shouldCall('__divlu')->with(0xff0000, 60)->using(new RiroCallingConvention())->andReturn(0x44000);
        $this->shouldWriteLong($this->addressOf('_var_fadeProgress_8c227d80'), 0xfbc000 + 0x44000);
        $this->shouldWriteLong($this->addressOf('_var_fadeProgress_8c227d80'), 0xff0000);
        $this->shouldCall('_njSetBackColor')->with(0, 0, 0);
        $this->shouldWriteLong($task + 0x08, 0);
        $this->shouldWriteLong($task + 0x0c, 1);
        $uVar1 = (0xff0000 & 0xff0000) << 8;
        $this->shouldWriteLong($init8c0455b4, $uVar1);
        $this->shouldWriteLong($init8c0455c4, $uVar1);
        $this->shouldWriteLong($init8c0455d4, $uVar1);
        $this->shouldWriteLong($init8c0455e4, $uVar1);
        $this->shouldCall('_njDrawPolygon')->with($init8c0455a8, 4, 1);
    }

    public function test_hold_ticks_without_freeing(): void {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0); // hold counter
        $this->initUint32($task + 0x0c, 1); // phase: holding

        $this->initUint32($this->addressOf('_var_fadeProgress_8c227d80'), 0xff0000);

        $init8c0455a8 = $this->addressOf('_init_fadeQuad_8c0455a8');
        [$init8c0455b4, $init8c0455c4, $init8c0455d4, $init8c0455e4] = $this->colAddresses($init8c0455a8);

        $this->call('_fadeOutTask_8c022ad0')->with($task, 0);

        $this->shouldWriteLong($task + 0x08, 1);
        $this->shouldWriteLong($this->addressOf('_var_fadeProgress_8c227d80'), 0xff0000);
        $uVar1 = (0xff0000 & 0xff0000) << 8;
        $this->shouldWriteLong($init8c0455b4, $uVar1);
        $this->shouldWriteLong($init8c0455c4, $uVar1);
        $this->shouldWriteLong($init8c0455d4, $uVar1);
        $this->shouldWriteLong($init8c0455e4, $uVar1);
        $this->shouldCall('_njDrawPolygon')->with($init8c0455a8, 4, 1);
    }

    public function test_hold_frees_after_two_ticks(): void {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 2); // hold counter
        $this->initUint32($task + 0x0c, 1); // phase: holding

        $this->initUint32($this->addressOf('_var_fadeProgress_8c227d80'), 0xff0000);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $init8c0455a8 = $this->addressOf('_init_fadeQuad_8c0455a8');
        [$init8c0455b4, $init8c0455c4, $init8c0455d4, $init8c0455e4] = $this->colAddresses($init8c0455a8);

        $this->call('_fadeOutTask_8c022ad0')->with($task, 0);

        $this->shouldWriteLong($task + 0x08, 3);
        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 0);
        $this->shouldCall('_TaskUninstall_8c014b66')->with($task);
        $this->shouldCall('_njSetBackColor')->with(0, 0, 0);
        $this->shouldWriteLong($this->addressOf('_var_fadeProgress_8c227d80'), 0xff0000);
        $uVar1 = (0xff0000 & 0xff0000) << 8;
        $this->shouldWriteLong($init8c0455b4, $uVar1);
        $this->shouldWriteLong($init8c0455c4, $uVar1);
        $this->shouldWriteLong($init8c0455d4, $uVar1);
        $this->shouldWriteLong($init8c0455e4, $uVar1);
        $this->shouldCall('_njDrawPolygon')->with($init8c0455a8, 4, 1);
    }
};
