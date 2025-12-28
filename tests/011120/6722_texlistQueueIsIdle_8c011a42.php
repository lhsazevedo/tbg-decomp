<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase
{
    public function test_returns(): void
    {
        $this->initUint32($this->addressOf('_var_texlistQueueIsIdle_8c157ab8'), 42);

        $this->singleCall('_texlistQueueIsIdle_8c011a42')
            ->singleShouldReturn(42)
            ->run();
    }
};
