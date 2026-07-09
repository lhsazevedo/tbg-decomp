<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_story_mode(): void
    {
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);

        $this->setSize('_var_menuState_8c1bc7a8', 0x80);
        // First sequence is the second one (index 1)
        $this->initUint32($this->addressOf('_var_dialogQueue_8c225fbc'), 1);

        $this->call('_CourseMenuFUN_8c017ef2');

        $createdTaskLocal = 0xffffe8;
        $createdStateLocal = 0xffffec;
        $this->shouldCall('_pushInputTask_8c0128cc');
        $this->shouldCall('_pushTask_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_task_8c012f44'),
            $createdTaskLocal,
            $createdStateLocal,
            0,
        );

        $createdTask = $this->alloc(0xc);
        $this->shouldCall('_pushTask_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_CourseMenuStoryMenuTask_8c017718'),
            $createdTaskLocal,
            $createdStateLocal,
            0,
        )->do(function ($params) use ($createdTask) {
            $this->memory->writeUInt32($params[2], U32::of($createdTask));
        });

        $this->shouldCall('_buildCourseMenuDialogFlow_8c017420');

        $this->shouldWriteLong(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0x60,
            0,
        );

        $this->shouldWriteLong($createdTask + 0x08, 0);

        $this->shouldWriteLongTo('_var_menuTextboxCharLimit_8c225fb8', 0);

        $this->shouldCall('_njGarbageTexture')->with(
            $this->addressOf('_var_tex_8c157af8'), 0xc00
        );
        $this->shouldCall('_FUN_8c02ae3e')->with(
            0x20, 0x180, -2.0, 0x240, 0x40, 0, 0, -1
        );
        $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with("");

        $this->shouldWriteLongTo('_var_playMode_8c1bb8d0', 0);

        $this->shouldCall('_FUN_8c017d54');
        $this->shouldCall('_AsqInitQueues_8c011f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_8c011f6c');
        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')->with(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0x0c,
            $this->addressOf('_init_mainMenuResourceGroup_8c044264'),
        );
        $this->shouldCall('_CourseMenuRequestCommonResources_8c01852c');
        $this->shouldCall('_setPvmReady_8c014330');
        $this->shouldCall('_AsqProcessQueues_8c011fe0')->with(
            $this->addressOf('_AsqNop_8c011120'),
            0,
            0,
            0,
            $this->addressOf('_resetPvmReady_8c014322')
        );
        $this->shouldWriteLong(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0x18,
            0
        );
    }

    /*
     * Returning from the practice screen in Free-Run mode (game_mode != 0).
     *
     * The original FUN_8c017ef2 branches on var_gameMode_8c1bb8fc and, for
     * Free-Run, installs CourseMenuFreeRunMenuTask_8c017ada +
     * buildFreeRunMenuDialogFlow_8c017a20. It also passes 0 to
     * pushInputTask_8c0128cc (MOV #0,R4) so the menu input pump is
     * (re)installed.
     *
     * The current decomp omits both: it always pushes the Story task +
     * buildCourseMenuDialogFlow and calls pushInputTask_8c0128cc() with no
     * argument. With no input pump the dialog never sees the TA press and is
     * stuck animating its sin/cos arrow. This test pins the original behavior;
     * it is RED on the current C and GREEN once FUN_8c017ef2 is fixed.
     */
    public function test_free_run_mode(): void
    {
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 1);

        $this->setSize('_var_menuState_8c1bc7a8', 0x80);
        // First sequence is the second one (index 1)
        $this->initUint32($this->addressOf('_var_dialogQueue_8c225fbc'), 1);

        // Enter with a non-zero arg register so the missing `MOV #0,R4`
        // (pushInputTask_8c0128cc(0)) is observable.
        $this->call('_CourseMenuFUN_8c017ef2')->with(0xdeadbeef);

        $createdTaskLocal = 0xffffe8;
        $createdStateLocal = 0xffffec;
        $this->shouldCall('_pushInputTask_8c0128cc')->with(0);
        $this->shouldCall('_pushTask_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_task_8c012f44'),
            $createdTaskLocal,
            $createdStateLocal,
            0,
        );

        $createdTask = $this->alloc(0xc);
        $this->shouldCall('_pushTask_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_CourseMenuFreeRunMenuTask_8c017ada'),
            $createdTaskLocal,
            $createdStateLocal,
            0,
        )->do(function ($params) use ($createdTask) {
            $this->memory->writeUInt32($params[2], U32::of($createdTask));
        });

        $this->shouldCall('_buildFreeRunMenuDialogFlow_8c017a20');

        $this->shouldWriteLong(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0x60,
            0,
        );

        $this->shouldWriteLong($createdTask + 0x08, 0);

        $this->shouldWriteLongTo('_var_menuTextboxCharLimit_8c225fb8', 0);

        $this->shouldCall('_njGarbageTexture')->with(
            $this->addressOf('_var_tex_8c157af8'), 0xc00
        );
        $this->shouldCall('_FUN_8c02ae3e')->with(
            0x20, 0x180, -2.0, 0x240, 0x40, 0, 0, -1
        );
        $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with("");

        $this->shouldWriteLongTo('_var_playMode_8c1bb8d0', 0);

        $this->shouldCall('_FUN_8c017d54');
        $this->shouldCall('_AsqInitQueues_8c011f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_8c011f6c');
        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')->with(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0x0c,
            $this->addressOf('_init_mainMenuResourceGroup_8c044264'),
        );
        $this->shouldCall('_CourseMenuRequestCommonResources_8c01852c');
        $this->shouldCall('_setPvmReady_8c014330');
        $this->shouldCall('_AsqProcessQueues_8c011fe0')->with(
            $this->addressOf('_AsqNop_8c011120'),
            0,
            0,
            0,
            $this->addressOf('_resetPvmReady_8c014322')
        );
        $this->shouldWriteLong(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0x18,
            0
        );
    }
};
