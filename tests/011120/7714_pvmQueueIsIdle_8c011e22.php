<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase
{
    public function test_returns(): void
    {
        $this->initUint32($this->addressOf('_var_pvmQueueIsIdle_8c157ac8'), 42);

        $this->singleCall('_pvmQueueIsIdle_8c011e22')
            ->singleShouldReturn(42)
            ->run();
    }
};
