<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_itWorks()
    {
        // Resolve modlu symbol
        $this->setSize('__modlu', 4);

        $this->shouldCall('_AsqGetRandomA_12166')->andReturn(42);

        $this->singleCall('_AsqGetRandomInRangeA_12178')
            ->with(20)
            ->singleShouldReturn(2)
            ->run();
    }

    public function test_itSkipsIfParamIsZero()
    {
        // Resolve modlu symbol
        $this->setSize('__modlu', 4);

        $this->singleCall('_AsqGetRandomInRangeA_12178')
            ->with(0)
            ->singleShouldReturn(0)
            ->run();
    }
};
