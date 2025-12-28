<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_itWorks()
    {
        // Resolve modlu symbol
        $this->setSize('__modlu', 4);

        $this->shouldCall('_AsqGetRandomB_121a8')->andReturn(42);

        $this->singleCall('_AsqGetRandomInRangeB_121be')
            ->with(20)
            ->singleShouldReturn(2)
            ->run();
    }
};
