<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_tasks_8c1ba3c8', 4);
        $this->setSize('_TaskInstall_8c014ae8', 4);
    }

    public function test_starts_fade_in_task(): void {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);

        $this->call('_RenderStartFadeIn_8c022a9c')->with(30);

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_fadeInTask_8c022a54'),
                0xffffec, // &task on the stack
                0xfffff0, // &state on the stack
                0,
            )
            ->do(function ($params) use ($task) {
                $this->memory->writeUInt32($params[2], U32::of($task));
            });
        $this->shouldWriteLong($task + 0x08, 30);
        $this->shouldWriteLong($this->addressOf('_var_fadeProgress_8c227d80'), 0xff000000);
        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 1);
    }
};
