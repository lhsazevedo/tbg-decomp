<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private const SEQ_INTRO_BRIEFING = 0;
    private const SEQ_CHOOSE_COURSE = 6;
    private const SEQ_SUCCESS_2 = 7;
    private const SEQ_SUCCESS = 8;
    private const SEQ_AWARD_BADGE_GOLD = 9;
    private const SEQ_AWARD_BADGE_SILVER = 10;
    private const SEQ_AWARD_BADGE_BRONZE = 11;
    private const SEQ_FAILURE = 12;
    private const SEQ_COURSE_UNLOCKED = 13;
    private const SEQ_PASSENGER_LETTER_RECEIVED = 14;

    public function test_choose_course()
    {
        $this->setSize('_var_dialogSequences_8c225fbc', 4 * 4);
        $this->setSize('_var_8c1ba1cc', 0x94);

        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 0);

        $this->call('_buildCourseMenuDialogFlow_8c017420');
        $this->shouldPushSequence(self::SEQ_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_intro_briefing()
    {
        $this->setSize('_var_dialogSequences_8c225fbc', 4 * 4);
        $this->setSize('_var_8c1ba1cc', 0x94);

        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0, 1);

        $this->call('_buildCourseMenuDialogFlow_8c017420');
        $this->shouldPushSequence(self::SEQ_INTRO_BRIEFING);
        $this->shouldPushSequence(self::SEQ_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_success_2()
    {
        $this->setSize('_var_dialogSequences_8c225fbc', 4 * 4);
        $this->setSize('_var_8c1ba1cc', 0x94);

        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0, 0);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 1);

        $this->call('_buildCourseMenuDialogFlow_8c017420');
        $this->shouldPushSequence(self::SEQ_SUCCESS_2);
        $this->shouldPushSequence(self::SEQ_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_success_without_unlocked_without_letter()
    {
        $this->setSize('_var_dialogSequences_8c225fbc', 4 * 4);
        $this->setSize('_var_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);

        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0, 0);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8dc'), 1);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $this->call('_buildCourseMenuDialogFlow_8c017420');
        $this->shouldPushSequence(self::SEQ_SUCCESS);
        $this->shouldCall('_FUN_checkCourses_8c0172dc')->andReturn(0);
        $this->shouldPushSequence(self::SEQ_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_success_without_unlocked_with_letter()
    {
        $this->setSize('_var_dialogSequences_8c225fbc', 4 * 4);
        $this->setSize('_var_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);

        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0, 6);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0x2c + 5 * 4, 0);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8dc'), 1);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $this->call('_buildCourseMenuDialogFlow_8c017420');
        $this->shouldPushSequence(self::SEQ_SUCCESS);
        $this->shouldCall('_FUN_checkCourses_8c0172dc')->andReturn(0);
        $this->shouldCall('_AsqGetRandomInRangeB_121be')->andReturn(5);
        $this->shouldWriteLong(
            $this->addressOf('_var_8c1ba1cc') + 0x2c + 5 * 4 + 0, 1
        );
        $this->shouldPushSequence(self::SEQ_PASSENGER_LETTER_RECEIVED);
        $this->shouldPushSequence(self::SEQ_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_success_with_unlocked_without_letter()
    {
        $this->setSize('_var_dialogSequences_8c225fbc', 4 * 4);
        $this->setSize('_var_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);

        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0, 0);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8dc'), 1);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $this->call('_buildCourseMenuDialogFlow_8c017420');
        $this->shouldPushSequence(self::SEQ_SUCCESS);
        $this->shouldCall('_FUN_checkCourses_8c0172dc')->andReturn(1);
        $this->shouldPushSequence(self::SEQ_COURSE_UNLOCKED);
        $this->shouldPushSequence(self::SEQ_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_success_with_unlocked_with_letter()
    {
        $this->setSize('_var_dialogSequences_8c225fbc', 4 * 4);
        $this->setSize('_var_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);

        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0, 6);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0x2c + 5 * 4, 0);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8dc'), 1);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $this->call('_buildCourseMenuDialogFlow_8c017420');
        $this->shouldPushSequence(self::SEQ_SUCCESS);
        $this->shouldCall('_FUN_checkCourses_8c0172dc')->andReturn(1);
        $this->shouldPushSequence(self::SEQ_COURSE_UNLOCKED);
        $this->shouldCall('_AsqGetRandomInRangeB_121be')->andReturn(5);
        $this->shouldWriteLong(
            $this->addressOf('_var_8c1ba1cc') + 0x2c + 5 * 4 + 0, 1
        );
        $this->shouldPushSequence(self::SEQ_PASSENGER_LETTER_RECEIVED);
        $this->shouldPushSequence(self::SEQ_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_failure_without_unlocked_without_letter()
    {
        $this->setSize('_var_dialogSequences_8c225fbc', 4 * 4);
        $this->setSize('_var_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);

        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0, 0);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8dc'), 0);

        $this->call('_buildCourseMenuDialogFlow_8c017420');
        $this->shouldPushSequence(self::SEQ_FAILURE);
        $this->shouldCall('_FUN_checkCourses_8c0172dc')->andReturn(0);
        $this->shouldPushSequence(self::SEQ_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_failure_without_unlocked_with_letter()
    {
        $this->setSize('_var_dialogSequences_8c225fbc', 4 * 4);
        $this->setSize('_var_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);
        $this->setSize('_AsqGetRandomInRangeB_121be', 4);

        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0, 6);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0x2c + 5 * 4, 0);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8dc'), 0);

        $this->call('_buildCourseMenuDialogFlow_8c017420');
        $this->shouldPushSequence(self::SEQ_FAILURE);
        $this->shouldCall('_FUN_checkCourses_8c0172dc')->andReturn(0);
        $this->shouldCall('_AsqGetRandomInRangeB_121be')->andReturn(5);
        $this->shouldWriteLong(
            $this->addressOf('_var_8c1ba1cc') + 0x2c + 5 * 4 + 0, 1
        );
        $this->shouldPushSequence(self::SEQ_PASSENGER_LETTER_RECEIVED);
        $this->shouldPushSequence(self::SEQ_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_failure_with_unlocked_without_letter()
    {
        $this->setSize('_var_dialogSequences_8c225fbc', 4 * 4);
        $this->setSize('_var_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);

        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0, 0);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8dc'), 0);

        $this->call('_buildCourseMenuDialogFlow_8c017420');
        $this->shouldPushSequence(self::SEQ_FAILURE);
        $this->shouldCall('_FUN_checkCourses_8c0172dc')->andReturn(1);
        $this->shouldPushSequence(self::SEQ_COURSE_UNLOCKED);
        $this->shouldPushSequence(self::SEQ_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }

    public function test_failure_with_unlocked_with_letter()
    {
        $this->setSize('_var_dialogSequences_8c225fbc', 4 * 4);
        $this->setSize('_var_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);
        $this->setSize('_AsqGetRandomInRangeB_121be', 4);

        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0, 6);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0x2c + 5 * 4, 0);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8dc'), 0);

        $this->call('_buildCourseMenuDialogFlow_8c017420');
        $this->shouldPushSequence(self::SEQ_FAILURE);
        $this->shouldCall('_FUN_checkCourses_8c0172dc')->andReturn(1);
        $this->shouldPushSequence(self::SEQ_COURSE_UNLOCKED);
        $this->shouldCall('_AsqGetRandomInRangeB_121be')->andReturn(5);
        $this->shouldWriteLong(
            $this->addressOf('_var_8c1ba1cc') + 0x2c + 5 * 4 + 0, 1
        );
        $this->shouldPushSequence(self::SEQ_PASSENGER_LETTER_RECEIVED);
        $this->shouldPushSequence(self::SEQ_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }


    public function test_bronze_without_unlocked_without_letter()
    {
        $this->generalSuccessTest(
            award: 1,
            sequence: self::SEQ_AWARD_BADGE_BRONZE,
            unlocked: false,
            letter: false
        );
    }

    public function test_bronze_without_unlocked_with_letter()
    {
        $this->generalSuccessTest(
            award: 1,
            sequence: self::SEQ_AWARD_BADGE_BRONZE,
            unlocked: false,
            letter: true
        );
    }

    public function test_bronze_with_unlocked_without_letter()
    {
        $this->generalSuccessTest(
            award: 1,
            sequence: self::SEQ_AWARD_BADGE_BRONZE,
            unlocked: true,
            letter: false
        );
    }

    public function test_bronze_with_unlocked_with_letter()
    {
        $this->generalSuccessTest(
            award: 1,
            sequence: self::SEQ_AWARD_BADGE_BRONZE,
            unlocked: true,
            letter: true
        );
    }


    public function test_silver_without_unlocked_without_letter()
    {
        $this->generalSuccessTest(
            award: 2,
            sequence: self::SEQ_AWARD_BADGE_SILVER,
            unlocked: false,
            letter: false
        );
    }

    public function test_silver_without_unlocked_with_letter()
    {
        $this->generalSuccessTest(
            award: 2,
            sequence: self::SEQ_AWARD_BADGE_SILVER,
            unlocked: false,
            letter: true
        );
    }

    public function test_silver_with_unlocked_without_letter()
    {
        $this->generalSuccessTest(
            award: 2,
            sequence: self::SEQ_AWARD_BADGE_SILVER,
            unlocked: true,
            letter: false
        );
    }

    public function test_silver_with_unlocked_with_letter()
    {
        $this->generalSuccessTest(
            award: 2,
            sequence: self::SEQ_AWARD_BADGE_SILVER,
            unlocked: true,
            letter: true
        );
    }


    public function test_gold_without_unlocked_without_letter()
    {
        $this->generalSuccessTest(
            award: 3,
            sequence: self::SEQ_AWARD_BADGE_GOLD,
            unlocked: false,
            letter: false
        );
    }

    public function test_gold_without_unlocked_with_letter()
    {
        $this->generalSuccessTest(
            award: 3,
            sequence: self::SEQ_AWARD_BADGE_GOLD,
            unlocked: false,
            letter: true
        );
    }

    public function test_gold_with_unlocked_without_letter()
    {
        $this->generalSuccessTest(
            award: 3,
            sequence: self::SEQ_AWARD_BADGE_GOLD,
            unlocked: true,
            letter: false
        );
    }

    public function test_gold_with_unlocked_with_letter()
    {
        $this->generalSuccessTest(
            award: 3,
            sequence: self::SEQ_AWARD_BADGE_GOLD,
            unlocked: true,
            letter: true
        );
    }

    private function shouldPushSequence(int $sequence): void
    {
        static $index = 0;
        $this->shouldWriteLong(
            $this->addressOf('_var_dialogSequences_8c225fbc') + $index++ * 4, $sequence
        );

        if ($sequence === -1) {
            $index = 0;
        }
    }

    private function initDialog8c225fbc(array $values): void
    {
        $this->setSize('_var_dialogSequences_8c225fbc', 4 * 4);
        foreach ($values as $index => $value) {
            $this->initUint32($this->addressOf('_var_dialogSequences_8c225fbc') + $index * 4, $value);
        }
    }

    public function generalSuccessTest(int $award, int $sequence, bool $unlocked, bool $letter)
    {
        $this->setSize('_var_dialogSequences_8c225fbc', 4 * 4);
        $this->setSize('_var_8c1ba1cc', 0x94);
        $this->setSize('__modls', 4);

        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0, $letter ? 6 : 0);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0x2c + 5 * 4, 0);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8dc'), 1);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), $award);

        $this->call('_buildCourseMenuDialogFlow_8c017420');
        $this->shouldPushSequence($sequence);

        $this->shouldCall('_FUN_checkCourses_8c0172dc')->andReturn($unlocked ? 1 : 0);
        if ($unlocked) {
            $this->shouldPushSequence(self::SEQ_COURSE_UNLOCKED);
        }

        if ($letter) {
            $this->shouldCall('_AsqGetRandomInRangeB_121be')->andReturn(5);
            $this->shouldWriteLong(
                $this->addressOf('_var_8c1ba1cc') + 0x2c + 5 * 4 + 0, 1
            );
            $this->shouldPushSequence(self::SEQ_PASSENGER_LETTER_RECEIVED);
        }

        $this->shouldPushSequence(self::SEQ_CHOOSE_COURSE);
        $this->shouldPushSequence(-1);
    }
};
