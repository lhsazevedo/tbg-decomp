<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_no_prior_result_queues_choose_only()
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);

        $this->call('_PracticeMenuReturn_8c01f21c');

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x44, 0);
        $this->shouldCall('_scrollTowardSelection_8c01ebc8');
        $this->shouldWriteByteTo('_var_award_8c1bb8f8', 0);

        $dialogQueue = $this->addressOf('_var_lessonDialogQueue_8c226414');
        $this->shouldWriteLong($dialogQueue, 0x18);
        $this->shouldWriteLong($dialogQueue + 4, -1);

        $this->expectCommonTail();
    }

    public function test_prior_result_but_no_improvement_rebuilds_queue()
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_runWasPractice_8c1bb8bc'), 1);
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0);

        $this->call('_PracticeMenuReturn_8c01f21c');

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x44, 0);
        $this->shouldCall('_scrollTowardSelection_8c01ebc8');
        $this->shouldWriteByteTo('_var_award_8c1bb8f8', 0);

        $this->shouldWriteLongTo('_var_lessonAttempts_8c22642c', 43);
        $this->shouldCall('_buildDialogQueue_8c01e992');

        $this->expectCommonTail();
    }

    public function test_prior_result_improves_best_score_sets_award()
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_runWasPractice_8c1bb8bc'), 1);
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 1);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 3);

        $progressBase = $this->addressOf('_var_progress_8c1ba1cc');
        $bestScoreAddr = $progressBase + 0x98 + 3 * 4;
        $this->initUint32($bestScoreAddr, 10);

        $candidateScoreAddr = $this->addressOf('_var_runState_8c2285c4') + 0x0c;
        $this->initUint32($candidateScoreAddr, 20);

        $this->call('_PracticeMenuReturn_8c01f21c');

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x44, 0);
        $this->shouldCall('_scrollTowardSelection_8c01ebc8');
        $this->shouldWriteByteTo('_var_award_8c1bb8f8', 0);

        $this->shouldWriteLong($bestScoreAddr, 20);
        $this->shouldWriteByteTo('_var_award_8c1bb8f8', 1);

        $this->shouldWriteLongTo('_var_lessonAttempts_8c22642c', 43);
        $this->shouldCall('_buildDialogQueue_8c01e992');

        $this->expectCommonTail();
    }

    private function expectCommonTail(): void
    {
        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');

        $this->shouldWriteLongTo('_var_playMode_8c1bb8d0', 1);
        $this->shouldCall('_InputSpawnTask_8c0128cc')->with(0);

        $createdTask = 0x2000;
        // Both calls seed their created_task out-param with a sentinel so the
        // later task->field_0x08 write has a known address to assert against.
        $writeCreatedTask = function ($params) use ($createdTask) {
            $this->memory->writeUInt32($params[2], U32::of($createdTask));
        };

        $this->shouldCall('_TaskSpawn_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_GameSoftResetTask_8c012f44'),
            0xffffe4, // created_task
            0xffffe8, // created_state
            0
        )->do($writeCreatedTask);

        $this->shouldCall('_TaskSpawn_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_lessonMenuTask_8c01ebf2'),
            0xffffe4, // created_task
            0xffffe8, // created_state
            0
        )->do($writeCreatedTask);

        $this->shouldWriteLong($menuStateBase + 0x18, 0);
        $this->shouldWriteLong($menuStateBase + 0x60, 0x2a);
        $this->shouldWriteLong($createdTask + 0x08, 0);

        $this->shouldCall('_njGarbageTexture')->with($this->addressOf('_var_tex_8c157af8'), 0xc00);
        $this->shouldCall('_MessageBoxOpenTextbox_8c02ae3e')->with(0x20, 0x180, -2.0, 0x240, 0x40, 0, 0, -1);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("");

        $this->shouldCall('_AsqInitQueues_8c011f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_8c011f6c');
        $this->shouldWriteLongTo('_var_currentSysResGroupInfo_8c225fb0', -1);
        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')->with(
            $menuStateBase + 0x0c,
            $this->addressOf('_init_practice01ResourceGroup_8c044274')
        );
        $this->shouldCall('_CourseMenuRequestCommonResources_8c01852c');
        $this->shouldCall('_RouteSetLatch_8c014330');
        $this->shouldCall('_AsqProcessQueues_8c011fe0')->with(
            $this->addressOf('_AsqNop_8c011120'),
            0,
            0,
            0,
            $this->addressOf('_RouteClearLatch_8c014322')
        );
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_InputSpawnTask_8c0128cc', 4);
        $this->setSize('_TaskSpawn_8c014ae8', 4);
        $this->setSize('_var_menuState_8c1bc7a8', 0x84);
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
        $this->setSize('_var_runState_8c2285c4', 0x9c);
        $this->setSize('_var_tex_8c157af8', 4);
        $this->setSize('_MessageBoxOpenTextbox_8c02ae3e', 4);
        $this->setSize('_MessageBoxSwapFor_8c02aefc', 4);
        $this->setSize('_AsqInitQueues_8c011f36', 4);
        $this->setSize('_AsqResetQueues_8c011f6c', 4);
        $this->setSize('_CourseMenuRequestSysResgrp_8c018568', 4);
        $this->setSize('_CourseMenuRequestCommonResources_8c01852c', 4);
        $this->setSize('_RouteSetLatch_8c014330', 4);
        $this->setSize('_AsqProcessQueues_8c011fe0', 4);
        $this->setSize('_AsqNop_8c011120', 4);
        $this->setSize('_RouteClearLatch_8c014322', 4);

        $this->initUint32($this->addressOf('_var_lessonAttempts_8c22642c'), 42);

        // var_lessonDialogQueue_8c226414[0] indexes init_instructorDialogs_8c044c08 for the
        // dialog's spriteNo_0x04 (InstructorLine+4). Every branch here
        // ends up with 0x18 (CHOOSE) as the first queued entry, since
        // buildDialogQueue_8c01e992 is mocked out (no real side effects).
        $this->initUint32($this->addressOf('_var_lessonDialogQueue_8c226414'), 0x18);
        $this->setSize('_init_instructorDialogs_8c044c08', 66 * 4);
        $dialog = $this->alloc(8);
        $this->initUint32($dialog + 4, 0x2a);
        $this->initUint32($this->addressOf('_init_instructorDialogs_8c044c08') + 0x18 * 4, $dialog);
    }
};
