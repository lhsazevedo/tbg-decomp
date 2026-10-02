<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // PedGroupEntry offsets.
    const OFF_ACTIVE = 0x00;
    const OFF_WANTED = 0x04;
    const OFF_LIST = 0x08;
    const ENTRY_SIZE = 0x0c;

    public function test_not_yet_loaded_is_a_noop(): void
    {
        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), -1);

        $this->call('_ObjectsUninstallPedestrianGroups_8c0297da');
    }

    public function test_frees_active_groups_then_the_array(): void
    {
        $groups = $this->alloc(2 * self::ENTRY_SIZE);

        $subTasksA = $this->alloc(4);
        $this->initUint32($groups + 0 * self::ENTRY_SIZE + self::OFF_ACTIVE, 1);
        $this->initUint32($groups + 0 * self::ENTRY_SIZE + self::OFF_WANTED, 0);
        $this->initUint32($groups + 0 * self::ENTRY_SIZE + self::OFF_LIST, $subTasksA);

        // inactive slot: must not be touched
        $this->initUint32($groups + 1 * self::ENTRY_SIZE + self::OFF_ACTIVE, 0);
        $this->initUint32($groups + 1 * self::ENTRY_SIZE + self::OFF_WANTED, 0);
        $this->initUint32($groups + 1 * self::ENTRY_SIZE + self::OFF_LIST, 0);

        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), $groups);
        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), 2);

        $this->call('_ObjectsUninstallPedestrianGroups_8c0297da');

        $this->shouldCall('_TaskUninstallGroup_8c014ab4')->with($subTasksA);
        $this->shouldCall('_syFree')->with($subTasksA);
        $this->shouldCall('_syFree')->with($groups);
        $this->shouldWriteLong($this->addressOf('_var_pedGroups_8c228230'), -1);
        $this->shouldWriteLong($this->addressOf('_var_pedGroupCount_8c228234'), -1);
    }
};
