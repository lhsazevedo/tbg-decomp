<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    public function test_parks_cursor_on_first_received_letter(): void
    {
        $this->setSize('_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        $this->setSize('_init_8c045170', 0x30);

        // First received letter is slot 2 at init_8c045170[2] = (100, 200).
        $this->seedLetters([0, 0, 1, 0, 0, 1]);
        $this->seedCursorTarget(2, 100.0, 200.0);

        $task = $this->alloc(0x20);
        $this->call('_AlbumSwitchFromTask_8c01d6e2')->with($task);

        $this->shouldCall('_setTaskAction_8c014b3e')->with(
            $task,
            $this->addressOf('_AlbumMenuTask_8c01d300'),
        );
        $this->shouldWriteLong($this->m(0x18), 0);
        $this->shouldWriteFloat($this->m(0x20), 100.0);
        $this->shouldWriteFloat($this->m(0x24), 200.0);
        $this->shouldWriteLong($this->m(0x38), 2);
        $this->shouldWriteLong($task + 0x08, 1);

        $this->expectResourceSetup();
    }

    public function test_no_letters_leaves_task_pending_flag_clear(): void
    {
        $this->setSize('_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        $this->setSize('_init_8c045170', 0x30);

        $this->seedLetters([0, 0, 0, 0, 0, 0]);

        $task = $this->alloc(0x20);
        $this->call('_AlbumSwitchFromTask_8c01d6e2')->with($task);

        $this->shouldCall('_setTaskAction_8c014b3e')->with(
            $task,
            $this->addressOf('_AlbumMenuTask_8c01d300'),
        );
        $this->shouldWriteLong($this->m(0x18), 0);
        // No cursor / selected writes: nothing to park on.
        $this->shouldWriteLong($task + 0x08, 0);

        $this->expectResourceSetup();
    }

    /* ---------------- Helpers ---------------- */

    private function m(int $offset): int
    {
        return $this->addressOf('_menuState_8c1bc7a8') + $offset;
    }

    private function seedLetters(array $letters): void
    {
        $base = $this->addressOf('_var_progress_8c1ba1cc') + 0x2c;
        foreach ($letters as $i => $value) {
            $this->initUint32($base + $i * 4, $value);
        }
    }

    private function seedCursorTarget(int $slot, float $x, float $y): void
    {
        $base = $this->addressOf('_init_8c045170') + $slot * 8;
        $this->initUint32($base + 0, fdec($x));
        $this->initUint32($base + 4, fdec($y));
    }

    private function expectResourceSetup(): void
    {
        $this->shouldCall('_CourseMenuFreeResourceGroup_8c0185c4')->with($this->m(0x00));
        $this->shouldCall('_njGarbageTexture')->with(
            $this->addressOf('_var_tex_8c157af8'), 0xc00
        );
        $this->shouldCall('_AsqInitQueues_11f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_11f6c');
        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')->with(
            $this->m(0x0c),
            $this->addressOf('_albumResourceGroup_8c045160'),
        );
        $this->shouldCall('_setUknPvmBool_8c014330');
        $this->shouldCall('_AsqProcessQueues_11fe0')->with(
            $this->addressOf('_AsqNop_11120'),
            0,
            0,
            0,
            $this->addressOf('_resetUknPvmBool_8c014322'),
        );
    }
};
