<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_itWorks()
    {
        // Resolve modlu symbol
        $this->setSize('__modlu', 4);

        $this->onCall('__modlu', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });

        $this->shouldCall('_AsqGetRandomB_8c0121a8')->andReturn(42);
        $this->shouldCall('__modlu');

        $this->singleCall('_AsqGetRandomInRangeB_8c0121be')
            ->with(20)
            ->singleShouldReturn(2)
            ->run();
    }
};
