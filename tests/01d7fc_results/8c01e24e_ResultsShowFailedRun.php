<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_sets_no_tally_flag_and_inits_task(): void
    {
        $this->call('_ResultsShowFailedRun_8c01e24e');

        $this->shouldWriteLong($this->addressOf('_var_runFailed_8c226408'), 1);
        $this->shouldCall('_installResults_8c01df8e');
    }
};
