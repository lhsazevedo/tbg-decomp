<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_basic()
    {
        $currentDemo = 2;
        $this->initUint32(
            $this->addressOf('_var_demoIndex_8c1bb8d8'), $currentDemo
        );

        $nextDemo = $currentDemo + 1;
        $nextDemoEntry =
            $this->addressOf('_init_demos_8c044154') + $nextDemo * 0xc;

        $createdTask = 0xffffec;
        $this->shouldCall('_pushTask_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_FUN_8c01594c'),
                $createdTask,
                0xfffff0,
                0
            )->do(function ($params) use ($createdTask) {
                $this->memory->writeUInt32($params[2], U32::of($createdTask));
            });

        $this->shouldWriteLong($createdTask + 0x08, 0);
        $this->shouldWriteLongTo('_var_demo_8c1bb8d0', 2);
        $this->shouldWriteLongTo('_var_8c1bb8d4', 1);
        $this->shouldWriteLongTo('_var_demoIndex_8c1bb8d8', $nextDemo);

        $this->shouldCall('_AsqInitQueues_11f36')->with(1, 0, 0, 0);
        $this->shouldCall('_AsqResetQueues_11f6c');
        $this->shouldCall('_AsqRequestDat_11182')->with(
            '\\SYSTEM',
            'demo0.bin',
            $this->addressOf('_var_demoBuf_8c1ba3c4')
        );

        $this->shouldWriteLongTo('_var_demoEntryValue_8c227e14', 0x08);
        $this->shouldWriteLongTo('_var_demoEntryValue_8c22822c', 0x04);

        $this->shouldCall('_resetPvmReady_8c014322');
        $this->shouldCall('_AsqProcessQueues_11fe0')
            ->with(
                $this->addressOf('_AsqNop_11120'),
                0,
                0,
                0,
                $this->addressOf('_setPvmReady_8c014330')
            );

        $this->singleCall('_FUN_demo_8c0159ac')->with(0xbeba1337)->run();
    }

    public function test_it_loops_demos()
    {
        $currentDemo = 19;
        $this->initUint32(
            $this->addressOf('_var_demoIndex_8c1bb8d8'), $currentDemo
        );

        $nextDemo = 0;
        $nextDemoEntry =
            $this->addressOf('_init_demos_8c044154') + $nextDemo * 0xc;

        $createdTask = 0xffffec;
        $this->shouldCall('_pushTask_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_FUN_8c01594c'),
                $createdTask,
                0xfffff0,
                0
            )->do(function ($params) use ($createdTask) {
                $this->memory->writeUInt32($params[2], U32::of($createdTask));
            });

        $this->shouldWriteLong($createdTask + 0x08, 0);
        $this->shouldWriteLongTo('_var_demo_8c1bb8d0', 2);
        $this->shouldWriteLongTo('_var_8c1bb8d4', 1);
        $this->shouldWriteLongTo('_var_demoIndex_8c1bb8d8', $currentDemo + 1);
        $this->shouldWriteLongTo('_var_demoIndex_8c1bb8d8', $nextDemo);

        $this->shouldCall('_AsqInitQueues_11f36')->with(1, 0, 0, 0);
        $this->shouldCall('_AsqResetQueues_11f6c');
        $this->shouldCall('_AsqRequestDat_11182')->with(
            '\\SYSTEM',
            'demo2.bin',
            $this->addressOf('_var_demoBuf_8c1ba3c4')
        );

        $this->shouldWriteLongTo('_var_demoEntryValue_8c227e14', 0x1e);
        $this->shouldWriteLongTo('_var_demoEntryValue_8c22822c', 0x15);

        $this->shouldCall('_resetPvmReady_8c014322');
        $this->shouldCall('_AsqProcessQueues_11fe0')
            ->with(
                $this->addressOf('_AsqNop_11120'),
                0,
                0,
                0,
                $this->addressOf('_setPvmReady_8c014330')
            );

        $this->singleCall('_FUN_demo_8c0159ac')->with(0xbeba1337)->run();
    }

    protected function resolveSymbols(): void
    {
        // Functions
        $this->setSize('_isPvmReady_8c01432a', 4);
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
