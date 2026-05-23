<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // var_inputMapSel_8c1bb8c8 == 0 -> run inputTask_8c012504.
    public function test_dispatches_to_inputTask_8c012504_when_flag_clear()
    {
        $this->setSize('_var_inputMapSel_8c1bb8c8', 4);
        $this->initUint32($this->addressOf('_var_inputMapSel_8c1bb8c8'), 0);

        $this->call('_dispatchInputTask_8c012970');

        // Tail call in asm, plain call in C; args (R4/R5=0) are not asserted
        // because the C `void` call leaves them undefined.
        $this->shouldCall('_inputTask_8c012504');
    }

    // var_inputMapSel_8c1bb8c8 != 0 -> run inputTaskAlt_8c012718.
    public function test_dispatches_to_inputTaskAlt_8c012718_when_flag_set()
    {
        $this->setSize('_var_inputMapSel_8c1bb8c8', 4);
        $this->initUint32($this->addressOf('_var_inputMapSel_8c1bb8c8'), 1);

        $this->call('_dispatchInputTask_8c012970');

        $this->shouldCall('_inputTaskAlt_8c012718');
    }
};
