<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_advances_day_and_reenters_free_run(): void
    {
        $this->setSize('_var_progress_8c1ba1cc', 4);
        $this->setSize('_var_runSucceeded_8c1bb8dc', 4);
        $this->setSize('_var_runReportPending_8c1bb8b8', 4);
        $this->setSize('_var_runWasPractice_8c1bb8bc', 4);

        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 5); // days_0x00

        $this->call('_onFadeRunFailed_8c02c76a');

        $this->shouldWriteLong($this->addressOf('_var_progress_8c1ba1cc'), 6);
        $this->shouldWriteLong($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0);
        $this->shouldWriteLong($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->shouldWriteLong($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);

        // enterDriveEnd_8c02c738 is a same-object callee with its own dependencies
        // not set up here; mock rather than let it really execute.
        $this->shouldCall('_enterDriveEnd_8c02c738')->andReturn(0);
    }
};
