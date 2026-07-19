<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_frees_the_four_task_groups()
    {
        $this->call('_FUN_8c01614c');

        $this->shouldCall('_FUN_8c0297da');
        $this->shouldCall('_FUN_8c0288be');
        $this->shouldCall('_FUN_8c02ca96');
        $this->shouldCall('_TaskFreeGroup_8c014ab4')->with($this->addressOf('_var_tasks_8c1bb448'));
        $this->shouldCall('_TaskFreeGroup_8c014ab4')->with($this->addressOf('_var_tasks_8c1bac28'));
        $this->shouldCall('_TaskFreeGroup_8c014ab4')->with($this->addressOf('_var_tasks_8c1ba808'));
        $this->shouldCall('_TaskFreeGroup_8c014ab4')->with($this->addressOf('_var_tasks_8c1ba5e8'));
    }
};
