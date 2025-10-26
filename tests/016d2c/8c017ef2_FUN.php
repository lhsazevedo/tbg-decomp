<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_story_mode(): void
    {
        $this->initUint32($this->addressOf('_var_game_mode_8c1bb8fc'), 0);

        $this->setSize('_menuState_8c1bc7a8', 0x80);
        $this->setSize('_init_dialogSequences_8c044c08', 0x8);
        $this->initUint32($this->addressOf('_init_dialogSequences_8c044c08') + 0 * 0x04, 0x12341234);
        $sequence = $this->alloc(0x8 * 4);
        $this->initUint32($sequence + 1 * 0x04, 0xbebacafe);
        $this->initUint32($this->addressOf('_init_dialogSequences_8c044c08') + 1 * 0x04, $sequence);
        // First sequence is the second one (index 1)
        $this->initUint32($this->addressOf('_var_dialogQueue_8c225fbc'), 1);

        $this->call('_FUN_8c017ef2');

        $createdTaskLocal = 0xffffe8;
        $createdStateLocal = 0xffffec;
        $this->shouldCall('_FUN_8c0128cc');
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
            $this->addressOf('_StoryMenuTask_8c017718'),
            $createdTaskLocal,
            $createdStateLocal,
            0,
        )->do(function ($params) use ($createdTask) {
            $this->memory->writeUInt32($params[2], U32::of($createdTask));
        });

        $this->shouldCall('_buildCourseMenuDialogFlow_8c017420');

        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x60,
            0xbebacafe,
        );

        $this->shouldWriteLong($createdTask + 0x08, 0);

        $this->shouldWriteLongTo('_var_8c225fb8', 0);

        $this->shouldCall('_njGarbageTexture')->with(
            $this->addressOf('_var_tex_8c157af8'), 0xc00
        );
        $this->shouldCall('_FUN_8c02ae3e')->with(
            0x20, 0x180, -2.0, 0x240, 0x40, 0, 0, -1
        );
        $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with($this->addressOf('_const_8c03628c'));

        $this->shouldWriteLongTo('_var_demo_8c1bb8d0', 0);

        $this->shouldCall('_FUN_8c017d54');
        $this->shouldCall('_AsqInitQueues_11f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_11f6c');
        $this->shouldCall('_requestSysResgrp_8c018568')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            $this->addressOf('_init_mainMenuResourceGroup_8c044264'),
        );
        $this->shouldCall('_requestCommonResources_8c01852c');
        $this->shouldCall('_setUknPvmBool_8c014330');
        $this->shouldCall('_AsqProcessQueues_11fe0')->with(
            $this->addressOf('_AsqNop_11120'),
            0,
            0,
            0,
            $this->addressOf('_resetUknPvmBool_8c014322')
        );
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x18,
            0
        );
    }
};
