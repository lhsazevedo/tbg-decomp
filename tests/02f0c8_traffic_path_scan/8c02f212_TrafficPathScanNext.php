<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// TrafficPathScanNext_8c02f212(): continues the sample scan TrafficPathScanFirst_8c02f0c8 started. Walks the
// (x, z) pairs in var_samples_8c228b48 from cursor var_sampleCursor_8c228b9c up to bound
// var_sampleEnd_8c228ba0, looking for the state of a live, non-excluded
// (var_excludedTask_8c228b98) task whose entry sits within 2.5 of a sample -- returning
// it (and advancing the cursor past that pair) or NULL once the bound is
// reached.
//
// Real asm quirk, preserved: the cursor advances by a *different* amount
// per task checked, not per sample pair -- 0 if the task is skipped
// (sentinel or excluded), 1 if its entry fails the x check, 2 if x passes
// but z fails. Only a full x+z match consumes the pair "properly" (+2) via
// the success path. This means a failed x check on one task shifts every
// later read in the same pass by one float, not one pair.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_tasks_8c1bac28', 4 * 0x20);
    }

    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    private function makeTask(int $index, int $action, int $state): int {
        $task = $this->addressOf('_var_tasks_8c1bac28') + $index * 0x20;
        $this->initUint32($task + 0x00, $action);
        $this->initUint32($task + 0x04, $state);
        return $task;
    }

    private function makeCandidate(float $x, float $z): int {
        $entry = $this->alloc(0x100);
        $this->initFloat($entry + 0xf4, $x);
        $this->initFloat($entry + 0xfc, $z);
        return $entry;
    }

    // Cursor already at (or past) the bound: returns NULL immediately,
    // without ever touching the task list.
    public function test_cursorAtBound_returnsNull(): void {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_samples_8c228b48');
        $this->initUint32($this->addressOf('_var_sampleCursor_8c228b9c'), $base);
        $this->initUint32($this->addressOf('_var_sampleEnd_8c228ba0'), $base);

        $this->call('_TrafficPathScanNext_8c02f212');

        $this->shouldReturn(0);
    }

    // The first task's entry matches the sample pair on both axes: returns
    // its state, cursor set to just past the pair.
    public function test_firstTaskMatches_returnsItsState(): void {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_samples_8c228b48');
        $this->initFloat($base + 0x00, 10.0);
        $this->initFloat($base + 0x04, 20.0);
        $this->initUint32($this->addressOf('_var_sampleCursor_8c228b9c'), $base);
        $this->initUint32($this->addressOf('_var_sampleEnd_8c228ba0'), $base + 0x08);
        $this->initUint32($this->addressOf('_var_excludedTask_8c228b98'), 0); // exclude nothing

        $candidate = $this->makeCandidate(10.0, 20.0);
        $this->makeTask(0, 1, $candidate);
        $this->makeTask(1, 0, 0);

        $this->call('_TrafficPathScanNext_8c02f212');

        $this->shouldWriteLongTo('_var_sampleCursor_8c228b9c', $base + 0x08);
        $this->shouldReturn($candidate);
    }

    // Task 0 is excluded via var_excludedTask_8c228b98 (advance 0 -- the cursor doesn't
    // move for it); task 1 is the real, matching candidate.
    public function test_excludedTaskSkipped_nextTaskMatches(): void {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_samples_8c228b48');
        $this->initFloat($base + 0x00, 10.0);
        $this->initFloat($base + 0x04, 20.0);
        $this->initUint32($this->addressOf('_var_sampleCursor_8c228b9c'), $base);
        $this->initUint32($this->addressOf('_var_sampleEnd_8c228ba0'), $base + 0x08);

        $excludedCandidate = $this->makeCandidate(10.0, 20.0); // would match, but excluded
        $excludedTask = $this->makeTask(0, 1, $excludedCandidate);
        $this->initUint32($this->addressOf('_var_excludedTask_8c228b98'), $excludedTask);

        $candidate = $this->makeCandidate(10.0, 20.0);
        $this->makeTask(1, 1, $candidate);
        $this->makeTask(2, 0, 0);

        $this->call('_TrafficPathScanNext_8c02f212');

        $this->shouldWriteLongTo('_var_sampleCursor_8c228b9c', $base + 0x08);
        $this->shouldReturn($candidate);
    }

    // A -1 (sentinel) action task is also skipped with advance 0.
    public function test_sentinelTaskSkipped_nextTaskMatches(): void {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_samples_8c228b48');
        $this->initFloat($base + 0x00, 10.0);
        $this->initFloat($base + 0x04, 20.0);
        $this->initUint32($this->addressOf('_var_sampleCursor_8c228b9c'), $base);
        $this->initUint32($this->addressOf('_var_sampleEnd_8c228ba0'), $base + 0x08);
        $this->initUint32($this->addressOf('_var_excludedTask_8c228b98'), 0);

        $this->makeTask(0, -1, 0xdeadbeef); // sentinel

        $candidate = $this->makeCandidate(10.0, 20.0);
        $this->makeTask(1, 1, $candidate);
        $this->makeTask(2, 0, 0);

        $this->call('_TrafficPathScanNext_8c02f212');

        $this->shouldWriteLongTo('_var_sampleCursor_8c228b9c', $base + 0x08);
        $this->shouldReturn($candidate);
    }

    // Task 0's entry fails the x check against sample[0] -- the cursor
    // advances by just 1 float (not 2), so task 1's x check reads
    // sample[1] as its own "x", and task 1's z check reads sample[2]
    // (the *next* pair's x) as its own "z". Both are engineered to match,
    // demonstrating the misaligned-read quirk survives into a real return.
    public function test_failedXCheck_misalignsNextTasksRead(): void {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_samples_8c228b48');
        $this->initFloat($base + 0x00, 10.0);  // sample0.x
        $this->initFloat($base + 0x04, 20.0);  // sample0.z / task1's "x"
        $this->initFloat($base + 0x08, 30.0);  // sample1.x / task1's "z"
        $this->initFloat($base + 0x0c, 40.0);  // sample1.z
        $this->initUint32($this->addressOf('_var_sampleCursor_8c228b9c'), $base);
        $this->initUint32($this->addressOf('_var_sampleEnd_8c228ba0'), $base + 0x10);
        $this->initUint32($this->addressOf('_var_excludedTask_8c228b98'), 0);

        $mismatched = $this->makeCandidate(999.0, 0.0); // x check fails against 10.0
        $this->makeTask(0, 1, $mismatched);

        $shifted = $this->makeCandidate(20.0, 30.0); // matches sample[1], sample[2]
        $this->makeTask(1, 1, $shifted);
        $this->makeTask(2, 0, 0);

        $this->call('_TrafficPathScanNext_8c02f212');

        // p started at base, task0 fails x -> p += 1 (base+0x04); task1's
        // x check reads p[0] = sample[1] = 20.0 (matches), z check reads
        // p[1] = sample[2] = 30.0 (matches) -> cursor = p + 2 = base+0x0c.
        $this->shouldWriteLongTo('_var_sampleCursor_8c228b9c', $base + 0x0c);
        $this->shouldReturn($shifted);
    }

    // Every task fails outright (empty list): the scan exhausts the task
    // list without ever reaching the bound (since nothing advances the
    // cursor), so it loops back and re-scans the same (still-empty) list
    // forever -- guard this with a bound reached via a task that partially
    // advances the cursor across passes instead. Here a single task fails
    // the x check every pass, advancing the cursor by 1 each time until it
    // reaches the bound.
    public function test_repeatedXFailure_advancesCursorAcrossPasses(): void {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_samples_8c228b48');
        $this->initFloat($base + 0x00, 999.0);
        $this->initFloat($base + 0x04, 999.0);
        $this->initUint32($this->addressOf('_var_sampleCursor_8c228b9c'), $base);
        $this->initUint32($this->addressOf('_var_sampleEnd_8c228ba0'), $base + 0x04);
        $this->initUint32($this->addressOf('_var_excludedTask_8c228b98'), 0);

        $mismatched = $this->makeCandidate(0.0, 0.0); // always fails x
        $this->makeTask(0, 1, $mismatched);
        $this->makeTask(1, 0, 0);

        $this->call('_TrafficPathScanNext_8c02f212');

        // Pass 1: p=base, x check fails -> p = base+1 (== bound) ->
        // outer loop re-checks bound -> NULL.
        $this->shouldReturn(0);
    }
};
