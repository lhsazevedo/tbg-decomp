<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    public function test_switches_to_free_run_mode()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->initUint32($this->addressOf('_var_game_mode_8c1bb8fc'), 1);

        $text1 = $this->allocString('Hey there!');
        $dialogs1 = $this->alloc(0x8 * 3);
        $this->initUint32($dialogs1 + 0x00, $text1);
        $this->initUint32($dialogs1 + 0x04, 1);

        $text2 = $this->allocString('Hey there!');
        $dialogs2 = $this->alloc(0x8 * 3);
        $this->initUint32($dialogs2 + 0x00, $text2);
        $this->initUint32($dialogs2 + 0x04, 42);

        $this->initUint32($this->addressOf('_init_8c044c08') + 0x00, $dialogs1);
        $this->initUint32($this->addressOf('_init_8c044c08') + 0x04, $dialogs2);
        $this->initUint32($this->addressOf('_var_dialog_8c225fbc'), 1);

        $this->call('_CourseMenuSwitchFromTask_8c017e18')->with($task);

        $this->shouldCall('_setTaskAction_8c014b3e')->with(
            $task, $this->addressOf('_FreeRunMenuTask_8c017ada')
        );
        $this->shouldCall('_FUN_8c017a20');

        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x60, 42
        );
        $this->shouldWriteLong($task + 0x08, 0);
        $this->shouldWriteLongTo('_var_8c225fb8', 0);
        $this->shouldWriteLongTo('_var_demo_8c1bb8d0', 0);

        $this->shouldCall('_FUN_8c017d54');
        $this->shouldCall('_njGarbageTexture')->with(
            $this->addressOf('_var_tex_8c157af8', 0xc00)
        );
        $this->shouldCall('_AsqInitQueues_11f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_11f6c');

        $this->shouldCall('_requestSysResgrp_8c018568')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            $this->addressOf('_init_mainMenuResourceGroup_8c044264'),
        )->andReturn(1);

        $this->shouldCall('_setUknPvmBool_8c014330');
        $this->shouldCall('_AsqProcessQueues_11fe0')->with(
            $this->addressOf('_AsqNop_11120'),
            0,
            0,
            0,
            $this->addressOf('_resetUknPvmBool_8c014322'),
        );
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0);
    }

    public function test_skips_loaded_resource_group_when_switching_to_free_run_mode()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->initUint32($this->addressOf('_var_game_mode_8c1bb8fc'), 1);

        $text1 = $this->allocString('Hey there!');
        $dialogs1 = $this->alloc(0x8 * 3);
        $this->initUint32($dialogs1 + 0x00, $text1);
        $this->initUint32($dialogs1 + 0x04, 1);

        $text2 = $this->allocString('Hey there!');
        $dialogs2 = $this->alloc(0x8 * 3);
        $this->initUint32($dialogs2 + 0x00, $text2);
        $this->initUint32($dialogs2 + 0x04, 42);

        $this->initUint32($this->addressOf('_init_8c044c08') + 0x00, $dialogs1);
        $this->initUint32($this->addressOf('_init_8c044c08') + 0x04, $dialogs2);
        $this->initUint32($this->addressOf('_var_dialog_8c225fbc'), 1);

        $this->call('_CourseMenuSwitchFromTask_8c017e18')->with($task);

        $this->shouldCall('_setTaskAction_8c014b3e')->with(
            $task, $this->addressOf('_FreeRunMenuTask_8c017ada')
        );
        $this->shouldCall('_FUN_8c017a20');

        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x60, 42
        );
        $this->shouldWriteLong($task + 0x08, 0);
        $this->shouldWriteLongTo('_var_8c225fb8', 0);
        $this->shouldWriteLongTo('_var_demo_8c1bb8d0', 0);

        $this->shouldCall('_FUN_8c017d54');
        $this->shouldCall('_njGarbageTexture')->with(
            $this->addressOf('_var_tex_8c157af8', 0xc00)
        );
        $this->shouldCall('_AsqInitQueues_11f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_11f6c');

        $this->shouldCall('_requestSysResgrp_8c018568')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            $this->addressOf('_init_mainMenuResourceGroup_8c044264'),
        )->andReturn(0);

        $this->shouldCall('_AsqFreeQueues_11f7e');
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 1);
        $this->shouldCall('_push_fadein_8c022a9c')->with(10);
        $this->shouldCall('_snd_8c010cd6')->with(0, 15);
    }

    public function test_switches_to_story_mode()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->initUint32($this->addressOf('_var_game_mode_8c1bb8fc'), 0);

        $text1 = $this->allocString('Hey there!');
        $dialogs1 = $this->alloc(0x8 * 3);
        $this->initUint32($dialogs1 + 0x00, $text1);
        $this->initUint32($dialogs1 + 0x04, 1);

        $text2 = $this->allocString('Hey there!');
        $dialogs2 = $this->alloc(0x8 * 3);
        $this->initUint32($dialogs2 + 0x00, $text2);
        $this->initUint32($dialogs2 + 0x04, 42);

        $this->initUint32($this->addressOf('_init_8c044c08') + 0x00, $dialogs1);
        $this->initUint32($this->addressOf('_init_8c044c08') + 0x04, $dialogs2);
        $this->initUint32($this->addressOf('_var_dialog_8c225fbc'), 1);

        $this->call('_CourseMenuSwitchFromTask_8c017e18')->with($task);

        $this->shouldCall('_setTaskAction_8c014b3e')->with(
            $task, $this->addressOf('_StoryMenuTask_8c017718')
        );
        $this->shouldCall('_buildCourseMenuDialogFlow_8c017420');

        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x60, 42
        );
        $this->shouldWriteLong($task + 0x08, 0);
        $this->shouldWriteLongTo('_var_8c225fb8', 0);
        $this->shouldWriteLongTo('_var_demo_8c1bb8d0', 0);

        $this->shouldCall('_FUN_8c017d54');
        $this->shouldCall('_njGarbageTexture')->with(
            $this->addressOf('_var_tex_8c157af8', 0xc00)
        );
        $this->shouldCall('_AsqInitQueues_11f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_11f6c');

        $this->shouldCall('_requestSysResgrp_8c018568')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            $this->addressOf('_init_mainMenuResourceGroup_8c044264'),
        )->andReturn(1);

        $this->shouldCall('_setUknPvmBool_8c014330');
        $this->shouldCall('_AsqProcessQueues_11fe0')->with(
            $this->addressOf('_AsqNop_11120'),
            0,
            0,
            0,
            $this->addressOf('_resetUknPvmBool_8c014322'),
        );
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0);
    }

    public function test_skips_loaded_resource_group_when_switching_to_story_mode()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->initUint32($this->addressOf('_var_game_mode_8c1bb8fc'), 0);

        $text1 = $this->allocString('Hey there!');
        $dialogs1 = $this->alloc(0x8 * 3);
        $this->initUint32($dialogs1 + 0x00, $text1);
        $this->initUint32($dialogs1 + 0x04, 1);

        $text2 = $this->allocString('Hey there!');
        $dialogs2 = $this->alloc(0x8 * 3);
        $this->initUint32($dialogs2 + 0x00, $text2);
        $this->initUint32($dialogs2 + 0x04, 42);

        $this->initUint32($this->addressOf('_init_8c044c08') + 0x00, $dialogs1);
        $this->initUint32($this->addressOf('_init_8c044c08') + 0x04, $dialogs2);
        $this->initUint32($this->addressOf('_var_dialog_8c225fbc'), 1);

        $this->call('_CourseMenuSwitchFromTask_8c017e18')->with($task);

        $this->shouldCall('_setTaskAction_8c014b3e')->with(
            $task, $this->addressOf('_StoryMenuTask_8c017718')
        );
        $this->shouldCall('_buildCourseMenuDialogFlow_8c017420');

        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x60, 42
        );
        $this->shouldWriteLong($task + 0x08, 0);
        $this->shouldWriteLongTo('_var_8c225fb8', 0);
        $this->shouldWriteLongTo('_var_demo_8c1bb8d0', 0);

        $this->shouldCall('_FUN_8c017d54');
        $this->shouldCall('_njGarbageTexture')->with(
            $this->addressOf('_var_tex_8c157af8', 0xc00)
        );
        $this->shouldCall('_AsqInitQueues_11f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_11f6c');

        $this->shouldCall('_requestSysResgrp_8c018568')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            $this->addressOf('_init_mainMenuResourceGroup_8c044264'),
        )->andReturn(0);

        $this->shouldCall('_AsqFreeQueues_11f7e');
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 1);
        $this->shouldCall('_push_fadein_8c022a9c')->with(10);
        $this->shouldCall('_snd_8c010cd6')->with(0, 15);
    }

    public function resolveSymbols()
    {
        $this->setSize('_menuState_8c1bc7a8', 0x6c);
        $this->setSize('_init_8c044c08', 0x08);
        // Functions
    }
};
