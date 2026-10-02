<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\CallingConventions\RiroCallingConvention;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('__divlu', 4);
        $this->setSize('_njDrawPolygon', 4);
        $this->setSize('_TaskKill_8c014b66', 4);
    }

    // Each vertex's .col field is written via displacement addressing from
    // init_fadeQuad_8c0455a8 (offsets 0xc/0x1c/0x2c/0x3c);
    // no separate symbol exists for them in either object.
    private function colAddresses(int $init8c0455a8): array {
        return [$init8c0455a8 + 12, $init8c0455a8 + 28, $init8c0455a8 + 44, $init8c0455a8 + 60];
    }

    public function test_steps_down_and_draws_while_above_floor(): void {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 60); // frame count

        $this->initUint32($this->addressOf('_var_fadeProgress_8c227d80'), 0xff000000);

        $init8c0455a8 = $this->addressOf('_init_fadeQuad_8c0455a8');
        [$init8c0455b4, $init8c0455c4, $init8c0455d4, $init8c0455e4] = $this->colAddresses($init8c0455a8);

        $this->call('_fadeInTask_8c022a54')->with($task, 0);

        // 0xff000000 / 60 = 0x4400000
        $this->shouldCall('__divlu')->with(0xff000000, 60)->using(new RiroCallingConvention())->andReturn(0x4400000);
        $this->shouldWriteLong($this->addressOf('_var_fadeProgress_8c227d80'), 0xff000000 - 0x4400000);
        $this->shouldWriteLong($init8c0455b4, (0xff000000 - 0x4400000) & 0xff000000);
        $this->shouldWriteLong($init8c0455c4, (0xff000000 - 0x4400000) & 0xff000000);
        $this->shouldWriteLong($init8c0455d4, (0xff000000 - 0x4400000) & 0xff000000);
        $this->shouldWriteLong($init8c0455e4, (0xff000000 - 0x4400000) & 0xff000000);
        $this->shouldCall('_njDrawPolygon')->with($init8c0455a8, 4, 1);
    }

    public function test_frees_itself_once_step_reaches_floor(): void {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 60); // frame count

        // One step (0x4400000) away from the 0x1000000 floor.
        $this->initUint32($this->addressOf('_var_fadeProgress_8c227d80'), 0x1000000 + 0x4400000);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_fadeInTask_8c022a54')->with($task, 0);

        $this->shouldCall('__divlu')->with(0xff000000, 60)->using(new RiroCallingConvention())->andReturn(0x4400000);
        $this->shouldWriteLong($this->addressOf('_var_fadeProgress_8c227d80'), 0x1000000);
        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 0);
        $this->shouldCall('_TaskKill_8c014b66')->with($task);
    }
};
