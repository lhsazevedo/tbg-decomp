<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_dispatches_to_the_manual_task()
    {
        $this->setSize('_var_driveMode_8c1bb8c8', 4);
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 0);

        $this->call('_InputPollDrive_8c012970');

        // Args (R4/R5) not asserted; void call leaves them undefined.
        $this->shouldCall('_inputManualTask_8c012504');
    }

    public function test_dispatches_to_the_auto_task()
    {
        $this->setSize('_var_driveMode_8c1bb8c8', 4);
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 1);

        $this->call('_InputPollDrive_8c012970');

        $this->shouldCall('_inputAutoTask_8c012718');
    }
};
