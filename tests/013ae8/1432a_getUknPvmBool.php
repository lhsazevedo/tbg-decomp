<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_returns_flag()
    {
        $this->initUint32($this->addressOf('_var_8c18adac'), 0x2a);

        $this->call('_getUknPvmBool_8c01432a');

        $this->shouldReturn(0x2a);
    }
};
