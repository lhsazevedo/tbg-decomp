<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_reset_requested_with_queues_idle_returns_to_title()
    {
        $task = $this->setup(resetRequested: 1);

        $this->call('_PauseDemoEndTask_8c012d5a')->with($task);

        $this->shouldCall('_FUN_8c016182');
        $this->shouldWriteLong($this->addressOf('_init_8c03bd80'), 1);
        $this->shouldWriteLong($this->addressOf('_init_8c03bd84'), 0);
    }

    public function test_phase0_start_pressed_starts_fade_and_advances_to_phase1()
    {
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');
        $task = $this->setup(press: 8, phase: 0, counter: 0);

        $this->call('_PauseDemoEndTask_8c012d5a')->with($task);

        $this->shouldCall('_updateAdxVolFade_8c010a40');
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldWriteLong($task + 8, 1);
        $this->shouldCall('_startAdxFadeOut_8c010bae')->with(0);
        $this->shouldCall('_startAdxFadeOut_8c010bae')->with(1);
        $this->shouldCall('_push_fadeout_8c022b60')->with(0x1e);
        $this->shouldCall('_FUN_8c02239c');
        $this->shouldCall('_execTasks_8c014b42')->with($this->addressOf('_var_tasks_8c1ba5e8'));
        $this->shouldCall('_FUN_8c022910');
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x7b, 0.0, 0.0, -1.1);
    }

    public function test_phase0_no_start_before_timeout_just_counts_and_draws()
    {
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');
        $task = $this->setup(press: 0, phase: 0, counter: 5);

        $this->call('_PauseDemoEndTask_8c012d5a')->with($task);

        $this->shouldCall('_updateAdxVolFade_8c010a40');
        $this->shouldWriteLong($task + 0xc, 6);
        $this->shouldCall('_FUN_8c02239c');
        $this->shouldCall('_execTasks_8c014b42')->with($this->addressOf('_var_tasks_8c1ba5e8'));
        $this->shouldCall('_FUN_8c022910');
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x7b, 0.0, 0.0, -1.1);
    }

    public function test_phase0_timeout_starts_fade_and_advances_to_phase2()
    {
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');
        $task = $this->setup(press: 0, phase: 0, counter: 0x708);

        $this->call('_PauseDemoEndTask_8c012d5a')->with($task);

        $this->shouldCall('_updateAdxVolFade_8c010a40');
        $this->shouldWriteLong($task + 0xc, 0x709);
        $this->shouldWriteLong($task + 8, 2);
        $this->shouldCall('_startAdxFadeOut_8c010bae')->with(0);
        $this->shouldCall('_startAdxFadeOut_8c010bae')->with(1);
        $this->shouldCall('_push_fadeout_8c022b60')->with(0x1e);
        $this->shouldCall('_FUN_8c02239c');
        $this->shouldCall('_execTasks_8c014b42')->with($this->addressOf('_var_tasks_8c1ba5e8'));
        $this->shouldCall('_FUN_8c022910');
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x7b, 0.0, 0.0, -1.1);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x7c, 0.0, 0.0, -1.1);
    }

    public function test_phase1_still_fading_counts_and_draws()
    {
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');
        $task = $this->setup(phase: 1, counter: 0, isFading: 1);

        $this->call('_PauseDemoEndTask_8c012d5a')->with($task);

        $this->shouldCall('_updateAdxVolFade_8c010a40');
        $this->shouldWriteLong($task + 0xc, 1);
        $this->shouldCall('_FUN_8c02239c');
        $this->shouldCall('_execTasks_8c014b42')->with($this->addressOf('_var_tasks_8c1ba5e8'));
        $this->shouldCall('_FUN_8c022910');
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x7b, 0.0, 0.0, -1.1);
    }

    public function test_phase1_fade_done_returns_to_title_via_skip()
    {
        $task = $this->setup(phase: 1, isFading: 0, init80: 0);

        $this->call('_PauseDemoEndTask_8c012d5a')->with($task);

        $this->shouldCall('_updateAdxVolFade_8c010a40');
        $this->shouldCall('_FUN_8c016182');
        $this->shouldCall('_pushTitle_8c015fd6')->with(1);
    }

    public function test_phase1_fade_done_but_reset_pending_does_nothing()
    {
        $task = $this->setup(phase: 1, isFading: 0, init80: 1);

        $this->call('_PauseDemoEndTask_8c012d5a')->with($task);

        $this->shouldCall('_updateAdxVolFade_8c010a40');
    }

    public function test_phase2_still_fading_counts_and_draws()
    {
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');
        $task = $this->setup(phase: 2, counter: 0, isFading: 1);

        $this->call('_PauseDemoEndTask_8c012d5a')->with($task);

        $this->shouldCall('_updateAdxVolFade_8c010a40');
        $this->shouldWriteLong($task + 0xc, 1);
        $this->shouldCall('_FUN_8c02239c');
        $this->shouldCall('_execTasks_8c014b42')->with($this->addressOf('_var_tasks_8c1ba5e8'));
        $this->shouldCall('_FUN_8c022910');
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x7b, 0.0, 0.0, -1.1);
    }

    public function test_phase2_fade_done_returns_to_title_via_timeout()
    {
        $task = $this->setup(phase: 2, isFading: 0, init80: 0);

        $this->call('_PauseDemoEndTask_8c012d5a')->with($task);

        $this->shouldCall('_updateAdxVolFade_8c010a40');
        $this->shouldCall('_FUN_8c016182');
        $this->shouldCall('_pushTitle_8c015fd6')->with(0);
    }

    private function setup(
        int $resetRequested = 0,
        int $press = 0,
        int $phase = 0,
        int $counter = 0,
        int $isFading = 0,
        int $init80 = 0,
    ): int {
        $this->setSize('_FUN_8c016182', 4);
        $this->setSize('_updateAdxVolFade_8c010a40', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_startAdxFadeOut_8c010bae', 4);
        $this->setSize('_push_fadeout_8c022b60', 4);
        $this->setSize('_pushTitle_8c015fd6', 4);
        $this->setSize('_FUN_8c02239c', 4);
        $this->setSize('_execTasks_8c014b42', 4);
        $this->setSize('_FUN_8c022910', 4);
        $this->setSize('_drawSprite_8c014f54', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_init_8c03bd80', 4);
        $this->setSize('_init_8c03bd84', 4);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_var_peripherals_8c1ba35c', 0x68);
        $this->setSize('_var_midiHandles_8c0fcd28', 4);

        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, $press);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x1234);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), $isFading);
        $this->initUint32($this->addressOf('_init_8c03bd80'), $init80);

        $this->initUint32($this->addressOf('_var_resetRequested_8c157a78'), $resetRequested);
        $this->initUint32($this->addressOf('_var_8c157a7c'), 0);
        $this->initUint32($this->addressOf('_var_queuesAreInitialized_8c157a60'), 0);

        $task = $this->alloc(0x20);
        $this->initUint32($task + 8, $phase);
        $this->initUint32($task + 0xc, $counter);

        return $task;
    }
};
