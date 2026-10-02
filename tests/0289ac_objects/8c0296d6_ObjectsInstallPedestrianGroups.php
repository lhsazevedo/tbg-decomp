<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    protected function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }

    // CurrentCourse.lineHum_0x2c/macHumG0_0x30/macHumM0_0x34 offsets.
    const OFF_LINE_HUM = 0x2c;
    const OFF_MAC_HUM_G0 = 0x30;
    const OFF_MAC_HUM_M0 = 0x34;
    const COURSE_SIZE = 4 + 19 * 4;

    private function setupCourse(int $paths, int $groupDefs, int $groupLists): void
    {
        $this->setSize('_var_currentCourse_8c1bb868', self::COURSE_SIZE);
        $course = $this->addressOf('_var_currentCourse_8c1bb868');
        $this->initUint32($course + self::OFF_LINE_HUM, $paths);
        $this->initUint32($course + self::OFF_MAC_HUM_G0, $groupDefs);
        $this->initUint32($course + self::OFF_MAC_HUM_M0, $groupLists);
    }

    public function testNoGroupsDefined(): void
    {
        $paths = 0x30001000;
        $groupDefs = 0x30002000;

        // Group-lists table with a single, immediately-terminated (NULL)
        // demo-entry list: no group id is ever seen.
        $lists = $this->alloc(4);
        $this->initUint32($lists, 0);

        $this->setupCourse($paths, $groupDefs, $lists);

        $this->call('_ObjectsInstallPedestrianGroups_8c0296d6');

        $this->shouldWriteLongTo('_var_pedPaths_8c228238', $paths);
        $this->shouldWriteLongTo('_var_pedGroupLists_8c228240', $lists);
        $this->shouldWriteLongTo('_var_pedGroupDefs_8c22823c', $groupDefs);
        $this->shouldWriteLong($this->addressOf('_var_pedGroupCount_8c228234'), -1);
        $this->shouldCall('_CollisionQueueReset_8c02e486');
    }

    public function testAllocatesGroupsFromHighestGroupId(): void
    {
        $paths = 0x30001000;
        $groupDefs = 0x30002000;

        // Demo entry 0's list references group ids 0 and 2 (holes at 1 are
        // fine -- only the max matters here); demo entry 1 is NULL,
        // terminating the outer scan.
        $list0 = $this->alloc(0xc);
        $this->initUint32($list0 + 0, 0);
        $this->initUint32($list0 + 4, 2);
        $this->initUint32($list0 + 8, -1);

        $lists = $this->alloc(8);
        $this->initUint32($lists + 0, $list0);
        $this->initUint32($lists + 4, 0);

        $this->setupCourse($paths, $groupDefs, $lists);

        $groups = 0x30003000;

        $this->call('_ObjectsInstallPedestrianGroups_8c0296d6');

        $this->shouldWriteLongTo('_var_pedPaths_8c228238', $paths);
        $this->shouldWriteLongTo('_var_pedGroupLists_8c228240', $lists);
        $this->shouldWriteLongTo('_var_pedGroupDefs_8c22823c', $groupDefs);
        $this->shouldWriteLong($this->addressOf('_var_pedGroupCount_8c228234'), -1);
        $this->shouldWriteLong($this->addressOf('_var_pedGroupCount_8c228234'), 0); // list0[0]
        $this->shouldWriteLong($this->addressOf('_var_pedGroupCount_8c228234'), 2); // list0[1]
        $this->shouldWriteLong($this->addressOf('_var_pedGroupCount_8c228234'), 3);
        $this->shouldCall('_syMalloc')->with(3 * 0xc)->andReturn($groups);
        $this->shouldWriteLong($this->addressOf('_var_pedGroups_8c228230'), $groups);
        $this->shouldWriteLong($groups + 0 * 0xc, 0);
        $this->shouldWriteLong($groups + 1 * 0xc, 0);
        $this->shouldWriteLong($groups + 2 * 0xc, 0);

        $taskLocal = 0xffffe4;
        $stateLocal = 0xffffe8;
        $this->shouldCall('_TaskInstall_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba5e8'),
            $this->addressOf('_pedestriansTask_8c0293f6'),
            $taskLocal,
            $stateLocal,
            0,
        )->do(function ($params) {
            $this->memory->writeUInt32($params[2], U32::of(0x30004000));
            $this->memory->writeUInt32($params[3], U32::of(0x30005000));
        });

        $this->shouldWriteLong(0x30004000 + 0x08, -1);
        $this->shouldWriteLong(0x30004000 + 0x0c, 1);
    }

    public function testMaxIdTracksAcrossMultipleDemoEntryLists(): void
    {
        $paths = 0x30001000;
        $groupDefs = 0x30002000;

        // Demo entry 0: ids 0, 2. Demo entry 1: ids 1, 5 (raises the max).
        // Demo entry 2 is NULL, terminating the outer scan.
        $list0 = $this->alloc(0xc);
        $this->initUint32($list0 + 0, 0);
        $this->initUint32($list0 + 4, 2);
        $this->initUint32($list0 + 8, -1);

        $list1 = $this->alloc(0xc);
        $this->initUint32($list1 + 0, 1);
        $this->initUint32($list1 + 4, 5);
        $this->initUint32($list1 + 8, -1);

        $lists = $this->alloc(0xc);
        $this->initUint32($lists + 0, $list0);
        $this->initUint32($lists + 4, $list1);
        $this->initUint32($lists + 8, 0);

        $this->setupCourse($paths, $groupDefs, $lists);

        $groups = 0x30003000;

        $this->call('_ObjectsInstallPedestrianGroups_8c0296d6');

        $this->shouldWriteLongTo('_var_pedPaths_8c228238', $paths);
        $this->shouldWriteLongTo('_var_pedGroupLists_8c228240', $lists);
        $this->shouldWriteLongTo('_var_pedGroupDefs_8c22823c', $groupDefs);
        $this->shouldWriteLong($this->addressOf('_var_pedGroupCount_8c228234'), -1);
        $this->shouldWriteLong($this->addressOf('_var_pedGroupCount_8c228234'), 0); // list0[0]
        $this->shouldWriteLong($this->addressOf('_var_pedGroupCount_8c228234'), 2); // list0[1]
        $this->shouldWriteLong($this->addressOf('_var_pedGroupCount_8c228234'), 5); // list1[1] (list1[0]==1 doesn't raise it)
        $this->shouldWriteLong($this->addressOf('_var_pedGroupCount_8c228234'), 6); // ++

        $this->shouldCall('_syMalloc')->with(6 * 0xc)->andReturn($groups);
        $this->shouldWriteLong($this->addressOf('_var_pedGroups_8c228230'), $groups);
        for ($i = 0; $i < 6; $i++) {
            $this->shouldWriteLong($groups + $i * 0xc, 0);
        }

        $taskLocal = 0xffffe4;
        $stateLocal = 0xffffe8;
        $this->shouldCall('_TaskInstall_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba5e8'),
            $this->addressOf('_pedestriansTask_8c0293f6'),
            $taskLocal,
            $stateLocal,
            0,
        )->do(function ($params) {
            $this->memory->writeUInt32($params[2], U32::of(0x30004000));
            $this->memory->writeUInt32($params[3], U32::of(0x30005000));
        });

        $this->shouldWriteLong(0x30004000 + 0x08, -1);
        $this->shouldWriteLong(0x30004000 + 0x0c, 1);
    }
};
