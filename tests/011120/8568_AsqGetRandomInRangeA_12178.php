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

        $this->shouldCall('_AsqGetRandomA_8c012166')->andReturn(42);
        $this->shouldCall('__modlu');

        $this->singleCall('_AsqGetRandomInRangeA_8c012178')
            ->with(20)
            ->singleShouldReturn(2)
            ->run();
    }

    public function test_itSkipsIfParamIsZero()
    {
        // Resolve modlu symbol
        $this->setSize('__modlu', 4);

        $this->singleCall('_AsqGetRandomInRangeA_8c012178')
            ->with(0)
            ->singleShouldReturn(0)
            ->run();
    }
};
