<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_course_0()
    {
        $this->testCourse(
            course: 0,
            unknown: 127,
            score: 1_000_000,
            writes: [-1]
        );
    }

    public function test_course_1()
    {
        $this->testCourse(
            course: 1, unknown: 8, score: 4000, writes: [1, -1],
        );
    }

    public function test_course_1_unknown_check()
    {
        $this->testCourse(
            course: 1, unknown: 3, score: 4000, writes: [-1],
        );
    }

    public function test_course_1_score_check()
    {
        $this->testCourse(
            course: 1, unknown: 8, score: 3999, writes: [-1],
        );
    }

    public function test_course_2()
    {
        $this->testCourse(
            course: 2, unknown: 9, score: 5500, writes: [2, -1],
        );
    }

    public function test_course_2_unknown_check()
    {
        $this->testCourse(
            course: 2, unknown: 8, score: 5500, writes: [-1],
        );
    }

    public function test_course_2_score_check()
    {
        $this->testCourse(
            course: 2, unknown: 9, score: 5499, writes: [-1],
        );
    }

    public function test_course_3()
    {
        $this->testCourse(
            course: 3, unknown: 5, score: 2000, writes: [3, -1],
        );
    }

    public function test_course_3_unknown_check()
    {
        $this->testCourse(
            course: 3, unknown: 4, score: 2000, writes: [-1],
        );
    }

    public function test_course_3_score_check()
    {
        $this->testCourse(
            course: 3, unknown: 5, score: 1999, writes: [-1],
        );
    }

    public function test_course_4()
    {
        $this->testCourse(
            course: 4, unknown: 11, score: 8000, writes: [4, -1],
        );
    }

    public function test_course_4_unknown_check()
    {
        $this->testCourse(
            course: 4, unknown: 10, score: 8000, writes: [-1],
        );
    }

    public function test_course_4_score_check()
    {
        $this->testCourse(
            course: 4, unknown: 11, score: 7999, writes: [-1],
        );
    }

    public function test_course_5()
    {
        $this->testCourse(
            course: 5, unknown: 13, score: 12000, writes: [5, -1],
        );
    }

    public function test_course_5_unknown_check()
    {
        $this->testCourse(
            course: 5, unknown: 12, score: 12000, writes: [-1],
        );
    }

    public function test_course_5_score_check()
    {
        $this->testCourse(
            course: 5, unknown: 13, score: 11999, writes: [-1],
        );
    }

    public function test_course_6()
    {
        $this->testCourse(
            course: 6, unknown: 127, score: 1_000_000, writes: [-1],
        );
    }

    public function test_course_7()
    {
        $this->testCourse(
            course: 7, unknown: 3, score: 500, writes: [7, -1],
        );
    }

    public function test_course_7_unknown_check()
    {
        $this->testCourse(
            course: 7, unknown: 2, score: 500, writes: [-1],
        );
    }

    public function test_course_7_score_check()
    {
        $this->testCourse(
            course: 7, unknown: 3, score: 499, writes: [-1],
        );
    }

    public function test_course_8()
    {
        $this->testCourse(
            course: 8, unknown: 6, score: 3000, writes: [8, -1],
        );
    }

    public function test_course_8_unknown_check()
    {
        $this->testCourse(
            course: 8, unknown: 5, score: 3000, writes: [-1],
        );
    }

    public function test_course_8_score_check()
    {
        $this->testCourse(
            course: 8, unknown: 6, score: 2999, writes: [-1],
        );
    }

    public function test_all_courses()
    {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initVarCourseSettings8c1ba1cc([0, 0, 0, 0, 0, 0, 0, 0, 0]);
        // Unknown check
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 127);
        // Score check
        $this->initUint32($this->addressOf('_var_exp_8c1ba25c'), 1_000_000);

        // -- Act ----------------------
        $this->call('_CourseMenuBuildCourseUnlockList_8c0172dc');

        // -- Assert -------------------
        foreach ([1, 2, 3, 4, 5, 7, 8, -1] as $offset => $value) {
            $this->shouldWriteByte($this->addressOf('_var_coursesToUnlock_8c225fd4') + $offset, $value);
        }
    }

    public function resolveSymbols()
    {
        $this->setSize('_var_coursesToUnlock_8c225fd4', 4 * 9);
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
    }

    private function initVarCourseSettings8c1ba1cc(array $values)
    {
        if (count($values) !== 9) {
            throw new \InvalidArgumentException('Expected exactly 9 values for _var_progress_8c1ba1cc.');
        }

        foreach ($values as $index => $value) {
            $this->initUint8($this->addressOf('_var_progress_8c1ba1cc') + 0x44 + $index * 8, $value);
        }
    }

    private function testCourse(
        int $course,
        int $unknown,
        int $score,
        array $writes
    ) {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initVarCourseSettings8c1ba1cc([
            0,
            $course === 1 ? 0 : 1,
            $course === 2 ? 0 : 1,
            $course === 3 ? 0 : 1,
            $course === 4 ? 0 : 1,
            $course === 5 ? 0 : 1,
            0,
            $course === 7 ? 0 : 1,
            $course === 8 ? 0 : 1
        ]);
        // Unknown check
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), $unknown);
        // Score check
        $this->initUint32($this->addressOf('_var_exp_8c1ba25c'), $score);

        // -- Act ----------------------
        $this->call('_CourseMenuBuildCourseUnlockList_8c0172dc');

        // -- Assert -------------------
        foreach ($writes as $offset => $value) {
            $this->shouldWriteByte($this->addressOf('_var_coursesToUnlock_8c225fd4') + $offset, $value);
        }

        $this->shouldReturn(count($writes) - 1);
    }
};
