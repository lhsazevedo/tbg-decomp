<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /* Mirrors the INSTR_* enum in 016d2c_course_menu.h. */
    private const INSTR_STORY_INTRO = 0;
    private const INSTR_STORY_CHOOSE_COURSE = 6;
    private const INSTR_GOOD_PRACTICE = 7;
    private const INSTR_SUCCESS = 8;
    private const INSTR_AWARD_BADGE_GOLD = 9;
    private const INSTR_AWARD_BADGE_SILVER = 10;
    private const INSTR_AWARD_BADGE_BRONZE = 11;
    private const INSTR_FAILURE_RETRY = 12;
    private const INSTR_COURSE_UNLOCKED = 13;
    private const INSTR_PASSENGER_LETTER = 14;

    public function test_choose_course()
    {
        $this->setSize('_var_progress_8c1ba1cc', 0x94);

        $this->initUint32($this->addressOf('_var_runReportPending_8c1bb8b8'), 0);

        $this->call('_buildStoryDialogQueue_8c017420');
        $this->shouldPushSequence(self::INSTR_STORY_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_intro_briefing()
    {
        $this->setSize('_var_progress_8c1ba1cc', 0x94);

        $this->initUint32($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0, 1);

        $this->call('_buildStoryDialogQueue_8c017420');
        $this->shouldPushSequence(self::INSTR_STORY_INTRO);
        $this->shouldPushSequence(self::INSTR_STORY_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_good_practice()
    {
        $this->setSize('_var_progress_8c1ba1cc', 0x94);

        $this->initUint32($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0, 0);
        $this->initUint32($this->addressOf('_var_runWasPractice_8c1bb8bc'), 1);

        $this->call('_buildStoryDialogQueue_8c017420');
        $this->shouldPushSequence(self::INSTR_GOOD_PRACTICE);
        $this->shouldPushSequence(self::INSTR_STORY_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_success_without_unlocked_without_letter()
    {
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });

        $this->initUint32($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0, 0);
        $this->initUint32($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 1);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $this->call('_buildStoryDialogQueue_8c017420');
        $this->shouldPushSequence(self::INSTR_SUCCESS);
        $this->shouldCall('_CourseMenuBuildCourseUnlockList_8c0172dc')->andReturn(0);
        $this->shouldCall('__modls');
        $this->shouldPushSequence(self::INSTR_STORY_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_success_without_unlocked_with_letter()
    {
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });

        $this->initUint32($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0, 6);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0x2c + 5 * 4, 0);
        $this->initUint32($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 1);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $this->call('_buildStoryDialogQueue_8c017420');
        $this->shouldPushSequence(self::INSTR_SUCCESS);
        $this->shouldCall('_CourseMenuBuildCourseUnlockList_8c0172dc')->andReturn(0);
        $this->shouldCall('__modls');
        $this->shouldCall('_AsqGetRandomInRangeB_8c0121be')->andReturn(5);
        $this->shouldWriteLong(
            $this->addressOf('_var_progress_8c1ba1cc') + 0x2c + 5 * 4 + 0, 1
        );
        $this->shouldPushSequence(self::INSTR_PASSENGER_LETTER);
        $this->shouldPushSequence(self::INSTR_STORY_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_success_with_unlocked_without_letter()
    {
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });

        $this->initUint32($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0, 0);
        $this->initUint32($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 1);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $this->call('_buildStoryDialogQueue_8c017420');
        $this->shouldPushSequence(self::INSTR_SUCCESS);
        $this->shouldCall('_CourseMenuBuildCourseUnlockList_8c0172dc')->andReturn(1);
        $this->shouldPushSequence(self::INSTR_COURSE_UNLOCKED);
        $this->shouldCall('__modls');
        $this->shouldPushSequence(self::INSTR_STORY_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_success_with_unlocked_with_letter()
    {
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });

        $this->initUint32($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0, 6);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0x2c + 5 * 4, 0);
        $this->initUint32($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 1);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $this->call('_buildStoryDialogQueue_8c017420');
        $this->shouldPushSequence(self::INSTR_SUCCESS);
        $this->shouldCall('_CourseMenuBuildCourseUnlockList_8c0172dc')->andReturn(1);
        $this->shouldPushSequence(self::INSTR_COURSE_UNLOCKED);
        $this->shouldCall('__modls');
        $this->shouldCall('_AsqGetRandomInRangeB_8c0121be')->andReturn(5);
        $this->shouldWriteLong(
            $this->addressOf('_var_progress_8c1ba1cc') + 0x2c + 5 * 4 + 0, 1
        );
        $this->shouldPushSequence(self::INSTR_PASSENGER_LETTER);
        $this->shouldPushSequence(self::INSTR_STORY_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_failure_without_unlocked_without_letter()
    {
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });

        $this->initUint32($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0, 0);
        $this->initUint32($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0);

        $this->call('_buildStoryDialogQueue_8c017420');
        $this->shouldPushSequence(self::INSTR_FAILURE_RETRY);
        $this->shouldCall('_CourseMenuBuildCourseUnlockList_8c0172dc')->andReturn(0);
        $this->shouldCall('__modls');
        $this->shouldPushSequence(self::INSTR_STORY_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_failure_without_unlocked_with_letter()
    {
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });
        $this->setSize('_AsqGetRandomInRangeB_8c0121be', 4);

        $this->initUint32($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0, 6);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0x2c + 5 * 4, 0);
        $this->initUint32($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0);

        $this->call('_buildStoryDialogQueue_8c017420');
        $this->shouldPushSequence(self::INSTR_FAILURE_RETRY);
        $this->shouldCall('_CourseMenuBuildCourseUnlockList_8c0172dc')->andReturn(0);
        $this->shouldCall('__modls');
        $this->shouldCall('_AsqGetRandomInRangeB_8c0121be')->andReturn(5);
        $this->shouldWriteLong(
            $this->addressOf('_var_progress_8c1ba1cc') + 0x2c + 5 * 4 + 0, 1
        );
        $this->shouldPushSequence(self::INSTR_PASSENGER_LETTER);
        $this->shouldPushSequence(self::INSTR_STORY_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_failure_with_unlocked_without_letter()
    {
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });

        $this->initUint32($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0, 0);
        $this->initUint32($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0);

        $this->call('_buildStoryDialogQueue_8c017420');
        $this->shouldPushSequence(self::INSTR_FAILURE_RETRY);
        $this->shouldCall('_CourseMenuBuildCourseUnlockList_8c0172dc')->andReturn(1);
        $this->shouldPushSequence(self::INSTR_COURSE_UNLOCKED);
        $this->shouldCall('__modls');
        $this->shouldPushSequence(self::INSTR_STORY_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_failure_with_unlocked_with_letter()
    {
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });
        $this->setSize('_AsqGetRandomInRangeB_8c0121be', 4);

        $this->initUint32($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0, 6);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0x2c + 5 * 4, 0);
        $this->initUint32($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0);

        $this->call('_buildStoryDialogQueue_8c017420');
        $this->shouldPushSequence(self::INSTR_FAILURE_RETRY);
        $this->shouldCall('_CourseMenuBuildCourseUnlockList_8c0172dc')->andReturn(1);
        $this->shouldPushSequence(self::INSTR_COURSE_UNLOCKED);
        $this->shouldCall('__modls');
        $this->shouldCall('_AsqGetRandomInRangeB_8c0121be')->andReturn(5);
        $this->shouldWriteLong(
            $this->addressOf('_var_progress_8c1ba1cc') + 0x2c + 5 * 4 + 0, 1
        );
        $this->shouldPushSequence(self::INSTR_PASSENGER_LETTER);
        $this->shouldPushSequence(self::INSTR_STORY_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }


    public function test_bronze_without_unlocked_without_letter()
    {
        $this->generalSuccessTest(
            award: 1,
            sequence: self::INSTR_AWARD_BADGE_BRONZE,
            unlocked: false,
            letter: false
        );
    }

    public function test_bronze_without_unlocked_with_letter()
    {
        $this->generalSuccessTest(
            award: 1,
            sequence: self::INSTR_AWARD_BADGE_BRONZE,
            unlocked: false,
            letter: true
        );
    }

    public function test_bronze_with_unlocked_without_letter()
    {
        $this->generalSuccessTest(
            award: 1,
            sequence: self::INSTR_AWARD_BADGE_BRONZE,
            unlocked: true,
            letter: false
        );
    }

    public function test_bronze_with_unlocked_with_letter()
    {
        $this->generalSuccessTest(
            award: 1,
            sequence: self::INSTR_AWARD_BADGE_BRONZE,
            unlocked: true,
            letter: true
        );
    }


    public function test_silver_without_unlocked_without_letter()
    {
        $this->generalSuccessTest(
            award: 2,
            sequence: self::INSTR_AWARD_BADGE_SILVER,
            unlocked: false,
            letter: false
        );
    }

    public function test_silver_without_unlocked_with_letter()
    {
        $this->generalSuccessTest(
            award: 2,
            sequence: self::INSTR_AWARD_BADGE_SILVER,
            unlocked: false,
            letter: true
        );
    }

    public function test_silver_with_unlocked_without_letter()
    {
        $this->generalSuccessTest(
            award: 2,
            sequence: self::INSTR_AWARD_BADGE_SILVER,
            unlocked: true,
            letter: false
        );
    }

    public function test_silver_with_unlocked_with_letter()
    {
        $this->generalSuccessTest(
            award: 2,
            sequence: self::INSTR_AWARD_BADGE_SILVER,
            unlocked: true,
            letter: true
        );
    }


    public function test_gold_without_unlocked_without_letter()
    {
        $this->generalSuccessTest(
            award: 3,
            sequence: self::INSTR_AWARD_BADGE_GOLD,
            unlocked: false,
            letter: false
        );
    }

    public function test_gold_without_unlocked_with_letter()
    {
        $this->generalSuccessTest(
            award: 3,
            sequence: self::INSTR_AWARD_BADGE_GOLD,
            unlocked: false,
            letter: true
        );
    }

    public function test_gold_with_unlocked_without_letter()
    {
        $this->generalSuccessTest(
            award: 3,
            sequence: self::INSTR_AWARD_BADGE_GOLD,
            unlocked: true,
            letter: false
        );
    }

    public function test_gold_with_unlocked_with_letter()
    {
        $this->generalSuccessTest(
            award: 3,
            sequence: self::INSTR_AWARD_BADGE_GOLD,
            unlocked: true,
            letter: true
        );
    }

    private function shouldPushSequence(int $sequence): void
    {
        static $index = 0;
        $this->shouldWriteLong(
            $this->addressOf('_var_dialogQueue_8c225fbc') + $index++ * 4, $sequence
        );

        if ($sequence === -1) {
            $index = 0;
        }
    }

    private function initDialog8c225fbc(array $values): void
    {
        foreach ($values as $index => $value) {
            $this->initUint32($this->addressOf('_var_dialogQueue_8c225fbc') + $index * 4, $value);
        }
    }

    public function generalSuccessTest(int $award, int $sequence, bool $unlocked, bool $letter)
    {
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });

        $this->initUint32($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0, $letter ? 6 : 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0x2c + 5 * 4, 0);
        $this->initUint32($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 1);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), $award);

        $this->call('_buildStoryDialogQueue_8c017420');
        $this->shouldPushSequence($sequence);

        $this->shouldCall('_CourseMenuBuildCourseUnlockList_8c0172dc')->andReturn($unlocked ? 1 : 0);
        if (!$unlocked) {
            $this->shouldCall('__modls');
        }
        if ($unlocked) {
            $this->shouldPushSequence(self::INSTR_COURSE_UNLOCKED);
            $this->shouldCall('__modls');
        }

        if ($letter) {
            $this->shouldCall('_AsqGetRandomInRangeB_8c0121be')->andReturn(5);
            $this->shouldWriteLong(
                $this->addressOf('_var_progress_8c1ba1cc') + 0x2c + 5 * 4 + 0, 1
            );
            $this->shouldPushSequence(self::INSTR_PASSENGER_LETTER);
        }

        $this->shouldPushSequence(self::INSTR_STORY_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }
};
