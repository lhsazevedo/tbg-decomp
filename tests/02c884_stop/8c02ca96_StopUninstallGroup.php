<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _StopUninstallGroup_8c02ca96(void): tears down the bus-stop task group
 * (var_stopTaskGroup_8c2288f8) if allocated -- frees its tasks via
 * TaskUninstallGroup, frees the backing allocation, then resets the handle to
 * -1 ("not allocated"). No-ops when already -1.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_TaskUninstallGroup_8c014ab4', 4);
        $this->setSize('_syFree', 4);
    }

    public function test_frees_group_when_allocated(): void
    {
        $this->resolveSymbols();

        $group = $this->addressOf('_var_stopTaskGroup_8c2288f8');
        $this->initUint32($group, 0x12345678);

        $this->call('_StopUninstallGroup_8c02ca96')->with();

        $this->shouldCall('_TaskUninstallGroup_8c014ab4')->with(0x12345678);
        $this->shouldCall('_syFree')->with(0x12345678);
        $this->shouldWriteLong($group, -1);
    }

    public function test_noop_when_not_allocated(): void
    {
        $this->resolveSymbols();

        $group = $this->addressOf('_var_stopTaskGroup_8c2288f8');
        $this->initUint32($group, -1);

        $this->call('_StopUninstallGroup_8c02ca96')->with();
    }
};
