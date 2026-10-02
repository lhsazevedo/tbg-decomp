<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceRules_8c226410', 4);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_nextStopSegment_8c228710', 4);
        $this->setSize('_var_runReportPending_8c1bb8b8', 4);
        $this->setSize('_var_runWasPractice_8c1bb8bc', 4);
        $this->setSize('_var_gameMode_8c1bb8fc', 4);
        $this->setSize('_var_progress_8c1ba1cc', 4);
        $this->setSize('_var_runSucceeded_8c1bb8dc', 4);
        $this->setSize('_var_tasks_8c1bb448', 4);
        $this->setSize('_var_tasks_8c1bac28', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_var_tasks_8c1ba3c8', 4);
    }

    public function test_threshold_not_met_reloads_route_segment(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 1);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 0); // < 0xf threshold

        $this->call('_onFadeStopEnded_8c02c624');

        $this->shouldCall('_GradeRunComplete_8c02c586')->andReturn(0);

        $this->shouldCall('_ObjectsUninstallPedestrianGroups_8c0297da');
        $this->shouldCall('_SignalFree_8c0288be');
        $this->shouldCall('_TaskUninstallGroup_8c014ab4')->with($this->addressOf('_var_tasks_8c1bb448'));
        $this->shouldCall('_TaskUninstallGroup_8c014ab4')->with($this->addressOf('_var_tasks_8c1bac28'));
        $this->shouldCall('_TaskUninstallGroup_8c014ab4')->with($this->addressOf('_var_tasks_8c1ba5e8'));
        $this->shouldCall('_TaskUninstallGroup_8c014ab4')->with($this->addressOf('_var_tasks_8c1ba3c8'));
        $this->shouldCall('_StopAdvance_8c02ccc6');
        $this->shouldCall('_RouteInstallSegmentReload_8c01468e');
    }

    public function test_threshold_met_course_mode_shows_results(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 1);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 0xf); // meets threshold
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 5); // days_0x00

        $this->call('_onFadeStopEnded_8c02c624');

        $this->shouldCall('_GradeRunComplete_8c02c586')->andReturn(1);

        $this->shouldWriteLong($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->shouldWriteLong($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);
        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');
        $this->shouldWriteLong($this->addressOf('_var_progress_8c1ba1cc'), 6);
        $this->shouldWriteLong($this->addressOf('_var_runSucceeded_8c1bb8dc'), 1);
        $this->shouldCall('_ResultsShowPassedRun_8c01e0b4');
    }
};
