<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_resgrp_ready_reprocesses_and_enters_init()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->call('_PracticeMenuEnter_8c01f114')->with($task);

        $this->shouldWriteLongTo('_var_playMode_8c1bb8d0', 1);
        $this->shouldCall('_TaskReinstall_8c014b3e')->with($task, $this->addressOf('_lessonMenuTask_8c01ebf2'));

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x44, 0);

        $this->shouldCall('_scrollTowardSelection_8c01ebc8');
        $this->shouldWriteLongTo('_var_lessonAttempts_8c22642c', 0);
        $this->shouldWriteByteTo('_var_award_8c1bb8f8', 0);
        $this->shouldCall('_buildDialogQueue_8c01e992');
        $this->shouldWriteLong($task + 0x08, 0);
        $this->shouldWriteLong($menuStateBase + 0x60, 0x2a);

        $this->shouldCall('_njSetBackColor')->with(0, 0, 0);
        $this->shouldCall('_njGarbageTexture')->with($this->addressOf('_var_tex_8c157af8'), 0xc00);

        $this->shouldCall('_AsqInitQueues_8c011f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_8c011f6c');
        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')->with(
            $menuStateBase + 0x0c,
            $this->addressOf('_init_practice01ResourceGroup_8c044274')
        )->andReturn(1);

        $this->shouldCall('_RouteSetLatch_8c014330');
        $this->shouldCall('_AsqProcessQueues_8c011fe0')->with(
            $this->addressOf('_AsqNop_8c011120'),
            0,
            0,
            0,
            $this->addressOf('_RouteClearLatch_8c014322')
        );

        $this->shouldWriteLong($menuStateBase + 0x18, 0);
    }

    public function test_resgrp_not_ready_frees_queues_and_fades_in()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->call('_PracticeMenuEnter_8c01f114')->with($task);

        $this->shouldWriteLongTo('_var_playMode_8c1bb8d0', 1);
        $this->shouldCall('_TaskReinstall_8c014b3e')->with($task, $this->addressOf('_lessonMenuTask_8c01ebf2'));

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x44, 0);

        $this->shouldCall('_scrollTowardSelection_8c01ebc8');
        $this->shouldWriteLongTo('_var_lessonAttempts_8c22642c', 0);
        $this->shouldWriteByteTo('_var_award_8c1bb8f8', 0);
        $this->shouldCall('_buildDialogQueue_8c01e992');
        $this->shouldWriteLong($task + 0x08, 0);
        $this->shouldWriteLong($menuStateBase + 0x60, 0x2a);

        $this->shouldCall('_njSetBackColor')->with(0, 0, 0);
        $this->shouldCall('_njGarbageTexture')->with($this->addressOf('_var_tex_8c157af8'), 0xc00);

        $this->shouldCall('_AsqInitQueues_8c011f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_8c011f6c');
        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')->with(
            $menuStateBase + 0x0c,
            $this->addressOf('_init_practice01ResourceGroup_8c044274')
        )->andReturn(0);

        $this->shouldCall('_AsqFreeQueues_8c011f7e');
        $this->shouldWriteLong($menuStateBase + 0x18, 1);
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    private function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_TaskReinstall_8c014b3e', 4);
        $this->setSize('_var_menuState_8c1bc7a8', 0x84);
        $this->setSize('_var_tex_8c157af8', 4);
        $this->setSize('_njSetBackColor', 4);
        $this->setSize('_njGarbageTexture', 4);
        $this->setSize('_AsqInitQueues_8c011f36', 4);
        $this->setSize('_AsqResetQueues_8c011f6c', 4);
        $this->setSize('_CourseMenuRequestSysResgrp_8c018568', 4);
        $this->setSize('_RouteSetLatch_8c014330', 4);
        $this->setSize('_AsqProcessQueues_8c011fe0', 4);
        $this->setSize('_AsqNop_8c011120', 4);
        $this->setSize('_RouteClearLatch_8c014322', 4);
        $this->setSize('_AsqFreeQueues_8c011f7e', 4);
        $this->setSize('_RenderStartFadeIn_8c022a9c', 4);

        // var_lessonDialogQueue_8c226414[0] indexes init_instructorDialogs_8c044c08 for the
        // dialog's spriteNo_0x04 (InstructorLine+4).
        $this->initUint32($this->addressOf('_var_lessonDialogQueue_8c226414'), 0);
        $dialog = $this->alloc(8);
        $this->initUint32($dialog + 4, 0x2a);
        $this->initUint32($this->addressOf('_init_instructorDialogs_8c044c08'), $dialog);
    }
};
