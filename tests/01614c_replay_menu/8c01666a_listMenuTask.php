<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /*
     * init_replayMenuEntries_8c04429c's 19 real debug-menu entries (SHINJYUKU_EVENT..VISUAL_MEMORY),
     * in order, terminated by a "" name. It's owned by C now (see 01614c_replay_menu.c), so
     * both objects have the real table -- no faking/seeding needed here.
     */
    private const ENTRY_NAMES = [
        'SHINJYUKU_EVENT',
        'WANGAN_EVENT',
        'OUME_EVENT',
        'WANGAN_DAY',
        'WANGAN_DAY_AUTO',
        'SHINJYUKU_DAY',
        'SHINJYUKU_EVENING',
        'SHINJYUKU_NIGHT',
        'SHINJYUKU_DAY_AUTO',
        'SHINJYUKU_EVENING',
        'SHINJYUKU_NIGHT_AUTO',
        'OUME_DAY',
        'OUME_DAY_AUTO',
        'WANGAN_NIGHT',
        'OUME_NIGHT',
        'WANGAN_NIGHT_AUTO',
        'OUME_NIGHT_AUTO',
        'REPLAY',
        'VISUAL_MEMORY',
    ];

    private function setupExterns()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34);
        $this->setSize('_TaskUninstall_8c014b66', 4);
        $this->setSize('_njPrintC', 4);
        $this->setSize('_AsqGetRandomA_8c012166', 4);
        // the only foreign entry in init_replayMenuEntries_8c04429c; the rest are
        // real functions in this unit and need no faking
        $this->setSize('_MessageBoxRequestAssets_8c02aa36', 4);
    }

    private function expectNameListPrinted()
    {
        foreach (self::ENTRY_NAMES as $i => $name) {
            $this->shouldCall('_njPrintC')->with((0xc << 16) | (8 + $i), $name);
        }
    }

    public function test_ta_press_frees_task_and_runs_selected_entry()
    {
        $this->setupExterns();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);   // cursor -> entry 1 (WANGAN_EVENT, func MessageBoxRequestAssets_8c02aa36)

        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x4); // PDD_DGT_TA

        $this->call('_listMenuTask_8c01666a')->with($task);

        $this->shouldCall('_TaskUninstall_8c014b66')->with($task);
        $this->shouldWriteLong(
            $this->addressOf('_var_replayMenuCourseSel_8c1bc824'),
            $this->addressOf('_init_replayMenuEntries_8c04429c') + 1 * 0x14 + 0x08
        );
        $this->shouldCall('_MessageBoxRequestAssets_8c02aa36');
    }

    public function test_kd_moves_cursor_down_and_prints_marker()
    {
        $this->setupExterns();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 5);   // cursor starts at entry 5

        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x20); // PDD_DGT_KD

        $this->call('_listMenuTask_8c01666a')->with($task);

        $this->expectNameListPrinted();
        $this->shouldCall('_njPrintC')->with((0xa << 16) | (8 + 6), "-");
        $this->shouldWriteLong($task + 0x08, 6);
        $this->shouldCall('_AsqGetRandomA_8c012166');
    }

    public function test_kd_wraps_from_last_entry_to_first()
    {
        $this->setupExterns();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 18);   // cursor at the last entry

        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x20); // PDD_DGT_KD

        $this->call('_listMenuTask_8c01666a')->with($task);

        $this->expectNameListPrinted();
        $this->shouldCall('_njPrintC')->with((0xa << 16) | (8 + 0), "-");
        $this->shouldWriteLong($task + 0x08, 0);
        $this->shouldCall('_AsqGetRandomA_8c012166');
    }

    public function test_ku_wraps_from_first_entry_to_last()
    {
        $this->setupExterns();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0);   // cursor at the first entry

        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x10); // PDD_DGT_KU

        $this->call('_listMenuTask_8c01666a')->with($task);

        $this->expectNameListPrinted();
        $this->shouldCall('_njPrintC')->with((0xa << 16) | (8 + 18), "-");
        $this->shouldWriteLong($task + 0x08, 18);
        $this->shouldCall('_AsqGetRandomA_8c012166');
    }
};
