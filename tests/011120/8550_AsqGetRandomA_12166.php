<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_itUpdatesAndReturnsValue()
    {
        $this->initUint32($this->addressOf('_var_seed_8c157acc'), 42);

        // 42 * 5 + 13 = 210 + 13 = 223
        $this->shouldWriteTo('_var_seed_8c157acc', 223);

        $this->singleCall('_AsqGetRandomA_12166')
            ->singleShouldReturn(223)
            ->run();
    }
};
