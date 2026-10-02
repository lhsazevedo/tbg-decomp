<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // offsetof(PlayerProgress, courses_0x44[3])
    const COURSE3 = 0x44 + 3 * 8;

    private function resolveSymbols(): void
    {
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
        $this->setSize('_var_runState_8c2285c4', 0x9c);
    }

    public function test_zero_score_no_bonuses(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0);
        $this->initUint32($this->addressOf('_var_firstClearOfCourse_8c1bb8e0'), 0);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x0c, 0);
        // ROUTE_SHINJUKU
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0);
        // TIME_OF_DAY_DAY
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0);
        $this->initUint32($this->addressOf('_var_passengerCount_8c1bb8e4'), 0);
        $this->initUint32($this->addressOf('_var_eventCount_8c1bb8e8'), 0);
        // var_award_8c1bb8f8 is only ever byte-written, but read back as a
        // full int for the course-progress comparison below
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $progress = $this->addressOf('_var_progress_8c1ba1cc');
        $this->initUint8($progress + self::COURSE3 + 3, 0); // storyAward_0x03
        $this->initUint32($progress + 0x90, 100); // exp_0x90

        $this->call('_ResultsShowPassedRun_8c01e0b4');

        $this->shouldWriteLong($this->addressOf('_var_scoreCourseClearBonus_8c2263ec'), 0);
        $this->shouldWriteLong($this->addressOf('_var_scoreFirstClearBonus_8c2263f0'), 0);
        $this->shouldWriteLong($this->addressOf('_var_scoreDriverPointsBonus_8c2263f4'), 0);
        $this->shouldWriteByte($this->addressOf('_var_award_8c1bb8f8'), 0);
        $this->shouldWriteByte($this->addressOf('_var_award_8c1bb8f8'), 0);
        $this->shouldWriteLong($this->addressOf('_var_scoreBadgeBonus_8c2263f8'), 0);
        $this->shouldWriteLong($this->addressOf('_var_scorePassengerBonus_8c2263fc'), 0);
        $this->shouldWriteLong($this->addressOf('_var_scoreEventBonus_8c226400'), 0);
        $this->shouldWriteLong($this->addressOf('_var_scoreTotal_8c226404'), 0);
        $this->shouldWriteLong($progress + 0x90, 100);
        $this->shouldWriteLong($this->addressOf('_var_runFailed_8c226408'), 0);
        $this->shouldCall('_spawnResultsTask_8c01df8e');
    }

    public function test_full_bonuses_top_tier_award_upgrade(): void
    {
        $this->resolveSymbols();

        // course1 = courses_0x44[1] (route WANGAN(1) remaps to slot 0, + tod EVENING(1))
        $course1 = 0x44 + 1 * 8;

        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 1);
        $this->initUint32($this->addressOf('_var_firstClearOfCourse_8c1bb8e0'), 1);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x0c, 0x5a); // 90 -> top award tier
        // ROUTE_WANGAN
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 1);
        // TIME_OF_DAY_EVENING
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 1);
        $this->initUint32($this->addressOf('_var_passengerCount_8c1bb8e4'), 2);
        $this->initUint32($this->addressOf('_var_eventCount_8c1bb8e8'), 3);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $progress = $this->addressOf('_var_progress_8c1ba1cc');
        $this->initUint8($progress + $course1 + 3, 0); // storyAward_0x03
        $this->initUint8($progress + $course1 + 4, 0); // freeRunAward_0x04
        $this->initUint32($progress + 0x90, 1000); // exp_0x90

        $this->call('_ResultsShowPassedRun_8c01e0b4');

        // ushort[route*3+tod] = ushort[4] of init_courseClearScoreTable_8c0451a0 (0x118 = 280)
        $this->shouldWriteLong($this->addressOf('_var_scoreCourseClearBonus_8c2263ec'), 280);
        $this->shouldWriteLong($this->addressOf('_var_scoreFirstClearBonus_8c2263f0'), 100);
        $this->shouldWriteLong($this->addressOf('_var_scoreDriverPointsBonus_8c2263f4'), 900);
        $this->shouldWriteByte($this->addressOf('_var_award_8c1bb8f8'), 3);
        $this->shouldWriteLong($this->addressOf('_var_scoreBadgeBonus_8c2263f8'), 200);
        $this->shouldWriteByte($progress + $course1 + 3, 3); // storyAward_0x03
        $this->shouldWriteByte($progress + $course1 + 4, 3); // freeRunAward_0x04
        $this->shouldWriteLong($this->addressOf('_var_scorePassengerBonus_8c2263fc'), 10);
        $this->shouldWriteLong($this->addressOf('_var_scoreEventBonus_8c226400'), 150);
        $this->shouldWriteLong($this->addressOf('_var_scoreTotal_8c226404'), 1640);
        $this->shouldWriteLong($progress + 0x90, 2640);
        $this->shouldWriteLong($this->addressOf('_var_runFailed_8c226408'), 0);
        $this->shouldCall('_spawnResultsTask_8c01df8e');
    }

    public function test_ome_route_low_tier_award_upgrade(): void
    {
        $this->resolveSymbols();

        // course8 = courses_0x44[8] (route OME(2) stays slot 2, + tod NIGHT(2))
        $course8 = 0x44 + 8 * 8;

        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0);
        $this->initUint32($this->addressOf('_var_firstClearOfCourse_8c1bb8e0'), 0);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x0c, 0x46); // 70 -> low award tier
        // ROUTE_OME
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 2);
        // TIME_OF_DAY_NIGHT
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 2);
        $this->initUint32($this->addressOf('_var_passengerCount_8c1bb8e4'), 0);
        $this->initUint32($this->addressOf('_var_eventCount_8c1bb8e8'), 0);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $progress = $this->addressOf('_var_progress_8c1ba1cc');
        $this->initUint8($progress + $course8 + 3, 0); // storyAward_0x03
        $this->initUint8($progress + $course8 + 4, 0); // freeRunAward_0x04
        $this->initUint32($progress + 0x90, 0); // exp_0x90

        $this->call('_ResultsShowPassedRun_8c01e0b4');

        $this->shouldWriteLong($this->addressOf('_var_scoreCourseClearBonus_8c2263ec'), 0);
        $this->shouldWriteLong($this->addressOf('_var_scoreFirstClearBonus_8c2263f0'), 0);
        $this->shouldWriteLong($this->addressOf('_var_scoreDriverPointsBonus_8c2263f4'), 700);
        $this->shouldWriteByte($this->addressOf('_var_award_8c1bb8f8'), 1);
        $this->shouldWriteLong($this->addressOf('_var_scoreBadgeBonus_8c2263f8'), 0x32);
        $this->shouldWriteByte($progress + $course8 + 3, 1); // storyAward_0x03
        $this->shouldWriteByte($progress + $course8 + 4, 1); // freeRunAward_0x04
        $this->shouldWriteLong($this->addressOf('_var_scorePassengerBonus_8c2263fc'), 0);
        $this->shouldWriteLong($this->addressOf('_var_scoreEventBonus_8c226400'), 0);
        $this->shouldWriteLong($this->addressOf('_var_scoreTotal_8c226404'), 750);
        $this->shouldWriteLong($progress + 0x90, 750);
        $this->shouldWriteLong($this->addressOf('_var_runFailed_8c226408'), 0);
        $this->shouldCall('_spawnResultsTask_8c01df8e');
    }

    public function test_mid_tier_award_and_exp_cap(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0);
        $this->initUint32($this->addressOf('_var_firstClearOfCourse_8c1bb8e0'), 0);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x0c, 0x50); // 80 -> mid award tier
        // ROUTE_SHINJUKU
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0);
        // TIME_OF_DAY_DAY
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0);
        $this->initUint32($this->addressOf('_var_passengerCount_8c1bb8e4'), 0);
        $this->initUint32($this->addressOf('_var_eventCount_8c1bb8e8'), 0);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $progress = $this->addressOf('_var_progress_8c1ba1cc');
        $this->initUint8($progress + self::COURSE3 + 3, 0); // storyAward_0x03
        $this->initUint8($progress + self::COURSE3 + 4, 5); // freeRunAward_0x04, already above the award
        $this->initUint32($progress + 0x90, 99500); // exp_0x90, close to the cap

        $this->call('_ResultsShowPassedRun_8c01e0b4');

        $this->shouldWriteLong($this->addressOf('_var_scoreCourseClearBonus_8c2263ec'), 0);
        $this->shouldWriteLong($this->addressOf('_var_scoreFirstClearBonus_8c2263f0'), 0);
        $this->shouldWriteLong($this->addressOf('_var_scoreDriverPointsBonus_8c2263f4'), 800);
        $this->shouldWriteByte($this->addressOf('_var_award_8c1bb8f8'), 2);
        $this->shouldWriteLong($this->addressOf('_var_scoreBadgeBonus_8c2263f8'), 100);
        $this->shouldWriteByte($progress + self::COURSE3 + 3, 2); // storyAward_0x03
        // freeRunAward_0x04 already >= award, not updated
        $this->shouldWriteLong($this->addressOf('_var_scorePassengerBonus_8c2263fc'), 0);
        $this->shouldWriteLong($this->addressOf('_var_scoreEventBonus_8c226400'), 0);
        $this->shouldWriteLong($this->addressOf('_var_scoreTotal_8c226404'), 900);
        $this->shouldWriteLong($progress + 0x90, 100400);
        $this->shouldWriteLong($progress + 0x90, 99999); // capped
        $this->shouldWriteLong($this->addressOf('_var_runFailed_8c226408'), 0);
        $this->shouldCall('_spawnResultsTask_8c01df8e');
    }
};
