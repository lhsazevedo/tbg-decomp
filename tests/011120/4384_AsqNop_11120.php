<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_AsqNop_8c011120()
    {
        $this->singleCall('_AsqNop_8c011120')->run();
    }
};
