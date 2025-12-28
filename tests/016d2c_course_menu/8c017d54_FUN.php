<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /**
     * Story mode:
     *  - sets menuState flag at +0x72 to 1
     *  - calls cursorOffTarget helper
     *  - copies cursor target (x,y) from +0x28/+0x2C into current cursor at +0x20/+0x24
     *  - enables non-course buttons [5] and [6]
     *  - propagates PlayerProgress.courses[i].unlocked into CourseMenuButton[ row*5+2..4 ].unlocked
     */
    public function test_story_mode_initializes_buttons_and_cursor(): void {
        $this->resolveSymbols();

        // Seed game mode = Story (0)
        $this->initUint32($this->addressOf('_var_game_mode_8c1bb8fc'), 0);

        // Seed cursor target (two 32-bit values copied by the routine)
        $menu = $this->addressOf('_menuState_8c1bc7a8');
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
        $this->call('_FUN_8c017d54');

        // --- Effects (ordered) ---
        // 1) menuState flag write
        $this->shouldWriteLong($menu + 0x48, 1);

        // 2) helper called to ensure cursor target is up-to-date
        $this->shouldCall('_cursorOffTarget_8c016dc6');

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
     * Free Run mode mirrors Story but disables buttons [5] and [6].
     */
    public function test_free_run_mode_initializes_buttons_and_cursor(): void {
        $this->resolveSymbols();

        // Seed game mode = Free Run (non-zero)
        $this->initUint32($this->addressOf('_var_game_mode_8c1bb8fc'), 1);

        // Seed cursor target and current
        $menu = $this->addressOf('_menuState_8c1bc7a8');
        $targetX = 0x99AABBCC;
        $targetY = 0xDDEEFF00;
        $this->initUint32($menu + 0x28, $targetX);
        $this->initUint32($menu + 0x2C, $targetY);
        $this->initUint32($menu + 0x20, 0);
        $this->initUint32($menu + 0x24, 0);

        // Seed progress unlocked flags
        $pp = $this->addressOf('_var_progress_8c1ba1cc');
        $baseCourses = $pp + 0x44;
        $unlocked = [0,1,0, 1,0,1, 1,0,1];
        for ($i = 0; $i < 9; $i++) {
            $this->initUint8($baseCourses + $i * 8 + 1, $unlocked[$i]);
        }

        // --- Invoke ---
        $this->call('_FUN_8c017d54');

        // --- Effects (ordered) ---
        $this->shouldWriteLong($menu + 0x48, 1);
        $this->shouldCall('_cursorOffTarget_8c016dc6');

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

        // Propagate unlocked flags
        $map = [2,3,4, 7,8,9, 12,13,14];
        for ($i = 0; $i < 9; $i++) {
            $idx = $map[$i];
            $this->shouldWriteLong($btns + $idx * 0x1C + 0x04, $unlocked[$i]);
        }
    }

    private function resolveSymbols(): void {
        // Menu state must cover cursor pos/target and a flag at +0x72
        $this->setSize('_menuState_8c1bc7a8', 0x80);

        // 15 CourseMenuButton entries, sizeof == 0x1C
        $this->setSize('_init_courseMenuButtons_8c04442c', 0x1C * 15);

        // PlayerProgress blob ? we only need courses array starting at +0x44
        $this->setSize('_var_progress_8c1ba1cc', 0x200);

        // Game mode flag (0 = Story, !=0 = Free Run)
        $this->setSize('_var_game_mode_8c1bb8fc', 4);
    }
};
