<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x3c);
        $this->setSize('_var_practiceLesson_8c22640c', 4);
        $this->setSize('_var_runReportPending_8c1bb8b8', 4);
        $this->setSize('_var_runWasPractice_8c1bb8bc', 4);
        $this->setSize('_var_runSucceeded_8c1bb8dc', 4);
        $this->setSize('_var_tasks_8c1bb448', 4);
        $this->setSize('_var_tasks_8c1bac28', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_var_tasks_8c1ba3c8', 4);
    }

    public function test_threshold_not_met_and_no_points_reloads_route(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x0c, 1);

        $this->call('_GradeOnFadeDriveEnd_8c02c784');

        $this->shouldCall('_GradeRunComplete_8c02c586')->andReturn(0);

        $this->shouldCall('_ObjectsKillPedestrianGroups_8c0297da');
        $this->shouldCall('_SignalFree_8c0288be');
        $this->shouldCall('_TaskKillGroup_8c014ab4')->with($this->addressOf('_var_tasks_8c1bb448'));
        $this->shouldCall('_TaskKillGroup_8c014ab4')->with($this->addressOf('_var_tasks_8c1bac28'));
        $this->shouldCall('_TaskKillGroup_8c014ab4')->with($this->addressOf('_var_tasks_8c1ba5e8'));
        $this->shouldCall('_TaskKillGroup_8c014ab4')->with($this->addressOf('_var_tasks_8c1ba3c8'));
        $this->shouldCall('_StopUpdateHeadings_8c02ccc6');
        $this->shouldCall('_RouteSpawnSegmentReloadTask_8c01468e');
    }

    public function test_no_points_at_all_skips_teardown_and_waits_for_fade(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x0c, 0);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 7);

        $this->call('_GradeOnFadeDriveEnd_8c02c784');

        // GradeRunComplete_8c02c586 is the left operand of the && so it's still called
        // even though driverPoints <= 0 makes the overall condition false.
        $this->shouldCall('_GradeRunComplete_8c02c586')->andReturn(0);

        $this->shouldWriteLong($this->addressOf('_var_menuState_8c1bc7a8') + 0x38, 7); // selected_0x38
        $this->shouldWriteLong($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->shouldWriteLong($this->addressOf('_var_runWasPractice_8c1bb8bc'), 1);
        $this->shouldWriteLong($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0);

        $this->shouldCall('_beginDriveEnd_8c02c738')->andReturn(0);
    }

    public function test_points_earned_goes_straight_to_practice_retry(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x0c, 5);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 3);

        $this->call('_GradeOnFadeDriveEnd_8c02c784');

        $this->shouldCall('_GradeRunComplete_8c02c586')->andReturn(1);

        $this->shouldWriteLong($this->addressOf('_var_menuState_8c1bc7a8') + 0x38, 3);
        $this->shouldWriteLong($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->shouldWriteLong($this->addressOf('_var_runWasPractice_8c1bb8bc'), 1);
        $this->shouldWriteLong($this->addressOf('_var_runSucceeded_8c1bb8dc'), 1);
        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');
        $this->shouldCall('_PracticeMenuReturn_8c01f21c');
    }
};
