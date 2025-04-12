<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_itUpdatesAndReturnsValue()
    {
        $this->initUint32($this->addressOf('_var_seed_8c157ad0'), 42);

        $this->shouldWriteTo('_var_seed_8c157ad0', 158);

        $this->singleCall('_AsqGetRandomB_121a8')
            ->singleShouldReturn(158)
            ->run();
    }
};
