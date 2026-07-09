<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_initQueue()
    {
        $this->shouldCall('_initDatQueue_8c011124')->with(42);
        $this->shouldCall('_initNjQueue_8c011430')->with(69);
        $this->shouldCall('_initTexlistQueue_8c0117b8')->with(37);
        $this->shouldCall('_initPvmQueue_8c011a5c')->with(73);
        $this->shouldCall('_vmsLcd_8c01c8fc')->with(2);
        $this->shouldCall('_vmsLcd_8c01c910');

        $this->shouldWriteTo('_var_queuesAreInitialized_8c157a60', 1);

        $this->singleCall('_AsqInitQueues_8c011f36')
            ->with(42, 69, 37, 73)
            ->run();
    }
};
