<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * EndingStart_8c01f954(void): entry point, reached once the career's month
 * of days runs out. Picks the ending dialog tier, seeds the instructor sprite
 * from it, spawns the usual per-screen tasks (peripheral support,
 * GameSoftResetTask_8c012f44) plus creditsTask_8c01f658, then sets up the ending's
 * resources.
 */
return new class extends TestCase {
    const STATE_0X18 = 0x18;
    const INSTRUCTOR_SPRITE_0X60 = 0x60;
    const RESOURCE_GROUP_B_0X0C = 0x0c;

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_var_dialogQueue_8c225fbc', 0x10);
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
        $this->setSize('_init_instructorDialogs_8c044c08', 66 * 4);
        $this->setSize('_var_tasks_8c1ba3c8', 4);
        $this->setSize('_var_tex_8c157af8', 4);
        $this->setSize('_var_menuTextboxCharLimit_8c225fb8', 4);
        $this->setSize('_InputSpawnTask_8c0128cc', 4);
        $this->setSize('_GameSoftResetTask_8c012f44', 4);
        $this->setSize('_TaskSpawn_8c014ae8', 4);
        $this->setSize('_njGarbageTexture', 4);
        $this->setSize('_MessageBoxOpenTextbox_8c02ae3e', 4);
        $this->setSize('_MessageBoxSetText_8c02aefc', 4);
        $this->setSize('_AsqInitQueues_8c011f36', 4);
        $this->setSize('_AsqResetQueues_8c011f6c', 4);
        $this->setSize('_CourseMenuRequestSysResgrp_8c018568', 4);
        $this->setSize('_CourseMenuRequestCommonResources_8c01852c', 4);
        $this->setSize('_RouteSetLatch_8c014330', 4);
        $this->setSize('_AsqProcessQueues_8c011fe0', 4);
        $this->setSize('_AsqNop_8c011120', 4);
        $this->setSize('_RouteClearLatch_8c014322', 4);
    }

    // Empirically, EndingStart_8c01f954's MessageBoxOpenTextbox_8c02ae3e call
    // pushes x2 last (closest to sp), then y2, then enable_offset first.
    private function stackArgsCheck(int $x2, int $y2, int $enableOffset): callable
    {
        $mask = 0xffffffff;
        $x2 &= $mask; $y2 &= $mask; $enableOffset &= $mask;
        return function () use ($x2, $y2, $enableOffset) {
            $sp = $this->registers[15]->value;
            $a = $this->memory->readUInt32($sp + 0)->value;
            $b = $this->memory->readUInt32($sp + 4)->value;
            $c = $this->memory->readUInt32($sp + 8)->value;
            if ($a !== $x2 || $b !== $y2 || $c !== $enableOffset) {
                throw new \Exception(sprintf(
                    "Unexpected stack args: got [%d, %d, %d], expecting x2=%d, y2=%d, enable_offset=%d",
                    $a, $b, $c, $x2, $y2, $enableOffset
                ));
            }
        };
    }

    // TaskSpawn's out-params (created_task/create_state) are the callee's own
    // stack-local addresses, unknowable ahead of time and not read back by
    // EndingStart_8c01f954 afterward -- only assert the args that matter.
    private function taskSpawnCheck(int $tasksAddr, int $actionAddr): callable
    {
        return function () use ($tasksAddr, $actionAddr) {
            $r4 = $this->registers[4]->value;
            $r5 = $this->registers[5]->value;
            $allocSize = $this->memory->readUInt32($this->registers[15]->value)->value;
            if ($r4 !== $tasksAddr || $r5 !== $actionAddr || $allocSize !== 0) {
                throw new \Exception(sprintf(
                    "Unexpected TaskSpawn args: tasks=0x%x action=0x%x allocSize=%d",
                    $r4, $r5, $allocSize
                ));
            }
        };
    }

    public function test_basic(): void
    {
        $this->resolveSymbols();

        $tasksAddr = $this->addressOf('_var_tasks_8c1ba3c8');
        $gameTaskAddr = $this->addressOf('_GameSoftResetTask_8c012f44');
        $creditsTaskAddr = $this->addressOf('_creditsTask_8c01f658');
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');

        // selectEndingDialog_8c01f3c0 is a sibling with its own dedicated
        // test (8c01f3c0_selectEndingDialog.php); mock it here and seed the
        // dialog tier it would have picked directly.
        $this->initUint32($this->addressOf('_var_dialogQueue_8c225fbc'), 4);

        $instructorLine = $this->alloc(8);
        $this->initUint32($instructorLine + 4, 0x2a);
        $this->initUint32($this->addressOf('_init_instructorDialogs_8c044c08') + 4 * 4, $instructorLine);

        $this->call('_EndingStart_8c01f954');

        $this->shouldCall('_selectEndingDialog_8c01f3c0');

        $this->shouldWriteLong($menuState + self::INSTRUCTOR_SPRITE_0X60, 0x2a);

        $this->shouldCall('_InputSpawnTask_8c0128cc')->with(0);

        $this->shouldCall('_TaskSpawn_8c014ae8')->do($this->taskSpawnCheck($tasksAddr, $gameTaskAddr));
        $this->shouldCall('_TaskSpawn_8c014ae8')->do($this->taskSpawnCheck($tasksAddr, $creditsTaskAddr));

        $this->shouldWriteLong($menuState + self::STATE_0X18, 0);
        $this->shouldCall('_njGarbageTexture')->with($this->addressOf('_var_tex_8c157af8'), 3072);

        $this->shouldCall('_MessageBoxOpenTextbox_8c02ae3e')
            ->with(32, 384, -2.0, 576, 64)
            ->do($this->stackArgsCheck(0, 0, -1));
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("");

        $this->shouldWriteLongTo('_var_menuTextboxCharLimit_8c225fb8', 0);
        $this->shouldCall('_AsqInitQueues_8c011f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_8c011f6c');

        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')
            ->with($menuState + self::RESOURCE_GROUP_B_0X0C, $this->addressOf('_init_endingResourceGroup_8c045324'));
        $this->shouldCall('_CourseMenuRequestCommonResources_8c01852c');

        $this->shouldCall('_RouteSetLatch_8c014330');
        $this->shouldCall('_AsqProcessQueues_8c011fe0')
            ->with($this->addressOf('_AsqNop_8c011120'), 0, 0, 0,
                   $this->addressOf('_RouteClearLatch_8c014322'));
    }
};
