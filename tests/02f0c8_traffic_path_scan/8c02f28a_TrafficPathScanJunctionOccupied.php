<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// TrafficPathScanJunctionOccupied_8c02f28a(signalId): if var_occupiedGroup_8c228b44 isn't cached (-1), scans
// var_tasks_8c1bac28 for the first live task whose entry has atGroundJunction_0x50c
// != 0, then walks the -1-separated id groups of var_signalGroups_8c228b40 to find the
// group containing that entry's signalWaitId_0x450 marker, caching its start in
// var_occupiedGroup_8c228b44. Either way, returns 1 if signalId appears (before a -1
// terminator) in that cached group, else 0.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_tasks_8c1bac28', 4 * 0x20);
        $this->setSize('_var_busState_8c1bb9d0', 0x400);
        $this->setSize('_var_playerBus_8c1bbd9c', 4);
    }

    private function makeTask(int $index, int $action, int $state): int {
        $task = $this->addressOf('_var_tasks_8c1bac28') + $index * 0x20;
        $this->initUint32($task + 0x00, $action);
        $this->initUint32($task + 0x04, $state);
        return $task;
    }

    // entry with signalWaitId_0x450 (marker) and atGroundJunction_0x50c (active flag)
    private function makeEntry(int $marker, int $active): int {
        $entry = $this->alloc(0x514);
        $this->initUint32($entry + 0x450, $marker);
        $this->initUint32($entry + 0x50c, $active);
        return $entry;
    }

    private function setTable(array $words): int {
        $table = $this->alloc(count($words) * 4);
        foreach ($words as $i => $w) {
            $this->initUint32($table + $i * 4, $w);
        }
        return $table;
    }

    // Already cached: no task scan, no table walk -- just searches the
    // cached group directly.
    public function test_cached_memberFound_returnsOne(): void {
        $this->resolveSymbols();

        $group = $this->setTable([5, 7, 9, -1]);
        $this->initUint32($this->addressOf('_var_occupiedGroup_8c228b44'), $group);
        $this->initUint32($this->addressOf('_var_signalGroups_8c228b40'), 0xdeadbeef); // must not be read

        $this->call('_TrafficPathScanJunctionOccupied_8c02f28a')->with(7);

        $this->shouldReturn(1);
    }

    public function test_cached_memberNotFound_returnsZero(): void {
        $this->resolveSymbols();

        $group = $this->setTable([5, 7, 9, -1]);
        $this->initUint32($this->addressOf('_var_occupiedGroup_8c228b44'), $group);

        $this->call('_TrafficPathScanJunctionOccupied_8c02f28a')->with(6);

        $this->shouldReturn(0);
    }

    // Not cached, and no task has an active (atGroundJunction_0x50c != 0) entry: the
    // task scan hits the zero-action terminator and returns 0 immediately,
    // leaving var_occupiedGroup_8c228b44 untouched.
    public function test_uncached_noActiveTask_returnsZero(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_occupiedGroup_8c228b44'), -1);
        $this->makeTask(0, 0, 0); // terminator

        $this->call('_TrafficPathScanJunctionOccupied_8c02f28a')->with(7);

        $this->shouldReturn(0);
    }

    // Not cached: finds the active task, walks the table's groups to find
    // the one containing the task's marker, caches it, then finds signalId
    // in that group.
    public function test_uncached_findsGroupAndMember_returnsOne(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_occupiedGroup_8c228b44'), -1);

        $entry = $this->makeEntry(marker: 42, active: 1);
        $this->makeTask(0, 1, $entry);
        $this->makeTask(1, 0, 0); // terminator; must not be reached

        // group 0: [1, 2, -1], group 1: [42, 43, -1] -- marker 42 is in
        // group 1, which also contains signalId 43.
        $table = $this->setTable([1, 2, -1, 42, 43, -1]);
        $this->initUint32($this->addressOf('_var_signalGroups_8c228b40'), $table);

        $this->call('_TrafficPathScanJunctionOccupied_8c02f28a')->with(43);

        $this->shouldWriteLongTo('_var_occupiedGroup_8c228b44', $table + 3 * 4); // group 1's start
        $this->shouldReturn(1);
    }

    public function test_uncached_findsGroupNoMember_returnsZero(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_occupiedGroup_8c228b44'), -1);

        $entry = $this->makeEntry(marker: 42, active: 1);
        $this->makeTask(0, 1, $entry);
        $this->makeTask(1, 0, 0);

        $table = $this->setTable([1, 2, -1, 42, 43, -1]);
        $this->initUint32($this->addressOf('_var_signalGroups_8c228b40'), $table);

        $this->call('_TrafficPathScanJunctionOccupied_8c02f28a')->with(99);

        $this->shouldWriteLongTo('_var_occupiedGroup_8c228b44', $table + 3 * 4); // group 1's start
        $this->shouldReturn(0);
    }

    // A task with a -1 (sentinel) action is skipped even though its state
    // pointer is set; a task with action 0 but not first is the terminator
    // and stops the scan.
    public function test_uncached_skipsSentinelAndSelfExcludedTasks(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_occupiedGroup_8c228b44'), -1);

        $this->makeTask(0, -1, 0xdeadbeef); // sentinel, skipped
        $entry = $this->makeEntry(marker: 5, active: 1);
        $this->makeTask(1, 1, $entry);
        $this->makeTask(2, 0, 0);

        $table = $this->setTable([5, -1]);
        $this->initUint32($this->addressOf('_var_signalGroups_8c228b40'), $table);

        $this->call('_TrafficPathScanJunctionOccupied_8c02f28a')->with(5);

        $this->shouldWriteLongTo('_var_occupiedGroup_8c228b44', $table); // group 0's start
        $this->shouldReturn(1);
    }
};
