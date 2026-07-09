<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_freeQueue()
    {
        $this->initUint32($this->addressOf('_var_pvmQueue_8c157abc'), 0xbebacafe);

        $this->shouldCall('_freeDatQueue_8c0113d8');
        $this->shouldCall('_freeNjQueue_8c0117a4');
        $this->shouldCall('_freeTexlistQueue_8c011a48');
        $this->shouldCall('_freePvmQueue_8c011e28');
        $this->shouldCall('_vmsLcd_8c01c8fc')->with(0);

        $this->shouldWriteTo('_var_queuesAreInitialized_8c157a60', 0);

        $this->singleCall('_AsqFreeQueues_8c011f7e')->run();
    }
};
