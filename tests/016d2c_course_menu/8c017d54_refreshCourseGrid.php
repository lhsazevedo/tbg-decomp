<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /**
     * Story mode:
     *  - sets cursorVisible_0x48
     *  - calls startCursorMove to recompute the target, then snaps the cursor onto it
     *  - enables the PROFILE FILE and ALBUM buttons [5] and [6]
     *  - propagates courses_0x44[i].unlocked_0x00 into CourseMenuButton[row*5 + 2..4].unlocked_0x04
     */
    public function test_story_mode_initializes_buttons_and_cursor(): void {
        $this->resolveSymbols();

        // Seed game mode = Story (0)
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);

        // Seed cursor target (two 32-bit values copied by the routine)
        $menu = $this->addressOf('_var_menuState_8c1bc7a8');
        $targetX = 0x11223344;
        $targetY = 0x55667788;
        $this->initUint32($menu + 0x28, $targetX);
        $this->initUint32($menu + 0x2C, $targetY);
        // Set current cursor to different values (just to emphasize a change); not asserted as reads
        $this->initUint32($menu + 0x20, 0);
        $this->initUint32($menu + 0x24, 0);

        // Seed PlayerProgress.courses[*].unlocked bytes.
        // Assume CourseProgress stride of 8 is sufficient for unlocked_0x00; we only touch +0.
        $pp = $this->addressOf('_var_progress_8c1ba1cc');
        $baseCourses = $pp + 0x44;
        $unlocked = [1,0,1, 0,1,0, 1,1,0];
        for ($i = 0; $i < 9; $i++) {
            $this->initUint8($baseCourses + $i * 8 + 0, $unlocked[$i]); // unlocked_0x00
        }

        // --- Invoke ---
        $this->call('_refreshCourseGrid_8c017d54');

        // --- Effects (ordered) ---
        // 1) cursorVisible_0x48
        $this->shouldWriteLong($menu + 0x48, 1);

        // 2) helper called to ensure cursor target is up-to-date
        $this->shouldCall('_startCursorMove_8c016dc6');

        $mvn = function () use ($menu) {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            if (!$src->equals($menu + 0x28)) {
                throw new \Exception('Unexpected move source ' . $this->registers[2]->readable());
            }

            if (!$dst->equals($menu + 0x20)) {
                throw new \Exception('Unexpected move dest ' . $this->registers[1]->readable());
            }

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8($dst->value + $i, $this->readUInt8($src->value + $i));
            }
        };

        // 3) copy target -> current cursor (8 bytes)
        $this->shouldCall('__quick_evn_mvn')->do($mvn);

        // 4) enable buttons [5] and [6]
        $btns = $this->addressOf('_init_courseMenuButtons_8c04442c');
        $this->shouldWriteLong($btns + 5 * 0x1C + 0x00, 1);
        $this->shouldWriteLong($btns + 6 * 0x1C + 0x00, 1);

        // 5) propagate 9 unlocked flags to slots [2,3,4], [7,8,9], [12,13,14]
        $map = [2,3,4, 7,8,9, 12,13,14];
        for ($i = 0; $i < 9; $i++) {
            $idx = $map[$i];
            $this->shouldWriteLong($btns + $idx * 0x1C + 0x04, $unlocked[$i]);
        }
    }

    /**
     * Free run disables buttons [5] and [6], and lights a course from
     * courses_0x44[i].new_0x01 instead of unlocked_0x00.
     */
    public function test_free_run_mode_initializes_buttons_and_cursor(): void {
        $this->resolveSymbols();

        // Seed game mode = Free Run (non-zero)
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 1);

        // Seed cursor target and current
        $menu = $this->addressOf('_var_menuState_8c1bc7a8');
        $targetX = 0x99AABBCC;
        $targetY = 0xDDEEFF00;
        $this->initUint32($menu + 0x28, $targetX);
        $this->initUint32($menu + 0x2C, $targetY);
        $this->initUint32($menu + 0x20, 0);
        $this->initUint32($menu + 0x24, 0);

        // Seed the courses' new_0x01 flags
        $pp = $this->addressOf('_var_progress_8c1ba1cc');
        $baseCourses = $pp + 0x44;
        $new = [0,1,0, 1,0,1, 1,0,1];
        for ($i = 0; $i < 9; $i++) {
            $this->initUint8($baseCourses + $i * 8 + 1, $new[$i]);
        }

        // --- Invoke ---
        $this->call('_refreshCourseGrid_8c017d54');

        // --- Effects (ordered) ---
        $this->shouldWriteLong($menu + 0x48, 1);
        $this->shouldCall('_startCursorMove_8c016dc6');

        $mvn = function () use ($menu) {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            if (!$src->equals($menu + 0x28)) {
                throw new \Exception('Unexpected move source ' . $this->registers[2]->readable());
            }

            if (!$dst->equals($menu + 0x20)) {
                throw new \Exception('Unexpected move dest ' . $this->registers[1]->readable());
            }

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8($dst->value + $i, $this->readUInt8($src->value + $i));
            }
        };

        // 3) copy target -> current cursor (8 bytes)
        $this->shouldCall('__quick_evn_mvn')->do($mvn);

        // Buttons [5] and [6] disabled in Free Run
        $btns = $this->addressOf('_init_courseMenuButtons_8c04442c');
        $this->shouldWriteLong($btns + 5 * 0x1C + 0x00, 0);
        $this->shouldWriteLong($btns + 6 * 0x1C + 0x00, 0);

        // Propagate the new flags
        $map = [2,3,4, 7,8,9, 12,13,14];
        for ($i = 0; $i < 9; $i++) {
            $idx = $map[$i];
            $this->shouldWriteLong($btns + $idx * 0x1C + 0x04, $new[$i]);
        }
    }

    private function resolveSymbols(): void {
        $this->setSize('_var_menuState_8c1bc7a8', 0x80);
        $this->setSize('_var_progress_8c1ba1cc', 0x200);

        // Game mode flag (0 = Story, !=0 = Free Run)
        $this->setSize('_var_gameMode_8c1bb8fc', 4);
    }
};
